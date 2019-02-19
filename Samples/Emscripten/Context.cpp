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

#include "OgreZip.h"

#include "SamplePlugin.h"
#include "CharacterSample.h"

#include <emscripten/html5.h>
#include "Context.h"

Context::Context()
    :   OgreBites::SampleContext("OGRE Emscripten Sample"), mBuffer(NULL), mNode(NULL)
{
}

bool Context::mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) {
    OgreBites::MouseWheelEvent _evt = evt;
    // chrome reports values of 53 here
    _evt.y = std::min(3, std::max(-3, evt.y));
    return OgreBites::SampleContext::mouseWheelRolled(evt);
}

void Context::_mainLoop(void* target)
{
    Context* thizz = static_cast<Context*>(target);
    if (thizz->mRoot->endRenderingQueued())
	{
	    emscripten_cancel_main_loop();
	}
	else
    {
	    try
	    {
            if (!thizz->mRoot->renderOneFrame())
            {
                emscripten_cancel_main_loop();
            }
	    }
	    catch (Ogre::Exception& e)
	    {
            size_t length = emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_DEMANGLE | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS,0,0) + 50;
            std::vector<char> buffer(length);
            emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_DEMANGLE | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS, buffer.data(), length);
            Ogre::LogManager::getSingleton().logMessage(buffer.data());

            emscripten_pause_main_loop();
	    }
    }
}


void Context::unloadResource(Ogre::ResourceManager* resMgr, const Ogre::String& resourceName)
{
    Ogre::ResourcePtr rPtr = resMgr->getResourceByName(resourceName, "General");
    if (!rPtr)
        return;

    rPtr->unload();
    resMgr->remove(resourceName, "General");
}

void Context::destroyMaterials( const Ogre::String& resourceGroupID )
{

    try
    {
        Ogre::MaterialManager* materialManager = Ogre::MaterialManager::getSingletonPtr();
        Ogre::ResourceManager::ResourceMapIterator resourceIterator = materialManager->getResourceIterator();

        std::vector< std::string > materialNamesToRemove;

        while( resourceIterator.hasMoreElements() )
        {
            Ogre::ResourcePtr material = resourceIterator.getNext();
            std::string matName = material->getName();

            if( resourceGroupID == material->getGroup())
            {
                mShaderGenerator->removeAllShaderBasedTechniques( matName, material->getGroup() );
                material->unload();
                material.reset();
                materialNamesToRemove.push_back( matName );
            }
        }

        for( size_t i = 0; i < materialNamesToRemove.size(); ++i )
        {
            materialManager->remove( materialNamesToRemove[i], resourceGroupID );
        }
        materialManager->removeUnreferencedResources();
    }
    catch( ... )
    {
        Ogre::LogManager::getSingleton().logMessage("An Error occurred trying to destroy Materials in " + resourceGroupID);
    }

}

void Context::destroyTextures( const Ogre::String& resourceGroupID )
{
    try
    {
        Ogre::TextureManager* textureManager = Ogre::TextureManager::getSingletonPtr();
        Ogre::ResourceManager::ResourceMapIterator resourceIterator = textureManager->getResourceIterator();

        std::vector< std::string > textureNamesToRemove;

        while( resourceIterator.hasMoreElements() )
        {
            Ogre::ResourcePtr texture = resourceIterator.getNext();
            Ogre::String resourceName = texture->getName();
            if( resourceGroupID == texture->getGroup())
            {
                texture->unload();
                texture.reset();
                textureNamesToRemove.push_back( resourceName );
            }
        }

        for( size_t i = 0; i < textureNamesToRemove.size(); ++i )
        {
            textureManager->remove( textureNamesToRemove[i], resourceGroupID );
        }
    }
    catch( ... )
    {
        Ogre::LogManager::getSingleton().logMessage("An Error occurred trying to destroy Textures in " + resourceGroupID);
    }

}

void Context::clearScene()
{
    if (mBuffer != NULL)
    {
        //for (auto mo : mNode->getAttachedObjects())
        //    mSceneMgr->destroyMovableObject(mo);
        mNode->detachAllObjects();

        Ogre::MaterialManager* matMgr = Ogre::MaterialManager::getSingletonPtr();
        matMgr->removeUnreferencedResources();

        Ogre::MeshManager* meshMgr = Ogre::MeshManager::getSingletonPtr();
        meshMgr->unloadUnreferencedResources();

        Ogre::TextureManager* texMgr = Ogre::TextureManager::getSingletonPtr();
        texMgr->removeUnreferencedResources();

        if( Ogre::ResourceGroupManager::getSingleton().resourceGroupExists("Download") && Ogre::ResourceGroupManager::getSingleton().isResourceGroupInitialised("Download") )
        {
            destroyMaterials( "Download" );
            destroyTextures( "Download" );

            Ogre::ResourceGroupManager::getSingleton().removeResourceLocation( "download.zip" );
            Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup( "Download" );
        }

        Ogre::EmbeddedZipArchiveFactory::removeEmbbeddedFile("download.zip");

        mShaderGenerator->removeAllShaderBasedTechniques();
        mShaderGenerator->flushShaderCache();

        //free(mBuffer);
        mBuffer = NULL;
    }
}

void Context::passAssetAsArrayBuffer(unsigned char* arr, int length) {

    try {

        clearScene();
        Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Download");
        mBuffer = arr;

        Ogre::EmbeddedZipArchiveFactory::addEmbbeddedFile("download.zip", mBuffer, length, NULL);
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation("download.zip","EmbeddedZip","Download");
        Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Download");

        Ogre::StringVectorPtr meshes = Ogre::ArchiveManager::getSingleton().load("download.zip","EmbeddedZip",true)->find("*.mesh");
        for (auto i : *meshes)
        {
            //mNode->attachObject(mSceneMgr->createEntity(i));
        }

    } catch (Ogre::Exception& ex) {
        Ogre::LogManager::getSingleton().logMessage(ex.what());

    }


}

void Context::setup() {
    OgreBites::ApplicationContext::setup();
    mWindow = getRenderWindow();
    addInputListener(this);

    mCurrentSample = new Sample_Character();
    mCurrentSample->setShaderGenerator(mShaderGenerator);
    mCurrentSample->_setup(mWindow, mFSLayer, mOverlaySystem);
}
