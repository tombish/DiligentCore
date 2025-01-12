/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#ifndef NOMINMAX
#    define NOMINMAX
#endif
#include <Windows.h>
#include <d3d12.h>

#ifndef ENGINE_DLL
#    define ENGINE_DLL 1
#endif

#include "DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"

void TestEngineFactoryD3D12CInterface()
{
    GetEngineFactoryD3D12Type    GetEngineFactoryD3D12 = Diligent_LoadGraphicsEngineD3D12();
    struct IEngineFactoryD3D12*  pFactory              = GetEngineFactoryD3D12();
    struct EngineD3D12CreateInfo EngineCI              = {0};
    IRenderDevice*               pDevice               = NULL;
    IDeviceContext*              pCtx                  = NULL;
    IEngineFactoryD3D12_CreateDeviceAndContextsD3D12(pFactory, &EngineCI, &pDevice, &pCtx);

    bool res = IEngineFactoryD3D12_LoadD3D12(pFactory, "d3d12.dll");
    (void)res;

    struct SwapChainDesc      SCDesc           = {0};
    struct FullScreenModeDesc FSDes            = {0};
    void*                     pNativeWndHandle = NULL;
    ISwapChain*               pSwapChain       = NULL;
    IEngineFactoryD3D12_CreateSwapChainD3D12(pFactory, pDevice, pCtx, &SCDesc, &FSDes, pNativeWndHandle, &pSwapChain);

    void*                      pd3d12NativeDevice = NULL;
    struct ICommandQueueD3D12* pd3d12CmdQueues    = NULL;
    IEngineFactoryD3D12_AttachToD3D12Device(pFactory, pd3d12NativeDevice, (size_t)1, &pd3d12CmdQueues, &EngineCI, &pDevice, &pCtx);

    Uint32                     NumDisplayModes = 0;
    struct DisplayModeAttribs* DisplayModes    = NULL;
    struct Version             FeatureLevel    = {11, 0};
    IEngineFactoryD3D12_EnumerateDisplayModes(pFactory, FeatureLevel, 0, 0, TEX_FORMAT_RGBA8_UNORM, &NumDisplayModes, DisplayModes);
}
