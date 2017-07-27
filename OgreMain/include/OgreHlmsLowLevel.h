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
#ifndef _OgreHlmsLowLevel_H_
#define _OgreHlmsLowLevel_H_

#include "OgreHlms.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    /** This is an HLMS implementation that acts as proxy to use the Material system from
        Ogre 1.9
        The older material system is data-driven (thanks to AutoParamDataSource) compared
        to HLMS where the user needs to write its own implementation in C++ (or modify an
        exisiting one). Old material system is still useful for:
            * Quick prototyping of shaders
            * Postprocessing effects.
        Take in mind that the old system is __slow__ compared to Hlms. So don't use this
        proxy for hundreds or more entities.
    @remarks
        Only Materials with programmable shaders are supported. Fixed Function pipeline
        is not allowed (use the other HLMS types if you need easy to use materials)
    */
    class _OgreExport HlmsLowLevel : public Hlms
    {
        OGRE_SIMD_ALIGNED_DECL( Matrix4, mTempXform[256] );
        AutoParamDataSource *mAutoParamDataSource;
        SceneManager    *mCurrentSceneManager;

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

    public:
        HlmsLowLevel();
        ~HlmsLowLevel();

        AutoParamDataSource* _getAutoParamDataSource(void) const    { return mAutoParamDataSource; }

        virtual void calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash );

        virtual uint32 fillBuffersForV1( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        virtual uint32 fillBuffersForV2( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        void executeCommand( const MovableObject *movableObject, Renderable *renderable,
                             bool casterPass );
    };

    struct _OgreExport LowLevelProp
    {
        static const IdString PassId;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
