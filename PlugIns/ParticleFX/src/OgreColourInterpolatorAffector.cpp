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
#include "OgreColourInterpolatorAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
    ColourInterpolatorAffector::CmdColourAdjust		ColourInterpolatorAffector::msColourCmd[MAX_STAGES];
    ColourInterpolatorAffector::CmdTimeAdjust		ColourInterpolatorAffector::msTimeCmd[MAX_STAGES];

    //-----------------------------------------------------------------------
    ColourInterpolatorAffector::ColourInterpolatorAffector(ParticleSystem* psys)
        : ParticleAffector(psys)
    {
		for (int i=0;i<MAX_STAGES;i++)
		{
			// set default colour to transparent grey, transparent since we might not want to display the particle here
			// grey because when a colour component is 0.5f the maximum difference to another colour component is 0.5f
			mColourAdj[i]	= ColourValue(0.5f, 0.5f, 0.5f, 0.0f);
			mTimeAdj[i]		= 1.0f;
		}

        mType = "ColourInterpolator";

        // Init parameters
        if (createParamDictionary("ColourInterpolatorAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

			for (int i=0;i<MAX_STAGES;i++)
			{
				msColourCmd[i].mIndex	= i;
				msTimeCmd[i].mIndex		= i;

				StringUtil::StrStreamType stage;
				stage << i;
				String	colour_title	= String("colour") + stage.str();
				String	time_title		= String("time") + stage.str();
				String	colour_descr	= String("Stage ") + stage.str() + String(" colour.");
				String	time_descr		= String("Stage ") + stage.str() + String(" time.");

				dict->addParameter(ParameterDef(colour_title, colour_descr, PT_COLOURVALUE), &msColourCmd[i]);
				dict->addParameter(ParameterDef(time_title,   time_descr,   PT_REAL),		 &msTimeCmd[i]);
			}
        }
    }
    //-----------------------------------------------------------------------
    void ColourInterpolatorAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        Particle*			p;
		ParticleIterator	pi				= pSystem->_getIterator();


		while (!pi.end())
        {
            p = pi.getNext();
			const Real		life_time		= p->totalTimeToLive;
			Real			particle_time	= 1.0f - (p->timeToLive / life_time); 

			if (particle_time <= mTimeAdj[0])
			{
				p->colour = mColourAdj[0];
			} else
			if (particle_time >= mTimeAdj[MAX_STAGES - 1])
			{
				p->colour = mColourAdj[MAX_STAGES-1];
			} else
			{
				for (int i=0;i<MAX_STAGES-1;i++)
				{
					if (particle_time >= mTimeAdj[i] && particle_time < mTimeAdj[i + 1])
					{
						particle_time -= mTimeAdj[i];
						particle_time /= (mTimeAdj[i+1]-mTimeAdj[i]);
						p->colour.r = ((mColourAdj[i+1].r * particle_time) + (mColourAdj[i].r * (1.0f - particle_time)));
						p->colour.g = ((mColourAdj[i+1].g * particle_time) + (mColourAdj[i].g * (1.0f - particle_time)));
						p->colour.b = ((mColourAdj[i+1].b * particle_time) + (mColourAdj[i].b * (1.0f - particle_time)));
						p->colour.a = ((mColourAdj[i+1].a * particle_time) + (mColourAdj[i].a * (1.0f - particle_time)));
						break;
					}
				}
			}
		}
    }
    
	//-----------------------------------------------------------------------
    void ColourInterpolatorAffector::setColourAdjust(size_t index, ColourValue colour)
    {
        mColourAdj[index] = colour;
    }
    //-----------------------------------------------------------------------
    ColourValue ColourInterpolatorAffector::getColourAdjust(size_t index) const
    {
        return mColourAdj[index];
    }


    //-----------------------------------------------------------------------
    void ColourInterpolatorAffector::setTimeAdjust(size_t index, Real time)
    {
        mTimeAdj[index] = time;
    }
    //-----------------------------------------------------------------------
    Real ColourInterpolatorAffector::getTimeAdjust(size_t index) const
    {
        return mTimeAdj[index];
    }
    
	
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ColourInterpolatorAffector::CmdColourAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourInterpolatorAffector*>(target)->getColourAdjust(mIndex) );
    }
    void ColourInterpolatorAffector::CmdColourAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourInterpolatorAffector*>(target)->setColourAdjust(mIndex,
            StringConverter::parseColourValue(val));
    }
	//-----------------------------------------------------------------------
    String ColourInterpolatorAffector::CmdTimeAdjust::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ColourInterpolatorAffector*>(target)->getTimeAdjust(mIndex) );
    }
    void ColourInterpolatorAffector::CmdTimeAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourInterpolatorAffector*>(target)->setTimeAdjust(mIndex,
            StringConverter::parseReal(val));
    }

}



