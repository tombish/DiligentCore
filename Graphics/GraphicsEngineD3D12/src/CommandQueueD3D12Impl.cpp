/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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
#include "CommandQueueD3D12Impl.h"

namespace Diligent
{

CommandQueueD3D12Impl::CommandQueueD3D12Impl(IReferenceCounters*    pRefCounters,
                                             ID3D12CommandQueue*    pd3d12NativeCmdQueue,
                                             ID3D12Fence*           pd3d12Fence) :
        TBase(pRefCounters),
        m_pd3d12CmdQueue(pd3d12NativeCmdQueue), 
        m_d3d12Fence(pd3d12Fence),
        m_NextFenceValue(1),
        m_WaitForGPUEventHandle( CreateEvent(nullptr, false, false, nullptr) )
{
    VERIFY_EXPR(m_WaitForGPUEventHandle != INVALID_HANDLE_VALUE);
    m_d3d12Fence->Signal(0);
}

CommandQueueD3D12Impl::~CommandQueueD3D12Impl()
{
    CloseHandle(m_WaitForGPUEventHandle);
}

IMPLEMENT_QUERY_INTERFACE( CommandQueueD3D12Impl, IID_CommandQueueD3D12, TBase )

UINT64 CommandQueueD3D12Impl::ExecuteCommandList(ID3D12GraphicsCommandList* commandList)
{
    ID3D12CommandList *const ppCmdLists[] = {commandList};
	m_pd3d12CmdQueue->ExecuteCommandLists(1, ppCmdLists);
    auto FenceValue = m_NextFenceValue;
    // Signal the fence
    m_pd3d12CmdQueue->Signal(m_d3d12Fence, FenceValue);
    // Increment the value
    Atomics::AtomicIncrement(m_NextFenceValue);
    return FenceValue;
}

void CommandQueueD3D12Impl::IdleGPU()
{
    Uint64 LastSignaledFenceValue = m_NextFenceValue;
    m_pd3d12CmdQueue->Signal(m_d3d12Fence, LastSignaledFenceValue);
    Atomics::AtomicIncrement(m_NextFenceValue);
    if (GetCompletedFenceValue() < LastSignaledFenceValue)
    {
        m_d3d12Fence->SetEventOnCompletion(LastSignaledFenceValue, m_WaitForGPUEventHandle);
        WaitForSingleObject(m_WaitForGPUEventHandle, INFINITE);
        VERIFY(GetCompletedFenceValue() == LastSignaledFenceValue, "Unexpected signaled fence value");
    }
}

Uint64 CommandQueueD3D12Impl::GetCompletedFenceValue()
{
    auto CompletedFenceValue = m_d3d12Fence->GetCompletedValue();
    if(CompletedFenceValue > m_LastCompletedFenceValue)
        m_LastCompletedFenceValue = CompletedFenceValue;
    return m_LastCompletedFenceValue;
}

void CommandQueueD3D12Impl::SignalFence(ID3D12Fence* pFence, Uint64 Value)
{
    m_pd3d12CmdQueue->Signal(pFence, Value);
}

}
