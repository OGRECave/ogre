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
#ifndef __OverlayElementFactory_H__
#define __OverlayElementFactory_H__

#include "OgrePrerequisites.h"
#include "OgreOverlayElement.h"
#include "OgrePanelOverlayElement.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Overlays
	*  @{
	*/
	/** Defines the interface which all components wishing to 
        supply OverlayElement subclasses must implement.
    @remarks
        To allow the OverlayElement types available for inclusion on 
        overlays to be extended, OGRE allows external apps or plugins
        to register their ability to create custom OverlayElements with
        the OverlayManager, using the addOverlayElementFactory method. Classes
        wanting to do this must implement this interface.
    @par
        Each OverlayElementFactory creates a single type of OverlayElement, 
        identified by a 'type name' which must be unique.
    */
	class _OgreExport OverlayElementFactory : public OverlayAlloc
    {
    public:
        /** Destroy the overlay element factory */
        virtual ~OverlayElementFactory () {}
        /** Creates a new OverlayElement instance with the name supplied. */
        virtual OverlayElement* createOverlayElement(const String& instanceName) = 0;
        /** Destroys a OverlayElement which this factory created previously. */
        virtual void destroyOverlayElement(OverlayElement* pElement) { delete pElement; };
        /** Gets the string uniquely identifying the type of element this factory creates. */
        virtual const String& getTypeName(void) const = 0;
    };


    /** Factory for creating PanelOverlayElement instances. */
    class _OgreExport PanelOverlayElementFactory: public OverlayElementFactory
    {
    public:
        /** See OverlayElementFactory */
        OverlayElement* createOverlayElement(const String& instanceName)
        {
            return OGRE_NEW PanelOverlayElement(instanceName);
        }
        /** See OverlayElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "Panel";
            return name;
        }
    };

    /** Factory for creating BorderPanelOverlayElement instances. */
    class _OgreExport BorderPanelOverlayElementFactory: public OverlayElementFactory
    {
    public:
        /** See OverlayElementFactory */
        OverlayElement* createOverlayElement(const String& instanceName)
        {
            return OGRE_NEW BorderPanelOverlayElement(instanceName);
        }
        /** See OverlayElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "BorderPanel";
            return name;
        }
    };

    /** Factory for creating TextAreaOverlayElement instances. */
    class _OgreExport TextAreaOverlayElementFactory: public OverlayElementFactory
    {
    public:
        /** See OverlayElementFactory */
        OverlayElement* createOverlayElement(const String& instanceName)
        {
            return OGRE_NEW TextAreaOverlayElement(instanceName);
        }
        /** See OverlayElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "TextArea";
            return name;
        }
    };
	/** @} */
	/** @} */

}



#endif
