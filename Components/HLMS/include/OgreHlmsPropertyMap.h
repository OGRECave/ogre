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

#pragma once


#include "Ogre.h"			
#include "OgreHlmsPrerequisites.h"
#include "OgreIdString.h"

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport PropertyMap : public PassAlloc
	{
	public:

		struct Property
		{
			IdString    keyName;
			int32 value;

			Property(IdString _keyName, int32 _value) :
				keyName(_keyName), value(_value) {}

			bool operator == (const Property &_r) const
			{
				return this->keyName == _r.keyName && this->value == _r.value;
			}
		};

		PropertyMap();
		virtual ~PropertyMap();

		/** Inserts common properties about the current Renderable,
			such as hlms_skeleton hlms_uv_count, etc
			*/
		void setCommonProperties();

		void setProperty(IdString key, int32 value);
		bool hasProperty(IdString key);
		int32 getProperty(IdString key, int32 defaultVal = 0);
		void removeProperty(IdString key);

		uint32 getHash();

	protected:

		std::vector<Property> mProperties;
		uint32 mHash;

		static bool orderPropertyByIdString(const PropertyMap::Property &_left, const PropertyMap::Property &_right)
		{
			return _left.keyName < _right.keyName;
		}
	};
}

