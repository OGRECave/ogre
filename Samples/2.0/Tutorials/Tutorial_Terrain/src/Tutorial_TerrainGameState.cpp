
#include "Tutorial_TerrainGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

#include "Terra/Terra.h"
#include "OgreHlmsManager.h"
#include "OgreHlms.h"

using namespace Demo;

namespace Demo
{
    Tutorial_TerrainGameState::Tutorial_TerrainGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mTerra( 0 ),
        mSunLight( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::createScene01(void)
    {
        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                  &sceneManager->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                  sceneManager, 0, root->getCompositorManager2(),
                                  mGraphicsSystem->getCamera() );
        mTerra->setCastShadows( false );

        //mTerra->load( "Heightmap.png", Ogre::Vector3::ZERO, Ogre::Vector3( 256.0f, 1.0f, 256.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 128.0f, 5.0f, 128.0f ) );
        //mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 1024.0f, 5.0f, 1024.0f ) );
        mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 4096.0f * 4, 15.0f * 64.0f*4, 4096.0f * 4 ) );

        Ogre::SceneNode *rootNode = sceneManager->getRootSceneNode( Ogre::SCENE_STATIC );
        Ogre::SceneNode *sceneNode = rootNode->createChildSceneNode( Ogre::SCENE_STATIC );
        sceneNode->attachObject( mTerra );

        Ogre::HlmsManager *hlmsManager = root->getHlmsManager();
        Ogre::HlmsDatablock *datablock = hlmsManager->getDatablock( "TerraExampleMaterial" );
//        Ogre::HlmsDatablock *datablock = hlmsManager->getHlms( Ogre::HLMS_USER3 )->getDefaultDatablock();
//        Ogre::HlmsMacroblock macroblock;
//        macroblock.mPolygonMode = Ogre::PM_WIREFRAME;
        //datablock->setMacroblock( macroblock );
        mTerra->setDatablock( datablock );

        mSunLight = sceneManager->createLight();
        Ogre::SceneNode *lightNode = rootNode->createChildSceneNode();
        lightNode->attachObject( mSunLight );
        mSunLight->setPowerScale( Ogre::Math::PI );
        mSunLight->setType( Ogre::Light::LT_DIRECTIONAL );
        mSunLight->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        mCameraController = new CameraController( mGraphicsSystem, false );
        mGraphicsSystem->getCamera()->setFarClipDistance( 10000.0f );

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::destroyScene(void)
    {
        delete mTerra;
        mTerra = 0;

        TutorialGameState::destroyScene();
    }
    //-----------------------------------------------------------------------------------
    void Tutorial_TerrainGameState::update( float timeSinceLast )
    {
		mTerra->update( mSunLight->getDerivedDirectionUpdated() );

        TutorialGameState::update( timeSinceLast );
    }
}
