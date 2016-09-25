
#include "TutorialCompute02_UavBufferGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"
#include "OgreRoot.h"
#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
    TutorialCompute02_UavBufferGameState::TutorialCompute02_UavBufferGameState(
            const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mSceneNode( 0 ),
        mDisplacement( 0 ),
        mComputeJob( 0 ),
        mLastWindowWidth( 0 ),
        mLastWindowHeight( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void TutorialCompute02_UavBufferGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

        Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
                                                     Ogre::ResourceGroupManager::
                                                     AUTODETECT_RESOURCE_GROUP_NAME,
                                                     Ogre::SCENE_DYNAMIC );

        mSceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                createChildSceneNode( Ogre::SCENE_DYNAMIC );

        mSceneNode->attachObject( item );

        Ogre::Root *root = mGraphicsSystem->getRoot();
        Ogre::HlmsCompute *hlmsCompute = root->getHlmsManager()->getComputeHlms();
        mComputeJob = hlmsCompute->findComputeJob( "TestJob" );
        mDrawFromUavBufferMat = Ogre::MaterialManager::getSingleton().load(
                    "DrawFromUavBuffer", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void TutorialCompute02_UavBufferGameState::destroyScene(void)
    {
        mDrawFromUavBufferMat.setNull();
    }
    //-----------------------------------------------------------------------------------
    void TutorialCompute02_UavBufferGameState::update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast * 4.0f;
        mDisplacement = fmodf( mDisplacement, 10.0f );

        mSceneNode->setPosition( origin + Ogre::Vector3::UNIT_X * mDisplacement );

        Ogre::RenderWindow *renderWindow = mGraphicsSystem->getRenderWindow();
        if( mLastWindowWidth != renderWindow->getWidth() ||
            mLastWindowHeight != renderWindow->getHeight() )
        {
            Ogre::uint32 res[2];
            res[0] = renderWindow->getWidth();
            res[1] = renderWindow->getHeight();

            //Update the compute shader's
            Ogre::ShaderParams &shaderParams = mComputeJob->getShaderParams( "default" );
            Ogre::ShaderParams::Param *texResolution = shaderParams.findParameter( "texResolution" );
            texResolution->setManualValue( res, sizeof(res) / sizeof(Ogre::uint32) );
            shaderParams.setDirty();

            mComputeJob->setNumThreadGroups( (res[0] + mComputeJob->getThreadsPerGroupX() - 1u) /
                                             mComputeJob->getThreadsPerGroupX(),
                                             (res[1] + mComputeJob->getThreadsPerGroupY() - 1u) /
                                             mComputeJob->getThreadsPerGroupY(), 1u );

            //Update the pass that draws the UAV Buffer into the RTT (we could
            //use auto param viewport_size, but this more flexible)
            Ogre::GpuProgramParametersSharedPtr psParams = mDrawFromUavBufferMat->getTechnique(0)->
                    getPass(0)->getFragmentProgramParameters();
            psParams->setNamedConstant( "texResolution", res, 1u, 2u );

            mLastWindowWidth  = renderWindow->getWidth();
            mLastWindowHeight = renderWindow->getHeight();
        }

        TutorialGameState::update( timeSinceLast );
    }
}
