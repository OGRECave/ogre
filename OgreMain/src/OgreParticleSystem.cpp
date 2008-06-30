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
#include "OgreStableHeaders.h"

#include "OgreParticleSystem.h"
#include "OgreParticleSystemManager.h"
#include "OgreRenderQueue.h"
#include "OgreBillboardSet.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreParticle.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreControllerManager.h"
#include "OgreRoot.h"

namespace Ogre {
    // Init statics
    ParticleSystem::CmdCull ParticleSystem::msCullCmd;
    ParticleSystem::CmdHeight ParticleSystem::msHeightCmd;
    ParticleSystem::CmdMaterial ParticleSystem::msMaterialCmd;
    ParticleSystem::CmdQuota ParticleSystem::msQuotaCmd;
	ParticleSystem::CmdEmittedEmitterQuota ParticleSystem::msEmittedEmitterQuotaCmd;
    ParticleSystem::CmdWidth ParticleSystem::msWidthCmd;
    ParticleSystem::CmdRenderer ParticleSystem::msRendererCmd;
	ParticleSystem::CmdSorted ParticleSystem::msSortedCmd;
	ParticleSystem::CmdLocalSpace ParticleSystem::msLocalSpaceCmd;
	ParticleSystem::CmdIterationInterval ParticleSystem::msIterationIntervalCmd;
	ParticleSystem::CmdNonvisibleTimeout ParticleSystem::msNonvisibleTimeoutCmd;

    RadixSort<ParticleSystem::ActiveParticleList, Particle*, float> ParticleSystem::mRadixSorter;

    Real ParticleSystem::msDefaultIterationInterval = 0;
    Real ParticleSystem::msDefaultNonvisibleTimeout = 0;

	//-----------------------------------------------------------------------
	// Local class for updating based on time
	class ParticleSystemUpdateValue : public ControllerValue<Real>
	{
	protected:
		ParticleSystem* mTarget;
	public:
		ParticleSystemUpdateValue(ParticleSystem* target) : mTarget(target) {}

		Real getValue(void) const { return 0; } // N/A

		void setValue(Real value) { mTarget->_update(value); }

	};
    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem() 
      : mAABB(),
        mBoundingRadius(1.0f),
        mBoundsAutoUpdate(true),
        mBoundsUpdateTime(10.0f),
        mUpdateRemainTime(0),
        mWorldAABB(),
        mResourceGroupName(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME),
        mIsRendererConfigured(false),
        mSpeedFactor(1.0f),
        mIterationInterval(0),
		mIterationIntervalSet(false),
        mSorted(false),
        mLocalSpace(false),
		mNonvisibleTimeout(0),
		mNonvisibleTimeoutSet(false),
		mTimeSinceLastVisible(0),
		mLastVisibleFrame(0),
        mTimeController(0),
		mEmittedEmitterPoolInitialised(false),
        mRenderer(0),
        mCullIndividual(false),
        mPoolSize(0),
		mEmittedEmitterPoolSize(0)
	{
        initParameters();

        // Default to billboard renderer
        setRenderer("billboard");

    }
    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem(const String& name, const String& resourceGroup)
      : MovableObject(name),
        mAABB(),
        mBoundingRadius(1.0f),
        mBoundsAutoUpdate(true),
        mBoundsUpdateTime(10.0f),
        mUpdateRemainTime(0),
        mWorldAABB(),
        mResourceGroupName(resourceGroup),
        mIsRendererConfigured(false),
        mSpeedFactor(1.0f),
        mIterationInterval(0),
		mIterationIntervalSet(false),
        mSorted(false),
        mLocalSpace(false),
		mNonvisibleTimeout(0),
		mNonvisibleTimeoutSet(false),
		mTimeSinceLastVisible(0),
		mLastVisibleFrame(Root::getSingleton().getNextFrameNumber()),
        mTimeController(0),
		mEmittedEmitterPoolInitialised(false),
        mRenderer(0), 
		mCullIndividual(false),
        mPoolSize(0),
		mEmittedEmitterPoolSize(0)
    {
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        // Default to 10 particles, expect app to specify (will only be increased, not decreased)
        setParticleQuota( 10 );
		setEmittedEmitterQuota( 3 );
        initParameters();

        // Default to billboard renderer
        setRenderer("billboard");
    }
    //-----------------------------------------------------------------------
    ParticleSystem::~ParticleSystem()
    {
        if (mTimeController)
        {
            // Destroy controller
            ControllerManager::getSingleton().destroyController(mTimeController);
            mTimeController = 0;
        }

		// Arrange for the deletion of emitters & affectors
        removeAllEmitters();
		removeAllEmittedEmitters();
        removeAllAffectors();

		// Deallocate all particles
		destroyVisualParticles(0, mParticlePool.size());
        // Free pool items
        ParticlePool::iterator i;
        for (i = mParticlePool.begin(); i != mParticlePool.end(); ++i)
        {
            OGRE_DELETE *i;
        }

        if (mRenderer)
        {
            ParticleSystemManager::getSingleton()._destroyRenderer(mRenderer);
            mRenderer = 0;
        }

    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::addEmitter(const String& emitterType)
    {
        ParticleEmitter* em = 
            ParticleSystemManager::getSingleton()._createEmitter(emitterType, this);
        mEmitters.push_back(em);
        return em;
    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::getEmitter(unsigned short index) const
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        return mEmitters[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumEmitters(void) const
    {
        return static_cast< unsigned short >( mEmitters.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeEmitter(unsigned short index)
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        ParticleEmitterList::iterator ei = mEmitters.begin() + index;
        ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        mEmitters.erase(ei);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllEmitters(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleEmitterList::iterator ei;
        for (ei = mEmitters.begin(); ei != mEmitters.end(); ++ei)
        {
            ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        }
        mEmitters.clear();
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::addAffector(const String& affectorType)
    {
        ParticleAffector* af = 
            ParticleSystemManager::getSingleton()._createAffector(affectorType, this);
        mAffectors.push_back(af);
        return af;
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::getAffector(unsigned short index) const
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        return mAffectors[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumAffectors(void) const
    {
        return static_cast< unsigned short >( mAffectors.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAffector(unsigned short index)
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        ParticleAffectorList::iterator ai = mAffectors.begin() + index;
        ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        mAffectors.erase(ai);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllAffectors(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleAffectorList::iterator ai;
        for (ai = mAffectors.begin(); ai != mAffectors.end(); ++ai)
        {
            ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        }
        mAffectors.clear();
    }
    //-----------------------------------------------------------------------
    ParticleSystem& ParticleSystem::operator=(const ParticleSystem& rhs)
    {
        // Blank this system's emitters & affectors
        removeAllEmitters();
		removeAllEmittedEmitters();
        removeAllAffectors();

        // Copy emitters
        unsigned int i;
        for(i = 0; i < rhs.getNumEmitters(); ++i)
        {
            ParticleEmitter* rhsEm = rhs.getEmitter(i);
            ParticleEmitter* newEm = addEmitter(rhsEm->getType());
            rhsEm->copyParametersTo(newEm);
        }
        // Copy affectors
        for(i = 0; i < rhs.getNumAffectors(); ++i)
        {
            ParticleAffector* rhsAf = rhs.getAffector(i);
            ParticleAffector* newAf = addAffector(rhsAf->getType());
            rhsAf->copyParametersTo(newAf);
        }
        setParticleQuota(rhs.getParticleQuota());
		setEmittedEmitterQuota(rhs.getEmittedEmitterQuota());
        setMaterialName(rhs.mMaterialName);
        setDefaultDimensions(rhs.mDefaultWidth, rhs.mDefaultHeight);
        mCullIndividual = rhs.mCullIndividual;
		mSorted = rhs.mSorted;
		mLocalSpace = rhs.mLocalSpace;
		mIterationInterval = rhs.mIterationInterval;
		mIterationIntervalSet = rhs.mIterationIntervalSet;
		mNonvisibleTimeout = rhs.mNonvisibleTimeout;
		mNonvisibleTimeoutSet = rhs.mNonvisibleTimeoutSet;
		// last frame visible and time since last visible should be left default

        setRenderer(rhs.getRendererName());
        // Copy settings
        if (mRenderer && rhs.getRenderer())
        {
            rhs.getRenderer()->copyParametersTo(mRenderer);
        }

        return *this;

    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getNumParticles(void) const
    {
        return mActiveParticles.size();
    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getParticleQuota(void) const
    {
        return mPoolSize;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setParticleQuota(size_t size)
    {
        // Never shrink below size()
        size_t currSize = mParticlePool.size();

        if( currSize < size )
        {
            // Will allocate particles on demand
            mPoolSize = size;
            
        }
    }
	//-----------------------------------------------------------------------
	size_t ParticleSystem::getEmittedEmitterQuota(void) const
	{
		return mEmittedEmitterPoolSize;
	}
	//-----------------------------------------------------------------------
	void ParticleSystem::setEmittedEmitterQuota(size_t size)
	{
		// Never shrink below size()
		EmittedEmitterPool::iterator i;
		size_t currSize = 0;
		for (i = mEmittedEmitterPool.begin(); i != mEmittedEmitterPool.end(); ++i)
		{
			currSize += i->second.size();
		}

		if( currSize < size )
		{
			// Will allocate emitted emitters on demand
			mEmittedEmitterPoolSize = size;
		}
	}
    //-----------------------------------------------------------------------
	void ParticleSystem::setNonVisibleUpdateTimeout(Real timeout)
	{
		mNonvisibleTimeout = timeout;
		mNonvisibleTimeoutSet = true;
	}
    //-----------------------------------------------------------------------
	void ParticleSystem::setIterationInterval(Real interval)
	{
		mIterationInterval = interval;
		mIterationIntervalSet = true;
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::_update(Real timeElapsed)
    {
        // Only update if attached to a node
        if (!mParentNode)
            return;

		Real nonvisibleTimeout = mNonvisibleTimeoutSet ?
			mNonvisibleTimeout : msDefaultNonvisibleTimeout;

		if (nonvisibleTimeout > 0)
		{
			// Check whether it's been more than one frame (update is ahead of
			// camera notification by one frame because of the ordering)
			long frameDiff = Root::getSingleton().getNextFrameNumber() - mLastVisibleFrame;
			if (frameDiff > 1 || frameDiff < 0) // < 0 if wrap only
			{
				mTimeSinceLastVisible += timeElapsed;
				if (mTimeSinceLastVisible >= nonvisibleTimeout)
				{
					// No update
					return;
				}
			}
		}

		// Scale incoming speed for the rest of the calculation
		timeElapsed *= mSpeedFactor;

        // Init renderer if not done already
        configureRenderer();

		// Initialise emitted emitters list if not done already
		initialiseEmittedEmitters();

		Real iterationInterval = mIterationIntervalSet ? 
            mIterationInterval : msDefaultIterationInterval;
        if (iterationInterval > 0)
        {
            mUpdateRemainTime += timeElapsed;

            while (mUpdateRemainTime >= iterationInterval)
            {
                // Update existing particles
                _expire(iterationInterval);
                _triggerAffectors(iterationInterval);
                _applyMotion(iterationInterval);
                // Emit new particles
                _triggerEmitters(iterationInterval);

                mUpdateRemainTime -= iterationInterval;
            }
        }
        else
        {
            // Update existing particles
            _expire(timeElapsed);
            _triggerAffectors(timeElapsed);
            _applyMotion(timeElapsed);
            // Emit new particles
            _triggerEmitters(timeElapsed);
        }

        if (!mBoundsAutoUpdate && mBoundsUpdateTime > 0.0f)
            mBoundsUpdateTime -= timeElapsed; // count down 
        _updateBounds();

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_expire(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;
		ParticleEmitter* pParticleEmitter;

        itEnd = mActiveParticles.end();

        for (i = mActiveParticles.begin(); i != itEnd; )
        {
            pParticle = static_cast<Particle*>(*i);
            if (pParticle->timeToLive < timeElapsed)
            {
                // Notify renderer
                mRenderer->_notifyParticleExpired(pParticle);

				// Identify the particle type
				if (pParticle->particleType == Particle::Visual)
				{
	                // Destroy this one
		            mFreeParticles.splice(mFreeParticles.end(), mActiveParticles, i++);
				}
				else
				{
					// For now, it can only be an emitted emitter
					pParticleEmitter = static_cast<ParticleEmitter*>(*i);
					std::list<ParticleEmitter*>* fee = findFreeEmittedEmitter(pParticleEmitter->getName());
					fee->push_back(pParticleEmitter);

					// Also erase from mActiveEmittedEmitters
					removeFromActiveEmittedEmitters (pParticleEmitter);

					// And erase from mActiveParticles
					i = mActiveParticles.erase( i );
				}
            }
            else
            {
                // Decrement TTL
                pParticle->timeToLive -= timeElapsed;
				++i;
            }

        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerEmitters(Real timeElapsed)
    {
        // Add up requests for emission
        static std::vector<unsigned> requested;
        if( requested.size() != mEmitters.size() )
            requested.resize( mEmitters.size() );

        size_t totalRequested, emitterCount, i, emissionAllowed;
        ParticleEmitterList::iterator itEmit, iEmitEnd;
		ActiveEmittedEmitterList::iterator itActiveEmit;
        iEmitEnd = mEmitters.end();
        emitterCount = mEmitters.size();
        emissionAllowed = mFreeParticles.size();
        totalRequested = 0;

        // Count up total requested emissions for regular emitters (and exclude the ones that are used as
		// a template for emitted emitters)
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
			if (!(*itEmit)->isEmitted())
			{
				requested[i] = (*itEmit)->_getEmissionCount(timeElapsed);
				totalRequested += requested[i];
			}
        }

		// Add up total requested emissions for (active) emitted emitters
		for (itActiveEmit = mActiveEmittedEmitters.begin(); itActiveEmit != mActiveEmittedEmitters.end(); ++itActiveEmit)
		{
			totalRequested += (*itActiveEmit)->_getEmissionCount(timeElapsed);
		}

        // Check if the quota will be exceeded, if so reduce demand
		Real ratio =  1.0f;
        if (totalRequested > emissionAllowed)
        {
            // Apportion down requested values to allotted values
            ratio =  (Real)emissionAllowed / (Real)totalRequested;
            for (i = 0; i < emitterCount; ++i)
            {
                requested[i] = static_cast<unsigned>(requested[i] * ratio);
            }
        }

        // Emit
		// For each emission, apply a subset of the motion for the frame
		// this ensures an even distribution of particles when many are
		// emitted in a single frame
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
			// Trigger the emitters, but exclude the emitters that are already in the emitted emitters list; 
			// they are handled in a separate loop
			if (!(*itEmit)->isEmitted())
				_executeTriggerEmitters (*itEmit, static_cast<unsigned>(requested[i]), timeElapsed);
        }

		// Do the same with all active emitted emitters
		for (itActiveEmit = mActiveEmittedEmitters.begin(), i = 0; itActiveEmit != mActiveEmittedEmitters.end(); ++itActiveEmit, ++i)
			_executeTriggerEmitters (*itActiveEmit, static_cast<unsigned>((*itActiveEmit)->_getEmissionCount(timeElapsed) * ratio), timeElapsed);
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::_executeTriggerEmitters(ParticleEmitter* emitter, unsigned requested, Real timeElapsed)
    {
		ParticleAffectorList::iterator	itAff, itAffEnd;
		Real timePoint = 0.0f;
		Real timeInc = timeElapsed / requested;

		for (unsigned int j = 0; j < requested; ++j)
		{
			// Create a new particle & init using emitter
			// The particle is a visual particle if the emit_emitter property of the emitter isn't set 
			Particle* p = 0;
			String	emitterName = emitter->getEmittedEmitter();
			if (emitterName == StringUtil::BLANK)
				p = createParticle();
			else
				p = createEmitterParticle(emitterName);

			// Only continue if the particle was really created (not null)
			if (!p)
				return;

			emitter->_initParticle(p);

			// Translate position & direction into world space
			if (!mLocalSpace)
			{
				p->position  = 
					(mParentNode->_getDerivedOrientation() *
					(mParentNode->_getDerivedScale() * p->position))
					+ mParentNode->_getDerivedPosition();
				p->direction = 
					(mParentNode->_getDerivedOrientation() * p->direction);
			}

			// apply partial frame motion to this particle
            p->position += (p->direction * timePoint);

			// apply particle initialization by the affectors
			itAffEnd = mAffectors.end();
			for (itAff = mAffectors.begin(); itAff != itAffEnd; ++itAff)
				(*itAff)->_initParticle(p);

			// Increment time fragment
			timePoint += timeInc;

			if (p->particleType == Particle::Emitter)
			{
				// If the particle is an emitter, the position on the emitter side must also be initialised
				// Note, that position of the emitter becomes a position in worldspace if mLocalSpace is set 
				// to false (will this become a problem?)
				ParticleEmitter* pParticleEmitter = static_cast<ParticleEmitter*>(p);
				pParticleEmitter->setPosition(p->position);
			}

            // Notify renderer
            mRenderer->_notifyParticleEmitted(p);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_applyMotion(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;
		ParticleEmitter* pParticleEmitter;

        itEnd = mActiveParticles.end();
        for (i = mActiveParticles.begin(); i != itEnd; ++i)
        {
            pParticle = static_cast<Particle*>(*i);
            pParticle->position += (pParticle->direction * timeElapsed);

			if (pParticle->particleType == Particle::Emitter)
			{
				// If it is an emitter, the emitter position must also be updated
				// Note, that position of the emitter becomes a position in worldspace if mLocalSpace is set 
				// to false (will this become a problem?)
				pParticleEmitter = static_cast<ParticleEmitter*>(*i);
				pParticleEmitter->setPosition(pParticle->position);
			}
        }

        // Notify renderer
        mRenderer->_notifyParticleMoved(mActiveParticles);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerAffectors(Real timeElapsed)
    {
        ParticleAffectorList::iterator i, itEnd;
        
        itEnd = mAffectors.end();
        for (i = mAffectors.begin(); i != itEnd; ++i)
        {
            (*i)->_affectParticles(this, timeElapsed);
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::increasePool(size_t size)
    {
        size_t oldSize = mParticlePool.size();

        // Increase size
        mParticlePool.reserve(size);
        mParticlePool.resize(size);

        // Create new particles
        for( size_t i = oldSize; i < size; i++ )
		{
            mParticlePool[i] = OGRE_NEW Particle();
		}

		if (mIsRendererConfigured)
		{
			createVisualParticles(oldSize, size);
		}


    }
    //-----------------------------------------------------------------------
    ParticleIterator ParticleSystem::_getIterator(void)
    {
        return ParticleIterator(mActiveParticles.begin(), mActiveParticles.end());
    }
    //-----------------------------------------------------------------------
	Particle* ParticleSystem::getParticle(size_t index) 
	{
		assert (index < mActiveParticles.size() && "Index out of bounds!");
		ActiveParticleList::iterator i = mActiveParticles.begin();
		std::advance(i, index);
		return *i;
	}
    //-----------------------------------------------------------------------
    Particle* ParticleSystem::createParticle(void)
    {
		Particle* p = 0;
		if (!mFreeParticles.empty())
		{
	        // Fast creation (don't use superclass since emitter will init)
	        p = mFreeParticles.front();
	        mActiveParticles.splice(mActiveParticles.end(), mFreeParticles, mFreeParticles.begin());

			p->_notifyOwner(this);
		}

        return p;

    }
    //-----------------------------------------------------------------------
    Particle* ParticleSystem::createEmitterParticle(const String& emitterName)
    {
		// Get the appropriate list and retrieve an emitter	
		Particle* p = 0;
		std::list<ParticleEmitter*>* fee = findFreeEmittedEmitter(emitterName);
		if (fee && !fee->empty())
		{
	        p = fee->front();
			p->particleType = Particle::Emitter;
			fee->pop_front();
			mActiveParticles.push_back(p);

			// Also add to mActiveEmittedEmitters. This is needed to traverse through all active emitters
			// that are emitted. Don't use mActiveParticles for that (although they are added to
			// mActiveParticles also), because it would take too long to traverse.
			mActiveEmittedEmitters.push_back(static_cast<ParticleEmitter*>(p));
			
			p->_notifyOwner(this);
		}

        return p;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateRenderQueue(RenderQueue* queue)
    {
        if (mRenderer)
        {
            mRenderer->_updateRenderQueue(queue, mActiveParticles, mCullIndividual);
        }
    }
	//---------------------------------------------------------------------
	void ParticleSystem::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		if (mRenderer)
		{
			mRenderer->visitRenderables(visitor, debugRenderables);
		}
	}
	//---------------------------------------------------------------------
    void ParticleSystem::initParameters(void)
    {
        if (createParamDictionary("ParticleSystem"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("quota", 
                "The maximum number of particle allowed at once in this system.",
                PT_UNSIGNED_INT),
                &msQuotaCmd);

            dict->addParameter(ParameterDef("emit_emitter_quota", 
                "The maximum number of emitters to be emitted at once in this system.",
                PT_UNSIGNED_INT),
				&msEmittedEmitterQuotaCmd);

			dict->addParameter(ParameterDef("material", 
                "The name of the material to be used to render all particles in this system.",
                PT_STRING),
                &msMaterialCmd);

            dict->addParameter(ParameterDef("particle_width", 
                "The width of particles in world units.",
                PT_REAL),
                &msWidthCmd);

            dict->addParameter(ParameterDef("particle_height", 
                "The height of particles in world units.",
                PT_REAL),
                &msHeightCmd);

            dict->addParameter(ParameterDef("cull_each", 
                "If true, each particle is culled in it's own right. If false, the entire system is culled as a whole.",
                PT_BOOL),
                &msCullCmd);

			dict->addParameter(ParameterDef("renderer", 
				"Sets the particle system renderer to use (default 'billboard').",
				PT_STRING),
				&msRendererCmd);

			dict->addParameter(ParameterDef("sorted", 
				"Sets whether particles should be sorted relative to the camera. ",
				PT_BOOL),
				&msSortedCmd);

			dict->addParameter(ParameterDef("local_space", 
				"Sets whether particles should be kept in local space rather than "
				"emitted into world space. ",
				PT_BOOL),
				&msLocalSpaceCmd);

			dict->addParameter(ParameterDef("iteration_interval", 
				"Sets a fixed update interval for the system, or 0 for the frame rate. ",
				PT_REAL),
				&msIterationIntervalCmd);

			dict->addParameter(ParameterDef("nonvisible_update_timeout", 
				"Sets a timeout on updates to the system if the system is not visible "
				"for the given number of seconds (0 to always update)",
				PT_REAL),
				&msNonvisibleTimeoutCmd);

        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateBounds()
    {

        if (mParentNode && (mBoundsAutoUpdate || mBoundsUpdateTime > 0.0f))
        {
            if (mActiveParticles.empty())
            {
                // No particles, reset to null if auto update bounds
                if (mBoundsAutoUpdate)
                {
                    mWorldAABB.setNull();
                }
            }
            else
            {
                Vector3 min;
                Vector3 max;
                if (!mBoundsAutoUpdate && mWorldAABB.isFinite())
                {
                    // We're on a limit, grow rather than reset each time
                    // so that we pick up the worst case scenario
                    min = mWorldAABB.getMinimum();
                    max = mWorldAABB.getMaximum();
                }
                else
                {
                    min.x = min.y = min.z = Math::POS_INFINITY;
                    max.x = max.y = max.z = Math::NEG_INFINITY;
                }
                ActiveParticleList::iterator p;
                Vector3 halfScale = Vector3::UNIT_SCALE * 0.5;
                Vector3 defaultPadding = 
                    halfScale * std::max(mDefaultHeight, mDefaultWidth);
                for (p = mActiveParticles.begin(); p != mActiveParticles.end(); ++p)
                {
                    if ((*p)->mOwnDimensions)
                    {
                        Vector3 padding = 
                            halfScale * std::max((*p)->mWidth, (*p)->mHeight);
                        min.makeFloor((*p)->position - padding);
                        max.makeCeil((*p)->position + padding);
                    }
                    else
                    {
                        min.makeFloor((*p)->position - defaultPadding);
                        max.makeCeil((*p)->position + defaultPadding);
                    }
                }
                mWorldAABB.setExtents(min, max);
            }


            if (mLocalSpace)
            {
                // Merge calculated box with current AABB to preserve any user-set AABB
                mAABB.merge(mWorldAABB);
            }
            else
            {
                // We've already put particles in world space to decouple them from the
                // node transform, so reverse transform back since we're expected to 
                // provide a local AABB
                AxisAlignedBox newAABB(mWorldAABB);
                newAABB.transformAffine(mParentNode->_getFullTransform().inverseAffine());

                // Merge calculated box with current AABB to preserve any user-set AABB
                mAABB.merge(newAABB);
            }

            mParentNode->needUpdate();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::fastForward(Real time, Real interval)
    {
        // First make sure all transforms are up to date

        for (Real ftime = 0; ftime < time; ftime += interval)
        {
            _update(interval);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMovableType(void) const
    {
        return ParticleSystemFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleResized(void)
    {
        if (mRenderer)
        {
            mRenderer->_notifyParticleResized();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleRotated(void)
    {
        if (mRenderer)
        {
            mRenderer->_notifyParticleRotated();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultDimensions( Real width, Real height )
    {
        mDefaultWidth = width;
        mDefaultHeight = height;
        if (mRenderer)
        {
            mRenderer->_notifyDefaultDimensions(width, height);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultWidth(Real width)
    {
        mDefaultWidth = width;
        if (mRenderer)
        {
            mRenderer->_notifyDefaultDimensions(mDefaultWidth, mDefaultHeight);
        }
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultWidth(void) const
    {
        return mDefaultWidth;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultHeight(Real height)
    {
        mDefaultHeight = height;
        if (mRenderer)
        {
            mRenderer->_notifyDefaultDimensions(mDefaultWidth, mDefaultHeight);
        }
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultHeight(void) const
    {
        return mDefaultHeight;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyCurrentCamera(Camera* cam)
    {
		MovableObject::_notifyCurrentCamera(cam);

		// Record visible
		if (isVisible())
		{			
			mLastVisibleFrame = Root::getSingleton().getNextFrameNumber();
			mTimeSinceLastVisible = 0.0f;

			if (mSorted)
			{
				_sortParticles(cam);
			}

			if (mRenderer)
			{
				if (!mIsRendererConfigured)
					configureRenderer();

				mRenderer->_notifyCurrentCamera(cam);
			}
		}
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyAttached(Node* parent, bool isTagPoint)
    {
        MovableObject::_notifyAttached(parent, isTagPoint);
        if (mRenderer && mIsRendererConfigured)
        {
            mRenderer->_notifyAttached(parent, isTagPoint);
        }

        if (parent && !mTimeController)
        {
            // Assume visible
            mTimeSinceLastVisible = 0;
            mLastVisibleFrame = Root::getSingleton().getNextFrameNumber();

            // Create time controller when attached
            ControllerManager& mgr = ControllerManager::getSingleton(); 
            ControllerValueRealPtr updValue(OGRE_NEW ParticleSystemUpdateValue(this));
            mTimeController = mgr.createFrameTimePassthroughController(updValue);
        }
        else if (!parent && mTimeController)
        {
            // Destroy controller
            ControllerManager::getSingleton().destroyController(mTimeController);
            mTimeController = 0;
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setMaterialName(const String& name)
    {
        mMaterialName = name;
        if (mIsRendererConfigured)
        {
            MaterialPtr mat = MaterialManager::getSingleton().load(
                mMaterialName, mResourceGroupName);
            mRenderer->_setMaterial(mat);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMaterialName(void) const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::clear()
    {
        // Notify renderer if exists
        if (mRenderer)
        {
            mRenderer->_notifyParticleCleared(mActiveParticles);
        }

        // Move actives to free list
        mFreeParticles.splice(mFreeParticles.end(), mActiveParticles);

        // Add active emitted emitters to free list
		addActiveEmittedEmittersToFreeList();

		// Remove all active emitted emitter instances
		mActiveEmittedEmitters.clear();

		// Reset update remain time
        mUpdateRemainTime = 0;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setRenderer(const String& rendererName)
    {
		if (mRenderer)
		{
			// Destroy existing
			destroyVisualParticles(0, mParticlePool.size());
			ParticleSystemManager::getSingleton()._destroyRenderer(mRenderer);
			mRenderer = 0;
		}

        if (!rendererName.empty())
        {
			mRenderer = ParticleSystemManager::getSingleton()._createRenderer(rendererName);
            mIsRendererConfigured = false;
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::configureRenderer(void)
    {
        // Actual allocate particles
        size_t currSize = mParticlePool.size();
        size_t size = mPoolSize;
        if( currSize < size )
        {
            this->increasePool(size);

            for( size_t i = currSize; i < size; ++i )
            {
                // Add new items to the queue
                mFreeParticles.push_back( mParticlePool[i] );
            }

            // Tell the renderer, if already configured
            if (mRenderer && mIsRendererConfigured)
            {
                mRenderer->_notifyParticleQuota(size);
            }
        }

        if (mRenderer && !mIsRendererConfigured)
        {
            mRenderer->_notifyParticleQuota(mParticlePool.size());
            mRenderer->_notifyAttached(mParentNode, mParentIsTagPoint);
            mRenderer->_notifyDefaultDimensions(mDefaultWidth, mDefaultHeight);
            createVisualParticles(0, mParticlePool.size());
            MaterialPtr mat = MaterialManager::getSingleton().load(
                mMaterialName, mResourceGroupName);
            mRenderer->_setMaterial(mat);
			if (mRenderQueueIDSet)
				mRenderer->setRenderQueueGroup(mRenderQueueID);
			mRenderer->setKeepParticlesInLocalSpace(mLocalSpace);
            mIsRendererConfigured = true;
        }
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* ParticleSystem::getRenderer(void) const
    {
        return mRenderer;
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getRendererName(void) const
    {
        if (mRenderer)
        {
            return mRenderer->getType();
        }
        else
        {
            return StringUtil::BLANK;
        }
    }
    //-----------------------------------------------------------------------
    bool ParticleSystem::getCullIndividually(void) const
    {
        return mCullIndividual;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setCullIndividually(bool cullIndividual)
    {
        mCullIndividual = cullIndividual;
    }
    //-----------------------------------------------------------------------
	void ParticleSystem::createVisualParticles(size_t poolstart, size_t poolend)
	{
		ParticlePool::iterator i = mParticlePool.begin();
		ParticlePool::iterator iend = mParticlePool.begin();
		std::advance(i, poolstart);
		std::advance(iend, poolend);
		for (; i != iend; ++i)
		{
			(*i)->_notifyVisualData(
				mRenderer->_createVisualData());
		}
	}
    //-----------------------------------------------------------------------
	void ParticleSystem::destroyVisualParticles(size_t poolstart, size_t poolend)
	{
		ParticlePool::iterator i = mParticlePool.begin();
		ParticlePool::iterator iend = mParticlePool.begin();
		std::advance(i, poolstart);
		std::advance(iend, poolend);
		for (; i != iend; ++i)
		{
			mRenderer->_destroyVisualData((*i)->getVisualData());
			(*i)->_notifyVisualData(0);
		}
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::setBounds(const AxisAlignedBox& aabb)
    {
        mAABB = aabb;
        Real sqDist = std::max(mAABB.getMinimum().squaredLength(), 
            mAABB.getMaximum().squaredLength());
        mBoundingRadius = Math::Sqrt(sqDist);

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setBoundsAutoUpdated(bool autoUpdate, Real stopIn)
    {
        mBoundsAutoUpdate = autoUpdate;
        mBoundsUpdateTime = stopIn;
    }
	//-----------------------------------------------------------------------
	void ParticleSystem::setRenderQueueGroup(uint8 queueID)
	{
		MovableObject::setRenderQueueGroup(queueID);
		if (mRenderer)
		{
			mRenderer->setRenderQueueGroup(queueID);
		}
	}
	//-----------------------------------------------------------------------
	void ParticleSystem::setKeepParticlesInLocalSpace(bool keepLocal)
	{
		mLocalSpace = keepLocal;
		if (mRenderer)
		{
			mRenderer->setKeepParticlesInLocalSpace(keepLocal);
		}
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::_sortParticles(Camera* cam)
    {
        if (mRenderer)
        {
            SortMode sortMode = mRenderer->_getSortMode();
            if (sortMode == SM_DIRECTION)
            {
                Vector3 camDir = cam->getDerivedDirection();
                if (mLocalSpace)
                {
                    // transform the camera direction into local space
                    camDir = mParentNode->_getDerivedOrientation().UnitInverse() * camDir;
                }
                mRadixSorter.sort(mActiveParticles, SortByDirectionFunctor(- camDir));
            }
            else if (sortMode == SM_DISTANCE)
            {
                Vector3 camPos = cam->getDerivedPosition();
                if (mLocalSpace)
                {
                    // transform the camera position into local space
                    camPos = mParentNode->_getDerivedOrientation().UnitInverse() *
                        (camPos - mParentNode->_getDerivedPosition()) / mParentNode->_getDerivedScale();
                }
                mRadixSorter.sort(mActiveParticles, SortByDistanceFunctor(camPos));
            }
        }
    }
    ParticleSystem::SortByDirectionFunctor::SortByDirectionFunctor(const Vector3& dir)
        : sortDir(dir)
    {
    }
    float ParticleSystem::SortByDirectionFunctor::operator()(Particle* p) const
    {
        return sortDir.dotProduct(p->position);
    }
    ParticleSystem::SortByDistanceFunctor::SortByDistanceFunctor(const Vector3& pos)
        : sortPos(pos)
    {
    }
    float ParticleSystem::SortByDistanceFunctor::operator()(Particle* p) const
    {
        // Sort descending by squared distance
        return - (sortPos - p->position).squaredLength();
    }
	//-----------------------------------------------------------------------
	uint32 ParticleSystem::getTypeFlags(void) const
	{
		return SceneManager::FX_TYPE_MASK;
	}
	//-----------------------------------------------------------------------
    void ParticleSystem::initialiseEmittedEmitters(void)
    {
		// Initialise the pool if needed
		size_t currSize = 0;
		if (mEmittedEmitterPool.empty())
		{
			if (mEmittedEmitterPoolInitialised)
			{
				// It was already initialised, but apparently no emitted emitters were used
				return;
			}
			else
			{
				initialiseEmittedEmitterPool();
			}
		}
		else
		{
			EmittedEmitterPool::iterator i;
			for (i = mEmittedEmitterPool.begin(); i != mEmittedEmitterPool.end(); ++i)
			{
				currSize += i->second.size();
			}
		}

        size_t size = mEmittedEmitterPoolSize;
        if( currSize < size && !mEmittedEmitterPool.empty())
        {
			// Increase the pool. Equally distribute over all vectors in the map
            increaseEmittedEmitterPool(size);
			
			// Add new items to the free list
			addFreeEmittedEmitters();
		}
    }

	//-----------------------------------------------------------------------
	void ParticleSystem::initialiseEmittedEmitterPool(void)
	{
		if (mEmittedEmitterPoolInitialised)
			return;

		// Run through mEmitters and add keys to the pool
		ParticleEmitterList::iterator emitterIterator;
		ParticleEmitterList::iterator emitterIteratorInner;
		ParticleEmitter* emitter = 0;
		ParticleEmitter* emitterInner = 0;
		for (emitterIterator = mEmitters.begin(); emitterIterator != mEmitters.end(); ++emitterIterator)
		{
			// Determine the names of all emitters that are emitted
			emitter = *emitterIterator ;
			if (emitter && emitter->getEmittedEmitter() != StringUtil::BLANK)
			{
				// This one will be emitted, register its name and leave the vector empty!
				EmittedEmitterList empty;
				mEmittedEmitterPool.insert(make_pair(emitter->getEmittedEmitter(), empty));
			}

			// Determine whether the emitter itself will be emitted and set the 'mEmitted' attribute
			for (emitterIteratorInner = mEmitters.begin(); emitterIteratorInner != mEmitters.end(); ++emitterIteratorInner)
			{
				emitterInner = *emitterIteratorInner;
				if (emitter && 
					emitterInner && 
					emitter->getName() != StringUtil::BLANK && 
					emitter->getName() == emitterInner->getEmittedEmitter())
				{
					emitter->setEmitted(true);
					break;
				}
				else
				{
					// Set explicitly to 'false' although the default value is already 'false'
					emitter->setEmitted(false);
				}
			}
		}

		mEmittedEmitterPoolInitialised = true;
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::increaseEmittedEmitterPool(size_t size)
    {
		// Don't proceed if the pool doesn't contain any keys of emitted emitters
		if (mEmittedEmitterPool.empty())
			return;

		EmittedEmitterPool::iterator emittedEmitterPoolIterator;
		ParticleEmitterList::iterator emitterIterator;
		ParticleEmitter* emitter = 0;
		ParticleEmitter* clonedEmitter = 0;
		String name = StringUtil::BLANK;
		EmittedEmitterList* e = 0;
		size_t maxNumberOfEmitters = size / mEmittedEmitterPool.size(); // equally distribute the number for each emitted emitter list
		size_t oldSize = 0;
	
		// Run through mEmittedEmitterPool and search for every key (=name) its corresponding emitter in mEmitters
		for (emittedEmitterPoolIterator = mEmittedEmitterPool.begin(); emittedEmitterPoolIterator != mEmittedEmitterPool.end(); ++emittedEmitterPoolIterator)
        {
			name = emittedEmitterPoolIterator->first;
			e = &emittedEmitterPoolIterator->second;

			// Search the correct emitter in the mEmitters vector
			emitter = 0;
			for (emitterIterator = mEmitters.begin(); emitterIterator != mEmitters.end(); ++emitterIterator)
			{
				emitter = *emitterIterator;
				if (emitter && 
					name != StringUtil::BLANK && 
					name == emitter->getName())
				{		
					// Found the right emitter, clone each emitter a number of times
					oldSize = e->size();
					for (size_t t = oldSize; t < maxNumberOfEmitters; ++t)
					{
						clonedEmitter = ParticleSystemManager::getSingleton()._createEmitter(emitter->getType(), this);
						emitter->copyParametersTo(clonedEmitter);
						clonedEmitter->setEmitted(emitter->isEmitted()); // is always 'true' by the way, but just in case

						// Initially deactivate the emitted emitter if duration/repeat_delay are set
						if (clonedEmitter->getDuration() > 0.0f && 
							(clonedEmitter->getRepeatDelay() > 0.0f || clonedEmitter->getMinRepeatDelay() > 0.0f || clonedEmitter->getMinRepeatDelay() > 0.0f))
							clonedEmitter->setEnabled(false);

						// Add cloned emitters to the pool
						e->push_back(clonedEmitter);
					}
				}
			}
        }
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::addFreeEmittedEmitters(void)
    {
		// Don't proceed if the EmittedEmitterPool is empty
		if (mEmittedEmitterPool.empty())
			return;

		// Copy all pooled emitters to the free list
		EmittedEmitterPool::iterator emittedEmitterPoolIterator;
		EmittedEmitterList::iterator emittedEmitterIterator;
		EmittedEmitterList* emittedEmitters = 0;
		std::list<ParticleEmitter*>* fee = 0;
		String name = StringUtil::BLANK;

		// Run through the emittedEmitterPool map
		for (emittedEmitterPoolIterator = mEmittedEmitterPool.begin(); emittedEmitterPoolIterator != mEmittedEmitterPool.end(); ++emittedEmitterPoolIterator)
        {
			name = emittedEmitterPoolIterator->first;
			emittedEmitters = &emittedEmitterPoolIterator->second;
			fee = findFreeEmittedEmitter(name);

			// If it´s not in the map, create an empty one
			if (!fee)
			{
				FreeEmittedEmitterList empty;
				mFreeEmittedEmitters.insert(make_pair(name, empty));
				fee = findFreeEmittedEmitter(name);
			}

			// Check anyway if it´s ok now
			if (!fee)
				return; // forget it!

			// Add all emitted emitters from the pool to the free list
			for(emittedEmitterIterator = emittedEmitters->begin(); emittedEmitterIterator != emittedEmitters->end(); ++emittedEmitterIterator)
			{
				fee->push_back(*emittedEmitterIterator);
			}
		}
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllEmittedEmitters(void)
    {
		EmittedEmitterPool::iterator emittedEmitterPoolIterator;
		EmittedEmitterList::iterator emittedEmitterListIterator;
		EmittedEmitterList* e = 0;
        for (emittedEmitterPoolIterator = mEmittedEmitterPool.begin(); emittedEmitterPoolIterator != mEmittedEmitterPool.end(); ++emittedEmitterPoolIterator)
        {
			e = &emittedEmitterPoolIterator->second;
			for (emittedEmitterListIterator = e->begin(); emittedEmitterListIterator != e->end(); ++emittedEmitterListIterator)
			{
				ParticleSystemManager::getSingleton()._destroyEmitter(*emittedEmitterListIterator);
			}
			e->clear();
        }

		// Don´t leave any references behind
		mEmittedEmitterPool.clear();
		mFreeEmittedEmitters.clear();
		mActiveEmittedEmitters.clear();
    }
	//-----------------------------------------------------------------------
	std::list<ParticleEmitter*>* ParticleSystem::findFreeEmittedEmitter (const String& name)
	{
		FreeEmittedEmitterMap::iterator it;
		it = mFreeEmittedEmitters.find (name);
		if (it != mFreeEmittedEmitters.end())
		{
			// Found it
			return &it->second;
		}

		return 0;
	}
	//-----------------------------------------------------------------------
	void ParticleSystem::removeFromActiveEmittedEmitters (ParticleEmitter* emitter)
	{
		assert(emitter && "Emitter to be removed is 0!");
		ActiveEmittedEmitterList::iterator itActiveEmit;
		for (itActiveEmit = mActiveEmittedEmitters.begin(); itActiveEmit != mActiveEmittedEmitters.end(); ++itActiveEmit)
		{
			if (emitter == (*itActiveEmit))
			{
				mActiveEmittedEmitters.erase(itActiveEmit);
				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	void ParticleSystem::addActiveEmittedEmittersToFreeList (void)
	{
		ActiveEmittedEmitterList::iterator itActiveEmit;
		for (itActiveEmit = mActiveEmittedEmitters.begin(); itActiveEmit != mActiveEmittedEmitters.end(); ++itActiveEmit)
		{
			std::list<ParticleEmitter*>* fee = findFreeEmittedEmitter ((*itActiveEmit)->getName());
			if (fee)
				fee->push_back(*itActiveEmit);
		}
	}
	//-----------------------------------------------------------------------
	void ParticleSystem::_notifyReorganiseEmittedEmitterData (void)
	{
		removeAllEmittedEmitters();
		mEmittedEmitterPoolInitialised = false; // Don´t rearrange immediately; it will be performed in the regular flow
	}
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdCull::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getCullIndividually() );
    }
    void ParticleSystem::CmdCull::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCullIndividually(
            StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdHeight::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultHeight() );
    }
    void ParticleSystem::CmdHeight::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultHeight(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdWidth::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultWidth() );
    }
    void ParticleSystem::CmdWidth::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultWidth(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdMaterial::doGet(const void* target) const
    {
        return static_cast<const ParticleSystem*>(target)->getMaterialName();
    }
    void ParticleSystem::CmdMaterial::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdQuota::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getParticleQuota() );
    }
    void ParticleSystem::CmdQuota::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setParticleQuota(
            StringConverter::parseUnsignedInt(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdEmittedEmitterQuota::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getEmittedEmitterQuota() );
    }
    void ParticleSystem::CmdEmittedEmitterQuota::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setEmittedEmitterQuota(
            StringConverter::parseUnsignedInt(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdRenderer::doGet(const void* target) const
    {
        return static_cast<const ParticleSystem*>(target)->getRendererName();
    }
    void ParticleSystem::CmdRenderer::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setRenderer(val);
    }
	//-----------------------------------------------------------------------
	String ParticleSystem::CmdSorted::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const ParticleSystem*>(target)->getSortingEnabled());
	}
	void ParticleSystem::CmdSorted::doSet(void* target, const String& val)
	{
		static_cast<ParticleSystem*>(target)->setSortingEnabled(
			StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String ParticleSystem::CmdLocalSpace::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const ParticleSystem*>(target)->getKeepParticlesInLocalSpace());
	}
	void ParticleSystem::CmdLocalSpace::doSet(void* target, const String& val)
	{
		static_cast<ParticleSystem*>(target)->setKeepParticlesInLocalSpace(
			StringConverter::parseBool(val));
	}
	//-----------------------------------------------------------------------
	String ParticleSystem::CmdIterationInterval::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const ParticleSystem*>(target)->getIterationInterval());
	}
	void ParticleSystem::CmdIterationInterval::doSet(void* target, const String& val)
	{
		static_cast<ParticleSystem*>(target)->setIterationInterval(
			StringConverter::parseReal(val));
	}
	//-----------------------------------------------------------------------
	String ParticleSystem::CmdNonvisibleTimeout::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const ParticleSystem*>(target)->getNonVisibleUpdateTimeout());
	}
	void ParticleSystem::CmdNonvisibleTimeout::doSet(void* target, const String& val)
	{
		static_cast<ParticleSystem*>(target)->setNonVisibleUpdateTimeout(
			StringConverter::parseReal(val));
	}
   //-----------------------------------------------------------------------
    ParticleAffector::~ParticleAffector() 
    {
    }
    //-----------------------------------------------------------------------
    ParticleAffectorFactory::~ParticleAffectorFactory() 
    {
        // Destroy all affectors
        std::vector<ParticleAffector*>::iterator i;
        for (i = mAffectors.begin(); i != mAffectors.end(); ++i)
        {
            OGRE_DELETE (*i);
        }
            
        mAffectors.clear();

    }
    //-----------------------------------------------------------------------
    void ParticleAffectorFactory::destroyAffector(ParticleAffector* e)
    {
        std::vector<ParticleAffector*>::iterator i;
        for (i = mAffectors.begin(); i != mAffectors.end(); ++i)
        {
            if ((*i) == e)
            {
                mAffectors.erase(i);
                OGRE_DELETE e;
                break;
            }
        }
    }

}
