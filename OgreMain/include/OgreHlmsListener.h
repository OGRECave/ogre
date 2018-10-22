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
#ifndef _OgreHlmsListener_H_
#define _OgreHlmsListener_H_

#include "OgreHlmsCommon.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** Listener that can be hooked to an Hlms implementation for extending it with custom
        code. See "8.6.5 Customizing an existing implementation" of the 2.x manual
        on the different approaches to customizing Hlms implementations.
    @remarks
        For performance reasons, the listener interface does not allow you to add
        customizations that work per Renderable, as that loop is performance sensitive.
        The only listener callback that works inside Hlms::fillBuffersFor is hlmsTypeChanged
        which only gets evaluated when the previous Renderable used a different Hlms
        implementation; which is rare, and since we sort the RenderQueue, it often branch
        predicts well.
    */
    class _OgreExport HlmsListener
    {
    public:
        /** Similar to HlmsListener::shaderCacheEntryCreated, but it gets called before creating
            any shader. The main difference is that there is no hlmsCacheEntry (because it hasn't been
            generated yet) and the properties are before they are transformed by the templates
        @brief propertiesMergedPreGenerationStep
        @param shaderProfile
        @param passCache
            Properties used by this pass
        @param renderableCacheProperties
            Properties assigned to the renderable
        @param renderableCachePieces
            Pieces that can be inserted, belonging to the renderable.
            The PiecesMap pointer cannot be null
        @param properties
            Combined properties of both renderableCacheProperties & passCache.setProperties
        @param queuedRenderable
        */
        virtual void propertiesMergedPreGenerationStep(
                const String &shaderProfile,
                const HlmsCache &passCache,
                const HlmsPropertyVec &renderableCacheProperties,
                const PiecesMap renderableCachePieces[NumShaderTypes],
                const HlmsPropertyVec &properties,
                const QueuedRenderable &queuedRenderable ) {}

        /** Called after the shader was created/compiled, and right before
            bindGpuProgramParameters (relevant information for OpenGL programs).
        @param shaderProfile
            @see Hlms::mShaderProfile
        @param hlmsCacheEntry
            The created shader.
        @param passCache
            @see Hlms::createShaderCacheEntry
        @param properties
            The current contents of Hlms::mSetProperties
        @param queuedRenderable
            @see Hlms::createShaderCacheEntry
        */
        virtual void shaderCacheEntryCreated( const String &shaderProfile,
                                              const HlmsCache *hlmsCacheEntry,
                                              const HlmsCache &passCache,
                                              const HlmsPropertyVec &properties,
                                              const QueuedRenderable &queuedRenderable ) {}

        /** Called right before creating the pass cache, to allow the listener
            to add/remove properties.
        @remarks
            For the rest of the parameters, @see Hlms::preparePassHash
        @param hlms
            Caller Hlms; from which you can alter the properties using Hlms::setProperty
        */
        virtual void preparePassHash( const CompositorShadowNode *shadowNode,
                                      bool casterPass, bool dualParaboloid,
                                      SceneManager *sceneManager, Hlms *hlms ) {}

        /// Listeners should return the extra bytes they wish to allocate for storing additional
        /// data in the pass buffer. Return value must be in bytes.
        virtual uint32 getPassBufferSize( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager ) const
                                                                    { return 0; }

        /// Users can write to passBufferPtr. Implementations must ensure they make the buffer
        /// big enough via getPassBufferSize.
        /// The passBufferPtr is already aligned to 16 bytes.
        /// Implementations must return the pointer past the end, aligned to 16 bytes.
        virtual float* preparePassBuffer( const CompositorShadowNode *shadowNode, bool casterPass,
                                          bool dualParaboloid, SceneManager *sceneManager,
                                          float *passBufferPtr )    { return passBufferPtr; }

        /// Called when the last Renderable processed was of a different Hlms type, thus we
        /// need to rebind certain buffers (like the pass buffer). You can use
        /// this moment to bind your own buffers.
        virtual void hlmsTypeChanged( bool casterPass, CommandBuffer *commandBuffer,
                                      const HlmsDatablock *datablock ) {}
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
