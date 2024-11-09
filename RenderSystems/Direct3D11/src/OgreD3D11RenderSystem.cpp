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
#include "OgreMeshManager.h"
#include "OgreD3D11HardwareBufferManager.h"
#include "OgreD3D11HardwareBuffer.h"
#include "OgreD3D11VertexDeclaration.h"
#include "OgreGpuProgramManager.h"
#include "OgreD3D11HLSLProgramFactory.h"

#include "OgreD3D11HardwareOcclusionQuery.h"
#include "OgreD3D11MultiRenderTarget.h"
#include "OgreD3D11HLSLProgram.h"

#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreD3D11RenderTarget.h"
#include "OgreException.h"
#include "OgreRoot.h"

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
#include "OgreNsightChecker.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT &&  defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
#include <dxgi1_3.h> // for IDXGIDevice3::Trim
#endif

#define CHECK_DEVICE_ERROR(errmsg) \
if (mDevice.isError()) \
{ \
    String desc = mDevice.getErrorDescription(); \
    throw RenderingAPIException(0, "D3D11 device cannot " errmsg "\nError Description: "+desc, __FUNCTION__, __FILE__, __LINE__); \
}

namespace Ogre 
{
    HRESULT WINAPI D3D11CreateDeviceN(
        _In_opt_ IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        const D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        _Out_ ID3D11DeviceN** ppDevice,
        _Out_ D3D_FEATURE_LEVEL* pFeatureLevel,
        _Out_ ID3D11DeviceContextN** ppImmediateContext )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        return D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

#elif OGRE_PLATFORM == OGRE_PLATFORM_WINRT
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        ComPtr<ID3D11DeviceN> deviceN;
        ComPtr<ID3D11DeviceContextN> contextN;
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT mainHr, hr;

        mainHr = hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
                                        (ppDevice ? device.GetAddressOf() : NULL), &featureLevel, (ppImmediateContext ? context.GetAddressOf() : NULL));
        if(FAILED(hr)) return hr;

        hr = device ? device.As(&deviceN) : S_OK;
        if(FAILED(hr)) return hr;

        hr = context ? context.As(&contextN) : S_OK;
        if(FAILED(hr)) return hr;

        if(ppDevice)            *ppDevice = deviceN.Detach();
        if(pFeatureLevel)       *pFeatureLevel = featureLevel;
        if(ppImmediateContext)  *ppImmediateContext = contextN.Detach();

        return mainHr;
#endif
    }

    //---------------------------------------------------------------------
    D3D11RenderSystem::D3D11RenderSystem()
		: mDevice()
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
		, mStereoDriver(NULL)
#endif	
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		, suspendingToken()
		, surfaceContentLostToken()
#endif
    {
        LogManager::getSingleton().logMessage( "D3D11: " + getName() + " created." );

        mRenderSystemWasInited = false;
        mSwitchingFullscreenCounter = 0;
        mDriverType = D3D_DRIVER_TYPE_HARDWARE;

        initRenderSystem();

        // set config options defaults
        initConfigOptions();

        // Clear class instance storage
        memset(mClassInstances, 0, sizeof(mClassInstances));
        memset(mNumClassInstances, 0, sizeof(mNumClassInstances));

        mEventNames.push_back("DeviceLost");
        mEventNames.push_back("DeviceRestored");

#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
		suspendingToken = (Windows::ApplicationModel::Core::CoreApplication::Suspending +=
			ref new Windows::Foundation::EventHandler<Windows::ApplicationModel::SuspendingEventArgs^>([this](Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e)
		{
			// Hints to the driver that the app is entering an idle state and that its memory can be used temporarily for other apps.
			ComPtr<IDXGIDevice3> pDXGIDevice;
			if(mDevice.get() && SUCCEEDED(mDevice->QueryInterface(pDXGIDevice.GetAddressOf())))
				pDXGIDevice->Trim();
		}));

		surfaceContentLostToken = (Windows::Graphics::Display::DisplayInformation::DisplayContentsInvalidated +=
			ref new Windows::Foundation::TypedEventHandler<Windows::Graphics::Display::DisplayInformation^, Platform::Object^>(
				[this](Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ arg)
		{
			LogManager::getSingleton().logMessage("D3D11: DisplayContentsInvalidated.");
			validateDevice(true);
		}));
#else // Win 8.0
		surfaceContentLostToken = (Windows::Graphics::Display::DisplayProperties::DisplayContentsInvalidated +=
			ref new Windows::Graphics::Display::DisplayPropertiesEventHandler([this](Platform::Object ^sender)
		{
			LogManager::getSingleton().logMessage("D3D11: DisplayContentsInvalidated.");
			validateDevice(true);
		}));
#endif
#endif
    }
    //---------------------------------------------------------------------
    D3D11RenderSystem::~D3D11RenderSystem()
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#if defined(_WIN32_WINNT_WINBLUE) && _WIN32_WINNT >= _WIN32_WINNT_WINBLUE
		Windows::ApplicationModel::Core::CoreApplication::Suspending -= suspendingToken;
		Windows::Graphics::Display::DisplayInformation::DisplayContentsInvalidated -= surfaceContentLostToken;
#else // Win 8.0
		Windows::Graphics::Display::DisplayProperties::DisplayContentsInvalidated -= surfaceContentLostToken;
#endif
#endif

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

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        // Stereo driver must be freed after device is created
        D3D11StereoDriverBridge* stereoBridge = D3D11StereoDriverBridge::getSingletonPtr();
        OGRE_DELETE stereoBridge;
#endif

        LogManager::getSingleton().logMessage( "D3D11: " + getName() + " destroyed." );
    }
    //---------------------------------------------------------------------
    const String& D3D11RenderSystem::getName() const
    {
        static String strName( "Direct3D11 Rendering Subsystem");
        return strName;
    }

	//---------------------------------------------------------------------
    D3D11DriverList* D3D11RenderSystem::getDirect3DDrivers(bool refreshList /* = false*/)
    {
        if(!mDriverList)
            mDriverList = new D3D11DriverList();

        if(refreshList || mDriverList->count() == 0)
            mDriverList->refresh();

        return mDriverList;
    }
    //---------------------------------------------------------------------
	ID3D11DeviceN* D3D11RenderSystem::createD3D11Device(D3D11Driver* d3dDriver, D3D_DRIVER_TYPE driverType,
		D3D_FEATURE_LEVEL minFL, D3D_FEATURE_LEVEL maxFL, D3D_FEATURE_LEVEL* pFeatureLevel)
	{
		IDXGIAdapterN* pAdapter = (d3dDriver && driverType == D3D_DRIVER_TYPE_HARDWARE) ? d3dDriver->getDeviceAdapter() : NULL;

		assert(driverType == D3D_DRIVER_TYPE_HARDWARE || driverType == D3D_DRIVER_TYPE_SOFTWARE || driverType == D3D_DRIVER_TYPE_WARP);
		if(d3dDriver != NULL)
		{
			if(0 == wcscmp(d3dDriver->getAdapterIdentifier().Description, L"NVIDIA PerfHUD"))
				driverType = D3D_DRIVER_TYPE_REFERENCE;
			else
				driverType = D3D_DRIVER_TYPE_UNKNOWN;
		}

		// determine deviceFlags
		UINT deviceFlags = 0;
#if OGRE_PLATFORM == OGRE_PLATFORM_WINRT
		// This flag is required in order to enable compatibility with Direct2D.
		deviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

        auto it = mOptions.find("Debug Layer");
        bool debugEnabled = false;
        if (it != mOptions.end())
        {
            debugEnabled = StringConverter::parseBool(it->second.currentValue);
        }

		if(debugEnabled && !IsWorkingUnderNsight() && D3D11Device::D3D_NO_EXCEPTION != D3D11Device::getExceptionsErrorLevel())
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
        RenderSystem::initConfigOptions();

        ConfigOption optDevice;
        ConfigOption optVideoMode;
        ConfigOption optAA;
        ConfigOption optNVPerfHUD;
        ConfigOption optMinFeatureLevels;
        ConfigOption optMaxFeatureLevels;
        ConfigOption optExceptionsErrorLevel;
        ConfigOption optDriverType;

        optDevice.name = "Rendering Device";
        optDevice.currentValue = "(default)";
        optDevice.possibleValues.push_back("(default)");
        D3D11DriverList* driverList = getDirect3DDrivers();
        for( unsigned j=0; j < driverList->count(); j++ )
        {
            D3D11Driver* driver = driverList->item(j);
            optDevice.possibleValues.push_back( driver->DriverDescription() );
        }
        optDevice.immutable = false;

        optVideoMode.name = "Video Mode";
        optVideoMode.currentValue = "800 x 600 @ 32-bit colour";
        optVideoMode.immutable = false;

        optAA.name = "FSAA";
        optAA.immutable = false;
        optAA.possibleValues.push_back( "None" );
        optAA.currentValue = "None";

        optNVPerfHUD.currentValue = "No";
        optNVPerfHUD.immutable = false;
        optNVPerfHUD.name = "Allow NVPerfHUD";
        optNVPerfHUD.possibleValues.push_back( "Yes" );
        optNVPerfHUD.possibleValues.push_back( "No" );

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
        optMaxFeatureLevels.currentValue = "11.0";
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

        mOptions[optDevice.name] = optDevice;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optAA.name] = optAA;
        mOptions[optNVPerfHUD.name] = optNVPerfHUD;
        mOptions[optMinFeatureLevels.name] = optMinFeatureLevels;
        mOptions[optMaxFeatureLevels.name] = optMaxFeatureLevels;
        mOptions[optExceptionsErrorLevel.name] = optExceptionsErrorLevel;
        mOptions[optDriverType.name] = optDriverType;

        ConfigOption opt;
        opt.name = "Reversed Z-Buffer";
        opt.possibleValues = {"No", "Yes"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;

        opt.name = "Debug Layer";
        opt.possibleValues = {"Off", "On"};
        opt.currentValue = opt.possibleValues[0];
        opt.immutable = false;

        mOptions[opt.name] = opt;

        refreshD3DSettings();

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::refreshD3DSettings()
    {
        ConfigOption* optVideoMode;
        D3D11VideoMode* videoMode;

        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        if( opt != mOptions.end() )
        {
            D3D11Driver *driver = getDirect3DDrivers()->findByName(opt->second.currentValue);
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
            << "D3D11: RenderSystem Option: " << name << " = " << value;

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
            mMinRequestedFeatureLevel = D3D11Device::parseFeatureLevel(value, D3D_FEATURE_LEVEL_9_1);
        }

        if( name == "Max Requested Feature Levels" )
        {
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
            mMaxRequestedFeatureLevel = D3D11Device::parseFeatureLevel(value, D3D_FEATURE_LEVEL_11_1);
#else
            mMaxRequestedFeatureLevel = D3D11Device::parseFeatureLevel(value, D3D_FEATURE_LEVEL_11_0);
#endif
        }

        if(name == "Reversed Z-Buffer")
            mIsReverseDepthBufferEnabled = StringConverter::parseBool(value);

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
        D3D11Driver *driver = getDirect3DDrivers()->findByName(it->second.currentValue);
        if (driver)
        {
            it = mOptions.find("Video Mode");
            ComPtr<ID3D11DeviceN> device;
            device.Attach(createD3D11Device(driver, mDriverType, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, NULL));
            D3D11VideoMode* videoMode = driver->getVideoModeList()->item(it->second.currentValue); // Could be NULL if working over RDP/Simulator
            DXGI_FORMAT format = videoMode ? videoMode->getFormat() : DXGI_FORMAT_R8G8B8A8_UNORM;
            UINT numLevels = 0;
            // set maskable levels supported
            for (unsigned int n = 1; n <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; n++)
            {
                // new style enumeration, with AMD EQAA names. NVidia CSAA names are misleading
                // see determineFSAASettings for references
                if(n == 8 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &numLevels)) && numLevels > 8)
                    optFSAA->possibleValues.push_back("4f8"); // 8x CSAA
                if(n == 16 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &numLevels)) && numLevels > 16)
                    optFSAA->possibleValues.push_back("4f16"); // 16x CSAA
                if(n == 16 && SUCCEEDED(device->CheckMultisampleQualityLevels(format, 8, &numLevels)) && numLevels > 16)
                    optFSAA->possibleValues.push_back("8f16"); // 16xQ CSAA
                if (SUCCEEDED(device->CheckMultisampleQualityLevels(format, n, &numLevels)) && numLevels > 0)
                    optFSAA->possibleValues.push_back(std::to_string(n)); // Nx MSAA
            }
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
        String driverName = it->second.currentValue;
        if(driverName != "(default)" && getDirect3DDrivers()->findByName(driverName)->DriverDescription() != driverName)
        {
            // Just pick default driver
            setConfigOption("Rendering Device", "(default)");
            return "Requested rendering device could not be found, default would be used instead.";
        }

        return BLANKSTRING;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_initialise()
    {
        // call superclass method
        RenderSystem::_initialise();

        LogManager::getSingleton().logMessage( "D3D11: Subsystem Initialising" );

		if(IsWorkingUnderNsight())
			LogManager::getSingleton().logMessage( "D3D11: Nvidia Nsight found");

        // Init using current settings
        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        if( opt == mOptions.end() )
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Can`t find requested Direct3D driver name!", "D3D11RenderSystem::initialise" );
        mDriverName = opt->second.currentValue;

        // Driver type
        opt = mOptions.find( "Driver type" );
        if( opt == mOptions.end() )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find driver type!", "D3D11RenderSystem::initialise" );
        mDriverType = D3D11Device::parseDriverType(opt->second.currentValue);

        opt = mOptions.find( "Information Queue Exceptions Bottom Level" );
        if( opt == mOptions.end() )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find Information Queue Exceptions Bottom Level option!", "D3D11RenderSystem::initialise" );
        D3D11Device::setExceptionsErrorLevel(opt->second.currentValue);

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        // Stereo driver must be created before device is created
        auto stereoMode = StringConverter::parseBool(mOptions["Frame Sequential Stereo"].currentValue);
        D3D11StereoDriverBridge* stereoBridge = OGRE_NEW D3D11StereoDriverBridge(stereoMode);
#endif

        // create the device for the selected adapter
        createDevice();

        LogManager::getSingleton().logMessage("***************************************");
        LogManager::getSingleton().logMessage("*** D3D11: Subsystem Initialized OK ***");
        LogManager::getSingleton().logMessage("***************************************");

        this->fireDeviceEvent(&mDevice, "DeviceCreated");
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::shutdown()
    {
        RenderSystem::shutdown();

        mRenderSystemWasInited = false;

        mPrimaryWindow = NULL; // primary window deleted by base class.
        freeDevice();
        SAFE_DELETE( mDriverList );
        mActiveD3DDriver = D3D11Driver();
        mDevice.ReleaseAll();
        LogManager::getSingleton().logMessage("D3D11: Shutting down cleanly.");
        SAFE_DELETE( mTextureManager );
        SAFE_DELETE( mHardwareBufferManager );
    }
    //---------------------------------------------------------------------
	RenderWindow* D3D11RenderSystem::_createRenderWindow(const String &name,
		unsigned int width, unsigned int height, bool fullScreen,
		const NameValuePairList *miscParams)
	{
        RenderSystem::_createRenderWindow(name, width, height, fullScreen, miscParams);

		// Check we're not creating a secondary window when the primary
		// was fullscreen
		if (mPrimaryWindow && mPrimaryWindow->isFullScreen() && fullScreen == false)
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
				"Cannot create secondary windows not in full screen when the primary is full screen",
				"D3D11RenderSystem::_createRenderWindow");
		}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		D3D11RenderWindowBase* win = new D3D11RenderWindowHwnd(mDevice);
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
			win = new D3D11RenderWindowImageSource(mDevice);
		if(win == NULL && windowType == "SwapChainPanel")
			win = new D3D11RenderWindowSwapChainPanel(mDevice);
#endif // !__OGRE_WINRT_PHONE_80
		if(win == NULL)
			win = new D3D11RenderWindowCoreWindow(mDevice);
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
			mHardwareBufferManager = new D3D11HardwareBufferManager(mDevice);

			// create & register HLSL factory
			if (mHLSLProgramFactory == NULL)
				mHLSLProgramFactory = new D3D11HLSLProgramFactory(mDevice);
			mRealCapabilities = createRenderSystemCapabilities();

            mNativeShadingLanguageVersion = 4;

			// if we are using custom capabilities, then 
			// mCurrentCapabilities has already been loaded
			if (!mUseCustomCapabilities)
				mCurrentCapabilities = mRealCapabilities;

			fireEvent("RenderSystemCapabilitiesCreated");

			initialiseFromRenderSystemCapabilities(mCurrentCapabilities, mPrimaryWindow);

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
        rsc->setDeviceName(mActiveD3DDriver.DriverDescription());
        rsc->setRenderSystemName(getName());
		
        // Does NOT support fixed-function!
        //rsc->setCapability(RSC_FIXED_FUNCTION);

        rsc->setCapability(RSC_HWSTENCIL);

        UINT formatSupport;
        if(mFeatureLevel >= D3D_FEATURE_LEVEL_9_2
        || SUCCEEDED(mDevice->CheckFormatSupport(DXGI_FORMAT_R32_UINT, &formatSupport)) && 0 != (formatSupport & D3D11_FORMAT_SUPPORT_IA_INDEX_BUFFER))
            rsc->setCapability(RSC_32BIT_INDEX);

        // Set number of texture units, cap at OGRE_MAX_TEXTURE_LAYERS
        rsc->setNumTextureUnits(OGRE_MAX_TEXTURE_LAYERS);
        rsc->setNumVertexAttributes(D3D11_STANDARD_VERTEX_ELEMENT_COUNT);
        rsc->setCapability(RSC_ANISOTROPY);
        // Cube map
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->setCapability(RSC_READ_BACK_AS_TEXTURE);
        }

        // We always support compression, D3DX will decompress if device does not support
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

		if(mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
			rsc->setCapability(RSC_TWO_SIDED_STENCIL);

        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_HWOCCLUSION);
        rsc->setCapability(RSC_HWOCCLUSION_ASYNCHRONOUS);

        convertVertexShaderCaps(rsc);
        convertPixelShaderCaps(rsc);
        convertGeometryShaderCaps(rsc);
        convertHullShaderCaps(rsc);
        convertComputeShaderCaps(rsc);
        rsc->addShaderProfile("hlsl");

        rsc->setCapability(RSC_USER_CLIP_PLANES);


        rsc->setCapability(RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE);


        // Adapter details
        const DXGI_ADAPTER_DESC1& adapterID = mActiveD3DDriver.getAdapterIdentifier();

        switch(mDriverType) {
        case D3D_DRIVER_TYPE_HARDWARE:
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
            default:
                rsc->setVendor(GPU_UNKNOWN);
                break;
            };
            break;
        case D3D_DRIVER_TYPE_SOFTWARE:
            rsc->setVendor(GPU_MS_SOFTWARE);
            break;
        case D3D_DRIVER_TYPE_WARP:
            rsc->setVendor(GPU_MS_WARP);
            break;
        default:
            rsc->setVendor(GPU_UNKNOWN);
            break;
        }

        rsc->setCapability(RSC_DEPTH_CLAMP);

        rsc->setCapability(RSC_TEXTURE_3D);
        rsc->setCapability(RSC_TEXTURE_2D_ARRAY);
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
            rsc->setCapability(RSC_HWRENDER_TO_TEXTURE_3D);
            rsc->setCapability(RSC_TEXTURE_1D);
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
    
        rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
        rsc->setNumVertexTextureUnits(4);

        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        // actually irrelevant, but set
        rsc->setCapability(RSC_PERSTAGECONSTANT);

        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);

        rsc->setCapability(RSC_PRIMITIVE_RESTART);

        return rsc;

    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::initialiseFromRenderSystemCapabilities(
        RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        // add hlsl
        HighLevelGpuProgramManager::getSingleton().addFactory(mHLSLProgramFactory);
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

        // float params, always 4D
        rsc->setVertexProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);

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
#endif
        }
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            rsc->addShaderProfile("ps_4_0");
#if SUPPORT_SM2_0_HLSL_SHADERS == 1
            rsc->addShaderProfile("ps_3_0");
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

        // float params, always 4D
        rsc->setFragmentProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertHullShaderCaps(RenderSystemCapabilities* rsc) const
    {
        // Only for shader model 5.0
        if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            rsc->addShaderProfile("hs_5_0");
            rsc->addShaderProfile("ds_5_0");
            
            rsc->setCapability(RSC_TESSELLATION_PROGRAM);

            // float params, always 4D
            rsc->setTessellationHullProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
            rsc->setTessellationDomainProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
        }

    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertDomainShaderCaps(RenderSystemCapabilities* rsc) const
    {
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

        // float params, always 4D
        rsc->setComputeProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
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

        rsc->setGeometryProgramConstantFloatCount(D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
        rsc->setGeometryProgramNumOutputVertices(1024);
    }
    //-----------------------------------------------------------------------
    bool D3D11RenderSystem::checkVertexTextureFormats(void)
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
    DepthBuffer* D3D11RenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        // Get surface data (mainly to get MSAA data)
        D3D11RenderTarget* d3d11RenderTarget = dynamic_cast<D3D11RenderTarget*>(renderTarget);
        ID3D11Texture2D* d3d11Texture = NULL;
        if (d3d11RenderTarget)
        {
            d3d11Texture = d3d11RenderTarget->getSurface();
        }

        if (!d3d11Texture)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid render target",
                        "D3D11RenderSystem::_createDepthBufferFor");
        }

        D3D11_TEXTURE2D_DESC BBDesc;
        d3d11Texture->GetDesc(&BBDesc);

        // Create depth stencil texture
        ComPtr<ID3D11Texture2D> pDepthStencil;
        D3D11_TEXTURE2D_DESC descDepth;

        descDepth.Width                 = renderTarget->getWidth();
        descDepth.Height                = renderTarget->getHeight();
        descDepth.MipLevels             = 1;
        descDepth.ArraySize             = BBDesc.ArraySize;

        if ( mFeatureLevel < D3D_FEATURE_LEVEL_10_0)
            descDepth.Format            = isReverseDepthBufferEnabled() ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
        else
            descDepth.Format            = isReverseDepthBufferEnabled() ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS;

        descDepth.SampleDesc.Count      = BBDesc.SampleDesc.Count;
        descDepth.SampleDesc.Quality    = BBDesc.SampleDesc.Quality;
        descDepth.Usage                 = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags             = D3D11_BIND_DEPTH_STENCIL;

        // If we tell we want to use it as a Shader Resource when in MSAA, we will fail
        // This is a recomandation from NVidia.
        if(!mReadBackAsTexture && mFeatureLevel >= D3D_FEATURE_LEVEL_10_0 && BBDesc.SampleDesc.Count == 1)
            descDepth.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

        descDepth.CPUAccessFlags        = 0;
        descDepth.MiscFlags             = 0;

        if (descDepth.ArraySize == 6)
        {
            descDepth.MiscFlags     |= D3D11_RESOURCE_MISC_TEXTURECUBE;
        }

        OGRE_CHECK_DX_ERROR(
            mDevice->CreateTexture2D(&descDepth, NULL, pDepthStencil.ReleaseAndGetAddressOf()));

        //
        // Create the View of the texture
        // If MSAA is used, we cannot do this
        //
        if(!mReadBackAsTexture && mFeatureLevel >= D3D_FEATURE_LEVEL_10_0 && BBDesc.SampleDesc.Count == 1)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            viewDesc.Format = isReverseDepthBufferEnabled() ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2D.MipLevels = 1;
            OGRE_CHECK_DX_ERROR(mDevice->CreateShaderResourceView(pDepthStencil.Get(), &viewDesc,
                                                                  mDSTResView.ReleaseAndGetAddressOf()));
        }

        // Create the depth stencil view
        ID3D11DepthStencilView      *depthStencilView;
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );

        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = (BBDesc.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Flags = 0 /* D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL */;    // TODO: Allows bind depth buffer as depth view AND texture simultaneously.

        if(isReverseDepthBufferEnabled())
        {
            descDSV.Format            = DXGI_FORMAT_D32_FLOAT;
        }
                                                                                            // TODO: Decide how to expose this feature
        descDSV.Texture2D.MipSlice = 0;
        OGRE_CHECK_DX_ERROR(
            mDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &depthStencilView));

        //Create the abstract container
        D3D11DepthBuffer *newDepthBuffer = new D3D11DepthBuffer( DepthBuffer::POOL_DEFAULT, this, depthStencilView,
                                                descDepth.Width, descDepth.Height,
                                                descDepth.SampleDesc.Count, descDepth.SampleDesc.Quality,
                                                false );

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
    DepthBuffer* D3D11RenderSystem::_addManualDepthBuffer( ID3D11DepthStencilView *depthSurface,
                                                            uint32 width, uint32 height,
                                                            uint32 fsaa, uint32 fsaaQuality )
    {
        //If this depth buffer was already added, return that one
        DepthBufferVec::const_iterator itor = mDepthBufferPool[DepthBuffer::POOL_DEFAULT].begin();
        DepthBufferVec::const_iterator end  = mDepthBufferPool[DepthBuffer::POOL_DEFAULT].end();

        while( itor != end )
        {
            if( static_cast<D3D11DepthBuffer*>(*itor)->getDepthStencilView() == depthSurface )
                return *itor;

            ++itor;
        }

        //Create a new container for it
        D3D11DepthBuffer *newDepthBuffer = new D3D11DepthBuffer( DepthBuffer::POOL_DEFAULT, this, depthSurface,
                                                                    width, height, fsaa, fsaaQuality, true );

        //Add the 'main' depth buffer to the pool
        mDepthBufferPool[newDepthBuffer->getPoolId()].push_back( newDepthBuffer );

        return newDepthBuffer;
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
            // Unbind any vertex streams to avoid memory leaks
            /*for (unsigned int i = 0; i < mLastVertexSourceCount; ++i)
            {
                HRESULT hr = mDevice->SetStreamSource(i, NULL, 0, 0);
            }
            */
            // Clean up depth stencil surfaces
            mDevice.ReleaseAll();
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::createDevice()
    {
        mDevice.ReleaseAll();

        D3D11Driver* d3dDriver = getDirect3DDrivers(true)->findByName(mDriverName);
        mActiveD3DDriver = *d3dDriver; // store copy of selected driver, so that it is not lost when drivers would be re-enumerated
        LogManager::getSingleton().stream() << "D3D11: Requested \"" << mDriverName << "\", selected \"" << d3dDriver->DriverDescription() << "\"";

        if(D3D11Driver* nvPerfHudDriver = (mDriverType == D3D_DRIVER_TYPE_HARDWARE && mUseNVPerfHUD) ? getDirect3DDrivers()->item("NVIDIA PerfHUD") : NULL)
        {
            d3dDriver = nvPerfHudDriver;
            LogManager::getSingleton().logMessage("D3D11: Actually \"NVIDIA PerfHUD\" is used");
        }

        ID3D11DeviceN * device = createD3D11Device(d3dDriver, mDriverType, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, &mFeatureLevel);
        mDevice.TransferOwnership(device);

        LogManager::getSingleton().stream() << "D3D11: Device Feature Level " << (mFeatureLevel >> 12)
                                            << "." << ((mFeatureLevel >> 8) & 0xF);

        LARGE_INTEGER driverVersion = mDevice.GetDriverVersion();
        mDriverVersion.major = HIWORD(driverVersion.HighPart);
        mDriverVersion.minor = LOWORD(driverVersion.HighPart);
        mDriverVersion.release = HIWORD(driverVersion.LowPart);
        mDriverVersion.build = LOWORD(driverVersion.LowPart);
    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::handleDeviceLost()
    {
        LogManager::getSingleton().logMessage("D3D11: Device was lost, recreating.");

        // release device depended resources
        fireDeviceEvent(&mDevice, "DeviceLost");

        for(auto& it : Root::getSingleton().getSceneManagers())
            it.second->_releaseManualHardwareResources();

        notifyDeviceLost(&mDevice);

        // Release all automatic temporary buffers and free unused
        // temporary buffers, so we doesn't need to recreate them,
        // and they will reallocate on demand.
        HardwareBufferManager::getSingleton()._releaseBufferCopies(true);

        // Cleanup depth stencils surfaces.
        _cleanupDepthBuffers();

        // recreate device
        createDevice();

        // recreate device depended resources
        notifyDeviceRestored(&mDevice);

        MeshManager::getSingleton().reloadAll(Resource::LF_PRESERVE_STATE);

        for(auto& it : Root::getSingleton().getSceneManagers())
            it.second->_restoreManualHardwareResources();

        // Invalidate active view port.
        mActiveViewport = NULL;

        fireDeviceEvent(&mDevice, "DeviceRestored");

        LogManager::getSingleton().logMessage("D3D11: Device was restored.");
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::validateDevice(bool forceDeviceElection)
    {
        if(mDevice.isNull())
            return;

        // The D3D Device is no longer valid if the elected adapter changes or if
        // the device has been removed.

        bool anotherIsElected = false;
        if(forceDeviceElection)
        {
            // elect new device
            D3D11Driver* newDriver = getDirect3DDrivers(true)->findByName(mDriverName);

            // check by LUID
            LUID newLUID = newDriver->getAdapterIdentifier().AdapterLuid;
            LUID prevLUID = mActiveD3DDriver.getAdapterIdentifier().AdapterLuid;
            anotherIsElected = (newLUID.LowPart != prevLUID.LowPart) || (newLUID.HighPart != prevLUID.HighPart);
        }

        if(anotherIsElected || mDevice.IsDeviceLost())
        {
            handleDeviceLost();
        }
    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::_updateAllRenderTargets(bool swapBuffers)
    {
        try
        {
            RenderSystem::_updateAllRenderTargets(swapBuffers);
        }
        catch(const D3D11RenderingAPIException& e)
        {
            if(e.getHResult() == DXGI_ERROR_DEVICE_REMOVED || e.getHResult() == DXGI_ERROR_DEVICE_RESET)
                LogManager::getSingleton().logMessage("D3D11: Device was lost while rendering.");
            else
                throw;
        }
    }
    //-----------------------------------------------------------------------
    void D3D11RenderSystem::_swapAllRenderTargetBuffers()
    {
        try
        {
            RenderSystem::_swapAllRenderTargetBuffers();
        }
        catch(const D3D11RenderingAPIException& e)
        {
            if(e.getHResult() == DXGI_ERROR_DEVICE_REMOVED || e.getHResult() == DXGI_ERROR_DEVICE_RESET)
                LogManager::getSingleton().logMessage("D3D11: Device was lost while rendering.");
            else
                throw;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
        Matrix4& dest, bool forGpuProgram)
    {
        dest = matrix;

        if (mIsReverseDepthBufferEnabled)
        {
            // Convert depth range from [-1,+1] to [1,0]
            dest[2][0] = (dest[2][0] - dest[3][0]) * -0.5f;
            dest[2][1] = (dest[2][1] - dest[3][1]) * -0.5f;
            dest[2][2] = (dest[2][2] - dest[3][2]) * -0.5f;
            dest[2][3] = (dest[2][3] - dest[3][3]) * -0.5f;
        }
        else
        {
            // Convert depth range from [-1,+1] to [0,1]
            dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
            dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
            dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
            dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
        }

        if (!forGpuProgram)
        {
            // Convert right-handed to left-handed
            dest[0][2] = -dest[0][2];
            dest[1][2] = -dest[1][2];
            dest[2][2] = -dest[2][2];
            dest[3][2] = -dest[3][2];
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setTexture( size_t stage, bool enabled, const TexturePtr& tex )
    {
        if (enabled && tex && tex->getSize() > 0)
        {
            // note used
            tex->touch();
            mTexStageDesc[stage].pTex = static_cast<D3D11Texture*>(tex.get());
            mTexStageDesc[stage].used = true;

            mLastTextureUnitState = stage+1;
        }
        else
        {
            mTexStageDesc[stage].used = false;
            // now we now what's the last texture unit set
			mLastTextureUnitState = std::min(mLastTextureUnitState,stage);
        }
        mSamplerStatesChanged = true;
    }
    void D3D11RenderSystem::_setSampler(size_t unit, Sampler& sampler)
    {
        mSamplerStatesChanged = true;

        mTexStageDesc[unit].pSampler = static_cast<D3D11Sampler&>(sampler).getState();
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage )
    {
        mBlendDesc.AlphaToCoverageEnable = alphaToCoverage;
        mBlendDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setCullingMode( CullingMode mode )
    {
        mCullingMode = mode;

		mRasterizerDesc.CullMode = D3D11Mappings::get(mode);
        mRasterizerDesc.FrontCounterClockwise = !flipFrontFace();
        mRasterizerDescChanged = true;
    }
    void D3D11RenderSystem::_setDepthClamp(bool enable)
    {
        mRasterizerDesc.DepthClipEnable = !enable;
        mRasterizerDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
    {
        mDepthStencilDesc.DepthEnable = depthTest;
        mDepthStencilDesc.DepthWriteMask = depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

        if(isReverseDepthBufferEnabled())
            depthFunction = reverseCompareFunction(depthFunction);
        mDepthStencilDesc.DepthFunc = D3D11Mappings::get(depthFunction);
        mDepthStencilDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        if(isReverseDepthBufferEnabled())
        {
            slopeScaleBias *= -1;
            constantBias *= -1;
        }

		const float nearFarFactor = 10.0; 
		mRasterizerDesc.DepthBias = static_cast<int>(-constantBias * nearFarFactor);
		mRasterizerDesc.SlopeScaledDepthBias = -slopeScaleBias;
        mRasterizerDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setColourBlendState(const ColourBlendState& state)
    {
        // record this
        mCurrentBlend = state;

        if (state.blendingEnabled())
        {
            mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
            mBlendDesc.RenderTarget[0].SrcBlend = D3D11Mappings::get(state.sourceFactor, false);
            mBlendDesc.RenderTarget[0].DestBlend = D3D11Mappings::get(state.destFactor, false);
            mBlendDesc.RenderTarget[0].BlendOp = D3D11Mappings::get(state.operation) ;
            mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11Mappings::get(state.sourceFactorAlpha, true);
            mBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11Mappings::get(state.destFactorAlpha, true);
            mBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11Mappings::get(state.alphaOperation) ;
            mBlendDesc.AlphaToCoverageEnable = false;

            mBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
        }
        else
        {
            mBlendDesc.RenderTarget[0].BlendEnable = FALSE;
        }

        UINT8 val = 0;
        if (state.writeR)
            val |= D3D11_COLOR_WRITE_ENABLE_RED;
        if (state.writeG)
            val |= D3D11_COLOR_WRITE_ENABLE_GREEN;
        if (state.writeB)
            val |= D3D11_COLOR_WRITE_ENABLE_BLUE;
        if (state.writeA)
            val |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

        mBlendDesc.RenderTarget[0].RenderTargetWriteMask = val;

        mBlendDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setPolygonMode(PolygonMode level)
    {
        if(mPolygonMode != level)
        {
            mPolygonMode = level;
            mRasterizerDesc.FillMode = D3D11Mappings::get(mPolygonMode);
            mRasterizerDescChanged = true;
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setStencilState(const StencilState& state)
    {
		// We honor user intent in case of one sided operation, and carefully tweak it in case of two sided operations.
		bool flipFront = state.twoSidedOperation;
		bool flipBack = state.twoSidedOperation && !flipFront;

        mDepthStencilDesc.StencilEnable = state.enabled;
        mStencilRef = state.referenceValue;
        mDepthStencilDesc.StencilReadMask = state.compareMask;
        mDepthStencilDesc.StencilWriteMask = state.writeMask;

		mDepthStencilDesc.FrontFace.StencilFailOp = D3D11Mappings::get(state.stencilFailOp, flipFront);
		mDepthStencilDesc.BackFace.StencilFailOp = D3D11Mappings::get(state.stencilFailOp, flipBack);
        
		mDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11Mappings::get(state.depthFailOp, flipFront);
		mDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11Mappings::get(state.depthFailOp, flipBack);
        
		mDepthStencilDesc.FrontFace.StencilPassOp = D3D11Mappings::get(state.depthStencilPassOp, flipFront);
		mDepthStencilDesc.BackFace.StencilPassOp = D3D11Mappings::get(state.depthStencilPassOp, flipBack);

		mDepthStencilDesc.FrontFace.StencilFunc = D3D11Mappings::get(state.compareOp);
		mDepthStencilDesc.BackFace.StencilFunc = D3D11Mappings::get(state.compareOp);
        mDepthStencilDescChanged = true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;
        if (mActiveRenderTarget)
        {
            // we need to clear the state 
            mDevice.GetImmediateContext()->ClearState();
            CHECK_DEVICE_ERROR("Clear State");
            _setRenderTargetViews();
        }
    }

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setRenderTargetViews()
    {
        RenderTarget* target = mActiveRenderTarget;
        D3D11RenderTarget* d3d11RenderTarget = dynamic_cast<D3D11RenderTarget*>(target);

        if (target && d3d11RenderTarget)
        {
            ID3D11RenderTargetView* pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            memset(pRTView, 0, sizeof(pRTView));

            uint numberOfViews = d3d11RenderTarget->getNumberOfViews();

            for (uint i = 0; i < OGRE_MAX_MULTIPLE_RENDER_TARGETS; i++)
            {
                pRTView[i] = d3d11RenderTarget->getRenderTargetView(i);
                if (!pRTView[i])
                {
                    break;
                }
            }

            //Retrieve depth buffer
            D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH && !depthBuffer )
            {
                //Depth is automatically managed and there is no depth buffer attached to this RT
                //or the Current D3D device doesn't match the one this Depth buffer was created
                setDepthBufferFor( target );
            }

            //Retrieve depth buffer again (it may have changed)
            depthBuffer = static_cast<D3D11DepthBuffer*>(target->getDepthBuffer());

            // now switch to the new render target
            mDevice.GetImmediateContext()->OMSetRenderTargets(
                numberOfViews,
                pRTView,
                depthBuffer ? depthBuffer->getDepthStencilView() : 0 );

            CHECK_DEVICE_ERROR("set render target");
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_setViewport( Viewport *vp )
    {
        if (!vp)
        {
            mActiveViewport = NULL;
            _setRenderTarget(NULL);
        }
        else if( vp != mActiveViewport || vp->_isUpdated() )
        {
            mActiveViewport = vp;

            // ok, it's different, time to set render target and viewport params
            D3D11_VIEWPORT d3dvp;

            // Set render target
            RenderTarget* target;
            target = vp->getTarget();

            _setRenderTarget(target);
            _setCullingMode( mCullingMode );

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
            CHECK_DEVICE_ERROR("set viewports");

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
			D3D11RenderWindowBase* d3d11Window = dynamic_cast<D3D11RenderWindowBase*>(target);
			if(d3d11Window)
				d3d11Window->_validateStereo();
#endif

            vp->_clearUpdatedFlag();
        }
        else
        {
            // if swapchain was created with DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL we need to reestablish render target views
            D3D11RenderWindowBase* d3d11Window = dynamic_cast<D3D11RenderWindowBase*>(vp->getTarget());
            if(d3d11Window && d3d11Window->_shouldRebindBackBuffer())
                _setRenderTargetViews();
        }
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::_endFrame()
    {
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setVertexDeclaration(VertexDeclaration* decl)
    {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                    "Cannot directly call setVertexDeclaration in the d3d11 render system - cast then use 'setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding)' .", 
                    "D3D11RenderSystem::setVertexDeclaration" );
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setVertexDeclaration(VertexDeclaration* decl, VertexBufferBinding* binding)
    {
        D3D11VertexDeclaration* d3ddecl = static_cast<D3D11VertexDeclaration*>(decl);
        d3ddecl->bindToShader(mBoundProgram[GPT_VERTEX_PROGRAM], binding);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
    {
        // TODO: attempt to detect duplicates
        const VertexBufferBinding::VertexBufferBindingMap& binds = binding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator i, iend;
        iend = binds.end();
        for (i = binds.begin(); i != iend; ++i)
        {
            const D3D11HardwareBuffer* d3d11buf = i->second->_getImpl<D3D11HardwareBuffer>();

            UINT stride = static_cast<UINT>(i->second->getVertexSize());
            UINT offset = 0; // no stream offset, this is handled in _render instead
            UINT slot = static_cast<UINT>(i->first);
            ID3D11Buffer * pVertexBuffers = d3d11buf->getD3DBuffer();
            mDevice.GetImmediateContext()->IASetVertexBuffers(
                slot, // The first input slot for binding.
                1, // The number of vertex buffers in the array.
                &pVertexBuffers,
                &stride,
                &offset 
                );

            CHECK_DEVICE_ERROR("set vertex buffers");
        }

        mLastVertexSourceCount = binds.size();      
    }

    //---------------------------------------------------------------------
    // TODO: Move this class to the right place.
    class D3D11RenderOperationState
    {
    public:
        ComPtr<ID3D11BlendState> mBlendState;
        ComPtr<ID3D11RasterizerState> mRasterizer;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;

        ID3D11SamplerState* mSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
        size_t mSamplerStatesCount;

        ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS]; // note - not owning
        size_t mTexturesCount;

        D3D11RenderOperationState() : mSamplerStatesCount(0), mTexturesCount(0) {}
        ~D3D11RenderOperationState() {}
    };

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_dispatchCompute(const Vector3i& workgroupDim)
    {
        mDevice.GetImmediateContext()->CSSetShader(mBoundProgram[GPT_COMPUTE_PROGRAM]->getComputeShader(),
                                                    mClassInstances[GPT_COMPUTE_PROGRAM],
                                                    mNumClassInstances[GPT_COMPUTE_PROGRAM]);
        CHECK_DEVICE_ERROR("set compute shader");

        ID3D11ShaderResourceView* nullSrv[] = { 0 };

        ID3D11UnorderedAccessView* uavs[OGRE_MAX_TEXTURE_LAYERS] = {NULL};
        ID3D11ShaderResourceView * srvs[OGRE_MAX_TEXTURE_LAYERS] = {NULL};
        ID3D11SamplerState* samplers[OGRE_MAX_TEXTURE_LAYERS] = {NULL};

        // samplers mapping
        size_t numberOfSamplers = std::min(mLastTextureUnitState,(size_t)(OGRE_MAX_TEXTURE_LAYERS + 1));
        for (size_t n = 0; n < numberOfSamplers; n++)
        {
            if(!mTexStageDesc[n].used)
                continue;

            if(mTexStageDesc[n].pTex->getUsage() & TU_UNORDERED_ACCESS)
                uavs[n] = mTexStageDesc[n].pTex->getUavView();
            else
            {
                srvs[n] = mTexStageDesc[n].pTex->getSrvView();
                samplers[n] = mTexStageDesc[n].pSampler;
            }
        }

        if(mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
        {
            // unbind SRVs from other stages
            mDevice.GetImmediateContext()->VSSetShaderResources(0, 1, nullSrv);
            mDevice.GetImmediateContext()->PSSetShaderResources(0, 1, nullSrv);
            mSamplerStatesChanged = true;

            mDevice.GetImmediateContext()->CSSetUnorderedAccessViews(0, static_cast<UINT>(numberOfSamplers), uavs, NULL);
            CHECK_DEVICE_ERROR("set compute UAVs");
        }

        if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
        {
            mDevice.GetImmediateContext()->CSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(numberOfSamplers), samplers);
            CHECK_DEVICE_ERROR("set compute shader samplers");
            mDevice.GetImmediateContext()->CSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(numberOfSamplers), srvs);
            CHECK_DEVICE_ERROR("set compute shader resources");
        }

        // Bound unordered access views
        mDevice.GetImmediateContext()->Dispatch(workgroupDim[0], workgroupDim[1], workgroupDim[2]);

        // unbind
        ID3D11UnorderedAccessView* views[] = { 0 };
        mDevice.GetImmediateContext()->CSSetShaderResources( 0, 1, nullSrv );
        mDevice.GetImmediateContext()->CSSetUnorderedAccessViews( 0, 1, views, NULL );
        mDevice.GetImmediateContext()->CSSetShader( NULL, NULL, 0 );
    }

    void D3D11RenderSystem::_render(const RenderOperation& op)
    {

        // Exit immediately if there is nothing to render
        if (op.vertexData==0 || op.vertexData->vertexCount == 0)
        {
            return;
        }

        size_t numberOfInstances = op.numberOfInstances;

        // Call super class
        RenderSystem::_render(op);
        
        D3D11RenderOperationState stackOpState;
        D3D11RenderOperationState * opState = &stackOpState;

        if(mBlendDescChanged)
        {
            mBlendDescChanged = false;
            mBoundBlendState = 0;

            OGRE_CHECK_DX_ERROR(
                mDevice->CreateBlendState(&mBlendDesc, opState->mBlendState.ReleaseAndGetAddressOf()));
        }
        else
        {
            opState->mBlendState = mBoundBlendState;
        }

        if(mRasterizerDescChanged)
		{
			mRasterizerDescChanged=false;
			mBoundRasterizer = 0;

            OGRE_CHECK_DX_ERROR(mDevice->CreateRasterizerState(&mRasterizerDesc, opState->mRasterizer.ReleaseAndGetAddressOf()));
        }
        else
        {
            opState->mRasterizer = mBoundRasterizer;
        }

        if(mDepthStencilDescChanged)
		{
			mBoundDepthStencilState = 0;
			mDepthStencilDescChanged=false;

            OGRE_CHECK_DX_ERROR(mDevice->CreateDepthStencilState(&mDepthStencilDesc, opState->mDepthStencilState.ReleaseAndGetAddressOf()));
        }
        else
		{
			opState->mDepthStencilState = mBoundDepthStencilState;
		}

        if(mSamplerStatesChanged)
		{
            // samplers mapping
            size_t numberOfSamplers = std::min(mLastTextureUnitState,(size_t)(OGRE_MAX_TEXTURE_LAYERS + 1));
            
            opState->mSamplerStatesCount = numberOfSamplers;
            opState->mTexturesCount = numberOfSamplers;
                            
            for (size_t n = 0; n < numberOfSamplers; n++)
            {
                ID3D11SamplerState *sampler = NULL;
                sD3DTextureStageDesc & stage = mTexStageDesc[n];
                opState->mSamplerStates[n]  = stage.used ? stage.pSampler : NULL;
                opState->mTextures[n]       = stage.used ? stage.pTex->getSrvView() : NULL;
            }
            for (size_t n = opState->mTexturesCount; n < OGRE_MAX_TEXTURE_LAYERS; n++)
			{
				opState->mTextures[n] = NULL;
			}
        }

        if (opState->mBlendState != mBoundBlendState)
        {
            mBoundBlendState = opState->mBlendState ;
            mDevice.GetImmediateContext()->OMSetBlendState(opState->mBlendState.Get(), 0, 0xffffffff); // TODO - find out where to get the parameters
            CHECK_DEVICE_ERROR("set blend state");
        }

        if (opState->mRasterizer != mBoundRasterizer)
        {
            mBoundRasterizer = opState->mRasterizer ;

            mDevice.GetImmediateContext()->RSSetState(opState->mRasterizer.Get());
            CHECK_DEVICE_ERROR("set rasterizer state");
        }
        

        if (opState->mDepthStencilState != mBoundDepthStencilState)
        {
            mBoundDepthStencilState = opState->mDepthStencilState ;

            mDevice.GetImmediateContext()->OMSetDepthStencilState(opState->mDepthStencilState.Get(), mStencilRef);
            CHECK_DEVICE_ERROR("set depth stencil state");
        }

        if (mSamplerStatesChanged && opState->mSamplerStatesCount > 0 ) //  if the NumSamplers is 0, the operation effectively does nothing.
        {
            mSamplerStatesChanged = false; // now it's time to set it to false
            /// Pixel Shader binding
            mDevice.GetImmediateContext()->PSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
            CHECK_DEVICE_ERROR("set pixel shader samplers");
            mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
            CHECK_DEVICE_ERROR("set pixel shader resources");
            
            /// Vertex Shader binding
            if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
            {
                mDevice.GetImmediateContext()->VSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
                CHECK_DEVICE_ERROR("set vertex shader samplers");
                mDevice.GetImmediateContext()->VSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                CHECK_DEVICE_ERROR("set vertex shader resources");
            }

            /// Geometry Shader binding
            if (mBoundProgram[GPT_GEOMETRY_PROGRAM] && mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
            {
                mDevice.GetImmediateContext()->GSSetSamplers(0, opState->mSamplerStatesCount, opState->mSamplerStates);
                CHECK_DEVICE_ERROR("set geometry shader samplers");
                mDevice.GetImmediateContext()->GSSetShaderResources(0, opState->mTexturesCount, &opState->mTextures[0]);
                CHECK_DEVICE_ERROR("set geometry shader resources");
            }

            /// Hull Shader binding
            if (mBoundProgram[GPT_HULL_PROGRAM] && mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
            {
                mDevice.GetImmediateContext()->HSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
                CHECK_DEVICE_ERROR("set hull shader samplers");
                mDevice.GetImmediateContext()->HSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                CHECK_DEVICE_ERROR("set hull shader resources");
            }
            
            /// Domain Shader binding
            if (mBoundProgram[GPT_DOMAIN_PROGRAM] && mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
            {
                mDevice.GetImmediateContext()->DSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
                CHECK_DEVICE_ERROR("set domain shader samplers");
                mDevice.GetImmediateContext()->DSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
                CHECK_DEVICE_ERROR("set domain shader resources");
            }
        }

        ComPtr<ID3D11Buffer> pSOTarget;
        // Mustn't bind a emulated vertex, pixel shader (see below), if we are rendering to a stream out buffer
        mDevice.GetImmediateContext()->SOGetTargets(1, pSOTarget.GetAddressOf());

        //check consistency of vertex-fragment shaders
        if (!mBoundProgram[GPT_VERTEX_PROGRAM] ||
            (!mBoundProgram[GPT_FRAGMENT_PROGRAM] && op.operationType != RenderOperation::OT_POINT_LIST && !pSOTarget))
        {
            
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to render to a D3D11 device without both vertex and fragment shaders there is no fixed pipeline in d3d11 - use the RTSS or write custom shaders.",
                "D3D11RenderSystem::_render");
        }

        // Check consistency of tessellation shaders
        if( (mBoundProgram[GPT_HULL_PROGRAM] && !mBoundProgram[GPT_DOMAIN_PROGRAM]) ||
            (!mBoundProgram[GPT_HULL_PROGRAM] && mBoundProgram[GPT_DOMAIN_PROGRAM]) )
        {
            if (mBoundProgram[GPT_HULL_PROGRAM] && !mBoundProgram[GPT_DOMAIN_PROGRAM]) {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to use tessellation, but domain shader is missing",
                "D3D11RenderSystem::_render");
            }
            else {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to use tessellation, but hull shader is missing",
                "D3D11RenderSystem::_render"); }
        }

        CHECK_DEVICE_ERROR("set geometry shader to null");

        // Defer program bind to here because we must bind shader class instances,
        // and this can only be made in SetShader calls.
        // Also, bind shader resources
        if (mBoundProgram[GPT_VERTEX_PROGRAM])
        {
            mDevice.GetImmediateContext()->VSSetShader(mBoundProgram[GPT_VERTEX_PROGRAM]->getVertexShader(),
                                                       mClassInstances[GPT_VERTEX_PROGRAM], 
                                                       mNumClassInstances[GPT_VERTEX_PROGRAM]);
            CHECK_DEVICE_ERROR("set vertex shader");
        }
        if (mBoundProgram[GPT_FRAGMENT_PROGRAM])
        {
            mDevice.GetImmediateContext()->PSSetShader(mBoundProgram[GPT_FRAGMENT_PROGRAM]->getPixelShader(),
                                                       mClassInstances[GPT_FRAGMENT_PROGRAM], 
                                                       mNumClassInstances[GPT_FRAGMENT_PROGRAM]);
            CHECK_DEVICE_ERROR("set pixel shader");
        }
        if (mBoundProgram[GPT_GEOMETRY_PROGRAM])
        {
            mDevice.GetImmediateContext()->GSSetShader(mBoundProgram[GPT_GEOMETRY_PROGRAM]->getGeometryShader(),
                                                       mClassInstances[GPT_GEOMETRY_PROGRAM], 
                                                       mNumClassInstances[GPT_GEOMETRY_PROGRAM]);
            CHECK_DEVICE_ERROR("set geometry shader");
        }
        if (mBoundProgram[GPT_HULL_PROGRAM])
        {
            mDevice.GetImmediateContext()->HSSetShader(mBoundProgram[GPT_HULL_PROGRAM]->getHullShader(),
                                                       mClassInstances[GPT_HULL_PROGRAM], 
                                                       mNumClassInstances[GPT_HULL_PROGRAM]);
            CHECK_DEVICE_ERROR("set hull shader");
        }
        if (mBoundProgram[GPT_DOMAIN_PROGRAM])
        {
            mDevice.GetImmediateContext()->DSSetShader(mBoundProgram[GPT_DOMAIN_PROGRAM]->getDomainShader(),
                                                       mClassInstances[GPT_DOMAIN_PROGRAM], 
                                                       mNumClassInstances[GPT_DOMAIN_PROGRAM]);
            CHECK_DEVICE_ERROR("set domain shader");
        }

        setVertexDeclaration(op.vertexData->vertexDeclaration, op.vertexData->vertexBufferBinding);
        setVertexBufferBinding(op.vertexData->vertexBufferBinding);


        // Determine rendering operation
        D3D11_PRIMITIVE_TOPOLOGY primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        DWORD primCount = 0;

        if(mBoundProgram[GPT_HULL_PROGRAM] && mBoundProgram[GPT_DOMAIN_PROGRAM])
        {
            // useful primitives for tessellation
            switch( op.operationType )
            {
            case RenderOperation::OT_LINE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
                break;

            case RenderOperation::OT_LINE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
                break;

            case RenderOperation::OT_TRIANGLE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
                break;

            case RenderOperation::OT_TRIANGLE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;
            }
        }
        else
        {
            //rendering without tessellation.   
            int operationType = op.operationType;
            if(mPolygonMode == PM_POINTS)
                operationType = RenderOperation::OT_POINT_LIST;

            switch( operationType )
            {
            case RenderOperation::OT_POINT_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
                break;

            case RenderOperation::OT_LINE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
                break;

            case RenderOperation::OT_LINE_LIST_ADJ:
                primType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 4;
                break;

            case RenderOperation::OT_LINE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
                break;

            case RenderOperation::OT_LINE_STRIP_ADJ:
                primType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;

            case RenderOperation::OT_TRIANGLE_LIST:
                primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
                break;

            case RenderOperation::OT_TRIANGLE_LIST_ADJ:
                primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 6;
                break;

            case RenderOperation::OT_TRIANGLE_STRIP:
                primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
                break;

            case RenderOperation::OT_TRIANGLE_STRIP_ADJ:
                primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
                primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2 - 2;
                break;

            case RenderOperation::OT_TRIANGLE_FAN:
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
                auto d3dBuf = op.indexData->indexBuffer->_getImpl<D3D11HardwareBuffer>();
                mDevice.GetImmediateContext()->IASetIndexBuffer( d3dBuf->getD3DBuffer(), D3D11Mappings::getFormat(op.indexData->indexBuffer->getType()), 0 );
                CHECK_DEVICE_ERROR("set index buffer");
            }

            mDevice.GetImmediateContext()->IASetPrimitiveTopology( primType );
            CHECK_DEVICE_ERROR("set primitive topology");

            do
            {
                if(op.useIndexes)
                {
                    if(numberOfInstances > 1)
                    {
                        mDevice.GetImmediateContext()->DrawIndexedInstanced(
                            static_cast<UINT>(op.indexData->indexCount), 
                            static_cast<UINT>(numberOfInstances), 
                            static_cast<UINT>(op.indexData->indexStart), 
                            static_cast<INT>(op.vertexData->vertexStart),
                            0);
                    }
                    else
                    {
                        mDevice.GetImmediateContext()->DrawIndexed(
                            static_cast<UINT>(op.indexData->indexCount),
                            static_cast<UINT>(op.indexData->indexStart),
                            static_cast<INT>(op.vertexData->vertexStart));
                    }
                }
                else // non indexed
                {
                    if(op.vertexData->vertexCount == -1) // -1 is a sign to use DrawAuto
                    {
                        mDevice.GetImmediateContext()->DrawAuto();
                    }
                    else if(numberOfInstances > 1)
                    {
                        mDevice.GetImmediateContext()->DrawInstanced(
                            static_cast<UINT>(op.vertexData->vertexCount),
                            static_cast<UINT>(numberOfInstances),
                            static_cast<UINT>(op.vertexData->vertexStart),
                            0);
                    }
                    else
                    {
                        mDevice.GetImmediateContext()->Draw(
                            static_cast<UINT>(op.vertexData->vertexCount),
                            static_cast<UINT>(op.vertexData->vertexStart));
                    }
                }

                if(mDevice.isError())
                {
                    String errorDescription = "D3D11 device cannot draw";
                    if(!op.useIndexes && op.vertexData->vertexCount == -1) // -1 is a sign to use DrawAuto
                        errorDescription.append(" auto");
                    else
                        errorDescription.append(op.useIndexes ? " indexed" : "").append(numberOfInstances > 1 ? " instanced" : "");
                    errorDescription.append("\nError Description:").append(mDevice.getErrorDescription());
                    errorDescription.append("\nActive OGRE shaders:");
                    for(auto shader : mBoundProgram)
                    {
                        if(shader)
                            errorDescription.append(("\n"+to_string(shader->getType())+" = " + shader->getName()).c_str());
                    }

                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, errorDescription, "D3D11RenderSystem::_render");
                }

            }while(updatePassIterationRenderState());
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
    void D3D11RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Null program bound");
        }

        mBoundProgram[prg->getType()] = static_cast<D3D11HLSLProgram*>(prg);

        RenderSystem::bindGpuProgram(prg);
   }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        mActiveParameters[gptype].reset();
        mBoundProgram[gptype] = NULL;

        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            {
                mDevice.GetImmediateContext()->VSSetShader(NULL, NULL, 0);
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            {
                mDevice.GetImmediateContext()->PSSetShader(NULL, NULL, 0);
            }

            break;
        case GPT_GEOMETRY_PROGRAM:
            {
                mDevice.GetImmediateContext()->GSSetShader( NULL, NULL, 0 );
            }
            break;
        case GPT_HULL_PROGRAM:
            {
                mDevice.GetImmediateContext()->HSSetShader( NULL, NULL, 0 );
            }
            break;
        case GPT_DOMAIN_PROGRAM:
            {
                mDevice.GetImmediateContext()->DSSetShader( NULL, NULL, 0 );
            }
            break;
        case GPT_COMPUTE_PROGRAM:
            {
                mDevice.GetImmediateContext()->CSSetShader( NULL, NULL, 0 );
            }
            break;
        default:
            assert(false && "Undefined Program Type!");
        };
        RenderSystem::unbindGpuProgram(gptype);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::bindGpuProgramParameters(GpuProgramType gptype, const GpuProgramParametersPtr& params, uint16 mask)
    {
        if (mask & (uint16)GPV_GLOBAL)
        {
            params->_updateSharedParams();
        }

        if (!mBoundProgram[gptype])
            return;

        std::vector<ID3D11Buffer*> buffers = {NULL};

        if(params->getConstantList().size())
        {
            auto& cbuffer = updateDefaultUniformBuffer(gptype, params->getConstantList());
            buffers[0] = static_cast<D3D11HardwareBuffer*>(cbuffer.get())->getD3DBuffer();
        }

        auto& bufferInfoMap = mBoundProgram[gptype]->getBufferInfoMap();
        for (const auto& usage : params->getSharedParameters())
        {
            if(const auto& buf = usage.getSharedParams()->_getHardwareBuffer())
            {
                // hardware baked cbuffer
                auto it = bufferInfoMap.find(usage.getName());
                if(it == bufferInfoMap.end())
                    continue; // TODO: error?

                size_t slot = it->second;
                buffers.resize(std::max(slot + 1, buffers.size()));
                buffers[slot] = static_cast<D3D11HardwareBuffer*>(buf.get())->getD3DBuffer();
            }
        }

        // Do everything here in Dx11, since deal with via buffers anyway so number of calls
        // is actually the same whether we categorise the updates or not
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->VSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set vertex shader constant buffers");
                }
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->PSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set fragment shader constant buffers");
                }
            }
            break;
        case GPT_GEOMETRY_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->GSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set Geometry shader constant buffers");
                }
            }
            break;
        case GPT_HULL_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->HSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set Hull shader constant buffers");
                }
            }
            break;
        case GPT_DOMAIN_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->DSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set Domain shader constant buffers");
                }
            }
            break;
        case GPT_COMPUTE_PROGRAM:
            {
                {
                    mDevice.GetImmediateContext()->CSSetConstantBuffers( 0, buffers.size(), buffers.data());
                    CHECK_DEVICE_ERROR("set Compute shader constant buffers");
                }
            }
            break;
        };

#ifdef SUBROUTINES
        // Now, set class instances
        std::map<uint32, String> subroutineMap;

        for(auto it& : subroutineMap)
        {
            setSubroutine(gptype, it.first, it.second);
        }
#endif
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
        unsigned int slotIdx = mBoundProgram[gptype] ? mBoundProgram[gptype]->getSubroutineSlot(slotName) : 0;
        // Set subroutine for slot
        setSubroutine(gptype, slotIdx, subroutineName);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setScissorTest(bool enabled, const Rect& rect)
    {
        mRasterizerDesc.ScissorEnable = enabled;
        mScissorRect.left = rect.left;
        mScissorRect.top = rect.top;
        mScissorRect.right = rect.right;
        mScissorRect.bottom = rect.bottom;

        mDevice.GetImmediateContext()->RSSetScissorRects(1, &mScissorRect);
        CHECK_DEVICE_ERROR("set scissor rects");
        mRasterizerDescChanged=true;
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, float depth, unsigned short stencil)
    {
        D3D11RenderTarget* d3d11RenderTarget = dynamic_cast<D3D11RenderTarget*>(mActiveRenderTarget);
        if (mActiveRenderTarget && d3d11RenderTarget)
        {
            ID3D11RenderTargetView* pRTView[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            memset(pRTView, 0, sizeof(pRTView));

            for (uint i = 0; i < OGRE_MAX_MULTIPLE_RENDER_TARGETS; i++)
            {
                pRTView[i] = d3d11RenderTarget->getRenderTargetView(i);
                if (!pRTView[i])
                {
                    break;
                }
            }

            if (buffers & FBT_COLOUR)
            {
                // Clear all views
                uint numberOfViews = d3d11RenderTarget->getNumberOfViews();
                for (uint i = 0; i < numberOfViews; ++i)
                {
                    mDevice.GetImmediateContext()->ClearRenderTargetView(pRTView[i], colour.ptr());
                }

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
                D3D11DepthBuffer *depthBuffer = static_cast<D3D11DepthBuffer*>(mActiveRenderTarget->
                                                                                        getDepthBuffer());
                if( depthBuffer )
                {
                    if (isReverseDepthBufferEnabled())
                    {
                        depth = 1.0f - 0.5f * (depth + 1.0f);
                    }

                    mDevice.GetImmediateContext()->ClearDepthStencilView(
                                                        depthBuffer->getDepthStencilView(),
                                                        ClearFlags, depth, static_cast<UINT8>(stencil) );
                }
            }
        }
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* D3D11RenderSystem::createHardwareOcclusionQuery(void)
    {
        D3D11HardwareOcclusionQuery* ret = new D3D11HardwareOcclusionQuery (mDevice); 
        mHwOcclusionQueries.push_back(ret);
        return ret;
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
    void D3D11RenderSystem::determineFSAASettings(uint fsaa, const String& fsaaHint, 
        DXGI_FORMAT format, DXGI_SAMPLE_DESC* outFSAASettings)
    {
        // "4f8" -> hint = "f8"
        bool useCSAA = !fsaaHint.empty() && fsaaHint.front() == 'f';
        uint32 quality = 0;
        if(useCSAA) StringConverter::parse(fsaaHint.substr(1), quality);

        // NVIDIA, AMD - enable CSAA
        // http://developer.download.nvidia.com/assets/gamedev/docs/CSAA_Tutorial.pdf
        // http://developer.amd.com/wordpress/media/2012/10/EQAA%20Modes%20for%20AMD%20HD%206900%20Series%20Cards.pdf

        // Modes are sorted from high quality to low quality, CSAA aka EQAA are listed first
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
                { 0 },
        };

        // Find matching AA mode
        DXGI_SAMPLE_DESC* mode = presets;
        for(; mode->Count != 0; ++mode)
        {
            bool tooHQ = (mode->Count > fsaa || mode->Quality > quality);
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
    void D3D11RenderSystem::initRenderSystem()
    {
        if (mRenderSystemWasInited)
        {
            return;
        }

        mRenderSystemWasInited = true;
        // set pointers to NULL
        mDriverList = NULL;
        mTextureManager = NULL;
        mHardwareBufferManager = NULL;
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

        mBoundProgram.fill(NULL);

        ZeroMemory( &mBlendDesc, sizeof(mBlendDesc));

        ZeroMemory( &mRasterizerDesc, sizeof(mRasterizerDesc));
        mRasterizerDesc.FrontCounterClockwise = true;
		mRasterizerDesc.DepthClipEnable = true;
        mRasterizerDesc.MultisampleEnable = true;


        ZeroMemory( &mDepthStencilDesc, sizeof(mDepthStencilDesc));

        ZeroMemory( &mDepthStencilDesc, sizeof(mDepthStencilDesc));
        ZeroMemory( &mScissorRect, sizeof(mScissorRect));

        mPolygonMode = PM_SOLID;
        mRasterizerDesc.FillMode = D3D11Mappings::get(mPolygonMode);

        //sets the modification trackers to true
        mBlendDescChanged = true;
		mRasterizerDescChanged = true;
		mDepthStencilDescChanged = true;
		mSamplerStatesChanged = true;
		mLastTextureUnitState = 0;

        ZeroMemory(mTexStageDesc, OGRE_MAX_TEXTURE_LAYERS * sizeof(sD3DTextureStageDesc));

        mLastVertexSourceCount = 0;
        mReadBackAsTexture = false;

        ID3D11DeviceN * device = createD3D11Device(NULL, D3D_DRIVER_TYPE_HARDWARE, mMinRequestedFeatureLevel, mMaxRequestedFeatureLevel, 0);
        mDevice.TransferOwnership(device);
    }
    //---------------------------------------------------------------------
    void D3D11RenderSystem::getCustomAttribute(const String& name, void* pData)
    {
        if( name == "D3DDEVICE" )
        {
            *(ID3D11DeviceN**)pData = mDevice.get();
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Attribute not found: " + name, "RenderSystem::getCustomAttribute");
        }
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
}
