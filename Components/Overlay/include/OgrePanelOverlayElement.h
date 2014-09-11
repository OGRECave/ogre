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

#ifndef __PanelOverlayElement_H__
#define __PanelOverlayElement_H__

#include "OgreOverlayContainer.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Overlays
	*  @{
	*/
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
    class _OgreOverlayExport PanelOverlayElement : public OverlayContainer
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
        /// Flag indicating if this panel should be visual or just group things
        bool mTransparent;
        // Texture tiling
        Real mTileX[OGRE_MAX_TEXTURE_LAYERS];
        Real mTileY[OGRE_MAX_TEXTURE_LAYERS];
        size_t mNumTexCoordsInBuffer;
        Real mU1, mV1, mU2, mV2;

        RenderOperation mRenderOp;

        /// Internal method for setting up geometry, called by OverlayElement::update
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
	/** @} */
	/** @} */

}

#endif
