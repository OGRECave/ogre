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
#ifndef __ParticleSystemRenderer_H__
#define __ParticleSystemRenderer_H__

#include "OgrePrerequisites.h"
#include "OgreStringInterface.h"
#include "OgreFactoryObj.h"
#include "OgreRenderQueue.h"
#include "OgreCommon.h"
#include "OgreRenderable.h"

namespace Ogre {

    typedef FastArray<Renderable*> RenderableArray;

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
        virtual void _updateRenderQueue(RenderQueue* queue, Camera *camera,
            const Camera *lodCamera, list<Particle*>::type& currentParticles,
            bool cullIndividually, RenderableArray &outRenderables ) = 0;

        /** Sets the HLMS material this renderer must use; called by ParticleSystem. */
        virtual void _setDatablock( HlmsDatablock *datablock ) = 0;
        /** Sets the material this renderer must use; called by ParticleSystem. */
        virtual void _setMaterialName( const String &matName, const String &resourceGroup ) = 0;
        /** Delegated to by ParticleSystem::_notifyCurrentCamera */
        virtual void _notifyCurrentCamera(const Camera* cam) = 0;
        /** Delegated to by ParticleSystem::_notifyAttached */
        virtual void _notifyAttached(Node* parent) = 0;
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

        virtual void setRenderQueueSubGroup( uint8 subGroupId ) = 0;

        /** Setting carried over from ParticleSystem.
        */
        virtual void setKeepParticlesInLocalSpace(bool keepLocal) = 0;

        /** Gets the desired particles sort mode of this renderer */
        virtual SortMode _getSortMode(void) const = 0;

    };

    /** Abstract class definition of a factory object for ParticleSystemRenderer. */
    class _OgreExport ParticleSystemRendererFactory : public FactoryObj<ParticleSystemRenderer>, public FXAlloc
    {
    public:
        // No methods, must just override all methods inherited from FactoryObj
        ParticleSystemRendererFactory() : mCurrentSceneManager(0) {}

        /// Needs to be set directly before calling createInstance
        SceneManager *mCurrentSceneManager;
    };
    /** @} */
    /** @} */

}

#endif
