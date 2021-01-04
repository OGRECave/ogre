/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreVulkanProgram.h"

#include "OgreLogManager.h"
#include "OgreProfiler.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanMappings.h"

#include "OgreStringConverter.h"
#include "OgreVulkanUtils.h"

#include "OgreRenderSystemCapabilities.h"
#include "OgreRoot.h"

#define OgreProfileExhaustive OgreProfile

namespace Ogre
{
    //-----------------------------------------------------------------------
    VulkanProgram::VulkanProgram( ResourceManager *creator, const String &name, ResourceHandle handle,
                                  const String &group, bool isManual, ManualResourceLoader *loader,
                                  VulkanDevice *device ) :
        GpuProgram( creator, name, handle, group, isManual, loader ),
        mDevice( device ),
        mShaderModule( VK_NULL_HANDLE )
    {
        if( createParamDictionary( "VulkanProgram" ) )
        {
            setupBaseParamDictionary();
        }

        mDrawIdLocation = /*( mShaderSyntax == GLSL ) */ true ? 15 : 0;
    }
    //---------------------------------------------------------------------------
    VulkanProgram::~VulkanProgram()
    {
        // Have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //-----------------------------------------------------------------------
    void VulkanProgram::loadFromSource()
    {
        mCompileError = false;

        VkShaderModuleCreateInfo moduleCi = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        moduleCi.codeSize = mSource.size();
        moduleCi.pCode = (const uint32*)mSource.data();
        OGRE_VK_CHECK(vkCreateShaderModule(mDevice->mDevice, &moduleCi, 0, &mShaderModule));

        setObjectName(mDevice->mDevice, (uint64_t)mShaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
                      mName.c_str());
    }
    //---------------------------------------------------------------------------
    void VulkanProgram::unloadImpl()
    {
        vkDestroyShaderModule( mDevice->mDevice, mShaderModule, 0 );
        mShaderModule = VK_NULL_HANDLE;
    }
    //-----------------------------------------------------------------------
    static VkShaderStageFlagBits get( GpuProgramType programType )
    {
        switch( programType )
        {
        // clang-format off
        case GPT_VERTEX_PROGRAM:    return VK_SHADER_STAGE_VERTEX_BIT;
        case GPT_FRAGMENT_PROGRAM:  return VK_SHADER_STAGE_FRAGMENT_BIT;
        case GPT_GEOMETRY_PROGRAM:  return VK_SHADER_STAGE_GEOMETRY_BIT;
        case GPT_HULL_PROGRAM:      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case GPT_DOMAIN_PROGRAM:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case GPT_COMPUTE_PROGRAM:   return VK_SHADER_STAGE_COMPUTE_BIT;
            // clang-format on
        }
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    VkPipelineShaderStageCreateInfo VulkanProgram::getPipelineShaderStageCi() const
    {
        VkPipelineShaderStageCreateInfo ret = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        ret.stage = get( mType );
        ret.module = mShaderModule;
        ret.pName = "main";
        return ret;
    }
    //-----------------------------------------------------------------------
    const String &VulkanProgram::getLanguage( void ) const
    {
        static const String language = "spirv";
        return language;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr VulkanProgram::createParameters( void )
    {
        GpuProgramParametersSharedPtr params = GpuProgram::createParameters();
        params->setTransposeMatrices( true );
        return params;
    }
    //-----------------------------------------------------------------------
    VulkanProgramFactory::VulkanProgramFactory( VulkanDevice *device ) :
        mDevice( device )
    {
    }
    //-----------------------------------------------------------------------
    VulkanProgramFactory::~VulkanProgramFactory( void )
    {
    }
    //-----------------------------------------------------------------------
    const String& VulkanProgramFactory::getLanguage(void) const
    {
        static String language = "spirv";
        return language;
    }
    //-----------------------------------------------------------------------
    GpuProgram *VulkanProgramFactory::create( ResourceManager *creator, const String &name,
                                                       ResourceHandle handle, const String &group,
                                                       bool isManual, ManualResourceLoader *loader )
    {
        return OGRE_NEW VulkanProgram(creator, name, handle, group, isManual, loader, mDevice);
    }
}  // namespace Ogre
