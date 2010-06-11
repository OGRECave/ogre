#include "ogrewrapper.h"
#include "AndroidLogListener.h"

#include <OgreGLES2Plugin.h>
#include <OgreParticleFXPlugin.h>

// Store global variables here
static Ogre::Root *g_root = 0;
static Ogre::GLES2Plugin *g_gles2Plugin = 0;
static Ogre::ParticleFXPlugin *g_pfxPlugin = 0;
static AndroidLogListener *g_ll = 0;
static Ogre::RenderWindow *g_renderWindow = 0; // The main render window

static Ogre::Timer g_timer;
static unsigned long g_lastTime = 0;

bool initOgreRoot(){
	try{
		// Create logs that funnel to android logs
		Ogre::LogManager *lm = OGRE_NEW Ogre::LogManager();
		Ogre::Log *l = lm->createLog("AndroidLog", true, true, true);
		g_ll = OGRE_NEW AndroidLogListener();
		l->addListener(g_ll);
		
		// Create a root object
		g_root = OGRE_NEW Ogre::Root("", "", "");
		
		// Register the ES2 plugin
		g_gles2Plugin = OGRE_NEW Ogre::GLES2Plugin();
		Ogre::Root::getSingleton().installPlugin(g_gles2Plugin);
		
		// Register particle plugin
		g_pfxPlugin = OGRE_NEW Ogre::ParticleFXPlugin();
		Ogre::Root::getSingleton().installPlugin(g_pfxPlugin);
		
		// Grab the available render systems
		const Ogre::RenderSystemList &renderSystemList = g_root->getAvailableRenderers();
		if(renderSystemList.empty())
		{
			return false;
		}
		
		// Set the render system and init
		Ogre::RenderSystem *system = renderSystemList.front();
		g_root->setRenderSystem(system);
		g_root->initialise(false);
		
		g_lastTime = g_timer.getMilliseconds();
		
		return true;
	}catch(Ogre::Exception &e){
	}
	return false;
}

Ogre::RenderWindow *initRenderWindow(unsigned int windowHandle, unsigned int width,
	unsigned int height, unsigned int contextHandle){
	if(g_root != 0 && g_renderWindow == 0){
		Ogre::NameValuePairList params;
		params["externalWindowHandle"] = Ogre::StringConverter::toString(windowHandle);
		params["externalGLContext"] = Ogre::StringConverter::toString(contextHandle);
		
		g_renderWindow = g_root->createRenderWindow("OgreAndroidPrimary", width, height, true, &params);
	}
	
	return g_renderWindow;
}

void destroyOgreRoot(){
	if(g_root)
		OGRE_DELETE g_root;
	if(g_gles2Plugin)
		OGRE_DELETE g_gles2Plugin;
	if(g_pfxPlugin)
		OGRE_DELETE g_pfxPlugin;
	
	g_root = 0;
	g_gles2Plugin = 0;
	g_pfxPlugin = 0;
}

void destroyRenderWindow(){
	if(g_renderWindow){
		g_renderWindow->destroy();
		Ogre::Root::getSingleton().detachRenderTarget(g_renderWindow);
	       
		g_renderWindow = 0; 
	}
}

Ogre::RenderWindow *getRenderWindow(){
	return g_renderWindow;
}

void renderOneFrame(){
	if(g_root && g_renderWindow){
		/*
		unsigned long t = g_timer.getMilliseconds();
		unsigned long d = t - g_lastTime;
		
		if(d > 16){
			g_lastTime = t;
			
			if(d > 250)
				d = 250;
			g_root->renderOneFrame(((Ogre::Real)d)*0.001f);
		}
		*/
		g_root->renderOneFrame();
	}
}