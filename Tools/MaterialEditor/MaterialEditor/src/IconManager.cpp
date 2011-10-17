#include "IconManager.h"

template<> IconManager* Ogre::Singleton<IconManager>::msSingleton = 0;

IconManager::IconManager()
{
	mIconMap[CLOSE] = wxBitmap(close_xpm);
	mIconMap[COPY] = wxBitmap(copy_xpm);
	mIconMap[CUT] = wxBitmap(cut_xpm);
	mIconMap[MATERIAL] = wxBitmap(material_xpm);
	mIconMap[MATERIAL_SCRIPT] = wxBitmap(material_script_xpm);
	mIconMap[OGRE] = wxBitmap(ogre_xpm);
	mIconMap[PASS] = wxBitmap(pass_xpm);
	mIconMap[PASTE] = wxBitmap(paste_xpm);
	mIconMap[PROGRAM_SCRIPT] = wxBitmap(program_script_xpm);
	mIconMap[PROJECT] = wxBitmap(project_xpm);
	mIconMap[PROJECT_NEW] = wxBitmap(project_new_xpm);
	mIconMap[PROJECT_SAVE] = wxBitmap(project_save_xpm);
	mIconMap[SAVE] = wxBitmap(save_xpm);
	mIconMap[SAVE_AS] = wxBitmap(save_as_xpm);
	mIconMap[SHADER] = wxBitmap(shader_xpm);
	mIconMap[TECHNIQUE] = wxBitmap(technique_xpm);
	mIconMap[WORKSPACE] = wxBitmap(workspace_xpm);
}

IconManager::~IconManager()
{
}

const wxBitmap& IconManager::getIcon(IconType type) const
{
	IconMap::const_iterator iconItr = mIconMap.find(type);
	if(iconItr == mIconMap.end())
	{
		return wxNullBitmap;
	}
	else
	{
		return iconItr->second;
	}
}

IconManager* IconManager::getSingletonPtr(void)
{
    return msSingleton;
}

IconManager& IconManager::getSingleton(void)
{
    assert( msSingleton );  return (*msSingleton);
}
