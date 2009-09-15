/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreStableHeaders.h"

#include "OgreFont.h"
#include "OgreMaterialManager.h"
#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreResourceGroupManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRenderWindow.h"
#include "OgreException.h"
#include "OgreBlendMode.h"
#include "OgreTextureUnitState.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterial.h"
#include "OgreBitwise.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H



namespace Ogre
{
    //---------------------------------------------------------------------
	Font::CmdType Font::msTypeCmd;
	Font::CmdSource Font::msSourceCmd;
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
    void Font::loadImpl()
    {
        // Create a new material
        mpMaterial =  MaterialManager::getSingleton().create(
			"Fonts/" + mName,  mGroup);

		if (mpMaterial.isNull())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                "Error creating new material!", "Font::load" );
        }

        TextureUnitState *texLayer;
        bool blendByAlpha = true;
        if (mType == FT_TRUETYPE)
        {
            createTextureFromFont();
            texLayer = mpMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
            // Always blend by alpha
            blendByAlpha = true;
        }
        else
        {
			// Manually load since we need to load to get alpha
			mTexture = TextureManager::getSingleton().load(mSource, mGroup, TEX_TYPE_2D, 0);
            blendByAlpha = mTexture->hasAlpha();
            texLayer = mpMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(mSource);
        }

		// Make sure material is aware of colour per vertex.
		mpMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(TVC_DIFFUSE);
        // Clamp to avoid fuzzy edges
        texLayer->setTextureAddressingMode( TextureUnitState::TAM_CLAMP );
		// Allow min/mag filter, but no mip
		texLayer->setTextureFiltering(FO_LINEAR, FO_LINEAR, FO_NONE);


        // Set up blending
        if (blendByAlpha)
        {
            mpMaterial->setSceneBlending( SBT_TRANSPARENT_ALPHA );
        }
        else
        {
            // Use add if no alpha (assume black background)
            mpMaterial->setSceneBlending(SBT_ADD);
        }
    }
    //---------------------------------------------------------------------
    void Font::unloadImpl()
    {
		if (!mpMaterial.isNull())
		{
			MaterialManager::getSingleton().remove(mpMaterial->getHandle());
			mpMaterial.setNull();
		}

		if (!mTexture.isNull())
		{
			TextureManager::getSingleton().remove(mTexture->getHandle());
			mTexture.setNull();
		}
    }
    //---------------------------------------------------------------------
    void Font::createTextureFromFont(void)
    {

		// Just create the texture here, and point it at ourselves for when
		// it wants to (re)load for real
		String texName = mName + "Texture";
		// Create, setting isManual to true and passing self as loader
		mTexture = TextureManager::getSingleton().create(
			texName, mGroup, true, this);
		mTexture->setTextureType(TEX_TYPE_2D);
		mTexture->setNumMipmaps(0);
		mTexture->load();

		TextureUnitState* t = mpMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( texName );
		// Allow min/mag filter, but no mip
		t->setTextureFiltering(FO_LINEAR, FO_LINEAR, FO_NONE);

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
        // Add a gap between letters vert and horz
        // prevents nasty artefacts when letters are too close together
        uint char_spacer = 5;

        // Locate ttf file, load it pre-buffered into memory by wrapping the
		// original DataStream in a MemoryDataStream
		DataStreamPtr dataStreamPtr =
			ResourceGroupManager::getSingleton().openResource(
				mSource, mGroup, true, this);
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

        int max_height = 0, max_width = 0;

		// Backwards compatibility - if codepoints not supplied, assume 33-166
		if (mCodePointRangeList.empty())
		{
			mCodePointRangeList.push_back(CodePointRange(33, 166));
		}

		// Calculate maximum width, height and bearing
		size_t glyphCount = 0;
		for (CodePointRangeList::const_iterator r = mCodePointRangeList.begin();
			r != mCodePointRangeList.end(); ++r)
		{
			const CodePointRange& range = *r;
			for(CodePoint cp = range.first; cp <= range.second; ++cp, ++glyphCount)
			{
				FT_Load_Char( face, cp, FT_LOAD_RENDER );

				if( ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY ) > max_height )
					max_height = ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY );
				if( face->glyph->metrics.horiBearingY > mTtfMaxBearingY )
					mTtfMaxBearingY = face->glyph->metrics.horiBearingY;

				if( (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 ) > max_width)
					max_width = (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 );
			}

		}

		// Now work out how big our texture needs to be
		size_t rawSize = (max_width + char_spacer) *
							((max_height >> 6) + char_spacer) * glyphCount;

		uint32 tex_side = static_cast<uint32>(Math::Sqrt(rawSize));
		// just in case the size might chop a glyph in half, add another glyph width/height
		tex_side += std::max(max_width, (max_height>>6));
		// Now round up to nearest power of two
		uint32 roundUpSize = Bitwise::firstPO2From(tex_side);

		// Would we benefit from using a non-square texture (2X width(
		size_t finalWidth, finalHeight;
		if (roundUpSize*roundUpSize*0.5 >= rawSize)
		{
			finalHeight = static_cast<size_t>(roundUpSize * 0.5);
		}
		else
		{
			finalHeight = roundUpSize;
		}
		finalWidth = roundUpSize;

		Real textureAspect = finalWidth / finalHeight;

		const size_t pixel_bytes = 2;
		size_t data_width = finalWidth * pixel_bytes;
		size_t data_size = finalWidth * finalHeight * pixel_bytes;

		LogManager::getSingleton().logMessage("Font " + mName + "using texture size " +
			StringConverter::toString(finalWidth) + "x" + StringConverter::toString(finalHeight));

        uchar* imageData = OGRE_ALLOC_T(uchar, data_size, MEMCATEGORY_GENERAL);
		// Reset content (White, transparent)
        for (size_t i = 0; i < data_size; i += pixel_bytes)
        {
            imageData[i + 0] = 0xFF; // luminance
            imageData[i + 1] = 0x00; // alpha
        }

        size_t l = 0, m = 0;
		for (CodePointRangeList::const_iterator r = mCodePointRangeList.begin();
			r != mCodePointRangeList.end(); ++r)
		{
			const CodePointRange& range = *r;
			for(CodePoint cp = range.first; cp <= range.second; ++cp )
			{
				FT_Error ftResult;

				// Load & render glyph
				ftResult = FT_Load_Char( face, cp, FT_LOAD_RENDER );
				if (ftResult)
				{
					// problem loading this glyph, continue
					LogManager::getSingleton().logMessage("Info: cannot load character " +
						StringConverter::toString(cp) + " in font " + mName);
					continue;
				}

				FT_Int advance = (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 );

				unsigned char* buffer = face->glyph->bitmap.buffer;

				if (!buffer)
				{
					// Yuck, FT didn't detect this but generated a null pointer!
					LogManager::getSingleton().logMessage("Info: Freetype returned null for character " +
						StringConverter::toString(cp) + " in font " + mName);
					continue;
				}

				int y_bearnig = ( mTtfMaxBearingY >> 6 ) - ( face->glyph->metrics.horiBearingY >> 6 );

				for(int j = 0; j < face->glyph->bitmap.rows; j++ )
				{
					size_t row = j + m + y_bearnig;
					uchar* pDest = &imageData[(row * data_width) + l * pixel_bytes];
					for(int k = 0; k < face->glyph->bitmap.width; k++ )
					{
						if (mAntialiasColour)
						{
							// Use the same greyscale pixel for all components RGBA
							*pDest++= *buffer;
						}
						else
						{
							// Always white whether 'on' or 'off' pixel, since alpha
							// will turn off
							*pDest++= 0xFF;
						}
						// Always use the greyscale value for alpha
						*pDest++= *buffer++; 
					}
				}

				this->setGlyphTexCoords(cp,
					(Real)l / (Real)finalWidth,  // u1
					(Real)m / (Real)finalHeight,  // v1
					(Real)( l + ( face->glyph->advance.x >> 6 ) ) / (Real)finalWidth, // u2
					( m + ( max_height >> 6 ) ) / (Real)finalHeight, // v2
					textureAspect
					);

				// Advance a column
				l += (advance + char_spacer);

				// If at end of row
				if( finalWidth - 1 < l + ( advance ) )
				{
					m += ( max_height >> 6 ) + char_spacer;
					l = 0;
				}
			}
		}

        DataStreamPtr memStream(
			OGRE_NEW MemoryDataStream(imageData, data_size, true));

        Image img;
		img.loadRawData( memStream, finalWidth, finalHeight, PF_BYTE_LA );

		Texture* tex = static_cast<Texture*>(res);
		// Call internal _loadImages, not loadImage since that's external and 
		// will determine load status etc again, and this is a manual loader inside load()
		ConstImagePtrList imagePtrs;
		imagePtrs.push_back(&img);
		tex->_loadImages( imagePtrs );


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
		StringUtil::StrStreamType str;
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
					StringConverter::parseLong(itemVec[0]), 
					StringConverter::parseLong(itemVec[1])));
			}
		}
	}


}
