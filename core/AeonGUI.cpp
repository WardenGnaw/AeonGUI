/*
Copyright (C) 2010-2012,2020 Rodrigo Jose Hernandez Cordoba

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

#include <memory>
#include "aeongui/AeonGUI.h"
#include "libplatform/libplatform.h"
#include "v8.h"

namespace AeonGUI
{
    static std::unique_ptr<v8::Platform> gPlatform{};
    bool Initialize ( int argc = 0, char *argv[] = nullptr )
    {
        /** @todo Provide a plugin loading mechanism,
         *  so different JavaScript engines can be used */
        // Initialize V8.
        v8::V8::InitializeICU();
        v8::V8::InitializeExternalStartupData ( argv[0] );
        gPlatform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform ( gPlatform.get() );
        v8::V8::Initialize();
        return true;
    }
    void Finalize()
    {
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        gPlatform.reset();
    }
}