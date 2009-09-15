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
#ifndef __ColourFaderAffector2_H__
#define __ColourFaderAffector2_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleAffector.h"
#include "OgreStringInterface.h"

namespace Ogre {


    /** This plugin subclass of ParticleAffector allows you to alter the colour of particles.
    @remarks
        This class supplies the ParticleAffector implementation required to modify the colour of
        particle in mid-flight.
    */
    class _OgreParticleFXExport ColourFaderAffector2 : public ParticleAffector
    {
    public:

        /** Command object for red adjust (see ParamCommand).*/
        class CmdRedAdjust1 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for green adjust (see ParamCommand).*/
        class CmdGreenAdjust1 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for blue adjust (see ParamCommand).*/
        class CmdBlueAdjust1 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdAlphaAdjust1 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for red adjust (see ParamCommand).*/
        class CmdRedAdjust2 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for green adjust (see ParamCommand).*/
        class CmdGreenAdjust2 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for blue adjust (see ParamCommand).*/
        class CmdBlueAdjust2 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdAlphaAdjust2 : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for alpha adjust (see ParamCommand).*/
        class CmdStateChange : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };


        /** Default constructor. */
        ColourFaderAffector2(ParticleSystem* psys);

        /** See ParticleAffector. */
        void _affectParticles(ParticleSystem* pSystem, Real timeElapsed);

        /** Sets the colour adjustment to be made per second to particles. 
        @param red, green, blue, alpha
            Sets the adjustment to be made to each of the colour components per second. These
            values will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setAdjust1(float red, float green, float blue, float alpha = 0.0);
		void setAdjust2(float red, float green, float blue, float alpha = 0.0);
        /** Sets the red adjustment to be made per second to particles. 
        @param red
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setRedAdjust1(float red);
		void setRedAdjust2(float red);

        /** Gets the red adjustment to be made per second to particles. */
        float getRedAdjust1(void) const;
		float getRedAdjust2(void) const;

        /** Sets the green adjustment to be made per second to particles. 
        @param green
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setGreenAdjust1(float green);
		void setGreenAdjust2(float green);
        /** Gets the green adjustment to be made per second to particles. */
        float getGreenAdjust1(void) const;
		float getGreenAdjust2(void) const;
        /** Sets the blue adjustment to be made per second to particles. 
        @param blue
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setBlueAdjust1(float blue);
		void setBlueAdjust2(float blue);
        /** Gets the blue adjustment to be made per second to particles. */
        float getBlueAdjust1(void) const;
		float getBlueAdjust2(void) const;

        /** Sets the alpha adjustment to be made per second to particles. 
        @param alpha
            The adjustment to be made to the colour component per second. This
            value will be added to the colour of all particles every second, scaled over each frame
            for a smooth adjustment.
        */
        void setAlphaAdjust1(float alpha);
		void setAlphaAdjust2(float alpha);
        /** Gets the alpha adjustment to be made per second to particles. */
        float getAlphaAdjust1(void) const;
		float getAlphaAdjust2(void) const;


        void setStateChange(Real NewValue );
        Real getStateChange(void) const;

        static CmdRedAdjust1 msRedCmd1;
		static CmdRedAdjust2 msRedCmd2;
        static CmdGreenAdjust1 msGreenCmd1;
		static CmdGreenAdjust2 msGreenCmd2;
        static CmdBlueAdjust1 msBlueCmd1;
		static CmdBlueAdjust2 msBlueCmd2;
        static CmdAlphaAdjust1 msAlphaCmd1;
		static CmdAlphaAdjust2 msAlphaCmd2;
        static CmdStateChange msStateCmd;

    protected:
        float mRedAdj1, mRedAdj2;
        float mGreenAdj1,  mGreenAdj2;
        float mBlueAdj1, mBlueAdj2;
        float mAlphaAdj1, mAlphaAdj2;
		Real StateChangeVal;

        /** Internal method for adjusting while clamping to [0,1] */
        inline void applyAdjustWithClamp(float* pComponent, float adjust)
        {
            *pComponent += adjust;
            // Limit to 0
            if (*pComponent < 0.0)
            {
                *pComponent = 0.0f;
            }
            // Limit to 1
            else if (*pComponent > 1.0)
            {
                *pComponent = 1.0f;
            }
        }

    };


}


#endif

