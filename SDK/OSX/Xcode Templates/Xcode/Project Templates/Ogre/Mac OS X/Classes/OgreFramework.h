//|||||||||||||||||||||||||||||||||||||||||||||||

#ifndef OGRE_FRAMEWORK_H
#define OGRE_FRAMEWORK_H

//|||||||||||||||||||||||||||||||||||||||||||||||

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#   include <OIS/OISMultiTouch.h>
#endif

#ifdef OGRE_STATIC_LIB
#  define OGRE_STATIC_GL
#  if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define OGRE_STATIC_Direct3D9
// dx10 will only work on vista, so be careful about statically linking
#    if OGRE_USE_D3D10
#      define OGRE_STATIC_Direct3D10
#    endif
#  endif
#  define OGRE_STATIC_BSPSceneManager
#  define OGRE_STATIC_ParticleFX
#  define OGRE_STATIC_CgProgramManager
#  ifdef OGRE_USE_PCZ
#    define OGRE_STATIC_PCZSceneManager
#    define OGRE_STATIC_OctreeZone
#  else
#    define OGRE_STATIC_OctreeSceneManager
#  endif
#  if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#     undef OGRE_STATIC_CgProgramManager
#     undef OGRE_STATIC_GL
#     define OGRE_STATIC_GLES 1
#     ifdef __OBJC__
#       import <UIKit/UIKit.h>
#     endif
#  endif
#  include "OgreStaticPluginLoader.h"
#endif

//|||||||||||||||||||||||||||||||||||||||||||||||

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
class OgreFramework : public Ogre::Singleton<OgreFramework>, OIS::KeyListener, OIS::MultiTouchListener
#else
class OgreFramework : public Ogre::Singleton<OgreFramework>, OIS::KeyListener, OIS::MouseListener
#endif
{
public:
	OgreFramework();
	~OgreFramework();

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
    bool initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MultiTouchListener *pMouseListener = 0);
#else
    bool initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MouseListener *pMouseListener = 0);
#endif
	void updateOgre(double timeSinceLastFrame);
	void updateStats();
	void moveCamera();
	void getInput();

	bool isOgreToBeShutDown()const{return m_bShutDownOgre;}  

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	bool touchMoved(const OIS::MultiTouchEvent &evt);
	bool touchPressed(const OIS::MultiTouchEvent &evt); 
	bool touchReleased(const OIS::MultiTouchEvent &evt);
	bool touchCancelled(const OIS::MultiTouchEvent &evt);
#else
	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id); 
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
#endif
	
	Ogre::Root*					m_pRoot;
	Ogre::SceneManager*			m_pSceneMgr;
	Ogre::RenderWindow*			m_pRenderWnd;
	Ogre::Camera*				m_pCamera;
	Ogre::Viewport*				m_pViewport;
	Ogre::Log*					m_pLog;
	Ogre::Timer*				m_pTimer;
	
	OIS::InputManager*			m_pInputMgr;
	OIS::Keyboard*				m_pKeyboard;
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
	OIS::MultiTouch*			m_pMouse;
#else
	OIS::Mouse*					m_pMouse;
#endif

protected:
   // Added for Mac compatibility
   Ogre::String                 m_ResourcePath;

private:
	OgreFramework(const OgreFramework&);
	OgreFramework& operator= (const OgreFramework&);

	Ogre::Overlay*				m_pDebugOverlay;
	Ogre::Overlay*				m_pInfoOverlay;
	int							m_iNumScreenShots;

	bool						m_bShutDownOgre;
	
	Ogre::Vector3				m_TranslateVector;
	Ogre::Real					m_MoveSpeed; 
	Ogre::Degree				m_RotateSpeed; 
	float						m_MoveScale;
	Ogre::Degree				m_RotScale;
#ifdef OGRE_STATIC_LIB
    Ogre::StaticPluginLoader m_StaticPluginLoader;
#endif
};

//|||||||||||||||||||||||||||||||||||||||||||||||

#endif 
