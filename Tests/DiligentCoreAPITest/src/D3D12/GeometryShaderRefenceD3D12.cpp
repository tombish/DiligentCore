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

#include "D3D12/TestingEnvironmentD3D12.hpp"
#include "D3D12/TestingSwapChainD3D12.hpp"
#include "../include/DXGITypeConversions.hpp"
#include "../include/d3dx12_win.h"

#include "DeviceContextD3D12.h"

#include "InlineShaders/GeometryShaderTestHLSL.h"

namespace Diligent
{

namespace Testing
{

void GeometryShaderReferenceD3D12(ISwapChain* pSwapChain)
{
    auto* pEnv                   = TestingEnvironmentD3D12::GetInstance();
    auto* pd3d12Device           = pEnv->GetD3D12Device();
    auto* pTestingSwapChainD3D12 = ClassPtrCast<TestingSwapChainD3D12>(pSwapChain);

    const auto& SCDesc = pSwapChain->GetDesc();

    CComPtr<ID3DBlob> pVSByteCode, pGSByteCode, pPSByteCode;

    auto hr = CompileD3DShader(HLSL::GSTest_VS, "main", nullptr, "vs_5_0", &pVSByteCode);
    ASSERT_HRESULT_SUCCEEDED(hr) << "Failed to compile vertex shader";

    hr = CompileD3DShader(HLSL::GSTest_GS, "main", nullptr, "gs_5_0", &pGSByteCode);
    ASSERT_HRESULT_SUCCEEDED(hr) << "Failed to compile pixel shader";

    hr = CompileD3DShader(HLSL::GSTest_PS, "main", nullptr, "ps_5_0", &pPSByteCode);
    ASSERT_HRESULT_SUCCEEDED(hr) << "Failed to compile pixel shader";


    D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {};

    RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    CComPtr<ID3DBlob>            signature;
    CComPtr<ID3D12RootSignature> pd3d12RootSignature;
    D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
    pd3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), __uuidof(pd3d12RootSignature), reinterpret_cast<void**>(static_cast<ID3D12RootSignature**>(&pd3d12RootSignature)));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};

    PSODesc.pRootSignature     = pd3d12RootSignature;
    PSODesc.VS.pShaderBytecode = pVSByteCode->GetBufferPointer();
    PSODesc.VS.BytecodeLength  = pVSByteCode->GetBufferSize();
    PSODesc.GS.pShaderBytecode = pGSByteCode->GetBufferPointer();
    PSODesc.GS.BytecodeLength  = pGSByteCode->GetBufferSize();
    PSODesc.PS.pShaderBytecode = pPSByteCode->GetBufferPointer();
    PSODesc.PS.BytecodeLength  = pPSByteCode->GetBufferSize();

    PSODesc.BlendState        = CD3DX12_BLEND_DESC{D3D12_DEFAULT};
    PSODesc.RasterizerState   = CD3DX12_RASTERIZER_DESC{D3D12_DEFAULT};
    PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC{D3D12_DEFAULT};

    PSODesc.RasterizerState.CullMode         = D3D12_CULL_MODE_NONE;
    PSODesc.DepthStencilState.DepthEnable    = FALSE;
    PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    PSODesc.SampleMask = 0xFFFFFFFF;

    PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    PSODesc.NumRenderTargets      = 1;
    PSODesc.RTVFormats[0]         = TexFormatToDXGI_Format(SCDesc.ColorBufferFormat);
    PSODesc.SampleDesc.Count      = 1;
    PSODesc.SampleDesc.Quality    = 0;
    PSODesc.NodeMask              = 0;
    PSODesc.Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE;

    CComPtr<ID3D12PipelineState> pd3d12PSO;
    hr = pd3d12Device->CreateGraphicsPipelineState(&PSODesc, __uuidof(pd3d12PSO), reinterpret_cast<void**>(static_cast<ID3D12PipelineState**>(&pd3d12PSO)));
    ASSERT_HRESULT_SUCCEEDED(hr) << "Failed to create pipeline state";

    auto pCmdList = pEnv->CreateGraphicsCommandList();
    pTestingSwapChainD3D12->TransitionRenderTarget(pCmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    D3D12_VIEWPORT d3d12VP = {};
    d3d12VP.Width          = static_cast<float>(SCDesc.Width);
    d3d12VP.Height         = static_cast<float>(SCDesc.Height);
    d3d12VP.MaxDepth       = 1;
    pCmdList->RSSetViewports(1, &d3d12VP);
    D3D12_RECT Rect = {0, 0, static_cast<LONG>(SCDesc.Width), static_cast<LONG>(SCDesc.Height)};
    pCmdList->RSSetScissorRects(1, &Rect);

    auto RTVDescriptorHandle = pTestingSwapChainD3D12->GetRTVDescriptorHandle();

    pCmdList->OMSetRenderTargets(1, &RTVDescriptorHandle, FALSE, nullptr);

    float ClearColor[] = {0, 0, 0, 0};
    pCmdList->ClearRenderTargetView(RTVDescriptorHandle, ClearColor, 0, nullptr);

    pCmdList->SetPipelineState(pd3d12PSO);
    pCmdList->SetGraphicsRootSignature(pd3d12RootSignature);
    pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    pCmdList->DrawInstanced(2, 1, 0, 0);

    pCmdList->Close();
    pEnv->ExecuteCommandList(pCmdList);
}

} // namespace Testing

} // namespace Diligent
