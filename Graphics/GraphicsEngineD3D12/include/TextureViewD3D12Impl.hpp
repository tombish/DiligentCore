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

#pragma once

/// \file
/// Declaration of Diligent::TextureViewD3D12Impl class

#include "EngineD3D12ImplTraits.hpp"
#include "TextureViewBase.hpp"
#include "DescriptorHeap.hpp"

namespace Diligent
{

/// Texture view object implementation in Direct3D12 backend.
class TextureViewD3D12Impl final : public TextureViewBase<EngineD3D12ImplTraits>
{
public:
    using TTextureViewBase = TextureViewBase<EngineD3D12ImplTraits>;

    TextureViewD3D12Impl(IReferenceCounters*        pRefCounters,
                         RenderDeviceD3D12Impl*     pDevice,
                         const TextureViewDesc&     ViewDesc,
                         ITexture*                  pTexture,
                         DescriptorHeapAllocation&& Descriptor,
                         DescriptorHeapAllocation&& TexArraySRVDescriptor,
                         DescriptorHeapAllocation&& MipLevelUAVDescriptors,
                         bool                       bIsDefaultView);
    ~TextureViewD3D12Impl();

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_TextureViewD3D12, TTextureViewBase)

    /// Implementation of ITextureViewD3D12::GetCPUDescriptorHandle().
    virtual D3D12_CPU_DESCRIPTOR_HANDLE DILIGENT_CALL_TYPE GetCPUDescriptorHandle() override final
    {
        return m_Descriptor.GetCpuHandle();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetMipLevelUAV(Uint32 Mip)
    {
        VERIFY_EXPR((m_Desc.Flags & TEXTURE_VIEW_FLAG_ALLOW_MIP_MAP_GENERATION) != 0 && m_MipGenerationDescriptors != nullptr && Mip < m_Desc.NumMipLevels);
        return m_MipGenerationDescriptors[1].GetCpuHandle(Mip);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetTexArraySRV()
    {
        VERIFY_EXPR((m_Desc.Flags & TEXTURE_VIEW_FLAG_ALLOW_MIP_MAP_GENERATION) != 0 && m_MipGenerationDescriptors != nullptr);
        return m_MipGenerationDescriptors[0].GetCpuHandle();
    }

protected:
    /// D3D12 view descriptor handle
    DescriptorHeapAllocation m_Descriptor;

    // Extra descriptors used for mipmap generation
    // [0] == texture array SRV used for mipmap generation
    // [1] == mip level UAVs used for mipmap generation
    DescriptorHeapAllocation* m_MipGenerationDescriptors = nullptr;
};

} // namespace Diligent
