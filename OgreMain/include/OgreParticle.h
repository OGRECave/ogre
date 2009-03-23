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
#ifndef __Particle_H__
#define __Particle_H__

#include "OgrePrerequisites.h"
#include "OgreBillboard.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Abstract class containing any additional data required to be associated
		with a particle to perform the required rendering. 
	@remarks
		Because you can specialise the way that particles are rendered by supplying
		custom ParticleSystemRenderer classes, you might well need some additional 
		data for your custom rendering routine which is not held on the default particle
		class. If that's the case, then you should define a subclass of this class, 
		and construct it when asked in your custom ParticleSystemRenderer class.
	*/
	class _OgreExport ParticleVisualData : public FXAlloc
	{
	public:
		ParticleVisualData() {}
		virtual ~ParticleVisualData() {}

	};

	/** Class representing a single particle instance. */
	class _OgreExport Particle : public FXAlloc
    {
    protected:
        /// Parent ParticleSystem
        ParticleSystem* mParentSystem;
        /// Additional visual data you might want to associate with the Particle
        ParticleVisualData* mVisual;
    public:
        /// Type of particle
        enum ParticleType
        {
            Visual,
            Emitter
        };

        /// Does this particle have it's own dimensions?
        bool mOwnDimensions;
        /// Personal width if mOwnDimensions == true
        Real mWidth;
        /// Personal height if mOwnDimensions == true
        Real mHeight;
        /// Current rotation value
        Radian rotation;
        // Note the intentional public access to internal variables
        // Accessing via get/set would be too costly for 000's of particles
        /// World position
        Vector3 position;
        /// Direction (and speed) 
        Vector3 direction;
        /// Current colour
        ColourValue colour;
        /// Time to live, number of seconds left of particles natural life
        Real timeToLive;
        /// Total Time to live, number of seconds of particles natural life
        Real totalTimeToLive;
        /// Speed of rotation in radians/sec
        Radian rotationSpeed;
        /// Determines the type of particle.
        ParticleType particleType;

        Particle()
            : mParentSystem(0), mVisual(0), mOwnDimensions(false), rotation(0), 
            position(Vector3::ZERO), direction(Vector3::ZERO), 
            colour(ColourValue::White), timeToLive(10), totalTimeToLive(10), 
            rotationSpeed(0), particleType(Visual)
        {
        }

        /** Sets the width and height for this particle.
        @remarks
        Note that it is most efficient for every particle in a ParticleSystem to have the same dimensions. If you
        choose to alter the dimensions of an individual particle the set will be less efficient. Do not call
        this method unless you really need to have different particle dimensions within the same set. Otherwise
        just call the ParticleSystem::setDefaultDimensions method instead.
        */
        void setDimensions(Real width, Real height); 

        /** Returns true if this particle deviates from the ParticleSystem's default dimensions (i.e. if the
        particle::setDimensions method has been called for this instance).
        @see
        particle::setDimensions
        */
        bool hasOwnDimensions(void) const { return mOwnDimensions; }

        /** Retrieves the particle's personal width, if hasOwnDimensions is true. */
        Real getOwnWidth(void) const { return mWidth; }

        /** Retrieves the particle's personal width, if hasOwnDimensions is true. */
        Real getOwnHeight(void) const { return mHeight; }
        
        /** Sets the current rotation */
        void setRotation(const Radian& rad);

        const Radian& getRotation(void) const { return rotation; }

        /** Internal method for notifying the particle of it's owner.
        */
        void _notifyOwner(ParticleSystem* owner);

        /** Internal method for notifying the particle of it's optional visual data.
        */
		void _notifyVisualData(ParticleVisualData* vis) { mVisual = vis; }

		/// Get the optional visual data associated with the class
		ParticleVisualData* getVisualData(void) const { return mVisual; }

        /// Utility method to reset this particle
        void resetDimensions(void);
    };
	/** @} */
	/** @} */
}

#endif

