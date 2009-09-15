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
#ifndef __ColourImageAffector_H__
#define __ColourImageAffector_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"
#include "OgreColourValue.h"
#include "OgreImage.h"

namespace Ogre {


    class _OgreParticleFXExport ColourImageAffector : public ParticleAffector
    {
    public:
		/** Command object for red adjust (see ParamCommand).*/
        class CmdImageAdjust : public ParamCommand
        {
		public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Default constructor. */
        ColourImageAffector(ParticleSystem* psys);

        /** See ParticleAffector. */
		void _initParticle(Particle* pParticle);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

		void setImageAdjust(String name);
		String getImageAdjust(void) const;
        
        
        static CmdImageAdjust	msImageCmd;

    protected:
        Image					mColourImage;
        bool                    mColourImageLoaded;
		String					mColourImageName;

        /** Internal method to load the image */
        void _loadImage(void);
    };


}


#endif

