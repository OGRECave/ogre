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
#ifndef _OgreHlmsGui2DMobile_H_
#define _OgreHlmsGui2DMobile_H_

#include "OgreHlms.h"
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

    /** User Interface (2D) implementation specfically designed for OpenGL ES 2.0 and other
        RenderSystems which do not support uniform buffers.
    */
    class _OgreExport HlmsGui2DMobile : public Hlms
    {
        struct PassData
        {
            /*FastArray<float>    vertexShaderSharedBuffer;
            FastArray<float>    pixelShaderSharedBuffer;*/

            Matrix4 viewProjMatrix;
        };

        PassData    mPreparedPass;

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( const HlmsParamVec &paramVec,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    IdString datablockName );

    public:
        HlmsGui2DMobile( Archive *dataFolder );
        ~HlmsGui2DMobile();

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual void fillBuffersFor(const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                     bool casterPass, const HlmsCache *lastCache,
                                     uint32 lastTextureHash );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
