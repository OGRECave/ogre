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
#ifndef __QUAKE3SHADER_H__
#define __QUAKE3SHADER_H__

#include "OgreResource.h"
#include "OgreBspPrerequisites.h"
#include "OgreQuake3Types.h"
#include "OgreCommon.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"
#include "OgreTextureUnitState.h"

namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup BSPSceneManager
    *  @{
    */

    /** Class for recording Quake3 shaders.
        This is a temporary holding area since shaders are actually converted into
        Material objects for use in the engine proper. However, because we have to read
        in shader definitions en masse (because they are stored in shared .shader files)
        without knowing which will actually be used, we store their definitions here
        temporarily since their instantiations as Materials would use precious resources
        because of the automatic loading of textures etc.
    */
    class Quake3Shader : public ResourceAlloc
    {
    protected:
        String getAlternateName(const String& texName);
        String mName;

    public:

        /** Default constructor - used by Quake3ShaderManager (do not call directly) */
        Quake3Shader(const String& name);
        ~Quake3Shader();

        /** Creates this shader as an OGRE material.
            Creates a new material based on this shaders settings. 
            Material name shader#lightmap.
        */
        MaterialPtr createAsMaterial(int lightmapNumber);

        struct Pass {
            unsigned int flags;
            String textureName;
            TexGen texGen;
            // Multitexture blend
            LayerBlendOperation blend;
            // Multipass blends (Quake3 only supports multipass?? Surely not?)
            SceneBlendFactor blendSrc;
            SceneBlendFactor blendDest;
            bool customBlend;
            CompareFunction depthFunc;
            TextureUnitState::TextureAddressingMode addressMode;
            // TODO - alphaFunc
            GenFunc rgbGenFunc;
            WaveType rgbGenWave;
            Real rgbGenParams[4];    // base, amplitude, phase, frequency
            Real tcModScale[2];
            Real tcModRotate;
            Real tcModScroll[2];
            Real tcModTransform[6];
            bool tcModTurbOn;
            Real tcModTurb[4];
            WaveType tcModStretchWave;
            Real tcModStretchParams[4];    // base, amplitude, phase, frequency
            CompareFunction alphaFunc;
            unsigned char alphaVal;

            Real animFps;
            unsigned int animNumFrames;
            String frames[32];
        };

        unsigned int flags;
        int numPasses;
        typedef std::vector<Pass> PassList;
        PassList pass;
        bool farbox;            // Skybox
        String farboxName;
        bool skyDome;
        Real cloudHeight;       // Skydome
        DeformFunc deformFunc;
        Real deformParams[5];
        ManualCullingMode cullMode;

        bool fog;
        ColourValue fogColour;
        Real fogDistance;

    };
    /** @} */
    /** @} */
}

#endif
