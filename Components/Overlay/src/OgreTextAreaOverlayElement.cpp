/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/

#include "OgreTextAreaOverlayElement.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreOverlayManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreFont.h"
#include "OgreFontManager.h"
#include "OgreOverlayElement.h"

namespace Ogre {

#define DEFAULT_INITIAL_CHARS 12
    //---------------------------------------------------------------------
    String TextAreaOverlayElement::msTypeName = "TextArea";
    //-----------------------------------------------------------------------------------------
    class _OgrePrivate CmdCaption : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdCharHeight : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdSpaceWidth : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdFontName : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdColourTop : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdColourBottom : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdColour : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    class _OgrePrivate CmdAlignment : public ParamCommand
    {
    public:
        String doGet( const void* target ) const override;
        void doSet( void* target, const String& val ) override;
    };
    // Command objects
    static CmdCharHeight msCmdCharHeight;
    static CmdSpaceWidth msCmdSpaceWidth;
    static CmdFontName msCmdFontName;
    static CmdColour msCmdColour;
    static CmdColourTop msCmdColourTop;
    static CmdColourBottom msCmdColourBottom;
    static CmdAlignment msCmdAlignment;
    //---------------------------------------------------------------------
    #define POS_TEX_BINDING 0
    #define COLOUR_BINDING 1
    #define UNICODE_NEL 0x0085
    #define UNICODE_CR 0x000D
    #define UNICODE_LF 0x000A
    #define UNICODE_SPACE 0x0020
    //---------------------------------------------------------------------
    TextAreaOverlayElement::TextAreaOverlayElement(const String& name)
        : OverlayElement(name), mColourBottom(ColourValue::White), mColourTop(ColourValue::White)
    {
        mTransparent = false;
        mAlignment = Left;

        mColoursChanged = true;

        mAllocSize = 0;

        mCharHeight = 0.02;
        mPixelCharHeight = 12;
        mSpaceWidth = 0;
        mPixelSpaceWidth = 0;
        mViewportAspectCoef = 1;

        if (createParamDictionary("TextAreaOverlayElement"))
        {
            addBaseParameters();
        }
    }

    void TextAreaOverlayElement::initialise(void)
    {
        if (!mInitialised)
        {
            // Set up the render op
            // Combine positions and texture coords since they tend to change together
            // since character sizes are different
            mRenderOp.vertexData = OGRE_NEW VertexData();
            VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
            size_t offset = 0;
            // Positions
            offset += decl->addElement(POS_TEX_BINDING, offset, VET_FLOAT3, VES_POSITION).getSize();
            // Texcoords
            decl->addElement(POS_TEX_BINDING, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
            // Colours - store these in a separate buffer because they change less often
            decl->addElement(COLOUR_BINDING, 0, VET_UBYTE4_NORM, VES_DIFFUSE);

            mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
            mRenderOp.useIndexes = false;
            mRenderOp.vertexData->vertexStart = 0;
            mRenderOp.useGlobalInstancing = false;
            // Vertex buffer will be created in checkMemoryAllocation

            mInitialised = true;

            checkMemoryAllocation( DEFAULT_INITIAL_CHARS );
        }

    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::checkMemoryAllocation( size_t numChars )
    {
        if( mAllocSize < numChars)
        {
            mAllocSize = numChars;

            // Create and bind new buffers
            // Note that old buffers will be deleted automatically through reference counting
            _restoreManualHardwareResources();
        }
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::_restoreManualHardwareResources()
    {
        if(!mInitialised)
            return;

        // 6 verts per char since we're doing tri lists without indexes
        // Allocate space for positions & texture coords
        // Note - mRenderOp.vertexData->vertexCount will be less than allocatedVertexCount
        size_t allocatedVertexCount = mAllocSize * 6;
        VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
        VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

        // Create dynamic since text tends to change a lot
        // positions & texcoords
        HardwareVertexBufferSharedPtr vbuf = 
            HardwareBufferManager::getSingleton().
                createVertexBuffer(
                    decl->getVertexSize(POS_TEX_BINDING), 
                    allocatedVertexCount,
                    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
                    true);//Workaround, using shadow buffer to avoid stall due to buffer mapping
        bind->setBinding(POS_TEX_BINDING, vbuf);

        // colours
        vbuf = HardwareBufferManager::getSingleton().
                createVertexBuffer(
                    decl->getVertexSize(COLOUR_BINDING), 
                    allocatedVertexCount,
                    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
                    true);//Workaround, using shadow buffer to avoid stall due to buffer mapping
        bind->setBinding(COLOUR_BINDING, vbuf);

        // Buffers are restored, but with trash within
        mGeomPositionsOutOfDate = true;
        mGeomUVsOutOfDate = true;
        mColoursChanged = true;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::_releaseManualHardwareResources()
    {
        if(!mInitialised)
            return;

        VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;
        bind->unsetAllBindings();
    }
    //---------------------------------------------------------------------

    void TextAreaOverlayElement::updatePositionGeometry()
    {
        float *pVert;

        if (!mFont)
        {
            // not initialised yet, probably due to the order of creation in a template
            return;
        }

        mFont->load();  // ensure glyph info is there

        auto decoded = utftoc32(mCaption);
        size_t charlen = decoded.size();
        checkMemoryAllocation( charlen );

        mRenderOp.vertexData->vertexCount = charlen * 6;
        // Get position / texcoord buffer
        const HardwareVertexBufferSharedPtr& vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(POS_TEX_BINDING);
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        pVert = static_cast<float*>(vbufLock.pData);

        float largestWidth = 0;
        float left = _getDerivedLeft() * 2.0f - 1.0f;
        float top = -( (_getDerivedTop() * 2.0f ) - 1.0f );

        // Use iterator
        auto iend = decoded.end();
        bool newLine = true;
        for( auto i = decoded.begin(); i != iend; ++i )
        {
            if( newLine )
            {
                Real len = 0.0f;
                for( auto j = i; j != iend; j++ )
                {
                    Font::CodePoint character = *j;
                    if (character == UNICODE_CR
                        || character == UNICODE_NEL
                        || character == UNICODE_LF) 
                    {
                        break;
                    }
                    else if (character == UNICODE_SPACE && mSpaceWidth) // space
                    {
                        len += mSpaceWidth * 2.0f * mViewportAspectCoef;
                    }
                    else 
                    {
                        len += mFont->getGlyphInfo(character).advance * mCharHeight * 2.0f * mViewportAspectCoef;
                    }
                }

                if( mAlignment == Right )
                    left -= len;
                else if( mAlignment == Center )
                    left -= len * 0.5f;

                newLine = false;
            }

            Font::CodePoint character = *i;
            if (character == UNICODE_CR
                || character == UNICODE_NEL
                || character == UNICODE_LF)
            {
                left = _getDerivedLeft() * 2.0f - 1.0f;
                top -= mCharHeight * 2.0f;
                newLine = true;
                // Also reduce tri count
                mRenderOp.vertexData->vertexCount -= 6;

                // consume CR/LF in one
                if (character == UNICODE_CR)
                {
                    auto peeki = i + 1;
                    if (peeki != iend && *peeki == UNICODE_LF)
                    {
                        i = peeki; // skip both as one newline
                        // Also reduce tri count
                        mRenderOp.vertexData->vertexCount -= 6;
                    }

                }
                continue;
            }
            else if (character == UNICODE_SPACE && mSpaceWidth) // space
            {
                // Just leave a gap, no tris
                left += mSpaceWidth * 2.0f * mViewportAspectCoef;
                // Also reduce tri count
                mRenderOp.vertexData->vertexCount -= 6;
                continue;
            }

            const auto& glyphInfo = mFont->getGlyphInfo(character);
            float width = glyphInfo.aspectRatio * mViewportAspectCoef * mCharHeight * 2.0f;
            const Font::UVRect& uvRect = glyphInfo.uvRect;

            if(uvRect.isNull())
            {
                // Just leave a gap, no tris
                left += glyphInfo.advance * mCharHeight * 2.0f * mViewportAspectCoef;
                // Also reduce tri count
                mRenderOp.vertexData->vertexCount -= 6;
                continue;
            }

            left += glyphInfo.bearing * mCharHeight * 2 * mViewportAspectCoef;
            FloatRect pos(left, top, left + width, top - mCharHeight * 2);

            // each vert is (x, y, z, u, v)
            //-------------------------------------------------------------------------------------
            // First tri
            *pVert++ = pos.left;
            *pVert++ = pos.top;
            *pVert++ = -1.0;
            *pVert++ = uvRect.left;
            *pVert++ = uvRect.top;

            *pVert++ = pos.left;
            *pVert++ = pos.bottom;
            *pVert++ = -1.0;
            *pVert++ = uvRect.left;
            *pVert++ = uvRect.bottom;

            *pVert++ = pos.right;
            *pVert++ = pos.top;
            *pVert++ = -1.0;
            *pVert++ = uvRect.right;
            *pVert++ = uvRect.top;
            //-------------------------------------------------------------------------------------

            //-------------------------------------------------------------------------------------
            // Second tri
            *pVert++ = pos.right;
            *pVert++ = pos.top;
            *pVert++ = -1.0;
            *pVert++ = uvRect.right;
            *pVert++ = uvRect.top;

            *pVert++ = pos.left;
            *pVert++ = pos.bottom;
            *pVert++ = -1.0;
            *pVert++ = uvRect.left;
            *pVert++ = uvRect.bottom;

            *pVert++ = pos.right;
            *pVert++ = pos.bottom;
            *pVert++ = -1.0;
            *pVert++ = uvRect.right;
            *pVert++ = uvRect.bottom;
            //-------------------------------------------------------------------------------------

            // advance
            left += (glyphInfo.advance  - glyphInfo.bearing) * mCharHeight * 2.0f * mViewportAspectCoef;

            float currentWidth = (left + 1)/2 - _getDerivedLeft();
            if (currentWidth > largestWidth)
            {
                largestWidth = currentWidth;

            }
        }

        if (mMetricsMode == GMM_PIXELS)
        {
            // Derive parametric version of dimensions
            Real vpWidth;
            vpWidth = (Real) (OverlayManager::getSingleton().getViewportWidth());

            largestWidth *= vpWidth;
        };

        if (getWidth() < largestWidth)
            setWidth(largestWidth);
    }

    void TextAreaOverlayElement::updateTextureGeometry()
    {
        // Nothing to do, we combine positions and textures
    }

    void TextAreaOverlayElement::setCaption( const DisplayString& caption )
    {
        mCaption = caption;
        mGeomPositionsOutOfDate = true;
        mGeomUVsOutOfDate = true;
    }

    void TextAreaOverlayElement::setFontName( const String& font, const String& group )
    {
        mFont = FontManager::getSingleton().getByName(font, group);
        if (!mFont)
            OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find font " + font,
                "TextAreaOverlayElement::setFontName" );
        mMaterial.reset();

        mGeomPositionsOutOfDate = true;
        mGeomUVsOutOfDate = true;
    }
    const String& TextAreaOverlayElement::getFontName() const
    {
        return mFont->getName();
    }

    void TextAreaOverlayElement::setCharHeight( Real height )
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelCharHeight = static_cast<unsigned short>(height);
        }
        else
        {
            mCharHeight = height;
        }
        mGeomPositionsOutOfDate = true;
    }
    Real TextAreaOverlayElement::getCharHeight() const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            return mPixelCharHeight;
        }
        else
        {
            return mCharHeight;
        }
    }

    void TextAreaOverlayElement::setSpaceWidth( Real width )
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelSpaceWidth = static_cast<unsigned short>(width);
        }
        else
        {
            mSpaceWidth = width;
        }

        mGeomPositionsOutOfDate = true;
    }
    Real TextAreaOverlayElement::getSpaceWidth() const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            return mPixelSpaceWidth;
        }
        else
        {
            return mSpaceWidth;
        }
    }

    //---------------------------------------------------------------------
    TextAreaOverlayElement::~TextAreaOverlayElement()
    {
        OGRE_DELETE mRenderOp.vertexData;
    }
    //---------------------------------------------------------------------
    const String& TextAreaOverlayElement::getTypeName(void) const
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    const MaterialPtr& TextAreaOverlayElement::getMaterial(void) const
    {
        // On-demand load
        // Moved from setFontName to avoid issues with background parsing of scripts
        if (!mMaterial && mFont)
        {
            mFont->load();
            // Ugly hack, but we need to override for lazy-load
            *const_cast<MaterialPtr*>(&mMaterial) = mFont->getMaterial();
            mMaterial->setDepthCheckEnabled(false);
        }
        return mMaterial;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::getRenderOperation(RenderOperation& op)
    { 
        op = mRenderOp;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::addBaseParameters(void)
    {
        OverlayElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("char_height", 
            "Sets the height of the characters in relation to the screen."
            , PT_REAL),
            &msCmdCharHeight);

        dict->addParameter(ParameterDef("space_width", 
            "Sets the width of a space in relation to the screen."
            , PT_REAL),
            &msCmdSpaceWidth);

        dict->addParameter(ParameterDef("font_name", 
            "Sets the name of the font to use."
            , PT_STRING),
            &msCmdFontName);

        dict->addParameter(ParameterDef("colour", 
            "Sets the colour of the font (a solid colour)."
            , PT_STRING),
            &msCmdColour);

        dict->addParameter(ParameterDef("colour_bottom", 
            "Sets the colour of the font at the bottom (a gradient colour)."
            , PT_STRING),
            &msCmdColourBottom);

        dict->addParameter(ParameterDef("colour_top", 
            "Sets the colour of the font at the top (a gradient colour)."
            , PT_STRING),
            &msCmdColourTop);

        dict->addParameter(ParameterDef("alignment", 
            "Sets the alignment of the text: 'left', 'center' or 'right'."
            , PT_STRING),
            &msCmdAlignment);
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::setColour(const ColourValue& col)
    {
        mColourBottom = mColourTop = col;
        mColoursChanged = true;
    }
    //---------------------------------------------------------------------
    const ColourValue& TextAreaOverlayElement::getColour(void) const
    {
        // Either one
        return mColourTop;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::setColourBottom(const ColourValue& col)
    {
        mColourBottom = col;
        mColoursChanged = true;
    }
    //---------------------------------------------------------------------
    const ColourValue& TextAreaOverlayElement::getColourBottom(void) const
    {
        return mColourBottom;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::setColourTop(const ColourValue& col)
    {
        mColourTop = col;
        mColoursChanged = true;
    }
    //---------------------------------------------------------------------
    const ColourValue& TextAreaOverlayElement::getColourTop(void) const
    {
        return mColourTop;
    }
    //---------------------------------------------------------------------
    void TextAreaOverlayElement::updateColours(void)
    {
        // Convert to system-specific
        RGBA topColour = mColourTop.getAsBYTE();
        RGBA bottomColour = mColourBottom.getAsBYTE();

        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(COLOUR_BINDING);

        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        RGBA* pDest = static_cast<RGBA*>(vbufLock.pData);

        for (size_t i = 0; i < mAllocSize; ++i)
        {
            // First tri (top, bottom, top)
            *pDest++ = topColour;
            *pDest++ = bottomColour;
            *pDest++ = topColour;
            // Second tri (top, bottom, bottom)
            *pDest++ = topColour;
            *pDest++ = bottomColour;
            *pDest++ = bottomColour;
        }
    }
    //-----------------------------------------------------------------------
    void TextAreaOverlayElement::setMetricsMode(GuiMetricsMode gmm)
    {
        Real vpWidth, vpHeight;
        vpWidth = (Real) (OverlayManager::getSingleton().getViewportWidth());
        vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

        mViewportAspectCoef = vpHeight/vpWidth;

        OverlayElement::setMetricsMode(gmm);

        switch (mMetricsMode)
        {
        case GMM_PIXELS:
            // set pixel variables based on viewport multipliers
            mPixelCharHeight = static_cast<unsigned short>(mCharHeight * vpHeight);
            mPixelSpaceWidth = static_cast<unsigned short>(mSpaceWidth * vpHeight);
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED:
            // set pixel variables multiplied by the height constant
            mPixelCharHeight = static_cast<unsigned short>(mCharHeight * 10000.0);
            mPixelSpaceWidth = static_cast<unsigned short>(mSpaceWidth * 10000.0);
            break;

        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
    void TextAreaOverlayElement::_update(void)
    {
        Real vpWidth, vpHeight;
        vpWidth = (Real) (OverlayManager::getSingleton().getViewportWidth());
        vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

        mViewportAspectCoef = vpHeight/vpWidth;

        // Check size if pixel-based / relative-aspect-adjusted
        switch (mMetricsMode)
        {
        case GMM_PIXELS:
            if(mGeomPositionsOutOfDate)
            {
                // recalculate character size
                mCharHeight = (Real) mPixelCharHeight / vpHeight;
                mSpaceWidth = (Real) mPixelSpaceWidth / vpHeight;
                mGeomPositionsOutOfDate = true;
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED:
            if(mGeomPositionsOutOfDate)
            {
                // recalculate character size
                mCharHeight = (Real) mPixelCharHeight / 10000.0f;
                mSpaceWidth = (Real) mPixelSpaceWidth / 10000.0f;
                mGeomPositionsOutOfDate = true;
            }
            break;

        default:
            break;
        }

        OverlayElement::_update();

        if (mColoursChanged && mInitialised)
        {
            updateColours();
            mColoursChanged = false;
        }
    }
    //---------------------------------------------------------------------------------------------
    // Char height command object
    //
    String CmdCharHeight::doGet( const void* target ) const
    {
        return StringConverter::toString( 
            static_cast< const TextAreaOverlayElement* >( target )->getCharHeight() );
    }
    void CmdCharHeight::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setCharHeight( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------
    // Space width command object
    //
    String CmdSpaceWidth::doGet( const void* target ) const
    {
        return StringConverter::toString( 
            static_cast< const TextAreaOverlayElement* >( target )->getSpaceWidth() );
    }
    void CmdSpaceWidth::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setSpaceWidth( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Font name command object
    //
    String CmdFontName::doGet( const void* target ) const
    {
        return static_cast< const TextAreaOverlayElement* >( target )->getFont()->getName();
    }
    void CmdFontName::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setFontName( val );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Colour command object
    //
    String CmdColour::doGet( const void* target ) const
    {
        return StringConverter::toString (
            static_cast< const TextAreaOverlayElement* >( target )->getColour());
    }
    void CmdColour::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setColour( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Top colour command object
    //
    String CmdColourTop::doGet( const void* target ) const
    {
        return StringConverter::toString (
            static_cast< const TextAreaOverlayElement* >( target )->getColourTop());
    }
    void CmdColourTop::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setColourTop( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Bottom colour command object
    //
    String CmdColourBottom::doGet( const void* target ) const
    {
        return StringConverter::toString (
            static_cast< const TextAreaOverlayElement* >( target )->getColourBottom());
    }
    void CmdColourBottom::doSet( void* target, const String& val )
    {
        static_cast< TextAreaOverlayElement* >( target )->setColourBottom( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Alignment command object
    //
    String CmdAlignment::doGet( const void* target ) const
    {
        TextAreaOverlayElement::Alignment align = static_cast< const TextAreaOverlayElement* >( target )->getAlignment();
        switch (align)
        {
            case TextAreaOverlayElement::Left:
                return "left";
            case TextAreaOverlayElement::Center:
                return "center";
            case TextAreaOverlayElement::Right:
                return "right";
                
        }
        // To keep compiler happy
        return "left";
    }
    void CmdAlignment::doSet( void* target, const String& val )
    {
        if (val == "center")
        {
            static_cast< TextAreaOverlayElement* >( target )->setAlignment(TextAreaOverlayElement::Center);
        }
        else if (val == "right")
        {
            static_cast< TextAreaOverlayElement* >( target )->setAlignment(TextAreaOverlayElement::Right);
        }
        else
        {
            static_cast< TextAreaOverlayElement* >( target )->setAlignment(TextAreaOverlayElement::Left);
        }
    }
    //---------------------------------------------------------------------------------------------
}
