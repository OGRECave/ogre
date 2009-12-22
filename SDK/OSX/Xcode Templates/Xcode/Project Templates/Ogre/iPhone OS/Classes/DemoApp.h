//|||||||||||||||||||||||||||||||||||||||||||||||

#ifndef OGRE_DEMO_H
#define OGRE_DEMO_H

//|||||||||||||||||||||||||||||||||||||||||||||||

#include "OgreFramework.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

class DemoApp : public OIS::KeyListener
{
public:
	DemoApp();
	~DemoApp();

	void startDemo();
	
	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

private:
	void setupDemoScene();
	void runDemo();

	Ogre::SceneNode*			m_pCubeNode;
	Ogre::Entity*				m_pCubeEntity;

	bool						m_bShutdown;
};

//|||||||||||||||||||||||||||||||||||||||||||||||

#endif 

//|||||||||||||||||||||||||||||||||||||||||||||||