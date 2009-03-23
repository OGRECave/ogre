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
#ifndef __Distance_Lod_Strategy_H__
#define __Distance_Lod_Strategy_H__

#include "OgrePrerequisites.h"

#include "OgreLodStrategy.h"
#include "OgreSingleton.h"
#include "OgreNode.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/

    /** Level of detail strategy based on distance from camera. */
    class _OgreExport DistanceLodStrategy : public LodStrategy, public Singleton<DistanceLodStrategy>
    {
    protected:
        /// @copydoc LodStrategy::getValueImpl
        virtual Real getValueImpl(const MovableObject *movableObject, const Camera *camera) const;

    public:
        /** Default constructor. */
        DistanceLodStrategy();

        /// @copydoc LodStrategy::getBaseValue
        virtual Real getBaseValue() const;

        /// @copydoc LodStrategy::transformBias
        virtual Real transformBias(Real factor) const;

        /// @copydoc LodStrategy::transformUserValue
        virtual Real transformUserValue(Real userValue) const;

        /// @copydoc LodStrategy::getIndex
        virtual ushort getIndex(Real value, const Mesh::MeshLodUsageList& meshLodUsageList) const;

        /// @copydoc LodStrategy::getIndex
        virtual ushort getIndex(Real value, const Material::LodValueList& materialLodValueList) const;

        /// @copydoc LodStrategy::sort
        virtual void sort(Mesh::MeshLodUsageList& meshLodUsageList) const;

        /// @copydoc LodStrategy::isSorted
        virtual bool isSorted(const Mesh::LodValueList& values) const;

        /** Sets the reference view upon which the distances were based.
        @note
            This automatically enables use of the reference view.
        @note
            There is no corresponding get method for these values as
            they are not saved, but used to compute a reference value.
        */
        void setReferenceView(Real viewportWidth, Real viewportHeight, Radian fovY);

        /** Enables to disables use of the reference view.
        @note Do not enable use of the reference view before setting it.
        */
        void setReferenceViewEnabled(bool value);

        /** Determine if use of the reference view is enabled */
        bool getReferenceViewEnabled() const;

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static DistanceLodStrategy& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static DistanceLodStrategy* getSingletonPtr(void);

    private:
        bool mReferenceViewEnabled;
        Real mReferenceViewValue;

    };
	/** @} */
	/** @} */

} // namespace

#endif
