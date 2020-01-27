/*
 *  Copyright 2019-2020 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// Defines Diligent::IEngineFactory interface

#include "../../../Primitives/interface/Object.h"
#include "APIInfo.h"

DILIGENT_BEGIN_NAMESPACE(Diligent)

class IShaderSourceInputStreamFactory;

// {D932B052-4ED6-4729-A532-F31DEEC100F3}
static const struct INTERFACE_ID IID_EngineFactory =
    {0xd932b052, 0x4ed6, 0x4729, {0xa5, 0x32, 0xf3, 0x1d, 0xee, 0xc1, 0x0, 0xf3}};

#if DILIGENT_CPP_INTERFACE

/// Engine factory base interface
class IEngineFactory : public IObject
{
public:
    /// Returns API info structure
    virtual const APIInfo& GetAPIInfo() const = 0;

    /// Creates default shader source input stream factory
    /// \param [in]  SearchDirectories           - Semicolon-seprated list of search directories.
    /// \param [out] ppShaderSourceStreamFactory - Memory address where pointer to the shader source stream factory will be written.
    virtual void CreateDefaultShaderSourceStreamFactory(const Char*                       SearchDirectories,
                                                        IShaderSourceInputStreamFactory** ppShaderSourceFactory) const = 0;

#    if PLATFORM_ANDROID
    /// On Android platform, it is necessary to initialize the file system before
    /// CreateDefaultShaderSourceStreamFactory() method can be called.
    /// \param [in] Activity          - Pointer to the activity.
    /// \param [in] ActivityClassName - Activity class name.
    virtual void InitAndroidFileSystem(void* Activity, const char* ActivityClassName) const = 0;
#    endif
};

#else

struct IEngineFactory;
struct IShaderSourceInputStreamFactory;

// clang-format off

struct IEngineFactoryMethods
{
    const struct APIInfo* (*GetAPIInfo)                            (struct IEngineFactory*);
    void                  (*CreateDefaultShaderSourceStreamFactory)(struct IEngineFactory*,
                                                                    const Char*                              SearchDirectories,
                                                                    struct IShaderSourceInputStreamFactory** ppShaderSourceFactory);

#    if PLATFORM_ANDROID
    void (*InitAndroidFileSystem)                                  (struct IEngineFactory*, void* Activity, const char* ActivityClassName);
#    endif
};

// clang-format on

struct IEngineFactoryVtbl
{
    struct IObjectMethods        Object;
    struct IEngineFactoryMethods EngineFactory;
};

struct IEngineFactory
{
    struct IEngineFactoryVtbl* pVtbl;
};

// clang-format off

#    define IEngineFactory_GetAPIInfo(This)                                  (This)->pVtbl->EngineFactory.GetAPIInfo                            ((struct IEngineFactory*)(This))
#    define IEngineFactory_CreateDefaultShaderSourceStreamFactory(This, ...) (This)->pVtbl->EngineFactory.CreateDefaultShaderSourceStreamFactory((struct IEngineFactory*)(This), __VA_ARGS__)
#    define IEngineFactory_InitAndroidFileSystem(This, ...)                  (This)->pVtbl->EngineFactory.InitAndroidFileSystem                 ((struct IEngineFactory*)(This), __VA_ARGS__)

// clang-format on

#endif

DILIGENT_END_NAMESPACE // namespace Diligent
