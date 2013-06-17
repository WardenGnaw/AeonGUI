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
    static float LanczosKernel ( float  x )
    {
        const float filter = 3.0f;
        const float fx = fabsf ( x );
        if ( fx == 0.0f )
        {
            return ( 1.0f );
        }
        else if ( fx >= filter )
        {
            return ( 0.0f );
        }
        const float pix = fx * static_cast<float> ( M_PI );
        const float pix_over_filter = pix / filter;
        return ( sinf ( pix ) / pix ) * ( sinf ( pix_over_filter ) / ( pix_over_filter ) );
    }

    static Color Lanczos1DInterpolation ( float x, const Color* samples, int32_t sample_count, uint32_t sample_stride )
    {
        const int32_t filter = 3;
        int32_t fx = static_cast<int32_t> ( floorf ( x ) );
        Color result = 0;
        float sum = 0;
        float b = 0, g = 0, r = 0, a = 0;
        float kernel[ ( filter * 2 ) - 1];

        for ( int32_t i = -2; i < 3; ++i )
        {
            sum += kernel[i + 2] = LanczosKernel ( x - ( fx + i ) );
        }

        // Normalize
        for ( int32_t i = 0; i < ( filter * 2 ) - 1; ++i )
        {
            kernel[i] /= sum;
        }

        for ( int32_t i = -2; i < 3; ++i )
        {
            if ( ( fx + i ) < 0 )
            {
                b += samples[0].b * kernel[i + 2];
                g += samples[0].g * kernel[i + 2];
                r += samples[0].r * kernel[i + 2];
                a += samples[0].a * kernel[i + 2];
            }
            else if ( ( fx + i ) >= sample_count )
            {
                b += samples[ ( sample_count - 1 ) * sample_stride].b * kernel[i + 2];
                g += samples[ ( sample_count - 1 ) * sample_stride].g * kernel[i + 2];
                r += samples[ ( sample_count - 1 ) * sample_stride].r * kernel[i + 2];
                a += samples[ ( sample_count - 1 ) * sample_stride].a * kernel[i + 2];
            }
            else
            {
                b += samples[ ( fx + i ) * sample_stride].b * kernel[i + 2];
                g += samples[ ( fx + i ) * sample_stride].g * kernel[i + 2];
                r += samples[ ( fx + i ) * sample_stride].r * kernel[i + 2];
                a += samples[ ( fx + i ) * sample_stride].a * kernel[i + 2];
            }
        }
        result.b = ( b < 0.0f ) ? 0 : ( b > 255.0f ) ? 255 : static_cast<uint8_t> ( b );
        result.g = ( g < 0.0f ) ? 0 : ( g > 255.0f ) ? 255 : static_cast<uint8_t> ( g );
        result.r = ( r < 0.0f ) ? 0 : ( r > 255.0f ) ? 255 : static_cast<uint8_t> ( r );
        result.a = ( a < 0.0f ) ? 0 : ( a > 255.0f ) ? 255 : static_cast<uint8_t> ( a );
        return result;
    }

    static Color NearestNeighbor1DInterpolation ( float x, const Color* samples, int32_t sample_count, uint32_t sample_stride )
    {
        int32_t fx = static_cast<int32_t> ( floorf ( x ) );
        if ( ( x - fx ) < 0.5 )
        {
            return samples[ fx * sample_stride];
        }
        return samples[ ( fx + 1 ) * sample_stride];
    }

    static Color NearestNeighbor2DInterpolation ( float x, float y, uint32_t w, uint32_t h, const Color* buffer )
    {
        assert ( x < w );
        assert ( y < h );
        int32_t fx = static_cast<int32_t> ( floorf ( x ) );
        int32_t fy = static_cast<int32_t> ( floorf ( y ) );
        float   dx = ( x - fx );
        float   dy = ( y - fy );
        if ( dx > 0.5 )
        {
            ++fx;
        }
        if ( dy > 0.5 )
        {
            ++fy;
        }
        return buffer[ ( fy * w ) + fx];
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
        uint32_t image_w = image->GetWidth();
        uint32_t image_h = image->GetHeight();
        if ( w == 0 )
        {
            w = image_w;
        }
        if ( h == 0 )
        {
            h = image_h;
        }
        const Color* image_bitmap = image->GetBitmap();
        Color* pixels = reinterpret_cast<Color*> ( screen_bitmap );

        int32_t x1 = x;
        int32_t x2 = x + w;
        int32_t y1 = y;
        int32_t y2 = y + h;

        if ( ( x1 > screen_w ) || ( x2 < 0 ) ||
             ( y1 > screen_h ) || ( y2 < 0 ) )
        {
            return;
        }

        if ( ( w == image_w ) && ( h == image_h ) )
        {
            // Same as original
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
                            pixels[ ( ( sy * screen_w ) + sx )].Blend ( image_bitmap[ ( ( iy * w ) + ix )] );
                        }
                        ++ix;
                    }
                }
                ++iy;
            }
        }
        else if ( w == image_w )
        {
            // Verticaly Scaled
            float ratio_h = static_cast<float> ( image_h ) / static_cast<float> ( h );
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
                            pixels[ ( ( sy * screen_w ) + sx )].Blend ( Lanczos1DInterpolation ( iy * ratio_h, image_bitmap + ( ix ), image_h, image_w ) );
                        }
                        ++ix;
                    }
                }
                ++iy;
            }
        }
        else if ( h == image_h )
        {
            // Horizontaly Scaled
            float ratio_w = static_cast<float> ( image_w ) / static_cast<float> ( w );
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
                            pixels[ ( ( sy * screen_w ) + sx )].Blend ( Lanczos1DInterpolation ( ix * ratio_w, image_bitmap + ( iy * image_w ), image_w, 1 ) );
                            //pixels[ ( ( sy * screen_w ) + sx )].Blend ( NearestNeighbor1DInterpolation ( ix * ratio_w, image_bitmap + ( iy * image_w ), image_w, 1 ) );
                        }
                        ++ix;
                    }
                }
                ++iy;
            }
        }
        else
        {
            // Both Horizontaly and Vertically Scaled
            float ratio_w = static_cast<float> ( image_w ) / static_cast<float> ( w );
            float ratio_h = static_cast<float> ( image_h ) / static_cast<float> ( h );

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
                            pixels[ ( ( sy * screen_w ) + sx )].Blend ( NearestNeighbor2DInterpolation ( ix * ratio_w, iy * ratio_h, image_w, image_h, image_bitmap ) );
                        }
                        ++ix;
                    }
                }
                ++iy;
            }
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
