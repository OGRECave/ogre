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
#ifndef _OgreHlmsUnlitMobile_H_
#define _OgreHlmsUnlitMobile_H_

#include "OgreHlmsUnlitMobilePrerequisites.h"
#include "OgreHlms.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"
#include "OgreMatrix4.h"

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
    class _OgreHlmsUnlitMobileExport HlmsUnlitMobile : public Hlms
    {
        struct PassData
        {
            /*FastArray<float>    vertexShaderSharedBuffer;
            FastArray<float>    pixelShaderSharedBuffer;*/

            Matrix4 viewProjMatrix[2];
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
        HlmsUnlitMobile( Archive *dataFolder, ArchiveVec *libraryFolders );
        ~HlmsUnlitMobile();

        virtual void calculateHashFor( Renderable *renderable, uint32 &outHash, uint32 &outCasterHash );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       CommandBuffer *commandBuffer );
    };

    struct _OgreHlmsUnlitMobileExport UnlitMobileProp
    {
        static const IdString TexMatrixCount;
        static const IdString TexMatrixCount0;
        static const IdString TexMatrixCount1;
        static const IdString TexMatrixCount2;
        static const IdString TexMatrixCount3;
        static const IdString TexMatrixCount4;
        static const IdString TexMatrixCount5;
        static const IdString TexMatrixCount6;
        static const IdString TexMatrixCount7;

        static const IdString DiffuseMap;

        static const IdString Diffuse;
        static const IdString DiffuseMapCount0;
        static const IdString DiffuseMapCount1;
        static const IdString DiffuseMapCount2;
        static const IdString DiffuseMapCount3;
        static const IdString DiffuseMapCount4;
        static const IdString DiffuseMapCount5;
        static const IdString DiffuseMapCount6;
        static const IdString DiffuseMapCount7;
        static const IdString DiffuseMapCount8;
        static const IdString DiffuseMapCount9;
        static const IdString DiffuseMapCount10;
        static const IdString DiffuseMapCount11;
        static const IdString DiffuseMapCount12;
        static const IdString DiffuseMapCount13;
        static const IdString DiffuseMapCount14;
        static const IdString DiffuseMapCount15;

        static const IdString BlendModeIdx0;
        static const IdString BlendModeIdx1;
        static const IdString BlendModeIdx2;
        static const IdString BlendModeIdx3;
        static const IdString BlendModeIdx4;
        static const IdString BlendModeIdx5;
        static const IdString BlendModeIdx6;
        static const IdString BlendModeIdx7;
        static const IdString BlendModeIdx8;
        static const IdString BlendModeIdx9;
        static const IdString BlendModeIdx10;
        static const IdString BlendModeIdx11;
        static const IdString BlendModeIdx12;
        static const IdString BlendModeIdx13;
        static const IdString BlendModeIdx14;
        static const IdString BlendModeIdx15;

        static const IdString UvAtlas;
        static const IdString UvAtlas0;
        static const IdString UvAtlas1;
        static const IdString UvAtlas2;
        static const IdString UvAtlas3;
        static const IdString UvAtlas4;
        static const IdString UvAtlas5;
        static const IdString UvAtlas6;
        static const IdString UvAtlas7;
        static const IdString UvAtlas8;
        static const IdString UvAtlas9;
        static const IdString UvAtlas10;
        static const IdString UvAtlas11;
        static const IdString UvAtlas12;
        static const IdString UvAtlas13;
        static const IdString UvAtlas14;
        static const IdString UvAtlas15;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
