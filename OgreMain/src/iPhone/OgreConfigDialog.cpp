#include "OgreLogManager.h"
#include "OgreConfigDialog.h"

namespace Ogre {

	ConfigDialog* dlg = NULL;

	ConfigDialog::ConfigDialog()
	{
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
	}
	
	void ConfigDialog::initialise()
    {
	}
	
	void ConfigDialog::run()
	{

	}
	
	void ConfigDialog::cancel()
	{
	}

	bool ConfigDialog::display()
	{
		return true;
	}

};
