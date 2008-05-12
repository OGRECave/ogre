
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreStableHeaders.h"

#include "OgrePass.h"
#include "OgreFixedFuncState.h"
#include "OgreException.h"
#include "OgreGpuProgramUsage.h"
#include "OgreTextureUnitState.h"
#include "OgreStringConverter.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
	GeneralFixedFuncState::GeneralFixedFuncState()
    {
		ZeroMemory(this, sizeof(GeneralFixedFuncState));
		mAlphaRejectFunc = CMPF_ALWAYS_PASS;
		mLightingEnabled = true;
		mShadeOptions = SO_GOURAUD;
		mFogMode = FOG_NONE;
		mNormaliseNormals = false;
    }
    //-----------------------------------------------------------------------------
    GeneralFixedFuncState::~GeneralFixedFuncState()
    {

    }
	//-----------------------------------------------------------------------
	void GeneralFixedFuncState::setAlphaRejectFunction(CompareFunction func)
	{
		mAlphaRejectFunc = func;
	}
    //-----------------------------------------------------------------------
    void GeneralFixedFuncState::setLightingEnabled(bool enabled)
    {
	    mLightingEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    bool GeneralFixedFuncState::getLightingEnabled(void) const
    {
	    return mLightingEnabled;
    }
    //-----------------------------------------------------------------------
    void GeneralFixedFuncState::setShadingMode(ShadeOptions mode)
    {
	    mShadeOptions = mode;
    }
    //-----------------------------------------------------------------------
    ShadeOptions GeneralFixedFuncState::getShadingMode(void) const
    {
	    return mShadeOptions;
    }
    //-----------------------------------------------------------------------
    void GeneralFixedFuncState::setFogMode(FogMode mode)
    {
	    mFogMode = mode;
    }
    //-----------------------------------------------------------------------
    FogMode GeneralFixedFuncState::getFogMode(void) const
    {
	    return mFogMode;
    }
	//-----------------------------------------------------------------------
	void GeneralFixedFuncState::setNormaliseNormals( bool normalise )
	{
		mNormaliseNormals = normalise;
	}
	//-----------------------------------------------------------------------
	bool GeneralFixedFuncState::getNormaliseNormals( void ) const
	{
		return mNormaliseNormals;
	}
	//-----------------------------------------------------------------------
	CompareFunction GeneralFixedFuncState::getAlphaRejectFunction( void ) const
	{
		return mAlphaRejectFunc;
	}
	//-----------------------------------------------------------------------
	const uint8 GeneralFixedFuncState::getLightTypeCount( const Light::LightTypes type ) const
	{
		return mLightFromTypeCount[type];
	}
	//-----------------------------------------------------------------------
	void GeneralFixedFuncState::setLightTypeCount( const Light::LightTypes type, const uint8 val )
	{
		mLightFromTypeCount[type] = val;
	}
	//-----------------------------------------------------------------------
	const uint8 GeneralFixedFuncState::getTotalNumberOfLights() const
	{
		uint8 res = 0;
		for( uint8 i = 0 ; i < LIGHT_TYPES_COUNT ; i++ )
		{
			res += mLightFromTypeCount[i];
		}

		return res;
	}	
	//-----------------------------------------------------------------------
	void GeneralFixedFuncState::resetLightTypeCounts()
	{
		for( uint8 i = 0 ; i < LIGHT_TYPES_COUNT ; i++ )
		{
			mLightFromTypeCount[i] = 0;
		}
	}
	//-----------------------------------------------------------------------
	void GeneralFixedFuncState::addOnetoLightTypeCount( const Light::LightTypes type )
	{
		mLightFromTypeCount[type]++;
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	FixedFuncState::FixedFuncState()
	{

	}
	//-----------------------------------------------------------------------
	FixedFuncState::~FixedFuncState()
	{

	}
	//-----------------------------------------------------------------------
	const TextureLayerStateList & FixedFuncState::getTextureLayerStateList() const
	{
		return mTextureLayerStateList;
	}
	//-----------------------------------------------------------------------
	void FixedFuncState::setTextureLayerStateList( const TextureLayerStateList & val )
	{
		mTextureLayerStateList = val;
	}
	//-----------------------------------------------------------------------
	GeneralFixedFuncState & FixedFuncState::getGeneralFixedFuncState()
	{
		return mGeneralFixedFuncState;
	}
	//-----------------------------------------------------------------------
	const bool FixedFuncState::operator<( const FixedFuncState & other ) const
	{
	

		// General
		int memcmpRes = memcmp(&this->mGeneralFixedFuncState, &other.mGeneralFixedFuncState, sizeof(GeneralFixedFuncState))  ;
		if (memcmpRes)
		{
			return memcmpRes > 0;
		}

		// mTextureLayerStateList
		if (mTextureLayerStateList.size() != other.mTextureLayerStateList.size())
		{
			return mTextureLayerStateList.size() > other.mTextureLayerStateList.size();
		}

		if (mTextureLayerStateList.size() > 0)
		{
			int memcmpRes =  memcmp(&mTextureLayerStateList[0], &other.mTextureLayerStateList[0], mTextureLayerStateList.size() * sizeof(TextureLayerState));
			if (memcmpRes)
			{
				return memcmpRes > 0;
			}
		}

		return false;

	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	const VertexBufferElementList & VertexBufferDeclaration::getVertexBufferElementList() const
	{
		return mVertexBufferElementList;
	}
	//-----------------------------------------------------------------------
	void VertexBufferDeclaration::setVertexBufferElementList(const VertexBufferElementList & val )
	{
		mVertexBufferElementList = val;
	}
	//-----------------------------------------------------------------------
	const bool VertexBufferDeclaration::operator<( const VertexBufferDeclaration & other ) const
	{
		if(mVertexBufferElementList.size() != other.mVertexBufferElementList.size())
		{
			return  (mVertexBufferElementList.size() > other.mVertexBufferElementList.size());
		}

		if (mVertexBufferElementList.size() > 0)
		{
			return memcmp(&mVertexBufferElementList[0], &other.mVertexBufferElementList[0], mVertexBufferElementList.size() * sizeof(VertexBufferElement)) > 0 ;
		}
		else
		{
			return false;
		}
	}
	//-----------------------------------------------------------------------
	bool VertexBufferDeclaration::hasColor() const 
	{
		return countVertexElementSemantic(VES_DIFFUSE) > 0;
	}
	//-----------------------------------------------------------------------
	uint8 VertexBufferDeclaration::getTexcoordCount() const
	{
		return (uint8)countVertexElementSemantic(VES_TEXTURE_COORDINATES);
	}
	//-----------------------------------------------------------------------
	unsigned short VertexBufferDeclaration::numberOfTexcoord() const
	{
		return countVertexElementSemantic(VES_TEXTURE_COORDINATES);
	}
	//-----------------------------------------------------------------------
	unsigned short VertexBufferDeclaration::countVertexElementSemantic( VertexElementSemantic semantic ) const
	{
		unsigned short res = 0;
		for (unsigned short i = 0 ; i < mVertexBufferElementList.size() ; i++)
		{
			if(mVertexBufferElementList[i].getVertexElementSemantic() == semantic)
			{
				res++;
			}
		}

		return res;
	}
	//-----------------------------------------------------------------------
	VertexBufferDeclaration::VertexBufferDeclaration()
	{
		ZeroMemory(this, sizeof(VertexBufferDeclaration));
	}
	//-----------------------------------------------------------------------
	VertexBufferDeclaration::~VertexBufferDeclaration()
	{

	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	TextureType TextureLayerState::getTextureType() const
	{
		return mTextureType;
	}
	//-----------------------------------------------------------------------
	void TextureLayerState::setTextureType( TextureType val )
	{
		mTextureType = val;
	}
	//-----------------------------------------------------------------------
	TexCoordCalcMethod TextureLayerState::getTexCoordCalcMethod() const
	{
		return mTexCoordCalcMethod;
	}
	//-----------------------------------------------------------------------
	void TextureLayerState::setTexCoordCalcMethod( TexCoordCalcMethod val )
	{
		mTexCoordCalcMethod = val;
	}
	//-----------------------------------------------------------------------
	LayerBlendModeEx TextureLayerState::getLayerBlendModeEx() const
	{
		return mLayerBlendOperationEx;
	}
	//-----------------------------------------------------------------------
	void TextureLayerState::setLayerBlendModeEx( LayerBlendModeEx val )
	{
		mLayerBlendOperationEx = val;
	}
	//-----------------------------------------------------------------------
	uint8 TextureLayerState::getCoordIndex() const
	{
		return mCoordIndex;
	}
	//-----------------------------------------------------------------------
	void TextureLayerState::setCoordIndex( uint8 val )
	{
		mCoordIndex = val;
	}
	//-----------------------------------------------------------------------
	TextureLayerState::TextureLayerState()
	{
		ZeroMemory(this, sizeof(TextureLayerState));
	}
	//-----------------------------------------------------------------------
	TextureLayerState::~TextureLayerState()
	{

	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	VertexElementSemantic VertexBufferElement::getVertexElementSemantic() const
	{
		return mVertexElementSemantic;
	}
	//-----------------------------------------------------------------------
	void VertexBufferElement::setVertexElementSemantic( VertexElementSemantic val )
	{
		mVertexElementSemantic = val;
	}
	//-----------------------------------------------------------------------
	VertexElementType VertexBufferElement::getVertexElementType() const
	{
		return mVertexElementType;
	}
	//-----------------------------------------------------------------------
	void VertexBufferElement::setVertexElementType( VertexElementType val )
	{
		mVertexElementType = val;
	}
	//-----------------------------------------------------------------------
	unsigned short VertexBufferElement::getVertexElementIndex() const
	{
		return mVertexElementIndex;
	}
	//-----------------------------------------------------------------------
	void VertexBufferElement::setVertexElementIndex( unsigned short val )
	{
		mVertexElementIndex = val;
	}
	//-----------------------------------------------------------------------
}
