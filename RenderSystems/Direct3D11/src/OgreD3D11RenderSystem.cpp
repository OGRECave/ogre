/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#include "OgreLogManager.h"
#include "OgreD3D11HardwareBufferManager.h"
#include "OgreD3D11HardwareIndexBuffer.h"
#include "OgreD3D11HardwareVertexBuffer.h"
#include "OgreD3D11VertexDeclaration.h"
#include "OgreD3D11GpuProgram.h"
#include "OgreD3D11GpuProgramManager.h"
#include "OgreD3D11HLSLProgramFactory.h"

#include "OgreD3D11HardwareOcclusionQuery.h"
#include "OgreFrustum.h"
#include "OgreD3D11MultiRenderTarget.h"
#include "OgreD3D11HLSLProgram.h"
#include "OgreD3D11VertexDeclaration.h"

#include "OgreD3D11DepthBuffer.h"
#include "OgreD3D11HardwarePixelBuffer.h"
#include "OgreException.h"

// DXGetErrorDescription
#include "DXErr.h"

//---------------------------------------------------------------------
#define FLOAT2DWORD(f) *((DWORD*)&f)
//---------------------------------------------------------------------

namespace Ogre 
{

	//---------------------------------------------------------------------
    D3D11RenderSystem::D3D11RenderSystem( HINSTANCE hInstance ) : mDevice(NULL)
	{
		LogManager::getSingleton().logMessage( "D3D11 : " + getName() + " created." );

#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
		mEnableFixedPipeline = false;
#endif

		// set the instance being passed 
		mhInstance = hInstance;

		mRenderSystemWasInited = false;
		initRenderSystem();

		// set config options defaults
		initConfigOptions();



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

		//SAFE_RELEASE( mpD3D );

		LogManager::getSingleton().logMessage( "D3D11 : " + getName() + " destroyed." );
	}
	//---------------------------------------------------------------------
	const String& D3D11RenderSystem::getName() const
	{
		static String strName( "Direct3D11 Rendering Subsystem");
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
	bool D3D11RenderSystem::_checkMultiSampleQuality(UINT SampleCount, UINT *outQuality, DXGI_FORMAT format)
	{
		// TODO: check if we need this function
		HRESULT hr;
		hr = mDevice->CheckMultisampleQualityLevels( 
				format,
			SampleCount,
			outQuality);

		if (SUCCEEDED(hr))
			return true;
		else
			return false;
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
		ConfigOption optAA;
		ConfigOption optFPUMode;
		ConfigOption optNVPerfHUD;
		ConfigOption optSRGB;
		ConfigOption optExceptionsErrorLevel;
		ConfigOption optDriverType;



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


		// Exceptions Error Level
		optExceptionsErrorLevel.name = "Information Queue Exceptions Bottom Level";
		optExceptionsErrorLevel.possibleValues.push_back("No information queue exceptions");
		optExceptionsErrorLevel.possibleValues.push_back("Corruption");
		optExceptionsErrorLevel.possibleValues.push_back("Error");
		optExceptionsErrorLevel.possibleValues.push_back("Warning");
		optExceptionsErrorLevel.possibleValues.push_back("Info (exception on any message)");
#ifdef OGRE_DEBUG_MODE
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
		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVSync.name] = optVSync;
		mOptions[optVSyncInterval.name] = optVSyncInterval;
		mOptions[optAA.name] = optAA;
		mOptions[optFPUMode.name] = optFPUMode;
		mOptions[optNVPerfHUD.name] = optNVPerfHUD;
		mOptions[optSRGB.name] = optSRGB;
		mOptions[optExceptionsErrorLevel.name] = optExceptionsErrorLevel;
		mOptions[optDriverType.name] = optDriverType;
		
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
			StringUtil::StrStreamType str;
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


		if( name == "VSync" )
		{
			if (value == "Yes")
				mVSync = true;
			else
				mVSync = false;
		}

		if( name == "VSync Interval" )
		{
			mVSyncInterval = StringConverter::parseUnsignedInt(value);
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
        optFSAA->possibleValues.push_back("0");

        it = mOptions.find("Rendering Device");
        D3D11Driver *driver = getDirect3DDrivers()->item(it->second.currentValue);
        if (driver)
        {
            it = mOptions.find("Video Mode");
            D3D11VideoMode *videoMode = driver->getVideoModeList()->item(it->second.currentValue);
            if (videoMode)
            {
                UINT numLevels = 0;
				 bool bOK=false;
                // set maskable levels supported
                for (unsigned int n = 2; n < 17; n++)
                {
                    bOK = this->_checkMultiSampleQuality(
                        n, 
                        &numLevels, 
                        videoMode->getFormat()
                        );
                    if (bOK)
					{
                        optFSAA->possibleValues.push_back(StringConverter::toString(n));
						if (n >=8)
						{
							optFSAA->possibleValues.push_back(StringConverter::toString(n) + " [Quality]");
						}
					}
                }
            }
        }

        // Reset FSAA to none if previous doesn't avail in new possible values
        StringVector::const_iterator itValue =
            std::find(optFSAA->possibleValues.begin(),
                      optFSAA->possibleValues.end(),
                      optFSAA->currentValue);
        if (itValue == optFSAA->possibleValues.end())
        {
            optFSAA->currentValue = "0";
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

        it = mOptions.find( "VSync" );
		if( it->second.currentValue == "Yes" )
			mVSync = true;
		else
			mVSync = false;

        return StringUtil::BLANK;
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
			if (!mDevice.isNull())
			{
				mDevice.release();
			}

		
			opt = mOptions.find( "Information Queue Exceptions Bottom Level" );
			if( opt == mOptions.end() )
				OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find Information Queue Exceptions Bottom Level option!", "D3D11RenderSystem::initialise" );
			String infoQType = opt->second.currentValue;

			if ("No information queue exceptions" == infoQType)
			{
				D3D11Device::setExceptionsErrorLevel(D3D11Device::D3D_NO_EXCEPTION);
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


			UINT deviceFlags = 0;
			if (D3D11Device::D3D_NO_EXCEPTION != D3D11Device::getExceptionsErrorLevel())
			{
				deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			}
			if (!OGRE_THREAD_SUPPORT)
			{
				deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
			}
			D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN ;

			// Search for a PerfHUD adapter
			UINT nAdapter = 0;
			IDXGIAdapter1* pAdapter = NULL;
			IDXGIAdapter1* pSelectedAdapter = mActiveD3DDriver->getDeviceAdapter();
			if ( mUseNVPerfHUD )
			{
				// Search for a PerfHUD adapter
				while( mpDXGIFactory->EnumAdapters1( nAdapter, &pAdapter ) != DXGI_ERROR_NOT_FOUND )
				{
					if ( pAdapter )
					{
						DXGI_ADAPTER_DESC1 adaptDesc;
						if ( SUCCEEDED( pAdapter->GetDesc1( &adaptDesc ) ) )
						{
							const bool isPerfHUD = wcscmp( adaptDesc.Description, L"NVIDIA PerfHUD" ) == 0;
							if ( isPerfHUD )
							{
								pSelectedAdapter = pAdapter;
								driverType = D3D_DRIVER_TYPE_REFERENCE;
							}
						}
						++nAdapter;
					}
				}

			}


			// Since June 2010 SDK, there's no need to load any other DLL
			/*
			HMODULE Software3d310Dll = NULL;
			if (mDriverType == DT_SOFTWARE)
			{
				driverType = D3D_DRIVER_TYPE_SOFTWARE; 
				pSelectedAdapter = NULL;
				Software3d310Dll = LoadLibrary(TEXT("D3D11Ref.dll"));
				if (Software3d310Dll == NULL) 
				{
					OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
						"Failed to load Direct3D11 software DLL (D3D11Ref.dll)", 
						"D3D11RenderSystem::D3D11RenderSystem" );

				}
			}
			else if (mDriverType == DT_WARP)
			{
				// you have to use D3D_DRIVER_TYPE_SOFTWARE (D3D_DRIVER_TYPE_WARP doesn't work)
				driverType = D3D_DRIVER_TYPE_SOFTWARE; 
				pSelectedAdapter = NULL;

				Software3d310Dll = LoadLibrary(TEXT("D3D10WARP.dll"));
				if (Software3d310Dll == NULL) 
				{
					// try to load the beta that was released
					Software3d310Dll = LoadLibrary(TEXT("D3D10WARP_beta.dll"));
					if (Software3d310Dll == NULL) 
					{
						OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
							"Failed to load Direct3D11 Wrap DLL (D3D11WARP.dll or D3D11WARP_beta.dll)", 
							"D3D11RenderSystem::D3D11RenderSystem" );

					}
				}
			}
			*/

			D3D_FEATURE_LEVEL RequestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
			unsigned int RequestedLevelsSize = sizeof( RequestedLevels ) / sizeof( RequestedLevels[0] );
			ID3D11Device * device;
			// But, if creating WARP or software, don't use a selected adapter, it will be selected automatically
			if(FAILED(D3D11CreateDevice(mDriverType != DT_WARP ? pSelectedAdapter : NULL,
										driverType,
										NULL,
										deviceFlags, 
										RequestedLevels, 
										RequestedLevelsSize,
										D3D11_SDK_VERSION, 
										&device, 
										0, 
										0)))         
			{
				OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
					"Failed to create Direct3D11 object", 
					"D3D11RenderSystem::D3D11RenderSystem" );
			}

			if (mDriverType != DT_HARDWARE)
			{
				// get the IDXGIFactory1 from the device for software drivers
				// Remark(dec-09):
				//  Seems that IDXGIFactory1::CreateSoftwareAdapter doesn't work with
				// D3D11CreateDevice - so I needed to create with pSelectedAdapter = 0.
				// If pSelectedAdapter == 0 then you have to get the IDXGIFactory1 from
				// the device - else CreateSwapChain fails later.
				//  Update (Jun 12, 2012)
				// If using WARP driver, get factory from created device
				SAFE_RELEASE(mpDXGIFactory);

				IDXGIDevice1 * pDXGIDevice;
				device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDXGIDevice);

				IDXGIAdapter1 * pDXGIAdapter;
				pDXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&pDXGIAdapter);

				pDXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&mpDXGIFactory);

				SAFE_RELEASE(pDXGIAdapter);
				SAFE_RELEASE(pDXGIDevice);
			}


			mDevice = D3D11Device(device) ;
		}




		// get driver version
		// TODO: no wayto do this on Dx11? Can't find a driver version structure
		/*
		mDriverVersion.major = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
		mDriverVersion.minor = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
		mDriverVersion.release = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);
		mDriverVersion.build = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);
		*/


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

				// In full screen we only want to allow supported resolutions, so temp and opt->second.currentValue need to 
				// match exacly, but in windowed mode we can allow for arbitrary window sized, so we only need
				// to match the colour values
				if(fullScreen && (temp == opt->second.currentValue) ||
				  !fullScreen && (temp.substr(temp.rfind('@')+1) == colourDepth))
				{
					videoMode = mActiveD3DDriver->getVideoModeList()->item(j);
					break;
				}
			}

			if( !videoMode )
				OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find requested video mode.", "D3D11RenderSystem::initialise" );
			
			// sRGB window option
			bool hwGamma = false;
			opt = mOptions.find( "sRGB Gamma Conversion" );
			if( opt == mOptions.end() )
				OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find sRGB option!", "D3D9RenderSystem::initialise" );
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
			

			NameValuePairList miscParams;
			miscParams["colourDepth"] = StringConverter::toString(videoMode->getColourDepth());
			miscParams["FSAA"] = StringConverter::toString(fsaa);
			miscParams["FSAAHint"] = fsaaHint;
			miscParams["vsync"] = StringConverter::toString(mVSync);
			miscParams["vsyncInterval"] = StringConverter::toString(mVSyncInterval);
			miscParams["useNVPerfHUD"] = StringConverter::toString(mUseNVPerfHUD);
			miscParams["gamma"] = StringConverter::toString(hwGamma);

			autoWindow = this->_createRenderWindow( windowTitle, width, height, 
				fullScreen, &miscParams );

            // If we have 16bit depth buffer enable w-buffering.
            assert( autoWindow );
            if ( autoWindow->getColourDepth() == 16 ) 
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


		return autoWindow;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::reinitialise()
	{
		LogManager::getSingleton().logMessage( "D3D11 : Reinitializing" );
		this->shutdown();
	//	this->initialise( true );
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::shutdown()
	{
		RenderSystem::shutdown();

		mRenderSystemWasInited = false;

		mPrimaryWindow = NULL; // primary window deleted by base class.
		freeDevice();
		SAFE_DELETE( mDriverList );
		SAFE_RELEASE( mpDXGIFactory );
		mActiveD3DDriver = NULL;
		mDevice = NULL;
		mBasicStatesInitialised = false;
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
		if (mPrimaryWindow && mPrimaryWindow->isFullScreen())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Cannot create secondary windows when the primary is full screen",
				"D3D11RenderSystem::_createRenderWindow");
		}
		if (mPrimaryWindow && fullScreen)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Cannot create full screen secondary windows",
				"D3D11RenderSystem::_createRenderWindow");
		}
		
		// Log a message
		StringStream ss;
		ss << "D3D11RenderSystem::_createRenderWindow \"" << name << "\", " <<
			width << "x" << height << " ";
		if(fullScreen)
			ss << "fullscreen ";
		else
			ss << "windowed ";
		if(miscParams)
		{
			ss << " miscParams: ";
			NameValuePairList::const_iterator it;
			for(it=miscParams->begin(); it!=miscParams->end(); ++it)
			{
				ss << it->first << "=" << it->second << " ";
			}
			LogManager::getSingleton().logMessage(ss.str());
		}
		
		String msg;

		// Make sure we don't already have a render target of the 
		// sam name as the one supplied
		if( mRenderTargets.find( name ) != mRenderTargets.end() )
		{
			msg = "A render target of the same name '" + name + "' already "
				"exists.  You cannot create a new window with this name.";
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, msg, "D3D11RenderSystem::_createRenderWindow" );
		}

		RenderWindow* win = new D3D11RenderWindow(mhInstance, mDevice, mpDXGIFactory);

		win->create( name, width, height, fullScreen, miscParams);

		attachRenderTarget( *win );

		// If this is the first window, get the D3D device and create the texture manager
		if( !mPrimaryWindow )
		{
			mPrimaryWindow = (D3D11RenderWindow *)win;
			win->getCustomAttribute( "D3DDEVICE", &mDevice );

			// Create the texture manager for use by others
			mTextureManager = new D3D11TextureManager( mDevice );
            // Also create hardware buffer manager
            mHardwareBufferManager = new D3D11HardwareBufferManager(mDevice);

			// Create the GPU program manager
			mGpuProgramManager = new D3D11GpuProgramManager(mDevice);
            // create & register HLSL factory
			if (mHLSLProgramFactory == NULL)
				mHLSLProgramFactory = new D3D11HLSLProgramFactory(mDevice);
			mRealCapabilities = createRenderSystemCapabilities();							
			mRealCapabilities->addShaderProfile("hlsl");

			// if we are using custom capabilities, then 
			// mCurrentCapabilities has already been loaded
			if(!mUseCustomCapabilities)
				mCurrentCapabilities = mRealCapabilities;

			initialiseFromRenderSystemCapabilities(mCurrentCapabilities, mPrimaryWindow);

		}
		else
		{
			mSecondaryWindows.push_back(static_cast<D3D11RenderWindow *>(win));
		}

		return win;

	}
    //---------------------------------------------------------------------
	RenderSystemCapabilities* D3D11RenderSystem::createRenderSystemCapabilities() const
    {
		RenderSystemCapabilities* rsc = new RenderSystemCapabilities();
		rsc->setDriverVersion(mDriverVersion);
		rsc->setDeviceName(mActiveD3DDriver->DriverDescription());
		rsc->setRenderSystemName(getName());

		// Does NOT support fixed-function!
		//rsc->setCapability(RSC_FIXED_FUNCTION);

		rsc->setCapability(RSC_HWSTENCIL);
		rsc->setStencilBufferBitDepth(8);

		// Set number of texture units, always 16
		rsc->setNumTextureUnits(16);
		rsc->setCapability(RSC_ANISOTROPY);
		rsc->setCapability(RSC_AUTOMIPMAP);
		rsc->setCapability(RSC_BLENDING);
		rsc->setCapability(RSC_DOT3);
		// Cube map
		rsc->setCapability(RSC_CUBEMAPPING);

		// We always support compression, D3DX will decompress if device does not support
		rsc->setCapability(RSC_TEXTURE_COMPRESSION);
		rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
		rsc->setCapability(RSC_VBO);
		rsc->setCapability(RSC_SCISSOR_TEST);
		rsc->setCapability(RSC_TWO_SIDED_STENCIL);
		rsc->setCapability(RSC_STENCIL_WRAP);
		rsc->setCapability(RSC_HWOCCLUSION);

		convertVertexShaderCaps(rsc);
		convertPixelShaderCaps(rsc);
		convertGeometryShaderCaps(rsc);

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
				rsc->setVendor(GPU_ATI);
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
		rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);
		rsc->setCapability(RSC_TEXTURE_FLOAT);

		rsc->setNumMultiRenderTargets(std::min(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, (int)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
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

		return rsc;

    }
	//-----------------------------------------------------------------------
	void D3D11RenderSystem::initialiseFromRenderSystemCapabilities(
		RenderSystemCapabilities* caps, RenderTarget* primary)
	{
		if(caps->getRenderSystemName() != getName())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"Trying to initialize GLRenderSystem from RenderSystemCapabilities that do not support Direct3D11",
				"D3D11RenderSystem::initialiseFromRenderSystemCapabilities");
		}
		
		// add hlsl
		HighLevelGpuProgramManager::getSingleton().addFactory(mHLSLProgramFactory);

		Log* defaultLog = LogManager::getSingleton().getDefaultLog();
		if (defaultLog)
		{
			caps->log(defaultLog);
		}
	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::convertVertexShaderCaps(RenderSystemCapabilities* rsc) const
    {

		rsc->addShaderProfile("vs_4_0");
		rsc->addShaderProfile("vs_4_1");
		rsc->addShaderProfile("vs_5_0");

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
		
		rsc->addShaderProfile("ps_4_0");
		rsc->addShaderProfile("ps_4_1");
		rsc->addShaderProfile("ps_5_0");

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
	void D3D11RenderSystem::convertGeometryShaderCaps(RenderSystemCapabilities* rsc) const
	{

		rsc->addShaderProfile("gs_4_0");
		rsc->addShaderProfile("gs_4_1");
		rsc->addShaderProfile("gs_5_0");

		rsc->setCapability(RSC_GEOMETRY_PROGRAM);
		rsc->setCapability(RSC_HWRENDER_TO_VERTEX_BUFFER);

		rsc->setGeometryProgramConstantFloatCount(0);
		rsc->setGeometryProgramConstantIntCount(0);
		rsc->setGeometryProgramConstantBoolCount(0);
		rsc->setGeometryProgramNumOutputVertices(0);
		/*
		/// The number of floating-point constants geometry programs support
		void setGeometryProgramConstantFloatCount(ushort c)
		{
			mGeometryProgramConstantFloatCount = c;           
		}
		/// The number of integer constants geometry programs support
		void setGeometryProgramConstantIntCount(ushort c)
		{
			mGeometryProgramConstantIntCount = c;           
		}
		/// The number of boolean constants geometry programs support
		void setGeometryProgramConstantBoolCount(ushort c)
		{
			mGeometryProgramConstantBoolCount = c;           
		}
		/// Set the number of vertices a single geometry program run can emit
		void setGeometryProgramNumOutputVertices(int numOutputVertices)
		{
		mGeometryProgramNumOutputVertices = numOutputVertices;
		}
		/// Get the number of vertices a single geometry program run can emit
		int getGeometryProgramNumOutputVertices(void) const
		{
		return mGeometryProgramNumOutputVertices;
		}

*/
/*		// TODO: constant buffers have no limits but lower models do
		// 16 boolean params allowed
		rsc->setFragmentProgramConstantBoolCount(16);
		// 16 integer params allowed, 4D
		rsc->setFragmentProgramConstantIntCount(16);
		// float params, always 4D
		rsc->setFragmentProgramConstantFloatCount(512);
*/
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
	DepthBuffer* D3D11RenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
	{
		//Get surface data (mainly to get MSAA data)
		D3D11HardwarePixelBuffer *pBuffer;
		renderTarget->getCustomAttribute( "BUFFER", &pBuffer );
		D3D11_TEXTURE2D_DESC BBDesc;
		static_cast<ID3D11Texture2D*>(pBuffer->getParentTexture()->getTextureResource())->GetDesc( &BBDesc );

		// Create depth stencil texture
		ID3D11Texture2D* pDepthStencil = NULL;
		D3D11_TEXTURE2D_DESC descDepth;

		descDepth.Width					= renderTarget->getWidth();
		descDepth.Height				= renderTarget->getHeight();
		descDepth.MipLevels				= 1;
		descDepth.ArraySize				= BBDesc.ArraySize;
		descDepth.Format				= DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count		= BBDesc.SampleDesc.Count;
		descDepth.SampleDesc.Quality	= BBDesc.SampleDesc.Quality;
		descDepth.Usage					= D3D11_USAGE_DEFAULT;
		descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags		= 0;
		descDepth.MiscFlags				= 0;

		if (descDepth.ArraySize == 6)
		{
			descDepth.MiscFlags		|= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}


		HRESULT hr = mDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil );
		if( FAILED(hr) || mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unable to create depth texture\nError Description:" + errorDescription,
				"D3D11RenderSystem::_createDepthBufferFor");
		}

		// Create the depth stencil view
		ID3D11DepthStencilView		*depthStencilView;
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory( &descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );

		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = (BBDesc.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = mDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &depthStencilView );
		SAFE_RELEASE( pDepthStencil );
		if( FAILED(hr) )
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Unable to create depth stencil view\nError Description:" + errorDescription,
				"D3D11RenderSystem::_createDepthBufferFor");
		}

		//Create the abstract container
		D3D11DepthBuffer *newDepthBuffer = new D3D11DepthBuffer( DepthBuffer::POOL_DEFAULT, depthStencilView,
												descDepth.Width, descDepth.Height,
												descDepth.SampleDesc.Count, descDepth.SampleDesc.Quality,
												false );

		return newDepthBuffer;
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
		D3D11DepthBuffer *newDepthBuffer = new D3D11DepthBuffer( DepthBuffer::POOL_DEFAULT, depthSurface,
																	width, height, fsaa, fsaaQuality, true );

		//Add the 'main' depth buffer to the pool
		mDepthBufferPool[newDepthBuffer->getPoolId()].push_back( newDepthBuffer );

		return newDepthBuffer;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::destroyRenderTarget(const String& name)
	{
		// Check in specialised lists
		if (mPrimaryWindow->getName() == name)
		{
			// We're destroying the primary window, so reset device and window
			mPrimaryWindow = 0;
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
			mDevice.release();
			//mActiveD3DDriver->setDevice(D3D11Device(NULL));
			mDevice = 0;

		}


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
	void D3D11RenderSystem::setAmbientLight( float r, float g, float b )
	{
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
	void D3D11RenderSystem::_setTexture( size_t stage, bool enabled, const TexturePtr& tex )
	{
		static D3D11TexturePtr dt;
		dt = tex;
		if (enabled)
		{
			// note used
			dt->touch();
			ID3D11ShaderResourceView * pTex = dt->getTexture();
			mTexStageDesc[stage].pTex = pTex;
			mTexStageDesc[stage].used = true;
			mTexStageDesc[stage].type = dt->getTextureType();
		}
		else
		{
			mTexStageDesc[stage].used = false;
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setVertexTexture(size_t stage, const TexturePtr& tex)
	{
		if (tex.isNull())
			_setTexture(stage, false, tex);
        else
			_setTexture(stage, true, tex);	
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
		// record the stage state
		mTexStageDesc[stage].autoTexCoordType = m;
		mTexStageDesc[stage].frustum = frustum;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureMipmapBias(size_t unit, float bias)
	{

	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureMatrix( size_t stage, const Matrix4& xForm )
	{
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureAddressingMode( size_t stage, 
		const TextureUnitState::UVWAddressingMode& uvw )
	{
		// record the stage state
		mTexStageDesc[stage].samplerDesc.AddressU = D3D11Mappings::get(uvw.u);
		mTexStageDesc[stage].samplerDesc.AddressV = D3D11Mappings::get(uvw.v);
		mTexStageDesc[stage].samplerDesc.AddressW = D3D11Mappings::get(uvw.w);
	}
    //-----------------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureBorderColour(size_t stage,
        const ColourValue& colour)
    {
		D3D11Mappings::get(colour, mTexStageDesc[stage].samplerDesc.BorderColor);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureBlendMode( size_t stage, const LayerBlendModeEx& bm )
	{
		if (bm.blendType == LBT_COLOUR)
		{
			mTexStageDesc[stage].layerBlendMode = bm;
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op /*= SBO_ADD*/ )
	{
		if( sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
		{
			mBlendDesc.RenderTarget[0].BlendEnable = FALSE;
		}
		else
		{
			mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
			mBlendDesc.RenderTarget[0].SrcBlend = D3D11Mappings::get(sourceFactor);
			mBlendDesc.RenderTarget[0].DestBlend = D3D11Mappings::get(destFactor);
			mBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD ;
			mBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD ;
			mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			mBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			mBlendDesc.AlphaToCoverageEnable = mSceneAlphaToCoverage;

			mBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha, SceneBlendOperation op /*= SBO_ADD*/, SceneBlendOperation alphaOp /*= SBO_ADD*/ )
	{
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage )
	{
		mSceneAlphaRejectFunc	= func;
		mSceneAlphaRejectValue	= value;
		mSceneAlphaToCoverage	= alphaToCoverage;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setCullingMode( CullingMode mode )
	{
		mCullingMode = mode;
		mRasterizerDesc.CullMode = D3D11Mappings::get(mode);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
	{
		_setDepthBufferCheckEnabled( depthTest );
		_setDepthBufferWriteEnabled( depthWrite );
		_setDepthBufferFunction( depthFunction );
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setDepthBufferCheckEnabled( bool enabled )
	{
		mDepthStencilDesc.DepthEnable = enabled;
		//mRasterizerDesc.DepthClipEnable = enabled;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setDepthBufferWriteEnabled( bool enabled )
	{
		if (enabled)
		{
			mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		}
		else
		{
			mDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setDepthBufferFunction( CompareFunction func )
	{
		mDepthStencilDesc.DepthFunc = D3D11Mappings::get(func);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
	{
		mRasterizerDesc.DepthBiasClamp = constantBias;
		mRasterizerDesc.SlopeScaledDepthBias = slopeScaleBias;

	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setColourBufferWriteEnabled(bool red, bool green, 
		bool blue, bool alpha)
	{
		UINT8 val = 0;
		if (red) 
			val |= D3D11_COLOR_WRITE_ENABLE_RED;
		if (green)
			val |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		if (blue)
			val |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		if (alpha)
			val |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

		mBlendDesc.RenderTarget[0].RenderTargetWriteMask = val; 
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setFog( FogMode mode, const ColourValue& colour, Real densitiy, Real start, Real end )
	{
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setPolygonMode(PolygonMode level)
	{
		mPolygonMode = level;
		mRasterizerDesc.FillMode = D3D11Mappings::get(mPolygonMode);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::setStencilCheckEnabled(bool enabled)
	{
		mDepthStencilDesc.StencilEnable = enabled;
	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setStencilBufferParams(CompareFunction func, 
        uint32 refValue, uint32 mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp, 
        bool twoSidedOperation)
    {
		mDepthStencilDesc.FrontFace.StencilFunc = D3D11Mappings::get(func);
		mDepthStencilDesc.BackFace.StencilFunc = D3D11Mappings::get(func);

		mStencilRef = refValue;
		mDepthStencilDesc.StencilReadMask = refValue;
		mDepthStencilDesc.StencilWriteMask = mask;

		mDepthStencilDesc.FrontFace.StencilFailOp = D3D11Mappings::get(stencilFailOp);
		mDepthStencilDesc.BackFace.StencilFailOp = D3D11Mappings::get(stencilFailOp);

		mDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11Mappings::get(stencilFailOp);
		mDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11Mappings::get(stencilFailOp);

		mDepthStencilDesc.FrontFace.StencilPassOp = D3D11Mappings::get(passOp);
		mDepthStencilDesc.BackFace.StencilPassOp = D3D11Mappings::get(passOp);

		if (!twoSidedOperation)
		{
			mDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
		}

	}
	//---------------------------------------------------------------------
    void D3D11RenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, 
        FilterOptions filter)
	{
		switch(ftype) {
		case FT_MIN:
			FilterMinification = filter;
			break;
		case FT_MAG:
			FilterMagnification = filter;
			break;
		case FT_MIP:
			FilterMips = filter;
			break;
		}

		mTexStageDesc[unit].samplerDesc.Filter = D3D11Mappings::get(FilterMinification, FilterMagnification, FilterMips);

	}
    //---------------------------------------------------------------------
	DWORD D3D11RenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		return 0;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
	{
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setRenderTarget(RenderTarget *target)
	{
		mActiveRenderTarget = target;
		if (mActiveRenderTarget)
		{
			ID3D11RenderTargetView ** pRTView = NULL;
			target->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );

			uint numberOfViews;
			target->getCustomAttribute( "numberOfViews", &numberOfViews );

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


			// we need to clear the state 
			mDevice.GetImmediateContext()->ClearState();

			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot Clear State\nError Description:" + errorDescription,
					"D3D11RenderSystem::_setViewport");
			}



			// now switch to the new render target
			mDevice.GetImmediateContext()->OMSetRenderTargets(
				numberOfViews,
				pRTView,
				depthBuffer ? depthBuffer->getDepthStencilView() : 0 );

			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set render target\nError Description:" + errorDescription,
					"D3D11RenderSystem::_setViewport");
			}
		}
		// TODO - support MRT

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
		//	HRESULT hr;

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
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set viewports\nError Description:" + errorDescription,
					"D3D11RenderSystem::_setViewport");
			}

			// Set sRGB write mode
			//__SetRenderState(D3DRS_SRGBWRITEENABLE, target->isHardwareGammaEnabled());
			// TODO where has sRGB state gone?
			
			vp->_clearUpdatedFlag();
		}
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_beginFrame()
	{
	
		if( !mActiveViewport )
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Cannot begin frame - no viewport selected.", "D3D11RenderSystem::_beginFrame" );
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
      	D3D11VertexDeclaration* d3ddecl = 
			static_cast<D3D11VertexDeclaration*>(decl);

		d3ddecl->bindToShader(mBoundVertexProgram, binding);
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
			const D3D11HardwareVertexBuffer* d3d11buf = 
				static_cast<const D3D11HardwareVertexBuffer*>(i->second.get());

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
		}

		mLastVertexSourceCount = binds.size();		
	}

	//---------------------------------------------------------------------
	// TODO: Move this class to the right place.
	class D3D11RenderOperationState : public Renderable::RenderSystemData
	{
	public:
		ID3D11BlendState * mBlendState;
		ID3D11RasterizerState * mRasterizer;
		ID3D11DepthStencilState * mDepthStencilState;

		ID3D11SamplerState * mSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
		size_t mSamplerStatesCount;

		ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS];
		size_t mTexturesCount;

        D3D11RenderOperationState() :
              mBlendState(NULL)
            , mRasterizer(NULL)
            , mDepthStencilState(NULL)
            , mSamplerStatesCount(0)
            , mTexturesCount(0)
        {
            for (size_t i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
            {
                mSamplerStates[i] = 0;
            }
        }


		~D3D11RenderOperationState()
		{
			SAFE_RELEASE( mBlendState );
			SAFE_RELEASE( mRasterizer );
			SAFE_RELEASE( mDepthStencilState );

			for (size_t i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
			{
				SAFE_RELEASE( mSamplerStates[i] );
			}
		}
	};

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render(const RenderOperation& op)
	{

		// Exit immediately if there is nothing to render
		if (op.vertexData==0 || op.vertexData->vertexCount == 0)
        {
			return;
        }

        HardwareVertexBufferSharedPtr globalInstanceVertexBuffer = getGlobalInstanceVertexBuffer();
        VertexDeclaration* globalVertexDeclaration = getGlobalInstanceVertexBufferVertexDeclaration();

        bool hasInstanceData = op.useGlobalInstancingVertexBufferIsAvailable &&
                    !globalInstanceVertexBuffer.isNull() && globalVertexDeclaration != NULL 
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

		//if(!opState)
		{
			//opState = new D3D11RenderOperationState;

			HRESULT hr = mDevice->CreateBlendState(&mBlendDesc, &opState->mBlendState) ;
			if (FAILED(hr))
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Failed to create blend state\nError Description:" + errorDescription, 
					"D3D11RenderSystem::_render" );
			}

			hr = mDevice->CreateRasterizerState(&mRasterizerDesc, &opState->mRasterizer) ;
			if (FAILED(hr))
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Failed to create rasterizer state\nError Description:" + errorDescription, 
					"D3D11RenderSystem::_render" );
			}

			hr = mDevice->CreateDepthStencilState(&mDepthStencilDesc, &opState->mDepthStencilState) ;
			if (FAILED(hr))
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"Failed to create depth stencil state\nError Description:" + errorDescription, 
					"D3D11RenderSystem::_render" );
			}

			// samplers mapping
			size_t numberOfSamplers = 0;
			opState->mTexturesCount = 0;

			for (size_t n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
			{
				sD3DTextureStageDesc & stage = mTexStageDesc[n];
				if(!stage.used)
				{
					break;
				}

				numberOfSamplers++;

				ID3D11ShaderResourceView * texture;
				texture = stage.pTex;
				opState->mTextures[opState->mTexturesCount] = texture;
				opState->mTexturesCount++;

				stage.samplerDesc.ComparisonFunc = D3D11Mappings::get(mSceneAlphaRejectFunc);
				stage.samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				stage.currentSamplerDesc = stage.samplerDesc;

				ID3D11SamplerState * samplerState;

				HRESULT hr = mDevice->CreateSamplerState(&stage.samplerDesc, &samplerState) ;
				if (FAILED(hr))
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
						"Failed to create sampler state\nError Description:" + errorDescription,
						"D3D11RenderSystem::_render" );
				}
				opState->mSamplerStates[n] = (samplerState);		
			}
			opState->mSamplerStatesCount = numberOfSamplers;
		}

		//if (opState->mBlendState != mBoundBlendState)
		{
			mBoundBlendState = opState->mBlendState ;
			mDevice.GetImmediateContext()->OMSetBlendState(opState->mBlendState, 0, 0xffffffff); // TODO - find out where to get the parameters
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set blend state\nError Description:" + errorDescription,
					"D3D11RenderSystem::_render");
			}
            // PPP: TO DO. Must bind only if the Geometry shader expects this
			if (mBoundGeometryProgram)
			{
				{
					mDevice.GetImmediateContext()->GSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
					if (mDevice.isError())
					{
						String errorDescription = mDevice.getErrorDescription();
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
							"D3D11 device cannot set pixel shader samplers\nError Description:" + errorDescription,
							"D3D11RenderSystem::_render");
					}
				}
				mDevice.GetImmediateContext()->GSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
						"D3D11RenderSystem::_render");
				}
			}
		}



		//if (opState->mRasterizer != mBoundRasterizer)
		{
			mBoundRasterizer = opState->mRasterizer ;

			mDevice.GetImmediateContext()->RSSetState(opState->mRasterizer);
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set rasterizer state\nError Description:" + errorDescription,
					"D3D11RenderSystem::_render");
			}
		}
		

		//if (opState->mDepthStencilState != mBoundDepthStencilState)
		{
			mBoundDepthStencilState = opState->mDepthStencilState ;

			mDevice.GetImmediateContext()->OMSetDepthStencilState(opState->mDepthStencilState, mStencilRef); 
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set depth stencil state\nError Description:" + errorDescription,
					"D3D11RenderSystem::_render");
			}
		}

		if (opState->mSamplerStatesCount > 0 ) //  if the NumSamplers is 0, the operation effectively does nothing.
		{
			// Assaf: seem I have better performance without this check... TODO - remove?
		//	// if ((mBoundSamplerStatesCount != opState->mSamplerStatesCount) || ( 0 != memcmp(opState->mSamplerStates, mBoundSamplerStates, mBoundSamplerStatesCount) ) )
			{
				//mBoundSamplerStatesCount = opState->mSamplerStatesCount;
				//memcpy(mBoundSamplerStates,opState->mSamplerStates, mBoundSamplerStatesCount);
				mDevice.GetImmediateContext()->PSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(opState->mSamplerStatesCount), opState->mSamplerStates);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D11 device cannot set pixel shader samplers\nError Description:" + errorDescription,
						"D3D11RenderSystem::_render");
				}


			}
			mDevice.GetImmediateContext()->PSSetShaderResources(static_cast<UINT>(0), static_cast<UINT>(opState->mTexturesCount), &opState->mTextures[0]);
			if (mDevice.isError())
			{
				String errorDescription = mDevice.getErrorDescription();
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
					"D3D11 device cannot set pixel shader resources\nError Description:" + errorDescription,
					"D3D11RenderSystem::_render");
			}
		}

		ID3D11Buffer* pSOTarget=0;
		// Mustn't bind a emulated vertex, pixel shader (see below), if we are rendering to a stream out buffer
		mDevice.GetImmediateContext()->SOGetTargets(1, &pSOTarget);

	 	if (!mBoundVertexProgram ||
			 (!mBoundFragmentProgram && op.operationType != RenderOperation::OT_POINT_LIST && pSOTarget==0 ) 
		   ) 
		{
			
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Attempted to render to a D3D11 device without both vertex and fragment shaders there is no fixed pipeline in d3d11 - use the RTSS or write custom shaders.",
				"D3D11RenderSystem::_render");
		}

		if (mDevice.isError())
		{
			// this will never happen but we want to be consistent with the error checks... 
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set geometry shader to null\nError Description:" + errorDescription,
				"D3D11RenderSystem::_render");
		}

		setVertexDeclaration(op.vertexData->vertexDeclaration, op.vertexData->vertexBufferBinding);
		setVertexBufferBinding(op.vertexData->vertexBufferBinding);


		// Determine rendering operation
		D3D11_PRIMITIVE_TOPOLOGY primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		DWORD primCount = 0;
		bool useAdjacency = (mGeometryProgramBound && mBoundGeometryProgram && mBoundGeometryProgram->isAdjacencyInfoRequired());
		switch( op.operationType )
		{
		case RenderOperation::OT_POINT_LIST:
			primType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
			break;

		case RenderOperation::OT_LINE_LIST:
			primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
			break;

		case RenderOperation::OT_LINE_STRIP:
			primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
			break;

		case RenderOperation::OT_TRIANGLE_LIST:
			primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
			break;

		case RenderOperation::OT_TRIANGLE_STRIP:
			primType = useAdjacency ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
			break;

		case RenderOperation::OT_TRIANGLE_FAN:
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error - DX11 render - no support for triangle fan (OT_TRIANGLE_FAN)", "D3D11RenderSystem::_render");
			primType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; // todo - no TRIANGLE_FAN in DX 11
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
			break;
		}

		if (primCount)
		{
			// Issue the op
			//HRESULT hr;
			if( op.useIndexes  )
			{
				D3D11HardwareIndexBuffer* d3dIdxBuf = 
					static_cast<D3D11HardwareIndexBuffer*>(op.indexData->indexBuffer.get());
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
								"D3D11 device cannot draw indexed\nError Description:" + errorDescription,
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


		if (true) // for now - clear the render state
		{
			mDevice.GetImmediateContext()->OMSetBlendState(0, 0, 0xffffffff); 
			mDevice.GetImmediateContext()->RSSetState(0);
			mDevice.GetImmediateContext()->OMSetDepthStencilState(0, 0); 
//			mDevice->PSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(0), 0);
			//delete opState;
		}


	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setNormaliseNormals(bool normalise)
    {
	}
	//---------------------------------------------------------------------
    void D3D11RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
		if (!prg)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Null program bound.",
				"D3D11RenderSystem::bindGpuProgram");
		}

		switch (prg->getType())
		{
		case GPT_VERTEX_PROGRAM:
			{
				// get the shader
				mBoundVertexProgram = static_cast<D3D11HLSLProgram*>(prg);
				ID3D11VertexShader * vsShaderToSet = mBoundVertexProgram->getVertexShader();

				// set the shader
				mDevice.GetImmediateContext()->VSSetShader(vsShaderToSet, NULL, 0);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D11 device cannot set vertex shader\nError Description:" + errorDescription,
						"D3D11RenderSystem::bindGpuProgram");
				}		
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			{
				mBoundFragmentProgram = static_cast<D3D11HLSLProgram*>(prg);
				ID3D11PixelShader* psShaderToSet = mBoundFragmentProgram->getPixelShader();

				mDevice.GetImmediateContext()->PSSetShader(psShaderToSet, NULL, 0);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D11 device cannot set fragment shader\nError Description:" + errorDescription,
						"D3D11RenderSystem::bindGpuProgram");
				}		
			}
			break;
		case GPT_GEOMETRY_PROGRAM:
			{
				mBoundGeometryProgram = static_cast<D3D11HLSLProgram*>(prg);
				ID3D11GeometryShader* gsShaderToSet = mBoundGeometryProgram->getGeometryShader();

				mDevice.GetImmediateContext()->GSSetShader(gsShaderToSet, NULL, 0);
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D11 device cannot set geometry shader\nError Description:" + errorDescription,
						"D3D11RenderSystem::bindGpuProgram");
				}		

			}
			break;
		};

		RenderSystem::bindGpuProgram(prg);
   }
	//---------------------------------------------------------------------
    void D3D11RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {

		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			{
				mActiveVertexGpuProgramParameters.setNull();
				mBoundVertexProgram = NULL;
				//mDevice->VSSetShader(NULL);
                mDevice.GetImmediateContext()->VSSetShader(NULL, NULL, 0);
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			{
				mActiveFragmentGpuProgramParameters.setNull();
				mBoundFragmentProgram = NULL;
				//mDevice->PSSetShader(NULL);
                mDevice.GetImmediateContext()->PSSetShader(NULL, NULL, 0);
			}

			break;
		case GPT_GEOMETRY_PROGRAM:
			{
				mActiveGeometryGpuProgramParameters.setNull();
				mBoundGeometryProgram = NULL;
				mDevice.GetImmediateContext()->GSSetShader( NULL, NULL, 0 );
 			}
			break;
		default:
			assert(false && "Undefined Program Type!");
		};
		RenderSystem::unbindGpuProgram(gptype);
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
		ID3D11Buffer* pBuffers[1] ;
		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			{
				//	if (params->getAutoConstantCount() > 0)
				//{
				if (mBoundVertexProgram)
				{
					pBuffers[0] = mBoundVertexProgram->getConstantBuffer(params, mask);
					mDevice.GetImmediateContext()->VSSetConstantBuffers( 0, 1, pBuffers );
					if (mDevice.isError())
					{
						String errorDescription = mDevice.getErrorDescription();
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
							"D3D11 device cannot set vertex shader constant buffers\nError Description:" + errorDescription,
							"D3D11RenderSystem::bindGpuProgramParameters");
					}		

				}
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			{
				//if (params->getAutoConstantCount() > 0)
				//{
				if (mBoundFragmentProgram)
				{
					pBuffers[0] = mBoundFragmentProgram->getConstantBuffer(params, mask);
					mDevice.GetImmediateContext()->PSSetConstantBuffers( 0, 1, pBuffers );
					if (mDevice.isError())
					{
						String errorDescription = mDevice.getErrorDescription();
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
							"D3D11 device cannot set fragment shader constant buffers\nError Description:" + errorDescription,
							"D3D11RenderSystem::bindGpuProgramParameters");
					}		

				}
			}
			break;
		case GPT_GEOMETRY_PROGRAM:
			{
				if (mBoundGeometryProgram)
				{
					pBuffers[0] = mBoundGeometryProgram->getConstantBuffer(params, mask);
					mDevice.GetImmediateContext()->GSSetConstantBuffers( 0, 1, pBuffers );
					if (mDevice.isError())
					{
						String errorDescription = mDevice.getErrorDescription();
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
							"D3D11 device cannot set Geometry shader constant buffers\nError Description:" + errorDescription,
							"D3D11RenderSystem::bindGpuProgramParameters");
					}		

				}
			}
			break;

		};
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

		}
    }
	//---------------------------------------------------------------------
	void D3D11RenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
	{
    }
	//---------------------------------------------------------------------
    void D3D11RenderSystem::setScissorTest(bool enabled, size_t left, size_t top, size_t right,
        size_t bottom)
    {
		mRasterizerDesc.ScissorEnable = enabled;
		mScissorRect.left = static_cast<LONG>(left);
		mScissorRect.top = static_cast<LONG>(top);
		mScissorRect.right = static_cast<LONG>(right);
		mScissorRect.bottom =static_cast<LONG>( bottom);

		mDevice.GetImmediateContext()->RSSetScissorRects(1, &mScissorRect);
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set scissor rects\nError Description:" + errorDescription,
				"D3D11RenderSystem::setScissorTest");
		}	
	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, Real depth, unsigned short stencil)
    {
		if (mActiveRenderTarget)
		{
			ID3D11RenderTargetView ** pRTView;
			mActiveRenderTarget->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );

			if (buffers & FBT_COLOUR)
			{
				float ClearColor[4];
				D3D11Mappings::get(colour, ClearColor);

				// Clear all views
				uint numberOfViews;
				mActiveRenderTarget->getCustomAttribute( "numberOfViews", &numberOfViews );
				if( numberOfViews == 1 )
					mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView[0], ClearColor );
				else
				{
					for( uint i = 0; i < numberOfViews; ++i )
						mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView[i], ClearColor );
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
					mDevice.GetImmediateContext()->ClearDepthStencilView(
														depthBuffer->getDepthStencilView(),
														ClearFlags, depth, static_cast<UINT8>(stencil) );
				}
			}
		}
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
		bool ok = false;
		bool qualityHint = fsaaHint.find("Quality") != String::npos;
		size_t origFSAA = fsaa;
		bool tryCSAA = false;
		// NVIDIA, prefer CSAA if available for 8+
		// it would be tempting to use getCapabilities()->getVendor() == GPU_NVIDIA but
		// if this is the first window, caps will not be initialised yet
		
		if (mActiveD3DDriver->getAdapterIdentifier().VendorId == 0x10DE && 
			fsaa >= 8)
		{
			tryCSAA	 = true;
		}

		while (!ok)
		{
			// Deal with special cases
			if (tryCSAA)
			{
				// see http://developer.nvidia.com/object/coverage-sampled-aa.html
				switch(fsaa)
				{
				case 8:
					if (qualityHint)
					{
						outFSAASettings->Count = 8;
						outFSAASettings->Quality = 8;
					}
					else
					{
						outFSAASettings->Count = 4;
						outFSAASettings->Quality = 8;
					}
					break;
				case 16:
					if (qualityHint)
					{
						outFSAASettings->Count = 8;
						outFSAASettings->Quality = 16;
					}
					else
					{
						outFSAASettings->Count = 4;
						outFSAASettings->Quality = 16;
					}
					break;
				}
			}
			else // !CSAA
			{
				outFSAASettings->Count = fsaa == 0 ? 1 : fsaa;
				outFSAASettings->Quality = 0;
			}


			HRESULT hr;
			UINT outQuality;
			hr = mDevice->CheckMultisampleQualityLevels( 
				format, 
				outFSAASettings->Count, 
				&outQuality);

			if (SUCCEEDED(hr) && 
				(!tryCSAA || outQuality > outFSAASettings->Quality))
			{
				ok = true;
			}
			else
			{
				// downgrade
				if (tryCSAA && fsaa == 8)
				{
					// for CSAA, we'll try downgrading with quality mode at all samples.
					// then try without quality, then drop CSAA
					if (qualityHint)
					{
						// drop quality first
						qualityHint = false;
					}
					else
					{
						// drop CSAA entirely 
						tryCSAA = false;
					}
					// return to original requested samples
					fsaa = static_cast<uint>(origFSAA);
				}
				else
				{
					// drop samples
					--fsaa;

					if (fsaa == 1)
					{
						// ran out of options, no FSAA
						fsaa = 0;
						ok = true;
					}
				}
			}

		} // while !ok

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
		hr = CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)&mpDXGIFactory );
		if( FAILED(hr) )
		{
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Failed to create Direct3D11 DXGIFactory1", 
				"D3D11RenderSystem::D3D11RenderSystem" );
		}

		mDriverList = NULL;
		mActiveD3DDriver = NULL;
		mTextureManager = NULL;
		mHardwareBufferManager = NULL;
		mGpuProgramManager = NULL;
		mPrimaryWindow = NULL;
		mBasicStatesInitialised = false;
		mUseNVPerfHUD = false;
		mHLSLProgramFactory = NULL;

		mBoundVertexProgram = NULL;
		mBoundFragmentProgram = NULL;
		mBoundGeometryProgram = NULL;

		ZeroMemory( &mBlendDesc, sizeof(mBlendDesc));

		ZeroMemory( &mRasterizerDesc, sizeof(mRasterizerDesc));
		mRasterizerDesc.FrontCounterClockwise = true;
		mRasterizerDesc.DepthClipEnable = false;
		mRasterizerDesc.MultisampleEnable = true;


		ZeroMemory( &mDepthStencilDesc, sizeof(mDepthStencilDesc));

		ZeroMemory( &mDepthStencilDesc, sizeof(mDepthStencilDesc));
		ZeroMemory( &mScissorRect, sizeof(mScissorRect));

		FilterMinification = FO_NONE;
		FilterMagnification = FO_NONE;
		FilterMips = FO_NONE;

		mPolygonMode = PM_SOLID;

		ZeroMemory(mTexStageDesc, OGRE_MAX_TEXTURE_LAYERS * sizeof(sD3DTextureStageDesc));

		UINT deviceFlags = 0;
		if (D3D11Device::D3D_NO_EXCEPTION != D3D11Device::getExceptionsErrorLevel() && OGRE_DEBUG_MODE)
		{
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		if (!OGRE_THREAD_SUPPORT)
		{
			deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
		}

		ID3D11Device * device;
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE ,0,deviceFlags, NULL, 0, D3D11_SDK_VERSION, &device, 0 , 0);

		if(FAILED(hr))
		{
			std::stringstream error;
			error<<"Failed to create Direct3D11 object."<<std::endl<<DXGetErrorDescription(hr)<<std::endl;

			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Failed to create Direct3D11 object", 
				"D3D11RenderSystem::D3D11RenderSystem" );
		}

		mDevice = D3D11Device(device) ;


		// set stages desc. to defaults
		for (size_t n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
		{
			mTexStageDesc[n].autoTexCoordType = TEXCALC_NONE;
			mTexStageDesc[n].coordIndex = 0;
			mTexStageDesc[n].pTex = 0;
		}

		mLastVertexSourceCount = 0;
	}
	//---------------------------------------------------------------------
	bool D3D11RenderSystem::_getDepthBufferCheckEnabled( void )
	{
		return mDepthStencilDesc.DepthEnable == TRUE;
	}
	//---------------------------------------------------------------------
	D3D11HLSLProgram* D3D11RenderSystem::_getBoundVertexProgram() const
	{
		return mBoundVertexProgram;
	}
	//---------------------------------------------------------------------
	D3D11HLSLProgram* D3D11RenderSystem::_getBoundFragmentProgram() const
	{
		return mBoundFragmentProgram;
	}
	//---------------------------------------------------------------------
	D3D11HLSLProgram* D3D11RenderSystem::_getBoundGeometryProgram() const
	{
		return mBoundGeometryProgram;
	}
}
