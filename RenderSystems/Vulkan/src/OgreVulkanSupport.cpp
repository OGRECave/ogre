/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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

#include "OgreVulkanSupport.h"

#include "OgreVulkanRenderSystem.h"

#include "OgreVulkanUtils.h"

namespace Ogre
{
    void VulkanSupport::enumerateDevices( VulkanRenderSystem *renderSystem )
    {
        mDevices.clear();

        VkInstance instance = renderSystem->getVkInstance();

        VkResult result = VK_SUCCESS;

        uint32 numDevices = 0u;
        result = vkEnumeratePhysicalDevices( instance, &numDevices, NULL );
        checkVkResult( result, "vkEnumeratePhysicalDevices" );

        if( numDevices == 0u )
        {
            LogManager::getSingleton().logMessage( "[Vulkan] No Vulkan devices found." );
            return;
        }

        char tmpBuffer[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE + 32];
        LwString logStr( LwString::FromEmptyPointer( tmpBuffer, sizeof( tmpBuffer ) ) );

        logStr.clear();
        logStr.a( "[Vulkan] Found ", numDevices, " devices" );
        LogManager::getSingleton().logMessage( logStr.c_str() );

        FastArray<VkPhysicalDevice> pd;
        pd.resize( numDevices );
        result = vkEnumeratePhysicalDevices( instance, &numDevices, pd.begin() );
        checkVkResult( result, "vkEnumeratePhysicalDevices" );

        LogManager::getSingleton().logMessage( "[Vulkan] Found devices:" );

        mDevices.reserve( numDevices );
        for( uint32 i = 0u; i < numDevices; ++i )
        {
            VkPhysicalDeviceProperties deviceProps;
            vkGetPhysicalDeviceProperties( pd[i], &deviceProps );

            logStr.clear();
            logStr.a( deviceProps.deviceName, " #", i );
            mDevices.push_back( logStr.c_str() );

            LogManager::getSingleton().logMessage( logStr.c_str() );
        }
    }
    //-------------------------------------------------------------------------
    void VulkanSupport::initialize( VulkanRenderSystem *renderSystem )
    {
        if( renderSystem->getVkInstance() )
            return;

        renderSystem->initializeVkInstance();
        enumerateDevices( renderSystem );
    }
    //-------------------------------------------------------------------------
    void VulkanSupport::addConfig( VulkanRenderSystem *renderSystem )
    {
        initialize( renderSystem );

        ConfigOption optDevices;

        optDevices.name = "Device";

        FastArray<String>::const_iterator itor = mDevices.begin();
        FastArray<String>::const_iterator endt = mDevices.end();

        while( itor != endt )
            optDevices.possibleValues.push_back( *itor++ );

        optDevices.currentValue = mDevices.front();
        optDevices.immutable = false;

        mOptions[optDevices.name] = optDevices;
    }
    //-------------------------------------------------------------------------
    void VulkanSupport::setConfigOption( const String &name, const String &value )
    {
        ConfigOptionMap::iterator it = mOptions.find( name );

        if( it == mOptions.end() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Option named " + name + " does not exist.",
                         "VulkanSupport::setConfigOption" );
        }
        else
        {
            it->second.currentValue = value;
        }
    }
    //-------------------------------------------------------------------------
    String VulkanSupport::validateConfigOptions( void )
    {
        ConfigOptionMap::iterator it;

        it = mOptions.find( "Device" );
        if( it != mOptions.end() )
        {
            const String deviceName = it->second.currentValue;
            if( std::find( mDevices.begin(), mDevices.end(), deviceName ) == mDevices.end() )
            {
                setConfigOption( "Device", mDevices.front() );
                return "Requested rendering device could not be found, default will be used instead.";
            }
        }

        return BLANKSTRING;
    }
    //-------------------------------------------------------------------------
    uint32 VulkanSupport::getSelectedDeviceIdx( void ) const
    {
        uint32 deviceIdx = 0u;

        ConfigOptionMap::const_iterator it = mOptions.find( "Device" );
        if( it != mOptions.end() )
        {
            const String deviceName = it->second.currentValue;
            FastArray<String>::const_iterator itDevice =
                std::find( mDevices.begin(), mDevices.end(), deviceName );
            if( itDevice != mDevices.end() )
                deviceIdx = itDevice - mDevices.begin();
        }

        return deviceIdx;
    }
    //-------------------------------------------------------------------------
    ConfigOptionMap &VulkanSupport::getConfigOptions( VulkanRenderSystem *renderSystem )
    {
        initialize( renderSystem );
        return mOptions;
    }
}  // namespace Ogre
