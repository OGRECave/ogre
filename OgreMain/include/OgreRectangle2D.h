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

#include "OgreRenderOperation.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"

namespace Ogre {
namespace v1 {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Allows the rendering of a simple 2D rectangle
    This class renders a simple 2D rectangle; this rectangle has no depth and
    therefore is best used with 'depth_write off' materials.
    @par
    Beginning Ogre 2.0, it supports building a full screen triangle instead
    of rectangle. Position & UVs are in the first source. Normals are in the second one
    */
    class _OgreExport Rectangle2D : public Renderable, public MovableObject
    {
    protected:
        Vector3     mPosition;
        Quaternion  mOrientation;
        Vector3     mScale;

        bool        mQuad;

        RenderOperation mRenderOp;

        void initRectangle2D(void);

    public:
        Rectangle2D( bool bQuad, IdType id, ObjectMemoryManager *objectMemoryManager,
                     SceneManager *manager );
        ~Rectangle2D();

        /** Sets the corners of the rectangle, in relative coordinates.
        @param
        left Left position in screen normalized coordinates, 0 = left edge, 1 = right edge
        @param top Top position in screen normalized coordinates, 0 = top edge, 1 = bottom edge
        @param width Width in screen normalized coordinates
        @param height Height in screen normalized coordinates
        */
        void setCorners( Real left, Real top, Real width, Real height );

        /** Sets the normals of the rectangle
        @remarks
            Be careful the normals can be bilinearly interpolated correctly, otherwise the
            results between Fullscreen Triangles & Fullscreen Quads will be different
        */
        void setNormals( const Ogre::Vector3 &topLeft, const Ogre::Vector3 &bottomLeft,
                        const Ogre::Vector3 &topRight, const Ogre::Vector3 &bottomRight );

        Real getSquaredViewDepth(const Camera* cam) const   { (void)cam; return 0; }

        virtual void getWorldTransforms( Matrix4* xform ) const;
        virtual void getRenderOperation( RenderOperation& op, bool casterPass );
        virtual const LightList& getLights(void) const;

        /** Returns the type name of this object. */
        virtual const String& getMovableType(void) const;
    };

    /** Factory object for creating Entity instances */
    class _OgreExport Rectangle2DFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        Rectangle2DFactory() {}
        ~Rectangle2DFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);

    };

    /** @} */
    /** @} */

}
}// namespace

#endif
