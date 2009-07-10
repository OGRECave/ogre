#include <QuickTime/QuickTime.h>
#include "OgreLogManager.h"
#include "OgreConfigDialog.h"

namespace Ogre {

	ConfigDialog* dlg = NULL;
//	static EventHandlerUPP WindowEventHandlerUPP;

	ConfigDialog::ConfigDialog() : 
        iSelectedRenderSystem( NULL ), iLogoViewRef( NULL ), iVideoModeRef( NULL ),
        iWindowRef( NULL ), iNibRef( NULL ), iMenuRef( NULL )
	{
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
	}
	
	void ConfigDialog::initialise()
	{
		const RenderSystemList& renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers.front();
		ConfigOptionMap config = renderer->getConfigOptions();

		ConfigOptionMap::iterator cfi;
		
		cfi = config.find( "Full Screen" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "Yes" )
			{
				SetControlValue( iFullScreenRef, 1 );
			}
			else
			{
				SetControlValue( iFullScreenRef, 2 );
			}
		}

		cfi = config.find( "FSAA" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "0" )
			{
				SetControlValue( iFSAARef, 1 );
			}
			else if( cfi->second.currentValue == "2" )
			{
				SetControlValue( iFSAARef, 2 );
			}
			else if( cfi->second.currentValue == "4" )
			{
				SetControlValue( iFSAARef, 3 );
			}
			else if( cfi->second.currentValue == "6" )
			{
				SetControlValue( iFSAARef, 4 );
			}
		}

		cfi = config.find( "Colour Depth" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "32" )
			{
				SetControlValue( iColorDepthRef, 1 );
			}
			else
			{
				SetControlValue( iColorDepthRef, 2 );
			}
		}
		
		cfi = config.find( "RTT Preferred Mode" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "FBO" )
			{
				SetControlValue( iRTTPrefModeRef, 1 );
			}
			else if( cfi->second.currentValue == "PBuffer" )
			{
				SetControlValue( iRTTPrefModeRef, 2 );
			}
			else if( cfi->second.currentValue == "Copy" )
			{
				SetControlValue( iRTTPrefModeRef, 3 );
			}
		}
	}
	
	void ConfigDialog::run()
	{
		const RenderSystemList& renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers.front();

		SInt16 value = 0;

		// temp
		value = GetControlValue( iVideoModeRef );
		renderer->setConfigOption( "Video Mode", "800 x 600" );

		// full screen
		value = GetControlValue( iFullScreenRef );
		if( value == 1 ) // Yes
		{
			LogManager::getSingleton().logMessage( "CONFIG => FullScreen [ Yes ]" );
			renderer->setConfigOption( "Full Screen", "Yes" );
		}
		else
		{
			LogManager::getSingleton().logMessage( "CONFIG => FullScreen [ No ]" );
			renderer->setConfigOption( "Full Screen", "No" );
		}
		
		// fsaa
		value = GetControlValue( iFSAARef );
		switch( value )
		{
			case 1:
				renderer->setConfigOption( "FSAA", "0" );
				break;
			case 2:
				renderer->setConfigOption( "FSAA", "2" );
				break;
			case 3:
				renderer->setConfigOption( "FSAA", "4" );
				break;
			case 4:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 6 ]" );
				renderer->setConfigOption( "FSAA", "6" );
				break;
			default:
				renderer->setConfigOption( "FSAA", "0" );
				break;
		}

		// fsaa
		value = GetControlValue( iColorDepthRef );
		if( value == 1 )
		{
			renderer->setConfigOption( "Colour Depth", "32" );
		}
		else
		{
			renderer->setConfigOption( "Colour Depth", "16" );
		}

		// rtt pref mode
		value = GetControlValue( iRTTPrefModeRef );
		switch( value )
		{
			case 1:
				renderer->setConfigOption( "RTT Preferred Mode", "FBO" );
				break;
			case 2:
				renderer->setConfigOption( "RTT Preferred Mode", "PBuffer" );
				break;
			case 3:
				renderer->setConfigOption( "RTT Preferred Mode", "Copy" );
				break;
		}

		Root::getSingleton().setRenderSystem( renderer );

		iDisplayStatus = true;

		QuitAppModalLoopForWindow( iWindowRef );
	}
	
	void ConfigDialog::cancel()
	{
		iDisplayStatus = false;
		QuitAppModalLoopForWindow( iWindowRef );
	}
	
	pascal OSStatus ConfigDialog::windowEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void* aUserData )
	{
		#pragma unused ( inCallRef )
		OSStatus status	= eventNotHandledErr;
		UInt32 eventKind = GetEventKind( aEvent );
		UInt32 eventClass = GetEventClass( aEvent );
		WindowRef window = ( WindowRef ) aUserData;
		HICommand command;
		if( eventClass == kEventClassWindow && eventKind == kEventWindowClose )
		{
			QuitAppModalLoopForWindow( window );
		}
		else if( eventClass == kEventClassCommand && eventKind == kEventCommandProcess )
		{
			GetEventParameter( aEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommand ), NULL, &command );
			switch( command.commandID )
			{
				case 'run!':
					{
					dlg->run();
					break;
					}
				case 'not!':
					{
					dlg->cancel();
					break;
					}
				default:
					break;
			}
		}
		return ( status );
	}

	bool ConfigDialog::display()
	{
		// TODO: Fix OS X Config dialog
		const RenderSystemList& renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers.front();

		// WARNING: restoreConfig() should not be invoked here as Root calls
		// it before this method anyway, and invoking restoreConfig() here
		// forces the client application to use Ogre.cfg, while it may have
		// different plans.
		if(!Root::getSingleton().restoreConfig())
		{
			// Set some defaults
			renderer->setConfigOption("Video Mode", "800 x 600");
			renderer->setConfigOption("Colour Depth", "32");
			renderer->setConfigOption("FSAA", "0");
			renderer->setConfigOption("Full Screen", "No");
			renderer->setConfigOption("RTT Preferred Mode", "FBO");
			// Set the rendersystem and save the config.
			Root::getSingleton().setRenderSystem(renderer);
		}
		return true;
	/*
		iDisplayStatus = false;
		OSStatus status;
		CFStringRef logoRef = NULL;
		CFURLRef logoUrlRef = NULL;
		OSType dataRefType;
		GraphicsImportComponent	ci = NULL;
		CGImageRef cgImageRef = NULL;
		Handle dataRef = NULL;

		HIViewID logoViewID = { 'CONF', 100 };
		HIViewID videoModeViewID = { 'CONF', 104 };
		HIViewID colorDepthViewID = { 'CONF', 105 };
		HIViewID fsaaViewID = { 'CONF', 106 };
		HIViewID rttPrefModeViewID = { 'CONF', 107 };
		HIViewID fullScreenViewID = { 'CONF', 108 };

		const EventTypeSpec windowEvents[] =
		{
			{ kEventClassCommand, kEventCommandProcess },
			{ kEventClassWindow, kEventWindowClose }
		};

		// We need to get the Ogre Bundle
		CFBundleRef baseBundle = CFBundleGetBundleWithIdentifier(CFSTR("org.ogre3d.Ogre"));
		status	= CreateNibReferenceWithCFBundle(baseBundle, CFSTR("main"), &iNibRef );
		require_noerr( status, CantGetNibRef );

		status	= CreateWindowFromNib( iNibRef, CFSTR( "ConfigWindow" ), &iWindowRef );
		require_noerr( status, CantCreateWindow );

		if( WindowEventHandlerUPP == NULL )
			WindowEventHandlerUPP = NewEventHandlerUPP( windowEventHandler );

		status = InstallWindowEventHandler( iWindowRef, WindowEventHandlerUPP, GetEventTypeCount( windowEvents ), windowEvents, iWindowRef, NULL );
		//status = InstallStandardEventHandler(GetWindowEventTarget(iWindowRef));
		require_noerr( status, CantInstallWindowEventHandler );

		//logoRef = CFStringCreateWithCString( kCFAllocatorDefault, "file:///Users/aljen/Code/OSXConfig/logo.bmp", NULL );
		//logoUrlRef = CFURLCreateWithString( kCFAllocatorDefault, logoRef, NULL );

		//status = QTNewDataReferenceFromCFURL( logoUrlRef, 0, &dataRef, &dataRefType );
		//status	= GetGraphicsImporterForDataRef( dataRef, dataRefType, &ci );
		//require( ci != NULL, ImporterError );
		//status	= GraphicsImportCreateCGImage( ci, &cgImageRef, kGraphicsImportCreateCGImageUsingCurrentSettings );
		
		// logo view
		status	= HIViewFindByID( HIViewGetRoot( iWindowRef ), logoViewID, &iLogoViewRef );
		require_noerr( status, LogoViewNotFound );
		status	= HIImageViewSetImage( iLogoViewRef, cgImageRef );
		require_noerr( status, SetImageFailed );
		status	= HIImageViewSetScaleToFit( iLogoViewRef, false );
		require_noerr( status, ScaleFailed );
		status	= HIViewSetVisible( iLogoViewRef, true );
		require_noerr( status, SetVisibleFailed );
		
		// video mode view
		status	= HIViewFindByID( HIViewGetRoot( iWindowRef ), videoModeViewID, &iVideoModeRef );

		CreateNewMenu( iMenuID, 0, &iMenuRef );

		if( iMenuRef != NULL )
		{
			CFStringRef itemNames[] =
			{
				CFSTR( "640 x 480" ),
				CFSTR( "800 x 600" ),
				CFSTR( "1024 x 768" )
			};

			MenuItemIndex numItems = ( sizeof( itemNames ) / sizeof( CFStringRef ) );
			for( int i = 0; i < numItems; i++ )
			{
				MenuItemIndex newItem;
				AppendMenuItemTextWithCFString( iMenuRef, itemNames[ i ], 0, 0, &newItem );
			}
			
			SetControlData( iVideoModeRef, kControlEntireControl, kControlPopupButtonMenuRefTag, sizeof( MenuRef ), &iMenuRef );
		}

		// color depth view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), colorDepthViewID, &iColorDepthRef );

		// fsaa view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), fsaaViewID, &iFSAARef );

		// rtt pref mode view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), rttPrefModeViewID, &iRTTPrefModeRef );

		// full screen view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), fullScreenViewID, &iFullScreenRef );

		DisposeNibReference( iNibRef );

		initialise();
		
		RepositionWindow( iWindowRef, NULL, kWindowCenterOnMainScreen );
		TransitionWindow( iWindowRef, kWindowFadeTransitionEffect, kWindowShowTransitionAction, NULL );
		SelectWindow( iWindowRef );
		ActivateWindow( iWindowRef, true );
		RunAppModalLoopForWindow( iWindowRef );
		
		TransitionWindow( iWindowRef, kWindowFadeTransitionEffect, kWindowHideTransitionAction, NULL );
		DisposeWindow( iWindowRef );

		return iDisplayStatus;

	CantGetNibRef:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantGetNibRef" );
	CantCreateWindow:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantCreateWindow" );
	CantInstallWindowEventHandler:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantInstallWindowEventHandler" );
	ImporterError:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:ImporterError" );
	LogoViewNotFound:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:LogoViewNotFound" );
	SetImageFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetImageFailed" );
	ScaleFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetScaleFailed" );
	SetVisibleFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetVisibleFailed" );
		return( iDisplayStatus );


		return true;
	*/
	}

};
