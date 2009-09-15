/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreStableHeaders.h"
#include "OgreOverlayElementCommands.h"
#include "OgreOverlayElement.h"
#include "OgreStringConverter.h"


namespace Ogre {

    namespace OverlayElementCommands {

        //-----------------------------------------------------------------------
        String CmdLeft::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const OverlayElement*>(target)->getLeft() );
        }
        void CmdLeft::doSet(void* target, const String& val)
        {
            static_cast<OverlayElement*>(target)->setLeft(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdTop::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const OverlayElement*>(target)->getTop() );
        }
        void CmdTop::doSet(void* target, const String& val)
        {
            static_cast<OverlayElement*>(target)->setTop(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdWidth::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const OverlayElement*>(target)->getWidth() );
        }
        void CmdWidth::doSet(void* target, const String& val)
        {
            static_cast<OverlayElement*>(target)->setWidth(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdHeight::doGet(const void* target) const
        {
            return StringConverter::toString(
                static_cast<const OverlayElement*>(target)->getHeight() );
        }
        void CmdHeight::doSet(void* target, const String& val)
        {
            static_cast<OverlayElement*>(target)->setHeight(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaterial::doGet(const void* target) const
        {
            return static_cast<const OverlayElement*>(target)->getMaterialName();
        }
        void CmdMaterial::doSet(void* target, const String& val)
        {
			if (val != "")
			{
				static_cast<OverlayElement*>(target)->setMaterialName(val);
			}
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdCaption::doGet(const void* target) const
        {
            return static_cast<const OverlayElement*>(target)->getCaption();
        }
        void CmdCaption::doSet(void* target, const String& val)
        {
            static_cast<OverlayElement*>(target)->setCaption(val);
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdMetricsMode::doGet(const void* target) const
        {
            GuiMetricsMode gmm = 
                static_cast<const OverlayElement*>(target)->getMetricsMode();

            switch (gmm)
            {
            case GMM_PIXELS :
                return "pixels";

            case GMM_RELATIVE_ASPECT_ADJUSTED :
                return "relative_aspect_adjusted";

            default :
                return "relative";
            }
        }
        void CmdMetricsMode::doSet(void* target, const String& val)
        {
            if (val == "pixels")
            {
                static_cast<OverlayElement*>(target)->setMetricsMode(GMM_PIXELS);
            }
            else if (val == "relative_aspect_adjusted")
            {
                static_cast<OverlayElement*>(target)->setMetricsMode(GMM_RELATIVE_ASPECT_ADJUSTED);
            }
            else
            {
                static_cast<OverlayElement*>(target)->setMetricsMode(GMM_RELATIVE);
            }
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdHorizontalAlign::doGet(const void* target) const
        {
            GuiHorizontalAlignment gha = 
                static_cast<const OverlayElement*>(target)->getHorizontalAlignment();
            switch(gha)
            {
            case GHA_LEFT:
                return "left";
            case GHA_RIGHT:
                return "right";
            case GHA_CENTER:
                return "center";
            }
            // To keep compiler happy
            return "center";
        }
        void CmdHorizontalAlign::doSet(void* target, const String& val)
        {
            if (val == "left")
            {
                static_cast<OverlayElement*>(target)->setHorizontalAlignment(GHA_LEFT);
            }
            else if (val == "right")
            {
                static_cast<OverlayElement*>(target)->setHorizontalAlignment(GHA_RIGHT);
            }
            else
            {
                static_cast<OverlayElement*>(target)->setHorizontalAlignment(GHA_CENTER);
            }
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdVerticalAlign::doGet(const void* target) const
        {
            GuiVerticalAlignment gva = 
                static_cast<const OverlayElement*>(target)->getVerticalAlignment();
            switch(gva)
            {
            case GVA_TOP:
                return "top";
            case GVA_BOTTOM:
                return "bottom";
            case GVA_CENTER:
                return "center";
            }
            // To keep compiler happy
            return "center";
        }
        void CmdVerticalAlign::doSet(void* target, const String& val)
        {
            if (val == "top")
            {
                static_cast<OverlayElement*>(target)->setVerticalAlignment(GVA_TOP);
            }
            else if (val == "bottom")
            {
                static_cast<OverlayElement*>(target)->setVerticalAlignment(GVA_BOTTOM);
            }
            else
            {
                static_cast<OverlayElement*>(target)->setVerticalAlignment(GVA_CENTER);
            }
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdVisible::doGet(const void* target) const
        {
            bool visible = 
                static_cast<const OverlayElement*>(target)->isVisible();
            switch(visible)
            {
            case true:
                return "true";
            case false:
                return "false";
            }
            // To keep compiler happy
            return "true";
        }
        void CmdVisible::doSet(void* target, const String& val)
        {
            if (val == "true")
            {
                static_cast<OverlayElement*>(target)->show();
            }
            else if (val == "false")
            {
                static_cast<OverlayElement*>(target)->hide();
            }
        }
        //-----------------------------------------------------------------------
    }
}

