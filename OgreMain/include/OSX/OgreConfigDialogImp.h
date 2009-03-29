#ifndef __OSXConfigDialog_H__
#define __OSXConfigDialog_H__

#include "OgrePrerequisites.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

#include <Carbon/Carbon.h>

namespace Ogre
{
	class ConfigDialog : public UtilityAlloc
	{
	public:
		ConfigDialog();
		~ConfigDialog();
	
	public:
		void initialise();
		void run();
		void cancel();

		bool display();

	public:
		static pascal OSStatus windowEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void* aUserData );

	protected:
		RenderSystem* iSelectedRenderSystem;
		HIViewRef	iLogoViewRef;
		HIViewRef	iVideoModeRef;
		HIViewRef	iColorDepthRef;
		HIViewRef	iFSAARef;
		HIViewRef	iRTTPrefModeRef;
		HIViewRef	iFullScreenRef;
		WindowRef	iWindowRef;
		IBNibRef	iNibRef;
		MenuRef		iMenuRef;
		MenuID		iMenuID;
		bool		iDisplayStatus;		
	};
}

#endif // __OSX_CONFIG_DIALOG_H__
