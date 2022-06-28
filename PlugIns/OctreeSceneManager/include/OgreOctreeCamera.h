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
/***************************************************************************
octreecamera.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

***************************************************************************/

#ifndef OCTREECAMERA_H
#define OCTREECAMERA_H

#include "OgreCamera.h"
#include "OgreOctreePrerequisites.h"

/**
*@author Jon Anderson
*/

namespace Ogre
{
/** \addtogroup Plugins Plugins
*  @{
*/
/** \addtogroup Octree OctreeSceneManager
* Octree datastructure for managing scene nodes.
*  @{
*/
/** Specialized viewpoint from which an Octree can be rendered.

This class contains several specializations of the Ogre::Camera class. It
implements the getRenderOperation method in order to return displayable geometry
for debugging purposes. It also implements a visibility function that is more granular
than the default.
*/

class _OgreOctreePluginExport OctreeCamera : public Camera
{
public:

    /** Visibility types */
    enum Visibility
    {
        NONE,
        PARTIAL,
        FULL
    };

    /* Standard constructor */
    OctreeCamera( const String& name, SceneManager* sm );
    /* Standard destructor */
    ~OctreeCamera();

    /** Returns the visibility of the box
    */
    OctreeCamera::Visibility getVisibility( const AxisAlignedBox &bound );

};
/** @} */
/** @} */
}

#endif
