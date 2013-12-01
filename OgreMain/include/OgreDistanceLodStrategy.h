/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

	class DistanceLodBoxStrategy;
	/// Backward compatible name for Distance_Box strategy.
	typedef DistanceLodBoxStrategy DistanceLodStrategy;

    /** Level of detail strategy based on distance from camera. This is an abstract base class for DistanceLodBoxStrategy and DistanceLodSphereStrategy.
        @remarks
            The purpose of the reference view is to ensure a consistent experience for all users. Monitors of different resolutions and aspect ratios will each have different results for the distance queries.
        @par
            It depends on gameplay testing. If all testers had 16:9 monitors and 110° FOV, then that's the value you should enter (to ensure as much as possible the experience stays consistent for all other users who don't have a 16:9 monitor and/or use a different FOV).
        @par
            If all your testers had 4:3 monitors, then enter a 4:3 resolution.
        @par
            If all your testers had varying resolutions or you just didn't care, then this feature is useless for you and should be disabled (default: disabled).
     */
    class _OgreExport DistanceLodStrategyBase : public LodStrategy
    {
    protected:
        /// @copydoc LodStrategy::getValueImpl
        virtual Real getValueImpl(const MovableObject *movableObject, const Camera *camera) const;

    public:
        /** Default constructor. */
        DistanceLodStrategyBase(const String& name);

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

        /** Get the squared depth from camera to the LOD object */
        virtual Real getSquaredDepth(const MovableObject *movableObject, const Ogre::Camera *camera) const = 0;

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
        bool isReferenceViewEnabled() const;

    private:
        bool mReferenceViewEnabled;
        Real mReferenceViewValue;

    };
	/** @} */
	/** @} */

    /** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/

    /** Level of detail strategy based on distance from camera to an object's bounding sphere.
        @remarks
            The purpose of the reference view is to ensure a consistent experience for all users. Monitors of different resolutions and aspect ratios will each have different results for the distance queries.
        @par
            It depends on gameplay testing. If all testers had 16:9 monitors and 110° FOV, then that's the value you should enter (to ensure as much as possible the experience stays consistent for all other users who don't have a 16:9 monitor and/or use a different FOV).
        @par
            If all your testers had 4:3 monitors, then enter a 4:3 resolution.
        @par
            If all your testers had varying resolutions or you just didn't care, then this feature is useless for you and should be disabled (default: disabled).
     */
    class _OgreExport DistanceLodSphereStrategy : public DistanceLodStrategyBase, public Singleton<DistanceLodSphereStrategy>
    {
    public:
        /** Default constructor. */
        DistanceLodSphereStrategy();

        /// @copydoc DistanceLodStrategy::getSquaredDepth
        Real getSquaredDepth(const MovableObject *movableObject, const Ogre::Camera *camera) const;

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
        static DistanceLodSphereStrategy& getSingleton(void);
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
        static DistanceLodSphereStrategy* getSingletonPtr(void);
    };
    /** @} */
    /** @} */

    /** \addtogroup Core
	*  @{
	*/
	/** \addtogroup LOD
	*  @{
	*/

    /** Level of detail strategy based on distance from camera to an object's bounding box.
        @remarks
            The purpose of the reference view is to ensure a consistent experience for all users. Monitors of different resolutions and aspect ratios will each have different results for the distance queries.
        @par
            It depends on gameplay testing. If all testers had 16:9 monitors and 110° FOV, then that's the value you should enter (to ensure as much as possible the experience stays consistent for all other users who don't have a 16:9 monitor and/or use a different FOV).
        @par
            If all your testers had 4:3 monitors, then enter a 4:3 resolution.
        @par
            If all your testers had varying resolutions or you just didn't care, then this feature is useless for you and should be disabled (default: disabled).
     */
    class _OgreExport DistanceLodBoxStrategy : public DistanceLodStrategyBase, public Singleton<DistanceLodBoxStrategy>
    {
    public:
        /** Default constructor. */
        DistanceLodBoxStrategy();

        /// @copydoc DistanceLodStrategy::getSquaredDepth
        Real getSquaredDepth(const MovableObject *movableObject, const Ogre::Camera *camera) const;

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
        static DistanceLodBoxStrategy& getSingleton(void);
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
        static DistanceLodBoxStrategy* getSingletonPtr(void);
    };

    /** @} */
    /** @} */

} // namespace

#endif
