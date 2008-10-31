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
#include "OgreColourImageAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"
#include "OgreException.h"
#include "OgreResourceGroupManager.h"

namespace Ogre {
    
    // init statics
	ColourImageAffector::CmdImageAdjust		ColourImageAffector::msImageCmd;

    //-----------------------------------------------------------------------
    ColourImageAffector::ColourImageAffector(ParticleSystem* psys)
        :ParticleAffector(psys), mColourImageLoaded(false)
    {
        mType = "ColourImage";

        // Init parameters
        if (createParamDictionary("ColourImageAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("image", "image where the colours come from", PT_STRING), &msImageCmd);
        }
    }
    //-----------------------------------------------------------------------
    void ColourImageAffector::_initParticle(Particle* pParticle)
	{
        if (!mColourImageLoaded)
        {
            _loadImage();
        }

        pParticle->colour = mColourImage.getColourAt(0, 0, 0);
    
	}
    //-----------------------------------------------------------------------
    void ColourImageAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        Particle*			p;
		ParticleIterator	pi				= pSystem->_getIterator();

        if (!mColourImageLoaded)
        {
            _loadImage();
        }

		int				   width			= (int)mColourImage.getWidth()  - 1;
        
		while (!pi.end())
		{
			p = pi.getNext();
			const Real		life_time		= p->totalTimeToLive;
			Real			particle_time	= 1.0f - (p->timeToLive / life_time); 

			if (particle_time > 1.0f)
				particle_time = 1.0f;
			if (particle_time < 0.0f)
				particle_time = 0.0f;

			const Real		float_index		= particle_time * width;
			const int		index			= (int)float_index;

            if(index < 0)
            {
				p->colour = mColourImage.getColourAt(0, 0, 0);
            }
            else if(index >= width) 
            {
                p->colour = mColourImage.getColourAt(width, 0, 0);
            }
            else
            {
                // Linear interpolation
				const Real		fract		= float_index - (Real)index;
				const Real		to_colour	= fract;
				const Real		from_colour	= 1.0f - to_colour;
             
                ColourValue from=mColourImage.getColourAt(index, 0, 0),
							to=mColourImage.getColourAt(index+1, 0, 0);

				p->colour.r = from.r*from_colour + to.r*to_colour;
                p->colour.g = from.g*from_colour + to.g*to_colour;
                p->colour.b = from.b*from_colour + to.b*to_colour;
                p->colour.a = from.a*from_colour + to.a*to_colour;
			}
		}
    }
    
    //-----------------------------------------------------------------------
    void ColourImageAffector::setImageAdjust(String name)
    {
		mColourImageName = name;
        mColourImageLoaded = false;
	}
    //-----------------------------------------------------------------------
    void ColourImageAffector::_loadImage(void)
    {
        mColourImage.load(mColourImageName, mParent->getResourceGroupName());

		PixelFormat	format = mColourImage.getFormat();

		if ( !PixelUtil::isAccessible(format) )
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Error: Image is not accessible (rgba) image.",
					"ColourImageAffector::_loadImage" );
		}

        mColourImageLoaded = true;
	}
    //-----------------------------------------------------------------------
    String ColourImageAffector::getImageAdjust(void) const
    {
        return mColourImageName;
    }
    
	
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ColourImageAffector::CmdImageAdjust::doGet(const void* target) const
    {
        return static_cast<const ColourImageAffector*>(target)->getImageAdjust();
    }
    void ColourImageAffector::CmdImageAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourImageAffector*>(target)->setImageAdjust(val);
    }

}



