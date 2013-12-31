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
#ifndef __ColourInterpolatorAffector_H__
#define __ColourInterpolatorAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"
#include "OgreColourValue.h"

namespace Ogre {


    class _OgreParticleFXExport ColourInterpolatorAffector : public ParticleAffector
    {
    public:
		// this is something of a hack.. 
		// needs to be replaced with something more.. 
		// ..elegant
		enum { MAX_STAGES = 6 };


        /** Command object for red adjust (see ParamCommand).*/
        class CmdColourAdjust : public ParamCommand
        {
		public:
			size_t		mIndex;

		public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for red adjust (see ParamCommand).*/
		class CmdTimeAdjust : public ParamCommand
        {
        public:
			size_t		mIndex;

		public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
		};

        /** Default constructor. */
        ColourInterpolatorAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

		void setColourAdjust(size_t index, ColourValue colour);
		ColourValue getColourAdjust(size_t index) const;
        
		void setTimeAdjust(size_t index, Real time);
        Real getTimeAdjust(size_t index) const;
        
        
        static CmdColourAdjust	msColourCmd[MAX_STAGES];
        static CmdTimeAdjust	msTimeCmd[MAX_STAGES];

    protected:
        ColourValue				mColourAdj[MAX_STAGES];
		Real					mTimeAdj[MAX_STAGES];

    };


}


#endif

