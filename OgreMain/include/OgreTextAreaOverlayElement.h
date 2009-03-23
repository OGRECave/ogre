/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/

#ifndef _TextAreaOverlayElement_H__
#define _TextAreaOverlayElement_H__

#include "OgreOverlayElement.h"
#include "OgreFont.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Overlays
	*  @{
	*/
	/** This class implements an overlay element which contains simple unformatted text.
    */
    class _OgreExport TextAreaOverlayElement : public OverlayElement
    {
    public:
        enum Alignment
        {
            Left,
            Right,
            Center
        };

    public:
        /** Constructor. */
        TextAreaOverlayElement(const String& name);
        virtual ~TextAreaOverlayElement();

        virtual void initialise(void);
		virtual void setCaption(const DisplayString& text);

        void setCharHeight( Real height );
        Real getCharHeight() const;

        void setSpaceWidth( Real width );
        Real getSpaceWidth() const;

        void setFontName( const String& font );
        const String& getFontName() const;

        /** See OverlayElement. */
        virtual const String& getTypeName(void) const;
        /** See Renderable. */
        void getRenderOperation(RenderOperation& op);
        /** Overridden from OverlayElement */
        void setMaterialName(const String& matName);

        /** Sets the colour of the text. 
        @remarks
            This method establishes a constant colour for 
            the entire text. Also see setColourBottom and 
            setColourTop which allow you to set a colour gradient.
        */
        void setColour(const ColourValue& col);

        /** Gets the colour of the text. */
        const ColourValue& getColour(void) const;
        /** Sets the colour of the bottom of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        void setColourBottom(const ColourValue& col);
        /** Gets the colour of the bottom of the letters. */
        const ColourValue& getColourBottom(void) const;
        /** Sets the colour of the top of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        void setColourTop(const ColourValue& col);
        /** Gets the colour of the top of the letters. */
        const ColourValue& getColourTop(void) const;

        inline void setAlignment( Alignment a )
        {
            mAlignment = a;
			mGeomPositionsOutOfDate = true;
        }
        inline Alignment getAlignment() const
        {
            return mAlignment;
        }

        /** Overridden from OverlayElement */
        void setMetricsMode(GuiMetricsMode gmm);

        /** Overridden from OverlayElement */
        void _update(void);

        //-----------------------------------------------------------------------------------------
        /** Command object for setting the caption.
                @see ParamCommand
        */
        class _OgrePrivate CmdCaption : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the char height.
                @see ParamCommand
        */
        class _OgrePrivate CmdCharHeight : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the width of a space.
                @see ParamCommand
        */
        class _OgrePrivate CmdSpaceWidth : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the caption.
                @see ParamCommand
        */
        class _OgrePrivate CmdFontName : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the top colour.
                @see ParamCommand
        */
        class _OgrePrivate CmdColourTop : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the bottom colour.
                @see ParamCommand
        */
        class _OgrePrivate CmdColourBottom : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the constant colour.
                @see ParamCommand
        */
        class _OgrePrivate CmdColour : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the alignment.
                @see ParamCommand
        */
        class _OgrePrivate CmdAlignment : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };

    protected:
        /// The text alignment
        Alignment mAlignment;

        /// Flag indicating if this panel should be visual or just group things
        bool mTransparent;

        /// Render operation
        RenderOperation mRenderOp;

        /// Method for setting up base parameters for this class
        void addBaseParameters(void);

        static String msTypeName;

        // Command objects
        static CmdCharHeight msCmdCharHeight;
        static CmdSpaceWidth msCmdSpaceWidth;
        static CmdFontName msCmdFontName;
        static CmdColour msCmdColour;
        static CmdColourTop msCmdColourTop;
        static CmdColourBottom msCmdColourBottom;
        static CmdAlignment msCmdAlignment;


        FontPtr mpFont;
        Real mCharHeight;
        ushort mPixelCharHeight;
        Real mSpaceWidth;
        ushort mPixelSpaceWidth;
        size_t mAllocSize;
		Real mViewportAspectCoef;

        /// Colours to use for the vertices
        ColourValue mColourBottom;
        ColourValue mColourTop;
        bool mColoursChanged;


        /// Internal method to allocate memory, only reallocates when necessary
        void checkMemoryAllocation( size_t numChars );
        /// Inherited function
        virtual void updatePositionGeometry();
		/// Inherited function
		virtual void updateTextureGeometry();
        /// Updates vertex colours
        virtual void updateColours(void);
    };
	/** @} */
	/** @} */
}

#endif

