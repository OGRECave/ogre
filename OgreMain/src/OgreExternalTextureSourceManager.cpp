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

/***************************************************************************
OgreExternalTextureSourceManager.cpp  -  
    Implementation of the manager class

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/

#include "OgreStableHeaders.h"
#include "OgreExternalTextureSourceManager.h"


namespace Ogre 
{
    //****************************************************************************************
    template<> ExternalTextureSourceManager* Singleton<ExternalTextureSourceManager>::msSingleton = 0;
    ExternalTextureSourceManager* ExternalTextureSourceManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    ExternalTextureSourceManager& ExternalTextureSourceManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //****************************************************************************************

    //****************************************************************************************
    ExternalTextureSourceManager::ExternalTextureSourceManager()
    {
        mCurrExternalTextureSource = 0;
    }

    //****************************************************************************************
    ExternalTextureSourceManager::~ExternalTextureSourceManager()
    {
        mTextureSystems.clear();
    }

    //****************************************************************************************
    
    void ExternalTextureSourceManager::setCurrentPlugIn( const String& sTexturePlugInType )
    {
        TextureSystemList::iterator i;
            
        for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
        {
            if( i->first == sTexturePlugInType )
            {
                mCurrExternalTextureSource = i->second;
                mCurrExternalTextureSource->initialise();   //Now call overridden Init function
                return;
            }
        }
        mCurrExternalTextureSource = 0;
    }

    //****************************************************************************************
    void ExternalTextureSourceManager::destroyAdvancedTexture( const String& sTextureName,
        const String& groupName )
    {
        TextureSystemList::iterator i;
        for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
        {
            //Broadcast to every registered System... Only the true one will destroy texture
            i->second->destroyAdvancedTexture( sTextureName, groupName );
        }
    }

    //****************************************************************************************
    void ExternalTextureSourceManager::setExternalTextureSource( const String& sTexturePlugInType, ExternalTextureSource* pTextureSystem )
    {
        LogManager::getSingleton().logMessage( "Registering Texture Controller: Type = "
                        + sTexturePlugInType + " Name = " + pTextureSystem->getPluginStringName());

        TextureSystemList::iterator i;
            
        for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
        {
            if( i->first == sTexturePlugInType )
            {
                LogManager::getSingleton().logMessage( "Shutting Down Texture Controller: " 
                        + i->second->getPluginStringName() 
                        + " To be replaced by: "
                        + pTextureSystem->getPluginStringName());

                i->second->shutDown();              //Only one plugIn of Sent Type can be registered at a time
                                                    //so shut down old plugin before starting new plugin
                i->second = pTextureSystem;
                // **Moved this line b/c Rendersystem needs to be selected before things
                // such as framelistners can be added
                // pTextureSystem->Initialise();
                return;
            }
        }
        mTextureSystems[sTexturePlugInType] = pTextureSystem;   //If we got here then add it to map
    }

    //****************************************************************************************
    ExternalTextureSource* ExternalTextureSourceManager::getExternalTextureSource( const String& sTexturePlugInType )
    {
        TextureSystemList::iterator i;
        for( i = mTextureSystems.begin(); i != mTextureSystems.end(); ++i )
        {
            if( i->first == sTexturePlugInType )
                return i->second;
        }
        return 0;
    }

    //****************************************************************************************

}  //End Ogre Namespace

