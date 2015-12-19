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
#ifndef _OgreTagPoint_H_
#define _OgreTagPoint_H_

#include "OgreSceneNode.h"

#include "OgreHeaderPrefix.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */

    /** TagPoints are like SceneNodes, that can be children of a Bone.

        Q: Can I use a SceneNode as child of a Bone?
        A: No.

        Q: Can I use a TagPoint as child of a Bone?
        A: Yes. That's the idea.

        Q: Can I use a SceneNode as child of a TagPoint?
        A: Yes.

        Q: Can I use a TagPoint as child of a TagPoint?
        A: Yes.

        Q: Can I use a TagPoint as child of a SceneNode?
        A: Yes.

        Q: Is there a performance hit for using a TagPoint as a SceneNode?
        A: No, though there is higher memory consumption (and indirectly could
           decrease performance by affecting the cache or the bandwidth)

        Q: So... TagPoints are better than SceneNodes, because they can be used
           like SceneNodes, but also be used with bones?
        A: Yes.

        Q: Why not merge TagPoints into SceneNodes then, and remove TagPoints?
        A: Good question. Theoretically the TagPoint functionality does not
           belong to SceneNode (more dependencies, more clutter in the same
           cpp files).
           Also TagPoints require more memory; which can add up when you have
           tens of thousands (or +100k's) of nodes.
           Only pay for what you use.

        Q: Can I attach a TagPoint to Skeleton 'A' bone, and then attach an Item/Entity
           with Skeleton 'B' to this TagPoint?
        A: Yesish. It will work, but Skeleton B will see the changes from Skeleton A
           with one frame of latency (i.e. lag behind). If the animation is too fast
           or too different between frames, it may be noticeable.
           That's because we update Skeleton A, then B, then the TagPoints.
           To fix this we would have to update Skeleton 'A', then the TagPoints, then
           Skeleton 'B'; which is too cumbersome for such rare corner case.

        Q: What happens on circular dependencies? i.e. Skeleton gets attached to
           TagPoint, TagPoint gets attached to bone of said Skeleton?
        A: Undefined. Probably very wonky behavior.

        Q: What happens if I call setStatic( true )?
        A: If the TagPoint is ultimately the child of a Bone, then nothing happens.
           If the TagPoint is ultimately the child of the Root SceneNode, then it
           works as usual.

        Q: What happens if I called setStatic( true ) on a TagPoint that belonged
           to the Root SceneNode, and now I've detached, and attached it to a bone?
        A: The information about being static is completely lost (i.e. it's like
           calling setStatic( false ) prior to attaching to the bone)
    */
    class _OgreExport TagPoint : public SceneNode
    {
    protected:
        /// Pointer to parent node
        Bone    *mParentBone;

        /// @copydoc Node::updateFromParentImpl.
        void updateFromParentImpl(void);

    public:
        TagPoint( IdType id, SceneManager* creator, NodeMemoryManager *nodeMemoryManager,
                  SceneNode *parent );
        virtual ~TagPoint();

        /// Don't call directly. @see Bone::addTagPoint
        void _setParentBone( Bone *bone );
        void _unsetParentBone(void);

        /// Gets this Bones's parent (NULL if no parent).
        Bone* getParentBone(void) const                                 { return mParentBone; }

        Matrix3 _getDerivedOrientationMatrix(void) const;

        /// @See Node::updateAllTransforms.
        /// This version grabs the parent Bone of a TagPoint, derives the final transform
        /// in world space (supporting non-uniform scaling), and decomposes the matrix
        /// into derived position/quaternion/scale (the quaternion and scale aren't very
        /// useful *if* the skeleton is actually using non-uniform scaling though)
        static void updateAllTransformsBoneToTag( const size_t numNodes, Transform t );

        /// @See Node::updateAllTransforms.
        /// This version grabs the parent of a TagPoint, and derives the final transform
        /// of another TagPoint, respecting non-uniform scaling.
        static void updateAllTransformsTagOnTag( const size_t numNodes, Transform t );

        virtual TagPoint* createChildTagPoint( const Vector3& vPos = Vector3::ZERO,
                                               const Quaternion& qRot = Quaternion::IDENTITY );
    };
    /** @} */
    /** @} */


}// namespace

#include "OgreHeaderSuffix.h"

#endif
