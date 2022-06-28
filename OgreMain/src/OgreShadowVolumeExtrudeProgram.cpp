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

#include "OgreStableHeaders.h"
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

namespace {
    enum Programs
    {
        // Point light extruder, infinite distance
        POINT_LIGHT = 0,
        // Directional light extruder, infinite distance
        DIRECTIONAL_LIGHT,
        // Point light extruder, finite distance
        POINT_LIGHT_FINITE,
        // Directional light extruder, finite distance
        DIRECTIONAL_LIGHT_FINITE,
        NUM_SHADOW_EXTRUDER_PROGRAMS
    };

    const char* programNames[NUM_SHADOW_EXTRUDER_PROGRAMS] =
    {
        "Ogre/ShadowExtrudePointLight",
        "Ogre/ShadowExtrudeDirLight",
        "Ogre/ShadowExtrudePointLightFinite",
        "Ogre/ShadowExtrudeDirLightFinite"
    };
}

namespace Ogre {
    std::vector<GpuProgramPtr> ShadowVolumeExtrudeProgram::mPrograms;

    void ShadowVolumeExtrudeProgram::initialise(void)
    {
		mPrograms.clear();

        // load all programs
        for (auto name : programNames)
        {
            auto vp = HighLevelGpuProgramManager::getSingleton().getByName(name, RGN_INTERNAL);
            if (!vp)
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            String(name) + " not found. Verify that you referenced the 'Media/Main' "
                                           "folder in your resources.cfg");
            vp->load();
            mPrograms.push_back(vp);
        }
    }
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::shutdown(void)
    {
        mPrograms.clear();
    }
    //---------------------------------------------------------------------
    const GpuProgramPtr& ShadowVolumeExtrudeProgram::get(Light::LightTypes lightType, bool finite)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            return mPrograms[finite ? DIRECTIONAL_LIGHT_FINITE : DIRECTIONAL_LIGHT];
        }
        else
        {
            return mPrograms[finite ? POINT_LIGHT_FINITE : POINT_LIGHT];
        }
    }

}
