#ifndef __iPhoneConfigDialog_H__
#define __iPhoneConfigDialog_H__

#include "OgrePrerequisites.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre
{
	class _OgreExport ConfigDialog : public UtilityAlloc
	{
	public:
		ConfigDialog();
		~ConfigDialog();
	
	public:
		void initialise();
		void run();
		void cancel();

		bool display();

	protected:
		RenderSystem* iSelectedRenderSystem;
		bool		iDisplayStatus;		
	};
}

#endif // __IPHONE_CONFIG_DIALOG_H__
