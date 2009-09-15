/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef _WireBoundingBox_H__
#define _WireBoundingBox_H__

#include "OgrePrerequisites.h"

#include "OgreSimpleRenderable.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Scene
	*  @{
	*/
	/** Allows the rendering of a wireframe bounding box.
        @remarks
            This class builds a wireframe renderable from a given aabb. A pointer to this class can be
			added to a render queue to display the bounding box of an object.
    */
	class _OgreExport WireBoundingBox : public SimpleRenderable
	{
    protected:
        /** Override this method to prevent parent transforms (rotation,translation,scale)
        */
		void getWorldTransforms( Matrix4* xform ) const;
		
        /** Builds the wireframe line list.
        */
		void setupBoundingBoxVertices(const AxisAlignedBox& aab);

        Real mRadius;

	public:
			
		WireBoundingBox();
		~WireBoundingBox();

        /** Builds the wireframe line list.
            @param
                aabb bounding box to build a wireframe from.
        */
		void setupBoundingBox(const AxisAlignedBox& aabb);

		Real getSquaredViewDepth(const Camera* cam) const;

        Real getBoundingRadius(void) const { return mRadius; }

	};
	/** @} */
	/** @} */

}// namespace

#endif


