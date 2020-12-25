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

#include "OgreFont.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreTextureUnitState.h"
#include "OgreTechnique.h"
#include "OgreBitwise.h"

#include "OgreUTFString.h"
#include "OgreBillboardSet.h"
#include "OgreBillboard.h"

#define generic _generic    // keyword for C++/CX
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#undef generic



namespace Ogre
{
    //---------------------------------------------------------------------
    Font::CmdType Font::msTypeCmd;
    Font::CmdSource Font::msSourceCmd;
    Font::CmdCharSpacer Font::msCharacterSpacerCmd;
    Font::CmdSize Font::msSizeCmd;
    Font::CmdResolution Font::msResolutionCmd;
    Font::CmdCodePoints Font::msCodePointsCmd;

    //---------------------------------------------------------------------
    Font::Font(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        :Resource (creator, name, handle, group, isManual, loader),
        mType(FT_TRUETYPE), mTtfSize(0), mTtfResolution(0), mTtfMaxBearingY(0), mAntialiasColour(false)
    {

        if (createParamDictionary("Font"))
        {
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(
                ParameterDef("type", "'truetype' or 'image' based font", PT_STRING),
                &msTypeCmd);
            dict->addParameter(
                ParameterDef("source", "Filename of the source of the font.", PT_STRING),
                &msSourceCmd);
            dict->addParameter(
                ParameterDef("character_spacer", "Spacing between characters to prevent overlap artifacts.", PT_STRING),
                &msCharacterSpacerCmd);
            dict->addParameter(
                ParameterDef("size", "True type size", PT_REAL),
                &msSizeCmd);
            dict->addParameter(
                ParameterDef("resolution", "True type resolution", PT_UNSIGNED_INT),
                &msResolutionCmd);
            dict->addParameter(
                ParameterDef("code_points", "Add a range of code points", PT_STRING),
                &msCodePointsCmd);
        }

    }
    //---------------------------------------------------------------------
    Font::~Font()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload();
    }
    //---------------------------------------------------------------------
    void Font::setType(FontType ftype)
    {
        mType = ftype;
    }
    //---------------------------------------------------------------------
    FontType Font::getType(void) const
    {
        return mType;
    }
    //---------------------------------------------------------------------
    void Font::setSource(const String& source)
    {
        mSource = source;
    }
    //---------------------------------------------------------------------
    void Font::setTrueTypeSize(Real ttfSize)
    {
        mTtfSize = ttfSize;
    }
    //---------------------------------------------------------------------
    void Font::setTrueTypeResolution(uint ttfResolution)
    {
        mTtfResolution = ttfResolution;
    }
    //---------------------------------------------------------------------
    const String& Font::getSource(void) const
    {
        return mSource;
    }
    //---------------------------------------------------------------------
    Real Font::getTrueTypeSize(void) const
    {
        return mTtfSize;
    }
    //---------------------------------------------------------------------
    uint Font::getTrueTypeResolution(void) const
    {
        return mTtfResolution;
    }
    //---------------------------------------------------------------------
    int Font::getTrueTypeMaxBearingY() const
    {
        return mTtfMaxBearingY;
    }
    //---------------------------------------------------------------------
    const Font::GlyphInfo& Font::getGlyphInfo(CodePoint id) const
    {
        CodePointMap::const_iterator i = mCodePointMap.find(id);
        if (i == mCodePointMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Code point " + StringConverter::toString(id) + " not found in font "
                + mName, "Font::getGlyphInfo");
        }
        return i->second;
    }
    //---------------------------------------------------------------------
    void Font::_setMaterial(const MaterialPtr &mat)
    {
        mMaterial = mat;
    }

    void Font::putText(BillboardSet* bbs, const String& text, float height, const ColourValue& colour)
    {
        // ensure loaded
        load();
        // configure Billboard for display
        bbs->setMaterial(mMaterial);
        bbs->setBillboardType(BBT_PERPENDICULAR_COMMON);
        bbs->setBillboardOrigin(BBO_CENTER_LEFT);
        bbs->setDefaultDimensions(0, 0);

        float spaceWidth = mCodePointMap.find('0')->second.aspectRatio * height;

        UTFString utfText = text;

        const auto& bbox = bbs->getBoundingBox();

        float left = 0;
        float top = bbox == AxisAlignedBox::BOX_NULL ? 0 : bbox.getMinimum().y - height;
        for (auto cpId : utfText)
        {
            if (cpId == ' ')
            {
                left += spaceWidth;
                continue;
            }

            if(cpId == '\n')
            {
                top -= height;
                left = 0;
                continue;
            }

            auto cp = mCodePointMap.find(cpId);
            if (cp == mCodePointMap.end())
                continue;

            float width = cp->second.aspectRatio * height;
            auto bb = bbs->createBillboard(Vector3(left, top, 0), colour);
            bb->setDimensions(width, height);
            bb->setTexcoordRect(cp->second.uvRect);

            left += width;
        }
    }

    //---------------------------------------------------------------------
    void Font::loadImpl()
    {
        // Create a new material
        mMaterial =  MaterialManager::getSingleton().create(
            "Fonts/" + mName,  mGroup);

        if (!mMaterial)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Error creating new material!", "Font::load" );
        }

        if (mType == FT_TRUETYPE)
        {
            createTextureFromFont();
        }
        else
        {
            // Manually load since we need to load to get alpha
            mTexture = TextureManager::getSingleton().load(mSource, mGroup, TEX_TYPE_2D, 0);
        }

        // Make sure material is aware of colour per vertex.
        auto pass = mMaterial->getTechnique(0)->getPass(0);
        pass->setVertexColourTracking(TVC_DIFFUSE);

        // lighting and culling also do not make much sense
        pass->setCullingMode(CULL_NONE);
        pass->setLightingEnabled(false);
        mMaterial->setReceiveShadows(false);
        // font quads should not occlude things
        pass->setDepthWriteEnabled(false);

        TextureUnitState *texLayer = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState();
        texLayer->setTexture(mTexture);
        // Clamp to avoid fuzzy edges
        texLayer->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );
        // Allow min/mag filter, but no mip
        texLayer->setTextureFiltering(FO_LINEAR, FO_LINEAR, FO_NONE);


        // Set up blending
        if (mTexture->hasAlpha())
        {
            mMaterial->setSceneBlending( SBT_TRANSPARENT_ALPHA );
        }
        else
        {
            // Use add if no alpha (assume black background)
            mMaterial->setSceneBlending(SBT_ADD);
        }
    }
    //---------------------------------------------------------------------
    void Font::unloadImpl()
    {
        if (mMaterial)
        {
            MaterialManager::getSingleton().remove(mMaterial->getHandle());
            mMaterial.reset();
        }

        if (mTexture)
        {
            mTexture->unload();
            mTexture.reset();
        }
    }
    //---------------------------------------------------------------------
    void Font::createTextureFromFont(void)
    {
        // Just create the texture here, and point it at ourselves for when
        // it wants to (re)load for real
        mTexture = TextureManager::getSingleton().create(mName + "Texture", mGroup, true, this);
        mTexture->setTextureType(TEX_TYPE_2D);
        mTexture->setNumMipmaps(0);
        mTexture->load();
    }
    //---------------------------------------------------------------------
    void Font::loadResource(Resource* res)
    {
        // ManualResourceLoader implementation - load the texture
        FT_Library ftLibrary;
        // Init freetype
        if( FT_Init_FreeType( &ftLibrary ) )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Could not init FreeType library!",
            "Font::Font");

        FT_Face face;

        // Locate ttf file, load it pre-buffered into memory by wrapping the
        // original DataStream in a MemoryDataStream
        DataStreamPtr dataStreamPtr =
            ResourceGroupManager::getSingleton().openResource(
                mSource, mGroup, this);
        MemoryDataStream ttfchunk(dataStreamPtr);

        // Load font
        if( FT_New_Memory_Face( ftLibrary, ttfchunk.getPtr(), (FT_Long)ttfchunk.size() , 0, &face ) )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
            "Could not open font face!", "Font::createTextureFromFont" );


        // Convert our point size to freetype 26.6 fixed point format
        FT_F26Dot6 ftSize = (FT_F26Dot6)(mTtfSize * (1 << 6));
        if( FT_Set_Char_Size( face, ftSize, 0, mTtfResolution, mTtfResolution ) )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR,
            "Could not set char size!", "Font::createTextureFromFont" );

        //FILE *fo_def = stdout;

        FT_Pos max_height = 0, max_width = 0;

        // If codepoints not supplied, assume ASCII
        if (mCodePointRangeList.empty())
        {
            mCodePointRangeList.push_back(CodePointRange(33, 126));
        }

        // Calculate maximum width, height and bearing
        size_t glyphCount = 0;
        for (const CodePointRange& range : mCodePointRangeList)
        {
            for(CodePoint cp = range.first; cp <= range.second; ++cp, ++glyphCount)
            {
                FT_Load_Char( face, cp, FT_LOAD_RENDER );

                max_height = std::max<FT_Pos>(2 * (face->glyph->bitmap.rows << 6) - face->glyph->metrics.horiBearingY, max_height);
                mTtfMaxBearingY = std::max(int(face->glyph->metrics.horiBearingY), mTtfMaxBearingY);
                max_width = std::max<FT_Pos>((face->glyph->advance.x >> 6) + (face->glyph->metrics.horiBearingX >> 6), max_width);
            }

        }

        uint char_spacer = 1;

        // Now work out how big our texture needs to be
        size_t rawSize = (max_width + char_spacer) * ((max_height >> 6) + char_spacer) * glyphCount;

        uint32 tex_side = static_cast<uint32>(Math::Sqrt((Real)rawSize));
        // Now round up to nearest power of two
        uint32 roundUpSize = Bitwise::firstPO2From(tex_side);

        // Would we benefit from using a non-square texture (2X width)
        uint32 finalWidth, finalHeight;
        if (roundUpSize * roundUpSize * 0.5 >= rawSize)
        {
            finalHeight = static_cast<uint32>(roundUpSize * 0.5);
        }
        else
        {
            finalHeight = roundUpSize;
        }
        finalWidth = roundUpSize;

        Real textureAspect = (Real)finalWidth / (Real)finalHeight;

        Image img(PF_BYTE_LA, finalWidth, finalHeight);
        // Reset content (transparent)
        img.setTo(ColourValue::ZERO);

        size_t l = 0, m = 0;
        for (const CodePointRange& range : mCodePointRangeList)
        {
            for(CodePoint cp = range.first; cp <= range.second; ++cp )
            {
                FT_Error ftResult;

                // Load & render glyph
                ftResult = FT_Load_Char( face, cp, FT_LOAD_RENDER );
                if (ftResult)
                {
                    // problem loading this glyph, continue
                    LogManager::getSingleton().logError(StringUtil::format(
                        "Freetype could not load charcode %u in font %s", cp, mSource.c_str()));
                    continue;
                }

                if (!face->glyph->bitmap.buffer)
                {
                    // Yuck, FT didn't detect this but generated a null pointer!
                    LogManager::getSingleton().logWarning(StringUtil::format(
                        "Freetype did not find charcode %u in font %s", cp, mSource.c_str()));
                    continue;
                }

                uint advance = face->glyph->advance.x >> 6;

                // If at end of row
                if( finalWidth - 1 < l + ( advance ) )
                {
                    m += ( max_height >> 6 ) + char_spacer;
                    l = 0;
                }

                FT_Pos y_bearing = ( mTtfMaxBearingY >> 6 ) - ( face->glyph->metrics.horiBearingY >> 6 );
                FT_Pos x_bearing = face->glyph->metrics.horiBearingX >> 6;

                // x_bearing might be negative
                uint x_offset = std::max(0, int(x_bearing));
                // width might be larger than advance
                uint start = x_offset - x_bearing; // case x_bearing is negative
                uint width = std::min(face->glyph->bitmap.width - start, advance - x_offset);

                for(unsigned int j = 0; j < face->glyph->bitmap.rows; j++ )
                {
                    uchar* pSrc = face->glyph->bitmap.buffer + j * face->glyph->bitmap.pitch + start;
                    size_t row = j + m + y_bearing;
                    uchar* pDest = img.getData(l + x_offset, row);
                    for(unsigned int k = 0; k < (width); k++ )
                    {
                        if (mAntialiasColour)
                        {
                            // Use the same greyscale pixel for all components RGBA
                            *pDest++= *pSrc;
                        }
                        else
                        {
                            // Always white whether 'on' or 'off' pixel, since alpha
                            // will turn off
                            *pDest++= 0xFF;
                        }
                        // Always use the greyscale value for alpha
                        *pDest++= *pSrc++;
                    }
                }

                this->setGlyphTexCoords(cp,
                    (Real)l / (Real)finalWidth,  // u1
                    (Real)m / (Real)finalHeight,  // v1
                    (Real)( l + advance ) / (Real)finalWidth, // u2
                    ( m + ( max_height >> 6 ) ) / (Real)finalHeight, // v2
                    textureAspect
                    );

                // Advance a column
                l += (advance + char_spacer);
            }
        }

        Texture* tex = static_cast<Texture*>(res);
        // Call internal _loadImages, not loadImage since that's external and 
        // will determine load status etc again, and this is a manual loader inside load()
        tex->_loadImages({&img});

        FT_Done_FreeType(ftLibrary);
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String Font::CmdType::doGet(const void* target) const
    {
        const Font* f = static_cast<const Font*>(target);
        if (f->getType() == FT_TRUETYPE)
        {
            return "truetype";
        }
        else
        {
            return "image";
        }
    }
    void Font::CmdType::doSet(void* target, const String& val)
    {
        Font* f = static_cast<Font*>(target);
        if (val == "truetype")
        {
            f->setType(FT_TRUETYPE);
        }
        else
        {
            f->setType(FT_IMAGE);
        }
    }
    //-----------------------------------------------------------------------
    String Font::CmdSource::doGet(const void* target) const
    {
        const Font* f = static_cast<const Font*>(target);
        return f->getSource();
    }
    void Font::CmdSource::doSet(void* target, const String& val)
    {
        Font* f = static_cast<Font*>(target);
        f->setSource(val);
    }
    //-----------------------------------------------------------------------
    String Font::CmdCharSpacer::doGet(const void* target) const
    {
        return "1";
    }
    void Font::CmdCharSpacer::doSet(void* target, const String& val) {}
    //-----------------------------------------------------------------------
    String Font::CmdSize::doGet(const void* target) const
    {
        const Font* f = static_cast<const Font*>(target);
        return StringConverter::toString(f->getTrueTypeSize());
    }
    void Font::CmdSize::doSet(void* target, const String& val)
    {
        Font* f = static_cast<Font*>(target);
        f->setTrueTypeSize(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String Font::CmdResolution::doGet(const void* target) const
    {
        const Font* f = static_cast<const Font*>(target);
        return StringConverter::toString(f->getTrueTypeResolution());
    }
    void Font::CmdResolution::doSet(void* target, const String& val)
    {
        Font* f = static_cast<Font*>(target);
        f->setTrueTypeResolution(StringConverter::parseUnsignedInt(val));
    }
    //-----------------------------------------------------------------------
    String Font::CmdCodePoints::doGet(const void* target) const
    {
        const Font* f = static_cast<const Font*>(target);
        const CodePointRangeList& rangeList = f->getCodePointRangeList();
        StringStream str;
        for (CodePointRangeList::const_iterator i = rangeList.begin(); i != rangeList.end(); ++i)
        {
            str << i->first << "-" << i->second << " ";
        }
        return str.str();
    }
    void Font::CmdCodePoints::doSet(void* target, const String& val)
    {
        // Format is "code_points start1-end1 start2-end2"
        Font* f = static_cast<Font*>(target);

        StringVector vec = StringUtil::split(val, " \t");
        for (StringVector::iterator i = vec.begin(); i != vec.end(); ++i)
        {
            String& item = *i;
            StringVector itemVec = StringUtil::split(item, "-");
            if (itemVec.size() == 2)
            {
                f->addCodePointRange(CodePointRange(
                    StringConverter::parseUnsignedInt(itemVec[0]),
                    StringConverter::parseUnsignedInt(itemVec[1])));
            }
        }
    }


}
