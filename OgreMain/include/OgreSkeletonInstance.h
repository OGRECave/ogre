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

#ifndef __SkeletonInstance_H__
#define __SkeletonInstance_H__

#include "OgrePrerequisites.h"
#include "OgreSkeleton.h"

namespace Ogre {

    /** A SkeletonInstance is a single instance of a Skeleton used by a world object.
    @remarks
        The difference between a Skeleton and a SkeletonInstance is that the
        Skeleton is the 'master' version much like Mesh is a 'master' version of
        Entity. Many SkeletonInstance objects can be based on a single Skeleton, 
        and are copies of it when created. Any changes made to this are not
        reflected in the master copy. The exception is animations; these are
        shared on the Skeleton itself and may not be modified here.
    */
    class _OgreExport SkeletonInstance : public Skeleton
    {
    public:
        /** Constructor, don't call directly, this will be created automatically
        when you create an Entity based on a skeletally animated Mesh.
        */
        SkeletonInstance(const SkeletonPtr& masterCopy);
        ~SkeletonInstance();

        /** Gets the number of animations on this skeleton. */
        unsigned short getNumAnimations(void) const;

        /** Gets a single animation by index. */
        Animation* getAnimation(unsigned short index) const;
		/// Internal accessor for animations (returns null if animation does not exist)
		Animation* _getAnimationImpl(const String& name, 
			const LinkedSkeletonAnimationSource** linker = 0) const;

        /** Creates a new Animation object for animating this skeleton. 
        @remarks
            This method updates the reference skeleton, not just this instance!
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        Animation* createAnimation(const String& name, Real length);

        /** Returns the named Animation object. */
        Animation* getAnimation(const String& name, 
			const LinkedSkeletonAnimationSource** linker = 0) const;

        /** Removes an Animation from this skeleton. 
        @remarks
            This method updates the reference skeleton, not just this instance!
        */
        void removeAnimation(const String& name);


        /** Creates a TagPoint ready to be attached to a bone */
        TagPoint* createTagPointOnBone(Bone* bone, 
            const Quaternion &offsetOrientation = Quaternion::IDENTITY, 
            const Vector3 &offsetPosition = Vector3::ZERO);

        /** Frees a TagPoint that already attached to a bone */
        void freeTagPoint(TagPoint* tagPoint);

		/// @copydoc Skeleton::addLinkedSkeletonAnimationSource
		void addLinkedSkeletonAnimationSource(const String& skelName, 
			Real scale = 1.0f);
		/// @copydoc Skeleton::removeAllLinkedSkeletonAnimationSources
		void removeAllLinkedSkeletonAnimationSources(void);
		/// @copydoc Skeleton::getLinkedSkeletonAnimationSourceIterator
		LinkedSkeletonAnimSourceIterator 
			getLinkedSkeletonAnimationSourceIterator(void) const;

		/// @copydoc Skeleton::_initAnimationState
		void _initAnimationState(AnimationStateSet* animSet);

		/// @copydoc Skeleton::_refreshAnimationState
		void _refreshAnimationState(AnimationStateSet* animSet);

		/// @copydoc Resource::getName
		const String& getName(void) const;
		/// @copydoc Resource::getHandle
		ResourceHandle getHandle(void) const;
		/// @copydoc Resource::getGroup
		const String& getGroup(void);

    protected:
        /// Pointer back to master Skeleton
        SkeletonPtr mSkeleton;

        typedef std::list<TagPoint*> TagPointList;

        /** Active tag point list.
        @remarks
            This is a linked list of pointers to active tag points
        @par
            This allows very fast insertions and deletions from anywhere in the list to activate / deactivate
            tag points (required for weapon / equip systems etc) as well as reuse of TagPoint instances
            without construction & destruction which avoids memory thrashing.
        */
        TagPointList mActiveTagPoints;

        /** Free tag point list.
        @remarks
            This contains a list of the tag points free for use as new instances
            as required by the set. When a TagPoint instance is deactivated, there will be a reference on this
            list. As they get used this list reduces, as they get released back to to the set they get added
            back to the list.
        */
        TagPointList mFreeTagPoints;

        /// TagPoint automatic handles
        unsigned short mNextTagPointAutoHandle;

        void cloneBoneAndChildren(Bone* source, Bone* parent);
        /** Overridden from Skeleton
        */
        void loadImpl(void);
        /** Overridden from Skeleton
        */
        void unloadImpl(void);

    };

}


#endif

