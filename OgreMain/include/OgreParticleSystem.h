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
#ifndef __ParticleSystem_H__
#define __ParticleSystem_H__

#include "OgrePrerequisites.h"

#include "OgreVector.h"
#include "OgreParticleIterator.h"
#include "OgreStringInterface.h"
#include "OgreMovableObject.h"
#include "OgreRadixSort.h"
#include "OgreResourceGroupManager.h"
#include "OgreHeaderPrefix.h"


namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    /** Class defining particle system based special effects.
    @remarks
        Particle systems are special effects generators which are based on a 
        number of moving points to create the impression of things like like 
        sparkles, smoke, blood spurts, dust etc.
    @par
        This class simply manages a single collection of particles in world space
        with a shared local origin for emission. The visual aspect of the 
        particles is handled by a ParticleSystemRenderer instance.
    @par
        Particle systems are created using the SceneManager, never directly.
        In addition, like all subclasses of MovableObject, the ParticleSystem 
        will only be considered for rendering once it has been attached to a 
        SceneNode. 
    */
    class _OgreExport ParticleSystem : public StringInterface, public MovableObject
    {
    public:

        /** Command object for quota (see ParamCommand).*/
        class _OgrePrivate CmdQuota : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for emittedEmitterQuota (see ParamCommand).*/
        class _OgrePrivate CmdEmittedEmitterQuota : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for material (see ParamCommand).*/
        class _OgrePrivate CmdMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for cull_each (see ParamCommand).*/
        class _OgrePrivate CmdCull : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_width (see ParamCommand).*/
        class _OgrePrivate CmdWidth : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_height (see ParamCommand).*/
        class _OgrePrivate CmdHeight : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for renderer (see ParamCommand).*/
        class _OgrePrivate CmdRenderer : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for sorting (see ParamCommand).*/
        class CmdSorted : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for local space (see ParamCommand).*/
        class CmdLocalSpace : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for iteration interval(see ParamCommand).*/
        class CmdIterationInterval : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for nonvisible timeout (see ParamCommand).*/
        class CmdNonvisibleTimeout : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /// Default constructor required for STL creation in manager
        ParticleSystem();
        /** Creates a particle system with no emitters or affectors.
        @remarks
            You should use the ParticleSystemManager to create particle systems rather than creating
            them directly.
        */
        ParticleSystem(const String& name, const String& resourceGroupName);

        virtual ~ParticleSystem();

        /** Sets the ParticleRenderer to be used to render this particle system.
        @remarks
            The main ParticleSystem just manages the creation and movement of 
            particles; they are rendered using functions in ParticleRenderer
            and the ParticleVisual instances they create.
        @param typeName String identifying the type of renderer to use; a new 
            instance of this type will be created; a factory must have been registered
            with ParticleSystemManager.
        */
        void setRenderer(const String& typeName);

        /** Gets the ParticleRenderer to be used to render this particle system. */
        ParticleSystemRenderer* getRenderer(void) const;
        /** Gets the name of the ParticleRenderer to be used to render this particle system. */
        const String& getRendererName(void) const;

        /** Adds an emitter to this particle system.
        @remarks
            Particles are created in a particle system by emitters - see the ParticleEmitter
            class for more details.
        @param 
            emitterType String identifying the emitter type to create. Emitter types are defined
            by registering new factories with the manager - see ParticleEmitterFactory for more details.
            Emitter types can be extended by OGRE, plugin authors or application developers.
        */
        ParticleEmitter* addEmitter(const String& emitterType);

        /** Retrieves an emitter by it's index (zero-based).
        @remarks
            Used to retrieve a pointer to an emitter for a particle system to procedurally change
            emission parameters etc.
            You should check how many emitters are registered against this system before calling
            this method with an arbitrary index using getNumEmitters.
        @param
            index Zero-based index of the emitter to retrieve.
        */
        ParticleEmitter* getEmitter(unsigned short index) const;

        /** Returns the number of emitters for this particle system. */
        unsigned short getNumEmitters(void) const;

        /** Removes an emitter from the system.
        @remarks
            Drops the emitter with the index specified from this system.
            You should check how many emitters are registered against this system before calling
            this method with an arbitrary index using getNumEmitters.
        @param
            index Zero-based index of the emitter to retrieve.
        */
        void removeEmitter(unsigned short index);

        /** Removes all the emitters from this system. */
        void removeAllEmitters(void);

        /** Removes an emitter from the system.
        @remarks
            Drops the emitter from this system.
        @param
            emitter Pointer to a particle emitter.
        */
        void removeEmitter(ParticleEmitter *emitter);
        
        /** Adds an affector to this particle system.
        @remarks
            Particles are modified over time in a particle system by affectors - see the ParticleAffector
            class for more details.
        @param 
            affectorType String identifying the affector type to create. Affector types are defined
            by registering new factories with the manager - see ParticleAffectorFactory for more details.
            Affector types can be extended by OGRE, plugin authors or application developers.
        */
        ParticleAffector* addAffector(const String& affectorType);

        /** Retrieves an affector by it's index (zero-based).
        @remarks
            Used to retrieve a pointer to an affector for a particle system to procedurally change
            affector parameters etc.
            You should check how many affectors are registered against this system before calling
            this method with an arbitrary index using getNumAffectors.
        @param
            index Zero-based index of the affector to retrieve.
        */
        ParticleAffector* getAffector(unsigned short index) const;

        /** Returns the number of affectors for this particle system. */
        unsigned short getNumAffectors(void) const;

        /** Removes an affector from the system.
        @remarks
            Drops the affector with the index specified from this system.
            You should check how many affectors are registered against this system before calling
            this method with an arbitrary index using getNumAffectors.
        @param
            index Zero-based index of the affector to retrieve.
        */
        void removeAffector(unsigned short index);

        /** Removes all the affectors from this system. */
        void removeAllAffectors(void);

        /** Empties this set of all particles.
        */
        void clear();

        /** Gets the number of individual particles in the system right now.
        @remarks
            The number of particles active in a system at a point in time depends on 
            the number of emitters, their emission rates, the time-to-live (TTL) each particle is
            given on emission (and whether any affectors modify that TTL) and the maximum
            number of particles allowed in this system at once (particle quota).
        */
        size_t getNumParticles(void) const;

        /** Manually add a particle to the system. 
        @remarks
            Instead of using an emitter, you can manually add a particle to the system.
            You must initialise the returned particle instance immediately with the
            'emission' state.
        @note
            There is no corresponding 'destroyParticle' method - if you want to dispose of a
            particle manually (say, if you've used setSpeedFactor(0) to make particles live forever)
            you should use getParticle() and modify it's timeToLive to zero, meaning that it will
            get cleaned up in the next update.
        */
        Particle* createParticle(void);

        /** Manually add an emitter particle to the system. 
        @remarks
            The purpose of a particle emitter is to emit particles. Besides visual particles, also other other
            particle types can be emitted, other emitters for example. The emitted emitters have a double role;
            they behave as particles and can be influenced by affectors, but they are still emitters and capable 
            to emit other particles (or emitters). It is possible to create a chain of emitters - emitters 
            emitting other emitters, which also emit emitters.
        @param emitterName The name of a particle emitter that must be emitted.
        */
        Particle* createEmitterParticle(const String& emitterName);

        /** Retrieve a particle from the system for manual tweaking.
        @remarks
            Normally you use an affector to alter particles in flight, but
            for small manually controlled particle systems you might want to use
            this method.
        */
        Particle* getParticle(size_t index);

        /** Returns the maximum number of particles this system is allowed to have active at once.
        @remarks
            See ParticleSystem::setParticleQuota for more info.
        */
        size_t getParticleQuota(void) const;

        /** Sets the maximum number of particles this system is allowed to have active at once.
        @remarks
            Particle systems all have a particle quota, i.e. a maximum number of particles they are 
            allowed to have active at a time. This allows the application to set a keep particle systems
            under control should they be affected by complex parameters which alter their emission rates
            etc. If a particle system reaches it's particle quota, none of the emitters will be able to 
            emit any more particles. As existing particles die, the spare capacity will be allocated
            equally across all emitters to be as consistent to the original particle system style as possible.
            The quota can be increased but not decreased after the system has been created.
        @param quota The maximum number of particles this system is allowed to have.
        */
        void setParticleQuota(size_t quota);

        /** Returns the maximum number of emitted emitters this system is allowed to have active at once.
        @remarks
            See ParticleSystem::setEmittedEmitterQuota for more info.
        */
        size_t getEmittedEmitterQuota(void) const;

        /** Sets the maximum number of emitted emitters this system is allowed to have active at once.
        @remarks
            Particle systems can have - besides a particle quota - also an emitted emitter quota.
        @param quota The maximum number of emitted emitters this system is allowed to have.
        */
        void setEmittedEmitterQuota(size_t quota);

        /** Assignment operator for copying.
        @remarks
            This operator deep copies all particle emitters and effectors, but not particles. The
            system's name is also not copied.
        */
        ParticleSystem& operator=(const ParticleSystem& rhs);

        /** Updates the particles in the system based on time elapsed.
        @remarks
            This is called automatically every frame by OGRE.
        @param
            timeElapsed The amount of time, in seconds, since the last frame.
        */
        void _update(Real timeElapsed);

        /** Returns an iterator for stepping through all particles in this system.
        @remarks
            This method is designed to be used by people providing new ParticleAffector subclasses,
            this is the easiest way to step through all the particles in a system and apply the
            changes the affector wants to make.
        */
        ParticleIterator _getIterator(void);

        /** Sets the name of the material to be used for this billboard set.
        */
        virtual void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Sets the name of the material to be used for this billboard set.
            @return The name of the material that is used for this set.
        */
        virtual const String& getMaterialName(void) const;

        virtual void _notifyCurrentCamera(Camera* cam) override;
        void _notifyAttached(Node* parent, bool isTagPoint = false) override;
        virtual const AxisAlignedBox& getBoundingBox(void) const override { return mAABB; }
        virtual Real getBoundingRadius(void) const override { return mBoundingRadius; }
        virtual void _updateRenderQueue(RenderQueue* queue) override;

        /// @copydoc MovableObject::visitRenderables
        void visitRenderables(Renderable::Visitor* visitor, 
            bool debugRenderables = false);

        /** Fast-forwards this system by the required number of seconds.
        @remarks
            This method allows you to fast-forward a system so that it effectively looks like
            it has already been running for the time you specify. This is useful to avoid the
            'startup sequence' of a system, when you want the system to be fully populated right
            from the start.
        @param
            time The number of seconds to fast-forward by.
        @param
            interval The sampling interval used to generate particles, apply affectors etc. The lower this
            is the more realistic the fast-forward, but it takes more iterations to do it.
        */
        void fastForward(Real time, Real interval = 0.1f);

        /** Sets a 'speed factor' on this particle system, which means it scales the elapsed
            real time which has passed by this factor before passing it to the emitters, affectors,
            and the particle life calculation.
        @remarks
            An interesting side effect - if you want to create a completely manual particle system
            where you control the emission and life of particles yourself, you can set the speed
            factor to 0.0f, thus disabling normal particle emission, alteration, and death.
        */
        void setSpeedFactor(Real speedFactor) { mSpeedFactor = speedFactor; }

        /** Gets the 'speed factor' on this particle system.
        */
        Real getSpeedFactor(void) const { return mSpeedFactor; }

        /** Sets a 'iteration interval' on this particle system.
        @remarks
            The default Particle system update interval, based on elapsed frame time,
            will cause different behavior between low frame-rate and high frame-rate. 
            By using this option, you can make the particle system update at
            a fixed interval, keeping the behavior the same no matter what frame-rate 
            is.
        @par
            When iteration interval is set to zero, it means the update occurs based 
            on an elapsed frame time, otherwise each iteration will take place 
            at the given interval, repeating until it has used up all the elapsed 
            frame time.
        @param
            iterationInterval The iteration interval, default to zero.
        */
        void setIterationInterval(Real iterationInterval);

        /** Gets a 'iteration interval' on this particle system.
        */
        Real getIterationInterval(void) const { return mIterationInterval; }

        /** Set the default iteration interval for all ParticleSystem instances.
        */
        static void setDefaultIterationInterval(Real iterationInterval) { msDefaultIterationInterval = iterationInterval; }

        /** Get the default iteration interval for all ParticleSystem instances.
        */
        static Real getDefaultIterationInterval(void) { return msDefaultIterationInterval; }

        /** Sets when the particle system should stop updating after it hasn't been
            visible for a while.
        @remarks
            By default, visible particle systems update all the time, even when 
            not in view. This means that they are guaranteed to be consistent when 
            they do enter view. However, this comes at a cost, updating particle
            systems can be expensive, especially if they are perpetual.
        @par
            This option lets you set a 'timeout' on the particle system, so that
            if it isn't visible for this amount of time, it will stop updating
            until it is next visible.
        @param timeout The time after which the particle system will be disabled
            if it is no longer visible. 0 to disable the timeout and always update.
        */
        void setNonVisibleUpdateTimeout(Real timeout);
        /** Gets when the particle system should stop updating after it hasn't been
            visible for a while.
        */
        Real getNonVisibleUpdateTimeout(void) const { return mNonvisibleTimeout; }

        /** Set the default nonvisible timeout for all ParticleSystem instances.
        */
        static void setDefaultNonVisibleUpdateTimeout(Real timeout) 
        { msDefaultNonvisibleTimeout = timeout; }

        /** Get the default nonvisible timeout for all ParticleSystem instances.
        */
        static Real getDefaultNonVisibleUpdateTimeout(void) { return msDefaultNonvisibleTimeout; }

        const String& getMovableType(void) const override;

        /** Internal callback used by Particles to notify their parent that they have been resized.
        */
        virtual void _notifyParticleResized(void);

        /** Internal callback used by Particles to notify their parent that they have been rotated.
        */
        virtual void _notifyParticleRotated(void);

        /** Sets the default dimensions of the particles in this set.
            @remarks
                All particles in a set are created with these default dimensions. The set will render most efficiently if
                all the particles in the set are the default size. It is possible to alter the size of individual
                particles at the expense of extra calculation. See the Particle class for more info.
            @param width
                The new default width for the particles in this set. Must be non-negative!
            @param height
                The new default height for the particles in this set. Must be non-negative!
        */
        virtual void setDefaultDimensions(Real width, Real height);

        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultWidth(Real width);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultWidth(void) const;
        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultHeight(Real height);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultHeight(void) const;
        /** Returns whether or not particles in this are tested individually for culling. */
        virtual bool getCullIndividually(void) const;
        /** Sets whether culling tests particles in this individually as well as in a group.
        @remarks
            Particle sets are always culled as a whole group, based on a bounding box which 
            encloses all particles in the set. For fairly localised sets, this is enough. However, you
            can optionally tell the set to also cull individual particles in the set, i.e. to test
            each individual particle before rendering. The default is not to do this.
        @par
            This is useful when you have a large, fairly distributed set of particles, like maybe 
            trees on a landscape. You probably still want to group them into more than one
            set (maybe one set per section of landscape), which will be culled coarsely, but you also
            want to cull the particles individually because they are spread out. Whilst you could have
            lots of single-tree sets which are culled separately, this would be inefficient to render
            because each tree would be issued as it's own rendering operation.
        @par
            By calling this method with a parameter of true, you can have large particle sets which 
            are spaced out and so get the benefit of batch rendering and coarse culling, but also have
            fine-grained culling so unnecessary rendering is avoided.
        @param cullIndividual If true, each particle is tested before being sent to the pipeline as well 
            as the whole set having to pass the coarse group bounding test.
        */
        virtual void setCullIndividually(bool cullIndividual);
        /// Return the resource group to be used to load dependent resources
        virtual const String& getResourceGroupName(void) const { return mResourceGroupName; }
        /** Get the origin of this particle system, e.g. a script file name.
        @remarks
            This property will only contain something if the creator of
            this particle system chose to populate it. Script loaders are advised
            to populate it.
        */
        const String& getOrigin(void) const { return mOrigin; }
        /// Notify this particle system of it's origin
        void _notifyOrigin(const String& origin) { mOrigin = origin; }

        /** @copydoc MovableObject::setRenderQueueGroup */
        void setRenderQueueGroup(uint8 queueID);
        /** @copydoc MovableObject::setRenderQueueGroupAndPriority */
        void setRenderQueueGroupAndPriority(uint8 queueID, ushort priority);

        /** Set whether or not particles are sorted according to the camera.
        @remarks
            Enabling sorting alters the order particles are sent to the renderer.
            When enabled, particles are sent to the renderer in order of 
            furthest distance from the camera.
        */
        void setSortingEnabled(bool enabled) { mSorted = enabled; }
        /// Gets whether particles are sorted relative to the camera.
        bool getSortingEnabled(void) const { return mSorted; }

        /** Set the (initial) bounds of the particle system manually. 
        @remarks
            If you can, set the bounds of a particle system up-front and 
            call setBoundsAutoUpdated(false); this is the most efficient way to
            organise it. Otherwise, set an initial bounds and let the bounds increase
            for a little while (the default is 5 seconds), after which time the 
            AABB is fixed to save time.
        @param aabb Bounds in local space.
        */
        void setBounds(const AxisAlignedBox& aabb);

        /** Sets whether the bounds will be automatically updated
            for the life of the particle system
        @remarks
            If you have a stationary particle system, it would be a good idea to
            call this method and set the value to 'false', since the maximum
            bounds of the particle system will eventually be static. If you do
            this, you can either set the bounds manually using the setBounds()
            method, or set the second parameter of this method to a positive
            number of seconds, so that the bounds are calculated for a few
            seconds and then frozen.
        @param autoUpdate If true (the default), the particle system will
            update it's bounds every frame. If false, the bounds update will 
            cease after the 'stopIn' number of seconds have passed.
        @param stopIn Only applicable if the first parameter is true, this is the
            number of seconds after which the automatic update will cease.
        */
        void setBoundsAutoUpdated(bool autoUpdate, Real stopIn = 0.0f);

        /** Sets whether particles (and any affector effects) remain relative 
            to the node the particle system is attached to.
        @remarks
            By default particles are in world space once emitted, so they are not
            affected by movement in the parent node of the particle system. This
            makes the most sense when dealing with completely independent particles, 
            but if you want to constrain them to follow local motion too, you
            can set this to true.
        */
        void setKeepParticlesInLocalSpace(bool keepLocal);

        /** Gets whether particles (and any affector effects) remain relative 
            to the node the particle system is attached to.
        */
        bool getKeepParticlesInLocalSpace(void) const { return mLocalSpace; }

        /** Internal method for updating the bounds of the particle system.
        @remarks
            This is called automatically for a period of time after the system's
            creation (10 seconds by default, settable by setBoundsAutoUpdated) 
            to increase (and only increase) the bounds of the system according 
            to the emitted and affected particles. After this period, the 
            system is assumed to achieved its maximum size, and the bounds are
            no longer computed for efficiency. You can tweak the behaviour by 
            either setting the bounds manually (setBounds, preferred), or 
            changing the time over which the bounds are updated (performance cost).
            You can also call this method manually if you need to update the 
            bounds on an ad-hoc basis.
        */
        void _updateBounds(void);

        /** This is used to turn on or off particle emission for this system.
        @remarks
            By default particle system is always emitting particles (if a emitters exists)
            and this can be used to stop the emission for all emitters. To turn it on again, 
            call it passing true.

            Note that this does not detach the particle system from the scene node, it will 
            still use some CPU.
        */
        void setEmitting(bool v);

        /** Returns true if the particle system emitting flag is turned on.
        @remarks
            This function will not actually return whether the particles are being emitted.
            It only returns the value of emitting flag.
        */
        bool getEmitting() const;

        /// Override to return specific type flag
        uint32 getTypeFlags(void) const;
    protected:

        /// Command objects
        static CmdCull msCullCmd;
        static CmdHeight msHeightCmd;
        static CmdMaterial msMaterialCmd;
        static CmdQuota msQuotaCmd;
        static CmdEmittedEmitterQuota msEmittedEmitterQuotaCmd;
        static CmdWidth msWidthCmd;
        static CmdRenderer msRendererCmd;
        static CmdSorted msSortedCmd;
        static CmdLocalSpace msLocalSpaceCmd;
        static CmdIterationInterval msIterationIntervalCmd;
        static CmdNonvisibleTimeout msNonvisibleTimeoutCmd;


        AxisAlignedBox mAABB;
        Real mBoundingRadius;
        bool mBoundsAutoUpdate;
        Real mBoundsUpdateTime;
        Real mUpdateRemainTime;

        /// Name of the resource group to use to load materials
        String mResourceGroupName;
        /// Have we set the material etc on the renderer?
        bool mIsRendererConfigured;
        /// Pointer to the material to use
        MaterialPtr mMaterial;
        /// Default width of each particle
        Real mDefaultWidth;
        /// Default height of each particle
        Real mDefaultHeight;
        /// Speed factor
        Real mSpeedFactor;
        /// Iteration interval
        Real mIterationInterval;
        /// Iteration interval set? Otherwise track default
        bool mIterationIntervalSet;
        /// Particles sorted according to camera?
        bool mSorted;
        /// Particles in local space?
        bool mLocalSpace;
        /// Update timeout when nonvisible (0 for no timeout)
        Real mNonvisibleTimeout;
        /// Update timeout when nonvisible set? Otherwise track default
        bool mNonvisibleTimeoutSet;
        /// Amount of time non-visible so far
        Real mTimeSinceLastVisible;
        /// Last frame in which known to be visible
        unsigned long mLastVisibleFrame;
        /// Controller for time update
        Controller<Real>* mTimeController;
        /// Indication whether the emitted emitter pool (= pool with particle emitters that are emitted) is initialised
        bool mEmittedEmitterPoolInitialised;
        /// Used to control if the particle system should emit particles or not.
        bool mIsEmitting;

        typedef std::list<Particle*> ActiveParticleList;
        typedef std::list<Particle*> FreeParticleList;
        typedef std::vector<Particle*> ParticlePool;

        /** Sort by direction functor */
        struct SortByDirectionFunctor
        {
            /// Direction to sort in
            Vector3 sortDir;

            SortByDirectionFunctor(const Vector3& dir);
            float operator()(Particle* p) const;
        };

        /** Sort by distance functor */
        struct SortByDistanceFunctor
        {
            /// Position to sort in
            Vector3 sortPos;

            SortByDistanceFunctor(const Vector3& pos);
            float operator()(Particle* p) const;
        };

        static RadixSort<ActiveParticleList, Particle*, float> mRadixSorter;

        /** Active particle list.
            @remarks
                This is a linked list of pointers to particles in the particle pool.
            @par
                This allows very fast insertions and deletions from anywhere in 
                the list to activate / deactivate particles as well as reuse of 
                Particle instances in the pool without construction & destruction 
                which avoids memory thrashing.
        */
        ActiveParticleList mActiveParticles;

        /** Free particle queue.
            @remarks
                This contains a list of the particles free for use as new instances
                as required by the set. Particle instances are preconstructed up 
                to the estimated size in the mParticlePool vector and are 
                referenced on this deque at startup. As they get used this list
                reduces, as they get released back to to the set they get added
                back to the list.
        */
        FreeParticleList mFreeParticles;

        /** Pool of particle instances for use and reuse in the active particle list.
            @remarks
                This vector will be preallocated with the estimated size of the set,and will extend as required.
        */
        ParticlePool mParticlePool;

        typedef std::list<ParticleEmitter*> FreeEmittedEmitterList;
        typedef std::list<ParticleEmitter*> ActiveEmittedEmitterList;
        typedef std::vector<ParticleEmitter*> EmittedEmitterList;
        typedef std::map<String, FreeEmittedEmitterList> FreeEmittedEmitterMap;
        typedef std::map<String, EmittedEmitterList> EmittedEmitterPool;

        /** Pool of emitted emitters for use and reuse in the active emitted emitter list.
        @remarks
            The emitters in this pool act as particles and as emitters. The pool is a map containing lists 
            of emitters, identified by their name.
        @par
            The emitters in this pool are cloned using emitters that are kept in the main emitter list
            of the ParticleSystem.
        */
        EmittedEmitterPool mEmittedEmitterPool;

        /** Free emitted emitter list.
            @remarks
                This contains a list of the emitters free for use as new instances as required by the set.
        */
        FreeEmittedEmitterMap mFreeEmittedEmitters;

        /** Active emitted emitter list.
            @remarks
                This is a linked list of pointers to emitters in the emitted emitter pool.
                Emitters that are used are stored (their pointers) in both the list with active particles and in 
                the list with active emitted emitters.        */
        ActiveEmittedEmitterList mActiveEmittedEmitters;

        typedef std::vector<ParticleEmitter*> ParticleEmitterList;
        typedef std::vector<ParticleAffector*> ParticleAffectorList;
        
        /// List of particle emitters, ie sources of particles
        ParticleEmitterList mEmitters;
        /// List of particle affectors, ie modifiers of particles
        ParticleAffectorList mAffectors;

        /// The renderer used to render this particle system
        ParticleSystemRenderer* mRenderer;

        /// Do we cull each particle individually?
        bool mCullIndividual;

        /// The name of the type of renderer used to render this system
        String mRendererType;
        
        /// The number of particles in the pool.
        size_t mPoolSize;

        /// The number of emitted emitters in the pool.
        size_t mEmittedEmitterPoolSize;

        /// Optional origin of this particle system (eg script name)
        String mOrigin;

        /// Default iteration interval
        static Real msDefaultIterationInterval;
        /// Default nonvisible update timeout
        static Real msDefaultNonvisibleTimeout;

        /** Internal method used to expire dead particles. */
        void _expire(Real timeElapsed);

        /** Spawn new particles based on free quota and emitter requirements. */
        void _triggerEmitters(Real timeElapsed);

        /** Helper function that actually performs the emission of particles
        */
        void _executeTriggerEmitters(ParticleEmitter* emitter, unsigned requested, Real timeElapsed);

        /** Updates existing particle based on their momentum. */
        void _applyMotion(Real timeElapsed);

        /** Applies the effects of affectors. */
        void _triggerAffectors(Real timeElapsed);

        /** Sort the particles in the system **/
        void _sortParticles(Camera* cam);

        /** Resize the internal pool of particles. */
        void increasePool(size_t size);

        /** Resize the internal pool of emitted emitters.
            @remarks
                The pool consists of multiple vectors containing pointers to particle emitters. Increasing the 
                pool with size implies that the vectors are equally increased. The quota of emitted emitters is 
                defined on a particle system level and not on a particle emitter level. This is to prevent that
                the number of created emitters becomes too high; the quota is shared amongst the emitted emitters.
        */
        void increaseEmittedEmitterPool(size_t size);

        /** Internal method for initialising string interface. */
        void initParameters(void);

        /** Internal method to configure the renderer. */
        void configureRenderer(void);

        /// Internal method for creating ParticleVisualData instances for the pool
        void createVisualParticles(size_t poolstart, size_t poolend);
        /// Internal method for destroying ParticleVisualData instances for the pool
        void destroyVisualParticles(size_t poolstart, size_t poolend);

        /** Create a pool of emitted emitters and assign them to the free emitter list.
            @remarks
                The emitters in the pool are grouped by name. This name is the name of the base emitter in the
                main list with particle emitters, which forms the template of the created emitted emitters.
        */
        void initialiseEmittedEmitters(void);

        /** Determine which emitters in the Particle Systems main emitter become a template for creating an
            pool of emitters that can be emitted.
        */
        void initialiseEmittedEmitterPool(void);

        /** Add  emitters from the pool to the free emitted emitter queue. */
        void addFreeEmittedEmitters(void);

        /** Removes all emitted emitters from this system.  */
        void removeAllEmittedEmitters(void);

        /** Find the list with free emitted emitters.
            @param name The name that identifies the list with free emitted emitters.
        */
        FreeEmittedEmitterList* findFreeEmittedEmitter (const String& name);

        /** Removes an emitter from the active emitted emitter list.
            @remarks
                The emitter will not be destroyed!
            @param emitter Pointer to a particle emitter.
        */
        void removeFromActiveEmittedEmitters (ParticleEmitter* emitter);

        /** Moves all emitted emitters from the active list to the free list
            @remarks
                The active emitted emitter list will not be cleared and still keeps references to the emitters!
        */
        void addActiveEmittedEmittersToFreeList (void);

        /** This function clears all data structures that are used in combination with emitted emitters and
            sets the flag to indicate that the emitted emitter pool must be initialised again.
            @remarks
                This function should be called if new emitters are added to a ParticleSystem or deleted from a
                ParticleSystem. The emitted emitter data structures become out of sync and need to be build up
                again. The data structures are not reorganised in this function, but by setting a flag, 
                they are rebuild in the regular process flow.
        */
        void _notifyReorganiseEmittedEmitterData (void);
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
