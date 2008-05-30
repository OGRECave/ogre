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
#ifndef __ANIMABLE_H__
#define __ANIMABLE_H__

#include "OgrePrerequisites.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"
#include "OgreSharedPtr.h"
#include "OgreStringVector.h"
#include "OgreException.h"
#include "OgreAny.h"

namespace Ogre {

	/** Defines an object property which is animable, i.e. may be keyframed.
	@remarks
		Animable properties are those which can be altered over time by a 
		predefined keyframe sequence. They may be set directly, or they may
		be modified from their existing state (common if multiple animations
		are expected to apply at once). Implementors of this interface are
		expected to override the 'setValue', 'setCurrentStateAsBaseValue' and 
		'applyDeltaValue' methods appropriate to the type in question, and to 
		initialise the type.
	@par
		AnimableValue instances are accessible through any class which extends
		AnimableObject in order to expose it's animable properties.
	@note
		This class is an instance of the Adapter pattern, since it generalises
		access to a particular property. Whilst it could have been templated
		such that the type which was being referenced was compiled in, this would
		make it more difficult to aggregated generically, and since animations
		are often comprised of multiple properties it helps to be able to deal
		with all values through a single class.
	*/
	class _OgreExport AnimableValue : public AnimableAlloc
	{
	public:
		/// The type of the value being animated
		enum ValueType
		{
			INT,
			REAL,
			VECTOR2,
			VECTOR3,
			VECTOR4,
			QUATERNION,
			COLOUR,
			RADIAN,
			DEGREE
		};
	protected:
		/// Value type
		ValueType mType;

		/// Base value data
		union
		{
			int mBaseValueInt;
			Real mBaseValueReal[4];
		};

		/// Internal method to set a value as base
		virtual void setAsBaseValue(int val) { mBaseValueInt = val; }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(Real val) { mBaseValueReal[0] = val; }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Vector2& val) 
		{ memcpy(mBaseValueReal, val.ptr(), sizeof(Real)*2); }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Vector3& val) 
		{ memcpy(mBaseValueReal, val.ptr(), sizeof(Real)*3); }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Vector4& val) 
		{ memcpy(mBaseValueReal, val.ptr(), sizeof(Real)*4); }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Quaternion& val) 
		{ memcpy(mBaseValueReal, val.ptr(), sizeof(Real)*4); }
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Any& val);
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const ColourValue& val)
		{ 
			mBaseValueReal[0] = val.r;
			mBaseValueReal[1] = val.g;
			mBaseValueReal[2] = val.b;
			mBaseValueReal[3] = val.a;
		}
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Radian& val)
		{ 
			mBaseValueReal[0] = val.valueRadians();
		}
		/// Internal method to set a value as base
		virtual void setAsBaseValue(const Degree& val)
		{ 
			mBaseValueReal[0] = val.valueRadians();
		}


	public:
		AnimableValue(ValueType t) : mType(t) {}
		virtual ~AnimableValue() {}

		/// Gets the value type of this animable value
		ValueType getType(void) const { return mType; }

		/// Sets the current state as the 'base' value; used for delta animation
		virtual void setCurrentStateAsBaseValue(void) = 0;

		/// Set value 
		virtual void setValue(int) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(Real) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Vector2&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Vector3&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Vector4&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Quaternion&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const ColourValue&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Radian&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Degree&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void setValue(const Any& val);

		// reset to base value
		virtual void resetToBaseValue(void);

		/// Apply delta value
		virtual void applyDeltaValue(int) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Set value 
		virtual void applyDeltaValue(Real) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Vector2&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Vector3&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Vector4&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Quaternion&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const ColourValue&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Degree&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Radian&) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "", "");
		}
		/// Apply delta value 
		virtual void applyDeltaValue(const Any& val);


	};

	typedef SharedPtr<AnimableValue> AnimableValuePtr;



	/** Defines an interface to classes which have one or more AnimableValue
		instances to expose.
	*/
	class _OgreExport AnimableObject
	{
	protected:
		typedef std::map<String, StringVector> AnimableDictionaryMap;
		/// Static map of class name to list of animable value names
		static AnimableDictionaryMap msAnimableDictionary;
		/** Get the name of the animable dictionary for this class.
		@remarks
			Subclasses must override this if they want to support animation of
			their values.
		*/
		virtual const String& getAnimableDictionaryName(void) const 
		{ return StringUtil::BLANK; }
        /** Internal method for creating a dictionary of animable value names 
			for the class, if it does not already exist.
        */
        void createAnimableDictionary(void) const
        {
            if (msAnimableDictionary.find(getAnimableDictionaryName()) 
				== msAnimableDictionary.end())
            {
				StringVector vec;
				initialiseAnimableDictionary(vec);
				msAnimableDictionary[getAnimableDictionaryName()] = vec;
            }

        }
	
		/// Get an updateable reference to animable value list
		StringVector& _getAnimableValueNames(void)
		{
			AnimableDictionaryMap::iterator i = 
				msAnimableDictionary.find(getAnimableDictionaryName());
			if (i != msAnimableDictionary.end())
			{
				return i->second;
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Animable value list not found for " + getAnimableDictionaryName(), 
					"AnimableObject::getAnimableValueNames");
			}

		}

		/** Internal method for initialising dictionary; should be implemented by 
			subclasses wanting to expose animable parameters.
		*/
		virtual void initialiseAnimableDictionary(StringVector&) const {}


	public:
		AnimableObject() {}
		virtual ~AnimableObject() {}

		/** Gets a list of animable value names for this object. */
		const StringVector& getAnimableValueNames(void) const
		{
			createAnimableDictionary();

			AnimableDictionaryMap::iterator i = 
				msAnimableDictionary.find(getAnimableDictionaryName());
			if (i != msAnimableDictionary.end())
			{
				return i->second;
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Animable value list not found for " + getAnimableDictionaryName(), 
					"AnimableObject::getAnimableValueNames");
			}

		}

		/** Create a reference-counted AnimableValuePtr for the named value.
		@remarks
			You can use the returned object to animate a value on this object,
			using AnimationTrack. Subclasses must override this if they wish 
			to support animation of their values.
		*/
		virtual AnimableValuePtr createAnimableValue(const String& valueName)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"No animable value named '" + valueName + "' present.", 
				"AnimableObject::createAnimableValue");
		}



	};


}
#endif

