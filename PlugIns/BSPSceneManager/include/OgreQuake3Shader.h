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
}

#endif
