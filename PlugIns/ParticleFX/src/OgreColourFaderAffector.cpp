/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreColourFaderAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
    ColourFaderAffector::CmdRedAdjust ColourFaderAffector::msRedCmd;
    ColourFaderAffector::CmdGreenAdjust ColourFaderAffector::msGreenCmd;
    ColourFaderAffector::CmdBlueAdjust ColourFaderAffector::msBlueCmd;
    ColourFaderAffector::CmdAlphaAdjust ColourFaderAffector::msAlphaCmd;

    //-----------------------------------------------------------------------
    ColourFaderAffector::ColourFaderAffector(ParticleSystem* psys) : ParticleAffector(psys)
    {
        mRedAdj = mGreenAdj = mBlueAdj = mAlphaAdj = 0;
        mType = "ColourFader";

        // Init parameters
        if (createParamDictionary("ColourFaderAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("red", 
                "The amount by which to adjust the red component of particles per second.",
                PT_REAL), &msRedCmd);
            dict->addParameter(ParameterDef("green", 
                "The amount by which to adjust the green component of particles per second.",
                PT_REAL), &msGreenCmd);
            dict->addParameter(ParameterDef("blue", 
                "The amount by which to adjust the blue component of particles per second.",
                PT_REAL), &msBlueCmd);
            dict->addParameter(ParameterDef("alpha", 
                "The amount by which to adjust the alpha component of particles per second.",
                PT_REAL), &msAlphaCmd);


        }
    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;
        float dr, dg, db, da;

        // Scale adjustments by time
        dr = mRedAdj * timeElapsed;
        dg = mGreenAdj * timeElapsed;
        db = mBlueAdj * timeElapsed;
        da = mAlphaAdj * timeElapsed;

        while (!pi.end())
        {
            p = pi.getNext();
            applyAdjustWithClamp(&p->colour.r, dr);
            applyAdjustWithClamp(&p->colour.g, dg);
            applyAdjustWithClamp(&p->colour.b, db);
            applyAdjustWithClamp(&p->colour.a, da);
        }

    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::setAdjust(float red, float green, float blue, float alpha)
    {
        mRedAdj = red;
        mGreenAdj = green;
        mBlueAdj = blue;
        mAlphaAdj = alpha;
    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::setRedAdjust(float red)
    {
        mRedAdj = red;
    }
    //-----------------------------------------------------------------------
    float ColourFaderAffector::getRedAdjust(void) const
    {
        return mRedAdj;
    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::setGreenAdjust(float green)
    {
        mGreenAdj = green;
    }
    //-----------------------------------------------------------------------
    float ColourFaderAffector::getGreenAdjust(void) const
    {
        return mGreenAdj;
    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::setBlueAdjust(float blue)
    {
        mBlueAdj = blue;
    }
    //-----------------------------------------------------------------------
    float ColourFaderAffector::getBlueAdjust(void) const
    {
        return mBlueAdj;
    }
    //-----------------------------------------------------------------------
    void ColourFaderAffector::setAlphaAdjust(float alpha)
    {
        mAlphaAdj = alpha;
    }
    //-----------------------------------------------------------------------
    float ColourFaderAffector::getAlphaAdjust(void) const
    {
        return mAlphaAdj;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ColourFaderAffector::CmdRedAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourFaderAffector*>(target)->getRedAdjust() );
    }
    void ColourFaderAffector::CmdRedAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourFaderAffector*>(target)->setRedAdjust(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ColourFaderAffector::CmdGreenAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourFaderAffector*>(target)->getGreenAdjust() );
    }
    void ColourFaderAffector::CmdGreenAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourFaderAffector*>(target)->setGreenAdjust(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ColourFaderAffector::CmdBlueAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourFaderAffector*>(target)->getBlueAdjust() );
    }
    void ColourFaderAffector::CmdBlueAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourFaderAffector*>(target)->setBlueAdjust(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ColourFaderAffector::CmdAlphaAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourFaderAffector*>(target)->getAlphaAdjust() );
    }
    void ColourFaderAffector::CmdAlphaAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourFaderAffector*>(target)->setAlphaAdjust(
            StringConverter::parseReal(val));
    }

}



