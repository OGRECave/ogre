/************************************************************************
	filename: 	OgreCEGUIResourceProvider.cpp
	created:	8/7/2004
	author:		James '_mental_' O'Sullivan

	purpose:	Implements the Resource Provider common functionality
*************************************************************************/
/*************************************************************************
    Crazy Eddie's GUI System (http://www.cegui.org.uk)
    Copyright (C)2004 - 2005 Paul D Turner (paul@cegui.org.uk)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*************************************************************************/
#include "OgreCEGUIResourceProvider.h"

#include <CEGUI/CEGUIExceptions.h>
#include <OgreArchiveManager.h>


// Start of CEGUI namespace section
namespace CEGUI
{
    OgreCEGUIResourceProvider::OgreCEGUIResourceProvider() : ResourceProvider()
    {
        // set deafult resource group for Ogre
        d_defaultResourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME.c_str();
    }

//    void OgreResourceProvider::loadInputSourceContainer(const String& filename, InputSourceContainer& output)
//   {
//        Ogre::DataStreamPtr input = Ogre::ResourceGroupManager::getSingleton().openResource(filename.c_str());
//
//		if (input.isNull())
//		{
//			throw InvalidRequestException((utf8*)
//				"Scheme::Scheme - Filename supplied for Scheme loading must be valid");
//		}
//
//       XERCES_CPP_NAMESPACE_USE
//        size_t buffsz = input->size();
//        unsigned char* mem = reinterpret_cast<unsigned char*>(XMLPlatformUtils::fgArrayMemoryManager->allocate(buffsz));
//        memcpy(mem, input.getPointer()->getAsString().c_str(), buffsz);
//        InputSource* mInputSource = new MemBufInputSource(mem, buffsz, filename.c_str(), true);
//        input.setNull();
//
//       output.setData(mInputSource);
//    }

    void OgreCEGUIResourceProvider::loadRawDataContainer(const String& filename, RawDataContainer& output,  const String& resourceGroup)
    {
        String orpGroup;
        if (resourceGroup.empty())
            orpGroup = d_defaultResourceGroup.empty() ? Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME.c_str() : d_defaultResourceGroup;
        else
            orpGroup = resourceGroup;

        Ogre::DataStreamPtr input =
            Ogre::ResourceGroupManager::getSingleton().openResource(filename.c_str(), orpGroup.c_str());

		if (input.isNull())
		{
            throw InvalidRequestException((utf8*)
                "OgreCEGUIResourceProvider::loadRawDataContainer - Unable to open resource file '" + filename + (utf8*)"' in resource group '" + orpGroup + (utf8*)"'.");
        }

		Ogre::String buf = input->getAsString();
		const size_t memBuffSize = buf.length();

        unsigned char* mem = new unsigned char[memBuffSize];
        memcpy(mem, buf.c_str(), memBuffSize);

        output.setData(mem);
        output.setSize(memBuffSize);
    }

	void OgreCEGUIResourceProvider::unloadRawDataContainer(RawDataContainer& data)
	{
		if (data.getDataPtr())
		{
			delete[] data.getDataPtr();
			data.setData(0);
			data.setSize(0);
		}
	}
} // End of  CEGUI namespace section
