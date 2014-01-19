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
OgreOctreeSceneQuery.h  -  description
-------------------
begin                : Tues July 20, 2004
copyright            : (C) 2004 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

#ifndef OCTREESCENEQUERY_H
#define OCTREESCENEQUERY_H

#include "OgreOctreePrerequisites.h"
#include "OgreSceneManager.h"


namespace Ogre
{
/** Octree implementation of IntersectionSceneQuery. */
class _OgreOctreePluginExport OctreeIntersectionSceneQuery :  public DefaultIntersectionSceneQuery
{
public:
    OctreeIntersectionSceneQuery(SceneManager* creator);
    ~OctreeIntersectionSceneQuery();

    /** See IntersectionSceneQuery. */
    void execute(IntersectionSceneQueryListener* listener);
};

/** Octree implementation of RaySceneQuery. */
class _OgreOctreePluginExport OctreeRaySceneQuery : public DefaultRaySceneQuery
{
public:
    OctreeRaySceneQuery(SceneManager* creator);
    ~OctreeRaySceneQuery();

    /** See RayScenQuery. */
    void execute(RaySceneQueryListener* listener);
};
/** Octree implementation of SphereSceneQuery. */
class _OgreOctreePluginExport OctreeSphereSceneQuery : public DefaultSphereSceneQuery
{
public:
    OctreeSphereSceneQuery(SceneManager* creator);
    ~OctreeSphereSceneQuery();

    /** See SceneQuery. */
    void execute(SceneQueryListener* listener);
};
/** Octree implementation of PlaneBoundedVolumeListSceneQuery. */
class _OgreOctreePluginExport OctreePlaneBoundedVolumeListSceneQuery : public DefaultPlaneBoundedVolumeListSceneQuery
{
public:
    OctreePlaneBoundedVolumeListSceneQuery(SceneManager* creator);
    ~OctreePlaneBoundedVolumeListSceneQuery();

    /** See SceneQuery. */
    void execute(SceneQueryListener* listener);
};
/** Octree implementation of AxisAlignedBoxSceneQuery. */
class _OgreOctreePluginExport OctreeAxisAlignedBoxSceneQuery : public DefaultAxisAlignedBoxSceneQuery
{
public:
    OctreeAxisAlignedBoxSceneQuery(SceneManager* creator);
    ~OctreeAxisAlignedBoxSceneQuery();

    /** See RaySceneQuery. */
    void execute(SceneQueryListener* listener);
};


}

#endif


