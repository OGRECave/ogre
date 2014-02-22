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
PCZCamera.h  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update    :
-----------------------------------------------------------------------------
*/

#ifndef PCZCAMERA_H
#define PCZCAMERA_H

#include "OgreCamera.h"
#include "OgrePCZFrustum.h"
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
    #define MAX_EXTRA_CULLING_PLANES    40

    class PCZone;

    /** Specialized viewpoint from which an PCZone Scene can be rendered.
    */

    class _OgrePCZPluginExport PCZCamera : public Camera
    {
    public:
        /** Visibility types */
        enum Visibility
        {
            NONE,
            PARTIAL,
            FULL
        };

        /* Standard constructor */
        PCZCamera( const String& name, SceneManager* sm );
        /* Standard destructor */
        ~PCZCamera();

        /** Overridden: Retrieves the local axis-aligned bounding box for this object.
            @remarks
                This bounding box is in local coordinates.
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const;

        /* Overridden isVisible function for aabb */
        virtual bool isVisible( const AxisAlignedBox &bound, FrustumPlane *culledBy=0) const;

        /* isVisible() function for portals */
        bool isVisible(PortalBase* portal, FrustumPlane* culledBy = 0) const;

        /** Returns the visibility of the box
        */
        bool isVisibile( const AxisAlignedBox &bound );

        /** Returns the detailed visibility of the box
        */
        PCZCamera::Visibility getVisibility( const AxisAlignedBox &bound );

        /// Sets the type of projection to use (orthographic or perspective).
        void setProjectionType(ProjectionType pt);

        /* Update function (currently used for making sure the origin stuff for the
           extra culling frustum is up to date */
        void update(void);

        /** Calculate extra culling planes from portal and camera
           origin and add to list of extra culling planes */
        int addPortalCullingPlanes(PortalBase* portal);
        /// Remove extra culling planes created from the given portal
        void removePortalCullingPlanes(PortalBase* portal);
        /// Remove all extra culling planes
        void removeAllExtraCullingPlanes(void);
    protected:
        AxisAlignedBox mBox;
        PCZFrustum mExtraCullingFrustum;
    };

}

#endif
