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

#include "OgreVulkanUtils.h"

#include "OgreStringConverter.h"
#include "OgreVulkanDevice.h"
#include "OgreVulkanMappings.h"

namespace Ogre
{
    PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag;
    PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName;
    PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
    PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;
    PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert;

    String vkResultToString( VkResult result )
    {
        // clang-format off
        switch( result )
        {
        case VK_SUCCESS:                        return "VK_SUCCESS";
        case VK_NOT_READY:                      return "VK_NOT_READY";
        case VK_TIMEOUT:                        return "VK_TIMEOUT";
        case VK_EVENT_SET:                      return "VK_EVENT_SET";
        case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
        case VK_INCOMPLETE:                     return"VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:    return"VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:              return"VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:        return"VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:        return"VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:    return"VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:      return"VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:      return"VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:         return"VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:     return"VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:          return"VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_OUT_OF_POOL_MEMORY:       return"VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:  return"VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_SURFACE_LOST_KHR:         return"VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:                 return"VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:          return"VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:    return"VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:        return"VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_NOT_PERMITTED_EXT:        return"VK_ERROR_NOT_PERMITTED_EXT";
        default:
            return StringConverter::toString( result );
        }
        // clang-format on
    }

    void initUtils( VkDevice device )
    {
        pfnDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(
            device, "vkDebugMarkerSetObjectTagEXT" );
        pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
            device, "vkDebugMarkerSetObjectNameEXT" );
        pfnCmdDebugMarkerBegin =
            (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr( device, "vkCmdDebugMarkerBeginEXT" );
        pfnCmdDebugMarkerEnd =
            (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr( device, "vkCmdDebugMarkerEndEXT" );
        pfnCmdDebugMarkerInsert =
            (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr( device, "vkCmdDebugMarkerInsertEXT" );
    }

    void setObjectName( VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType,
                        const char *name )
    {
        // Check for a valid function pointer
        if( pfnDebugMarkerSetObjectName )
        {
            VkDebugMarkerObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = objectType;
            nameInfo.object = object;
            nameInfo.pObjectName = name;
            pfnDebugMarkerSetObjectName( device, &nameInfo );
        }
    }

    PixelFormatGpu findSupportedFormat( VkPhysicalDevice physicalDevice,
                                        const FastArray<PixelFormatGpu> &candidates,
                                        VkImageTiling tiling, VkFormatFeatureFlags features )
    {
        FastArray<PixelFormatGpu>::const_iterator itor = candidates.begin();
        FastArray<PixelFormatGpu>::const_iterator endt = candidates.end();

        while( itor != endt )
        {
            const VkFormat format = VulkanMappings::get( *itor );
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties( physicalDevice, format, &props );

            if( tiling == VK_IMAGE_TILING_LINEAR &&
                ( props.linearTilingFeatures & features ) == features )
            {
                return *itor;
            }
            else if( tiling == VK_IMAGE_TILING_OPTIMAL &&
                     ( props.optimalTilingFeatures & features ) == features )
            {
                return *itor;
            }

            ++itor;
        }

        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "failed to find supported format!",
                     "findSupportedFormat" );
    }
    //-------------------------------------------------------------------------
    uint32_t findMemoryType( VkPhysicalDeviceMemoryProperties &memProperties, uint32_t typeFilter,
                             VkMemoryPropertyFlags properties )
    {
        for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
        {
            if( ( typeFilter & ( 1 << i ) ) &&
                ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
            {
                return i;
            }
        }

        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, "failed to find suitable memory type!",
                     "findMemoryType" );
    }
    //-------------------------------------------------------------------------
    String getSpirvReflectError( SpvReflectResult spirvReflectResult )
    {
        switch( spirvReflectResult )
        {
        case SPV_REFLECT_RESULT_SUCCESS:
            return "SPV_REFLECT_RESULT_SUCCESS";
        case SPV_REFLECT_RESULT_NOT_READY:
            return "SPV_REFLECT_RESULT_NOT_READY";
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:
            return "SPV_REFLECT_RESULT_ERROR_PARSE_FAILED";
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:
            return "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED";
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:
            return "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED";
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:
            return "SPV_REFLECT_RESULT_ERROR_NULL_POINTER";
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:
            return "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR";
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:
            return "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH";
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:
            return "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
            /*case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:
                return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT";
            case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:
                return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE";*/
        }

        return "SPV_REFLECT_INVALID_ERROR_CODE";
    }

    VkSampleCountFlagBits getMaxUsableSampleCount( VkPhysicalDeviceProperties &physicalDeviceProperties,
                                                   uint32 requestedSampleCount )
    {
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        counts = std::min( counts, requestedSampleCount );
        if( counts & VK_SAMPLE_COUNT_64_BIT )
        {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        if( counts & VK_SAMPLE_COUNT_32_BIT )
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        if( counts & VK_SAMPLE_COUNT_16_BIT )
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        if( counts & VK_SAMPLE_COUNT_8_BIT )
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        if( counts & VK_SAMPLE_COUNT_4_BIT )
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        if( counts & VK_SAMPLE_COUNT_2_BIT )
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }
}  // namespace Ogre
