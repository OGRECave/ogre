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
#ifndef __ParticleIterator_H__
#define __ParticleIterator_H__

#include "OgrePrerequisites.h"

namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Convenience class to make it easy to step through all particles in a ParticleSystem.
    */
    class _OgreExport ParticleIterator
    {
        friend class ParticleSystem;
    protected:
        list<Particle*>::type::iterator mPos;
        list<Particle*>::type::iterator mStart;
        list<Particle*>::type::iterator mEnd;

        /// Protected constructor, only available from ParticleSystem::getIterator
        ParticleIterator(list<Particle*>::type::iterator start, list<Particle*>::type::iterator end);

    public:
        // Returns true when at the end of the particle list
        bool end(void);

        /** Returns a pointer to the next particle, and moves the iterator on by 1 element. */
        Particle* getNext(void);
    };
	/** @} */
	/** @} */
}


#endif

