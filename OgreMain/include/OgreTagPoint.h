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
#ifndef __TagPoint_H_
#define __TagPoint_H_

#include "OgrePrerequisites.h"

#include "OgreOldBone.h"
#include "OgreMatrix4.h"

namespace Ogre  {
namespace v1 {

    
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Animation
    *  @{
    */
    /** A tagged point on a skeleton, which can be used to attach entities to on specific
        other entities.
    @remarks
        A Skeleton, like a Mesh, is shared between Entity objects and simply updated as required
        when it comes to rendering. However there are times when you want to attach another object
        to an animated entity, and make sure that attachment follows the parent entity's animation
        (for example, a character holding a gun in his / her hand). This class simply identifies
        attachment points on a skeleton which can be used to attach child objects. 
    @par
        The child objects themselves are not physically attached to this class; as it's name suggests
        this class just 'tags' the area. The actual child objects are attached to the Entity using the
        skeleton which has this tag point. Use the Entity::attachMovableObjectToBone method to attach
        the objects, which creates a new TagPoint on demand.
    */
    class _OgreExport TagPoint : public OldBone
    {

    public:
        TagPoint(unsigned short handle, Skeleton* creator);
        virtual ~TagPoint();

        Entity *getParentEntity(void) const;
        MovableObject* getChildObject(void) const;
        
        void setParentEntity(Entity *pEntity);
        void setChildObject(MovableObject *pObject);

        /** Tells the TagPoint whether it should inherit orientation from it's parent entity.
        @param inherit If true, this TagPoint's orientation will be affected by
            its parent entity's orientation. If false, it will not be affected.
        */
        void setInheritParentEntityOrientation(bool inherit);

        /** Returns true if this TagPoint is affected by orientation applied to the parent entity. 
        */
        bool getInheritParentEntityOrientation(void) const;

        /** Tells the TagPoint whether it should inherit scaling factors from it's parent entity.
        @param inherit If true, this TagPoint's scaling factors will be affected by
            its parent entity's scaling factors. If false, it will not be affected.
        */
        void setInheritParentEntityScale(bool inherit);

        /** Returns true if this TagPoint is affected by scaling factors applied to the parent entity. 
        */
        bool getInheritParentEntityScale(void) const;

        /** Gets the transform of parent entity. */
        const Matrix4& getParentEntityTransform(void) const;

        /** Gets the transform of this node just for the skeleton (not entity) */
        const Matrix4& _getFullLocalTransform(void) const;

        /** @copydoc Node::needUpdate */
        void needUpdate(bool forceParentUpdate = false);

        /** Overridden from Node in order to include parent Entity transform. */
        void updateFromParentImpl(void) const;
        /** @copydoc Renderable::getLights */
        const LightList& getLights(void) const;



    private:
        Entity *mParentEntity;
        MovableObject *mChildObject;
        mutable Matrix4 mFullLocalTransform;
        bool mInheritParentEntityOrientation;
        bool mInheritParentEntityScale;
    };

    /** @} */
    /** @} */
}
} //namespace


#endif//__TagPoint_H_
