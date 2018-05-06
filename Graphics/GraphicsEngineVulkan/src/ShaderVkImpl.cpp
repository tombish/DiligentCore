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

#include "ShaderVkImpl.h"
#include "RenderDeviceVkImpl.h"
#include "DataBlobImpl.h"
#include "GLSLSourceBuilder.h"
#include "GLSL2SPIRV.h"

using namespace Diligent;

namespace Diligent
{


ShaderVkImpl::ShaderVkImpl(IReferenceCounters *pRefCounters, RenderDeviceVkImpl *pRenderDeviceVk, const ShaderCreationAttribs &CreationAttribs) : 
    TShaderBase(pRefCounters, pRenderDeviceVk, CreationAttribs.Desc),
    m_StaticResLayout(*this, GetRawAllocator()),
    m_DummyShaderVar(*this),
    m_ConstResCache(ShaderResourceCacheVk::DbgCacheContentType::StaticShaderResources)
{
    auto GLSLSource = BuildGLSLSourceString(CreationAttribs, TargetGLSLCompiler::glslang);
    m_SPIRV = GLSLtoSPIRV(m_Desc.ShaderType, GLSLSource.c_str());
    if(m_SPIRV.empty())
    {
        LOG_ERROR_AND_THROW("Failed to compile shader");
    }

    // We cannot create shader module here because resource bindings are assigned when
    // pipeline state is created

    // Load shader resources
    auto &Allocator = GetRawAllocator();
    auto *pRawMem = ALLOCATE(Allocator, "Allocator for ShaderResources", sizeof(SPIRVShaderResources));
    auto *pResources = new (pRawMem) SPIRVShaderResources(Allocator, m_Desc.ShaderType, m_SPIRV, m_Desc.DefaultVariableType, m_Desc.VariableDesc, m_Desc.NumVariables, m_Desc.Name);
    m_pShaderResources.reset(pResources, STDDeleterRawMem<SPIRVShaderResources>(Allocator));

    // Clone only static resources that will be set directly in the shader
    // http://diligentgraphics.com/diligent-engine/architecture/Vk/shader-resource-layout#Initializing-Special-Resource-Layout-for-Managing-Static-Shader-Resources
    SHADER_VARIABLE_TYPE VarTypes[] = {SHADER_VARIABLE_TYPE_STATIC};
    m_StaticResLayout.Initialize(pRenderDeviceVk->GetLogicalDevice(), m_pShaderResources, GetRawAllocator(), VarTypes, _countof(VarTypes), &m_ConstResCache, nullptr, nullptr);
}

ShaderVkImpl::~ShaderVkImpl()
{
}

void ShaderVkImpl::BindResources(IResourceMapping* pResourceMapping, Uint32 Flags)
{
   m_StaticResLayout.BindResources(pResourceMapping, Flags, &m_ConstResCache);
}
    
IShaderVariable* ShaderVkImpl::GetShaderVariable(const Char* Name)
{
    auto *pVar = m_StaticResLayout.GetShaderVariable(Name);
    if(pVar == nullptr)
        pVar = &m_DummyShaderVar;
    return pVar;
}

#ifdef VERIFY_SHADER_BINDINGS
void ShaderVkImpl::DbgVerifyStaticResourceBindings()
{
    m_StaticResLayout.dbgVerifyBindings();
}
#endif

}
