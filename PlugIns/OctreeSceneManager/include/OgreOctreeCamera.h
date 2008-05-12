/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
/***************************************************************************
octreecamera.h  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team
***************************************************************************/

#ifndef OCTREECAMERA_H
#define OCTREECAMERA_H

#include <OgreCamera.h>
#include <OgreHardwareBufferManager.h>
#include <OgreSimpleRenderable.h>
#include "OgreTerrainPrerequisites.h"

/**
*@author Jon Anderson
*/

namespace Ogre
{

class Octree;


/** Specialized viewpoint from which an Octree can be rendered.
@remarks
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

    /** Returns the visiblity of the box
    */
    OctreeCamera::Visibility getVisibility( const AxisAlignedBox &bound );

};

}

#endif
