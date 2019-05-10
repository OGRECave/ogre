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
#include "OgreRoot.h"
#include "OgreProperty.h"

#include <gtest/gtest.h>

#include <functional>

using namespace Ogre;
//--------------------------------------------------------------------------
class Foo
{
protected:
    String mName;
public:
    void setName(const String& name) { mName = name; }
    const String& getName() const { return mName; }
};
//--------------------------------------------------------------------------

TEST(Property, StringProp) {
    using namespace std;

    PropertyDefMap propertyDefs;
    Foo foo;
    PropertySet props;

    PropertyDefMap::iterator defi =
        propertyDefs.emplace("name", PropertyDef("name", "The name of the object.", PROP_STRING)).first;

    props.addProperty(
        OGRE_NEW Property<String>(&(defi->second),
        bind(&Foo::getName, &foo),
        bind(&Foo::setName, &foo, placeholders::_1)));

    Ogre::String strName, strTest;
    strTest = "A simple name";
    props.setValue("name", strTest);
    props.getValue("name", strName);

    ASSERT_EQ(strTest, strName);
}
//--------------------------------------------------------------------------
