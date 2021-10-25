/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#include "pch.h"
#include "RenderDeviceVkImpl.hpp"
#include "DeviceObjectArchiveVkImpl.hpp"

namespace Diligent
{

static constexpr auto DevType = RENDER_DEVICE_TYPE_VULKAN;

DeviceObjectArchiveVkImpl::DeviceObjectArchiveVkImpl(IReferenceCounters* pRefCounters) :
    DeviceObjectArchiveBase{pRefCounters}
{
}

DeviceObjectArchiveVkImpl::~DeviceObjectArchiveVkImpl()
{
}

void DeviceObjectArchiveVkImpl::UnpackGraphicsPSO(const PipelineStateUnpackInfo& DeArchiveInfo, RenderDeviceVkImpl* pRenderDeviceVk, IPipelineState** ppPSO)
{
    VERIFY_EXPR(pRenderDeviceVk != nullptr);
    VERIFY_EXPR(ppPSO != nullptr);

    std::shared_lock<std::shared_mutex> ReadLock{m_Guard};

    PSOData<GraphicsPipelineStateCreateInfo> PSO{GetRawAllocator()};
    if (!ReadGraphicsPSOData(DeArchiveInfo.Name, PSO))
        return;

    const auto& Header = *PSO.pHeader;
    if (Header.DeviceSpecificDataSize[DevType] == 0)
    {
        LOG_ERROR_MESSAGE("Vulkan specific data is not specified for resource signature");
        return;
    }
    if (Header.DeviceSpecificDataOffset[DevType] + Header.DeviceSpecificDataSize[DevType] > m_pSource->GetSize())
    {
        LOG_ERROR_MESSAGE("Invalid offset in archive");
        return;
    }

    const auto DataSize = Header.DeviceSpecificDataSize[DevType];
    auto*      pData    = static_cast<Uint8*>(PSO.Allocator.Allocate(DataSize, DataPtrAlign));
    if (!m_pSource->Read(Header.DeviceSpecificDataOffset[DevType], pData, DataSize))
    {
        LOG_ERROR_MESSAGE("Failed to read resource signature data");
        return;
    }

    const void* Ptr = pData;

    VERIFY_EXPR(Ptr <= pData + DataSize);

    // AZ TODO: spirv parsing is slow and should be avoided

    pRenderDeviceVk->CreateGraphicsPipelineState(PSO.CreateInfo, ppPSO);
}

void DeviceObjectArchiveVkImpl::UnpackComputePSO(const PipelineStateUnpackInfo& DeArchiveInfo, RenderDeviceVkImpl* pRenderDeviceVk, IPipelineState** ppPSO)
{
    VERIFY_EXPR(pRenderDeviceVk != nullptr);
    VERIFY_EXPR(ppPSO != nullptr);

    std::shared_lock<std::shared_mutex> ReadLock{m_Guard};

    PSOData<ComputePipelineStateCreateInfo> PSO{GetRawAllocator()};
    if (!ReadComputePSOData(DeArchiveInfo.Name, PSO))
        return;

    // AZ TODO

    pRenderDeviceVk->CreateComputePipelineState(PSO.CreateInfo, ppPSO);
}

void DeviceObjectArchiveVkImpl::UnpackRayTracingPSO(const PipelineStateUnpackInfo& DeArchiveInfo, RenderDeviceVkImpl* pRenderDeviceVk, IPipelineState** ppPSO)
{
    VERIFY_EXPR(pRenderDeviceVk != nullptr);
    VERIFY_EXPR(ppPSO != nullptr);

    std::shared_lock<std::shared_mutex> ReadLock{m_Guard};

    PSOData<RayTracingPipelineStateCreateInfo> PSO{GetRawAllocator()};
    if (!ReadRayTracingPSOData(DeArchiveInfo.Name, PSO))
        return;

    // AZ TODO

    pRenderDeviceVk->CreateRayTracingPipelineState(PSO.CreateInfo, ppPSO);
}

void DeviceObjectArchiveVkImpl::UnpackResourceSignature(const ResourceSignatureUnpackInfo& DeArchiveInfo, RenderDeviceVkImpl* pRenderDeviceVk, IPipelineResourceSignature** ppSignature)
{
    VERIFY_EXPR(pRenderDeviceVk != nullptr);
    VERIFY_EXPR(ppSignature != nullptr);

    std::shared_lock<std::shared_mutex> ReadLock{m_Guard};

    PRSData PRS{GetRawAllocator()};
    if (!ReadPRSData(DeArchiveInfo.Name, PRS))
        return;

    const auto& Header = *PRS.pHeader;
    if (Header.DeviceSpecificDataSize[DevType] == 0)
    {
        LOG_ERROR_MESSAGE("Vulkan specific data is not specified for resource signature");
        return;
    }
    if (Header.DeviceSpecificDataOffset[DevType] + Header.DeviceSpecificDataSize[DevType] > m_pSource->GetSize())
    {
        LOG_ERROR_MESSAGE("Invalid offset in archive");
        return;
    }

    const auto DataSize = Header.DeviceSpecificDataSize[DevType];
    auto*      pData    = PRS.Allocator.Allocate(DataSize, DataPtrAlign);
    if (!m_pSource->Read(Header.DeviceSpecificDataOffset[DevType], pData, DataSize))
    {
        LOG_ERROR_MESSAGE("Failed to read resource signature data");
        return;
    }

    Serializer<SerializerMode::Read> Ser{pData, DataSize};

    PipelineResourceSignatureVkImpl::SerializedData SerializedData;
    SerializedData.Base = PRS.Serialized;
    SerializerVkImpl<SerializerMode::Read>::SerializePRS(Ser, SerializedData, &PRS.Allocator);
    VERIFY_EXPR(Ser.IsEnd());

    pRenderDeviceVk->CreatePipelineResourceSignature(PRS.Desc, SerializedData, ppSignature);
}

template <SerializerMode Mode>
void DeviceObjectArchiveVkImpl::SerializerVkImpl<Mode>::SerializePRS(
    Serializer<Mode>&                                       Ser,
    TQual<PipelineResourceSignatureVkImpl::SerializedData>& Serialized,
    DynamicLinearAllocator*                                 Allocator)
{
    Ser(Serialized.NumResources,
        Serialized.DynamicUniformBufferCount,
        Serialized.DynamicStorageBufferCount);

    auto* pAttribs = ArraySerializerHelper<Mode>::Create(Serialized.pResourceAttribs, Serialized.NumResources, Allocator);
    for (Uint32 i = 0; i < Serialized.NumResources; ++i)
    {
        Ser(pAttribs[i]);
    }

#if defined(_MSC_VER) && defined(_WIN64)
    static_assert(sizeof(Serialized) == 32, "Did you add a new member to PipelineResourceSignatureVkImpl::SerializedData? Please add serialization here.");
#endif
}

template struct DeviceObjectArchiveVkImpl::SerializerVkImpl<SerializerMode::Read>;
template struct DeviceObjectArchiveVkImpl::SerializerVkImpl<SerializerMode::Write>;
template struct DeviceObjectArchiveVkImpl::SerializerVkImpl<SerializerMode::Measure>;

} // namespace Diligent
