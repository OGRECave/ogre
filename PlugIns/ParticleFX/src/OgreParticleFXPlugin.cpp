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
