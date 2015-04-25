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

    /** HLMS stands for "High Level Material System". */
    class _OgreExport HlmsListener
    {
    public:
        /// Listeners should return the extra bytes they wish to allocate for storing additional
        /// data in the pass buffer.
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
