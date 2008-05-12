/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright ) 2002 Tels <http://bloodgate.com> Based on OgrreBoxEmitter
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
    void AreaEmitter::setDirection( const Vector3& direction )
    {
        ParticleEmitter::setDirection( direction );

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


