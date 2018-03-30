/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#include "OgreHlmsPropertyMap.h"
#include "OgreHlmsPropertyHelper.h"
#include "OgreHlmsShaderCommon.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------------
	PropertyMap::PropertyMap() : mHash(0)
    {
    }
    //-----------------------------------------------------------------------------------
	PropertyMap::~PropertyMap()
    {
    }
    //-----------------------------------------------------------------------------------
	void PropertyMap::setCommonProperties(void)
    {
        uint16 numWorldTransforms = 2;

		setProperty(DefaultProperties::Skeleton, numWorldTransforms > 1);
		setProperty(DefaultProperties::UvCount, 2);
        setProperty( "true", 1 );
        setProperty( "false", 0 );

		setProperty(DefaultProperties::DualParaboloidMapping, 0);

		setProperty(DefaultProperties::Normal, 1);

		setProperty(DefaultProperties::UvCount0, 2);
		setProperty(DefaultProperties::UvCount1, 4);
		setProperty(DefaultProperties::BonesPerVertex, 4);

		setProperty(DefaultProperties::NumShadowMaps, 3);
		setProperty(DefaultProperties::PssmSplits, 3);
		setProperty(DefaultProperties::ShadowCaster, 0);

		setProperty(DefaultProperties::LightsDirectional, 1);
		setProperty(DefaultProperties::LightsPoint, 2);
		setProperty(DefaultProperties::LightsSpot, 3);
    }
    //-----------------------------------------------------------------------------------
	void PropertyMap::setProperty(IdString key, int32 value)
    {
        Property p( key, value );
		std::vector<Property>::iterator it = std::lower_bound(mProperties.begin(), mProperties.end(), p, orderPropertyByIdString);
		if (it == mProperties.end() || it->keyName != p.keyName)
			mProperties.insert(it, p);
        else
            *it = p;

		mHash = 0;
    }
	//-----------------------------------------------------------------------------------
	bool PropertyMap::hasProperty(IdString key)
	{
		Property p(key, 0);
		std::vector<Property>::iterator it = std::lower_bound(mProperties.begin(), mProperties.end(), p, orderPropertyByIdString);
		return it != mProperties.end() && it->keyName == p.keyName;
	}
    //-----------------------------------------------------------------------------------
	int32 PropertyMap::getProperty(IdString key, int32 defaultVal)
    {
        Property p( key, 0 );
		std::vector<Property>::iterator it = std::lower_bound(mProperties.begin(), mProperties.end(), p, orderPropertyByIdString);
		if (it != mProperties.end() && it->keyName == p.keyName)
            defaultVal = it->value;

        return defaultVal;
    }
	//-----------------------------------------------------------------------------------
	void PropertyMap::removeProperty(IdString key)
	{
		Property p(key, 0);
		std::vector<Property>::iterator it = std::lower_bound(mProperties.begin(), mProperties.end(), p, orderPropertyByIdString);
		if (it != mProperties.end() && it->keyName == p.keyName)
			mProperties.erase(it);
	}
	//-----------------------------------------------------------------------------------
	uint32 PropertyMap::getHash()
	{
		if (mProperties.size() == 0)
		{
			mHash = 0;
		}
		else
		{
			if (mHash == 0)
			{
				std::vector<int32> buffer(mProperties.size() * 2);
				int j = 0;
				for (size_t i = 0; i < mProperties.size(); i++)
				{
					buffer[j++] = int32(mProperties[i].keyName.mHash);
					buffer[j++] = mProperties[i].value;
				}

				mHash = calcHash(&buffer[0], mProperties.size() * 2 * sizeof(int32));
			}
		}

		return mHash;
	}
	//-----------------------------------------------------------------------------------
}
