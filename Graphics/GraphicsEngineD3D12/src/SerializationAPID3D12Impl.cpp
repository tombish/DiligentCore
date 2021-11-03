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
#include "RenderDeviceD3D12Impl.hpp"
#include "SerializationAPID3D12Impl.hpp"
#include "DeviceObjectArchiveD3D12Impl.hpp"

namespace Diligent
{

SerializationAPID3D12Impl::SerializationAPID3D12Impl(IReferenceCounters* pRefCounters) :
    TSerializationAPIBase{pRefCounters}
{
}

void SerializationAPID3D12Impl::CreateDeviceObjectArchive(IArchive*              pSource,
                                                          IDeviceObjectArchive** ppArchive)
{
    DEV_CHECK_ERR(ppArchive != nullptr, "ppArchive must not be null");
    if (!ppArchive)
        return;

    *ppArchive = nullptr;
    try
    {
        auto& RawMemAllocator = GetRawAllocator();
        auto* pArchiveImpl(NEW_RC_OBJ(RawMemAllocator, "Device object archive instance", DeviceObjectArchiveD3D12Impl)(pSource));
        pArchiveImpl->QueryInterface(IID_DeviceObjectArchive, reinterpret_cast<IObject**>(ppArchive));
    }
    catch (...)
    {
        LOG_ERROR("Failed to create the device object archive");
    }
}

void SerializationAPID3D12Impl::UnpackPipelineState(const PipelineStateUnpackInfo& DeArchiveInfo, IPipelineState** ppPSO)
{
    if (!VerifyUnpackPipelineState(DeArchiveInfo, ppPSO))
        return;

    auto* pArchiveD3D12 = ClassPtrCast<DeviceObjectArchiveD3D12Impl>(DeArchiveInfo.pArchive);

    *ppPSO = nullptr;
    switch (DeArchiveInfo.PipelineType)
    {
        case PIPELINE_TYPE_GRAPHICS:
        case PIPELINE_TYPE_MESH:
            pArchiveD3D12->UnpackGraphicsPSO(DeArchiveInfo, *ppPSO);
            break;
        case PIPELINE_TYPE_COMPUTE:
            pArchiveD3D12->UnpackComputePSO(DeArchiveInfo, *ppPSO);
            break;
        case PIPELINE_TYPE_RAY_TRACING:
            pArchiveD3D12->UnpackRayTracingPSO(DeArchiveInfo, *ppPSO);
            break;
        case PIPELINE_TYPE_TILE:
        case PIPELINE_TYPE_INVALID:
        default:
            LOG_ERROR_MESSAGE("Unsupported pipeline type");
            return;
    }
}

void SerializationAPID3D12Impl::UnpackResourceSignature(const ResourceSignatureUnpackInfo& DeArchiveInfo, IPipelineResourceSignature** ppSignature)
{
    if (!VerifyUnpackResourceSignature(DeArchiveInfo, ppSignature))
        return;

    auto* pArchiveD3D12 = ClassPtrCast<DeviceObjectArchiveD3D12Impl>(DeArchiveInfo.pArchive);

    *ppSignature = nullptr;
    pArchiveD3D12->UnpackResourceSignature(DeArchiveInfo, *ppSignature);
}

void SerializationAPID3D12Impl::UnpackRenderPass(const RenderPassUnpackInfo& DeArchiveInfo, IRenderPass** ppRP)
{
    if (!VerifyUnpackRenderPass(DeArchiveInfo, ppRP))
        return;

    auto* pArchiveD3D12 = ClassPtrCast<DeviceObjectArchiveD3D12Impl>(DeArchiveInfo.pArchive);

    *ppRP = nullptr;
    pArchiveD3D12->UnpackRenderPass(DeArchiveInfo, *ppRP);
}

} // namespace Diligent
