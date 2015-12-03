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

#include "OgreStableHeaders.h"

#include "Compositor/Pass/PassMipmap/OgreCompositorPassMipmap.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"

#include "OgreRenderSystem.h"
#include "OgreRenderTexture.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{
    CompositorPassMipmap::CompositorPassMipmap( const CompositorPassMipmapDef *definition,
                                                const CompositorChannel &target,
                                                CompositorNode *parentNode ) :
                CompositorPass( definition, target, parentNode ),
                mDefinition( definition )
    {
        mTextures = target.textures;

        if( mTextures.empty() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "PASS_MIPMAP can only be used with RenderTargets that can "
                         "be interpreted as textures",
                         "CompositorPassMipmap::CompositorPassMipmap" );
        }
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassMipmap::execute( const Camera *lodCamera )
    {
        //Execute a limited number of times?
        if( mNumPassesLeft != std::numeric_limits<uint32>::max() )
        {
            if( !mNumPassesLeft )
                return;
            --mNumPassesLeft;
        }

        executeResourceTransitions();

        //Fire the listener in case it wants to change anything
        CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if( listener )
            listener->passPreExecute( this );

        TextureVec::const_iterator itor = mTextures.begin();
        TextureVec::const_iterator end  = mTextures.end();

        while( itor != end )
        {
            (*itor)->_autogenerateMipmaps();
            ++itor;
        }

        if( listener )
            listener->passPosExecute( this );
    }
    //-----------------------------------------------------------------------------------
    void CompositorPassMipmap::_placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                                     ResourceAccessMap &uavsAccess,
                                                                     ResourceLayoutMap &resourcesLayout )
    {
        RenderSystem *renderSystem = mParentNode->getRenderSystem();
        const RenderSystemCapabilities *caps = renderSystem->getCapabilities();
        const bool explicitApi = caps->hasCapability( RSC_EXPLICIT_API );

        //Check <anything> -> RT for every RTT in the textures we'll be generating mipmaps.
        TextureVec::const_iterator itTex = mTextures.begin();
        TextureVec::const_iterator enTex = mTextures.end();

        while( itTex != enTex )
        {
            const Ogre::TexturePtr tex = *itTex;
            const size_t numFaces = tex->getNumFaces();
            const uint8 numMips = tex->getNumMipmaps() + 1;
            const uint32 numSlices = tex->getTextureType() == TEX_TYPE_CUBE_MAP ? 1u :
                                                                                  tex->getDepth();
            for( size_t face=0; face<numFaces; ++face )
            {
                for( uint8 mip=0; mip<numMips; ++mip )
                {
                    for( uint32 slice=0; slice<numSlices; ++slice )
                    {
                        RenderTarget *rt = tex->getBuffer( face, mip )->getRenderTarget( slice );
                        ResourceLayoutMap::iterator currentLayout = resourcesLayout.find( rt );
                        if( (currentLayout->second != ResourceLayout::RenderTarget && explicitApi) ||
                            currentLayout->second == ResourceLayout::Uav )
                        {
                            addResourceTransition( currentLayout,
                                                   ResourceLayout::RenderTarget,
                                                   ReadBarrier::RenderTarget );
                        }
                    }
                }
            }

            ++itTex;
        }

        //Do not use base class functionality at all.
        //CompositorPass::_placeBarriersAndEmulateUavExecution();
    }
}
