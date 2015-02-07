
#include "GraphicsSystem.h"
#include "GameState.h"

using namespace Demo;

int main()
{
    GameState gameState;
    GraphicsSystem graphicsSystem( &gameState );

    graphicsSystem.initialize( "V2 Mesh importing from V1 sample" );

    graphicsSystem.createScene01();
    graphicsSystem.createScene02();

    while( !graphicsSystem.getQuit() )
    {
        graphicsSystem.update( 0 );
    }

    graphicsSystem.destroyScene();
    graphicsSystem.deinitialize();

    return 0;
}
