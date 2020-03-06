/*
Copyright (C) 2019,2020 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <iostream>
#include "SVGPathElement.h"
#include "aeongui/Canvas.h"

namespace AeonGUI
{
    namespace Elements
    {
        int ParsePathData ( std::vector<DrawType>& aPath, const char* s );
        SVGPathElement::SVGPathElement ( const AttributeMap& aAttributeMap ) : SVGGeometryElement ( aAttributeMap )
        {
            auto d = GetAttribute ( "d" );
            if ( std::holds_alternative<std::string> ( d ) )
            {
                std::vector<DrawType> path;
                if ( ParsePathData ( path, std::get<std::string> ( d ).c_str() ) )
                {
                    auto id = GetAttribute ( "id" );
                    if ( std::holds_alternative<std::string> ( id ) )
                    {
                        std::cerr << "Path Id: " << std::get<std::string> ( id ) << std::endl;
                    }
                    std::cerr << "Path Data: " << std::get<std::string> ( d ) << std::endl;
                }
                mPath.Construct ( path );
            }
        }

        SVGPathElement::~SVGPathElement() = default;
    }
}
