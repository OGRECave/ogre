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
#ifndef _Rectangle2D_H__
#define _Rectangle2D_H__

#include "OgrePrerequisites.h"

#include "OgreSimpleRenderable.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Allows the rendering of a simple 2D rectangle
    This class renders a simple 2D rectangle; this rectangle has no depth and
    therefore is best used with specific render queue and depth settings,
    like RENDER_QUEUE_BACKGROUND and 'depth_write off' for backdrops, and 
    RENDER_QUEUE_OVERLAY and 'depth_check off' for fullscreen quads.
    */
    class _OgreExport Rectangle2D : public SimpleRenderable
    {
    protected:
        /** Override this method to prevent parent transforms (rotation,translation,scale)
        */
        void getWorldTransforms( Matrix4* xform ) const;

        void _initRectangle2D(bool includeTextureCoords, Ogre::HardwareBuffer::Usage vBufUsage);

    public:

        Rectangle2D(bool includeTextureCoordinates = false, HardwareBuffer::Usage vBufUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        Rectangle2D(const String& name, bool includeTextureCoordinates = false, HardwareBuffer::Usage vBufUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        ~Rectangle2D();

        /** Sets the corners of the rectangle, in relative coordinates.
        @param
        left Left position in screen relative coordinates, -1 = left edge, 1.0 = right edge
        @param top Top position in screen relative coordinates, 1 = top edge, -1 = bottom edge
        @param right Right position in screen relative coordinates
        @param bottom Bottom position in screen relative coordinates
        @param updateAABB Tells if you want to recalculate the AABB according to 
        the new corners. If false, the axis aligned bounding box will remain identical.
        */
        void setCorners(Real left, Real top, Real right, Real bottom, bool updateAABB = true);

        /** Sets the normals of the rectangle
        */
        void setNormals(const Ogre::Vector3 &topLeft, const Ogre::Vector3 &bottomLeft, const Ogre::Vector3 &topRight, const Ogre::Vector3 &bottomRight);

        /** Sets the UVs of the rectangle
        @remarks
        Doesn't do anything if the rectangle wasn't built with texture coordinates
        */
        void setUVs( const Ogre::Vector2 &topLeft, const Ogre::Vector2 &bottomLeft,
                     const Ogre::Vector2 &topRight, const Ogre::Vector2 &bottomRight);

        void setDefaultUVs();

        Real getSquaredViewDepth(const Camera* cam) const
        { (void)cam; return 0; }

        Real getBoundingRadius(void) const { return 0; }

    };
    /** @} */
    /** @} */

}// namespace

#endif


