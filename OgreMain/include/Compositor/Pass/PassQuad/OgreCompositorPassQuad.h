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

#ifndef __CompositorPassQuad_H__
#define __CompositorPassQuad_H__

#include "OgreHeaderPrefix.h"

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreMaterial.h"

namespace Ogre
{
    namespace v1
    {
        class Rectangle2D;
    }
    class CompositorPassQuadDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** Implementation of CompositorPass
        This implementation will render a fullscreen triangle/quad to the RenderTarget using
        the parameters from definition (rectangle area, whether triangle or quad pass,
        whether to include frustum corners, etc)
    @par
        A fullscreen quad are two triangles that form a rectangle, which cover the entire screen.
        This is however an inefficient way of harnessing the GPU's power. Modern GPUs iterate
        a whole bounding rect that encloses each triangle (it's highly more parallelizable & scalable
        than the scanline algorithm used by older Voodoo cards in the 90's). That means we're wasting
        50% of GPU reasources in pixels that aren't drawn.
    @par
        A much faster approach made famous by Emil Persson (see max area triangulation
        http://www.humus.name/index.php?page=News&ID=228) is to render a single oversized triangle
        that covers the entire screen, which has it's UV scaled up; so that when interpolated at
        screen position, it appears it goes from the range [0; 1) like a regular quad pass.
        The difference is that pixels that go past the viewport are much better clipped/earlied-out
        by the GPU than those that fall inside the viewport but outside the triangle (like the
        rectangle approach)
    @par
        This faster technique only works for passes that cover the entire screen. When the viewport
        is not 0 0 1 1; the CompositorPassQuad automatically switches to a fullscreen quad.
    @par
        <b>IMPORTANT:</b> The texel-to-pixel adjustment offset is included in the world matrix passed
        to the vertex shader. This differs from Ogre 1.x which tried to embed the offset into the
        vertices.
    @par
        <b>IMPORTANT:</b> When the viewport is not 0 0 1 1; the scale & position factors are passed
        through the world matrix. This behavior differs from Ogre 1.x
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport CompositorPassQuad : public CompositorPass
    {
        CompositorPassQuadDef const *mDefinition;
    protected:
        v1::Rectangle2D *mFsRect;
        HlmsDatablock   *mDatablock;
        MaterialPtr     mMaterial;
        Pass            *mPass;
        Camera          *mCamera;

        Real        mHorizonalTexelOffset;
        Real        mVerticalTexelOffset;

    public:
        CompositorPassQuad( const CompositorPassQuadDef *definition, Camera *defaultCamera,
                            CompositorNode *parentNode, const CompositorChannel &target,
                            Real horizonalTexelOffset, Real verticalTexelOffset );

        virtual void execute( const Camera *lodCamera );

        /// Don't make this const (useful for compile-time multithreading errors)
        /// Pointer can be null if using HLMS
        Pass* getPass(void)                                     { return mPass; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
