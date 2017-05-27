/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#include "OgreStableHeaders.h"

#include "OgrePlanarReflections.h"
#include "OgreSceneManager.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderTexture.h"
#include "Compositor/OgreCompositorManager2.h"
#include "OgreLogManager.h"

namespace Ogre
{
    PlanarReflections::PlanarReflections( SceneManager *sceneManager,
                                          CompositorManager2 *compositorManager ) :
        mSceneManager( sceneManager ),
        mCompositorManager( compositorManager )
    {
    }
    //-----------------------------------------------------------------------------------
    PlanarReflections::~PlanarReflections()
    {
        destroyAllActors();
    }
    //-----------------------------------------------------------------------------------
    PlanarReflectionActor* PlanarReflections::addActor( const PlanarReflectionActor &actor,
                                                        bool useAccurateLighting,
                                                        uint32 width, uint32 height, bool withMipmaps,
                                                        PixelFormat pixelFormat,
                                                        bool mipmapMethodCompute )
    {
        const size_t uniqueId = Id::generateNewId<PlanarReflections>();

        mActors.push_back( new PlanarReflectionActor( actor ) );
        PlanarReflectionActor *newActor = mActors.back();
        String cameraName = actor.masterCamera->getName();
        cameraName += "_PlanarReflectionActor #" + StringConverter::toString( uniqueId );
        newActor->reflectionCamera = mSceneManager->createCamera( cameraName, useAccurateLighting );
        newActor->reflectionCamera->enableReflection( newActor->plane );

        int usage = TU_RENDERTARGET;
        usage |= (withMipmaps && mipmapMethodCompute) ? TU_UAV : TU_AUTOMIPMAP;
        const uint32 numMips = withMipmaps ? 0 : PixelUtil::getMaxMipmapCount( width, height, 1u );

        newActor->reflectionTexture =
                TextureManager::getSingleton().createManual(
                    "PlanarReflections #" + StringConverter::toString( uniqueId ),
                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    TEX_TYPE_2D, width, height, numMips, pixelFormat, usage, 0, true );

        CompositorChannel channel;
        channel.textures.push_back( newActor->reflectionTexture );
        channel.target = newActor->reflectionTexture->getBuffer()->getRenderTarget();
        CompositorChannelVec channels;
        channels.push_back( channel );
        newActor->workspace = mCompositorManager->addWorkspace( mSceneManager, channels,
                                                               newActor->reflectionCamera,
                                                               newActor->workspaceName, false );
        return newActor;
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflections::destroyActor( PlanarReflectionActor *actor )
    {
        PlanarReflectionActorVec::iterator itor = std::find( mActors.begin(), mActors.end(), actor );

        if( itor == mActors.end() )
        {
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND,
                         "Actor was not created by this PlanarReflections class "
                         "or was already destroyed!", "PlanarReflections::destroyActor" );
        }

        mCompositorManager->removeWorkspace( actor->workspace );
        mSceneManager->destroyCamera( actor->reflectionCamera );
        TextureManager::getSingleton().remove( actor->reflectionTexture );
        delete actor;

        efficientVectorRemove( mActors, itor );
    }
    //-----------------------------------------------------------------------------------
    void PlanarReflections::destroyAllActors(void)
    {
        PlanarReflectionActorVec::iterator itor = mActors.begin();
        PlanarReflectionActorVec::iterator end  = mActors.end();

        while( itor != end )
        {
            PlanarReflectionActor *actor = *itor;
            mCompositorManager->removeWorkspace( actor->workspace );
            mSceneManager->destroyCamera( actor->reflectionCamera );
            TextureManager::getSingleton().remove( actor->reflectionTexture );
            delete actor;
            ++itor;
        }

        mActors.clear();
    }
}
