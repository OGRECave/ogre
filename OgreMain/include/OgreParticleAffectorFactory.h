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
#ifndef __ParticleAffectorFactory_H__
#define __ParticleAffectorFactory_H__


#include "OgrePrerequisites.h"
#include "OgreParticleAffector.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Abstract class defining the interface to be implemented by creators of ParticleAffector subclasses.
    @remarks
        Plugins or 3rd party applications can add new types of particle affectors to Ogre by creating
        subclasses of the ParticleAffector class. Because multiple instances of these affectors may be
        required, a factory class to manage the instances is also required. 
    @par
        ParticleAffectorFactory subclasses must allow the creation and destruction of ParticleAffector
        subclasses. They must also be registered with the ParticleSystemManager. All factories have
        a name which identifies them, examples might be 'force_vector', 'attractor', or 'fader', and these can be 
        also be used from particle system scripts.
    */
	class _OgreExport ParticleAffectorFactory : public FXAlloc
    {
    protected:
        vector<ParticleAffector*>::type mAffectors;
    public:
        ParticleAffectorFactory() {};
        virtual ~ParticleAffectorFactory();
        /** Returns the name of the factory, the name which identifies the particle affector type this factory creates. */
        virtual String getName() const = 0;

        /** Creates a new affector instance.
        @remarks
            The subclass MUST add a pointer to the created instance to mAffectors.
        */
        virtual ParticleAffector* createAffector(ParticleSystem* psys) = 0;

        /** Destroys the affector pointed to by the parameter (for early clean up if required). */
        virtual void destroyAffector(ParticleAffector* e);
    };

	/** @} */
	/** @} */

}


#endif

