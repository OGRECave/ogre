/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"

#include "OgreRenderSystemCapabilities.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    RenderSystemCapabilities::RenderSystemCapabilities()
        : mVendor(GPU_UNKNOWN)
        , mNumTextureUnits(0)
        , mStencilBufferBitDepth(0)
        , mNumVertexBlendMatrices(0)
        , mNumMultiRenderTargets(1)
        , mNonPOW2TexturesLimited(false)
        , mMaxSupportedAnisotropy(0)
        , mVertexTextureUnitsShared(0)
        , mGeometryProgramNumOutputVertices(0)
        , mNumVertexAttributes(1)
    {
        for(int i = 0; i < CAPS_CATEGORY_COUNT; i++)
        {
            mCapabilities[i] = 0;
        }
        mCategoryRelevant[CAPS_CATEGORY_COMMON] = true;
        mCategoryRelevant[CAPS_CATEGORY_COMMON_2] = true;
        // each rendersystem should enable these
        mCategoryRelevant[CAPS_CATEGORY_D3D9] = false;
        mCategoryRelevant[CAPS_CATEGORY_GL] = false;
    }

    //-----------------------------------------------------------------------
    void RenderSystemCapabilities::log(Log* pLog) const
    {
        pLog->logMessage("RenderSystem capabilities");
        pLog->logMessage("-------------------------");
        pLog->logMessage("RenderSystem Name: " + getRenderSystemName());
        pLog->logMessage("GPU Vendor: " + vendorToString(getVendor()));
        pLog->logMessage("Device Name: " + getDeviceName());
        pLog->logMessage("Driver Version: " + getDriverVersion().toString());
        pLog->logMessage(" * Fixed function pipeline: " +
                         StringConverter::toString(hasCapability(RSC_FIXED_FUNCTION), true));
        pLog->logMessage(" * 32-bit index buffers: " + StringConverter::toString(hasCapability(RSC_32BIT_INDEX), true));
        pLog->logMessage(" * Hardware stencil buffer: " +
                         StringConverter::toString(hasCapability(RSC_HWSTENCIL), true));
        if (hasCapability(RSC_HWSTENCIL))
        {
            pLog->logMessage("   - Stencil depth: " + StringConverter::toString(getStencilBufferBitDepth()));
            pLog->logMessage("   - Two sided stencil support: " +
                             StringConverter::toString(hasCapability(RSC_TWO_SIDED_STENCIL), true));
            pLog->logMessage("   - Wrap stencil values: " +
                             StringConverter::toString(hasCapability(RSC_STENCIL_WRAP), true));
        }
        pLog->logMessage(" * Vertex programs: yes");
        pLog->logMessage("   - Number of floating-point constants: " +
                         StringConverter::toString(mVertexProgramConstantFloatCount));
        pLog->logMessage("   - Number of integer constants: " +
                         StringConverter::toString(mVertexProgramConstantIntCount));
        pLog->logMessage("   - Number of boolean constants: " +
                         StringConverter::toString(mVertexProgramConstantBoolCount));
        pLog->logMessage(" * Fragment programs: yes");
        pLog->logMessage("   - Number of floating-point constants: " +
                         StringConverter::toString(mFragmentProgramConstantFloatCount));
        pLog->logMessage("   - Number of integer constants: " +
                         StringConverter::toString(mFragmentProgramConstantIntCount));
        pLog->logMessage("   - Number of boolean constants: " +
                         StringConverter::toString(mFragmentProgramConstantBoolCount));
        pLog->logMessage(" * Geometry programs: " +
                         StringConverter::toString(hasCapability(RSC_GEOMETRY_PROGRAM), true));
        if (hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            pLog->logMessage("   - Number of floating-point constants: " +
                             StringConverter::toString(mGeometryProgramConstantFloatCount));
            pLog->logMessage("   - Number of integer constants: " +
                             StringConverter::toString(mGeometryProgramConstantIntCount));
            pLog->logMessage("   - Number of boolean constants: " +
                             StringConverter::toString(mGeometryProgramConstantBoolCount));
        }
        pLog->logMessage(" * Tessellation Hull programs: " +
                         StringConverter::toString(hasCapability(RSC_TESSELLATION_HULL_PROGRAM), true));
        if (hasCapability(RSC_TESSELLATION_HULL_PROGRAM))
        {
            pLog->logMessage("   - Number of floating-point constants: " +
                             StringConverter::toString(mTessellationHullProgramConstantFloatCount));
            pLog->logMessage("   - Number of integer constants: " +
                             StringConverter::toString(mTessellationHullProgramConstantIntCount));
            pLog->logMessage("   - Number of boolean constants: " +
                             StringConverter::toString(mTessellationHullProgramConstantBoolCount));
        }
        pLog->logMessage(" * Tessellation Domain programs: " +
                         StringConverter::toString(hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM), true));
        if (hasCapability(RSC_TESSELLATION_DOMAIN_PROGRAM))
        {
            pLog->logMessage("   - Number of floating-point constants: " +
                             StringConverter::toString(mTessellationDomainProgramConstantFloatCount));
            pLog->logMessage("   - Number of integer constants: " +
                             StringConverter::toString(mTessellationDomainProgramConstantIntCount));
            pLog->logMessage("   - Number of boolean constants: " +
                             StringConverter::toString(mTessellationDomainProgramConstantBoolCount));
        }
        pLog->logMessage(" * Compute programs: " + StringConverter::toString(hasCapability(RSC_COMPUTE_PROGRAM), true));
        if (hasCapability(RSC_COMPUTE_PROGRAM))
        {
            pLog->logMessage("   - Number of floating-point constants: " +
                             StringConverter::toString(mComputeProgramConstantFloatCount));
            pLog->logMessage("   - Number of integer constants: " +
                             StringConverter::toString(mComputeProgramConstantIntCount));
            pLog->logMessage("   - Number of boolean constants: " +
                             StringConverter::toString(mComputeProgramConstantBoolCount));
        }
        pLog->logMessage(
            " * Supported Shader Profiles: " +
            StringConverter::toString(StringVector(mSupportedShaderProfiles.begin(), mSupportedShaderProfiles.end())));
        pLog->logMessage(" * Read-back compiled shader: " +
                         StringConverter::toString(hasCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER), true));
        pLog->logMessage(" * Number of vertex attributes: " + StringConverter::toString(mNumVertexAttributes));
        pLog->logMessage(" * Textures");
        pLog->logMessage("   - Number of texture units: " + StringConverter::toString(mNumTextureUnits));
        pLog->logMessage("   - Floating point: " + StringConverter::toString(hasCapability(RSC_TEXTURE_FLOAT), true));
        pLog->logMessage(
            "   - Non-power-of-two: " + StringConverter::toString(hasCapability(RSC_NON_POWER_OF_2_TEXTURES), true) +
            (mNonPOW2TexturesLimited ? " (limited)" : ""));
        pLog->logMessage("   - 1D textures: " + StringConverter::toString(hasCapability(RSC_TEXTURE_1D), true));
        pLog->logMessage("   - 3D textures: " + StringConverter::toString(hasCapability(RSC_TEXTURE_3D), true));
        pLog->logMessage("   - Anisotropic filtering: " + StringConverter::toString(hasCapability(RSC_ANISOTROPY), true));

        pLog->logMessage(
            " * Texture Compression: "
            + StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION), true));
        if (hasCapability(RSC_TEXTURE_COMPRESSION))
        {
            pLog->logMessage("   - DXT: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_DXT), true));
            pLog->logMessage("   - VTC: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_VTC), true));
            pLog->logMessage("   - PVRTC: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_PVRTC), true));
            pLog->logMessage("   - ATC: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_ATC), true));
            pLog->logMessage("   - ETC1: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_ETC1), true));
            pLog->logMessage("   - ETC2: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_ETC2), true));
            pLog->logMessage("   - BC4/BC5: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5), true));
            pLog->logMessage("   - BC6H/BC7: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7), true));
            pLog->logMessage("   - ASTC: " +
                             StringConverter::toString(hasCapability(RSC_TEXTURE_COMPRESSION_ASTC), true));
            pLog->logMessage("   - Mipmaps for compressed formats: " +
                             StringConverter::toString(hasCapability(RSC_AUTOMIPMAP_COMPRESSED), true));
        }

        pLog->logMessage(" * Vertex Buffers");
        pLog->logMessage("   - VET_UBYTE4 element type: " +
                         StringConverter::toString(hasCapability(RSC_VERTEX_FORMAT_UBYTE4), true));
        pLog->logMessage("   - Render to Vertex Buffer: " +
                         StringConverter::toString(hasCapability(RSC_HWRENDER_TO_VERTEX_BUFFER), true));
        pLog->logMessage("   - Instance Data: " +
                         StringConverter::toString(hasCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA), true));
        pLog->logMessage("   - Primitive Restart: " +
                         StringConverter::toString(hasCapability(RSC_PRIMITIVE_RESTART), true));
        pLog->logMessage(" * Vertex texture fetch: " +
                         StringConverter::toString(hasCapability(RSC_VERTEX_TEXTURE_FETCH), true));
        if (hasCapability(RSC_VERTEX_TEXTURE_FETCH))
        {
            pLog->logMessage("   - Max vertex textures: " + StringConverter::toString(mNumVertexTextureUnits));
            pLog->logMessage("   - Vertex textures shared: " +
                             StringConverter::toString(mVertexTextureUnitsShared, true));
        }
        pLog->logMessage(" * Read/Write Buffers: " +
                         StringConverter::toString(hasCapability(RSC_READ_WRITE_BUFFERS), true));
        pLog->logMessage(
            " * Hardware Occlusion Query: "
            + StringConverter::toString(hasCapability(RSC_HWOCCLUSION), true));
        pLog->logMessage(
            " * User clip planes: "
            + StringConverter::toString(hasCapability(RSC_USER_CLIP_PLANES), true));
        pLog->logMessage(
            " * Infinite far plane projection: "
            + StringConverter::toString(hasCapability(RSC_INFINITE_FAR_PLANE), true));
        pLog->logMessage(
            " * Hardware render-to-texture: "
            + StringConverter::toString(hasCapability(RSC_HWRENDER_TO_TEXTURE), true));
        pLog->logMessage("   - Multiple Render Targets: " + StringConverter::toString(mNumMultiRenderTargets));
        pLog->logMessage("   - With different bit depths: " +
                         StringConverter::toString(hasCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS), true));
        pLog->logMessage(" * Point Sprites: " + StringConverter::toString(hasCapability(RSC_POINT_SPRITES), true));
        if (hasCapability(RSC_POINT_SPRITES))
        {
            pLog->logMessage("   - Extended parameters: " +
                             StringConverter::toString(hasCapability(RSC_POINT_EXTENDED_PARAMETERS), true));
            pLog->logMessage("   - Max Size: " + StringConverter::toString(mMaxPointSize));
        }
        pLog->logMessage(
            " * Wide Lines: "
            + StringConverter::toString(hasCapability(RSC_WIDE_LINES), true));
        pLog->logMessage(
            " * Hardware Gamma: "
            + StringConverter::toString(hasCapability(RSC_HW_GAMMA), true));
        if (mCategoryRelevant[CAPS_CATEGORY_GL])
        {
            pLog->logMessage(
                " * PBuffer support: "
                + StringConverter::toString(hasCapability(RSC_PBUFFER), true));
            pLog->logMessage(
                " * Vertex Array Objects: "
                + StringConverter::toString(hasCapability(RSC_VAO), true));
            pLog->logMessage(" * Separate shader objects: " +
                             StringConverter::toString(hasCapability(RSC_SEPARATE_SHADER_OBJECTS), true));
            pLog->logMessage("   - redeclare GLSL interface block: " +
                             StringConverter::toString(hasCapability(RSC_GLSL_SSO_REDECLARE), true));
            pLog->logMessage(
                " * Debugging/ profiling events: "
                + StringConverter::toString(hasCapability(RSC_DEBUG), true));
            pLog->logMessage(
                " * Map buffer storage: "
                + StringConverter::toString(hasCapability(RSC_MAPBUFFER), true));
        }

        if (mCategoryRelevant[CAPS_CATEGORY_D3D9])
        {
            pLog->logMessage(
                " * DirectX per stage constants: "
                + StringConverter::toString(hasCapability(RSC_PERSTAGECONSTANT), true));
            pLog->logMessage(
                " * W-Buffer supported: "
                + StringConverter::toString(hasCapability(RSC_WBUFFER), true));
        }
    }
    //---------------------------------------------------------------------
    String RenderSystemCapabilities::msGPUVendorStrings[GPU_VENDOR_COUNT];
    //---------------------------------------------------------------------
    GPUVendor RenderSystemCapabilities::vendorFromString(const String& vendorString)
    {
        initVendorStrings();
        GPUVendor ret = GPU_UNKNOWN;
        String cmpString = vendorString;
        StringUtil::toLowerCase(cmpString);
        for (int i = 0; i < GPU_VENDOR_COUNT; ++i)
        {
            // case insensitive (lower case)
            if (msGPUVendorStrings[i] == cmpString)
            {
                ret = static_cast<GPUVendor>(i);
                break;
            }
        }

        return ret;
        
    }
    //---------------------------------------------------------------------
    const String& RenderSystemCapabilities::vendorToString(GPUVendor v)
    {
        initVendorStrings();
        return msGPUVendorStrings[v];
    }
    //---------------------------------------------------------------------
    void RenderSystemCapabilities::initVendorStrings()
    {
        if (msGPUVendorStrings[0].empty())
        {
            // Always lower case!
            msGPUVendorStrings[GPU_UNKNOWN] = "unknown";
            msGPUVendorStrings[GPU_NVIDIA] = "nvidia";
            msGPUVendorStrings[GPU_AMD] = "amd";
            msGPUVendorStrings[GPU_INTEL] = "intel";
            msGPUVendorStrings[GPU_IMAGINATION_TECHNOLOGIES] = "imagination technologies";
            msGPUVendorStrings[GPU_APPLE] = "apple";    // iOS Simulator
            msGPUVendorStrings[GPU_NOKIA] = "nokia";
            msGPUVendorStrings[GPU_MS_SOFTWARE] = "microsoft"; // Microsoft software device
            msGPUVendorStrings[GPU_MS_WARP] = "ms warp";
            msGPUVendorStrings[GPU_ARM] = "arm";
            msGPUVendorStrings[GPU_QUALCOMM] = "qualcomm";
            msGPUVendorStrings[GPU_MOZILLA] = "mozilla";
            msGPUVendorStrings[GPU_WEBKIT] = "webkit";
        }
    }

}
