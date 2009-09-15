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
#ifndef __ParticleSystemRenderer_H__
#define __ParticleSystemRenderer_H__

#include "OgrePrerequisites.h"
#include "OgreStringInterface.h"
#include "OgreFactoryObj.h"
#include "OgreRenderQueue.h"
#include "OgreCommon.h"
#include "OgreRenderable.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Abstract class defining the interface required to be implemented
        by classes which provide rendering capability to ParticleSystem instances.
    */
    class _OgreExport ParticleSystemRenderer : public StringInterface, public FXAlloc
    {
    public:
        /// Constructor
        ParticleSystemRenderer() {}
        /// Destructor
        virtual ~ParticleSystemRenderer() {}

        /** Gets the type of this renderer - must be implemented by subclasses */
		virtual const String& getType(void) const = 0;

		/** Delegated to by ParticleSystem::_updateRenderQueue
        @remarks
            The subclass must update the render queue using whichever Renderable
            instance(s) it wishes.
        */
        virtual void _updateRenderQueue(RenderQueue* queue, 
            list<Particle*>::type& currentParticles, bool cullIndividually) = 0;

        /** Sets the material this renderer must use; called by ParticleSystem. */
        virtual void _setMaterial(MaterialPtr& mat) = 0;
        /** Delegated to by ParticleSystem::_notifyCurrentCamera */
        virtual void _notifyCurrentCamera(Camera* cam) = 0;
        /** Delegated to by ParticleSystem::_notifyAttached */
        virtual void _notifyAttached(Node* parent, bool isTagPoint = false) = 0;
        /** Optional callback notified when particles are rotated */
        virtual void _notifyParticleRotated(void) {}
        /** Optional callback notified when particles are resized individually */
        virtual void _notifyParticleResized(void) {}
        /** Tells the renderer that the particle quota has changed */
        virtual void _notifyParticleQuota(size_t quota) = 0;
        /** Tells the renderer that the particle default size has changed */
        virtual void _notifyDefaultDimensions(Real width, Real height) = 0;
        /** Optional callback notified when particle emitted */
        virtual void _notifyParticleEmitted(Particle* particle) {}
        /** Optional callback notified when particle expired */
        virtual void _notifyParticleExpired(Particle* particle) {}
        /** Optional callback notified when particles moved */
        virtual void _notifyParticleMoved(list<Particle*>::type& currentParticles) {}
        /** Optional callback notified when particles cleared */
        virtual void _notifyParticleCleared(list<Particle*>::type& currentParticles) {}
		/** Create a new ParticleVisualData instance for attachment to a particle.
		@remarks
			If this renderer needs additional data in each particle, then this should
			be held in an instance of a subclass of ParticleVisualData, and this method
			should be overridden to return a new instance of it. The default
			behaviour is to return null.
		*/
		virtual ParticleVisualData* _createVisualData(void) { return 0; }
		/** Destroy a ParticleVisualData instance.
		@remarks
			If this renderer needs additional data in each particle, then this should
			be held in an instance of a subclass of ParticleVisualData, and this method
			should be overridden to destroy an instance of it. The default
			behaviour is to do nothing.
		*/
		virtual void _destroyVisualData(ParticleVisualData* vis) { assert (vis == 0); }

		/** Sets which render queue group this renderer should target with it's
			output.
		*/
		virtual void setRenderQueueGroup(uint8 queueID) = 0;

		/** Setting carried over from ParticleSystem.
		*/
		virtual void setKeepParticlesInLocalSpace(bool keepLocal) = 0;

        /** Gets the desired particles sort mode of this renderer */
        virtual SortMode _getSortMode(void) const = 0;

		/** Required method to allow the renderer to communicate the Renderables
			it will be using to render the system to a visitor.
		@see MovableObject::visitRenderables
		*/
		virtual void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false) = 0;

    };

    /** Abstract class definition of a factory object for ParticleSystemRenderer. */
    class _OgreExport ParticleSystemRendererFactory : public FactoryObj<ParticleSystemRenderer>, public FXAlloc
    {
    public:
        // No methods, must just override all methods inherited from FactoryObj
    };
	/** @} */
	/** @} */

}

#endif
