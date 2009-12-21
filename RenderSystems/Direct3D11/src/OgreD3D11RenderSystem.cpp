/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

//---------------------------------------------------------------------
#define FLOAT2DWORD(f) *((DWORD*)&f)
//---------------------------------------------------------------------

namespace Ogre 
{

	//---------------------------------------------------------------------
	D3D11RenderSystem::D3D11RenderSystem( HINSTANCE hInstance ) 
	{
		LogManager::getSingleton().logMessage( "D3D11 : " + getName() + " created." );

		// set the instance being passed 
		mhInstance = hInstance;

		// set pointers to NULL
		mpDXGIFactory = NULL;
		HRESULT hr;
		hr = CreateDXGIFactory1( IID_IDXGIFactory1, (void**)&mpDXGIFactory );
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
		mDeviceLost = false;
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

		// Create our Direct3D object
	//	if( NULL == (mpD3D = Direct3DCreate9(D3D_SDK_VERSION)) )
	//		OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Failed to create Direct3D9 object", "D3D11RenderSystem::D3D11RenderSystem" );
		UINT deviceFlags = 0;
		if (D3D11Device::D3D_NO_EXCEPTION != D3D11Device::getExceptionsErrorLevel())
		{
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		if (!OGRE_THREAD_SUPPORT)
		{
			deviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
		}

		ID3D11Device * device;
		if(FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE ,0,deviceFlags, NULL, 0, D3D11_SDK_VERSION, &device, 0 , 0)))
		{
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
				"Failed to create Direct3D11 object", 
				"D3D11RenderSystem::D3D11RenderSystem" );
		}
		mDevice = D3D11Device(device) ;
		// set config options defaults
		initConfigOptions();


		// set stages desc. to defaults
		for (size_t n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
		{
			mTexStageDesc[n].autoTexCoordType = TEXCALC_NONE;
			mTexStageDesc[n].coordIndex = 0;
			mTexStageDesc[n].pTex = 0;
		}

		mLastVertexSourceCount = 0;

		// Enumerate events
	//	mEventNames.push_back("DeviceLost");
	//	mEventNames.push_back("DeviceRestored");

		mFixedFuncEmuShaderManager.registerGenerator(&mHlslFixedFuncEmuShaderGenerator);


	}
	//---------------------------------------------------------------------
	D3D11RenderSystem::~D3D11RenderSystem()
	{
        shutdown();

		mFixedFuncEmuShaderManager.unregisterGenerator(&mHlslFixedFuncEmuShaderGenerator);

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
			IDXGIAdapter* pAdapter = NULL;
			IDXGIAdapter* pSelectedAdapter = mActiveD3DDriver->getDeviceAdapter();
			if ( mUseNVPerfHUD )
			{

				HRESULT hr;

				// Search for a PerfHUD adapter
				while( mpDXGIFactory->EnumAdapters( nAdapter, &pAdapter ) != DXGI_ERROR_NOT_FOUND )
				{
					if ( pAdapter )
					{
						DXGI_ADAPTER_DESC adaptDesc;
						if ( SUCCEEDED( pAdapter->GetDesc( &adaptDesc ) ) )
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

			ID3D11Device * device;

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

			if(FAILED(D3D11CreateDevice(pSelectedAdapter, driverType , Software3d310Dll, deviceFlags, NULL, 0, D3D11_SDK_VERSION, &device, 0, 0)))         
			{
				OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
					"Failed to create Direct3D11 object", 
					"D3D11RenderSystem::D3D11RenderSystem" );
			}

			if (Software3d310Dll != NULL)
			{
				// get the IDXGIFactory1 from the device for software drivers
				// Remark(dec-09):
				//  Seems that IDXGIFactory::CreateSoftwareAdapter doesn't work with
				// D3D11CreateDevice - so I needed to create with pSelectedAdapter = 0.
				// If pSelectedAdapter == 0 then you have to get the IDXGIFactory1 from
				// the device - else CreateSwapChain fails later.
				SAFE_RELEASE(mpDXGIFactory);

				IDXGIDevice * pDXGIDevice;
				device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);

				IDXGIAdapter * pDXGIAdapter;
				pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

				pDXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&mpDXGIFactory);

				SAFE_RELEASE(pDXGIAdapter);
				SAFE_RELEASE(pDXGIDevice);
			}


			mDevice = D3D11Device(device) ;

			mActiveD3DDriver->setDevice(mDevice);
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
			miscParams["FSAA"] = fsaa;
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
		LogManager::getSingleton().logMessage("*** D3D11 : Subsystem Initialised OK ***");
		LogManager::getSingleton().logMessage("***************************************");

		// call superclass method
		RenderSystem::_initialise( autoCreateWindow );


		return autoWindow;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::reinitialise()
	{
		LogManager::getSingleton().logMessage( "D3D11 : Reinitialising" );
		this->shutdown();
	//	this->initialise( true );
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::shutdown()
	{
		RenderSystem::shutdown();


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


		// Adapter details
		const DXGI_ADAPTER_DESC& adapterID = mActiveD3DDriver->getAdapterIdentifier();

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
		// Documentation\dx11help\d3d11.chm::/D3D11CompileShader.htm
		// The Direct3D 10 currently supports only "vs_4_0", "ps_4_0", and "gs_4_0". 

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
	/*	bool anySupported = false;

		IDXGISurface * bbSurf;
		mPrimaryWindow->getCustomAttribute("DDBACKBUFFER", &bbSurf);
		D3DSURFACE_DESC bbSurfDesc;
		bbSurf->GetDesc(&bbSurfDesc);

		for (uint ipf = (uint)PF_L8; ipf < (uint)PF_COUNT; ++ipf)
		{
			PixelFormat pf = (PixelFormat)ipf;

			DXGI_FORMAT fmt = 
				D3D11Mappings::_getPF(D3D11Mappings::_getClosestSupportedPF(pf));

			if (SUCCEEDED(mpD3D->CheckDeviceFormat(
				mActiveD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, bbSurfDesc.Format, 
				D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, fmt)))
			{
				// cool, at least one supported
				anySupported = true;
				LogManager::getSingleton().stream()
					<< "D3D11: Vertex texture format supported - "
					<< PixelUtil::getFormatName(pf);
			}
		}

		return anySupported;
*/

	}
	//-----------------------------------------------------------------------
    bool D3D11RenderSystem::_checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage)
    {
	/*	// Gets D3D format
		DXGI_FORMAT d3dPF = D3D11Mappings::_getPF(format);
        if (d3dPF == D3DFMT_UNKNOWN)
            return false;

		IDXGISurface * pSurface = mPrimaryWindow->getRenderSurface();
		D3DSURFACE_DESC srfDesc;
		if (FAILED(pSurface->GetDesc(&srfDesc)))
            return false;

		// Calculate usage
		DWORD d3dusage = D3DUSAGE_QUERY_FILTER;
		if (usage & TU_RENDERTARGET) 
			d3dusage |= D3DUSAGE_RENDERTARGET;
		if (usage & TU_DYNAMIC)
			d3dusage |= D3D11_USAGE_DYNAMIC;

        // Detect resource type
        D3D11_RESOURCE_DIMENSION rtype;
		switch(ttype)
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
            rtype = D3DRTYPE_TEXTURE;
            break;
        case TEX_TYPE_3D:
            rtype = D3DRTYPE_VOLUMETEXTURE;
            break;
        case TEX_TYPE_CUBE_MAP:
            rtype = D3DRTYPE_CUBETEXTURE;
            break;
        default:
            return false;
        }

        HRESULT hr = mpD3D->CheckDeviceFormat(
            mActiveD3DDriver->getAdapterNumber(),
            D3DDEVTYPE_HAL,
            srfDesc.Format,
            d3dusage,
            rtype,
            d3dPF);

        return SUCCEEDED(hr);
		*/
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
			_cleanupDepthStencils();
			mDevice.release();
			//mActiveD3DDriver->setDevice(D3D11Device(NULL));
			mDevice = 0;

		}


	}
    //---------------------------------------------------------------------
	VertexElementType D3D11RenderSystem::getColourVertexElementType(void) const
	{
		return VET_COLOUR_ARGB;
	}
   	//---------------------------------------------------------------------
	void D3D11RenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
        Matrix4& dest, bool forGpuProgram)
    {
        dest = matrix;

		/*
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
		*/
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
        Real iw = 1.0 / half_w;
        Real ih = 1.0 / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 1.0 / (farPlane - nearPlane);
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
		mFixedFuncProgramsParameters.setLightAmbient(ColourValue(r, g, b, 0.0f));
	}
	//---------------------------------------------------------------------
    void D3D11RenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
		size_t currentLightsCount = lights.size();
		if (currentLightsCount > limit)
		{
			currentLightsCount = limit;
		}

		LightList lightsList;
		mFixedFuncState.getGeneralFixedFuncState().resetLightTypeCounts();
		for(size_t i = 0 ; i < currentLightsCount ; i++)
		{
			Light * curLight = lights[i];
			lightsList.push_back(curLight);
			mFixedFuncState.getGeneralFixedFuncState().addOnetoLightTypeCount(curLight->getType());
		}
		mFixedFuncProgramsParameters.setLights(lightsList);
    }
	//---------------------------------------------------------------------
	void D3D11RenderSystem::setShadingType( ShadeOptions so )
	{
	/*	HRESULT hr = __SetRenderState( D3DRS_SHADEMODE, D3D11Mappings::get(so) );
		if( FAILED( hr ) )
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
			"Failed to set render stat D3DRS_SHADEMODE", "D3D11RenderSystem::setShadingType" );
	*/
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::setLightingEnabled( bool enabled )
	{
		mFixedFuncProgramsParameters.setLightingEnabled(enabled);
		mFixedFuncState.getGeneralFixedFuncState().setLightingEnabled(enabled);

	
	/*	HRESULT hr;
		if( FAILED( hr = __SetRenderState( D3DRS_LIGHTING, enabled ) ) )
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
			"Failed to set render state D3DRS_LIGHTING", "D3D11RenderSystem::setLightingEnabled" );
	*/}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setViewMatrix( const Matrix4 &m )
	{
		// save latest view matrix
		mFixedFuncProgramsParameters.setViewMat(m);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setProjectionMatrix( const Matrix4 &m )
	{
		 // save latest projection matrix
		mFixedFuncProgramsParameters.setProjectionMat(m);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setWorldMatrix( const Matrix4 &m )
	{
		// save latest world matrix
		mFixedFuncProgramsParameters.setWorldMat(m);
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
		const ColourValue &specular, const ColourValue &emissive, Real shininess,
        TrackVertexColourType tracking )
	{
	/*	
		D3DMATERIAL9 material;
		material.Diffuse = D3DXCOLOR( diffuse.r, diffuse.g, diffuse.b, diffuse.a );
		material.Ambient = D3DXCOLOR( ambient.r, ambient.g, ambient.b, ambient.a );
		material.Specular = D3DXCOLOR( specular.r, specular.g, specular.b, specular.a );
		material.Emissive = D3DXCOLOR( emissive.r, emissive.g, emissive.b, emissive.a );
		material.Power = shininess;

		HRESULT hr = mDevice->SetMaterial( &material );
		if( FAILED( hr ) )
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting D3D material", "D3D11RenderSystem::_setSurfaceParams" );


		if(tracking != TVC_NONE) 
        {
            __SetRenderState(D3DRS_COLORVERTEX, TRUE);
            __SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, (tracking&TVC_AMBIENT)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, (tracking&TVC_DIFFUSE)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_SPECULARMATERIALSOURCE, (tracking&TVC_SPECULAR)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, (tracking&TVC_EMISSIVE)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
        } 
        else 
        {
            __SetRenderState(D3DRS_COLORVERTEX, FALSE);               
        }
    */    
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setPointParameters(Real size, 
		bool attenuationEnabled, Real constant, Real linear, Real quadratic,
		Real minSize, Real maxSize)
    {
	/*	if(attenuationEnabled)
		{
			// scaling required
			__SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
			__SetFloatRenderState(D3DRS_POINTSCALE_A, constant);
			__SetFloatRenderState(D3DRS_POINTSCALE_B, linear);
			__SetFloatRenderState(D3DRS_POINTSCALE_C, quadratic);
		}
		else
		{
			// no scaling required
			__SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
		}
		__SetFloatRenderState(D3DRS_POINTSIZE, size);
		__SetFloatRenderState(D3DRS_POINTSIZE_MIN, minSize);
		if (maxSize == 0.0f)
			maxSize = mCapabilities->getMaxPointSize();
		__SetFloatRenderState(D3DRS_POINTSIZE_MAX, maxSize);

*/
    }
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setPointSpritesEnabled(bool enabled)
	{
	/*	if (enabled)
		{
			__SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
		}
		else
		{
			__SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
		}
	*/
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTexture( size_t stage, bool enabled, const TexturePtr& tex )
	{
		static D3D11TexturePtr dt;
		dt = tex;
		if (dt.isNull())
		{
			enabled = false;
		}
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

		mFixedFuncProgramsParameters.setTextureEnabled(stage, enabled);


	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setVertexTexture(size_t stage, const TexturePtr& tex)
	{
		_setTexture(stage,true, tex);
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
	/*	HRESULT hr;
        // Record settings
        mTexStageDesc[stage].coordIndex = index;

		hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3D11Mappings::get(mTexStageDesc[stage].autoTexCoordType, mCaps) | index );
		if( FAILED( hr ) )
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture coord. set index", "D3D11RenderSystem::_setTextureCoordSet" );
	*/
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
	/*	if (mCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
		{
			// ugh - have to pass float data through DWORD with no conversion
			HRESULT hr = __SetSamplerState(unit, D3DSAMP_MIPMAPLODBIAS, 
				*(DWORD*)&bias);
			if(FAILED(hr))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture mipmap bias", 
				"D3D11RenderSystem::_setTextureMipmapBias" );

		}
	*/
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureMatrix( size_t stage, const Matrix4& xForm )
	{
		mFixedFuncProgramsParameters.setTextureMatrix(stage, xForm);
	/*	HRESULT hr;
		D3DXMATRIX d3dMat; // the matrix we'll maybe apply
		Matrix4 newMat = xForm; // the matrix we'll apply after conv. to D3D format
		// Cache texcoord calc method to register
		TexCoordCalcMethod autoTexCoordType = mTexStageDesc[stage].autoTexCoordType;

		if (autoTexCoordType == TEXCALC_ENVIRONMENT_MAP)
        {
            if (mCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
            {
                // Invert the texture for the spheremap 
                Matrix4 ogreMatEnvMap = Matrix4::IDENTITY;
			    // set env_map values
			    ogreMatEnvMap[1][1] = -1.0f;
			    // concatenate with the xForm
			    newMat = newMat.concatenate(ogreMatEnvMap);
            }
            else
            {
		        // If envmap is applied, but device doesn't support spheremap,
		        //then we have to use texture transform to make the camera space normal
		        //reference the envmap properly. This isn't exactly the same as spheremap
		        //(it looks nasty on flat areas because the camera space normals are the same)
		        //but it's the best approximation we have in the absence of a proper spheremap 
			    // concatenate with the xForm
                newMat = newMat.concatenate(Matrix4::CLIPSPACE2DTOIMAGESPACE);
            }
		}

        // If this is a cubic reflection, we need to modify using the view matrix
        if (autoTexCoordType == TEXCALC_ENVIRONMENT_MAP_REFLECTION)
        {
            // Get transposed 3x3
            // We want to transpose since that will invert an orthonormal matrix ie rotation
            Matrix4 ogreViewTransposed;
            ogreViewTransposed[0][0] = mViewMatrix[0][0];
            ogreViewTransposed[0][1] = mViewMatrix[1][0];
            ogreViewTransposed[0][2] = mViewMatrix[2][0];
            ogreViewTransposed[0][3] = 0.0f;

            ogreViewTransposed[1][0] = mViewMatrix[0][1];
            ogreViewTransposed[1][1] = mViewMatrix[1][1];
            ogreViewTransposed[1][2] = mViewMatrix[2][1];
            ogreViewTransposed[1][3] = 0.0f;

            ogreViewTransposed[2][0] = mViewMatrix[0][2];
            ogreViewTransposed[2][1] = mViewMatrix[1][2];
            ogreViewTransposed[2][2] = mViewMatrix[2][2];
            ogreViewTransposed[2][3] = 0.0f;

            ogreViewTransposed[3][0] = 0.0f;
            ogreViewTransposed[3][1] = 0.0f;
            ogreViewTransposed[3][2] = 0.0f;
            ogreViewTransposed[3][3] = 1.0f;
            
            newMat = newMat.concatenate(ogreViewTransposed);
        }

        if (autoTexCoordType == TEXCALC_PROJECTIVE_TEXTURE)
        {
            // Derive camera space to projector space transform
            // To do this, we need to undo the camera view matrix, then 
            // apply the projector view & projection matrices
            newMat = mViewMatrix.inverse();
			if(mTexProjRelative)
			{
				Matrix4 viewMatrix;
				mTexStageDesc[stage].frustum->calcViewMatrixRelative(mTexProjRelativeOrigin, viewMatrix);
				newMat = viewMatrix * newMat;
			}
			else
			{
				newMat = mTexStageDesc[stage].frustum->getViewMatrix() * newMat;
			}
            newMat = mTexStageDesc[stage].frustum->getProjectionMatrix() * newMat;
            newMat = Matrix4::CLIPSPACE2DTOIMAGESPACE * newMat;
            newMat = xForm * newMat;
        }

		// need this if texture is a cube map, to invert D3D's z coord
		if (autoTexCoordType != TEXCALC_NONE &&
            autoTexCoordType != TEXCALC_PROJECTIVE_TEXTURE)
		{
            newMat[2][0] = -newMat[2][0];
            newMat[2][1] = -newMat[2][1];
            newMat[2][2] = -newMat[2][2];
            newMat[2][3] = -newMat[2][3];
		}

        // convert our matrix to D3D format
		d3dMat = D3D11Mappings::makeD3DXMatrix(newMat);

		// set the matrix if it's not the identity
		if (!D3DXMatrixIsIdentity(&d3dMat))
		{
            /* It's seems D3D automatically add a texture coordinate with value 1,
            and fill up the remaining texture coordinates with 0 for the input
            texture coordinates before pass to texture coordinate transformation.

               NOTE: It's difference with D3DDECLTYPE enumerated type expand in
            DirectX SDK documentation!

               So we should prepare the texcoord transform, make the transformation
            just like standardized vector expand, thus, fill w with value 1 and
            others with 0.
            * /
            if (autoTexCoordType == TEXCALC_NONE)
            {
                /* FIXME: The actually input texture coordinate dimensions should
                be determine by texture coordinate vertex element. Now, just trust
                user supplied texture type matchs texture coordinate vertex element.
                * /
                if (mTexStageDesc[stage].texType == D3D11Mappings::D3D_TEX_TYPE_NORMAL)
                {
                    /* It's 2D input texture coordinate:

                      texcoord in vertex buffer     D3D expanded to     We are adjusted to
                                                -->                 -->
                                (u, v)               (u, v, 1, 0)          (u, v, 0, 1)
                    * /
                    std::swap(d3dMat._31, d3dMat._41);
                    std::swap(d3dMat._32, d3dMat._42);
                    std::swap(d3dMat._33, d3dMat._43);
                    std::swap(d3dMat._34, d3dMat._44);
                }
            }
            else
            {
                // All texgen generate 3D input texture coordinates.
            }

			// tell D3D the dimension of tex. coord.
			int texCoordDim = D3DTTFF_COUNT2;
            if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_PROJECTIVE_TEXTURE)
            {
                /* We want texcoords (u, v, w, q) always get divided by q, but D3D
                projected texcoords is divided by the last element (in the case of
                2D texcoord, is w). So we tweak the transform matrix, transform the
                texcoords with w and q swapped: (u, v, q, w), and then D3D will
                divide u, v by q. The w and q just ignored as it wasn't used by
                rasterizer.
                * /
			    switch (mTexStageDesc[stage].texType)
			    {
			    case D3D11Mappings::D3D_TEX_TYPE_NORMAL:
                    std::swap(d3dMat._13, d3dMat._14);
                    std::swap(d3dMat._23, d3dMat._24);
                    std::swap(d3dMat._33, d3dMat._34);
                    std::swap(d3dMat._43, d3dMat._44);

                    texCoordDim = D3DTTFF_PROJECTED | D3DTTFF_COUNT3;
                    break;

			    case D3D11Mappings::D3D_TEX_TYPE_CUBE:
			    case D3D11Mappings::D3D_TEX_TYPE_VOLUME:
                    // Yes, we support 3D projective texture.
				    texCoordDim = D3DTTFF_PROJECTED | D3DTTFF_COUNT4;
                    break;
                }
            }
            else
            {
			    switch (mTexStageDesc[stage].texType)
			    {
			    case D3D11Mappings::D3D_TEX_TYPE_NORMAL:
				    texCoordDim = D3DTTFF_COUNT2;
				    break;
			    case D3D11Mappings::D3D_TEX_TYPE_CUBE:
			    case D3D11Mappings::D3D_TEX_TYPE_VOLUME:
				    texCoordDim = D3DTTFF_COUNT3;
                    break;
			    }
            }

			hr = __SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, texCoordDim );
			if (FAILED(hr))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture coord. dimension", "D3D11RenderSystem::_setTextureMatrix" );

			hr = mDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &d3dMat );
			if (FAILED(hr))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture matrix", "D3D11RenderSystem::_setTextureMatrix" );
		}
		else
		{
			// disable all of this
			hr = __SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
			if( FAILED( hr ) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to disable texture coordinate transform", "D3D11RenderSystem::_setTextureMatrix" );

			// Needless to sets texture transform here, it's never used at all
		}
		*/
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
	/*	HRESULT hr;
		if( sourceFactor == SBF_ONE && destFactor == SBF_ZERO && 
			sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
		{
			if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE)))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D11RenderSystem::_setSceneBlending" );
		}
		else
		{
			if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE)))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D11RenderSystem::_setSeperateSceneBlending" );
			if (FAILED(hr = __SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE)))
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set separate alpha blending option", "D3D11RenderSystem::_setSeperateSceneBlending" );
			if( FAILED( hr = __SetRenderState( D3DRS_SRCBLEND, D3D11Mappings::get(sourceFactor) ) ) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set source blend", "D3D11RenderSystem::_setSeperateSceneBlending" );
			if( FAILED( hr = __SetRenderState( D3DRS_DESTBLEND, D3D11Mappings::get(destFactor) ) ) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set destination blend", "D3D11RenderSystem::_setSeperateSceneBlending" );
			if( FAILED( hr = __SetRenderState( D3DRS_SRCBLENDALPHA, D3D11Mappings::get(sourceFactorAlpha) ) ) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha source blend", "D3D11RenderSystem::_setSeperateSceneBlending" );
			if( FAILED( hr = __SetRenderState( D3DRS_DESTBLENDALPHA, D3D11Mappings::get(destFactorAlpha) ) ) )
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha destination blend", "D3D11RenderSystem::_setSeperateSceneBlending" );
		}
	*/
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
		mFixedFuncProgramsParameters.setFogMode(mode);
		mFixedFuncProgramsParameters.setFogColour(colour);
		mFixedFuncProgramsParameters.setFogDensitiy(densitiy);
		mFixedFuncProgramsParameters.setFogStart(start);
		mFixedFuncProgramsParameters.setFogEnd(end);
		mFixedFuncState.getGeneralFixedFuncState().setFogMode(mode);
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
	/*	DWORD oldVal;
		mDevice->GetSamplerState(unit, D3DSAMP_MAXANISOTROPY, &oldVal);
			return oldVal;
	*/
		return 0;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
	{
	/*	if ((DWORD)maxAnisotropy > mCaps.MaxAnisotropy)
			maxAnisotropy = mCaps.MaxAnisotropy;

		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			__SetSamplerState( unit, D3DSAMP_MAXANISOTROPY, maxAnisotropy );
	*/}
	//---------------------------------------------------------------------
	/*HRESULT D3D11RenderSystem::__SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;

		if ( FAILED( hr = mDevice->GetRenderState(state, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mDevice->SetRenderState(state, value);
	
	}*/
	//---------------------------------------------------------------------
	/*HRESULT D3D11RenderSystem::__SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;

		if ( FAILED( hr = mDevice->GetSamplerState(sampler, type, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mDevice->SetSamplerState(sampler, type, value);
	}*/
	//---------------------------------------------------------------------
	/*HRESULT D3D11RenderSystem::__SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;
		
		if ( FAILED( hr = mDevice->GetTextureStageState(stage, type, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mDevice->SetTextureStageState(stage, type, value);
	
	}*/
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setRenderTarget(RenderTarget *target)
	{
		mActiveRenderTarget = target;


		// Retrieve render surfaces (up to OGRE_MAX_MULTIPLE_RENDER_TARGETS)
		/*	IDXGISurface * pBack[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
		memset(pBack, 0, sizeof(pBack));
		target->getCustomAttribute( "DDBACKBUFFER", &pBack );
		if (!pBack[0])
		return;

		IDXGISurface * pDepth = NULL;
		target->getCustomAttribute( "D3DZBUFFER", &pDepth );
		if (!pDepth)
		{
		/// No depth buffer provided, use our own
		/// Request a depth stencil that is compatible with the format, multisample type and
		/// dimensions of the render target.
		D3DSURFACE_DESC srfDesc;
		if(FAILED(pBack[0]->GetDesc(&srfDesc)))
		return; // ?
		pDepth = _getDepthStencilFor(srfDesc.Format, srfDesc.MultiSampleType, srfDesc.Width, srfDesc.Height);
		}
		// Bind render targets
		uint count = mCapabilities->numMultiRenderTargets();
		for(uint x=0; x<count; ++x)
		{
		hr = mDevice->SetRenderTarget(x, pBack[x]);
		if (FAILED(hr))
		{
		String msg ;//= DXGetErrorDescription(hr);
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to setRenderTarget : " + msg, "D3D11RenderSystem::_setViewport" );
		}
		}
		*/
		ID3D11RenderTargetView * pRTView;
		target->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );
		ID3D11DepthStencilView * pRTDepthView;
		target->getCustomAttribute( "ID3D11DepthStencilView", &pRTDepthView );


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
		mDevice.GetImmediateContext()->OMSetRenderTargets(1,
			&pRTView,
			pRTDepthView);


		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set render target\nError Description:" + errorDescription,
				"D3D11RenderSystem::_setViewport");
		}

		// TODO - support MRT

		/*	hr = mDevice->SetDepthStencilSurface(pDepth);
		if (FAILED(hr))
		{
		String msg ;//= DXGetErrorDescription(hr);
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to setDepthStencil : " + msg, "D3D11RenderSystem::_setViewport" );
		}
		*/

	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_setViewport( Viewport *vp )
	{
		if( vp != mActiveViewport || vp->_isUpdated() )
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
			d3dvp.TopLeftX = vp->getActualLeft();
			d3dvp.TopLeftY = vp->getActualTop();
			d3dvp.Width = vp->getActualWidth();
			d3dvp.Height = vp->getActualHeight();
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
/*
		if( FAILED( hr = mDevice->BeginScene() ) )
		{
			String msg = DXGetErrorDescription(hr);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error beginning frame :" + msg, "D3D11RenderSystem::_beginFrame" );
		}

		if(!mBasicStatesInitialised)
		{
			// First-time 
			// setup some defaults
			// Allow specular
			hr = __SetRenderState(D3DRS_SPECULARENABLE, TRUE);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error enabling alpha blending option : " + msg, "D3D11RenderSystem::_beginFrame");
			}
			mBasicStatesInitialised = true;
		}
*/
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_endFrame()
	{
/*
		HRESULT hr;
		if( FAILED( hr = mDevice->EndScene() ) )
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error ending frame", "D3D11RenderSystem::_endFrame" );
*/
	}
	//---------------------------------------------------------------------
/*	inline bool D3D11RenderSystem::compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, size_t size )
	{
		for( size_t i=0; i < size; i++ )
		{
			if( pDecl1[i].Method != pDecl2[i].Method ||
				pDecl1[i].Offset != pDecl2[i].Offset ||
				pDecl1[i].Stream != pDecl2[i].Stream ||
				pDecl1[i].Type != pDecl2[i].Type ||
				pDecl1[i].Usage != pDecl2[i].Usage ||
				pDecl1[i].SemanticIndex != pDecl2[i].SemanticIndex)
			{
				return false;
			}
		}

		return true;
	}
*/    //---------------------------------------------------------------------
	void D3D11RenderSystem::setVertexDeclaration(VertexDeclaration* decl)
	{
      	D3D11VertexDeclaration* d3ddecl = 
			static_cast<D3D11VertexDeclaration*>(decl);

		d3ddecl->bindToShader(mBoundVertexProgram);
       

	}
    //---------------------------------------------------------------------
	void D3D11RenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
	{
     
		//HRESULT hr;

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
			/*
			if (FAILED(hr))
			{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D11 stream source for buffer binding", 
			"D3D11RenderSystem::setVertexBufferBinding");
			}*/


		}

		// Unbind any unused sources
		/*for (size_t unused = binds.size(); unused < mLastVertexSourceCount; ++unused)
		{

		hr = mDevice->SetStreamSource(static_cast<UINT>(unused), NULL, 0, 0);
		if (FAILED(hr))
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to reset unused D3D11 stream source", 
		"D3D11RenderSystem::setVertexBufferBinding");
		}

		}*/
		mLastVertexSourceCount = binds.size();


		
	}

    //---------------------------------------------------------------------
    void D3D11RenderSystem::_render(const RenderOperation& op)
	{

		// Exit immediately if there is nothing to render
		// This caused a problem on FireGL 8800
		if (op.vertexData->vertexCount == 0)
			return;


		// Call super class
		RenderSystem::_render(op);
		
		// TODO: Move this class to the right place.
		class D3D11RenderOperationState : public Renderable::RenderSystemData
		{
		public:
			ID3D11BlendState * mBlendState;
			ID3D11RasterizerState * mRasterizer;
			ID3D11DepthStencilState * mDepthStencilState;
			TextureLayerStateList mTextureLayerStateList;

			ID3D11SamplerState * mSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
			size_t mSamplerStatesCount;

			ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS];
			size_t mTexturesCount;

			FixedFuncPrograms * mFixedFuncPrograms;

			~D3D11RenderOperationState()
			{
				SAFE_RELEASE( mBlendState );
				SAFE_RELEASE( mRasterizer );
				SAFE_RELEASE( mDepthStencilState );

				for (size_t i = 0 ; i < mSamplerStatesCount ; i++)
				{
					SAFE_RELEASE( mSamplerStates[i] );
				}

			}
		};

		D3D11RenderOperationState * opState = NULL;
		bool unstandardRenderOperation = false;
		/*if (op.srcRenderable)
		{
			opState = (D3D11RenderOperationState *) op.srcRenderable->getRenderSystemData();
		}
		else
		*/{
			unstandardRenderOperation = true;
		}

		if(!opState)
		{
			opState =  new D3D11RenderOperationState;

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



			for (size_t i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
			{
				sD3DTextureStageDesc & curDesc = mTexStageDesc[i];
				if (curDesc.used)
				{
					TextureLayerState textureLayerState;
					textureLayerState.setTextureType(curDesc.type);
					textureLayerState.setTexCoordCalcMethod(curDesc.autoTexCoordType);
					textureLayerState.setLayerBlendModeEx(curDesc.layerBlendMode);
					textureLayerState.setCoordIndex((uint8)curDesc.coordIndex);
					opState->mTextureLayerStateList.push_back(textureLayerState);

				}
			}


			if (!unstandardRenderOperation)
			{
				op.srcRenderable->setRenderSystemData(opState);
			}

		}

		mFixedFuncState.setTextureLayerStateList(opState->mTextureLayerStateList);

		if (unstandardRenderOperation || opState->mBlendState != mBoundBlendState)
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
		}

		if (unstandardRenderOperation || opState->mRasterizer != mBoundRasterizer)
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
		

		if (unstandardRenderOperation || opState->mDepthStencilState != mBoundDepthStencilState)
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
		//	if ((mBoundSamplerStatesCount != opState->mSamplerStatesCount) || ( 0 != memcmp(opState->mSamplerStates, mBoundSamplerStates, mBoundSamplerStatesCount) ) )
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



		// well, in D3D11 we have to make sure that we have a vertex and fragment shader
		// bound before we start rendering, so we do that first...
		bool needToUnmapFS = false;
		bool needToUnmapVS = false;
	 	if (!mBoundVertexProgram || !mBoundFragmentProgram) // I know this is bad code - but I want to get things going
		{
			

	
			{
				const VertexBufferDeclaration &  vertexBufferDeclaration = 
					(static_cast<D3D11VertexDeclaration *>(op.vertexData->vertexDeclaration))->getVertexBufferDeclaration();

				opState->mFixedFuncPrograms = mFixedFuncEmuShaderManager.getShaderPrograms("hlsl4", 
					vertexBufferDeclaration,
					mFixedFuncState
					);
			}



			FixedFuncPrograms * fixedFuncPrograms = opState->mFixedFuncPrograms;
				
			
			fixedFuncPrograms->setFixedFuncProgramsParameters(mFixedFuncProgramsParameters);

			if (!mBoundVertexProgram)
			{
				needToUnmapVS = true;
				// Bind Vertex Program
				bindGpuProgram(fixedFuncPrograms->getVertexProgramUsage()->getProgram().get());
				bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
					fixedFuncPrograms->getVertexProgramUsage()->getParameters(), (uint16)GPV_ALL);

			}

			if (!mBoundFragmentProgram)
			{
				needToUnmapFS = true;
				// Bind Fragment Program 
				bindGpuProgram(fixedFuncPrograms->getFragmentProgramUsage()->getProgram().get());
				bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM, 
					fixedFuncPrograms->getFragmentProgramUsage()->getParameters(), (uint16)GPV_ALL);
			}
				

		
		}

		mDevice.GetImmediateContext()->GSSetShader( NULL, NULL, 0 );
		if (mDevice.isError())
		{
			// this will never happen but we want to be consistent with the error checks... 
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D11 device cannot set geometry shader to null\nError Description:" + errorDescription,
				"D3D11RenderSystem::_render");
		}

		setVertexDeclaration(op.vertexData->vertexDeclaration);
		setVertexBufferBinding(op.vertexData->vertexBufferBinding);


		// Determine rendering operation
		D3D11_PRIMITIVE_TOPOLOGY primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		DWORD primCount = 0;
		switch( op.operationType )
		{
		case RenderOperation::OT_POINT_LIST:
			primType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
			break;

		case RenderOperation::OT_LINE_LIST:
			primType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
			break;

		case RenderOperation::OT_LINE_STRIP:
			primType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
			break;

		case RenderOperation::OT_TRIANGLE_LIST:
			primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
			break;

		case RenderOperation::OT_TRIANGLE_STRIP:
			primType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
			break;

		case RenderOperation::OT_TRIANGLE_FAN:
			primType = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED; // todo - no TRIANGLE_FAN in DX 10
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error - DX11 render - no support for triangle fan (OT_TRIANGLE_FAN)", "D3D11RenderSystem::_render");

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

					mDevice.GetImmediateContext()->Draw(
						static_cast<UINT>(op.vertexData->vertexCount), 
						static_cast<INT>(op.vertexData->vertexStart)
						); 
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

		if (needToUnmapVS)
		{
			unbindGpuProgram(GPT_VERTEX_PROGRAM);
		}

		if (needToUnmapFS)
		{
			unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
		} 	

		if (unstandardRenderOperation)
		{
			mDevice.GetImmediateContext()->OMSetBlendState(0, 0, 0xffffffff); 
			mDevice.GetImmediateContext()->RSSetState(0);
			mDevice.GetImmediateContext()->OMSetDepthStencilState(0, 0); 
//			mDevice->PSSetSamplers(static_cast<UINT>(0), static_cast<UINT>(0), 0);
			delete opState;
		}


	}
    //---------------------------------------------------------------------
    void D3D11RenderSystem::setNormaliseNormals(bool normalise)
    {
    //    __SetRenderState(D3DRS_NORMALIZENORMALS, 
    //      normalise ? TRUE : FALSE);
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
				ID3D11GeometryShader* psShaderToSet = mBoundFragmentProgram->getGeometryShader();

				mDevice.GetImmediateContext()->GSSetShader(psShaderToSet, NULL, 0);
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
			}
			break;
		case GPT_FRAGMENT_PROGRAM:
			{
				mActiveFragmentGpuProgramParameters.setNull();
				mBoundFragmentProgram = NULL;
				//mDevice->PSSetShader(NULL);
			}

			break;
		case GPT_GEOMETRY_PROGRAM:
			{
				mActiveGeometryGpuProgramParameters.setNull();
				mBoundGeometryProgram = NULL;

			}
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
				//}
				//else
				//{
				//	mDevice->VSSetConstantBuffers( 0, 1, NULL);
				//}
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
				//}
				//else
				//{
				// if I do this:
				//mDevice->PSSetConstantBuffers( 0, 0, NULL);
				// I get this info message that I don't want: 
				//  Since NumBuffers is 0, the operation effectively does nothing. 
				//  This is probably not intentional, nor is the most efficient way 
				//  to achieve this operation. Avoid calling the routine at all. 
				//  [ STATE_SETTING INFO #257: DEVICE_PSSETCONSTANTBUFFERS_BUFFERS_EMPTY ]
				// 
				// so - I don't want to do it for now
				//}
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
    /*    size_t i;
        size_t numClipPlanes;
        D3DXPLANE dx9ClipPlane;
        DWORD mask = 0;
        HRESULT hr;

		numClipPlanes = clipPlanes.size();
        for (i = 0; i < numClipPlanes; ++i)
        {
            const Plane& plane = clipPlanes[i];

			dx9ClipPlane.a = plane.normal.x;
			dx9ClipPlane.b = plane.normal.y;
			dx9ClipPlane.c = plane.normal.z;
			dx9ClipPlane.d = plane.d;

			if (mVertexProgramBound)
			{
				// programmable clips in clip space (ugh)
				// must transform worldspace planes by view/proj
				D3DXMATRIX xform;
				D3DXMatrixMultiply(&xform, &mDxViewMat, &mDxProjMat);
				D3DXMatrixInverse(&xform, NULL, &xform);
				D3DXMatrixTranspose(&xform, &xform);
				D3DXPlaneTransform(&dx9ClipPlane, &dx9ClipPlane, &xform);
			}

            hr = mDevice->SetClipPlane(i, dx9ClipPlane);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set clip plane", 
                    "D3D11RenderSystem::setClipPlanes");
            }

            mask |= (1 << i);
        }

        hr = __SetRenderState(D3DRS_CLIPPLANEENABLE, mask);
        if (FAILED(hr))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set render state for clip planes", 
                "D3D11RenderSystem::setClipPlanes");
        }
	*/
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
			ID3D11RenderTargetView * pRTView;
			mActiveRenderTarget->getCustomAttribute( "ID3D11RenderTargetView", &pRTView );
			ID3D11DepthStencilView * pRTDepthView;
			mActiveRenderTarget->getCustomAttribute( "ID3D11DepthStencilView", &pRTDepthView );

			if (buffers & FBT_COLOUR)
			{
				float ClearColor[4];
				D3D11Mappings::get(colour, ClearColor);
				mDevice.GetImmediateContext()->ClearRenderTargetView( pRTView, ClearColor );

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
				mDevice.GetImmediateContext()->ClearDepthStencilView( pRTDepthView, ClearFlags, depth, stencil );
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
    //    float plane[4] = { A, B, C, D };
    //    mDevice->SetClipPlane (index, plane);
    }

    // ------------------------------------------------------------------
    void D3D11RenderSystem::enableClipPlane (ushort index, bool enable)
    {
    /*    DWORD prev;
        mDevice->GetRenderState(D3DRS_CLIPPLANEENABLE, &prev);
        __SetRenderState(D3DRS_CLIPPLANEENABLE, enable?
			(prev | (1 << index)) : (prev & ~(1 << index)));
	*/
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
	void D3D11RenderSystem::restoreLostDevice(void)
	{
	/*	// Release all non-managed resources

		// Cleanup depth stencils
		_cleanupDepthStencils();

		// Set all texture units to nothing
		_disableTextureUnitsFrom(0);

		// Unbind any vertex streams
		for (size_t i = 0; i < mLastVertexSourceCount; ++i)
		{
			mDevice->SetStreamSource(i, NULL, 0, 0);
		}
        mLastVertexSourceCount = 0;

        // Release all automatic temporary buffers and free unused
        // temporary buffers, so we doesn't need to recreate them,
        // and they will reallocate on demand. This save a lot of
        // release/recreate of non-managed vertex buffers which
        // wasn't need at all.
        mHardwareBufferManager->_releaseBufferCopies(true);

		// We have to deal with non-managed textures and vertex buffers
		// GPU programs don't have to be restored
		static_cast<D3D11TextureManager*>(mTextureManager)->releaseDefaultPoolResources();
		static_cast<D3D11HardwareBufferManager*>(mHardwareBufferManager)
			->releaseDefaultPoolResources();

		// release additional swap chains (secondary windows)
		SecondaryWindowList::iterator sw;
		for (sw = mSecondaryWindows.begin(); sw != mSecondaryWindows.end(); ++sw)
		{
			(*sw)->destroyD3DResources();
		}

		DXGI_SWAP_CHAIN_DESC* presParams = mPrimaryWindow->getPresentationParameters();
		// Reset the device, using the primary window presentation params
		HRESULT hr = mDevice->Reset(presParams);

		if (hr == D3DERR_DEVICELOST)
		{
			// Don't continue
			return;
		}
		else if (FAILED(hr))
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Cannot reset device! " + getErrorDescription(hr), 
				"D3D11RenderWindow::restoreLostDevice" );
		}

		LogManager::getSingleton().stream()
			<< "Reset device ok w:" << presParams->BufferDesc.Height
			<< " h:" << presParams->BufferDesc.Height;
		// If windowed, we have to reset the size here
		// since a fullscreen switch may have occurred
		if (mPrimaryWindow->_getSwitchingFullscreen())
		{
			mPrimaryWindow->_finishSwitchingFullscreen();
		}


		// will have lost basic states
		mBasicStatesInitialised = false;
        mVertexProgramBound = false;
        mFragmentProgramBound = false;


		// recreate additional swap chains
		for (sw = mSecondaryWindows.begin(); sw != mSecondaryWindows.end(); ++sw)
		{
			(*sw)->createD3DResources();
		}

		// Recreate all non-managed resources
		static_cast<D3D11TextureManager*>(mTextureManager)
			->recreateDefaultPoolResources();
		static_cast<D3D11HardwareBufferManager*>(mHardwareBufferManager)
			->recreateDefaultPoolResources();
			
		LogManager::getSingleton().logMessage("!!! Direct3D Device successfully restored.");

		mDeviceLost = false;

		fireEvent("DeviceRestored");
*/
	}
	//---------------------------------------------------------------------
	bool D3D11RenderSystem::isDeviceLost(void)
	{
		return mDeviceLost;
	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_notifyDeviceLost(void)
	{
		LogManager::getSingleton().logMessage("!!! Direct3D Device Lost!");
		mDeviceLost = true;
		// will have lost basic states
		mBasicStatesInitialised = false;

		fireEvent("DeviceLost");
	}
	//---------------------------------------------------------------------
	// Formats to try, in decreasing order of preference
/*	DXGI_FORMAT ddDepthStencilFormats[]={
		D3DFMT_D24FS8,
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D24X8,
		D3DFMT_D15S1,
		D3DFMT_D16,
		D3DFMT_D32
	};
#define NDSFORMATS (sizeof(ddDepthStencilFormats)/sizeof(DXGI_FORMAT))
	
	DXGI_FORMAT D3D11RenderSystem::_getDepthStencilFormatFor(DXGI_FORMAT fmt)
	{
		/// Check if result is cached
		DepthStencilHash::iterator i = mDepthStencilHash.find((unsigned int)fmt);
		if(i != mDepthStencilHash.end())
			return i->second;
		/// If not, probe with CheckDepthStencilMatch
		DXGI_FORMAT dsfmt = D3DFMT_UNKNOWN;

		/// Get description of primary render target
		IDXGISurface * mSurface = mPrimaryWindow->getRenderSurface();
		D3DSURFACE_DESC srfDesc;

		if(!FAILED(mSurface->GetDesc(&srfDesc)))
		{
			/// Probe all depth stencil formats
			/// Break on first one that matches
			for(size_t x=0; x<NDSFORMATS; ++x)
			{
                // Verify that the depth format exists
                if (mpD3D->CheckDeviceFormat(
                    mActiveD3DDriver->getAdapterNumber(),
                    D3DDEVTYPE_HAL,
                    srfDesc.Format,
                    D3DUSAGE_DEPTHSTENCIL,
                    D3DRTYPE_SURFACE,
                    ddDepthStencilFormats[x]) != D3D_OK)
                {
                    continue;
                }
                // Verify that the depth format is compatible
				if(mpD3D->CheckDepthStencilMatch(
					mActiveD3DDriver->getAdapterNumber(),
					D3DDEVTYPE_HAL, srfDesc.Format,
					fmt, ddDepthStencilFormats[x]) == D3D_OK)
				{
					dsfmt = ddDepthStencilFormats[x];
					break;
				}
			}
		}
		/// Cache result
		mDepthStencilHash[(unsigned int)fmt] = dsfmt;
		return dsfmt;
	}
	*/
	//---------------------------------------------------------------------
	IDXGISurface* D3D11RenderSystem::_getDepthStencilFor(DXGI_FORMAT fmt, DXGI_SAMPLE_DESC multisample, size_t width, size_t height)
	{
	IDXGISurface *surface = 0;
	return surface;
/*	DXGI_FORMAT dsfmt = _getDepthStencilFormatFor(fmt);
		if(dsfmt == D3DFMT_UNKNOWN)
			return 0;
		

		/// Check if result is cached
		ZBufferFormat zbfmt(dsfmt, multisample);
		ZBufferHash::iterator i = mZBufferHash.find(zbfmt);
		if(i != mZBufferHash.end())
		{
			/// Check if size is larger or equal
			if(i->second.width >= width && i->second.height >= height)
			{
				surface = i->second.surface;
			} 
			else
			{
				/// If not, destroy current buffer
				i->second.surface->Release();
				mZBufferHash.erase(i);
			}
		}
		if(!surface)
		{
			/// If not, create the depthstencil surface
			HRESULT hr = mDevice->CreateDepthStencilSurface( 
				width, 
				height, 
				dsfmt, 
				multisample, 
				NULL, 
				TRUE,  // discard true or false?
				&surface, 
				NULL);
			if(FAILED(hr))
			{
				String msg = DXGetErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error CreateDepthStencilSurface : " + msg, "D3D11RenderSystem::_getDepthStencilFor" );
			}
			/// And cache it
			ZBufferRef zb;
			zb.surface = surface;
			zb.width = width;
			zb.height = height;
			mZBufferHash[zbfmt] = zb;
		}
		return surface;
*/	}
	//---------------------------------------------------------------------
	void D3D11RenderSystem::_cleanupDepthStencils()
	{
		for(ZBufferHash::iterator i = mZBufferHash.begin(); i != mZBufferHash.end(); ++i)
		{
			/// Release buffer
			i->second.surface->Release();
		}
		mZBufferHash.clear();
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
}
