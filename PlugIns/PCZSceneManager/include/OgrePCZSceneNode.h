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
PCZSceneNode.h  -  Node Zone Info header file.
The PCZSceneNode is an extension used to store zone information and provide
additional functionality for a given Ogre::SceneNode.  A PCZSceneNode contains
a pointer to the home zone for the node and a list of all zones being visited by
the node.  The PCZSceneManager contains a STD::MAP of PCZSceneNodes which are
keyed by the name of each node (each PCZSceneNode has an identical name to the 
scene node which it is associated with).  This allows quick lookup of
a given scenenode's PCZSceneNode by the scene manager.
-----------------------------------------------------------------------------
begin                : Sat Mar 24 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :
-----------------------------------------------------------------------------
*/

#ifndef PCZ_SCENE_NODE_H
#define PCZ_SCENE_NODE_H

#include <OgreSceneNode.h>
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
	// forward declarations
	class PCZone;
	class ZoneData;
	class PCZCamera;
    typedef map<String, PCZone*>::type ZoneMap;
	typedef map<String, ZoneData*>::type ZoneDataMap;

	class _OgrePCZPluginExport PCZSceneNode : public SceneNode
	{
	public:
		/** Standard constructor */
		PCZSceneNode( SceneManager* creator );
		/** Standard constructor */
		PCZSceneNode( SceneManager* creator, const String& name );
		/** Standard destructor */
		~PCZSceneNode();
		void _update(bool updateChildren, bool parentHasChanged);
		void updateFromParentImpl() const;

        /** Creates an unnamed new SceneNode as a child of this node.
        @param
            translate Initial translation offset of child relative to parent
        @param
            rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(
            const Vector3& translate = Vector3::ZERO, 
            const Quaternion& rotate = Quaternion::IDENTITY );

        /** Creates a new named SceneNode as a child of this node.
        @remarks
            This creates a child node with a given name, which allows you to look the node up from 
            the parent which holds this collection of nodes.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        virtual SceneNode* createChildSceneNode(const String& name, const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);


		PCZone* 	getHomeZone(void);
		void		setHomeZone(PCZone * zone);
		void		anchorToHomeZone(PCZone * zone);
		bool		isAnchored(void) {return mAnchored;}
		void		allowToVisit(bool yesno) {mAllowedToVisit = yesno;}
		bool		allowedToVisit(void) {return mAllowedToVisit;}
		void		addZoneToVisitingZonesMap(PCZone * zone);
		void		clearVisitingZonesMap(void);
		void		clearNodeFromVisitedZones( void );
		void		removeReferencesToZone(PCZone * zone);
		bool		isVisitingZone(PCZone * zone);
	    void		_addToRenderQueue( Camera* cam, 
                                       RenderQueue *queue, 
                                       bool onlyShadowCasters, 
                                       VisibleObjectsBoundsInfo* visibleBounds );
		void		savePrevPosition(void);
		Vector3&	getPrevPosition(void) {return mPrevPosition;}
		unsigned long		getLastVisibleFrame(void) {return mLastVisibleFrame;}
		void		setLastVisibleFrame(unsigned long newLVF) {mLastVisibleFrame = newLVF;}
		void		setLastVisibleFromCamera(PCZCamera * camera) {mLastVisibleFromCamera = camera;}
		PCZCamera*	getLastVisibleFromCamera() {return mLastVisibleFromCamera;}
		void		setZoneData(PCZone * zone, ZoneData * zoneData);
		ZoneData*	getZoneData(PCZone * zone);
		void		updateZoneData(void);
		void		enable(bool yesno) {mEnabled = yesno;}
		bool		isEnabled(void) {return mEnabled;}
		bool		isMoved(void) {return mMoved;}
		void		setMoved(bool value) {mMoved = value;}
	protected:
		mutable Vector3	mNewPosition; 
		PCZone *		mHomeZone;
		bool			mAnchored;
		bool			mAllowedToVisit;
		ZoneMap			mVisitingZones;
		mutable Vector3	mPrevPosition;
		unsigned long	mLastVisibleFrame;
		PCZCamera*		mLastVisibleFromCamera;
		ZoneDataMap		mZoneData;
		bool			mEnabled;
		mutable bool	mMoved;
	};
}

#endif // PCZ_SCENE_NODE_H
