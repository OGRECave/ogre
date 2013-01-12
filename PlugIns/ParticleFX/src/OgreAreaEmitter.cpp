/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
// Original author: Tels <http://bloodgate.com>, released as public domain
#include "OgreAreaEmitter.h"
#include "OgreParticle.h"
#include "OgreQuaternion.h"
#include "OgreException.h"
#include "OgreStringConverter.h"



namespace Ogre {

    // Instatiate statics
    AreaEmitter::CmdWidth AreaEmitter::msWidthCmd;
    AreaEmitter::CmdHeight AreaEmitter::msHeightCmd;
    AreaEmitter::CmdDepth AreaEmitter::msDepthCmd;

    //-----------------------------------------------------------------------
    bool AreaEmitter::initDefaults(const String& t)
    {
        // called by the constructor as initDefaults("Type")

        // Defaults
        mDirection = Vector3::UNIT_Z;
        mUp = Vector3::UNIT_Y;
        setSize(100,100,100);
        mType = t;

        // Set up parameters
        if (createParamDictionary(mType + "Emitter"))
        {

            addBaseParameters();
            ParamDictionary* dict = getParamDictionary();

            // Custom params
            dict->addParameter(ParameterDef("width", 
                "Width of the shape in world coordinates.",
                PT_REAL),&msWidthCmd);
            dict->addParameter(ParameterDef("height", 
                "Height of the shape in world coordinates.",
                PT_REAL),&msHeightCmd);
            dict->addParameter(ParameterDef("depth", 
                "Depth of the shape in world coordinates.",
                PT_REAL),&msDepthCmd);
            return true;

        }
        return false;
    }

    //-----------------------------------------------------------------------
    unsigned short AreaEmitter::_getEmissionCount(Real timeElapsed)
    {
        // Use basic constant emission 
        return genConstantEmissionCount(timeElapsed);
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setDirection( const Vector3& inDirection )
    {
        ParticleEmitter::setDirection( inDirection );

        // Update the ranges
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setSize(const Vector3& size)
    {
        mSize = size;
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setSize(Real x, Real y, Real z)
    {
        mSize.x = x;
        mSize.y = y;
        mSize.z = z;
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setWidth(Real width)
    {
        mSize.x = width;
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    Real AreaEmitter::getWidth(void) const
    {
        return mSize.x;
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setHeight(Real height)
    {
        mSize.y = height;
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    Real AreaEmitter::getHeight(void) const
    {
        return mSize.y;
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::setDepth(Real depth)
    {
        mSize.z = depth;
        genAreaAxes();
    }
    //-----------------------------------------------------------------------
    Real AreaEmitter::getDepth(void) const
    {
        return mSize.z;
    }
    //-----------------------------------------------------------------------
    void AreaEmitter::genAreaAxes(void)
    {
        Vector3 mLeft = mUp.crossProduct(mDirection);

        mXRange = mLeft * (mSize.x * 0.5f);
        mYRange = mUp * (mSize.y * 0.5f);
        mZRange = mDirection * (mSize.z * 0.5f);
    }

    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String AreaEmitter::CmdWidth::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const AreaEmitter*>(target)->getWidth() );
    }
    void AreaEmitter::CmdWidth::doSet(void* target, const String& val)
    {
        static_cast<AreaEmitter*>(target)->setWidth(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String AreaEmitter::CmdHeight::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const AreaEmitter*>(target)->getHeight() );
    }
    void AreaEmitter::CmdHeight::doSet(void* target, const String& val)
    {
        static_cast<AreaEmitter*>(target)->setHeight(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String AreaEmitter::CmdDepth::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const AreaEmitter*>(target)->getDepth() );
    }
    void AreaEmitter::CmdDepth::doSet(void* target, const String& val)
    {
        static_cast<AreaEmitter*>(target)->setDepth(StringConverter::parseReal(val));
    }



}


