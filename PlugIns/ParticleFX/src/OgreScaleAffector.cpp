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
#include "OgreScaleAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
    ScaleAffector::CmdScaleAdjust ScaleAffector::msScaleCmd;

    //-----------------------------------------------------------------------
    ScaleAffector::ScaleAffector(ParticleSystem* psys)
        :ParticleAffector(psys)
    {
        mScaleAdj = 0;
        mType = "Scaler";

        // Init parameters
        if (createParamDictionary("ScaleAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("rate", 
                "The amount by which to adjust the x and y scale components of particles per second.",
                PT_REAL), &msScaleCmd);
        }
    }
    //-----------------------------------------------------------------------
    void ScaleAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;
        Real ds;

        // Scale adjustments by time
        ds = mScaleAdj * timeElapsed;

		Real NewWide, NewHigh;

        while (!pi.end())
        {
            p = pi.getNext();

			if( p->hasOwnDimensions() == false )
			{
            	NewWide = pSystem->getDefaultWidth() + ds;
	            NewHigh = pSystem->getDefaultHeight() + ds;

			}
			else
			{
            	NewWide = p->getOwnWidth()  + ds;
            	NewHigh = p->getOwnHeight() + ds;
			}
			p->setDimensions( NewWide, NewHigh ); 
        }

    }
    //-----------------------------------------------------------------------
    void ScaleAffector::setAdjust( Real rate )
    {
        mScaleAdj = rate;
    }
    //-----------------------------------------------------------------------
    Real ScaleAffector::getAdjust(void) const
    {
        return mScaleAdj;
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ScaleAffector::CmdScaleAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ScaleAffector*>(target)->getAdjust() );
    }
    void ScaleAffector::CmdScaleAdjust::doSet(void* target, const String& val)
    {
        static_cast<ScaleAffector*>(target)->setAdjust(
            StringConverter::parseReal(val));
    }

}



