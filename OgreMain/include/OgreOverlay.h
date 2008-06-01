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

#ifndef __Overlay_H__
#define __Overlay_H__

#include "OgrePrerequisites.h"
#include "OgreSceneNode.h"
#include "OgreIteratorWrappers.h"
#include "OgreMatrix4.h"

namespace Ogre {


    /** Represents a layer which is rendered on top of the 'normal' scene contents.
    @remarks
        An overlay is a container for visual components (2D and 3D) which will be 
        rendered after the main scene in order to composite heads-up-displays, menus
        or other layers on top of the contents of the scene.
    @par
        An overlay always takes up the entire size of the viewport, although the 
        components attached to it do not have to. An overlay has no visual element
        in itself, it it merely a container for visual elements.
    @par
        Overlays are created by calling OverlayManager::create, or by defining them
        in special text scripts (.overlay files). As many overlays
        as you like can be defined; after creation an overlay is hidden i.e. not
        visible until you specifically enable it by calling 'show'. This allows you to have multiple
        overlays predefined (menus etc) which you make visible only when you want.
        It is possible to have multiple overlays enabled at once; in this case the
        relative 'zorder' parameter of the overlays determine which one is displayed
        on top.
    @par
        By default overlays are rendered into all viewports. This is fine when you only
        have fullscreen viewports, but if you have picture-in-picture views, you probably
        don't want the overlay displayed in the smaller viewports. You turn this off for 
        a specific viewport by calling the Viewport::setDisplayOverlays method.
    */
	class _OgreExport Overlay : public OverlayAlloc
    {

    public:
              typedef std::list<OverlayContainer*> OverlayContainerList;
    protected:
        String mName;
        /// Internal root node, used as parent for 3D objects
        SceneNode* mRootNode;
        // 2D elements
        // OverlayContainers, linked list for easy sorting by zorder later
        // Not a map because sort can be saved since changes infrequent (unlike render queue)
        OverlayContainerList m2DElements;

        // Degrees of rotation around center
        Radian mRotate;
        // Scroll values, offsets
        Real mScrollX, mScrollY;
        // Scale values
        Real mScaleX, mScaleY;

        mutable Matrix4 mTransform;
        mutable bool mTransformOutOfDate;
        bool mTransformUpdated;
        ulong mZOrder;
        bool mVisible;
		bool mInitialised;
		String mOrigin;
        /** Internal lazy update method. */
        void updateTransform(void) const;
		/** Internal method for initialising an overlay */
		void initialise(void);
		/** Internal method for updating container elements' Z-ordering */
		void assignZOrders(void);

    public:
        /// Constructor: do not call direct, use OverlayManager::create
        Overlay(const String& name);
        virtual ~Overlay();


	    OverlayContainer* getChild(const String& name);

        /** Gets the name of this overlay. */
        const String& getName(void) const;
        /** Alters the ZOrder of this overlay. 
        @remarks
            Values between 0 and 650 are valid here.
        */
        void setZOrder(ushort zorder);
        /** Gets the ZOrder of this overlay. */
        ushort getZOrder(void) const;

        /** Gets whether the overlay is displayed or not. */
        bool isVisible(void) const;

		/** Gets whether the overlay is initialised or not. */
		bool isInitialised(void) const { return mInitialised; }

		/** Shows the overlay if it was hidden. */
        void show(void);

        /** Hides the overlay if it was visible. */
        void hide(void);

        /** Adds a 2D 'container' to the overlay.
        @remarks
            Containers are created and managed using the OverlayManager. A container
            could be as simple as a square panel, or something more complex like
            a grid or tree view. Containers group collections of other elements,
            giving them a relative coordinate space and a common z-order.
            If you want to attach a GUI widget to an overlay, you have to do it via
            a container.
        @param cont Pointer to a container to add, created using OverlayManager.
        */
        void add2D(OverlayContainer* cont);


        /** Removes a 2D container from the overlay. 
        @remarks
            NOT FAST. Consider OverlayElement::hide.
        */
        void remove2D(OverlayContainer* cont);

        /** Adds a node capable of holding 3D objects to the overlay.
        @remarks    
            Although overlays are traditionally associated with 2D elements, there 
            are reasons why you might want to attach 3D elements to the overlay too.
            For example, if you wanted to have a 3D cockpit, which was overlaid with a
            HUD, then you would create 2 overlays, one with a 3D object attached for the
            cockpit, and one with the HUD elements attached (the zorder of the HUD 
            overlay would be higher than the cockpit to ensure it was always on top).
        @par    
            A SceneNode can have any number of 3D objects attached to it. SceneNodes
            are usually created using SceneManager::createSceneNode, but in this case
			you should create a standard SceneNode instance <b>manually</b>; this is
			because these scene nodes are not managed by the SceneManager and some custom
			SceneManager plugins will rely on specialist behaviour the overlay does not
			support. By attaching a SceneNode to an overlay, you indicate that:<OL>
            <LI>You want the contents of this node to only appear when the overlay is active</LI>
            <LI>You want the node to inherit a coordinate space relative to the camera,
                rather than relative to the root scene node</LI>
            <LI>You want these objects to be rendered after the contents of the main scene
                to ensure they are rendered on top</LI>
            </OL>
            One major consideration when using 3D objects in overlays is the behaviour of 
            the depth buffer. Overlays should use materials with depth checking off, to ensure
            that their contents are always displayed on top of the main scene (to do 
            otherwise would result in objects 'poking through' the overlay). The problem
            with using 3D objects is that if they are concave, or self-overlap, then you
            can get artefacts because of the lack of depth buffer checking. So you should 
            ensure that any 3D objects you us in the overlay are convex, and don't overlap
            each other. If they must overlap, split them up and put them in 2 overlays.
			Alternatively, use a 2D element underneath them which will clear the depth buffer
			values underneath ready for the 3D element to be rendered correctly.
        */
        void add3D(SceneNode* node);

        /** Removes a 3D element from the overlay. */
        void remove3D(SceneNode* node);

        /** Clears the overlay of all attached items. */
        void clear();

        /** Sets the scrolling factor of this overlay.
        @remarks
            You can use this to set an offset to be used to scroll an 
            overlay around the screen.
        @param x Horizontal scroll value, where 0 = normal, -0.5 = scroll so that only
            the right half the screen is visible etc
        @param y Vertical scroll value, where 0 = normal, 0.5 = scroll down by half 
            a screen etc.
        */
        void setScroll(Real x, Real y);

        /** Gets the current X scroll value */
        Real getScrollX(void) const;

        /** Gets the current Y scroll value */
        Real getScrollY(void) const;

        /** Scrolls the overlay by the offsets provided.
        @remarks
            This method moves the overlay by the amounts provided. As with
            other methods on this object, a full screen width / height is represented
            by the value 1.0.
        */
        void scroll(Real xoff, Real yoff);

        /** Sets the rotation applied to this overlay.*/
        void setRotate(const Radian& angle);

        /** Gets the rotation applied to this overlay, in degrees.*/
        const Radian &getRotate(void) const { return mRotate; }

        /** Adds the passed in angle to the rotation applied to this overlay. */
        void rotate(const Radian& angle);

        /** Sets the scaling factor of this overlay.
        @remarks
            You can use this to set an scale factor to be used to zoom an 
            overlay.
        @param x Horizontal scale value, where 1.0 = normal, 0.5 = half size etc
        @param y Vertical scale value, where 1.0 = normal, 0.5 = half size etc
        */
        void setScale(Real x, Real y);

        /** Gets the current X scale value */
        Real getScaleX(void) const;

        /** Gets the current Y scale value */
        Real getScaleY(void) const;

        /** Used to transform the overlay when scrolling, scaling etc. */
        void _getWorldTransforms(Matrix4* xform) const;

        /** Internal method to put the overlay contents onto the render queue. */
        void _findVisibleObjects(Camera* cam, RenderQueue* queue);

        /** This returns a OverlayElement at position x,y. */
		virtual OverlayElement* findElementAt(Real x, Real y);

        /** Returns an iterator over all 2D elements in this manager.
        @remarks
            VectorIterator is actually a too generic name, since it also works for lists.
        */
        typedef VectorIterator<OverlayContainerList> Overlay2DElementsIterator ;
        Overlay2DElementsIterator get2DElementsIterator ()
        {
            return Overlay2DElementsIterator (m2DElements.begin(), m2DElements.end());
        }
		/** Get the origin of this overlay, e.g. a script file name.
		@remarks
			This property will only contain something if the creator of
			this overlay chose to populate it. Script loaders are advised
			to populate it.
		*/
		const String& getOrigin(void) const { return mOrigin; }
		/// Notify this overlay of it's origin
		void _notifyOrigin(const String& origin) { mOrigin = origin; }


    };

}


#endif

