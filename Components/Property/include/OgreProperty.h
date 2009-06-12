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
#ifndef __OGRE_PROPERTY_H__
#define __OGRE_PROPERTY_H__

#include "OgrePropertyPrerequisites.h"
#include "OgreAny.h"
#include "OgreIteratorWrappers.h"
#include "OgreString.h"
#include "OgreException.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreColourValue.h"
#include "OgreQuaternion.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
/** \addtogroup Optional Components
*  @{
*/
/** \addtogroup Property
*  @{
*/

/** @file
	OGRE's property system allows you to associate values of arbitrary type with
	names, and have those values exposed via a self-describing interface. Unlike
	Ogre::StringInterface, the values are passed as their native types without
	needing conversion to or from strings; they are simply wrapped in an Ogre::Any
	and casts are performed to access them.
	@par
	Property values are actually not stored in this system; instead the property
	definitions reference getter & setter methods which provide the 'backing' for
	the property. This means you can directly expose features of your classes as properties
	without any duplication.
	@par
	There are two aspects to exposing a property on your class. One is exposing 
	the definition of the property (PropertyDef), which should be shared between 
	all instances and probably stored in a static PropertyDefMap somewhere. The second
	is the instance 'wiring' which ensures that a call to set a property calls 
	a method on this particular instance of the class; this is formed by a number of
	Property instances, contained in a PropertySet. Each Property has an explicit
	binding to getter and setter instance methods.
	@par
	So, here's an example of setting up properties on an instance:

	@code
	// Make sure the property definition is created 
	// propertyDefs is a variable of type PropertyDefMap, shared between instances
	PropertyDefMap::iterator defi = propertyDefs.find("name");
	if (defi == propertyDefs.end())
	{
		defi = propertyDefs.insert(PropertyDefMap::value_type("name", 
			PropertyDef("name", 
				"The name of the object.", PROP_STRING))).first;
	}
	// This has established the property definition, and its description.
	// Now, we need to 'wire' a property instance for this object instance
	// We assume the class is called 'Foo' and the instance is pointed to by a variable called 'inst'
	// 'props' is a PropertySet, specific to the instance
	props.addProperty(
		OGRE_NEW Property<String>(&(defi->second),
			boost::bind(&Foo::getName, inst), 
			boost::bind(&Foo::setName, inst, _1)));

	@endcode

*/

/** @} */
/** @} */
namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Property
	*  @{
	*/

	/// The type of a property
	enum PropertyType
	{
		PROP_SHORT = 0,
		PROP_UNSIGNED_SHORT = 1,
		PROP_INT = 2,
		PROP_UNSIGNED_INT = 3,
		PROP_LONG = 4, 
		PROP_UNSIGNED_LONG = 5,
		PROP_REAL = 6,
		PROP_STRING = 7,
		PROP_VECTOR2 = 8, 
		PROP_VECTOR3 = 9,
		PROP_VECTOR4 = 10, 
		PROP_COLOUR = 11,
		PROP_BOOL = 12,
		PROP_QUATERNION = 13, 
		PROP_MATRIX3 = 14,
		PROP_MATRIX4 = 15, 

		PROP_UNKNOWN = 999
	};

	/** Definition of a property of an object.
	@remarks
	This definition is shared between all instances of an object and therefore
	has no value. Property contains values.
	*/
	class _OgrePropertyExport PropertyDef : public PropertyAlloc
	{
	public:

		/* Construct a property.
		@param name The name of the property
		@param desc A (potentially) long description of the property
		@param pType The type of the property
		*/
		PropertyDef(const String& name, const String& desc, PropertyType pType)
			: mName(name), mDesc(desc), mType(pType) {}

		/// Get the name of the property
		const String& getName() const { return mName; }

		/// Get the description of the property
		const String& getDescription() const { return mDesc; }

		/// Get the type of the property
		PropertyType getType() const { return mType; }

		/// Get a string name of a property type
		static const String& getTypeName(PropertyType theType);

		static PropertyType PropertyDef::getTypeForValue(const short& val) { return PROP_SHORT; }
		static PropertyType PropertyDef::getTypeForValue(const unsigned short& val) { return PROP_UNSIGNED_SHORT; }
		static PropertyType PropertyDef::getTypeForValue(const int& val) { return PROP_INT; }
		static PropertyType PropertyDef::getTypeForValue(const unsigned int& val) { return PROP_UNSIGNED_INT; }
		static PropertyType PropertyDef::getTypeForValue(const long& val) { return PROP_LONG; }
		static PropertyType PropertyDef::getTypeForValue(const unsigned long& val) { return PROP_UNSIGNED_LONG; }
		static PropertyType PropertyDef::getTypeForValue(const Real& val) { return PROP_REAL; }
		static PropertyType PropertyDef::getTypeForValue(const String& val) { return PROP_STRING; }
		static PropertyType PropertyDef::getTypeForValue(const Vector2& val) { return PROP_VECTOR2; }
		static PropertyType PropertyDef::getTypeForValue(const Vector3& val) { return PROP_VECTOR3; }
		static PropertyType PropertyDef::getTypeForValue(const Vector4& val) { return PROP_VECTOR4; }
		static PropertyType PropertyDef::getTypeForValue(const ColourValue& val) { return PROP_COLOUR; }
		static PropertyType PropertyDef::getTypeForValue(const bool& val) { return PROP_BOOL; }
		static PropertyType PropertyDef::getTypeForValue(const Quaternion& val) { return PROP_QUATERNION; }
		static PropertyType PropertyDef::getTypeForValue(const Matrix3& val) { return PROP_MATRIX3; }
		static PropertyType PropertyDef::getTypeForValue(const Matrix4& val) { return PROP_MATRIX4; }

	protected:
		// no default construction
		PropertyDef() {}

		String mName;
		String mDesc;
		PropertyType mType;

	};

	/// Map from property name to shared definition
	typedef map<String, PropertyDef>::type PropertyDefMap;

	/** Base interface for an instance of a property.
	*/
	class _OgrePropertyExport PropertyBase : public PropertyAlloc
	{
	public:
		/// Constructor
		PropertyBase(PropertyDef* def) : mDef(def) {}
		virtual ~PropertyBase() {}

		/// Get the name of the property
		const String& getName() const { return mDef->getName(); }

		/// Get the description of the property
		const String& getDescription() const { return mDef->getDescription(); }

		/// Get the type of the property
		PropertyType getType() const { return mDef->getType(); }

		/// Return the current value as an Any
		virtual Ogre::Any getValue() const = 0;

	protected:
		// disallow default construction
		PropertyBase() {}
		PropertyDef* mDef;

	};

	/** Property instance with passthrough calls to a given object. */
	template <typename T>
	class Property : public PropertyBase
	{
	public:
		typedef T value_type;
		typedef boost::function< T (void) > getter_func;
		typedef boost::function< void (T) > setter_func;

		/** Construct a property which is able to directly call a given 
		getter and setter on a specific object instance, via functors.
		*/
		Property(PropertyDef* def, getter_func getter, setter_func setter)
			: PropertyBase(def)
			, mGetter(getter)
			, mSetter(setter) 
		{
		}

		/** Set the property value.
		*/
		virtual void set(T val)
		{
			mSetter(val);
		}

		virtual T get() const
		{
			return mGetter();
		}

		Ogre::Any getValue() const
		{
			return Ogre::Any(get());
		}

	protected:
		// disallow default construction
		Property() {}
		~Property() {}

		getter_func mGetter;
		setter_func mSetter;
	};

	/** A simple structure designed just as a holder of property values between
	the instances of objects they might target. There is just enough information
	here to be able to interpret the results accurately but no more.
	*/
	struct PropertyValue
	{
		PropertyType propType;
		Ogre::Any val;
	};
	/// Defines a transferable map of properties using wrapped value types (Ogre::Any)
	typedef map<String, PropertyValue>::type PropertyValueMap;


	/** Defines a complete set of properties for a single object instance.
	*/
	class _OgrePropertyExport PropertySet : public PropertyAlloc
	{
	public:
		PropertySet();
		~PropertySet();

		/** Adds a property to this set. 
		@remarks
		The PropertySet is responsible for deleting this object.
		*/
		void addProperty(PropertyBase* prop);

		/** Gets the property object for a given property name. 
		@remarks
		Note that this property will need to be cast to a templated property
		compatible with the type you will be setting. You might find the 
		overloaded set and get<type> methods quicker if 
		you already know the type.
		*/
		PropertyBase* getProperty(const String& name) const;

		/** Reports whether this property set contains a named property. */
		bool hasProperty(const String& name) const;

		typedef map<String, PropertyBase*>::type PropertyMap;
		typedef Ogre::MapIterator<PropertyMap> PropertyIterator;
		/// Get an iterator over the available properties
		PropertyIterator getPropertyIterator();

		/** Gets an independently usable collection of property values from the
		current state.
		*/
		PropertyValueMap getValueMap() const;

		/** Sets the current state from a given value map.
		*/
		void setValueMap(const PropertyValueMap& values);

		/** Get a named property value. 
		*/
		template<typename T>
		void getValue(const String& name, T& value) const
		{
			getPropertyImpl(name, value, PropertyDef::getTypeForValue(value));
		}

		/** Set a named property value (via pointer to avoid copy). 
		*/
		template<typename T>
		void setValue(const String& name, const T* value)
		{
			setPropertyImpl(name, *value, PropertyDef::getTypeForValue(*value));
		}
		/** Set a named property value. 
		*/
		template<typename T>
		void setValue(const String& name, T value)
		{
			setPropertyImpl(name, value, PropertyDef::getTypeForValue(value));
		}
		/** Special-case char*, convert to String automatically. 
		*/
		void setValue(const String& name, const char* pChar)
		{
			String v(pChar);
			setPropertyImpl(name, v, PROP_STRING);
		}


	protected:
		PropertyMap mPropertyMap;

		/// Set a named property value, internal implementation (type match required)
		template <typename T>
		void setPropertyImpl(const String& name, const T& val, PropertyType typeCheck)
		{
			PropertyBase* baseProp = getProperty(name);
			if (baseProp->getType() != typeCheck)
			{
				StringUtil::StrStreamType msg;
				msg << "Property error: type passed in: '" << PropertyDef::getTypeName(typeCheck)
					<< "', type of property: '" << PropertyDef::getTypeName(baseProp->getType()) << "'";
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, msg.str(), "PropertySet::setPropertyImpl");
			}
			static_cast<Property<T>*>(baseProp)->set(val);
		}

		/// Set a named property value, internal implementation (type match required)
		template <typename T>
		void getPropertyImpl(const String& name, T& refVal, PropertyType typeCheck) const
		{
			PropertyBase* baseProp = getProperty(name);
			if (baseProp->getType() != typeCheck)
			{
				StringUtil::StrStreamType msg;
				msg << "Property error: type requested: '" << PropertyDef::getTypeName(typeCheck)
					<< "', type of property: '" << PropertyDef::getTypeName(baseProp->getType()) << "'";
				OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, msg.str(), "PropertySet::getPropertyImpl");
			}
			refVal = static_cast<Property<T>*>(baseProp)->get();
		}

	};

	/** @} */
	/** @} */

}

#endif 
