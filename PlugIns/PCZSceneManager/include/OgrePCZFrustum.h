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
OgrePCZFrustum.h  -  PCZ Supplemental Culling Frustum

This isn't really a traditional "frustum", but more a collection of
extra culling planes used by the PCZ Scene Manager for supplementing
the camera culling and light zone culling by creating extra culling
planes from visible portals.  Since portals are 4 sided, the extra 
culling planes tend to form frustums (pyramids) but nothing in the 
code really assumes that the culling planes are frustums.  They are
just treated as planes.  

The "originPlane" is a culling plane which passes through the origin
point specified.  It is used to cull portals which are close to, but
behind the camera view.  (the nature of the culling routine doesn't
give correct results if you just use the "near" plane of the standard
camera frustum (unless that near plane distance is 0.0, but that is
highly not recommended for other reasons having to do with having a 
legal view frustum).
---------------------------------------------------------------------------
begin                : Mon May 29 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 :

---------------------------------------------------------------------------
*/

#ifndef PCZ_FRUSTUM_H
#define PCZ_FRUSTUM_H

#include "OgrePCPlane.h"
#include "OgrePCZPrerequisites.h"
#include "OgreFrustum.h"

namespace Ogre
{
    class PortalBase;

    typedef list< PCPlane * >::type PCPlaneList;

    /** Specialized frustum shaped culling volume that has culling planes created from portals 
    */
    
    class _OgrePCZPluginExport PCZFrustum 
    {
    public:
        // visibility types
		enum Visibility
		{
			NONE,
			PARTIAL,
			FULL
		};

		/** Standard constructor */
		PCZFrustum();
		/** Standard destructor */
		~PCZFrustum();

		/* isVisible function for aabb */
		bool isVisible( const AxisAlignedBox &bound) const;
		/* isVisible function for sphere */
		bool isVisible( const Sphere &bound) const;
		/* isVisible() function for portals */
		bool isVisible(const PortalBase* portal) const;
		/* special function that returns true only when aabb fully fits inside the frustum. */
		bool isFullyVisible(const AxisAlignedBox& bound) const;
		/* special function that returns true only when sphere fully fits inside the frustum. */
		bool isFullyVisible(const Sphere& bound) const;
		/* special function that returns true only when portal fully fits inside the frustum. */
		bool isFullyVisible(const PortalBase* portal) const;
        /* more detailed check for visibility of an AABB */
        PCZFrustum::Visibility getVisibility(const AxisAlignedBox & bound);

		/** Calculate  culling planes from portal and Frustum
            origin and add to list of culling planes */
		int addPortalCullingPlanes(PortalBase* portal);
		/// Remove  culling planes created from the given portal
		void removePortalCullingPlanes(PortalBase* portal);
		/// Remove all  culling planes
		void removeAllCullingPlanes(void);
        /// Set the origin value
        void setOrigin(const Vector3 & newOrigin) {mOrigin = newOrigin;}
        /// Set the origin plane
        void setOriginPlane(const Vector3 &rkNormal, const Vector3 &rkPoint);
        /// Tell the frustum whether or not to use the originplane
        void setUseOriginPlane(bool yesno) {mUseOriginPlane = yesno;}
		/// Get an unused PCPlane from the CullingPlane Reservoir
		PCPlane * getUnusedCullingPlane(void);

		/// Set the projection type of this PCZFrustum.
		inline void setProjectionType(ProjectionType projType)
		{ mProjType = projType; }
		/// Get the projection type of this PCZFrustum.
		inline ProjectionType getProjectionType() const
		{ return mProjType; }

    protected:
        Vector3     mOrigin;
        Plane       mOriginPlane;
        bool        mUseOriginPlane;
        PCPlaneList mActiveCullingPlanes;
		PCPlaneList mCullingPlaneReservoir;
		ProjectionType mProjType;

    };

}


#endif
