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

    void setObjectName( VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType,
                        const char *name )
    {
        // Check for a valid function pointer
        if( vkDebugMarkerSetObjectNameEXT )
        {
            VkDebugMarkerObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = objectType;
            nameInfo.object = object;
            nameInfo.pObjectName = name;
            vkDebugMarkerSetObjectNameEXT( device, &nameInfo );
        }
    }
}  // namespace Ogre
