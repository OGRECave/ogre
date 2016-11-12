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

#include "Sample.h"
#include <OgreSceneManager.h>
#include <OgreManualObject.h>
#include <OgrePrefabFactory.h>
#include <OgreGLES2RenderSystem.h>
#include <OgreZip.h>

#include <emscripten/html5.h>

#define SAFE_DELETE(x) if(x){delete x;  x= NULL;}

Sample::Sample()
    :   OgreBites::ApplicationContext("OGRE Emscripten Sample", false), mSceneMgr(NULL), mCamera(NULL), mNode(NULL), mBuffer(NULL)
{
}

void Sample::setup()
{
    OgreBites::ApplicationContext::setup();

    // Create the scenemanger
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
    mShaderGenerator->addSceneManager(mSceneMgr);
    mSceneMgr->addRenderQueueListener(mOverlaySystem);
    
    // Setup UI
    mTrayMgr = OGRE_NEW OgreBites::TrayManager("InterfaceName", mWindow);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);

    // Create Camera / Viewport
    setupScene();
}

bool Sample::mouseMoved(const OgreBites::MouseMotionEvent& evt) {
    mCameraMan->injectMouseMove(evt);
    mTrayMgr->injectMouseMove(evt);
    return true;
}

bool Sample::mouseWheelRolled(const OgreBites::MouseWheelEvent& evt) {
    OgreBites::MouseWheelEvent _evt = evt;
    // chrome reports values of 53 here
    _evt.y = std::min(3, std::max(-3, evt.y));
    mCameraMan->injectMouseWheel(_evt);
    return true;
}

bool Sample::mousePressed(const OgreBites::MouseButtonEvent& evt) {
    mCameraMan->injectMouseDown(evt);
    return true;
}

bool Sample::mouseReleased(const OgreBites::MouseButtonEvent& evt) {
    mCameraMan->injectMouseUp(evt);
    return true;
}

bool Sample::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    mAnimation->addTime(evt.timeSinceLastFrame);

    mTrayMgr->frameRenderingQueued(evt);
    return true;
}

void Sample::_mainLoop(void* target)
{
    Sample* thizz = static_cast<Sample*>(target);
    if (thizz->mRoot->endRenderingQueued())
	{
	    emscripten_cancel_main_loop();
	}
	else
    {
	    try
	    {
            //Pump messages in all registered RenderWindow windows
            Ogre::WindowEventUtilities::messagePump();
            
            if (!thizz->mRoot->renderOneFrame())
            {
                emscripten_cancel_main_loop();
            }
	    }
	    catch (Ogre::Exception& e)
	    {
            size_t length = emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_DEMANGLE | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS,0,0) + 50;
            char* buffer = new char[length];
            emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_DEMANGLE | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS, buffer, length);
            Ogre::LogManager::getSingleton().logMessage(buffer);
            delete[] buffer;
            
            emscripten_pause_main_loop();
	    }
    }
}

void Sample::setupScene()
{
    mSceneMgr->setSkyBox(true, "SkyBox");
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));

	Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Ogre::Light* pDirLight = mSceneMgr->createLight();
	pDirLight->setPosition(Ogre::Vector3(0,10,15));
	pNode->attachObject(pDirLight);
    
    mCamera = mSceneMgr->createCamera("MyCam");
	mCamera->setNearClipDistance(1.0f);

	mCamera->lookAt(0,0,0);
	mCamera->setAutoAspectRatio(true);

	mCameraMan = new OgreBites::CameraMan(mCamera);
	mCameraMan->setStyle(OgreBites::CS_ORBIT);
	mCameraMan->setYawPitchDist(Ogre::Radian(0),Ogre::Radian(0.3),15.0f);

    mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("EntityNode");
    Ogre::Entity* ent = mSceneMgr->createEntity("Sinbad.mesh");
    mNode->attachObject(ent);
    
    mAnimation = ent->getAnimationState("IdleTop");
    mAnimation->setEnabled(true);

	Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0.3,0.3,0.3));
}

void Sample::unloadResource(Ogre::ResourceManager* resMgr, const Ogre::String& resourceName)
{
    Ogre::ResourcePtr rPtr = resMgr->getResourceByName(resourceName);
    if (rPtr.isNull())
        return;
    
    rPtr->unload();
    resMgr->remove(resourceName);
}

void Sample::destroyMaterials( Ogre::String resourceGroupID )
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
                mShaderGenerator->removeAllShaderBasedTechniques( matName );
                material->unload();
                material.setNull();
                materialNamesToRemove.push_back( matName );
            }
        }
        
        for( size_t i = 0; i < materialNamesToRemove.size(); ++i )
        {
            materialManager->remove( materialNamesToRemove[i] );
        }
        materialManager->removeUnreferencedResources();
    }
    catch( ... )
    {
        Ogre::LogManager::getSingleton().logMessage("An Error occurred trying to destroy Materials in " + resourceGroupID);
    }
    
}

void Sample::destroyTextures( Ogre::String resourceGroupID )
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
                texture.setNull();
                textureNamesToRemove.push_back( resourceName );
            }
        }
        
        for( size_t i = 0; i < textureNamesToRemove.size(); ++i )
        {
            textureManager->remove( textureNamesToRemove[i] );
        }
    }
    catch( ... )
    {
        Ogre::LogManager::getSingleton().logMessage("An Error occurred trying to destroy Textures in " + resourceGroupID);
    }
    
}

void Sample::clearScene()
{
    if (mBuffer != NULL)
    {
        auto it = mNode->getAttachedObjectIterator();
        while (it.hasMoreElements())
        {
            mSceneMgr->destroyMovableObject(it.getNext());
        }
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

void Sample::passAssetAsArrayBuffer(unsigned char* arr, int length) {
    
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
            mNode->attachObject(mSceneMgr->createEntity(i));
        }
        
    } catch (Ogre::Exception ex) {
        Ogre::LogManager::getSingleton().logMessage(ex.what());
        
    }
    
}
