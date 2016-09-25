
#include "V2ManualObjectGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreManualObject2.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace Demo;

namespace Demo
{
    const size_t GridSize = 15;
    const float GridStep = 1.0f / GridSize;

    V2ManualObjectGameState::V2ManualObjectGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription ),
        mManualObject(0),
        mFirstFrame(true),
        mAccumulator(0.0f),
        mAnimate(true)
    {
    }
    //-----------------------------------------------------------------------------------
    void V2ManualObjectGameState::createScene01(void)
    {
        Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();


        //---------------------------------------------------------------------------------------
        //
        //---------------------------------------------------------------------------------------
        Ogre::ManualObject * manualObject = sceneManager->createManualObject();

        manualObject->begin("BaseWhite", Ogre::OT_LINE_LIST);

        // Back
        manualObject->position(0.0f, 0.0f, 0.0f);
        manualObject->position(0.0f, 1.0f, 0.0f);
        manualObject->line(0, 1);

        manualObject->position(0.0f, 1.0f, 0.0f);
        manualObject->position(1.0f, 1.0f, 0.0f);
        manualObject->line(2, 3);

        manualObject->position(1.0f, 1.0f, 0.0f);
        manualObject->position(1.0f, 0.0f, 0.0f);
        manualObject->line(4, 5);

        manualObject->position(1.0f, 0.0f, 0.0f);
        manualObject->position(0.0f, 0.0f, 0.0f);
        manualObject->line(6, 7);

        // Front
        manualObject->position(0.0f, 0.0f, 1.0f);
        manualObject->position(0.0f, 1.0f, 1.0f);
        manualObject->line(8, 9);

        manualObject->position(0.0f, 1.0f, 1.0f);
        manualObject->position(1.0f, 1.0f, 1.0f);
        manualObject->line(10, 11);

        manualObject->position(1.0f, 1.0f, 1.0f);
        manualObject->position(1.0f, 0.0f, 1.0f);
        manualObject->line(12, 13);

        manualObject->position(1.0f, 0.0f, 1.0f);
        manualObject->position(0.0f, 0.0f, 1.0f);
        manualObject->line(14, 15);

        // Sides
        manualObject->position(0.0f, 0.0f, 0.0f);
        manualObject->position(0.0f, 0.0f, 1.0f);
        manualObject->line(16, 17);

        manualObject->position(0.0f, 1.0f, 0.0f);
        manualObject->position(0.0f, 1.0f, 1.0f);
        manualObject->line(18, 19);

        manualObject->position(1.0f, 0.0f, 0.0f);
        manualObject->position(1.0f, 0.0f, 1.0f);
        manualObject->line(20, 21);

        manualObject->position(1.0f, 1.0f, 0.0f);
        manualObject->position(1.0f, 1.0f, 1.0f);
        manualObject->line(22, 23);

        manualObject->end();

        Ogre::SceneNode *sceneNodeLines = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                     createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNodeLines->attachObject(manualObject);
        sceneNodeLines->scale(4.0f, 4.0f, 4.0f);
        sceneNodeLines->translate(-4.5f, -1.5f, 0.0f, Ogre::SceneNode::TS_LOCAL);
        //---------------------------------------------------------------------------------------
        //
        //---------------------------------------------------------------------------------------
        for (size_t i = 0; i < GridSize; i++)
        {
            for (size_t j = 0; j < GridSize; j++)
            {
                mVertices.push_back(Ogre::Vector3(GridStep * i, GridStep * j, 0.0f));
                mVertices.push_back(Ogre::Vector3(GridStep * (i + 1), GridStep * j, 0.0f));
                mVertices.push_back(Ogre::Vector3(GridStep * i, GridStep * (j + 1), 0.0f));
                mVertices.push_back(Ogre::Vector3(GridStep * (i + 1), GridStep * (j + 1), 0.0f));
            }
        }

        mManualObject = sceneManager->createManualObject();

        mManualObject->begin("Rocks", Ogre::OT_TRIANGLE_LIST);

        fillBuffer(0.0f);

        mManualObject->end();

        Ogre::SceneNode *sceneNodeGrid = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                                     createChildSceneNode( Ogre::SCENE_DYNAMIC );
        sceneNodeGrid->attachObject(mManualObject);
        sceneNodeGrid->scale(4.0f, 4.0f, 4.0f);
        sceneNodeGrid->translate(1.5f, -1.5f, 0.0f, Ogre::SceneNode::TS_LOCAL);
        //---------------------------------------------------------------------------------------
        //
        //---------------------------------------------------------------------------------------

        Ogre::Light *light = sceneManager->createLight();
        Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
        lightNode->attachObject( light );
        light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
        light->setType( Ogre::Light::LT_DIRECTIONAL );
        light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

        //mCameraController = new CameraController( mGraphicsSystem, false );

        TutorialGameState::createScene01();
    }

    void V2ManualObjectGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );

        outText += "\nPress F2 to toggle animation ";
        outText += mAnimate ? "[On]" : "[Off]";

        if (mAnimate)
        {
            std::stringstream ss;
            ss << "\n[currently animating " << mVertices.size() << " vertices]";
            outText += ss.str();
        }
    }

    void V2ManualObjectGameState::update(float timeSinceLast)
    {
        TutorialGameState::update(timeSinceLast);

        // Avoid updating buffer twice for the first time
        if (mFirstFrame)
        {
            mFirstFrame = false;
            return;
        }

        if (mAnimate)
        {
            // Simple forward/backward moving quad animation
            for (size_t i = 0; i < mVertices.size(); i++)
            {
                mVertices[i].z = Ogre::Math::Sin(mAccumulator) * i * 0.002f;
            }

            mAccumulator += timeSinceLast;

            // Update section 0 with the new vertex data
            // Make sure the data layout is the same that was used
            // when creating the section with a sequence between
            // ManualObject::begin and ManualObject::end
            mManualObject->beginUpdate(0);

            fillBuffer(fmod(mAccumulator, 1.0f));

            mManualObject->end();
        }
    }

    void V2ManualObjectGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        if( arg.keysym.sym == SDLK_F2 )
        {
            mAnimate = !mAnimate;
        }
        else
        {
            TutorialGameState::keyReleased( arg );
        }
    }

    void V2ManualObjectGameState::fillBuffer(float uvOffset)
    {
        for (size_t i = 0; i < mVertices.size();)
        {
            mManualObject->position(mVertices[i]);
            mManualObject->normal(0.0f, 1.0f, 0.0f);
            mManualObject->tangent(1.0f, 0.0f, 0.0f);
            mManualObject->textureCoord(0.0f + uvOffset, 0.0f + uvOffset);

            mManualObject->position(mVertices[i + 1]);
            mManualObject->normal(0.0f, 1.0f, 0.0f);
            mManualObject->tangent(1.0f, 0.0f, 0.0f);
            mManualObject->textureCoord(1.0f + uvOffset, 0.0f + uvOffset);

            mManualObject->position(mVertices[i + 2]);
            mManualObject->normal(0.0f, 1.0f, 0.0f);
            mManualObject->tangent(1.0f, 0.0f, 0.0f);
            mManualObject->textureCoord(0.0f + uvOffset, 1.0f + uvOffset);

            mManualObject->position(mVertices[i + 3]);
            mManualObject->normal(0.0f, 1.0f, 0.0f);
            mManualObject->tangent(1.0f, 0.0f, 0.0f);
            mManualObject->textureCoord(1.0f + uvOffset, 1.0f + uvOffset);

            mManualObject->quad(i, i + 1, i + 3, i + 2);

            i += 4;
        }
    }
}
