#ifndef AEONGUI_COLOR_H
#define AEONGUI_COLOR_H
/******************************************************************************
Copyright 2010-2012 Rodrigo Hernandez Cordoba

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
#include "Integer.h"
namespace AeonGUI
{
    union Color
    {
        Color() {}
        Color ( uint32_t value ) : bgra ( value ) {}
        Color ( uint8_t A, uint8_t R, uint8_t G, uint8_t B )
            : b ( B ), g ( G ), r ( R ), a ( A ) {}
        uint32_t bgra;
        struct
        {
#if 0
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
#else
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
#endif
        };
        void Blend ( Color src );
    };
}
#endif