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
	/** \addtogroup Component
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport PropertyMap
	{
	public:

		struct Property
		{
			IdString    keyName;
			Ogre::int32 value;

			Property(IdString _keyName, Ogre::int32 _value) :
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

		void setProperty(IdString key, Ogre::int32 value);
		bool hasProperty(IdString key);
		Ogre::int32 getProperty(IdString key, Ogre::int32 defaultVal = 0);
		void removeProperty(IdString key);

		Ogre::uint32 getHash();

		/** Finds the parameter with key 'key' in the given 'paramVec'. If found, outputs
			the value to 'inOut', otherwise leaves 'inOut' as is.
			@return
			True if the key was found (inOut was modified), false otherwise
			@remarks
			Assumes paramVec is sorted by key.
			*/
		//static bool findParamInVec(const std::vector<Property> &paramVec, IdString key, String &inOut);

		/// For debugging stuff. I.e. the Command line uses it for testing manually set properties
		//void _setProperty(IdString key, int32 value)      { setProperty(key, value); }

	protected:

		std::vector<Property> mProperties;
		Ogre::uint32 mHash;

		static bool orderPropertyByIdString(const PropertyMap::Property &_left, const PropertyMap::Property &_right)
		{
			return _left.keyName < _right.keyName;
		}
	};
}

