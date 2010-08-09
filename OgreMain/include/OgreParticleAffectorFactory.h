/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
        ParticleAffectorFactory() {}
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

