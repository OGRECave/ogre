/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "MaterialControls.h"
#include "OgreLogManager.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"
#include "OgreException.h"

/********************************************************************************
            MaterialControls Methods
*********************************************************************************/
void MaterialControls::addControl(const Ogre::String& params)
{
    // params is a string containing using the following format:
    //  "<Control Name>, <Shader parameter name>, <Parameter Type>, <Min Val>, <Max Val>, <Parameter Sub Index>"

    // break up long string into components
    Ogre::StringVector vecparams = Ogre::StringUtil::split(params, ",");

    // if there are not five elements then log error and move on
    if (vecparams.size() != 6)
    {
        Ogre::LogManager::getSingleton().logMessage(
            "Incorrect number of parameters passed in params string for MaterialControls::addControl()" );

        return;
    }

    ShaderControl newControl;
    Ogre::StringUtil::trim(vecparams[0]);
    newControl.Name = vecparams[0];

    Ogre::StringUtil::trim(vecparams[1]);
    newControl.ParamName = vecparams[1];

    Ogre::StringUtil::trim(vecparams[2]);
    if (vecparams[2] == "GPU_VERTEX")
        newControl.ValType = GPU_VERTEX;
    else if (vecparams[2] == "GPU_FRAGMENT")
        newControl.ValType = GPU_FRAGMENT;

    newControl.MinVal = Ogre::StringConverter::parseReal(vecparams[3]);
    newControl.MaxVal = Ogre::StringConverter::parseReal(vecparams[4]);
    newControl.ElementIndex = Ogre::StringConverter::parseInt(vecparams[5]);

    mShaderControlsContainer.push_back(newControl);

}

void loadMaterialControlsFile(MaterialControlsContainer& controlsContainer, const Ogre::String& filename)
{
    // Load material controls from config file
    Ogre::ConfigFile cf;

    try
    {

        cf.load(filename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "\t;=", true);

        // Go through all sections & controls in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, materialName, dataString;

        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
            if (!secName.empty() && settings)
            {
                materialName = cf.getSetting("material", secName);

                MaterialControls newMaaterialControls(secName, materialName);
                controlsContainer.push_back(newMaaterialControls);

                size_t idx = controlsContainer.size() - 1;

                Ogre::ConfigFile::SettingsMultiMap::iterator i;

                for (i = settings->begin(); i != settings->end(); ++i)
                {
                    typeName = i->first;
                    dataString = i->second;
                    if (typeName == "control")
                        controlsContainer[idx].addControl(dataString);
                }
            }
        }

	    Ogre::LogManager::getSingleton().logMessage( "Material Controls setup" );
    }
    catch (Ogre::Exception e)
    {
        // Guess the file didn't exist
    }
}


void loadAllMaterialControlFiles(MaterialControlsContainer& controlsContainer)
{
    Ogre::StringVectorPtr fileStringVector = Ogre::ResourceGroupManager::getSingleton().findResourceNames( Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "*.controls" );
	Ogre::StringVector::iterator controlsFileNameIterator = fileStringVector->begin();

    while ( controlsFileNameIterator != fileStringVector->end() )
	{
        loadMaterialControlsFile(controlsContainer, *controlsFileNameIterator);
        ++controlsFileNameIterator;
	}
}
