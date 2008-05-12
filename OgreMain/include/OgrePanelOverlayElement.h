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

#ifndef __PanelOverlayElement_H__
#define __PanelOverlayElement_H__

#include "OgreOverlayContainer.h"

namespace Ogre {


    /** OverlayElement representing a flat, single-material (or transparent) panel which can contain other elements.
    @remarks
        This class subclasses OverlayContainer because it can contain other elements. Like other
        containers, if hidden it's contents are also hidden, if moved it's contents also move etc. 
        The panel itself is a 2D rectangle which is either completely transparent, or is rendered 
        with a single material. The texture(s) on the panel can be tiled depending on your requirements.
    @par
        This component is suitable for backgrounds and grouping other elements. Note that because
        it has a single repeating material it cannot have a discrete border (unless the texture has one and
        the texture is tiled only once). For a bordered panel, see it's subclass BorderPanelOverlayElement.
    @par
        Note that the material can have all the usual effects applied to it like multiple texture
        layers, scrolling / animated textures etc. For multiple texture layers, you have to set 
        the tiling level for each layer.
    */
    class _OgreExport PanelOverlayElement : public OverlayContainer
    {
    public:
        /** Constructor. */
        PanelOverlayElement(const String& name);
        virtual ~PanelOverlayElement();

        /** Initialise */
        virtual void initialise(void);

        /** Sets the number of times textures should repeat. 
        @param x The number of times the texture should repeat horizontally
        @param y The number of times the texture should repeat vertically
        @param layer The texture layer to specify (only needs to be altered if 
            you're using a multi-texture layer material)
        */
        void setTiling(Real x, Real y, ushort layer = 0);

        Real getTileX(ushort layer = 0) const;
        /** Gets the number of times the texture should repeat vertically. 
        @param layer The texture layer to specify (only needs to be altered if 
            you're using a multi-texture layer material)
        */
        Real getTileY(ushort layer = 0) const;

        /** Sets the texture coordinates for the panel. */
        void setUV(Real u1, Real v1, Real u2, Real v2);

        /** Get the uv coordinates for the panel*/
        void getUV(Real& u1, Real& v1, Real& u2, Real& v2) const;

        /** Sets whether this panel is transparent (used only as a grouping level), or 
            if it is actually rendered.
        */
        void setTransparent(bool isTransparent);

        /** Returns whether this panel is transparent. */
        bool isTransparent(void) const;

        /** See OverlayElement. */
        virtual const String& getTypeName(void) const;
        /** See Renderable. */
        void getRenderOperation(RenderOperation& op);
        /** Overridden from OverlayElement */
        void setMaterialName(const String& matName);
        /** Overridden from OverlayContainer */
        void _updateRenderQueue(RenderQueue* queue);


        /** Command object for specifying tiling (see ParamCommand).*/
        class _OgrePrivate CmdTiling : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying transparency (see ParamCommand).*/
        class _OgrePrivate CmdTransparent : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying UV coordinates (see ParamCommand).*/
        class _OgrePrivate CmdUVCoords : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
    protected:
        // Flag indicating if this panel should be visual or just group things
        bool mTransparent;
        // Texture tiling
        Real mTileX[OGRE_MAX_TEXTURE_LAYERS];
        Real mTileY[OGRE_MAX_TEXTURE_LAYERS];
        size_t mNumTexCoordsInBuffer;
        Real mU1, mV1, mU2, mV2;

        RenderOperation mRenderOp;

        /// internal method for setting up geometry, called by OverlayElement::update
        virtual void updatePositionGeometry(void);

        /// Called to update the texture coords when layers change
        virtual void updateTextureGeometry(void);

        /// Method for setting up base parameters for this class
        void addBaseParameters(void);

        static String msTypeName;

        // Command objects
        static CmdTiling msCmdTiling;
        static CmdTransparent msCmdTransparent;
        static CmdUVCoords msCmdUVCoords;

    };

}

#endif
