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
OgrePCZSceneQuery.h  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21, 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#ifndef PCZSCENEQUERY_H
#define PCZSCENEQUERY_H

#include "OgreSceneManager.h"
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
    class PCZone;

    /** PCZ implementation of IntersectionSceneQuery. */
    class _OgrePCZPluginExport PCZIntersectionSceneQuery :  public DefaultIntersectionSceneQuery
    {
    public:
        PCZIntersectionSceneQuery(SceneManager* creator);
        ~PCZIntersectionSceneQuery();

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);
    };
    /** PCZ implementation of AxisAlignedBoxSceneQuery. */
    class _OgrePCZPluginExport PCZAxisAlignedBoxSceneQuery : public DefaultAxisAlignedBoxSceneQuery
    {
    public:
        PCZAxisAlignedBoxSceneQuery(SceneManager* creator);
        ~PCZAxisAlignedBoxSceneQuery();

        /** See RaySceneQuery. */
        void execute(SceneQueryListener* listener);

        /** set the zone to start the scene query */
        void setStartZone(PCZone * startZone) {mStartZone = startZone;}
        /** set node to exclude from query */
        void setExcludeNode(SceneNode * excludeNode) {mExcludeNode = excludeNode;}
    protected:
        PCZone * mStartZone;
        SceneNode * mExcludeNode;
    };
    /** PCZ implementation of RaySceneQuery. */
    class _OgrePCZPluginExport PCZRaySceneQuery : public DefaultRaySceneQuery
    {
    public:
        PCZRaySceneQuery(SceneManager* creator);
        ~PCZRaySceneQuery();

        /** See RayScenQuery. */
        void execute(RaySceneQueryListener* listener);

        /** set the zone to start the scene query */
        void setStartZone(PCZone * startZone) {mStartZone = startZone;}
        /** set node to exclude from query */
        void setExcludeNode(SceneNode * excludeNode) {mExcludeNode = excludeNode;}
    protected:
        PCZone * mStartZone;
        SceneNode * mExcludeNode;
    };
    /** PCZ implementation of SphereSceneQuery. */
    class _OgrePCZPluginExport PCZSphereSceneQuery : public DefaultSphereSceneQuery
    {
    public:
        PCZSphereSceneQuery(SceneManager* creator);
        ~PCZSphereSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);

        /** set the zone to start the scene query */
        void setStartZone(PCZone * startZone) {mStartZone = startZone;}
        /** set node to exclude from query */
        void setExcludeNode(SceneNode * excludeNode) {mExcludeNode = excludeNode;}
    protected:
        PCZone * mStartZone;
        SceneNode * mExcludeNode;
    };
    /** PCZ implementation of PlaneBoundedVolumeListSceneQuery. */
    class _OgrePCZPluginExport PCZPlaneBoundedVolumeListSceneQuery : public DefaultPlaneBoundedVolumeListSceneQuery
    {
    public:
        PCZPlaneBoundedVolumeListSceneQuery(SceneManager* creator);
        ~PCZPlaneBoundedVolumeListSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);

        /** set the zone to start the scene query */
        void setStartZone(PCZone * startZone) {mStartZone = startZone;}
        /** set node to exclude from query */
        void setExcludeNode(SceneNode * excludeNode) {mExcludeNode = excludeNode;}
    protected:
        PCZone * mStartZone;
        SceneNode * mExcludeNode;
    };


}

#endif


