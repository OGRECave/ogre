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

#include "OgreHlmsGui2DMobilePrerequisites.h"
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
    class _OgreHlmsGui2DMobileExport HlmsGui2DMobile : public Hlms
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

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

    public:
        HlmsGui2DMobile( Archive *dataFolder );
        ~HlmsGui2DMobile();

        virtual void calculateHashFor( Renderable *renderable, const HlmsParamVec &params,
                                       uint32 &outHash, uint32 &outCasterHash );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual void fillBuffersFor(const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                     bool casterPass, const HlmsCache *lastCache,
                                     uint32 lastTextureHash );

        static const IdString PropertyTexMatrixCount;
        static const IdString PropertyTexMatrixCount0;
        static const IdString PropertyTexMatrixCount1;
        static const IdString PropertyTexMatrixCount2;
        static const IdString PropertyTexMatrixCount3;
        static const IdString PropertyTexMatrixCount4;
        static const IdString PropertyTexMatrixCount5;
        static const IdString PropertyTexMatrixCount6;
        static const IdString PropertyTexMatrixCount7;

        static const IdString PropertyDiffuse;
        static const IdString PropertyDiffuseMapCount0;
        static const IdString PropertyDiffuseMapCount1;
        static const IdString PropertyDiffuseMapCount2;
        static const IdString PropertyDiffuseMapCount3;
        static const IdString PropertyDiffuseMapCount4;
        static const IdString PropertyDiffuseMapCount5;
        static const IdString PropertyDiffuseMapCount6;
        static const IdString PropertyDiffuseMapCount7;
        static const IdString PropertyDiffuseMapCount8;
        static const IdString PropertyDiffuseMapCount9;
        static const IdString PropertyDiffuseMapCount10;
        static const IdString PropertyDiffuseMapCount11;
        static const IdString PropertyDiffuseMapCount12;
        static const IdString PropertyDiffuseMapCount13;
        static const IdString PropertyDiffuseMapCount14;
        static const IdString PropertyDiffuseMapCount15;

        static const IdString PropertyBlendModeIdx0;
        static const IdString PropertyBlendModeIdx1;
        static const IdString PropertyBlendModeIdx2;
        static const IdString PropertyBlendModeIdx3;
        static const IdString PropertyBlendModeIdx4;
        static const IdString PropertyBlendModeIdx5;
        static const IdString PropertyBlendModeIdx6;
        static const IdString PropertyBlendModeIdx7;
        static const IdString PropertyBlendModeIdx8;
        static const IdString PropertyBlendModeIdx9;
        static const IdString PropertyBlendModeIdx10;
        static const IdString PropertyBlendModeIdx11;
        static const IdString PropertyBlendModeIdx12;
        static const IdString PropertyBlendModeIdx13;
        static const IdString PropertyBlendModeIdx14;
        static const IdString PropertyBlendModeIdx15;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
