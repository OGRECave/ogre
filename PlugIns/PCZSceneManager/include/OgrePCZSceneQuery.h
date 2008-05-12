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
OgrePCZSceneQuery.h  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21, 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZSCENEQUERY_H
#define PCZSCENEQUERY_H

#include <OgreSceneManager.h>
#include "OgrePCZPrerequisites.h"
#include "OgrePCZone.h"


namespace Ogre
{
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


