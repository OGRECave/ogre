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

#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <Ogre.h>
#include <RTShaderSystem/OgreShaderGenerator.h>
#include <ShaderGeneratorTechniqueResolverListener.h>
#include <SdkTrays.h>
#include <emscripten/html5.h>


class Sample : public Ogre::FrameListener, public OgreBites::SdkTrayListener
{
public:
	Sample();
	virtual ~Sample();

	void setupEngine();
	void setupScene();
	void startMainLoop();
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    bool frameStarted(const Ogre::FrameEvent& evt);
    
    void passAssetAsArrayBuffer(unsigned char*, int length);
    void clearScene();
    
private:
	Ogre::Root* mRoot;
	Ogre::RenderWindow* mWindow;
	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;
	bool mExitMainLoop;
	Ogre::SceneNode* mEntity;
    unsigned char* mBuffer;
	std::vector<Ogre::AnimationState*> mAnimations;
    
    bool mOrbiting;
    bool mZoom;
    
    Ogre::OverlaySystem* mOverlaySystem;
    Ogre::RTShader::ShaderGenerator* mShaderGenerator;
    Ogre::MaterialManager::Listener* mMaterialListener;
    OgreBites::SdkTrayManager* mTrayMgr;

	void parseResources();
    void destroyMaterials( Ogre::String resourceGroupID );
    void destroyTextures( Ogre::String resourceGroupID );
    void unloadResource(Ogre::ResourceManager* resMgr, const Ogre::String& resourceName);

    static EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);
    static EM_BOOL keyup_callback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);
    static EM_BOOL keypress_callback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);
    static EM_BOOL mousedown_callback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
    static EM_BOOL mouseup_callback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
    static EM_BOOL mousemove_callback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
    static EM_BOOL mousewheel_callback(int eventType, const EmscriptenWheelEvent* mouseEvent, void* userData);
    static const char* beforeunload_callback(int eventType, const void* reserved, void* userData);
    static void _mainLoop(void* target);
};
#endif
