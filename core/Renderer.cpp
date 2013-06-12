/******************************************************************************
Copyright 2010-2013 Rodrigo Hernandez Cordoba

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/
#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include "Renderer.h"
#include "Widget.h"

namespace AeonGUI
{
    static float LanczosKernel ( float  x, int32_t a )
    {
        if ( x == 0.0f )
        {
            return 1.0f;
        }
        else if ( x < a )
        {
            return ( a * sinf ( static_cast<float> ( M_PI ) * x ) * sinf ( static_cast<float> ( M_PI ) * x / a ) ) / static_cast<float> ( M_PI * M_PI ) * x * x;
        }
        return ( 0.0f );
    }

    static float LanczosInterpolation ( float x, int32_t a, uint8_t* samples, int32_t sample_count, uint32_t stride )
    {
        int32_t fx = static_cast<int32_t> ( floorf ( x ) );
        float result = 0;
        int32_t start = fx - a + 1;
        int32_t end = fx + a;
        start = ( start < 0 ) ? start : 0;
        end = ( end < sample_count ) ? end : sample_count;
        for ( int32_t i = start; i < end; ++i )
        {
            result += samples[i * stride] * LanczosKernel ( x - i, a );
        }
    }

    Renderer::Renderer() : font ( NULL ), screen_w ( 0 ), screen_h ( 0 ), screen_bitmap ( NULL ), widgets ( NULL )
    {
    }

    Renderer::~Renderer()
    {
    }

    bool Renderer::Initialize ( )
    {
        return true;
    }

    void Renderer::Finalize()
    {
        font = NULL;
        screen_w = 0;
        screen_h = 0;
        if ( screen_bitmap != NULL )
        {
            delete[] screen_bitmap;
            screen_bitmap = NULL;
        }
    }

    bool Renderer::ChangeScreenSize ( int32_t screen_width, int32_t screen_height )
    {
        screen_w = screen_width;
        screen_h = screen_height;
        if ( screen_bitmap != NULL )
        {
            delete[] screen_bitmap;
        }
        screen_bitmap = new uint8_t[screen_w * screen_h * 4];
        return true;
    }

    void Renderer::SetFont ( Font* newfont )
    {
        font = newfont;
    }

    const Font* Renderer::GetFont()
    {
        return font;
    }

    void Renderer::DrawRect ( Color color, const Rect* rect )
    {
        Color* pixels = reinterpret_cast<Color*> ( screen_bitmap );
        int32_t x1 = ( rect->GetLeft() < 0 ) ? 0 : rect->GetLeft();
        int32_t x2 = ( rect->GetRight() > screen_w ) ? screen_w : rect->GetRight();
        int32_t y1 = ( rect->GetTop() < 0 ) ? 0 : rect->GetTop();
        int32_t y2 = ( rect->GetBottom() > screen_h ) ? screen_h : rect->GetBottom();

        if ( ( x1 > screen_w ) || ( x2 < 0 ) ||
             ( y1 > screen_h ) || ( y2 < 0 ) )
        {
            return;
        }
        for ( int32_t y = y1; y < y2; ++y )
        {
            for ( int32_t x = x1; x < x2; ++x )
            {
                pixels[ ( y * screen_w ) + x].Blend ( color );
            }
        }
    }
    void Renderer::DrawRectOutline ( Color color, const Rect* rect )
    {
        Color* pixels = reinterpret_cast<Color*> ( screen_bitmap );
        int32_t x1 = ( rect->GetLeft() < 0 ) ? 0 : rect->GetLeft();
        int32_t x2 = ( rect->GetRight() > screen_w ) ? ( screen_w - 1 ) : ( rect->GetRight() - 1 );
        int32_t y1 = ( rect->GetTop() < 0 ) ? 0 : rect->GetTop();
        int32_t y2 = ( rect->GetBottom() > screen_h ) ? ( screen_h - 1 ) : ( rect->GetBottom() - 1 );

        if ( ( x1 >= screen_w ) || ( x2 < 0 ) ||
             ( y1 >= screen_h ) || ( y2 < 0 ) )
        {
            return;
        }

        for ( int32_t y = y1; y <= y2; ++y )
        {
            pixels[ ( y * screen_w ) + x1].Blend ( color );
            pixels[ ( y * screen_w ) + x2].Blend ( color );
        }
        // Avoid setting the corner pixels twice
        for ( int32_t x = x1 + 1; x < x2; ++x )
        {
            pixels[ ( y1 * screen_w ) + x].Blend ( color );
            pixels[ ( y2 * screen_w ) + x].Blend ( color );
        }
    }

    void Renderer::DrawImage ( Image* image, int32_t x, int32_t y, int32_t w, int32_t h )
    {
        assert ( image != NULL );
        const Color* image_bitmap = image->GetBitmap();
        Color* pixels = reinterpret_cast<Color*> ( screen_bitmap );

        int32_t x1 = x;
        int32_t x2 = x + image->GetWidth();
        int32_t y1 = y;
        int32_t y2 = y + image->GetHeight();

        if ( ( x1 > screen_w ) || ( x2 < 0 ) ||
             ( y1 > screen_h ) || ( y2 < 0 ) )
        {
            return;
        }

        int32_t iy = 0;
        for ( int32_t sy = y1; sy < y2; ++sy )
        {
            if ( ( sy >= 0 ) && ( sy < screen_h ) )
            {
                int32_t ix = 0;
                for ( int32_t sx = x1; sx < x2; ++sx )
                {
                    if ( ( sx >= 0 ) && ( sx < screen_w ) )
                    {
                        pixels[ ( ( sy * screen_w ) + sx )].Blend ( image_bitmap[ ( ( iy * image->GetWidth() ) + ix )] );
                    }
                    ++ix;
                }
            }
            ++iy;
        }
    }

    void Renderer::DrawString ( Color color, int32_t x, int32_t y, const wchar_t* text )
    {
        size_t textlength = wcslen ( text );
        Color* pixels = reinterpret_cast<Color*> ( screen_bitmap );
        int32_t curx;
        int32_t cury;
        Font::Glyph* glyph;
        const uint8_t* glyph_map = font->GetGlyphMap();
        int32_t glyph_map_width = font->GetMapWidth();
        Color pixel;
        pixel.r = color.r;
        pixel.g = color.g;
        pixel.b = color.b;
        for ( size_t i = 0; i < textlength; ++i )
        {
            // Find Character Code
            glyph = font->GetGlyph ( text[i] );

            if ( glyph == NULL )
            {
                continue;
            };

            cury = y - glyph->top;
            for ( int32_t Y = glyph->min[1]; Y < glyph->max[1]; ++Y )
            {
                if ( ( cury >= 0 ) && ( cury < screen_h ) )
                {
                    curx = x + glyph->left;
                    for ( int32_t X = glyph->min[0]; X < ( glyph->max[0] ); ++X )
                    {
                        if ( ( curx >= 0 ) && ( curx < screen_w ) )
                        {
                            pixel.a = glyph_map[ ( Y * glyph_map_width ) + X];
                            pixels[ ( cury * screen_w ) + curx ].Blend ( pixel );
                        }
                        ++curx;
                    }
                }
                ++cury;
            }
            // Advance pen position
            x += glyph->advance[0];
        }
    }

    void Renderer::AddWidget ( Widget* widget )
    {
        if ( widget != NULL )
        {
            if ( widgets == NULL )
            {
                widgets = widget;
            }
            else
            {
                Widget* sibling = widgets;
                while ( sibling != NULL )
                {
                    if ( sibling->next == NULL )
                    {
                        sibling->next = widget;
                        sibling = NULL;
                    }
                    else
                    {
                        sibling = sibling->next;
                    }
                }
            }
        }
    }

    void Renderer::RemoveWidget ( Widget* widget )
    {
        if ( widgets->next == NULL )
        {
            widgets = NULL;
        }
        else
        {
            Widget* sibling = widgets;
            while ( sibling != NULL )
            {
                if ( sibling->next == widget )
                {
                    sibling->next = sibling->next->next;
                    widget->next = NULL;
                    sibling = NULL;
                }
                else
                {
                    sibling = sibling->next;
                }
            }
        }
    }

    void Renderer::RenderWidgets()
    {
        Widget* sibling = widgets;
        while ( sibling != NULL )
        {
            sibling->Render ( this );
            sibling = sibling->next;
        }
    }
}
