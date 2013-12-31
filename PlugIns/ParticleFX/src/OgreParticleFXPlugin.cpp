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

#include "OgreParticleFXPlugin.h"
#include "OgreRoot.h"
#include "OgreParticleSystemManager.h"

#include "OgrePointEmitterFactory.h"
#include "OgreBoxEmitterFactory.h"
#include "OgreEllipsoidEmitterFactory.h"
#include "OgreHollowEllipsoidEmitterFactory.h"
#include "OgreRingEmitterFactory.h"
#include "OgreCylinderEmitterFactory.h"
#include "OgreLinearForceAffectorFactory.h"
#include "OgreColourFaderAffectorFactory.h"
#include "OgreColourFaderAffectorFactory2.h"
#include "OgreColourImageAffectorFactory.h"
#include "OgreColourInterpolatorAffectorFactory.h"
#include "OgreScaleAffectorFactory.h"
#include "OgreRotationAffectorFactory.h"
#include "OgreDirectionRandomiserAffectorFactory.h"
#include "OgreDeflectorPlaneAffectorFactory.h"

namespace Ogre 
{
	const String sPluginName = "ParticleFX";
	//---------------------------------------------------------------------
	ParticleFXPlugin::ParticleFXPlugin()
	{

	}
	//---------------------------------------------------------------------
	const String& ParticleFXPlugin::getName() const
	{
		return sPluginName;
	}
	//---------------------------------------------------------------------
	void ParticleFXPlugin::install()
	{
		// -- Create all new particle emitter factories --
		ParticleEmitterFactory* pEmitFact;

		// PointEmitter
		pEmitFact = OGRE_NEW PointEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// BoxEmitter
		pEmitFact = OGRE_NEW BoxEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// EllipsoidEmitter
		pEmitFact = OGRE_NEW EllipsoidEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// CylinderEmitter
		pEmitFact = OGRE_NEW CylinderEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// RingEmitter
		pEmitFact = OGRE_NEW RingEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// HollowEllipsoidEmitter
		pEmitFact = OGRE_NEW HollowEllipsoidEmitterFactory();
		ParticleSystemManager::getSingleton().addEmitterFactory(pEmitFact);
		mEmitterFactories.push_back(pEmitFact);

		// -- Create all new particle affector factories --
		ParticleAffectorFactory* pAffFact;

		// LinearForceAffector
		pAffFact = OGRE_NEW LinearForceAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// ColourFaderAffector
		pAffFact = OGRE_NEW ColourFaderAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// ColourFaderAffector2
		pAffFact = OGRE_NEW ColourFaderAffectorFactory2();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// ColourImageAffector
		pAffFact = OGRE_NEW ColourImageAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// ColourInterpolatorAffector
		pAffFact = OGRE_NEW ColourInterpolatorAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// ScaleAffector
		pAffFact = OGRE_NEW ScaleAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// RotationAffector
		pAffFact = OGRE_NEW RotationAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);


		// DirectionRandomiserAffector
		pAffFact = OGRE_NEW DirectionRandomiserAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);

		// DeflectorPlaneAffector
		pAffFact = OGRE_NEW DeflectorPlaneAffectorFactory();
		ParticleSystemManager::getSingleton().addAffectorFactory(pAffFact);
		mAffectorFactories.push_back(pAffFact);
	}
	//---------------------------------------------------------------------
	void ParticleFXPlugin::initialise()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void ParticleFXPlugin::shutdown()
	{
		// nothing to do
	}
	//---------------------------------------------------------------------
	void ParticleFXPlugin::uninstall()
	{
		// destroy 
		vector<ParticleEmitterFactory*>::type::iterator ei;
		vector<ParticleAffectorFactory*>::type::iterator ai;

		for (ei = mEmitterFactories.begin(); ei != mEmitterFactories.end(); ++ei)
		{
			OGRE_DELETE (*ei);
		}

		for (ai = mAffectorFactories.begin(); ai != mAffectorFactories.end(); ++ai)
		{
			OGRE_DELETE (*ai);
		}


	}


}
