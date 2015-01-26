
#include "LogicGameState.h"
#include "GraphicsGameState.h"

#include "OgreVector3.h"

using namespace Demo;

namespace Demo
{
    LogicGameState::LogicGameState() :
        mDisplacement( 0 ),
        mGraphicsGameState( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::_notifyGraphicsGameState( GraphicsGameState *graphicsGameState )
    {
        mGraphicsGameState = graphicsGameState;
    }
    //-----------------------------------------------------------------------------------
    void LogicGameState::update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast * 4.0f;
        mDisplacement = fmodf( mDisplacement, 10.0f );

        Ogre::Vector3 &lastPosition = mGraphicsGameState->_getLastPositionRef();

        lastPosition = mGraphicsGameState->_getCurrentPositionRef();
        mGraphicsGameState->_getCurrentPositionRef() = origin + Ogre::Vector3::UNIT_X * mDisplacement;

        GameState::update( timeSinceLast );
    }
}
