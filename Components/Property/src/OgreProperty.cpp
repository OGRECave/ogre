/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreProperty.h"

#include <istream>

namespace Ogre
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const String& PropertyDef::getTypeName(PropertyType theType)
    {
        static const String sPropNames[] = {
            "short",
            "unsigned short",
            "int",
            "unsigned int",
            "long",
            "unsigned long",
            "Real",
            "String",
            "Vector2",
            "Vector3",
            "Vector4",
            "ColourValue",
            "bool",
            "Quaternion",
            "Matrix3",
            "Matrix4"
        };

        return sPropNames[(int)theType];
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    PropertySet::PropertySet()
    {

    }
    //---------------------------------------------------------------------
    PropertySet::~PropertySet()
    {
        for (PropertyMap::iterator i = mPropertyMap.begin(); i != mPropertyMap.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mPropertyMap.clear();
    }
    //---------------------------------------------------------------------
    void PropertySet::addProperty(PropertyBase* prop)
    {
        std::pair<PropertyMap::iterator, bool> retPair = 
            mPropertyMap.insert(PropertyMap::value_type(prop->getName(), prop));
        if (!retPair.second)
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Duplicate property entry!", 
                "PropertySet::addProperty");
    }
    //---------------------------------------------------------------------
    bool PropertySet::hasProperty(const String& name) const
    {
        PropertyMap::const_iterator i = mPropertyMap.find(name);
        return i != mPropertyMap.end();
    }
    //---------------------------------------------------------------------
    PropertyBase* PropertySet::getProperty(const String& name) const
    {
        PropertyMap::const_iterator i = mPropertyMap.find(name);
        if (i != mPropertyMap.end())
            return i->second;
        else
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Property not found!", 
                "PropertySet::getProperty");
    }
    //---------------------------------------------------------------------
    void PropertySet::removeProperty(const String& name)
    {
        mPropertyMap.erase(name);
    }
    //---------------------------------------------------------------------
    PropertySet::PropertyIterator PropertySet::getPropertyIterator()
    {
        return PropertyIterator(mPropertyMap.begin(), mPropertyMap.end());
    }
    //---------------------------------------------------------------------
    PropertyValueMap PropertySet::getValueMap() const
    {
        PropertyValueMap ret;
        for (PropertyMap::const_iterator i = mPropertyMap.begin(); i != mPropertyMap.end(); ++i)
        {
            PropertyValue val;
            val.propType = i->second->getType();
            switch(val.propType)
            {
            case PROP_SHORT:
                val.val = Ogre::Any(static_cast<Property<short>*>(i->second)->get());
                break;
            case PROP_UNSIGNED_SHORT:
                val.val = Ogre::Any(static_cast<Property<unsigned short>*>(i->second)->get());
                break;
            case PROP_INT:
                val.val = Ogre::Any(static_cast<Property<int>*>(i->second)->get());
                break;
            case PROP_UNSIGNED_INT:
                val.val = Ogre::Any(static_cast<Property<unsigned int>*>(i->second)->get());
                break;
            case PROP_LONG:
                val.val = Ogre::Any(static_cast<Property<long>*>(i->second)->get());
                break;
            case PROP_UNSIGNED_LONG:
                val.val = Ogre::Any(static_cast<Property<unsigned long>*>(i->second)->get());
                break;
            case PROP_REAL:
                val.val = Ogre::Any(static_cast<Property<Real>*>(i->second)->get());
                break;
            case PROP_STRING:
                val.val = Ogre::Any(static_cast<Property<String>*>(i->second)->get());
                break;
            case PROP_VECTOR2:
                val.val = Ogre::Any(static_cast<Property<Ogre::Vector2>*>(i->second)->get());
                break;
            case PROP_VECTOR3:
                val.val = Ogre::Any(static_cast<Property<Ogre::Vector3>*>(i->second)->get());
                break;
            case PROP_VECTOR4:
                val.val = Ogre::Any(static_cast<Property<Ogre::Vector4>*>(i->second)->get());
                break;
            case PROP_COLOUR:
                val.val = Ogre::Any(static_cast<Property<Ogre::ColourValue>*>(i->second)->get());
                break;
            case PROP_BOOL:
                val.val = Ogre::Any(static_cast<Property<bool>*>(i->second)->get());
                break;
            case PROP_QUATERNION:
                val.val = Ogre::Any(static_cast<Property<Ogre::Quaternion>*>(i->second)->get());
                break;
            case PROP_MATRIX3:
                val.val = Ogre::Any(static_cast<Property<Ogre::Matrix3>*>(i->second)->get());
                break;
            case PROP_MATRIX4:
                val.val = Ogre::Any(static_cast<Property<Ogre::Matrix4>*>(i->second)->get());
                break;
            case PROP_UNKNOWN:
            default:
                break;
            };
            ret[i->second->getName()] = val;
        }

        return ret;


    }
    //---------------------------------------------------------------------
    void PropertySet::setValueMap(const PropertyValueMap& values)
    {
        for (PropertyValueMap::const_iterator i = values.begin(); i != values.end(); ++i)
        {
            PropertyMap::iterator j = mPropertyMap.find(i->first);
            if (j != mPropertyMap.end())
            {
                // matching properties
                // check type
                if (j->second->getType() != i->second.propType)
                {
                    StringStream msg;
                    msg << "Property " << i->first << " mismatched type; incoming type: '"
                        << PropertyDef::getTypeName(i->second.propType) << "', property type: '"
                        << PropertyDef::getTypeName(j->second->getType()) << "'";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, msg.str(), "PropertySet::setValueMap");
                }
                switch(i->second.propType)
                {
                case PROP_SHORT:
                    static_cast<Property<short>*>(j->second)->set(Ogre::any_cast<short>(i->second.val));
                    break;
                case PROP_UNSIGNED_SHORT:
                    static_cast<Property<short>*>(j->second)->set(Ogre::any_cast<unsigned short>(i->second.val));
                    break;
                case PROP_INT:
                    static_cast<Property<int>*>(j->second)->set(Ogre::any_cast<int>(i->second.val));
                    break;
                case PROP_UNSIGNED_INT:
                    static_cast<Property<int>*>(j->second)->set(Ogre::any_cast<unsigned int>(i->second.val));
                    break;
                case PROP_LONG:
                    static_cast<Property<long>*>(j->second)->set(Ogre::any_cast<long>(i->second.val));
                    break;
                case PROP_UNSIGNED_LONG:
                    static_cast<Property<long>*>(j->second)->set(Ogre::any_cast<unsigned long>(i->second.val));
                    break;
                case PROP_REAL:
                    static_cast<Property<Real>*>(j->second)->set(Ogre::any_cast<Real>(i->second.val));
                    break;
                case PROP_STRING:
                    static_cast<Property<String>*>(j->second)->set(Ogre::any_cast<String>(i->second.val));
                    break;
                case PROP_VECTOR2:
                    static_cast<Property<Ogre::Vector2>*>(j->second)->set(Ogre::any_cast<Ogre::Vector2>(i->second.val));
                    break;
                case PROP_VECTOR3:
                    static_cast<Property<Ogre::Vector3>*>(j->second)->set(Ogre::any_cast<Ogre::Vector3>(i->second.val));
                    break;
                case PROP_VECTOR4:
                    static_cast<Property<Ogre::Vector4>*>(j->second)->set(Ogre::any_cast<Ogre::Vector4>(i->second.val));
                    break;
                case PROP_COLOUR:
                    static_cast<Property<Ogre::ColourValue>*>(j->second)->set(Ogre::any_cast<Ogre::ColourValue>(i->second.val));
                    break;
                case PROP_BOOL:
                    static_cast<Property<bool>*>(j->second)->set(Ogre::any_cast<bool>(i->second.val));
                    break;
                case PROP_QUATERNION:
                    static_cast<Property<Ogre::Quaternion>*>(j->second)->set(Ogre::any_cast<Ogre::Quaternion>(i->second.val));
                    break;
                case PROP_MATRIX3:
                    static_cast<Property<Ogre::Matrix3>*>(j->second)->set(Ogre::any_cast<Ogre::Matrix3>(i->second.val));
                    break;
                case PROP_MATRIX4:
                    static_cast<Property<Ogre::Matrix4>*>(j->second)->set(Ogre::any_cast<Ogre::Matrix4>(i->second.val));
                    break;
                case PROP_UNKNOWN:
                default:
                    break;
                        
                };


            }
        }


    }

}

