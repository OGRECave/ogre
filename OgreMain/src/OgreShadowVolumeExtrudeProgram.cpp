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
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreString.h"
#include "OgreGpuProgramManager.h"
#include "OgreGpuProgram.h"
#include "OgreException.h"

namespace Ogre {

    // c4 is the light position/direction in these
    String ShadowVolumeExtrudeProgram::mPointArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1 = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1 = 
        "!!ARBvp1.0\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1 = 
        "vs_1_1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";


    String ShadowVolumeExtrudeProgram::mPointArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c6.x;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "def c6, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c6.x\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1};\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c5.x;"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c5.x\n";


    // c4 is the light position/direction in these
    // c5 is extrusion distance
    String ShadowVolumeExtrudeProgram::mPointArbvp1Finite = 
        "!!ARBvp1.0\n" 
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.w, R0.x, c5.x;\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MAD R0.xyz, R0.w, R0.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1Finite = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r1.x, r0.x, c5.x\n"
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"
        "rsq r0.x, r0.x\n"
        "mul r0.xyz, r0.x, r0.yzw\n"
        "mad r0.xyz, r1.x, r0.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";
    String ShadowVolumeExtrudeProgram::mDirArbvp1Finite = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "PARAM c5 = program.local[5];\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.x, R0.x, c5.x;\n"
        "MAD R0.xyz, -R0.x, c4.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";
    String ShadowVolumeExtrudeProgram::mDirVs_1_1Finite = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r0.x, r0.x, c5.x\n"
        "mad r0.xyz, -r0.x, c4.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";
    String ShadowVolumeExtrudeProgram::mPointArbvp1FiniteDebug = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0, R1;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c4 = program.local[4];\n"
        "MOV result.color.front.primary, c6.x;\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R1.x, R0.x, c5.x;\n"
        "ADD R0.yzw, v16.xxyz, -c4.xxyz;\n"
        "DP3 R0.x, R0.yzwy, R0.yzwy;\n"
        "RSQ R0.x, R0.x;\n"
        "MUL R0.xyz, R0.x, R0.yzwy;\n"
        "MAD R0.xyz, R1.x, R0.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";
    String ShadowVolumeExtrudeProgram::mPointVs_1_1FiniteDebug = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "mov oD0, c6.x\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r1.x, r0.x, c5.x\n"
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"
        "rsq r0.x, r0.x\n"
        "mul r0.xyz, r0.x, r0.yzw\n"
        "mad r0.xyz, r1.x, r0.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";
    String ShadowVolumeExtrudeProgram::mDirArbvp1FiniteDebug = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "PARAM c5 = program.local[5];\n"
        "MOV result.color.front.primary, c6.x;\n"
        "ADD R0.x, c6.x, -v24.x;\n"
        "MUL R0.x, R0.x, c5.x;\n"
        "MAD R0.xyz, -R0.x, c4.xyzx, v16.xyzx;\n"
        "DPH result.position.x, R0.xyzz, c0[0];\n"
        "DPH result.position.y, R0.xyzz, c0[1];\n"
        "DPH result.position.z, R0.xyzz, c0[2];\n"
        "DPH result.position.w, R0.xyzz, c0[3];\n"
        "END\n";
    String ShadowVolumeExtrudeProgram::mDirVs_1_1FiniteDebug = 
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "mov oD0, c6.x\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r0.x, r0.x, c5.x\n"
        "mad r0.xyz, -r0.x, c4.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";


    const String ShadowVolumeExtrudeProgram::programNames[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS] = 
    {
        "Ogre/ShadowExtrudePointLight",
            "Ogre/ShadowExtrudePointLightDebug",
            "Ogre/ShadowExtrudeDirLight",
            "Ogre/ShadowExtrudeDirLightDebug",
            "Ogre/ShadowExtrudePointLightFinite",
            "Ogre/ShadowExtrudePointLightFiniteDebug",
            "Ogre/ShadowExtrudeDirLightFinite",
            "Ogre/ShadowExtrudeDirLightFiniteDebug"
    };

	bool ShadowVolumeExtrudeProgram::mInitialised = false;
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::initialise(void)
    {
		if (!mInitialised)
		{
			String syntax;
			bool vertexProgramFinite[8] = 
			{
				false, false, false, false, 
					true, true, true, true
			};
			bool vertexProgramDebug[8] = 
			{
				false, true, false, true, 
					false, true, false, true
			};
			Light::LightTypes vertexProgramLightTypes[8] = 
			{
				Light::LT_POINT, Light::LT_POINT, 
					Light::LT_DIRECTIONAL, Light::LT_DIRECTIONAL, 
					Light::LT_POINT, Light::LT_POINT, 
					Light::LT_DIRECTIONAL, Light::LT_DIRECTIONAL 
			};

			// load hardware extrusion programs for point & dir lights
			if (GpuProgramManager::getSingleton().isSyntaxSupported("arbvp1"))
			{
				// ARBvp1
				syntax = "arbvp1";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_1_1"))
			{
				syntax = "vs_1_1";
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Vertex programs are supposedly supported, but neither "
					"arbvp1 nor vs_1_1 syntaxes are present.", 
					"SceneManager::initShadowVolumeMaterials");
			}
			// Create all programs
			for (unsigned short v = 0; v < OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
			{
				// Create debug extruders
				if (GpuProgramManager::getSingleton().getByName(
					programNames[v]).isNull())
				{
					GpuProgramPtr vp = 
						GpuProgramManager::getSingleton().createProgramFromString(
						programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
						ShadowVolumeExtrudeProgram::getProgramSource(
						vertexProgramLightTypes[v], syntax, 
						vertexProgramFinite[v], vertexProgramDebug[v]),
						GPT_VERTEX_PROGRAM, syntax);
					vp->load();
				}
			}
			mInitialised = true;
		}
    }
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::shutdown(void)
    {
        if (mInitialised)
        {
            for (unsigned short v = 0; v < OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
            {
                // Destroy debug extruders
                GpuProgramManager::getSingleton().remove(programNames[v]);
            }
            mInitialised = false;
        }
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramSource(
        Light::LightTypes lightType, const String syntax, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderArbvp1FiniteDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderArbvp1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderArbvp1Debug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderArbvp1();
                    }
                }
            }
            else
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_1_1FiniteDebug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_1_1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getDirectionalLightExtruderVs_1_1Debug();
                    }
                    else
                    {
                        return getDirectionalLightExtruderVs_1_1();
                    }
                }
            }
        }
        else
        {
            if (syntax == "arbvp1")
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getPointLightExtruderArbvp1FiniteDebug();
                    }
                    else
                    {
                        return getPointLightExtruderArbvp1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getPointLightExtruderArbvp1Debug();
                    }
                    else
                    {
                        return getPointLightExtruderArbvp1();
                    }
                }
            }
            else
            {
                if (finite)
                {
                    if (debug)
                    {
                        return getPointLightExtruderVs_1_1FiniteDebug();
                    }
                    else
                    {
                        return getPointLightExtruderVs_1_1Finite();
                    }
                }
                else
                {
                    if (debug)
                    {
                        return getPointLightExtruderVs_1_1Debug();
                    }
                    else
                    {
                        return getPointLightExtruderVs_1_1();
                    }
                }
            }
        }
        // to keep compiler happy
        return StringUtil::BLANK;
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramName(
        Light::LightTypes lightType, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (finite)
            {
                if (debug)
                {
                    return programNames[DIRECTIONAL_LIGHT_FINITE_DEBUG];
                }
                else
                {
                    return programNames[DIRECTIONAL_LIGHT_FINITE];
                }
            }
            else
            {
                if (debug)
                {
                    return programNames[DIRECTIONAL_LIGHT_DEBUG];
                }
                else
                {
                    return programNames[DIRECTIONAL_LIGHT];
                }
            }
        }
        else
        {
            if (finite)
            {
                if (debug)
                {
                    return programNames[POINT_LIGHT_FINITE_DEBUG];
                }
                else
                {
                    return programNames[POINT_LIGHT_FINITE];
                }
            }
            else
            {
                if (debug)
                {
                    return programNames[POINT_LIGHT_DEBUG];
                }
                else
                {
                    return programNames[POINT_LIGHT];
                }
            }
        }
    }



}
