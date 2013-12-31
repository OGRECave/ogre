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

#ifndef __VertexBoneAssignment_H__
#define __VertexBoneAssignment_H__

#include "OgrePrerequisites.h"


namespace Ogre 
{
    
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Animation
	*  @{
	*/
	/** Records the assignment of a single vertex to a single bone with the corresponding weight.
    @remarks
        This simple struct simply holds a vertex index, bone index and weight representing the
        assignment of a vertex to a bone for skeletal animation. There may be many of these
        per vertex if blended vertex assignments are allowed.
    */
    typedef struct VertexBoneAssignment_s
    {
        unsigned int vertexIndex;
        unsigned short boneIndex;
        Real weight;

    } VertexBoneAssignment;

	/** @} */
	/** @} */

}

#endif
