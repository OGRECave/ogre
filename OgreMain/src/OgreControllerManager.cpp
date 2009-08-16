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
#include "OgreControllerManager.h"

#include "OgreLogManager.h"
#include "OgreTextureUnitState.h"
#include "OgreRoot.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> ControllerManager* Singleton<ControllerManager>::ms_Singleton = 0;
    ControllerManager* ControllerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ControllerManager& ControllerManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    ControllerManager::ControllerManager()
		: mFrameTimeController(OGRE_NEW FrameTimeControllerValue())
		, mPassthroughFunction(OGRE_NEW PassthroughControllerFunction())
		, mLastFrameNumber(0)
    {

    }
    //-----------------------------------------------------------------------
    ControllerManager::~ControllerManager()
    {
        clearControllers();
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createController(
        const ControllerValueRealPtr& src, const ControllerValueRealPtr& dest,
        const ControllerFunctionRealPtr& func)
    {
        Controller<Real>* c = OGRE_NEW Controller<Real>(src, dest, func);

        mControllers.insert(c);
        return c;
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createFrameTimePassthroughController(
        const ControllerValueRealPtr& dest)
    {
        return createController(getFrameTimeSource(), dest, getPassthroughControllerFunction());
    }
    //-----------------------------------------------------------------------
    void ControllerManager::updateAllControllers(void)
    {
        // Only update once per frame
        unsigned long thisFrameNumber = Root::getSingleton().getNextFrameNumber();
        if (thisFrameNumber != mLastFrameNumber)
        {
            ControllerList::const_iterator ci;
            for (ci = mControllers.begin(); ci != mControllers.end(); ++ci)
            {
                (*ci)->update();
            }
            mLastFrameNumber = thisFrameNumber;
        }
    }
    //-----------------------------------------------------------------------
    void ControllerManager::clearControllers(void)
    {
        ControllerList::iterator ci;
        for (ci = mControllers.begin(); ci != mControllers.end(); ++ci)
        {
            OGRE_DELETE *ci;
        }
        mControllers.clear();
    }
    //-----------------------------------------------------------------------
    const ControllerValueRealPtr& ControllerManager::getFrameTimeSource(void) const
    {
        return mFrameTimeController;
    }
	//-----------------------------------------------------------------------
	const ControllerFunctionRealPtr& ControllerManager::getPassthroughControllerFunction(void) const
	{
		return mPassthroughFunction;
	}
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureAnimator(TextureUnitState* layer, Real sequenceTime)
    {
        SharedPtr< ControllerValue<Real> > texVal(OGRE_NEW TextureFrameControllerValue(layer));
        SharedPtr< ControllerFunction<Real> > animFunc(OGRE_NEW AnimationControllerFunction(sequenceTime));

        return createController(mFrameTimeController, texVal, animFunc);
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureUVScroller(TextureUnitState* layer, Real speed)
    {
		Controller<Real>* ret = 0;

		if (speed != 0)
        {
			SharedPtr< ControllerValue<Real> > val;
			SharedPtr< ControllerFunction<Real> > func;

			// We do both scrolls with a single controller
			val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, true, true));
			// Create function: use -speed since we're altering texture coords so they have reverse effect
            func.bind(OGRE_NEW ScaleControllerFunction(-speed, true));
            ret = createController(mFrameTimeController, val, func);
		}

		return ret;
	}
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureUScroller(TextureUnitState* layer, Real uSpeed)
    {
        Controller<Real>* ret = 0;

        if (uSpeed != 0)
        {
			SharedPtr< ControllerValue<Real> > uVal;
			SharedPtr< ControllerFunction<Real> > uFunc;

                uVal.bind(OGRE_NEW TexCoordModifierControllerValue(layer, true));
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            uFunc.bind(OGRE_NEW ScaleControllerFunction(-uSpeed, true));
            ret = createController(mFrameTimeController, uVal, uFunc);
        }

        return ret;
    }
	//-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureVScroller(TextureUnitState* layer, Real vSpeed)
    {
		Controller<Real>* ret = 0;

		if (vSpeed != 0)
        {
			SharedPtr< ControllerValue<Real> > vVal;
			SharedPtr< ControllerFunction<Real> > vFunc;

            // Set up a second controller for v scroll
            vVal.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, true));
            // Create function: use -speed since we're altering texture coords so they have reverse effect
            vFunc.bind(OGRE_NEW ScaleControllerFunction(-vSpeed, true));
            ret = createController(mFrameTimeController, vVal, vFunc);
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureRotater(TextureUnitState* layer, Real speed)
    {
        SharedPtr< ControllerValue<Real> > val;
        SharedPtr< ControllerFunction<Real> > func;

        // Target value is texture coord rotation
        val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, false, false, false, true));
        // Function is simple scale (seconds * speed)
        // Use -speed since altering texture coords has the reverse visible effect
        func.bind(OGRE_NEW ScaleControllerFunction(-speed, true));

        return createController(mFrameTimeController, val, func);

    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createTextureWaveTransformer(TextureUnitState* layer,
        TextureUnitState::TextureTransformType ttype, WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        SharedPtr< ControllerValue<Real> > val;
        SharedPtr< ControllerFunction<Real> > func;

        switch (ttype)
        {
        case TextureUnitState::TT_TRANSLATE_U:
            // Target value is a u scroll
            val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, true));
            break;
        case TextureUnitState::TT_TRANSLATE_V:
            // Target value is a v scroll
            val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, true));
            break;
        case TextureUnitState::TT_SCALE_U:
            // Target value is a u scale
            val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, false, true));
            break;
        case TextureUnitState::TT_SCALE_V:
            // Target value is a v scale
            val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, false, false, true));
            break;
        case TextureUnitState::TT_ROTATE:
            // Target value is texture coord rotation
            val.bind(OGRE_NEW TexCoordModifierControllerValue(layer, false, false, false, false, true));
            break;
        }
        // Create new wave function for alterations
        func.bind(OGRE_NEW WaveformControllerFunction(waveType, base, frequency, phase, amplitude, true));

        return createController(mFrameTimeController, val, func);
    }
    //-----------------------------------------------------------------------
    Controller<Real>* ControllerManager::createGpuProgramTimerParam(
        GpuProgramParametersSharedPtr params, size_t paramIndex, Real timeFactor)
    {
        SharedPtr< ControllerValue<Real> > val;
        SharedPtr< ControllerFunction<Real> > func;

        val.bind(OGRE_NEW FloatGpuParameterControllerValue(params, paramIndex));
        func.bind(OGRE_NEW ScaleControllerFunction(timeFactor, true));

        return createController(mFrameTimeController, val, func);

    }
    //-----------------------------------------------------------------------
    void ControllerManager::destroyController(Controller<Real>* controller)
    {
        ControllerList::iterator i = mControllers.find(controller);
        if (i != mControllers.end())
        {
            mControllers.erase(i);
            OGRE_DELETE controller;
        }
    }
	//-----------------------------------------------------------------------
	Real ControllerManager::getTimeFactor(void) const {
		return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getTimeFactor();
	}
	//-----------------------------------------------------------------------
	void ControllerManager::setTimeFactor(Real tf) {
		static_cast<FrameTimeControllerValue*>(mFrameTimeController.getPointer())->setTimeFactor(tf);
	}
	//-----------------------------------------------------------------------
	Real ControllerManager::getFrameDelay(void) const {
		return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getFrameDelay();
	}
	//-----------------------------------------------------------------------
	void ControllerManager::setFrameDelay(Real fd) {
		static_cast<FrameTimeControllerValue*>(mFrameTimeController.getPointer())->setFrameDelay(fd);
	}
	//-----------------------------------------------------------------------
	Real ControllerManager::getElapsedTime(void) const
	{
		return static_cast<const FrameTimeControllerValue*>(mFrameTimeController.get())->getElapsedTime();
	}
	//-----------------------------------------------------------------------
	void ControllerManager::setElapsedTime(Real elapsedTime)
	{
		static_cast<FrameTimeControllerValue*>(mFrameTimeController.get())->setElapsedTime(elapsedTime);
	}
}
