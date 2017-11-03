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
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11Prerequisites.h"
#include "OgreD3D11DriverList.h"
#include "OgreD3D11Driver.h"
#include "OgreD3D11VideoModeList.h"
#include "OgreD3D11VideoMode.h"
#include "OgreD3D11RenderWindow.h"
#include "OgreD3D11TextureManager.h"
#include "OgreD3D11Texture.h"
#include "OgreViewport.h"
#include "OgreLogManager.h"
#include "OgreD3D11HardwareBufferManager.h"
#include "OgreD3D11HardwareIndexBuffer.h"
#include "OgreD3D11HardwareVertexBuffer.h"
#include "OgreD3D11GpuProgram.h"
#include "OgreD3D11GpuProgramManager.h"
#include "OgreD3D11HLSLProgramFactory.h"

#include "OgreD3D11HardwareOcclusionQuery.h"
#include "OgreFrustum.h"
#include "OgreD3D11MultiRenderTarget.h"
#include "OgreD3D11HLSLProgram.h"

#include "OgreHlmsDatablock.h"
#include "OgreHlmsSamplerblock.h"
#include "OgreD3D11HlmsPso.h"

#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11DepthTexture.h"
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreException.h"

#include "Vao/OgreD3D11VaoManager.h"
#include "Vao/OgreD3D11BufferInterface.h"
#include "Vao/OgreD3D11VertexArrayObject.h"
#include "Vao/OgreD3D11UavBufferPacked.h"
#include "Vao/OgreIndexBufferPacked.h"
#include "Vao/OgreIndirectBufferPacked.h"
#include "CommandBuffer/OgreCbDrawCall.h"

#include "OgreProfiler.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreD3D11StereoDriverBridge.h"
#endif


#ifndef D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT
#   define D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT 4
#endif

#ifndef D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT
#   define D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT 1
#endif
//---------------------------------------------------------------------
#include <d3d10.h>
#include <OgreNsightChecker.h>


namespace Ogre 
{
#if 1
    HRESULT WINAPI D3D11CreateDeviceN(
        _In_opt_ IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        /*_In_reads_opt_( FeatureLevels )*/ CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        _Out_opt_ ID3D11DeviceN** ppDevice,
        _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
        _Out_opt_ ID3D11DeviceContextN** ppImmediateContext )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        return D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        ID3D11Device * device = NULL;
        ID3D11DeviceContext * context = NULL;
        ID3D11DeviceN * deviceN = NULL;
        ID3D11DeviceContextN * contextN = NULL;
        D3D_FEATURE_LEVEL featureLevel;

        HRESULT hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, 
                                        (ppDevice ? &device : NULL), &featureLevel, (ppImmediateContext ? &context : NULL));
        if(FAILED(hr)) goto bail;

        hr = device ? device->QueryInterface(__uuidof(ID3D11DeviceN), (void **)&deviceN) : hr;
        if(FAILED(hr)) goto bail;

        hr = context ? context->QueryInterface(__uuidof(ID3D11DeviceContextN), (void **)&contextN) : hr;
        if(FAILED(hr)) goto bail;

        if(ppDevice)            { *ppDevice = deviceN; deviceN = NULL; }
        if(pFeatureLevel)       { *pFeatureLevel = featureLevel; }
        if(ppImmediateContext)  { *ppImmediateContext = contextN; contextN = NULL; }

bail:
        SAFE_RELEASE(deviceN);
        SAFE_RELEASE(contextN);
        SAFE_RELEASE(device);
        SAFE_RELEASE(context);
        return hr;
#endif
    }
#else
    HRESULT WINAPI D3D11CreateDeviceN(
        IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        /*_In_reads_opt_( FeatureLevels )*/ CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        ID3D11DeviceN** ppDevice,
        D3D_FEATURE_LEVEL* pFeatureLevel,
        ID3D11DeviceContextN** ppImmediateContext );
#endif

    //---------------------------------------------------------------------
    D3D11RenderSystem::D3D11RenderSystem()
        : mDevice(),
          mBoundIndirectBuffer( 0 ),
          mSwIndirectBufferPtr( 0 ),
          mPso( 0 ),
          mBoundComputeProgram( 0 ),
          mMaxBoundUavCS( 0 ),
          mCurrentVertexBuffer( 0 ),
          mCurrentIndexBuffer( 0 ),
          mpDXGIFactory(0),
          mNumberOfViews( 0 ),
          mDepthStencilView( 0 ),
          mMaxModifiedUavPlusOne( 0 ),
          mUavsDirty( false ),
          mDSTResView(0)
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		 ,mStereoDriver(NULL)
#endif	
    {
        LogManager::getSingleton().logMessage( "D3D11 : " + getName() + " created." );

        memset( mRenderTargetViews, 0, sizeof( mRenderTargetViews ) );
        memset( mUavBuffers, 0, sizeof( mUavBuffers ) );
        memset( mUavs, 0, sizeof( mUavs ) );

        mRenderSystemWasInited = false;
        mSwitchingFullscreenCounter = 0;
        mDriverType = DT_HARDWARE;

        initRenderSystem();

        // set config options defaults
        initConfigOptions();

        // Clear class instance storage
        memset(mClassInstances, 0, sizeof(mClassInstances));
        memset(mNumClassInstances, 0, sizeof(mNumClassInstances));
    }
    //---------------------------------------------------------------------
    D3D11RenderSystem::~D3D11RenderSystem()
    {
        shutdown();

        // Deleting the HLSL program factory
        if (mHLSLProgramFactory)
        {
            // Remove from manager safely
            if (HighLevelGpuProgramManager::getSingletonPtr())
                HighLevelGpuProgramManager::getSingleton().removeFactory(mHLSLProgramFactory);
            delete mHLSLProgramFactory;
            mHLSLProgramFactory = 0;
        }

        LogManager::getSingleton().logMessage( "D3D11 : " + getName() + " destroyed." );
    }
    //---------------------------------------------------------------------
    const String& D3D11RenderSystem::getName() const
    {
        static String strName( "Direct3D11 Rendering Subsystem");
        return strName;
    }
    //---------------------------------------------------------------------
	const String& D3D11RenderSystem::getFriendlyName(void) const
	{
		static String strName("Direct3D 11");
		return strName;
	}
	//---------------------------------------------------------------------
    D3D11DriverList* D3D11RenderSystem::getDirect3DDrivers()
    {
        if( !mDriverList )
            mDriverList = new D3D11DriverList( mpDXGIFactory );

        return mDriverList;
    }
    //---------------------------------------------------------------------
	ID3D11DeviceN* D3D11RenderSystem::createD3D11Device(D3D11Driver* d3dDriver, OGRE_D3D11_DRIVER_TYPE ogreDriverType,
		D3D_FEATURE_LEVEL minFL, D3D_FEATURE_LEVEL maxFL, D3D_FEATURE_LEVEL* pFeatureLevel)
	{
		IDXGIAdapterN* pAdapter = (d3dDriver && ogreDriverType == DT_HARDWARE) ? d3dDriver->getDeviceAdapter() : NULL;

		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
		switch(ogreDriverType)
		{
		case DT_HARDWARE:
			if(d3dDriver == NULL)
				driverType = D3D_DRIVER_TYPE_HARDWARE;
			else if(0 == wcscmp(d3dDriver->getAdapterIdentifier().Description, L"NVIDIA PerfHUD"))
					driverType = D3D_DRIVER_TYPE_REFERENCE;
			break;
		case DT_SOFTWARE:
			driverType = D3D_DRIVER_TYPE_SOFTWARE;
			break;
		case DT_WARP:
			driverType = D3D_DRIVER_TYPE_WARP;
			break;
		}

		// determine deviceFlags
		UINT deviceFlags = 0;
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		// This flag is required in order to enable compatibility with Direct2D.
		deviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif
		if(OGRE_DEBUG_MODE && !IsWorkingUnderNsight() && D3D11Device::D3D_NO_EXCEPTION != D3D11Device::getExceptionsErrorLevel())
		{
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		if(!OGRE_THREAD_SUPPORT)
		{
			deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
		}

		// determine feature levels
		D3D_FEATURE_LEVEL requestedLevels[] = {
#if !__OGRE_WINRT_PHONE // Windows Phone support only FL 9.3, but simulator can create much more capable device, so restrict it artificially here
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
			D3D_FEATURE_LEVEL_11_1,
#endif
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
#endif // !__OGRE_WINRT_PHONE
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		D3D_FEATURE_LEVEL *pFirstFL = requestedLevels, *pLastFL = pFirstFL + ARRAYSIZE(requestedLevels) - 1;
		for(unsigned int i = 0; i < ARRAYSIZE(requestedLevels); i++)
		{
			if(minFL == requestedLevels[i])
				pLastFL = &requestedLevels[i];
			if(maxFL == requestedLevels[i])
				pFirstFL = &requestedLevels[i];
		}
		if(pLastFL < pFirstFL)
		{
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
				"Requested min level feature is bigger the requested max level feature.",
				"D3D11RenderSystem::initialise");
		}

		// create device
		ID3D11DeviceN* device = NULL;
		HRESULT hr = D3D11CreateDeviceN(pAdapter, driverType, NULL, deviceFlags, pFirstFL, pLastFL - pFirstFL + 1, D3D11_SDK_VERSION, &device, pFeatureLevel, 0);

		if(FAILED(hr) && 0 != (deviceFlags & D3D11_CREATE_DEVICE_DEBUG))
		{
			StringStream error;
			error << "Failed to create Direct3D11 device with debug layer (" << hr << ")\nRetrying without debug layer.";
			Ogre::LogManager::getSingleton().logMessage(error.str());

			// create device - second attempt, without debug layer
			deviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
			hr = D3D11CreateDeviceN(pAdapter, driverType, NULL, deviceFlags, pFirstFL, pLastFL - pFirstFL + 1, D3D11_SDK_VERSION, &device, pFeatureLevel, 0);
		}
		if(FAILED(hr))
		{
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr, "Failed to create Direct3D11 device", "D3D11RenderSystem::D3D11RenderSystem");
		}
		return device;
	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::initConfigOptions()
    {
        D3D11DriverList* driverList;
        D3D11Driver* driver;

        ConfigOption optDevice;
        ConfigOption optVideoMode;
        ConfigOption optFullScreen;
        ConfigOption optVSync;
        ConfigOption optVSyncInterval;
		ConfigOption optBackBufferCount;
        ConfigOption optAA;
        ConfigOption optFPUMode;
        ConfigOption optNVPerfHUD;
        ConfigOption optSRGB;
        ConfigOption optMinFeatureLevels;
        ConfigOption optMaxFeatureLevels;
        ConfigOption optExceptionsErrorLevel;
        ConfigOption optDriverType;
        ConfigOption optFastShaderBuildHack;
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		ConfigOption optStereoMode;
#endif

        driverList = this->getDirect3DDrivers();

        optDevice.name = "Rendering Device";
        optDevice.currentValue.clear();
        optDevice.possibleValues.clear();
        optDevice.immutable = false;

        optVideoMode.name = "Video Mode";
        optVideoMode.currentValue = "800 x 600 @ 32-bit colour";
        optVideoMode.immutable = false;

        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back( "Yes" );
        optFullScreen.possibleValues.push_back( "No" );
        optFullScreen.currentValue = "Yes";
        optFullScreen.immutable = false;

        for( unsigned j=0; j < driverList->count(); j++ )
        {
            driver = driverList->item(j);
            optDevice.possibleValues.push_back( driver->DriverDescription() );
            // Make first one default
            if( j==0 )
                optDevice.currentValue = driver->DriverDescription();
        }

        optVSync.name = "VSync";
        optVSync.immutable = false;
        optVSync.possibleValues.push_back( "Yes" );
        optVSync.possibleValues.push_back( "No" );
        optVSync.currentValue = "No";

        optVSyncInterval.name = "VSync Interval";
        optVSyncInterval.immutable = false;
        optVSyncInterval.possibleValues.push_back( "1" );
        optVSyncInterval.possibleValues.push_back( "2" );
        optVSyncInterval.possibleValues.push_back( "3" );
        optVSyncInterval.possibleValues.push_back( "4" );
        optVSyncInterval.currentValue = "1";

		optBackBufferCount.name = "Backbuffer Count";
		optBackBufferCount.immutable = false;
		optBackBufferCount.possibleValues.push_back( "Auto" );
		optBackBufferCount.possibleValues.push_back( "1" );
		optBackBufferCount.possibleValues.push_back( "2" );
		optBackBufferCount.currentValue = "Auto";


        optAA.name = "FSAA";
        optAA.immutable = false;
        optAA.possibleValues.push_back( "None" );
        optAA.currentValue = "None";

        optFPUMode.name = "Floating-point mode";
#if OGRE_DOUBLE_PRECISION
        optFPUMode.currentValue = "Consistent";
#else
        optFPUMode.currentValue = "Fastest";
#endif
        optFPUMode.possibleValues.clear();
        optFPUMode.possibleValues.push_back("Fastest");
        optFPUMode.possibleValues.push_back("Consistent");
        optFPUMode.immutable = false;

        optNVPerfHUD.currentValue = "No";
        optNVPerfHUD.immutable = false;
        optNVPerfHUD.name = "Allow NVPerfHUD";
        optNVPerfHUD.possibleValues.push_back( "Yes" );
        optNVPerfHUD.possibleValues.push_back( "No" );

        // SRGB on auto window
        optSRGB.name = "sRGB Gamma Conversion";
        optSRGB.possibleValues.push_back("Yes");
        optSRGB.possibleValues.push_back("No");
        optSRGB.currentValue = "No";
        optSRGB.immutable = false;      

        // min feature level
        optMinFeatureLevels;
        optMinFeatureLevels.name = "Min Requested Feature Levels";
        optMinFeatureLevels.possibleValues.push_back("9.1");
        optMinFeatureLevels.possibleValues.push_back("9.3");
        optMinFeatureLevels.possibleValues.push_back("10.0");
        optMinFeatureLevels.possibleValues.push_back("10.1");
        optMinFeatureLevels.possibleValues.push_back("11.0");

        optMinFeatureLevels.currentValue = "9.1";
        optMinFeatureLevels.immutable = false;      


        // max feature level
        optMaxFeatureLevels;
        optMaxFeatureLevels.name = "Max Requested Feature Levels";
        optMaxFeatureLevels.possibleValues.push_back("9.1");

#if __OGRE_WINRT_PHONE_80
        optMaxFeatureLevels.possibleValues.push_back("9.2");
        optMaxFeatureLevels.possibleValues.push_back("9.3");
        optMaxFeatureLevels.currentValue = "9.3";
#elif __OGRE_WINRT_PHONE || __OGRE_WINRT_STORE
        optMaxFeatureLevels.possibleValues.push_back("9.3");
        optMaxFeatureLevels.possibleValues.push_back("10.0");
        optMaxFeatureLevels.possibleValues.push_back("10.1");
        optMaxFeatureLevels.possibleValues.push_back("11.0");
        optMaxFeatureLevels.possibleValues.push_back("11.1");
        optMaxFeatureLevels.currentValue = "11.1";
#else
        optMaxFeatureLevels.possibleValues.push_back("9.3");
        optMaxFeatureLevels.possibleValues.push_back("10.0");
        optMaxFeatureLevels.possibleValues.push_back("10.1");
        optMaxFeatureLevels.possibleValues.push_back("11.0");
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
        if (isWindows8OrGreater())
        {
            optMaxFeatureLevels.possibleValues.push_back("11.1");
            optMaxFeatureLevels.currentValue = "11.1";
        }
        else
        {
            optMaxFeatureLevels.currentValue = "11.0";
        }
#else
        optMaxFeatureLevels.currentValue = "11.0";
#endif
#endif 
        optMaxFeatureLevels.immutable = false;      

        // Exceptions Error Level
        optExceptionsErrorLevel.name = "Information Queue Exceptions Bottom Level";
        optExceptionsErrorLevel.possibleValues.push_back("No information queue exceptions");
        optExceptionsErrorLevel.possibleValues.push_back("Corruption");
        optExceptionsErrorLevel.possibleValues.push_back("Error");
        optExceptionsErrorLevel.possibleValues.push_back("Warning");
        optExceptionsErrorLevel.possibleValues.push_back("Info (exception on any message)");
#if OGRE_DEBUG_MODE
        optExceptionsErrorLevel.currentValue = "Info (exception on any message)";
#else
        optExceptionsErrorLevel.currentValue = "No information queue exceptions";
#endif
        optExceptionsErrorLevel.immutable = false;
        

        // Driver type
        optDriverType.name = "Driver type";
        optDriverType.possibleValues.push_back("Hardware");
        optDriverType.possibleValues.push_back("Software");
        optDriverType.possibleValues.push_back("Warp");
        optDriverType.currentValue = "Hardware";
        optDriverType.immutable = false;

        //This option improves shader compilation times by massive amounts
        //(requires Hlms to be aware of it), making shader compile times comparable
        //to GL (which is measured in milliseconds per shader, instead of seconds).
        //There's two possible reasons to disable this hack:
        //  1. Easier debugging. Shader structs like "Material m[256];" get declared
        //     as "Material m[2];" which cause debuggers to show only 2 entires,
        //     instead of all of them. Some debuggers (like RenderDoc) allow changing
        //     the amount of elements displayed and workaround it; nonetheless
        //     disabling it makes your life easier.
        //  2. Troubleshooting an obscure GPU/driver combination. I tested this hack
        //     with a lot of hardware and it seems to work. However the possibility
        //     that it breaks with a specific GPU/driver combo always exists. In
        //     such case, the end user should be able to turn this off.
        optFastShaderBuildHack.name = "Fast Shader Build Hack";
        optFastShaderBuildHack.possibleValues.push_back( "Yes" );
        optFastShaderBuildHack.possibleValues.push_back( "No" );
        optFastShaderBuildHack.currentValue = "Yes";
        optFastShaderBuildHack.immutable = false;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		optStereoMode.name = "Stereo Mode";
		optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_NONE));
		optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_FRAME_SEQUENTIAL));
		optStereoMode.currentValue = optStereoMode.possibleValues[0];
		optStereoMode.immutable = false;
		
		mOptions[optStereoMode.name] = optStereoMode;
#endif

        mOptions[optDevice.name] = optDevice;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optVSync.name] = optVSync;
        mOptions[optVSyncInterval.name] = optVSyncInterval;
        mOptions[optAA.name] = optAA;
        mOptions[optFPUMode.name] = optFPUMode;
        mOptions[optNVPerfHUD.name] = optNVPerfHUD;
        mOptions[optSRGB.name] = optSRGB;
        mOptions[optMinFeatureLevels.name] = optMinFeatureLevels;
        mOptions[optMaxFeatureLevels.name] = optMaxFeatureLevels;
        mOptions[optExceptionsErrorLevel.name] = optExceptionsErrorLevel;
        mOptions[optDriverType.name] = optDriverType;
        mOptions[optFastShaderBuildHack.name] = optFastShaderBuildHack;

		mOptions[optBackBufferCount.name] = optBackBufferCount;

        
        refreshD3DSettings();
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::refreshD3DSettings()
    {
        ConfigOption* optVideoMode;
        D3D11Driver* driver = 0;
        D3D11VideoMode* videoMode;

        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        if( opt != mOptions.end() )
        {
            for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
            {
                driver = getDirect3DDrivers()->item(j);
                if( driver->DriverDescription() == opt->second.currentValue )
                    break;
            }

            if (driver)
            {
                opt = mOptions.find( "Video Mode" );
                optVideoMode = &opt->second;
                optVideoMode->possibleValues.clear();
                // get vide modes for this device
                for( unsigned k=0; k < driver->getVideoModeList()->count(); k++ )
                {
                    videoMode = driver->getVideoModeList()->item( k );
                    optVideoMode->possibleValues.push_back( videoMode->getDescription() );
                }

                // Reset video mode to default if previous doesn't avail in new possible values
                StringVector::const_iterator itValue =
                    std::find(optVideoMode->possibleValues.begin(),
                              optVideoMode->possibleValues.end(),
                              optVideoMode->currentValue);
                if (itValue == optVideoMode->possibleValues.end())
                {
                    optVideoMode->currentValue = "800 x 600 @ 32-bit colour";
                }

                // Also refresh FSAA options
                refreshFSAAOptions();
            }
        }

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setConfigOption( const String &name, const String &value )
    {
        initRenderSystem();

        LogManager::getSingleton().stream()
            << "D3D11 : RenderSystem Option: " << name << " = " << value;

        bool viewModeChanged = false;

        // Find option
        ConfigOptionMap::iterator it = mOptions.find( name );

        // Update
        if( it != mOptions.end() )
            it->second.currentValue = value;
        else
        {
            StringStream str;
            str << "Option named '" << name << "' does not exist.";
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, str.str(), "D3D11RenderSystem::setConfigOption" );
        }

        // Refresh other options if D3DDriver changed
        if( name == "Rendering Device" )
            refreshD3DSettings();

        if( name == "Full Screen" )
        {
            // Video mode is applicable
            it = mOptions.find( "Video Mode" );
            if (it->second.currentValue.empty())
            {
                it->second.currentValue = "800 x 600 @ 32-bit colour";
                viewModeChanged = true;
            }
        }

        if( name == "Min Requested Feature Levels" )
        {
            if (value == "9.1")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
            else if (value == "9.2")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_2;
            else if (value == "9.3")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_3;
            else if (value == "10.0")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_10_0;
            else if (value == "10.1")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_10_1;
            else if (value == "11.0")
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
            else
                mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
        }

        if( name == "Max Requested Feature Levels" )
        {
            if (value == "9.1")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
            else if (value == "9.2")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_2;
            else if (value == "9.3")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_3;
            else if (value == "10.0")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_10_0;
            else if (value == "10.1")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_10_1;
            else if (value == "11.0")
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
            else
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
#else
                mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
#endif
        }

        if( name == "Allow NVPerfHUD" )
        {
            if (value == "Yes")
                mUseNVPerfHUD = true;
            else
                mUseNVPerfHUD = false;
        }

        if (viewModeChanged || name == "Video Mode")
        {
            refreshFSAAOptions();
        }

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::refreshFSAAOptions(void)
    {

        ConfigOptionMap::iterator it = mOptions.find( "FSAA" );
        ConfigOption* optFSAA = &it->second;
        optFSAA->possibleValues.clear();

        it = mOptions.find("Rendering Device");
        D3D11Driver *driver = getDirect3DDrivers()->item(it->second.currentValue);
        if (driver)
        {
            it = mOptions.find("Video Mode");
            ID3D11DeviceN* device = createD3D11Device(driver, mDriverType, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, NULL);
            D3D11VideoMode* videoMode = driver->getVideoModeList()->item(it->second.currentValue); // Could be NULL if working over RDP/Simulator
            DXGI_FORMAT format = videoMode ? videoMode->getFormat() : DXGI_FORMAT_R8G8B8A8_UNORM;
            UINT numLevels = 0;
            // set maskable levels supported
            for (unsigned int n = 1; n <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; n++)
            {
                HRESULT hr = device->CheckMultisampleQualityLevels(format, n, &numLevels);
                if (SUCCEEDED(hr) && numLevels > 0)
                {
                    optFSAA->possibleValues.push_back(StringConverter::toString(n));

                    // 8x could mean 8xCSAA, and we need other designation for 8xMSAA
                    if((n == 8 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &numLevels)) && numLevels > 8)    // 8x CSAA
                    || (n == 16 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &numLevels)) && numLevels > 16)  // 16x CSAA
                    || (n == 16 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 8, &numLevels)) && numLevels > 16)) // 16xQ CSAA
                    {
                        optFSAA->possibleValues.push_back(StringConverter::toString(n) + " [Quality]");
                    }
                }
                else if(n == 16) // there could be case when 16xMSAA is not supported but 16xCSAA and may be 16xQ CSAA are supported
                {
                    bool csaa16x = SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &numLevels)) && numLevels > 16;
                    bool csaa16xQ = SUCCEEDED(device->CheckMultisampleQualityLevels(format, 8, &numLevels)) && numLevels > 16;
                    if(csaa16x || csaa16xQ)
                        optFSAA->possibleValues.push_back("16");
                    if(csaa16x && csaa16xQ)
                        optFSAA->possibleValues.push_back("16 [Quality]");
                }
            }
            SAFE_RELEASE(device);
        }

        if(optFSAA->possibleValues.empty())
        {
            optFSAA->possibleValues.push_back("1"); // D3D11 does not distinguish between noMSAA and 1xMSAA
        }

        // Reset FSAA to none if previous doesn't avail in new possible values
        StringVector::const_iterator itValue =
            std::find(optFSAA->possibleValues.begin(),
                      optFSAA->possibleValues.end(),
                      optFSAA->currentValue);
        if (itValue == optFSAA->possibleValues.end())
        {
            optFSAA->currentValue = optFSAA->possibleValues[0];
        }

    }
    //---------------------------------------------------------------------
    String D3D11RenderSystem::validateConfigOptions()
    {
        ConfigOptionMap::iterator it;
        
        // check if video mode is selected
        it = mOptions.find( "Video Mode" );
        if (it->second.currentValue.empty())
            return "A video mode must be selected.";

        it = mOptions.find( "Rendering Device" );
        bool foundDriver = false;
        D3D11DriverList* driverList = getDirect3DDrivers();
        for( ushort j=0; j < driverList->count(); j++ )
        {
            if( driverList->item(j)->DriverDescription() == it->second.currentValue )
            {
                foundDriver = true;
                break;
            }
        }

        if (!foundDriver)
        {
            // Just pick the first driver
            setConfigOption("Rendering Device", driverList->item(0)->DriverDescription());
            return "Your DirectX driver name has changed since the last time you ran OGRE; "
                "the 'Rendering Device' has been changed.";
        }

        return BLANKSTRING;
    }
    //---------------------------------------------------------------------
    ConfigOptionMap& D3D11RenderSystem::getConfigOptions()
    {
        // return a COPY of the current config options
        return mOptions;
    }
    //---------------------------------------------------------------------
    RenderWindow* D3D11RenderSystem::_initialise( bool autoCreateWindow, const String& windowTitle )
    {
        RenderWindow* autoWindow = NULL;
        LogManager::getSingleton().logMessage( "D3D11 : Subsystem Initialising" );

		if(IsWorkingUnderNsight())
			LogManager::getSingleton().logMessage( "D3D11 : Nvidia Nsight found");

        // Init using current settings
        mActiveD3DDriver = NULL;
        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
        {
            if( getDirect3DDrivers()->item(j)->DriverDescription() == opt->second.currentValue )
            {
                mActiveD3DDriver = getDirect3DDrivers()->item(j);
                break;
            }
        }

        if( !mActiveD3DDriver )
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Problems finding requested Direct3D driver!", "D3D11RenderSystem::initialise" );

        //AIZ:recreate the device for the selected adapter
        {
            mDevice.ReleaseAll();
            _cleanupDepthBuffers( false );

            opt = mOptions.find( "Information Queue Exceptions Bottom Level" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find Information Queue Exceptions Bottom Level option!", "D3D11RenderSystem::initialise" );
            String infoQType = opt->second.currentValue;

            if ("No information queue exceptions" == infoQType)
            {
#if OGRE_DEBUG_MODE
                // a debug build should always enable the debug layer and report errors
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_ERROR);
#else
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_NO_EXCEPTION);
#endif
            }
            else if ("Corruption" == infoQType)
            {
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_CORRUPTION);
            }
            else if ("Error" == infoQType)
            {
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_ERROR);
            }
            else if ("Warning" == infoQType)
            {
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_WARNING);
            }
            else if ("Info (exception on any message)" == infoQType)
            {
                D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_INFO);
            }


            // Driver type
            opt = mOptions.find( "Driver type" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find driver type!", "D3D11RenderSystem::initialise" );
            String driverTypeName = opt->second.currentValue;

            mDriverType = DT_HARDWARE;
            if ("Hardware" == driverTypeName)
            {
                 mDriverType = DT_HARDWARE;
            }
            if ("Software" == driverTypeName)
            {
                mDriverType = DT_SOFTWARE;
            }
            if ("Warp" == driverTypeName)
            {
                mDriverType = DT_WARP;
            }

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
            // Stereo driver must be created before device is created
            StereoModeType stereoMode = StringConverter::parseStereoMode(mOptions["Stereo Mode"].currentValue);
            D3D11StereoDriverBridge* stereoBridge = OGRE_NEW D3D11StereoDriverBridge(stereoMode);
#endif

            D3D11Driver* d3dDriver = mActiveD3DDriver;
            if(D3D11Driver* d3dDriverOverride = (mDriverType == DT_HARDWARE && mUseNVPerfHUD) ? getDirect3DDrivers()->item("NVIDIA PerfHUD") : NULL)
                d3dDriver = d3dDriverOverride;

            ID3D11DeviceN * device = createD3D11Device(d3dDriver, mDriverType, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, &mFeatureLevel);

            IDXGIDeviceN * pDXGIDevice;
            device->QueryInterface(__uuidof(IDXGIDeviceN), (void **)&pDXGIDevice);

            IDXGIAdapterN * pDXGIAdapter;
            pDXGIDevice->GetParent(__uuidof(IDXGIAdapterN), (void **)&pDXGIAdapter);

            if (mDriverType != DT_HARDWARE)
            {
                // get the IDXGIFactoryN from the device for software drivers
                // Remark(dec-09):
                //  Seems that IDXGIFactoryN::CreateSoftwareAdapter doesn't work with
                // D3D11CreateDevice - so I needed to create with pSelectedAdapter = 0.
                // If pSelectedAdapter == 0 then you have to get the IDXGIFactory1 from
                // the device - else CreateSwapChain fails later.
                //  Update (Jun 12, 2012)
                // If using WARP driver, get factory from created device
                SAFE_RELEASE(mpDXGIFactory);
                pDXGIAdapter->GetParent(__uuidof(IDXGIFactoryN), (void **)&mpDXGIFactory);
            }

            // We intentionally check for ID3D10Device support instead of ID3D11Device as CheckInterfaceSupport() is not supported for later.
            // We hope, that there would be one UMD for both D3D10 and D3D11, or two different but with the same version number,
            // or with different but correlated version numbers, so that blacklisting could be done with high confidence level.
            LARGE_INTEGER driverVersion;
            if(SUCCEEDED(pDXGIAdapter->CheckInterfaceSupport(IID_ID3D10Device /* intentionally D3D10, not D3D11 */, &driverVersion)))
            {
                mDriverVersion.major = HIWORD(driverVersion.HighPart);
                mDriverVersion.minor = LOWORD(driverVersion.HighPart);
                mDriverVersion.release = HIWORD(driverVersion.LowPart);
                mDriverVersion.build = LOWORD(driverVersion.LowPart);
            }

            SAFE_RELEASE(pDXGIAdapter);
            SAFE_RELEASE(pDXGIDevice);

            mDevice.TransferOwnership(device) ;

            //On AMD's GCN cards, there is no performance or memory difference between
            //PF_D24_UNORM_S8_UINT & PF_D32_FLOAT_X24_S8_UINT, so prefer the latter
            //on modern cards (GL >= 4.3) and that also claim to support this format.
            //NVIDIA's preference? Dunno, they don't tell. But at least the quality
            //will be consistent.
            if( mFeatureLevel >= D3D_FEATURE_LEVEL_11_0 )
                DepthBuffer::DefaultDepthBufferFormat = PF_D32_FLOAT_X24_S8_UINT;
        }

        if( autoCreateWindow )
        {
            bool fullScreen;
            opt = mOptions.find( "Full Screen" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find full screen option!", "D3D11RenderSystem::initialise" );
            fullScreen = opt->second.currentValue == "Yes";

            D3D11VideoMode* videoMode = NULL;
            unsigned int width, height;
            String temp;

            opt = mOptions.find( "Video Mode" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find Video Mode option!", "D3D11RenderSystem::initialise" );

            // The string we are manipulating looks like this :width x height @ colourDepth
            // Pull out the colour depth by getting what comes after the @ and a space
            String colourDepth = opt->second.currentValue.substr(opt->second.currentValue.rfind('@')+1);
            // Now we know that the width starts a 0, so if we can find the end we can parse that out
            String::size_type widthEnd = opt->second.currentValue.find(' ');
            // we know that the height starts 3 characters after the width and goes until the next space
            String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd+3);
            // Now we can parse out the values
            width = StringConverter::parseInt(opt->second.currentValue.substr(0, widthEnd));
            height = StringConverter::parseInt(opt->second.currentValue.substr(widthEnd+3, heightEnd));

            for( unsigned j=0; j < mActiveD3DDriver->getVideoModeList()->count(); j++ )
            {
                temp = mActiveD3DDriver->getVideoModeList()->item(j)->getDescription();

                // In full screen we only want to allow supported resolutions, so temp and
                // opt->second.currentValue need to match exactly, but in windowed mode we
                // can allow for arbitrary window sized, so we only need to match the
                // colour values
                if( (fullScreen && (temp == opt->second.currentValue)) ||
                    (!fullScreen && (temp.substr(temp.rfind('@')+1) == colourDepth)) )
                {
                    videoMode = mActiveD3DDriver->getVideoModeList()->item(j);
                    break;
                }
            }

            // sRGB window option
            bool hwGamma = false;
            opt = mOptions.find( "sRGB Gamma Conversion" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find sRGB option!", "D3D11RenderSystem::initialise" );
            hwGamma = opt->second.currentValue == "Yes";
            uint fsaa = 0;
            String fsaaHint;
            if( (opt = mOptions.find("FSAA")) != mOptions.end() )
            {
                StringVector values = StringUtil::split(opt->second.currentValue, " ", 1);
                fsaa = StringConverter::parseUnsignedInt(values[0]);
                if (values.size() > 1)
                    fsaaHint = values[1];
            }

            if( !videoMode )
            {
                LogManager::getSingleton().logMessage(
                            "WARNING D3D11: Couldn't find requested video mode. Forcing 32bpp. "
                            "If you have two GPUs and you're rendering to the GPU that is not "
                            "plugged to the monitor you can then ignore this message.",
                            LML_CRITICAL );
            }

            NameValuePairList miscParams;
            miscParams["colourDepth"] = StringConverter::toString(videoMode ? videoMode->getColourDepth() : 32);
            miscParams["FSAA"] = StringConverter::toString(fsaa);
            miscParams["FSAAHint"] = fsaaHint;
            miscParams["useNVPerfHUD"] = StringConverter::toString(mUseNVPerfHUD);
            miscParams["gamma"] = StringConverter::toString(hwGamma);
            //miscParams["useFlipSequentialMode"] = StringConverter::toString(true);

            opt = mOptions.find("VSync");
            if (opt == mOptions.end())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find VSync options!", "D3D11RenderSystem::initialise");
            bool vsync = (opt->second.currentValue == "Yes");
            miscParams["vsync"] = StringConverter::toString(vsync);

            opt = mOptions.find("VSync Interval");
            if (opt == mOptions.end())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find VSync Interval options!", "D3D11RenderSystem::initialise");
            miscParams["vsyncInterval"] = opt->second.currentValue;

            autoWindow = this->_createRenderWindow( windowTitle, width, height, 
                fullScreen, &miscParams );

            // If we have 16bit depth buffer enable w-buffering.
            assert( autoWindow );
            if ( PixelUtil::getNumElemBits( autoWindow->getFormat() ) == 16 )
            { 
                mWBuffer = true;
            } 
            else 
            {
                mWBuffer = false;
            }           
        }

        LogManager::getSingleton().logMessage("***************************************");
        LogManager::getSingleton().logMessage("*** D3D11 : Subsystem Initialized OK ***");
        LogManager::getSingleton().logMessage("***************************************");

        // call superclass method
        RenderSystem::_initialise( autoCreateWindow );
        this->fireDeviceEvent(&mDevice, "DeviceCreated");
        return autoWindow;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::reinitialise()
    {
        LogManager::getSingleton().logMessage( "D3D11 : Reinitializing" );
        this->shutdown();
    //  this->initialise( true );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::shutdown()
    {
        RenderSystem::shutdown();

        mRenderSystemWasInited = false;

        SAFE_RELEASE(mDSTResView);

        mPrimaryWindow = NULL; // primary window deleted by base class.
        freeDevice();
        SAFE_DELETE( mDriverList );
        SAFE_RELEASE( mpDXGIFactory );
        mActiveD3DDriver = NULL;
        mDevice.ReleaseAll();
        LogManager::getSingleton().logMessage("D3D11 : Shutting down cleanly.");
        SAFE_DELETE( mTextureManager );
        SAFE_DELETE( mHardwareBufferManager );
        SAFE_DELETE( mGpuProgramManager );
    }
    //---------------------------------------------------------------------
	RenderWindow* D3D11RenderSystem::_createRenderWindow(const String &name,
		unsigned int width, unsigned int height, bool fullScreen,
		const NameValuePairList *miscParams)
	{

		// Check we're not creating a secondary window when the primary
		// was fullscreen
		if (mPrimaryWindow && mPrimaryWindow->isFullScreen() && fullScreen == false)
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
				"Cannot create secondary windows not in full screen when the primary is full screen",
				"D3D11RenderSystem::_createRenderWindow");
		}

		// Log a message
		StringStream ss;
		ss << "D3D11RenderSystem::_createRenderWindow \"" << name << "\", " <<
			width << "x" << height << " ";
		if (fullScreen)
			ss << "fullscreen ";
		else
			ss << "windowed ";
		if (miscParams)
		{
			ss << " miscParams: ";
			NameValuePairList::const_iterator it;
			for (it = miscParams->begin(); it != miscParams->end(); ++it)
			{
				ss << it->first << "=" << it->second << " ";
			}
			LogManager::getSingleton().logMessage(ss.str());
		}

		String msg;

		// Make sure we don't already have a render target of the 
		// sam name as the one supplied
		if (mRenderTargets.find(name) != mRenderTargets.end())
		{
			msg = "A render target of the same name '" + name + "' already "
				"exists.  You cannot create a new window with this name.";
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msg, "D3D11RenderSystem::_createRenderWindow");
		}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		D3D11RenderWindowBase* win = new D3D11RenderWindowHwnd(mDevice, mpDXGIFactory);
#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		String windowType;
		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt = miscParams->find("windowType");
			if(opt != miscParams->end())
				windowType = opt->second;
		}

		D3D11RenderWindowBase* win = NULL;
#if !__OGRE_WINRT_PHONE_80
		if(win == NULL && windowType == "SurfaceImageSource")
			win = new D3D11RenderWindowImageSource(mDevice, mpDXGIFactory);
#endif // !__OGRE_WINRT_PHONE_80
		if(win == NULL)
			win = new D3D11RenderWindowCoreWindow(mDevice, mpDXGIFactory);
#endif
		win->create(name, width, height, fullScreen, miscParams);

		attachRenderTarget(*win);

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		// Must be called after device has been linked to window
		D3D11StereoDriverBridge::getSingleton().addRenderWindow(win);
		win->_validateStereo();
#endif

		// If this is the first window, get the D3D device and create the texture manager
		if (!mPrimaryWindow)
		{
			mPrimaryWindow = win;
			win->getCustomAttribute("D3DDEVICE", &mDevice);

			// Create the texture manager for use by others
			mTextureManager = new D3D11TextureManager(mDevice);
			// Also create hardware buffer manager
            mHardwareBufferManager = new v1::D3D11HardwareBufferManager(mDevice);

			// Create the GPU program manager
			mGpuProgramManager = new D3D11GpuProgramManager(mDevice);
			// create & register HLSL factory
			if (mHLSLProgramFactory == NULL)
				mHLSLProgramFactory = new D3D11HLSLProgramFactory(mDevice);
			mRealCapabilities = createRenderSystemCapabilities();
			mRealCapabilities->addShaderProfile("hlsl");

			// if we are using custom capabilities, then 
			// mCurrentCapabilities has already been loaded
			if (!mUseCustomCapabilities)
				mCurrentCapabilities = mRealCapabilities;

			fireEvent("RenderSystemCapabilitiesCreated");

			initialiseFromRenderSystemCapabilities(mCurrentCapabilities, mPrimaryWindow);

            assert( !mVaoManager );
            mVaoManager = OGRE_NEW D3D11VaoManager( false, mDevice, this );
		}
		else
		{
			mSecondaryWindows.push_back(win);
		}

		return win;
	}

    //---------------------------------------------------------------------
    void D3D11RenderSystem::fireDeviceEvent(D3D11Device* device, const String & name, D3D11RenderWindowBase* sendingWindow /* = NULL */)
    {
        NameValuePairList params;
        params["D3DDEVICE"] =  StringConverter::toString((size_t)device->get());
        if(sendingWindow)
            params["RenderWindow"] = StringConverter::toString((size_t)sendingWindow);
        fireEvent(name, &params);
    }
    //---------------------------------------------------------------------
    RenderSystemCapabilities* D3D11RenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();
        rsc->setDriverVersion(mDriverVersion);
        rsc->setDeviceName(mActiveD3DDriver->DriverDescription());
        rsc->setRenderSystemName(getName());

        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setStencilBufferBitDepth(8);

        rsc->setCapability(RSC_HW_GAMMA);
        rsc->setCapability(RSC_TEXTURE_SIGNED_INT);

        rsc->setCapability(RSC_VBO);
        UINT formatSupport;
        if( mFeatureLevel >= D3D_FEATURE_LEVEL_9_2 ||
            (SUCCEEDED( mDevice->CheckFormatSupport(DXGI_FORMAT_R32_UINT, &formatSupport)) &&
            0 != (formatSupport & D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER)) )
        {
            rsc->setCapability(RSC_32BIT_INDEX);
        }

        // Set number of texture units, always 16
        rsc->setNumTextureUnits(16);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_AUTOMIPMAP);
        rsc->setCapability(RSC_BLENDING);
        rsc->setCapability(RSC_DOT3);
        // Cube map
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->setCapability(RSC_CUBEMAPPING);
            rsc->setCapability(RSC_READ_BACK_AS_TEXTURE);
        }

        rsc->setCapability(RSC_EXPLICIT_FSAA_RESOLVE);

        // We always support compression, D3DX will decompress if device does not support
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
			rsc->setCapability(RSC_TWO_SIDED_STENCIL);

        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_HWOCCLUSION);
        rsc->setCapability(RSC_HWOCCLUSION_ASYNCHRONOUS);

        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);

        if( mFeatureLevel >= D3D_FEATURE_LEVEL_11_0 )
            rsc->setCapability(RSC_TEXTURE_CUBE_MAP_ARRAY);

        if( mFeatureLevel >= D3D_FEATURE_LEVEL_10_1 )
            rsc->setCapability(RSC_TEXTURE_GATHER);

        if( mFeatureLevel >= D3D_FEATURE_LEVEL_11_0 )
        {
            rsc->setMaximumResolutions( static_cast<ushort>(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION),
                                        static_cast<ushort>(D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION),
                                        static_cast<ushort>(D3D11_REQ_TEXTURECUBE_DIMENSION) );
        }
        else if( mFeatureLevel >= D3D_FEATURE_LEVEL_10_0 )
        {
            rsc->setMaximumResolutions( static_cast<ushort>(D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION),
                                        static_cast<ushort>(D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION),
                                        static_cast<ushort>(D3D10_REQ_TEXTURECUBE_DIMENSION) );
        }
        /*TODO
        else if( mFeatureLevel >= D3D_FEATURE_LEVEL_9_3 )
        {
            rsc->setMaximumResolutions( static_cast<ushort>(D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION),
                                        static_cast<ushort>(D3D_FL9_3_REQ_TEXTURE3D_U_V_OR_W_DIMENSION),
                                        static_cast<ushort>(D3D_FL9_3_REQ_TEXTURECUBE_DIMENSION) );
        }
        else
        {
            rsc->setMaximumResolutions( static_cast<ushort>(D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION),
                                        static_cast<ushort>(D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION),
                                        static_cast<ushort>(D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION) );
        }*/

        convertVertexShaderCaps(rsc);
        convertPixelShaderCaps(rsc);
        convertGeometryShaderCaps(rsc);
        convertHullShaderCaps(rsc);
        convertDomainShaderCaps(rsc);
        convertComputeShaderCaps(rsc);

        // Check support for dynamic linkage
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->setCapability(RSC_SHADER_SUBROUTINE);
        }

        rsc->setCapability(RSC_USER_CLIP_PLANES);
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

        rsc->setCapability(RSC_RTT_SEPARATE_DEPTHBUFFER);
        rsc->setCapability(RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE);


        // Adapter details
        const DXGI_ADAPTER_DESC1& adapterID = mActiveD3DDriver->getAdapterIdentifier();

        switch(mDriverType) {
        case DT_HARDWARE:
            // determine vendor
            // Full list of vendors here: http://www.pcidatabase.com/vendors.php?sort=id
            switch(adapterID.VendorId)
            {
            case 0x10DE:
                rsc->setVendor(GPU_NVIDIA);
                break;
            case 0x1002:
                rsc->setVendor(GPU_AMD);
                break;
            case 0x163C:
            case 0x8086:
                rsc->setVendor(GPU_INTEL);
                break;
            case 0x5333:
                rsc->setVendor(GPU_S3);
                break;
            case 0x3D3D:
                rsc->setVendor(GPU_3DLABS);
                break;
            case 0x102B:
                rsc->setVendor(GPU_MATROX);
                break;
            default:
                rsc->setVendor(GPU_UNKNOWN);
                break;
            };
            break;
        case DT_SOFTWARE:
            rsc->setVendor(GPU_MS_SOFTWARE);
            break;
        case DT_WARP:
            rsc->setVendor(GPU_MS_WARP);
            break;
        default:
            rsc->setVendor(GPU_UNKNOWN);
            break;
        }

        rsc->setCapability(RSC_INFINITE_FAR_PLANE);

        rsc->setCapability(RSC_TEXTURE_3D);
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE_3D);
            rsc->setCapability(RSC_TEXTURE_1D);
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5);
            rsc->setCapability(RSC_COMPLETE_TEXTURE_BINDING);
        }

        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->setCapability(RSC_TEXTURE_COMPRESSION_BC6H_BC7);
        }

        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
        rsc->setCapability(RSC_TEXTURE_FLOAT);

#ifdef D3D_FEATURE_LEVEL_9_3
        int numMultiRenderTargets = (mFeatureLevel > D3D_FEATURE_LEVEL_9_3) ? D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT :      // 8
                                    (mFeatureLevel == D3D_FEATURE_LEVEL_9_3) ? 4/*D3D_FL9_3_SIMULTANEOUS_RENDER_TARGET_COUNT*/ :    // 4
                                    1/*D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT*/;                                                // 1
#else
        int numMultiRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;     // 8
#endif

        rsc->setNumMultiRenderTargets(std::min(numMultiRenderTargets, (int)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
        rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);

        rsc->setCapability(RSC_POINT_SPRITES);
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
        rsc->setMaxPointSize(256); // TODO: guess!
    
        rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
        rsc->setNumVertexTextureUnits(4);
        rsc->setVertexTextureUnitsShared(false);

        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // actually irrelevant, but set
        rsc->setCapability(RSC_PERSTAGECONSTANT);

        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);

        rsc->setCapability(RSC_CONST_BUFFER_SLOTS_IN_SHADER);

        return rsc;

    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::initialiseFromRenderSystemCapabilities(
        RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if(caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Trying to initialize D3D11RenderSystem from RenderSystemCapabilities that do not support Direct3D11",
                "D3D11RenderSystem::initialiseFromRenderSystemCapabilities");
        }
        
        // add hlsl
        HighLevelGpuProgramManager::getSingleton().addFactory(mHLSLProgramFactory);

        Log* defaultLog = LogManager::getSingleton().getDefaultLog();
        if (defaultLog)
        {
            caps->log(defaultLog);
        }

        mGpuProgramManager->setSaveMicrocodesToCache(true);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertVertexShaderCaps(RenderSystemCapabilities* rsc) const
    {
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_9_1)
        {
            rsc->addShaderProfile("vs_4_0_level_9_1");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("vs_2_0");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_9_3)
        {
            rsc->addShaderProfile("vs_4_0_level_9_3");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("vs_2_a");
            rsc->addShaderProfile("vs_2_x");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->addShaderProfile("vs_4_0");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("vs_3_0");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
        {
            rsc->addShaderProfile("vs_4_1");
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("vs_5_0");
        }

        rsc->setCapability(RSC_VERTEX_PROGRAM);

        // TODO: constant buffers have no limits but lower models do
        // 16 boolean params allowed
        rsc->setVertexProgramConstantBoolCount(16);
        // 16 integer params allowed, 4D
        rsc->setVertexProgramConstantIntCount(16);
        // float params, always 4D
        rsc->setVertexProgramConstantFloatCount(512);

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertPixelShaderCaps(RenderSystemCapabilities* rsc) const
    {
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_9_1)
        {
            rsc->addShaderProfile("ps_4_0_level_9_1");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("ps_2_0");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_9_3)
        {
            rsc->addShaderProfile("ps_4_0_level_9_3");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("ps_2_a");
            rsc->addShaderProfile("ps_2_b");
            rsc->addShaderProfile("ps_2_x");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->addShaderProfile("ps_4_0");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("ps_3_0");
            rsc->addShaderProfile("ps_3_x");
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
        {
            rsc->addShaderProfile("ps_4_1");
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("ps_5_0");
        }


        rsc->setCapability(RSC_FRAGMENT_PROGRAM);


        // TODO: constant buffers have no limits but lower models do
        // 16 boolean params allowed
        rsc->setFragmentProgramConstantBoolCount(16);
        // 16 integer params allowed, 4D
        rsc->setFragmentProgramConstantIntCount(16);
        // float params, always 4D
        rsc->setFragmentProgramConstantFloatCount(512);

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertHullShaderCaps(RenderSystemCapabilities* rsc) const
    {
        // Only for shader model 5.0
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("hs_5_0");
            
            rsc->setCapability(RSC_TESSELLATION_HULL_PROGRAM);

            // TODO: constant buffers have no limits but lower models do
            // 16 boolean params allowed
            rsc->setTessellationHullProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setTessellationHullProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setTessellationHullProgramConstantFloatCount(512);
        }

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertDomainShaderCaps(RenderSystemCapabilities* rsc) const
    {
        // Only for shader model 5.0
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("ds_5_0");

            rsc->setCapability(RSC_TESSELLATION_DOMAIN_PROGRAM);


            // TODO: constant buffers have no limits but lower models do
            // 16 boolean params allowed
            rsc->setTessellationDomainProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setTessellationDomainProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setTessellationDomainProgramConstantFloatCount(512);
        }

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertComputeShaderCaps(RenderSystemCapabilities* rsc) const
    {

        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->addShaderProfile("cs_4_0");
            rsc->setCapability(RSC_COMPUTE_PROGRAM);
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
        {
            rsc->addShaderProfile("cs_4_1");
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("cs_5_0");
        }



        // TODO: constant buffers have no limits but lower models do
        // 16 boolean params allowed
        rsc->setComputeProgramConstantBoolCount(16);
        // 16 integer params allowed, 4D
        rsc->setComputeProgramConstantIntCount(16);
        // float params, always 4D
        rsc->setComputeProgramConstantFloatCount(512);

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertGeometryShaderCaps(RenderSystemCapabilities* rsc) const
    {
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->addShaderProfile("gs_4_0");
            rsc->setCapability(RSC_GEOMETRY_PROGRAM);
            rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1)
        {
            rsc->addShaderProfile("gs_4_1");
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("gs_5_0");
        }

        rsc->setGeometryProgramConstantFloatCount(512);
        rsc->setGeometryProgramConstantIntCount(16);
        rsc->setGeometryProgramConstantBoolCount(16);
        rsc->setGeometryProgramNumOutputVertices(1024);
    }
    //-----------------------------------------------------------------------
    bool D3D11RenderSystem::checkVertexTextureFormats(void)
    {
        return true;
    }
    //-----------------------------------------------------------------------
    bool D3D11RenderSystem::_checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage)
    {
        return true;
    }
    //-----------------------------------------------------------------------
    MultiRenderTarget * D3D11RenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget *retval;
        retval = new D3D11MultiRenderTarget(name);
        attachRenderTarget(*retval);

        return retval;
    }
    //-----------------------------------------------------------------------
    DepthBuffer* D3D11RenderSystem::_createDepthBufferFor( RenderTarget *renderTarget,
                                                           bool exactMatchFormat )
    {
        //Get surface data (mainly to get MSAA data)
        ID3D11Texture2D *rtTexture = 0;
        renderTarget->getCustomAttribute( "First_ID3D11Texture2D", &rtTexture );

        D3D11_TEXTURE2D_DESC BBDesc;
        ZeroMemory( &BBDesc, sizeof(D3D11_TEXTURE2D_DESC) );
        if( rtTexture )
        {
            rtTexture->GetDesc( &BBDesc );
        }
        else
        {
            //Depth textures.
            assert( dynamic_cast<D3D11DepthTextureTarget*>(renderTarget) );
            //BBDesc.ArraySize = renderTarget;
            BBDesc.SampleDesc.Count     = std::max( 1u, renderTarget->getFSAA() );
            BBDesc.SampleDesc.Quality   = atoi( renderTarget->getFSAAHint().c_str() );
        }

        // Create depth stencil texture
        ID3D11Texture2D* pDepthStencil = NULL;
        D3D11_TEXTURE2D_DESC descDepth;

        descDepth.Width     = renderTarget->getWidth();
        descDepth.Height    = renderTarget->getHeight();
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1; //BBDesc.ArraySize?

        PixelFormat desiredDepthBufferFormat = renderTarget->getDesiredDepthBufferFormat();

        if( !exactMatchFormat )
            desiredDepthBufferFormat = PF_D24_UNORM_S8_UINT;

        const bool bDepthTexture = renderTarget->prefersDepthTexture();

        if( bDepthTexture )
        {
            switch( desiredDepthBufferFormat )
            {
            case PF_D24_UNORM_S8_UINT:
            case PF_D24_UNORM_X8:
            case PF_X24_S8_UINT:
            case PF_D24_UNORM:
                descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
                break;
            case PF_D16_UNORM:
                descDepth.Format = DXGI_FORMAT_R16_TYPELESS;
                break;
            case PF_D32_FLOAT:
                descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
                break;
            case PF_D32_FLOAT_X24_S8_UINT:
            case PF_D32_FLOAT_X24_X8:
            case PF_X32_X24_S8_UINT:
                descDepth.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
                break;
            default:
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "PixelFormat '" + PixelUtil::getFormatName( desiredDepthBufferFormat ) +
                             "' is not a valid depth buffer format",
                             "D3D11RenderSystem::_createDepthBufferFor" );
            }
        }
        else
        {
            switch( desiredDepthBufferFormat )
            {
            case PF_D24_UNORM_S8_UINT:
            case PF_D24_UNORM_X8:
            case PF_X24_S8_UINT:
            case PF_D24_UNORM:
                descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                break;
            case PF_D16_UNORM:
                descDepth.Format = DXGI_FORMAT_D16_UNORM;
                break;
            case PF_D32_FLOAT:
                descDepth.Format = DXGI_FORMAT_D32_FLOAT;
                break;
            case PF_D32_FLOAT_X24_S8_UINT:
            case PF_D32_FLOAT_X24_X8:
            case PF_X32_X24_S8_UINT:
                descDepth.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                break;
            default:
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "PixelFormat '" + PixelUtil::getFormatName( desiredDepthBufferFormat ) +
                             "' is not a valid depth buffer format",
                             "D3D11RenderSystem::_createDepthBufferFor" );
            }
        }

        {
            UINT supported = 0;
            mDevice->CheckFormatSupport( descDepth.Format, &supported );

            if( !supported )
                return 0;
        }

        descDepth.SampleDesc.Count      = BBDesc.SampleDesc.Count;
        descDepth.SampleDesc.Quality    = BBDesc.SampleDesc.Quality;
        descDepth.Usage                 = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags             = D3D11_BIND_DEPTH_STENCIL;

        // If we tell we want to use it as a Shader Resource when in MSAA, we will fail
        // This is a recomandation from NVidia.
        if( bDepthTexture && (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1 || BBDesc.SampleDesc.Count == 1) )
            descDepth.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

        descDepth.CPUAccessFlags        = 0;
        descDepth.MiscFlags             = 0;

        if (descDepth.ArraySize == 6)
        {
            descDepth.MiscFlags     |= D3D11_RESOURCE_MISC_TEXTURECUBE;
        }


        HRESULT hr = mDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
        if( FAILED(hr) || mDevice.isError())
        {
            String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to create depth texture\nError Description:" + errorDescription,
                "D3D11RenderSystem::_createDepthBufferFor");
        }

        //
        // Create the View of the texture
        // If MSAA is used, we cannot do this
        //
        ID3D11ShaderResourceView *depthTextureView = 0;
        if( bDepthTexture && (mFeatureLevel >= D3D_FEATURE_LEVEL_10_1 || BBDesc.SampleDesc.Count == 1) )
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            //viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
            viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            switch( desiredDepthBufferFormat )
            {
            case PF_D24_UNORM_S8_UINT:
                //TODO: Unsupported by API?
                viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                break;
            case PF_D24_UNORM_X8:
                viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                break;
            case PF_X24_S8_UINT:
                viewDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
                break;
            case PF_D24_UNORM:
                viewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                break;
            case PF_D16_UNORM:
                viewDesc.Format = DXGI_FORMAT_R16_UNORM;
                break;
            case PF_D32_FLOAT:
                viewDesc.Format = DXGI_FORMAT_R32_FLOAT;
                break;
            case PF_D32_FLOAT_X24_S8_UINT:
                //TODO: Unsupported by API?
                viewDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                break;
            case PF_D32_FLOAT_X24_X8:
                viewDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                break;
            case PF_X32_X24_S8_UINT:
                viewDesc.Format = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                break;
            default:
                assert( false && "This is impossible" );
                break;
            }

            viewDesc.ViewDimension = BBDesc.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS :
                                                                   D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2D.MipLevels = 1;
            HRESULT hr = mDevice->CreateShaderResourceView( pDepthStencil, &viewDesc, &depthTextureView);
            if( FAILED(hr) || mDevice.isError())
            {
                SAFE_RELEASE( pDepthStencil );
                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                    "Unable to create the view of the depth texture \nError Description:" + errorDescription,
                    "D3D11RenderSystem::_createDepthBufferFor");
            }
        }

        // Create the depth stencil view
        ID3D11DepthStencilView      *depthStencilView;
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );

        if( bDepthTexture )
        {
            switch( desiredDepthBufferFormat )
            {
            case PF_D24_UNORM_S8_UINT:
            case PF_D24_UNORM_X8:
            case PF_X24_S8_UINT:
            case PF_D24_UNORM:
                descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                break;
            case PF_D16_UNORM:
                descDSV.Format = DXGI_FORMAT_R16_UNORM;
                break;
            case PF_D32_FLOAT:
                descDSV.Format = DXGI_FORMAT_D32_FLOAT;
                break;
            case PF_D32_FLOAT_X24_S8_UINT:
            case PF_D32_FLOAT_X24_X8:
            case PF_X32_X24_S8_UINT:
                descDSV.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                break;
            default:
                assert( false && "This is impossible" );
                break;
            }
        }
        else
            descDSV.Format = descDepth.Format;

        descDSV.ViewDimension = (BBDesc.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS :
                                                                D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Flags = 0 /* D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL */;    // TODO: Allows bind depth buffer as depth view AND texture simultaneously.
                                                                                            // TODO: Decide how to expose this feature
        descDSV.Texture2D.MipSlice = 0;
        hr = mDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &depthStencilView );
        if( FAILED(hr) )
        {
            SAFE_RELEASE( depthTextureView );
            SAFE_RELEASE( pDepthStencil );
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Unable to create depth stencil view\nError Description:" + errorDescription,
                "D3D11RenderSystem::_createDepthBufferFor");
        }

        //Create the abstract container
        D3D11DepthBuffer *newDepthBuffer = new D3D11DepthBuffer( DepthBuffer::POOL_DEFAULT, this,
                                                                 pDepthStencil,
                                                                 depthStencilView,
                                                                 depthTextureView,
                                                                 descDepth.Width, descDepth.Height,
                                                                 descDepth.SampleDesc.Count,
                                                                 descDepth.SampleDesc.Quality,
                                                                 desiredDepthBufferFormat,
                                                                 bDepthTexture, false );

        return newDepthBuffer;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_removeManualDepthBuffer(DepthBuffer *depthBuffer)
    {
        if(depthBuffer != NULL)
        {
            DepthBufferVec& pool = mDepthBufferPool[depthBuffer->getPoolId()];
            pool.erase(std::remove(pool.begin(), pool.end(), depthBuffer), pool.end());
        }
    }
    //---------------------------------------------------------------------
    RenderTarget* D3D11RenderSystem::detachRenderTarget(const String &name)
    {
        RenderTarget* target = RenderSystem::detachRenderTarget(name);
        detachRenderTargetImpl(name);
        return target;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::detachRenderTargetImpl(const String& name)
    {
        // Check in specialized lists
		if (mPrimaryWindow != NULL && mPrimaryWindow->getName() == name)
        {
            // We're destroying the primary window, so reset device and window
			mPrimaryWindow = NULL;
        }
        else
        {
            // Check secondary windows
            SecondaryWindowList::iterator sw;
            for (sw = mSecondaryWindows.begin(); sw != mSecondaryWindows.end(); ++sw)
            {
                if ((*sw)->getName() == name)
                {
                    mSecondaryWindows.erase(sw);
                    break;
                }
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::destroyRenderTarget(const String& name)
    {
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		D3D11StereoDriverBridge::getSingleton().removeRenderWindow(name);
#endif

        detachRenderTargetImpl(name);

        // Do the real removal
        RenderSystem::destroyRenderTarget(name);

        // Did we destroy the primary?
        if (!mPrimaryWindow)
        {
            // device is no longer valid, so free it all up
            freeDevice();
        }

    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::freeDevice(void)
    {
        if (!mDevice.isNull() && mCurrentCapabilities)
        {
            // Set all texture units to nothing to release texture surfaces
            _disableTextureUnitsFrom(0);
            _cleanupDepthBuffers( false );
            // Clean up depth stencil surfaces
            mDevice.ReleaseAll();
            //mActiveD3DDriver->setDevice(D3D11Device(NULL));
        }
    }
    //---------------------------------------------------------------------
    bool D3D11RenderSystem::isWindows8OrGreater()
    {
        DWORD version = GetVersion();
        DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
        DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));
        return (major > 6) || ((major == 6) && (minor >= 2));
    }
    //---------------------------------------------------------------------
    VertexElementType D3D11RenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ABGR;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
        Matrix4& dest, bool forGpuProgram)
    {
        dest = matrix;

        // Convert depth range from [-1,+1] to [0,1]
        dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
        dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
        dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
        dest[2][3] = (dest[2][3] + dest[3][3]) / 2;

        if (!forGpuProgram)
        {
            // Convert right-handed to left-handed
            dest[0][2] = -dest[0][2];
            dest[1][2] = -dest[1][2];
            dest[2][2] = -dest[2][2];
            dest[3][2] = -dest[3][2];
        }
    }
    //-------------------------------------------------------------------------
    Real D3D11RenderSystem::getRSDepthRange(void) const
    {
        return 1.0f;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, 
        Real farPlane, Matrix4& dest, bool forGpuProgram)
    {
        Radian theta ( fovy * 0.5 );
        Real h = 1 / Math::Tan(theta);
        Real w = h / aspect;
        Real q, qn;
        if (farPlane == 0)
        {
            q = 1 - Frustum::INFINITE_FAR_PLANE_ADJUST;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 1);
        }
        else
        {
            q = farPlane / ( farPlane - nearPlane );
            qn = -q * nearPlane;
        }

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;

        if (forGpuProgram)
        {
            dest[2][2] = -q;
            dest[3][2] = -1.0f;
        }
        else
        {
            dest[2][2] = q;
            dest[3][2] = 1.0f;
        }

        dest[2][3] = qn;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
        Matrix4& dest, bool forGpuProgram )
    {
        Radian thetaY (fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        //Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; //Math::Tan(thetaX);
        Real half_w = tanThetaX * nearPlane;
        Real half_h = tanThetaY * nearPlane;
        Real iw = 1.0f / half_w;
        Real ih = 1.0f / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 1.0f / (farPlane - nearPlane);
        }

        dest = Matrix4::ZERO;
        dest[0][0] = iw;
        dest[1][1] = ih;
        dest[2][2] = q;
        dest[2][3] = -nearPlane / (farPlane - nearPlane);
        dest[3][3] = 1;

        if (forGpuProgram)
        {
            dest[2][2] = -dest[2][2];
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setShadingType( ShadeOptions so )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setLightingEnabled( bool enabled )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setViewMatrix( const Matrix4 &m )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setProjectionMatrix( const Matrix4 &m )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setWorldMatrix( const Matrix4 &m )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
        const ColourValue &specular, const ColourValue &emissive, Real shininess,
        TrackVertexColourType tracking )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setPointParameters(Real size, 
        bool attenuationEnabled, Real constant, Real linear, Real quadratic,
        Real minSize, Real maxSize)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setPointSpritesEnabled(bool enabled)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTexture( size_t stage, bool enabled, Texture *tex )
    {
        D3D11Texture *dt = static_cast<D3D11Texture*>( tex );
        if (enabled && dt && dt->getSize() > 0)
        {
            // note used
            dt->touch();
            ID3D11ShaderResourceView * pTex = dt->getTexture();
            mTexStageDesc[stage].pTex = pTex;
            mTexStageDesc[stage].used = true;
            mTexStageDesc[stage].type = dt->getTextureType();

            mLastTextureUnitState = stage+1;

            mDevice.GetImmediateContext()->VSSetShaderResources(static_cast<UINT>(stage), static_cast<UINT>(1), &pTex);
            mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(stage), static_cast<UINT>(1), &pTex);
            //mDevice.GetImmediateContext()->VSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
        }
        else
        {
            mTexStageDesc[stage].used = false;
            // now we now what's the last texture unit set
			mLastTextureUnitState = std::min(mLastTextureUnitState,stage);
        }
        mSamplerStatesChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setBindingType(TextureUnitState::BindingType bindingType)
    {
        mBindingType = bindingType;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setVertexTexture(size_t stage, const TexturePtr& tex)
    {
        if (tex.isNull())
            _setTexture(stage, false, tex.get());
        else
            _setTexture(stage, true, tex.get());
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setGeometryTexture(size_t stage, const TexturePtr& tex)
    {
        if (tex.isNull())
            _setTexture(stage, false, tex.get());
        else
            _setTexture(stage, true, tex.get());
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTessellationHullTexture(size_t stage, const TexturePtr& tex)
    {
        if (tex.isNull())
            _setTexture(stage, false, tex.get());
        else
            _setTexture(stage, true, tex.get());
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTessellationDomainTexture(size_t stage, const TexturePtr& tex)
    {
        if (tex.isNull())
            _setTexture(stage, false, tex.get());
        else
            _setTexture(stage, true, tex.get());
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_disableTextureUnit(size_t texUnit)
    {
        RenderSystem::_disableTextureUnit(texUnit);
        // also disable vertex texture unit
        static TexturePtr nullPtr;
        _setVertexTexture(texUnit, nullPtr);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureCoordSet( size_t stage, size_t index )
    {
        mTexStageDesc[stage].coordIndex = index;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureCoordCalculation( size_t stage, TexCoordCalcMethod m,
        const Frustum* frustum)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureMatrix( size_t stage, const Matrix4& xForm )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureBlendMode( size_t stage, const LayerBlendModeEx& bm )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setFog( FogMode mode, const ColourValue& colour, Real densitiy, Real start, Real end )
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setStencilBufferParams( uint32 refValue, const StencilParams &stencilParams )
    {
        RenderSystem::setStencilBufferParams( refValue, stencilParams );

        mStencilRef = refValue;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setRenderTarget( RenderTarget *target, uint8 viewportRenderTargetFlags )
    {
        mActiveViewport = 0;
        mActiveRenderTarget = target;
        if (mActiveRenderTarget)
        {
            // we need to clear the state -- dark_sylinc: No we don't.
            //mDevice.GetImmediateContext()->ClearState();

            //Set all textures to 0 to prevent the runtime from thinkin we might
            //be sampling from the render target (common when doing shadow map
            //rendering)
            ID3D11ShaderResourceView *nullViews[16];
            memset( nullViews, 0, sizeof( nullViews ) );
            ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();
            deviceContext->VSSetShaderResources( 0, 16, nullViews );
            deviceContext->PSSetShaderResources( 0, 16, nullViews );
            deviceContext->HSSetShaderResources( 0, 16, nullViews );
            deviceContext->DSSetShaderResources( 0, 16, nullViews );
            deviceContext->GSSetShaderResources( 0, 16, nullViews );
            deviceContext->CSSetShaderResources( 0, 16, nullViews );

            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "D3D11 device cannot Clear State\nError Description:" + errorDescription,
                    "D3D11RenderSystem::_setRenderTarget");
            }

            _setRenderTargetViews( viewportRenderTargetFlags );
        }
    }

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setRenderTargetViews( uint8 viewportRenderTargetFlags )
    {
        RenderTarget *target = mActiveRenderTarget;

        if (target)
        {
            target->getCustomAttribute( "ID3D11RenderTargetView", &mRenderTargetViews );
            target->getCustomAttribute( "numberOfViews", &mNumberOfViews );

            //Retrieve depth buffer
            D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH && !depthBuffer )
            {
                //Depth is automatically managed and there is no depth buffer attached to this RT
                //or the Current D3D device doesn't match the one this Depth buffer was created
                setDepthBufferFor( target, true );
            }

            //Retrieve depth buffer again (it may have changed)
            depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

            if( !(viewportRenderTargetFlags & VP_RTT_COLOUR_WRITE) )
                mNumberOfViews = 0;

            mDepthStencilView = depthBuffer ?
                        depthBuffer->getDepthStencilView( viewportRenderTargetFlags ) : 0;

            if( mMaxModifiedUavPlusOne )
            {
                if( mUavStartingSlot < mNumberOfViews )
                {
                    OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                        "mUavStartingSlot is lower than the number of RenderTargets attached.\n"
                        "There are " + StringConverter::toString( mNumberOfViews ) + " RTs attached,\n"
                        "and mUavStartingSlot = " + StringConverter::toString( mUavStartingSlot ) + "\n"
                        "use setUavStartingSlot to fix this, or set a MRT with less RTs",
                        "D3D11RenderSystem::_setRenderTargetViews" );
                }

                mDevice.GetImmediateContext()->OMSetRenderTargetsAndUnorderedAccessViews(
                            mNumberOfViews, mRenderTargetViews, mDepthStencilView,
                            mUavStartingSlot, mMaxModifiedUavPlusOne, mUavs, 0 );
            }
            else
            {
                // now switch to the new render target
                mDevice.GetImmediateContext()->OMSetRenderTargets( mNumberOfViews, mRenderTargetViews,
                                                                   mDepthStencilView );
            }

            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "D3D11 device cannot set render target\nError Description:" + errorDescription,
                    "D3D11RenderSystem::_setRenderTargetViews");
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setViewport( Viewport *vp )
    {
        if (!vp)
        {
            mActiveViewport = NULL;
            _setRenderTarget(NULL, VP_RTT_COLOUR_WRITE);
        }
        else if( vp != mActiveViewport || vp->_isUpdated() )
        {
            // ok, it's different, time to set render target and viewport params
            D3D11_VIEWPORT d3dvp;

            // Set render target
            RenderTarget* target;
            target = vp->getTarget();

            _setRenderTarget(target, vp->getViewportRenderTargetFlags());

            mActiveViewport = vp;

            // set viewport dimensions
            d3dvp.TopLeftX = static_cast<FLOAT>(vp->getActualLeft());
            d3dvp.TopLeftY = static_cast<FLOAT>(vp->getActualTop());
            d3dvp.Width = static_cast<FLOAT>(vp->getActualWidth());
            d3dvp.Height = static_cast<FLOAT>(vp->getActualHeight());
            if (target->requiresTextureFlipping())
            {
                // Convert "top-left" to "bottom-left"
                d3dvp.TopLeftY = target->getHeight() - d3dvp.Height - d3dvp.TopLeftY;
            }

            // Z-values from 0.0 to 1.0 (TODO: standardise with OpenGL)
            d3dvp.MinDepth = 0.0f;
            d3dvp.MaxDepth = 1.0f;

            mDevice.GetImmediateContext()->RSSetViewports(1, &d3dvp);
            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                    "D3D11 device cannot set viewports\nError Description: " + errorDescription,
                    "D3D11RenderSystem::_setViewport");
            }

            D3D11_RECT scissorRect;
            scissorRect.left    = static_cast<LONG>(vp->getScissorActualLeft());
            scissorRect.top     = static_cast<LONG>(vp->getScissorActualTop());
            scissorRect.right   = scissorRect.left + static_cast<LONG>(vp->getScissorActualWidth());
            scissorRect.bottom  = scissorRect.top + static_cast<LONG>(vp->getScissorActualHeight());

            mDevice.GetImmediateContext()->RSSetScissorRects( 1, &scissorRect );
            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "D3D11 device cannot set scissor rects\nError Description: " + errorDescription,
                    "D3D11RenderSystem::_setViewport");
            }

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
            if( target->isRenderWindow() )
            {
                assert( dynamic_cast<D3D11RenderWindowBase*>(target) );
                D3D11RenderWindowBase* d3d11Window = static_cast<D3D11RenderWindowBase*>(target);
                d3d11Window->_validateStereo();
            }
#endif

            vp->_clearUpdatedFlag();
        }
        else
        {
            if( vp->getTarget()->isRenderWindow() )
            {
                // if swapchain was created with DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
                // we need to reestablish render target views
                assert( dynamic_cast<D3D11RenderWindowBase*>(vp->getTarget()) );

                if( static_cast<D3D11RenderWindowBase*>(vp->getTarget())->_shouldRebindBackBuffer() )
                    _setRenderTargetViews( vp->getViewportRenderTargetFlags() );
            }
            else if( mUavsDirty )
            {
                flushUAVs();
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::queueBindUAV( uint32 slot, TexturePtr texture,
                                          ResourceAccess::ResourceAccess access,
                                          int32 mipmapLevel, int32 textureArrayIndex,
                                          PixelFormat pixelFormat )
    {
        assert( slot < 64 );

        if( !mUavBuffers[slot] && mUavTexPtr[slot].isNull() && texture.isNull() )
            return;

        mUavsDirty = true;

        mUavTexPtr[slot] = texture;

        if( mUavBuffers[slot] )
        {
            //If the UAV view belonged to a buffer, don't decrement the reference count.
            mUavBuffers[slot] = 0;
            mUavs[slot] = 0;
        }

        //Release oldUav *after* we've created the new UAV (if D3D11 needs
        //to return the same UAV, if we release it earlier we may cause
        //unnecessary alloc/deallocations)
        ID3D11UnorderedAccessView *oldUav = mUavs[slot];
        mUavs[slot] = 0;

        if( !texture.isNull() )
        {
            if( !(texture->getUsage() & TU_UAV) )
            {
                if( oldUav )
                {
                    oldUav->Release();
                    oldUav = 0;
                }

                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                             "Texture " + texture->getName() +
                             "must have been created with TU_UAV to be bound as UAV",
                             "D3D11RenderSystem::queueBindUAV" );
            }

            D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
            descUAV.Format = D3D11Mappings::_getPF( pixelFormat );

            switch( texture->getTextureType() )
            {
            case TEX_TYPE_1D:
                descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
                descUAV.Texture1D.MipSlice = static_cast<UINT>( mipmapLevel );
                break;
            case TEX_TYPE_2D:
                descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                descUAV.Texture2D.MipSlice = static_cast<UINT>( mipmapLevel );
                break;
            case TEX_TYPE_2D_ARRAY:
                descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                descUAV.Texture2DArray.MipSlice         = static_cast<UINT>( mipmapLevel );
                descUAV.Texture2DArray.FirstArraySlice  = textureArrayIndex;
                descUAV.Texture2DArray.ArraySize        = static_cast<UINT>( texture->getDepth() -
                                                                             textureArrayIndex );
                break;
            case TEX_TYPE_3D:
                descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
                descUAV.Texture3D.MipSlice      = static_cast<UINT>( mipmapLevel );
                descUAV.Texture3D.FirstWSlice   = 0;
                descUAV.Texture3D.WSize         = static_cast<UINT>(texture->getDepth());
                break;
            default:
                break;
            }

            D3D11Texture *dt = static_cast<D3D11Texture*>( texture.get() );

            HRESULT hr = mDevice->CreateUnorderedAccessView( dt->getTextureResource(), &descUAV,
                                                             &mUavs[slot] );
            if( FAILED(hr) )
            {
                if( oldUav )
                {
                    oldUav->Release();
                    oldUav = 0;
                }

                String errorDescription = mDevice.getErrorDescription(hr);
                OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                    "Failed to create UAV state on texture '" + texture->getName() +
                    "'\nError Description: " + errorDescription,
                    "D3D11RenderSystem::queueBindUAV" );
            }

            mMaxModifiedUavPlusOne = std::max( mMaxModifiedUavPlusOne, static_cast<uint8>( slot + 1 ) );
        }
        else
        {
            if( slot + 1 == mMaxModifiedUavPlusOne )
            {
                --mMaxModifiedUavPlusOne;
                while( mMaxModifiedUavPlusOne != 0 && !mUavs[mMaxModifiedUavPlusOne-1] )
                    --mMaxModifiedUavPlusOne;
            }
        }

        if( oldUav )
        {
            oldUav->Release();
            oldUav = 0;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::queueBindUAV( uint32 slot, UavBufferPacked *buffer,
                                          ResourceAccess::ResourceAccess access,
                                          size_t offset, size_t sizeBytes )
    {
        assert( slot < 64 );

        if( mUavTexPtr[slot].isNull() && !mUavBuffers[slot] && !buffer )
            return;

        mUavsDirty = true;

        if( mUavBuffers[slot] )
        {
            //If the UAV view belonged to a buffer, don't decrement the reference count.
            mUavs[slot] = 0;
        }

        mUavBuffers[slot] = buffer;
        mUavTexPtr[slot].setNull();

        //Release oldUav *after* we've created the new UAV (if D3D11 needs
        //to return the same UAV, if we release it earlier we may cause
        //unnecessary alloc/deallocations)
        ID3D11UnorderedAccessView *oldUav = mUavs[slot];
        mUavs[slot] = 0;

        if( buffer )
        {
            assert( dynamic_cast<D3D11UavBufferPacked*>( buffer ) );
            D3D11UavBufferPacked *uavBufferPacked = static_cast<D3D11UavBufferPacked*>( buffer );

            mUavs[slot] = uavBufferPacked->_bindBufferCommon( offset, sizeBytes );

            mMaxModifiedUavPlusOne = std::max( mMaxModifiedUavPlusOne, static_cast<uint8>( slot + 1 ) );
        }
        else
        {
            if( slot + 1 == mMaxModifiedUavPlusOne )
            {
                --mMaxModifiedUavPlusOne;
                while( mMaxModifiedUavPlusOne != 0 && !mUavs[mMaxModifiedUavPlusOne-1] )
                    --mMaxModifiedUavPlusOne;
            }
        }

        if( oldUav )
        {
            oldUav->Release();
            oldUav = 0;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::clearUAVs(void)
    {
        mUavsDirty = true;

        for( size_t i=0; i<64; ++i )
        {
            mUavTexPtr[i].setNull();

            if( mUavs[i] )
            {
                //If the UAV view belonged to a buffer, don't decrement the reference count.
                if( !mUavBuffers[i] )
                    mUavs[i]->Release();
                mUavs[i] = 0;
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::flushUAVs(void)
    {
        mUavsDirty = false;

        uint8 flags = VP_RTT_COLOUR_WRITE;
        if( mActiveViewport )
            flags = mActiveViewport->getViewportRenderTargetFlags();
        _setRenderTargetViews( flags );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_bindTextureUavCS( uint32 slot, Texture *texture,
                                               ResourceAccess::ResourceAccess access,
                                               int32 mipmapLevel, int32 textureArrayIndex,
                                               PixelFormat pixelFormat )
    {
        if( texture )
        {
            D3D11Texture *dt = static_cast<D3D11Texture*>( texture );
            ID3D11UnorderedAccessView *uavView = dt->getUavView( mipmapLevel, textureArrayIndex, pixelFormat );
            mDevice.GetImmediateContext()->CSSetUnorderedAccessViews( slot, 1, &uavView, NULL );

            mMaxBoundUavCS = std::max( mMaxBoundUavCS, slot );
        }
        else
        {
            ID3D11UnorderedAccessView *nullUavView = NULL;
            mDevice.GetImmediateContext()->CSSetUnorderedAccessViews( slot, 1, &nullUavView, NULL );
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureCS( uint32 slot, bool enabled, Texture *texPtr )
    {
        D3D11Texture *dt = static_cast<D3D11Texture*>( texPtr );
        if (enabled && dt && dt->getSize() > 0)
        {
            // note used
            dt->touch();
            ID3D11ShaderResourceView * pTex = dt->getTexture();

            mDevice.GetImmediateContext()->CSSetShaderResources(static_cast<UINT>(slot), static_cast<UINT>(1), &pTex);
        }
        else
        {
            ID3D11ShaderResourceView *nullSrv = NULL;
            mDevice.GetImmediateContext()->CSSetShaderResources(static_cast<UINT>(slot), static_cast<UINT>(1), &nullSrv);
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setHlmsSamplerblockCS( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
        assert( samplerblock->mRsData &&
                "The block must have been created via HlmsManager::getSamplerblock!" );

        ID3D11SamplerState *samplerState = reinterpret_cast<ID3D11SamplerState*>( samplerblock->mRsData );

        mDevice.GetImmediateContext()->CSSetSamplers( static_cast<UINT>(texUnit), static_cast<UINT>(1),
                                                      &samplerState );
        if( mDevice.isError() )
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "D3D11 device cannot set pixel shader samplers\nError Description:" + errorDescription,
                "D3D11RenderSystem::_render");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsPipelineStateObjectCreated( HlmsPso *block )
    {
        D3D11HlmsPso *pso = new D3D11HlmsPso();
        memset( pso, 0, sizeof(D3D11HlmsPso) );

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

        ZeroMemory( &depthStencilDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
        depthStencilDesc.DepthEnable        = block->macroblock->mDepthCheck;
        depthStencilDesc.DepthWriteMask     =
                block->macroblock->mDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL :
                                                  D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc          = D3D11Mappings::get( block->macroblock->mDepthFunc );
        depthStencilDesc.StencilEnable      = block->pass.stencilParams.enabled;
        depthStencilDesc.StencilReadMask    = block->pass.stencilParams.readMask;
        depthStencilDesc.StencilWriteMask   = block->pass.stencilParams.writeMask;
        const StencilStateOp &stateFront = block->pass.stencilParams.stencilFront;
        depthStencilDesc.FrontFace.StencilFunc          = D3D11Mappings::get( stateFront.compareOp );
        depthStencilDesc.FrontFace.StencilDepthFailOp   = D3D11Mappings::get( stateFront.stencilDepthFailOp );
        depthStencilDesc.FrontFace.StencilPassOp        = D3D11Mappings::get( stateFront.stencilPassOp );
        depthStencilDesc.FrontFace.StencilFailOp        = D3D11Mappings::get( stateFront.stencilFailOp );
        const StencilStateOp &stateBack = block->pass.stencilParams.stencilBack;
        depthStencilDesc.BackFace.StencilFunc           = D3D11Mappings::get( stateBack.compareOp );
        depthStencilDesc.BackFace.StencilDepthFailOp    = D3D11Mappings::get( stateBack.stencilDepthFailOp );
        depthStencilDesc.BackFace.StencilPassOp         = D3D11Mappings::get( stateBack.stencilPassOp );
        depthStencilDesc.BackFace.StencilFailOp         = D3D11Mappings::get( stateBack.stencilFailOp );

        HRESULT hr = mDevice->CreateDepthStencilState( &depthStencilDesc, &pso->depthStencilState );
        if( FAILED(hr) )
        {
            delete pso;
            pso = 0;

            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Failed to create depth stencil state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_hlmsPipelineStateObjectCreated" );
        }

        const bool useTesselation = !block->tesselationDomainShader.isNull();
        const bool useAdjacency   = !block->geometryShader.isNull() &&
                                    block->geometryShader->isAdjacencyInfoRequired();

        switch( block->operationType )
        {
        case OT_POINT_LIST:
            pso->topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case OT_LINE_LIST:
            if( useTesselation )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            else if( useAdjacency )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            else
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case OT_LINE_STRIP:
            if( useTesselation )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            else if( useAdjacency )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            else
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        default:
        case OT_TRIANGLE_LIST:
            if( useTesselation )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            else if( useAdjacency )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            else
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case OT_TRIANGLE_STRIP:
            if( useTesselation )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            else if( useAdjacency )
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            else
                pso->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case OT_TRIANGLE_FAN:
            pso->topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

            delete pso;
            pso = 0;
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "Error - DX11 render - no support for triangle fan (OT_TRIANGLE_FAN)",
                         "D3D11RenderSystem::_hlmsPipelineStateObjectCreated" );
            break;
        }

        //No subroutines for now
        if( !block->vertexShader.isNull() )
        {
            pso->vertexShader = static_cast<D3D11HLSLProgram*>( block->vertexShader->
                                                                _getBindingDelegate() );
        }
        if( !block->geometryShader.isNull() )
        {
            pso->geometryShader = static_cast<D3D11HLSLProgram*>( block->geometryShader->
                                                                  _getBindingDelegate() );
        }
        if( mFeatureLevel >= D3D_FEATURE_LEVEL_11_0 )
        {
            if( !block->tesselationHullShader.isNull() )
            {
                pso->hullShader = static_cast<D3D11HLSLProgram*>( block->tesselationHullShader->
                                                                  _getBindingDelegate() );
            }
            if( !block->tesselationDomainShader.isNull() )
            {
                pso->domainShader = static_cast<D3D11HLSLProgram*>( block->tesselationDomainShader->
                                                                    _getBindingDelegate() );
            }

            // Check consistency of tessellation shaders
            if( block->tesselationHullShader.isNull() != block->tesselationDomainShader.isNull() )
            {
                delete pso;
                pso = 0;
                if( block->tesselationHullShader.isNull() )
                {
                    OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                 "Attempted to use tessellation, but domain shader is missing",
                                 "D3D11RenderSystem::_hlmsPipelineStateObjectCreated" );
                }
                else
                {
                    OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                 "Attempted to use tessellation, but hull shader is missing",
                                 "D3D11RenderSystem::_hlmsPipelineStateObjectCreated" );
                }
            }
        }
        if( !block->pixelShader.isNull() )
        {
            pso->pixelShader = static_cast<D3D11HLSLProgram*>( block->pixelShader->
                                                               _getBindingDelegate() );
        }

        if( pso->vertexShader )
        {
            try
            {
                pso->inputLayout = pso->vertexShader->getLayoutForPso( block->vertexElements );
            }
            catch( Exception &e )
            {
                delete pso;
                pso = 0;
                throw e;
            }
        }

        block->rsData = pso;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsPipelineStateObjectDestroyed( HlmsPso *pso )
    {
        D3D11HlmsPso *d3dPso = reinterpret_cast<D3D11HlmsPso*>( pso->rsData );
        d3dPso->depthStencilState->Release();
        d3dPso->inputLayout->Release();
        delete d3dPso;
        pso->rsData = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsMacroblockCreated( HlmsMacroblock *newBlock )
    {
        D3D11_RASTERIZER_DESC rasterDesc;
        switch( newBlock->mCullMode )
        {
        case CULL_NONE:
            rasterDesc.CullMode = D3D11_CULL_NONE;
            break;
        default:
        case CULL_CLOCKWISE:
            rasterDesc.CullMode = D3D11_CULL_BACK;
            break;
        case CULL_ANTICLOCKWISE:
            rasterDesc.CullMode = D3D11_CULL_FRONT;
            break;
        }

        // This should/will be done in a geometry shader like in the FixedFuncEMU sample and the shader needs solid
        rasterDesc.FillMode = newBlock->mPolygonMode == PM_WIREFRAME ? D3D11_FILL_WIREFRAME :
                                                                       D3D11_FILL_SOLID;

        rasterDesc.FrontCounterClockwise = true;

        const float nearFarFactor = 10.0;
        rasterDesc.DepthBias        = static_cast<int>(-nearFarFactor * newBlock->mDepthBiasConstant);
        rasterDesc.DepthBiasClamp   = 0;
        rasterDesc.SlopeScaledDepthBias = newBlock->mDepthBiasSlopeScale;

        rasterDesc.DepthClipEnable  = true;
        rasterDesc.ScissorEnable    = newBlock->mScissorTestEnabled;

        rasterDesc.MultisampleEnable     = true;
        rasterDesc.AntialiasedLineEnable = false;

        ID3D11RasterizerState *rasterizerState = 0;

        HRESULT hr = mDevice->CreateRasterizerState( &rasterDesc, &rasterizerState );
        if( FAILED(hr) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Failed to create rasterizer state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_hlmsMacroblockCreated" );
        }

        newBlock->mRsData = rasterizerState;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsMacroblockDestroyed( HlmsMacroblock *block )
    {
        ID3D11RasterizerState *rasterizerState = reinterpret_cast<ID3D11RasterizerState*>(
                                                                        block->mRsData );
        rasterizerState->Release();
        block->mRsData = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsBlendblockCreated( HlmsBlendblock *newBlock )
    {
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory( &blendDesc, sizeof(D3D11_BLEND_DESC) );
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = newBlock->mBlendOperation;

        blendDesc.RenderTarget[0].RenderTargetWriteMask = newBlock->mBlendChannelMask;

        if( newBlock->mSeparateBlend )
        {
            if( newBlock->mSourceBlendFactor == SBF_ONE &&
                newBlock->mDestBlendFactor == SBF_ZERO &&
                newBlock->mSourceBlendFactorAlpha == SBF_ONE &&
                newBlock->mDestBlendFactorAlpha == SBF_ZERO )
            {
                blendDesc.RenderTarget[0].BlendEnable = FALSE;
            }
            else
            {
                blendDesc.RenderTarget[0].BlendEnable = TRUE;
                blendDesc.RenderTarget[0].SrcBlend = D3D11Mappings::get(newBlock->mSourceBlendFactor, false);
                blendDesc.RenderTarget[0].DestBlend = D3D11Mappings::get(newBlock->mDestBlendFactor, false);
                blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11Mappings::get(newBlock->mSourceBlendFactorAlpha, true);
                blendDesc.RenderTarget[0].DestBlendAlpha = D3D11Mappings::get(newBlock->mDestBlendFactorAlpha, true);
                blendDesc.RenderTarget[0].BlendOp = blendDesc.RenderTarget[0].BlendOpAlpha =
                        D3D11Mappings::get( newBlock->mBlendOperation );

                blendDesc.RenderTarget[0].RenderTargetWriteMask = newBlock->mBlendChannelMask;
            }
        }
        else
        {
            if( newBlock->mSourceBlendFactor == SBF_ONE && newBlock->mDestBlendFactor == SBF_ZERO )
            {
                blendDesc.RenderTarget[0].BlendEnable = FALSE;
            }
            else
            {
                blendDesc.RenderTarget[0].BlendEnable = TRUE;
                blendDesc.RenderTarget[0].SrcBlend = D3D11Mappings::get(newBlock->mSourceBlendFactor, false);
                blendDesc.RenderTarget[0].DestBlend = D3D11Mappings::get(newBlock->mDestBlendFactor, false);
                blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11Mappings::get(newBlock->mSourceBlendFactor, true);
                blendDesc.RenderTarget[0].DestBlendAlpha = D3D11Mappings::get(newBlock->mDestBlendFactor, true);
                blendDesc.RenderTarget[0].BlendOp = D3D11Mappings::get( newBlock->mBlendOperation );
                blendDesc.RenderTarget[0].BlendOpAlpha = D3D11Mappings::get( newBlock->mBlendOperationAlpha );

                blendDesc.RenderTarget[0].RenderTargetWriteMask = newBlock->mBlendChannelMask;
            }
        }

        // feature level 9 and below does not support alpha to coverage.
        if (mFeatureLevel < D3D_FEATURE_LEVEL_10_0)
            blendDesc.AlphaToCoverageEnable = false;
        else
            blendDesc.AlphaToCoverageEnable = newBlock->mAlphaToCoverageEnabled;

        ID3D11BlendState *blendState = 0;

        HRESULT hr = mDevice->CreateBlendState( &blendDesc, &blendState );
        if( FAILED(hr) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Failed to create blend state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_hlmsBlendblockCreated" );
        }

        newBlock->mRsData = blendState;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsBlendblockDestroyed( HlmsBlendblock *block )
    {
        ID3D11BlendState *blendState = reinterpret_cast<ID3D11BlendState*>( block->mRsData );
        blendState->Release();
        block->mRsData = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsSamplerblockCreated( HlmsSamplerblock *newBlock )
    {
        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory( &samplerDesc, sizeof(D3D11_SAMPLER_DESC) );
        samplerDesc.Filter = D3D11Mappings::get( newBlock->mMinFilter, newBlock->mMagFilter,
                                                 newBlock->mMipFilter,
                                                 newBlock->mCompareFunction != NUM_COMPARE_FUNCTIONS );
        if( newBlock->mCompareFunction == NUM_COMPARE_FUNCTIONS )
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        else
            samplerDesc.ComparisonFunc = D3D11Mappings::get( newBlock->mCompareFunction );

        samplerDesc.AddressU = D3D11Mappings::get( newBlock->mU );
        samplerDesc.AddressV = D3D11Mappings::get( newBlock->mV );
        samplerDesc.AddressW = D3D11Mappings::get( newBlock->mW );

        samplerDesc.MipLODBias      = newBlock->mMipLodBias;
        samplerDesc.MaxAnisotropy   = static_cast<UINT>( newBlock->mMaxAnisotropy );
        samplerDesc.BorderColor[0]  = newBlock->mBorderColour.r;
        samplerDesc.BorderColor[1]  = newBlock->mBorderColour.g;
        samplerDesc.BorderColor[2]  = newBlock->mBorderColour.b;
        samplerDesc.BorderColor[3]  = newBlock->mBorderColour.a;
        samplerDesc.MinLOD          = newBlock->mMinLod;
        samplerDesc.MaxLOD          = newBlock->mMaxLod;

        ID3D11SamplerState *samplerState = 0;

        HRESULT hr = mDevice->CreateSamplerState( &samplerDesc, &samplerState ) ;
        if( FAILED(hr) )
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                "Failed to create sampler state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_hlmsSamplerblockCreated" );
        }

        newBlock->mRsData = samplerState;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsSamplerblockDestroyed( HlmsSamplerblock *block )
    {
        ID3D11SamplerState *samplerState = reinterpret_cast<ID3D11SamplerState*>( block->mRsData );
        samplerState->Release();
        block->mRsData = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setHlmsMacroblock( const HlmsMacroblock *macroblock )
    {
        assert( macroblock->mRsData &&
                "The block must have been created via HlmsManager::getMacroblock!" );

        ID3D11RasterizerState *rasterizerState = reinterpret_cast<ID3D11RasterizerState*>(
                                                                        macroblock->mRsData );

        mDevice.GetImmediateContext()->RSSetState( rasterizerState );
        if( mDevice.isError() )
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "D3D11 device cannot set rasterizer state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_setHlmsMacroblock");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setHlmsBlendblock( const HlmsBlendblock *blendblock )
    {
        assert( blendblock->mRsData &&
                "The block must have been created via HlmsManager::getBlendblock!" );

        ID3D11BlendState *blendState = reinterpret_cast<ID3D11BlendState*>( blendblock->mRsData );

        // TODO - Add this functionality to Ogre (what's the GL equivalent?)
        mDevice.GetImmediateContext()->OMSetBlendState( blendState, 0, 0xffffffff );
        if( mDevice.isError() )
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "D3D11 device cannot set blend state\nError Description: " + errorDescription,
                "D3D11RenderSystem::_setHlmsBlendblock");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setHlmsSamplerblock( uint8 texUnit, const HlmsSamplerblock *samplerblock )
    {
        assert( samplerblock->mRsData &&
                "The block must have been created via HlmsManager::getSamplerblock!" );

        ID3D11SamplerState *samplerState = reinterpret_cast<ID3D11SamplerState*>( samplerblock->mRsData );

        //TODO: Refactor Ogre to:
        //  a. Separate samplerblocks from textures (GL can emulate the merge).
        //  b. Set all of them at once.
        mDevice.GetImmediateContext()->VSSetSamplers( static_cast<UINT>(texUnit), static_cast<UINT>(1),
                                                      &samplerState );
        mDevice.GetImmediateContext()->PSSetSamplers( static_cast<UINT>(texUnit), static_cast<UINT>(1),
                                                      &samplerState );
        if( mDevice.isError() )
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "D3D11 device cannot set pixel shader samplers\nError Description:" + errorDescription,
                "D3D11RenderSystem::_render");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setPipelineStateObject( const HlmsPso *pso )
    {
        RenderSystem::_setPipelineStateObject( pso );

        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        //deviceContext->IASetInputLayout( 0 );
        deviceContext->VSSetShader( 0, 0, 0 );
        deviceContext->GSSetShader( 0, 0, 0 );
        deviceContext->HSSetShader( 0, 0, 0 );
        deviceContext->DSSetShader( 0, 0, 0 );
        deviceContext->PSSetShader( 0, 0, 0 );
        deviceContext->CSSetShader( 0, 0, 0 );

        if( !pso )
            return;

        _setHlmsMacroblock( pso->macroblock );
        _setHlmsBlendblock( pso->blendblock );

        D3D11HlmsPso *d3dPso = reinterpret_cast<D3D11HlmsPso*>( pso->rsData );

        mPso = d3dPso;

        deviceContext->OMSetDepthStencilState( d3dPso->depthStencilState, mStencilRef );
        deviceContext->IASetPrimitiveTopology( d3dPso->topology );
        deviceContext->IASetInputLayout( d3dPso->inputLayout );

        if( d3dPso->vertexShader )
        {
            deviceContext->VSSetShader( d3dPso->vertexShader->getVertexShader(), 0, 0 );
            mVertexProgramBound = true;
        }

        if( d3dPso->geometryShader )
        {
            deviceContext->GSSetShader( d3dPso->geometryShader->getGeometryShader(), 0, 0 );
            mGeometryProgramBound = true;
        }

        if( mFeatureLevel >= D3D_FEATURE_LEVEL_11_0 )
        {
            if( d3dPso->hullShader )
            {
                deviceContext->HSSetShader( d3dPso->hullShader->getHullShader(), 0, 0 );
                mTessellationHullProgramBound = true;
            }

            if( d3dPso->domainShader )
            {
                deviceContext->DSSetShader( d3dPso->domainShader->getDomainShader(), 0, 0 );
                mTessellationDomainProgramBound = true;
            }
        }

        if( d3dPso->pixelShader )
        {
            deviceContext->PSSetShader( d3dPso->pixelShader->getPixelShader(), 0, 0 );
            mFragmentProgramBound = true;
        }

        if (mDevice.isError())
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "D3D11 device cannot set shaders\nError Description: " +
                         errorDescription, "D3D11RenderSystem::_setPipelineStateObject" );
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setIndirectBuffer( IndirectBufferPacked *indirectBuffer )
    {
        if( mVaoManager->supportsIndirectBuffers() )
        {
            if( mBoundIndirectBuffer )
            {
                D3D11BufferInterface *bufferInterface = static_cast<D3D11BufferInterface*>(
                                                        indirectBuffer->getBufferInterface() );
                mBoundIndirectBuffer = bufferInterface->getVboName();
            }
            else
            {
                mBoundIndirectBuffer = 0;
            }
        }
        else
        {
            if( indirectBuffer )
                mSwIndirectBufferPtr = indirectBuffer->getSwBufferPtr();
            else
                mSwIndirectBufferPtr = 0;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsComputePipelineStateObjectCreated( HlmsComputePso *newPso )
    {
        newPso->rsData = reinterpret_cast<void*>( static_cast<D3D11HLSLProgram*>(
                                                      newPso->computeShader->_getBindingDelegate() ) );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_hlmsComputePipelineStateObjectDestroyed( HlmsComputePso *newPso )
    {
        newPso->rsData = 0;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setComputePso( const HlmsComputePso *pso )
    {
        D3D11HLSLProgram *newComputeShader = 0;

        if( pso )
        {
            newComputeShader = reinterpret_cast<D3D11HLSLProgram*>( pso->rsData );

            {
                //Using Compute Shaders? Unset the UAV from rendering
                mDevice.GetImmediateContext()->OMSetRenderTargets( 0, 0, 0 );
                mUavsDirty = true;
            }

            if( mBoundComputeProgram == newComputeShader )
                return;
        }

        RenderSystem::_setPipelineStateObject( (HlmsPso*)0 );

        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        //deviceContext->IASetInputLayout( 0 );
        deviceContext->VSSetShader( 0, 0, 0 );
        deviceContext->GSSetShader( 0, 0, 0 );
        deviceContext->HSSetShader( 0, 0, 0 );
        deviceContext->DSSetShader( 0, 0, 0 );
        deviceContext->PSSetShader( 0, 0, 0 );
        deviceContext->CSSetShader( 0, 0, 0 );

        if( !pso )
            return;

        mBoundComputeProgram = newComputeShader;

        deviceContext->CSSetShader( mBoundComputeProgram->getComputeShader(), 0, 0 );
        mActiveComputeGpuProgramParameters = pso->computeParams;
        mComputeProgramBound = true;

        if (mDevice.isError())
        {
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                         "D3D11 device cannot set shaders\nError Description: " +
                         errorDescription, "D3D11RenderSystem::_setComputePso" );
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_beginFrame()
    {
        mHardwareBufferManager->_updateDirtyInputLayouts();
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_endFrame()
    {
        mBoundComputeProgram = 0;
        mActiveComputeGpuProgramParameters.setNull();
        mComputeProgramBound = false;
    }
    //---------------------------------------------------------------------
    // TODO: Move this class to the right place.
    class D3D11RenderOperationState
    {
    public:
        ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS];
        size_t mTexturesCount;

        D3D11RenderOperationState() :
            mTexturesCount(0)
        {
            for (size_t i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
            {
                mTextures[i] = 0;
            }
        }


        ~D3D11RenderOperationState()
        {
        }
    };

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render(const v1::RenderOperation& op)
    {

        // Exit immediately if there is nothing to render
        if (op.vertexData==0 || op.vertexData->vertexCount == 0)
        {
            return;
        }

        v1::HardwareVertexBufferSharedPtr globalInstanceVertexBuffer = getGlobalInstanceVertexBuffer();
        v1::VertexDeclaration* globalVertexDeclaration = getGlobalInstanceVertexBufferVertexDeclaration();

        bool hasInstanceData = (op.useGlobalInstancingVertexBufferIsAvailable &&
                    !globalInstanceVertexBuffer.isNull() && globalVertexDeclaration != NULL)
                || op.vertexData->vertexBufferBinding->getHasInstanceData();

        size_t numberOfInstances = op.numberOfInstances;

        if (op.useGlobalInstancingVertexBufferIsAvailable)
        {
            numberOfInstances *= getGlobalNumberOfInstances();
        }

        // Call super class
        RenderSystem::_render(op);
        
        D3D11RenderOperationState stackOpState;
        D3D11RenderOperationState * opState = &stackOpState;

        if(mSamplerStatesChanged)
		{
            // samplers mapping
            const size_t numberOfSamplers = std::min( mLastTextureUnitState,
                                                      (size_t)(OGRE_MAX_TEXTURE_LAYERS + 1) );
            opState->mTexturesCount = numberOfSamplers;
                            
            for (size_t n = 0; n < numberOfSamplers; n++)
            {
                ID3D11SamplerState * samplerState  = NULL;
                ID3D11ShaderResourceView *texture = NULL;
                sD3DTextureStageDesc & stage = mTexStageDesc[n];
                if(!stage.used)
                {
                    samplerState    = NULL;
                    texture         = NULL;
                }
                else
                {
                    texture = stage.pTex;
                }
                opState->mTextures[n]       = texture;
            }
            for (size_t n = opState->mTexturesCount; n < OGRE_MAX_TEXTURE_LAYERS; n++)
			{
				opState->mTextures[n] = NULL;
			}
        }

        if (mSamplerStatesChanged && opState->mTexturesCount > 0 ) //  if the NumTextures is 0, the operation effectively does nothing.
        {
            mSamplerStatesChanged = false; // now it's time to set it to false
            /// Pixel Shader binding
            {
                mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                if (mDevice.isError())
                {
                    String errorDescription = mDevice.getErrorDescription();
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
                        "D3D11RenderSystem::_render");
                }
            }
            
            /// Vertex Shader binding
			
			/*if (mBindingType == TextureUnitState::BindingType::BT_VERTEX)*/
			
            {
                if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
                {
                    mDevice.GetImmediateContext()->VSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }
                }
            }

            /// Hull Shader binding
            if (mPso->hullShader && mBindingType == TextureUnitState::BT_TESSELLATION_HULL)
            {
                if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
                {
                    mDevice.GetImmediateContext()->HSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set hull shader resources\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }
                }
            }
            
            /// Domain Shader binding
            if (mPso->domainShader && mBindingType == TextureUnitState::BT_TESSELLATION_DOMAIN)
            {
                if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
                {
                    mDevice.GetImmediateContext()->DSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set domain shader resources\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }
                }
            }
        }

        ID3D11Buffer* pSOTarget=0;
        // Mustn't bind a emulated vertex, pixel shader (see below), if we are rendering to a stream out buffer
        mDevice.GetImmediateContext()->SOGetTargets(1, &pSOTarget);

        //check consistency of vertex-fragment shaders
        if (!mPso->vertexShader ||
             (!mPso->pixelShader && op.operationType != OT_POINT_LIST && pSOTarget==0 )
           ) 
        {
            
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to render to a D3D11 device without both vertex and fragment shaders there is no fixed pipeline in d3d11 - use the RTSS or write custom shaders.",
                "D3D11RenderSystem::_render");
        }

        // Check consistency of tessellation shaders
        if( (mPso->hullShader && !mPso->domainShader) ||
            (!mPso->hullShader && mPso->domainShader) )
        {
            if (mPso->hullShader && !mPso->domainShader) {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to use tessellation, but domain shader is missing",
                "D3D11RenderSystem::_render");
            }
            else {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to use tessellation, but hull shader is missing",
                "D3D11RenderSystem::_render"); }
        }

        if (mDevice.isError())
        {
            // this will never happen but we want to be consistent with the error checks... 
            String errorDescription = mDevice.getErrorDescription();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "D3D11 device cannot set geometry shader to null\nError Description:" + errorDescription,
                "D3D11RenderSystem::_render");
        }

        // Determine rendering operation
        D3D11_PRIMITIVE_TOPOLOGY primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        DWORD primCount = 0;

        // Handle computing
        if(mPso->hullShader && mPso->domainShader)
        {
            // useful primitives for tessellation
            switch( op.operationType )
            {
            case OT_LINE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
                break;

            case OT_LINE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
                break;

            case OT_TRIANGLE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
                break;

            case OT_TRIANGLE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;
            }
        }
        else
        {
            //rendering without tessellation.
            bool useAdjacency = (mGeometryProgramBound && mPso->geometryShader && mPso->geometryShader->isAdjacencyInfoRequired());
            switch( op.operationType )
            {
            case OT_POINT_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
                break;

            case OT_LINE_LIST:
                primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
                break;

            case OT_LINE_STRIP:
                primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
                break;

            case OT_TRIANGLE_LIST:
                primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
                break;

            case OT_TRIANGLE_STRIP:
                primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;

            case OT_TRIANGLE_FAN:
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error - DX11 render - no support for triangle fan (OT_TRIANGLE_FAN)", "D3D11RenderSystem::_render");
                primType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; // todo - no TRIANGLE_FAN in DX 11
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;
            }
        }
        
        if (primCount)
        {
            // Issue the op
            //HRESULT hr;
            if( op.useIndexes  )
            {
                v1::D3D11HardwareIndexBuffer* d3dIdxBuf =
                    static_cast<v1::D3D11HardwareIndexBuffer*>(op.indexData->indexBuffer.get());
                mDevice.GetImmediateContext()->IASetIndexBuffer( d3dIdxBuf->getD3DIndexBuffer(), D3D11Mappings::getFormat(d3dIdxBuf->getType()), 0 );
                if (mDevice.isError())
                {
                    String errorDescription = mDevice.getErrorDescription();
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "D3D11 device cannot set index buffer\nError Description:" + errorDescription,
                        "D3D11RenderSystem::_render");
                }

                do
                {
                    // do indexed draw operation
                    mDevice.GetImmediateContext()->IASetPrimitiveTopology( primType );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set primitive topology\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }
                    
                    if (hasInstanceData)
                    {
                        mDevice.GetImmediateContext()->DrawIndexedInstanced(    
                            static_cast<UINT>(op.indexData->indexCount), 
                            static_cast<UINT>(numberOfInstances), 
                            static_cast<UINT>(op.indexData->indexStart), 
                            static_cast<INT>(op.vertexData->vertexStart),
                            0
                            );
                        if (mDevice.isError())
                        {
                            String errorDescription = mDevice.getErrorDescription();
                            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                "D3D11 device cannot draw indexed instanced\nError Description:" + errorDescription,
                                "D3D11RenderSystem::_render");
                        }
                    }
                    else
                    {
                        mDevice.GetImmediateContext()->DrawIndexed(    
                            static_cast<UINT>(op.indexData->indexCount), 
                            static_cast<UINT>(op.indexData->indexStart), 
                            static_cast<INT>(op.vertexData->vertexStart)
                            );
                        if (mDevice.isError())
                        {
                            String errorDescription = mDevice.getErrorDescription();
                            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                "D3D11 device cannot draw indexed\nError Description:" + errorDescription +
                                "Active OGRE vertex shader name: " + mPso->vertexShader->getName() +
                                "\nActive OGRE fragment shader name: " + mPso->pixelShader->getName(),
                                "D3D11RenderSystem::_render");
                        }
                    }
                } while (updatePassIterationRenderState());
            }
            else
            {
                // nfz: gpu_iterate
                do
                {
                    // Unindexed, a little simpler!
                    mDevice.GetImmediateContext()->IASetPrimitiveTopology( primType );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set primitive topology\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }       
                    
                    if (op.vertexData->vertexCount == -1) // -1 is a sign to use DrawAuto
                    {
                        mDevice.GetImmediateContext()->DrawAuto(); 
                    }
                    else
                    {
                        if (hasInstanceData)
                        {
                            mDevice.GetImmediateContext()->DrawInstanced(
                                static_cast<UINT>(numberOfInstances), 
                                static_cast<UINT>(op.vertexData->vertexCount), 
                                static_cast<INT>(op.vertexData->vertexStart),
                                0
                                ); 
                        }
                        else
                        {
                            mDevice.GetImmediateContext()->Draw(
                                static_cast<UINT>(op.vertexData->vertexCount), 
                                static_cast<INT>(op.vertexData->vertexStart)
                                ); 
                        }
                    }
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot draw\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_render");
                    }       


                } while (updatePassIterationRenderState());
            } 
        }


        // Crashy : commented this, 99% sure it's useless but really time consuming
        /*if (true) // for now - clear the render state
        {
            mDevice.GetImmediateContext()->OMSetBlendState(0, 0, 0xffffffff); 
            mDevice.GetImmediateContext()->RSSetState(0);
            mDevice.GetImmediateContext()->OMSetDepthStencilState(0, 0); 
//          mDevice->PSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(0), 0);
            
            // Clear class instance storage
            memset(mClassInstances, 0, sizeof(mClassInstances));
            memset(mNumClassInstances, 0, sizeof(mNumClassInstances));      
        }*/

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_dispatch( const HlmsComputePso &pso )
    {
        mDevice.GetImmediateContext()->Dispatch( pso.mNumThreadGroups[0],
                                                 pso.mNumThreadGroups[1],
                                                 pso.mNumThreadGroups[2] );

        assert( mMaxBoundUavCS < 8u );
        ID3D11UnorderedAccessView *nullUavViews[8];
        memset( nullUavViews, 0, sizeof( nullUavViews ) );
        mDevice.GetImmediateContext()->CSSetUnorderedAccessViews( 0, mMaxBoundUavCS + 1u,
                                                                  nullUavViews, NULL );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setVertexArrayObject( const VertexArrayObject *_vao )
    {
        const D3D11VertexArrayObject *vao = static_cast<const D3D11VertexArrayObject*>( _vao );
        D3D11VertexArrayObjectShared *sharedData = vao->mSharedData;

        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        deviceContext->IASetVertexBuffers( 0, vao->mVertexBuffers.size() + 1, //+1 due to DrawId
                                           sharedData->mVertexBuffers,
                                           sharedData->mStrides,
                                           sharedData->mOffsets );
        deviceContext->IASetIndexBuffer( sharedData->mIndexBuffer, sharedData->mIndexFormat, 0 );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render( const CbDrawCallIndexed *cmd )
    {
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        UINT indirectBufferOffset = reinterpret_cast<UINT>(cmd->indirectBufferOffset);
        for( uint32 i=cmd->numDraws; i--; )
        {
            deviceContext->DrawIndexedInstancedIndirect( mBoundIndirectBuffer, indirectBufferOffset );

            indirectBufferOffset += sizeof( CbDrawIndexed );
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render( const CbDrawCallStrip *cmd )
    {
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        UINT indirectBufferOffset = reinterpret_cast<UINT>(cmd->indirectBufferOffset);
        for( uint32 i=cmd->numDraws; i--; )
        {
            deviceContext->DrawInstancedIndirect( mBoundIndirectBuffer, indirectBufferOffset );

            indirectBufferOffset += sizeof( CbDrawStrip );
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_renderEmulated( const CbDrawCallIndexed *cmd )
    {
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        CbDrawIndexed *drawCmd = reinterpret_cast<CbDrawIndexed*>(
                                    mSwIndirectBufferPtr + (size_t)cmd->indirectBufferOffset );

        for( uint32 i=cmd->numDraws; i--; )
        {
            deviceContext->DrawIndexedInstanced( drawCmd->primCount,
                                                 drawCmd->instanceCount,
                                                 drawCmd->firstVertexIndex,
                                                 drawCmd->baseVertex,
                                                 drawCmd->baseInstance );
            ++drawCmd;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_renderEmulated( const CbDrawCallStrip *cmd )
    {
        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        CbDrawStrip *drawCmd = reinterpret_cast<CbDrawStrip*>(
                                    mSwIndirectBufferPtr + (size_t)cmd->indirectBufferOffset );

        for( uint32 i=cmd->numDraws; i--; )
        {
            deviceContext->DrawInstanced( drawCmd->primCount,
                                          drawCmd->instanceCount,
                                          drawCmd->firstVertexIndex,
                                          drawCmd->baseInstance );
            ++drawCmd;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setRenderOperation( const v1::CbRenderOp *cmd )
    {
        mCurrentVertexBuffer    = cmd->vertexData;
        mCurrentIndexBuffer     = cmd->indexData;

        ID3D11DeviceContextN *deviceContext = mDevice.GetImmediateContext();

        if( cmd->indexData )
        {
            v1::D3D11HardwareIndexBuffer* indexBuffer =
                    static_cast<v1::D3D11HardwareIndexBuffer*>( cmd->indexData->indexBuffer.get() );
            deviceContext->IASetIndexBuffer( indexBuffer->getD3DIndexBuffer(),
                                             D3D11Mappings::getFormat( indexBuffer->getType() ),
                                             0 );
        }
        else
        {
            deviceContext->IASetIndexBuffer( 0, DXGI_FORMAT_UNKNOWN, 0 );
        }

        uint32 usedSlots = 0;
        const v1::VertexBufferBinding::VertexBufferBindingMap& binds =
                cmd->vertexData->vertexBufferBinding->getBindings();
        v1::VertexBufferBinding::VertexBufferBindingMap::const_iterator i, iend;
        iend = binds.end();
        for (i = binds.begin(); i != iend; ++i)
        {
            const v1::D3D11HardwareVertexBuffer* d3d11buf =
                static_cast<const v1::D3D11HardwareVertexBuffer*>(i->second.get());

            UINT stride = static_cast<UINT>(d3d11buf->getVertexSize());
            UINT offset = 0; // no stream offset, this is handled in _render instead
            UINT slot = static_cast<UINT>(i->first);
            ID3D11Buffer * pVertexBuffers = d3d11buf->getD3DVertexBuffer();
            mDevice.GetImmediateContext()->IASetVertexBuffers(
                slot, // The first input slot for binding.
                1, // The number of vertex buffers in the array.
                &pVertexBuffers,
                &stride,
                &offset
                );

            if (mDevice.isError())
            {
                String errorDescription = mDevice.getErrorDescription();
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                    "D3D11 device cannot set vertex buffers\nError Description:" + errorDescription,
                    "D3D11RenderSystem::setVertexBufferBinding");
            }

            ++usedSlots;
        }

        static_cast<D3D11VaoManager*>(mVaoManager)->bindDrawId( usedSlots );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render( const v1::CbDrawCallIndexed *cmd )
    {
        mDevice.GetImmediateContext()->DrawIndexedInstanced(
            cmd->primCount,
            cmd->instanceCount,
            cmd->firstVertexIndex,
            static_cast<INT>(mCurrentVertexBuffer->vertexStart),
            cmd->baseInstance );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render( const v1::CbDrawCallStrip *cmd )
    {
        mDevice.GetImmediateContext()->DrawInstanced(
                    cmd->primCount,
                    cmd->instanceCount,
                    cmd->firstVertexIndex,
                    cmd->baseInstance );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_renderUsingReadBackAsTexture(unsigned int passNr, Ogre::String variableName, unsigned int StartSlot)
    {
        RenderTarget *target = mActiveRenderTarget;
        switch (passNr)
        {
        case 1:
            if (target)
            {
                ID3D11RenderTargetView * pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
                memset(pRTView, 0, sizeof(pRTView));

                target->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );

                uint numberOfViews;
                target->getCustomAttribute( "numberOfViews", &numberOfViews );

                //Retrieve depth buffer
                D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

                // now switch to the new render target
                mDevice.GetImmediateContext()->OMSetRenderTargets(
                    numberOfViews,
                    pRTView,
                    depthBuffer->getDepthStencilView(0));

                if (mDevice.isError())
                {
                    String errorDescription = mDevice.getErrorDescription();
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "D3D11 device cannot set render target\nError Description:" + errorDescription,
                        "D3D11RenderSystem::_renderUsingReadBackAsTexture");
                }
                
                mDevice.GetImmediateContext()->ClearDepthStencilView(depthBuffer->getDepthStencilView(0), D3D11_CLEAR_DEPTH, 1.0f, 0);

                float ClearColor[4];
                //D3D11Mappings::get(colour, ClearColor);
                // Clear all views
                mActiveRenderTarget->getCustomAttribute( "numberOfViews", &numberOfViews );
                if( numberOfViews == 1 )
                    mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView[0], ClearColor );
                else
                {
                    for( uint i = 0; i < numberOfViews; ++i )
                        mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView[i], ClearColor );
                }

            }
            break;
        case 2:
            if (target)
            {
                //
                // We need to remove the the DST from the Render Targets if we want to use it as a texture :
                //
                ID3D11RenderTargetView * pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
                memset(pRTView, 0, sizeof(pRTView));

                target->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );

                uint numberOfViews;
                target->getCustomAttribute( "numberOfViews", &numberOfViews );

                //Retrieve depth buffer
                D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

                // now switch to the new render target
                mDevice.GetImmediateContext()->OMSetRenderTargets(
                    numberOfViews,
                    pRTView,
                    NULL);

                mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(StartSlot), static_cast<UINT>(numberOfViews), &mDSTResView);
                if (mDevice.isError())
                {
                    String errorDescription = mDevice.getErrorDescription();
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
                        "D3D11RenderSystem::_renderUsingReadBackAsTexture");
                }

            }
            break;
        case 3:
            //
            // We need to unbind mDSTResView from the given variable because this buffer
            // will be used later as the typical depth buffer, again
            // must call Apply(0) here : to flush SetResource(NULL)
            //
            
            if (target)
            {
                uint numberOfViews;
                target->getCustomAttribute( "numberOfViews", &numberOfViews );

                mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(StartSlot), static_cast<UINT>(numberOfViews), NULL);
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                            "D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
                            "D3D11RenderSystem::_renderUsingReadBackAsTexture");
                    }           
            }

            break;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setNormaliseNormals(bool normalise)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask)
    {
        if (mask & (uint16)GPV_GLOBAL)
        {
            // TODO: Dx11 supports shared constant buffers, so use them
            // check the match to constant buffers & use rendersystem data hooks to store
            // for now, just copy
            params->_copySharedParams();
        }

        // Do everything here in Dx11, since deal with via buffers anyway so number of calls
        // is actually the same whether we categorise the updates or not
        ID3D11Buffer* pBuffers[2];
        UINT slotStart, numBuffers;
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            if( mPso->vertexShader )
            {
                mPso->vertexShader->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->VSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set vertex shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            if( mPso->pixelShader )
            {
                mPso->pixelShader->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->PSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set pixel shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        case GPT_GEOMETRY_PROGRAM:
            if( mPso->geometryShader )
            {
                mPso->geometryShader->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->GSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set geometry shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        case GPT_HULL_PROGRAM:
            if( mPso->hullShader )
            {
                mPso->hullShader->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->HSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set hull shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        case GPT_DOMAIN_PROGRAM:
            if( mPso->domainShader )
            {
                mPso->domainShader->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->DSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set domain shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        case GPT_COMPUTE_PROGRAM:
            if( mBoundComputeProgram )
            {
                mBoundComputeProgram->getConstantBuffers( pBuffers, slotStart, numBuffers, params, mask );
                if( numBuffers > 0 )
                {
                    mDevice.GetImmediateContext()->CSSetConstantBuffers( slotStart, numBuffers,
                                                                         pBuffers );
                    if (mDevice.isError())
                    {
                        String errorDescription = mDevice.getErrorDescription();
                        OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                                     "D3D11 device cannot set compute shader constant buffers\n"
                                     "Error Description:" + errorDescription,
                                     "D3D11RenderSystem::bindGpuProgramParameters" );
                    }
                }
            }
            break;
        };

        // Now, set class instances
        const GpuProgramParameters::SubroutineMap& subroutineMap = params->getSubroutineMap();
        if (subroutineMap.empty())
            return;

        GpuProgramParameters::SubroutineIterator it;
        GpuProgramParameters::SubroutineIterator end = subroutineMap.end();
        for(it = subroutineMap.begin(); it != end; ++it)
        {
            setSubroutine(gptype, it->first, it->second);
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {

        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveVertexGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;

        case GPT_FRAGMENT_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveFragmentGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;
        case GPT_GEOMETRY_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveGeometryGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;
        case GPT_HULL_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveTessellationHullGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;
        case GPT_DOMAIN_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveTessellationDomainGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;
        case GPT_COMPUTE_PROGRAM:
            bindGpuProgramParameters(gptype, mActiveComputeGpuProgramParameters, (uint16)GPV_PASS_ITERATION_NUMBER);
            break;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setSubroutine(GpuProgramType gptype, unsigned int slotIndex, const String& subroutineName)
    {
        ID3D11ClassInstance* instance = 0;
        
        ClassInstanceIterator it = mInstanceMap.find(subroutineName);
        if (it == mInstanceMap.end())
        {
            // try to get instance already created (must have at least one field)
            HRESULT hr = mDevice.GetClassLinkage()->GetClassInstance(subroutineName.c_str(), 0, &instance);
            if (FAILED(hr) || instance == 0)
            {
                // probably class don't have a field, try create a new
                hr = mDevice.GetClassLinkage()->CreateClassInstance(subroutineName.c_str(), 0, 0, 0, 0, &instance);
                if (FAILED(hr) || instance == 0)
                {
					OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
						"Shader subroutine with name " + subroutineName + " doesn't exist.",
						"D3D11RenderSystem::setSubroutineName");
                }
            }

            // Store class instance
            mInstanceMap.insert(std::make_pair(subroutineName, instance));
        }
        else
        {
            instance = it->second;
        }
        
        // If already created, store class instance
        mClassInstances[gptype][slotIndex] = instance;
        mNumClassInstances[gptype] = mNumClassInstances[gptype] + 1;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setSubroutine(GpuProgramType gptype, const String& slotName, const String& subroutineName)
    {
        unsigned int slotIdx = 0;
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            {
                if (mPso->vertexShader)
                {
                    slotIdx = mPso->vertexShader->getSubroutineSlot(slotName);
                }
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            {
                if (mPso->pixelShader)
                {
                    slotIdx = mPso->pixelShader->getSubroutineSlot(slotName);
                }
            }
            break;
        case GPT_GEOMETRY_PROGRAM:
            {
                if (mPso->geometryShader)
                {
                    slotIdx = mPso->geometryShader->getSubroutineSlot(slotName);
                }
            }
            break;
        case GPT_HULL_PROGRAM:
            {
                if (mPso->hullShader)
                {
                    slotIdx = mPso->hullShader->getSubroutineSlot(slotName);
                }
            }
            break;
        case GPT_DOMAIN_PROGRAM:
            {
                if (mPso->domainShader)
                {
                    slotIdx = mPso->domainShader->getSubroutineSlot(slotName);
                }
            }
            break;
        case GPT_COMPUTE_PROGRAM:
            {
                if (mBoundComputeProgram)
                {
                    slotIdx = mBoundComputeProgram->getSubroutineSlot(slotName);
                }
            }
            break;
        };
        
        // Set subroutine for slot
        setSubroutine(gptype, slotIdx, subroutineName);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, Real depth, unsigned short stencil)
    {
        if (mActiveRenderTarget)
        {
            ID3D11RenderTargetView * pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            memset(pRTView, 0, sizeof(pRTView));

            mActiveRenderTarget->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );
            
            if (buffers & FBT_COLOUR)
            {
                float ClearColor[4];
                D3D11Mappings::get(colour, ClearColor);

                // Clear all views
                uint numberOfViews;
                mActiveRenderTarget->getCustomAttribute( "numberOfViews", &numberOfViews );
                for( uint i = 0; i < numberOfViews; ++i )
                    mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView[i], ClearColor );

            }
            UINT ClearFlags = 0;
            if (buffers & FBT_DEPTH)
            {
                ClearFlags |= D3D11_CLEAR_DEPTH;
            }
            if (buffers & FBT_STENCIL)
            {
                ClearFlags |= D3D11_CLEAR_STENCIL;
            }

            if (ClearFlags)
            {
                D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(
                                                    mActiveRenderTarget->getDepthBuffer());
                if( depthBuffer )
                {
                    mDevice.GetImmediateContext()->ClearDepthStencilView(
                                                        depthBuffer->getDepthStencilView(0),
                                                        ClearFlags, depth, static_cast<UINT8>(stencil) );
                }
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::discardFrameBuffer( unsigned int buffers )
    {
        //TODO: Use DX11.1 interfaces on Windows too.
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        if (mActiveRenderTarget)
        {
            ID3D11RenderTargetView * pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            memset(pRTView, 0, sizeof(pRTView));

            mActiveRenderTarget->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );

            if (buffers & FBT_COLOUR)
            {
                float ClearColor[4];
                D3D11Mappings::get(colour, ClearColor);

                // Clear all views
                uint numberOfViews;
                mActiveRenderTarget->getCustomAttribute( "numberOfViews", &numberOfViews );
                for( uint i = 0; i < numberOfViews; ++i )
                    mDevice.GetImmediateContext()->DiscardView( pRTView[i], ClearColor );
            }

            if( buffers & (FBT_DEPTH|FBT_STENCIL) )
            {
                D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(
                                                    mActiveRenderTarget-> getDepthBuffer() );
                if( depthBuffer )
                    mDevice.GetImmediateContext()->DiscardView( depthBuffer->getDepthStencilView() );
            }
        }
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_makeProjectionMatrix(Real left, Real right, 
        Real bottom, Real top, Real nearPlane, Real farPlane, Matrix4& dest,
        bool forGpuProgram)
    {
        // Correct position for off-axis projection matrix
        if (!forGpuProgram)
        {
            Real offsetX = left + right;
            Real offsetY = top + bottom;

            left -= offsetX;
            right -= offsetX;
            top -= offsetY;
            bottom -= offsetY;
        }

        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
            q = 1 - Frustum::INFINITE_FAR_PLANE_ADJUST;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 1);
        }
        else
        {
            q = farPlane / ( farPlane - nearPlane );
            qn = -q * nearPlane;
        }
        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        if (forGpuProgram)
        {
            dest[2][2] = -q;
            dest[3][2] = -1.0f;
        }
        else
        {
            dest[2][2] = q;
            dest[3][2] = 1.0f;
        }
        dest[2][3] = qn;
    }

    // ------------------------------------------------------------------
    void D3D11RenderSystem::setClipPlane (ushort index, Real A, Real B, Real C, Real D)
    {
    }

    // ------------------------------------------------------------------
    void D3D11RenderSystem::enableClipPlane (ushort index, bool enable)
    {
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* D3D11RenderSystem::createHardwareOcclusionQuery(void)
    {
        D3D11HardwareOcclusionQuery* ret = new D3D11HardwareOcclusionQuery (mDevice); 
        mHwOcclusionQueries.push_back(ret);
        return ret;
    }
    //---------------------------------------------------------------------
    Real D3D11RenderSystem::getHorizontalTexelOffset(void)
    {
        // D3D11 is now like GL
        return 0.0f;
    }
    //---------------------------------------------------------------------
    Real D3D11RenderSystem::getVerticalTexelOffset(void)
    {
        // D3D11 is now like GL
        return 0.0f;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
        bool forGpuProgram)
    {
        // Thanks to Eric Lenyel for posting this calculation at www.terathon.com

        // Calculate the clip-space corner point opposite the clipping plane
        // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
        // transform it into camera space by multiplying it
        // by the inverse of the projection matrix

        /* generalised version
        Vector4 q = matrix.inverse() * 
            Vector4(Math::Sign(plane.normal.x), Math::Sign(plane.normal.y), 1.0f, 1.0f);
        */
        Vector4 q;
        q.x = Math::Sign(plane.normal.x) / matrix[0][0];
        q.y = Math::Sign(plane.normal.y) / matrix[1][1];
        q.z = 1.0F; 
        // flip the next bit from Lengyel since we're right-handed
        if (forGpuProgram)
        {
            q.w = (1.0F - matrix[2][2]) / matrix[2][3];
        }
        else
        {
            q.w = (1.0F + matrix[2][2]) / matrix[2][3];
        }

        // Calculate the scaled plane vector
        Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
        Vector4 c = clipPlane4d * (1.0F / (clipPlane4d.dotProduct(q)));

        // Replace the third row of the projection matrix
        matrix[2][0] = c.x;
        matrix[2][1] = c.y;
        // flip the next bit from Lengyel since we're right-handed
        if (forGpuProgram)
        {
            matrix[2][2] = c.z; 
        }
        else
        {
            matrix[2][2] = -c.z; 
        }
        matrix[2][3] = c.w;        
    }
    //---------------------------------------------------------------------
    Real D3D11RenderSystem::getMinimumDepthInputValue(void)
    {
        // Range [0.0f, 1.0f]
        return 0.0f;
    }
    //---------------------------------------------------------------------
    Real D3D11RenderSystem::getMaximumDepthInputValue(void)
    {
        // Range [0.0f, 1.0f]
        // D3D inverts even identity view matrices, so maximum INPUT is -1.0
        return -1.0f;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::registerThread()
    {
        // nothing to do - D3D11 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::unregisterThread()
    {
        // nothing to do - D3D11 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::preExtraThreadsStarted()
    {
        // nothing to do - D3D11 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::postExtraThreadsStarted()
    {
        // nothing to do - D3D11 shares rendering context already
    }
    //---------------------------------------------------------------------
    String D3D11RenderSystem::getErrorDescription( long errorNumber ) const
    {
        return mDevice.getErrorDescription(errorNumber);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::determineFSAASettings(uint fsaa, const String& fsaaHint, 
        DXGI_FORMAT format, DXGI_SAMPLE_DESC* outFSAASettings)
    {
        bool qualityHint = fsaa >= 8 && fsaaHint.find("Quality") != String::npos;
        
        // NVIDIA, AMD - prefer CSAA aka EQAA if available.
        // see http://developer.nvidia.com/object/coverage-sampled-aa.html
        // see http://developer.amd.com/wordpress/media/2012/10/EQAA%20Modes%20for%20AMD%20HD%206900%20Series%20Cards.pdf

        // Modes are sorted from high quality to low quality, CSAA aka EQAA are listed first
        // Note, that max(Count, Quality) == FSAA level and (Count >= 8 && Quality != 0) == quality hint
        DXGI_SAMPLE_DESC presets[] = {
                { 8, 16 }, // CSAA 16xQ, EQAA 8f16x
                { 4, 16 }, // CSAA 16x,  EQAA 4f16x
                { 16, 0 }, // MSAA 16x

                { 12, 0 }, // MSAA 12x

                { 8, 8 },  // CSAA 8xQ
                { 4, 8 },  // CSAA 8x,  EQAA 4f8x
                { 8, 0 },  // MSAA 8x

                { 6, 0 },  // MSAA 6x
                { 4, 0 },  // MSAA 4x
                { 2, 0 },  // MSAA 2x
                { 1, 0 },  // MSAA 1x
                { NULL },
        };

        // Skip too HQ modes
        DXGI_SAMPLE_DESC* mode = presets;
        for(; mode->Count != 0; ++mode)
        {
            unsigned modeFSAA = std::max(mode->Count, mode->Quality);
            bool modeQuality = mode->Count >= 8 && mode->Quality != 0;
            bool tooHQ = (modeFSAA > fsaa || (modeFSAA == fsaa && modeQuality && !qualityHint));
            if(!tooHQ)
                break;
        }

        // Use first supported mode
        for(; mode->Count != 0; ++mode)
        {
            UINT outQuality;
            HRESULT hr = mDevice->CheckMultisampleQualityLevels(format, mode->Count, &outQuality);

            if(SUCCEEDED(hr) && outQuality > mode->Quality)
            {
                *outFSAASettings = *mode;
                return;
            }
        }

        outFSAASettings->Count = 1;
        outFSAASettings->Quality = 0;
    }
    //---------------------------------------------------------------------
    unsigned int D3D11RenderSystem::getDisplayMonitorCount() const
    {
        unsigned int monitorCount = 0;
        HRESULT hr;
        IDXGIOutput *pOutput;

        if (!mDriverList)
        {
            return 0;
        }
        
        for (size_t i = 0; i < mDriverList->count(); ++i)
        {
            for (size_t m = 0;; ++m)
            {
                hr = mDriverList->item(i)->getDeviceAdapter()->EnumOutputs(m, &pOutput);
                if (DXGI_ERROR_NOT_FOUND == hr)
                {
                    break;
                }
                else if (FAILED(hr))
                {
                    break;   //Something bad happened.
                }
                else
                {
                    SAFE_RELEASE(pOutput);
                    ++monitorCount;
                }
            }
        }
        return monitorCount;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::initRenderSystem()
    {
        if (mRenderSystemWasInited)
        {
            return;
        }

        mRenderSystemWasInited = true;
        // set pointers to NULL
        mpDXGIFactory = NULL;
        HRESULT hr;
        hr = CreateDXGIFactory1( __uuidof(IDXGIFactoryN), (void**)&mpDXGIFactory );
        if( FAILED(hr) )
        {
			OGRE_EXCEPT_EX( Exception::ERR_RENDERINGAPI_ERROR, hr, 
                "Failed to create Direct3D11 DXGIFactory1", 
                "D3D11RenderSystem::D3D11RenderSystem" );
        }

        mDriverList = NULL;
        mActiveD3DDriver = NULL;
        mTextureManager = NULL;
        mHardwareBufferManager = NULL;
        mGpuProgramManager = NULL;
        mPrimaryWindow = NULL;
        mMinRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
#if __OGRE_WINRT_PHONE // Windows Phone support only FL 9.3, but simulator can create much more capable device, so restrict it artificially here
        mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_9_3;
#elif defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
        mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_1;
#else
        mMaxRequestedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
#endif
        mUseNVPerfHUD = false;
        mHLSLProgramFactory = NULL;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		OGRE_DELETE mStereoDriver;
		mStereoDriver = NULL;
#endif

        mPso = NULL;
        mBoundComputeProgram = NULL;

        mBindingType = TextureUnitState::BT_FRAGMENT;

        //sets the modification trackers to true
		mSamplerStatesChanged = true;
		mLastTextureUnitState = 0;

        ZeroMemory(mTexStageDesc, OGRE_MAX_TEXTURE_LAYERS * sizeof(sD3DTextureStageDesc));
        mReadBackAsTexture = false;

        ID3D11DeviceN * device = createD3D11Device(NULL, DT_HARDWARE, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, 0);
        mDevice.TransferOwnership(device);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::getCustomAttribute(const String& name, void* pData)
    {
        if( name == "D3DDEVICE" )
        {
            ID3D11DeviceN  **device = (ID3D11DeviceN **)pData;
            *device = mDevice.get();
            return;
        }
        else if( name == "MapNoOverwriteOnDynamicConstantBuffer" )
        {
            *reinterpret_cast<bool*>(pData) = false; //TODO
            return;
        }
        else if( name == "MapNoOverwriteOnDynamicBufferSRV" )
        {
            *reinterpret_cast<bool*>(pData) = false; //TODO
            return;
        }

        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found: " + name, "RenderSystem::getCustomAttribute");
    }
    //---------------------------------------------------------------------
    D3D11HLSLProgram* D3D11RenderSystem::_getBoundComputeProgram() const
    {
        return mBoundComputeProgram;
    }
	//---------------------------------------------------------------------
	bool D3D11RenderSystem::setDrawBuffer(ColourBufferType colourBuffer)
	{
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		return D3D11StereoDriverBridge::getSingleton().setDrawBuffer(colourBuffer);
#else
		return false;
#endif
	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::beginProfileEvent( const String &eventName )
    {
#if OGRE_D3D11_PROFILING
        if(mDevice.GetProfiler())
        {			
            wchar_t wideName[256]; // Let avoid heap memory allocation if we are in profiling code.
            bool wideNameOk = !eventName.empty() && 0 != MultiByteToWideChar(CP_ACP, 0, eventName.data(), eventName.length() + 1, wideName, ARRAYSIZE(wideName));
            mDevice.GetProfiler()->BeginEvent(wideNameOk ? wideName : L"<too long or empty event name>");
        }
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::endProfileEvent( void )
    {
#if OGRE_D3D11_PROFILING
        if(mDevice.GetProfiler())
            mDevice.GetProfiler()->EndEvent();
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::markProfileEvent( const String &eventName )
    {
#if OGRE_D3D11_PROFILING
        if(mDevice.GetProfiler())
        {
            wchar_t wideName[256]; // Let avoid heap memory allocation if we are in profiling code.
            bool wideNameOk = !eventName.empty() && 0 != MultiByteToWideChar(CP_ACP, 0, eventName.data(), eventName.length() + 1, wideName, ARRAYSIZE(wideName));
            mDevice.GetProfiler()->SetMarker(wideNameOk ? wideName : L"<too long or empty event name>");
        }
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::initGPUProfiling(void)
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_BindD3D11( (void*)mDevice.get(), (void*)mDevice.GetImmediateContext() );
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::deinitGPUProfiling(void)
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_UnbindD3D11();
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::beginGPUSampleProfile( const String &name, uint32 *hashCache )
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_BeginD3D11Sample( name.c_str(), hashCache );
#endif
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::endGPUSampleProfile( const String &name )
    {
#if OGRE_PROFILING == OGRE_PROFILING_REMOTERY
        _rmt_EndD3D11Sample();
#endif
    }
    //---------------------------------------------------------------------
    const PixelFormatToShaderType* D3D11RenderSystem::getPixelFormatToShaderType(void) const
    {
        return &mD3D11PixelFormatToShaderType;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_clearStateAndFlushCommandBuffer(void)
    {
        mDevice.GetImmediateContext()->ClearState();
        mDevice.GetImmediateContext()->Flush();
    }
}
