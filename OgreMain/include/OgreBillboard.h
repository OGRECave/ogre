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

#ifndef __Billboard_H__
#define __Billboard_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreCommon.h"
#include "OgreHeaderPrefix.h"
#include "OgreMath.h"
#include "OgreVector.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** A billboard is a primitive which always faces the camera in every frame.
        @remarks
            Billboards can be used for special effects or some other trickery which requires the
            triangles to always facing the camera no matter where it is. Ogre groups billboards into
            sets for efficiency, so you should never create a billboard on it's own (it's ok to have a
            set of one if you need it).
        @par
            Billboards have their geometry generated every frame depending on where the camera is. It is most
            beneficial for all billboards in a set to be identically sized since Ogre can take advantage of this and
            save some calculations - useful when you have sets of hundreds of billboards as is possible with special
            effects. You can deviate from this if you wish (example: a smoke effect would probably have smoke puffs
            expanding as they rise, so each billboard will legitimately have it's own size) but be aware the extra
            overhead this brings and try to avoid it if you can.
        @par
            Billboards are just the mechanism for rendering a range of effects such as particles. It is other classes
            which use billboards to create their individual effects, so the methods here are quite generic.
        @see
            BillboardSet
    */

    class _OgreExport Billboard : public FXAlloc
    {
        friend class BillboardSet;
        friend class BillboardParticleRenderer;
    protected:
        bool mOwnDimensions;
        bool mUseTexcoordRect;
        uint16 mTexcoordIndex;      /// Index into the BillboardSet array of texture coordinates
        FloatRect mTexcoordRect;    /// Individual texture coordinates
        Real mWidth;
        Real mHeight;
    public:
        // Note the intentional public access to main internal variables used at runtime
        // Forcing access via get/set would be too costly for 000's of billboards
        Vector3 mPosition;
        /// Normalised direction vector
        Vector3 mDirection;
        ColourValue mColour;
        Radian mRotation;
        BillboardSet* mParentSet;

        /** Default constructor.
        */
        Billboard();

        /** Default destructor.
        */
        ~Billboard();

        /** Normal constructor as called by BillboardSet.
        */
        Billboard(const Vector3& position, BillboardSet* owner, const ColourValue& colour = ColourValue::White);

        /** Get the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
        */
        const Radian& getRotation(void) const { return mRotation; }

        /** Set the rotation of the billboard.
            @remarks
                This rotation is relative to the center of the billboard.
        */
        void setRotation(const Radian& rotation) { mRotation = rotation; }

        /** Set the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        void setPosition(const Vector3& position) { mPosition = position; }

        /// @overload
        void setPosition(Real x, Real y, Real z) { setPosition({x, y, z}); }

        /** Get the position of the billboard.
            @remarks
                This position is relative to a point on the quad which is the billboard. Depending on the BillboardSet,
                this may be the center of the quad, the top-left etc. See BillboardSet::setBillboardOrigin for more info.
        */
        const Vector3& getPosition(void) const { return mPosition; }

        /** Sets the width and height for this billboard.
            @remarks
                Note that it is most efficient for every billboard in a BillboardSet to have the same dimensions. If you
                choose to alter the dimensions of an individual billboard the set will be less efficient. Do not call
                this method unless you really need to have different billboard dimensions within the same set. Otherwise
                just call the BillboardSet::setDefaultDimensions method instead.
        */
        void setDimensions(Real width, Real height);

        /** Resets this Billboard to use the parent BillboardSet's dimensions instead of it's own. */
        void resetDimensions(void) { mOwnDimensions = false; }
        /** Sets the colour of this billboard.
            @remarks
                Billboards can be tinted based on a base colour. This allows variations in colour irrespective of the
                base colour of the material allowing more varied billboards. The default colour is white.
                The tinting is effected using vertex colours.
        */
        void setColour(const ColourValue& colour) { mColour = colour; }

        /** Gets the colour of this billboard.
        */
        const ColourValue& getColour(void) const { return mColour; }

        /** Returns true if this billboard deviates from the BillboardSet's default dimensions (i.e. if the
            Billboard::setDimensions method has been called for this instance).
            @see
                Billboard::setDimensions
        */
        bool hasOwnDimensions(void) const { return mOwnDimensions; }

        /** Retrieves the billboard's personal width, if hasOwnDimensions is true. */
        Real getOwnWidth(void) const { return mWidth; }

        /** Retrieves the billboard's personal height, if hasOwnDimensions is true. */
        Real getOwnHeight(void) const { return mHeight; }

        /** Internal method for notifying the billboard of it's owner.
        */
        void _notifyOwner(BillboardSet* owner) { mParentSet = owner; }

        /** Returns true if this billboard use individual texture coordinate rect (i.e. if the 
            Billboard::setTexcoordRect method has been called for this instance), or returns
            false if use texture coordinates defined in the parent BillboardSet's texture
            coordinates array (i.e. if the Billboard::setTexcoordIndex method has been called
            for this instance).
            @see
                Billboard::setTexcoordIndex()
                Billboard::setTexcoordRect()
        */
        bool isUseTexcoordRect(void) const { return mUseTexcoordRect; }

        /** setTexcoordIndex() sets which texture coordinate rect this billboard will use 
            when rendering. The parent billboard set may contain more than one, in which 
            case a billboard can be textured with different pieces of a larger texture 
            sheet very efficiently.
          @see
            BillboardSet::setTextureCoords()
          */
        void setTexcoordIndex(uint16 texcoordIndex);

        /** getTexcoordIndex() returns the previous value set by setTexcoordIndex(). 
            The default value is 0, which is always a valid texture coordinate set.
            @remarks
                This value is useful only when isUseTexcoordRect return false.
          */
        uint16 getTexcoordIndex(void) const { return mTexcoordIndex; }

        /** sets the individual texture coordinate rect of this billboard will use when rendering.
            The parent billboard set may contain more than one, in
            which case a billboard can be textured with different pieces of a larger texture
            sheet very efficiently.
        */
        void setTexcoordRect(const FloatRect& texcoordRect);

        /// @overload
        void setTexcoordRect(float u0, float v0, float u1, float v1) { setTexcoordRect({u0, v0, u1, v1}); }

        /** getTexcoordRect() returns the previous value set by setTexcoordRect(). 
            @remarks
                This value is useful only when isUseTexcoordRect returns true.
        */
        const FloatRect& getTexcoordRect(void) const { return mTexcoordRect; }
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
