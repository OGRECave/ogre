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

#ifndef __SkeletonInstance_H__
#define __SkeletonInstance_H__

#include "OgrePrerequisites.h"
#include "OgreSkeleton.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** A SkeletonInstance is a single instance of a Skeleton used by a world object.

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
        unsigned short getNumAnimations(void) const override;

        /** Gets a single animation by index. */
        Animation* getAnimation(unsigned short index) const override;
        /// Internal accessor for animations (returns null if animation does not exist)
        Animation* _getAnimationImpl(const String& name, 
            const LinkedSkeletonAnimationSource** linker = 0) const override;

        /** Creates a new Animation object for animating this skeleton. 

            This method updates the reference skeleton, not just this instance!
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        Animation* createAnimation(const String& name, Real length) override;

        /** Returns the named Animation object. */
        Animation* getAnimation(const String& name, 
            const LinkedSkeletonAnimationSource** linker = 0) const override;

        /** Removes an Animation from this skeleton. 

            This method updates the reference skeleton, not just this instance!
        */
        void removeAnimation(const String& name) override;


        /** Creates a TagPoint ready to be attached to a bone */
        TagPoint* createTagPointOnBone(Bone* bone, 
            const Quaternion &offsetOrientation = Quaternion::IDENTITY, 
            const Vector3 &offsetPosition = Vector3::ZERO);

        /** Frees a TagPoint that already attached to a bone */
        void freeTagPoint(TagPoint* tagPoint);

        /// @copydoc Skeleton::addLinkedSkeletonAnimationSource
        void addLinkedSkeletonAnimationSource(const String& skelName, 
            Real scale = 1.0f) override;
        /// @copydoc Skeleton::removeAllLinkedSkeletonAnimationSources
        void removeAllLinkedSkeletonAnimationSources(void) override;
        const LinkedSkeletonAnimSourceList&
                    getLinkedSkeletonAnimationSources() const override;
        OGRE_DEPRECATED LinkedSkeletonAnimSourceIterator
            getLinkedSkeletonAnimationSourceIterator(void) const override;

        /// @copydoc Skeleton::_initAnimationState
        void _initAnimationState(AnimationStateSet* animSet) override;

        /// @copydoc Skeleton::_refreshAnimationState
        void _refreshAnimationState(AnimationStateSet* animSet) override;

        /// @copydoc Resource::getName
        const String& getName(void) const;
        /// @copydoc Resource::getHandle
        ResourceHandle getHandle(void) const;
        /// @copydoc Resource::getGroup
        const String& getGroup(void) const;

    private:
        /// Pointer back to master Skeleton
        SkeletonPtr mSkeleton;

        typedef std::list<TagPoint*> TagPointList;

        /** Active tag point list.

            This is a linked list of pointers to active tag points
        @par
            This allows very fast insertions and deletions from anywhere in the list to activate / deactivate
            tag points (required for weapon / equip systems etc) as well as reuse of TagPoint instances
            without construction & destruction which avoids memory thrashing.
        */
        TagPointList mActiveTagPoints;

        /** Free tag point list.

            This contains a list of the tag points free for use as new instances
            as required by the set. When a TagPoint instance is deactivated, there will be a reference on this
            list. As they get used this list reduces, as they get released back to to the set they get added
            back to the list.
        */
        TagPointList mFreeTagPoints;

        void cloneBoneAndChildren(Bone* source, Bone* parent);
        void prepareImpl(void) override;
        void unprepareImpl(void) override;

    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

