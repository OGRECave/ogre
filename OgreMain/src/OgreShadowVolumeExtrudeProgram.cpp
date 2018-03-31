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
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

namespace {
    using Ogre::String;
    // c4 is the light position/direction in these
    String mPointVs_1_1 =
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

    String mPointVs_4_0 =
        "// Point light shadow volume extrude\n"
        "struct VS_OUTPUT\n"
        "{\n"
        "\tfloat4 Pos : SV_POSITION;\n"
        "};\n"
        "VS_OUTPUT vs_main (\n"
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space // homogeneous, object space\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
        "    float4 newpos = \n"
        "        (wcoord.xxxx * light_position_object_space) + \n"
        "        float4(position.xyz - light_position_object_space.xyz, 0);\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "}\n";

    const String glsles_prefix =
        "#version 100\n"
        "precision highp float;\n"
        "precision highp int;\n";

    String mPointVs_glsl =
        "// Point light shadow volume extrude\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * light_position_object_space) + \n"
        "        vec4(position.xyz - light_position_object_space.xyz, 0.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";
    String mPointVs_glsles = glsles_prefix + mPointVs_glsl;

    String mDirVs_1_1 =
        "vs_1_1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

    String mDirVs_4_0 =
        "// Directional light extrude\n"
        "struct VS_OUTPUT\n"
        "{\n"
        "\tfloat4 Pos : SV_POSITION;\n"
        "};\n"
        "VS_OUTPUT vs_main (\n"
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space // homogenous, object space\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
        "    float4 newpos = \n"
        "        (wcoord.xxxx * (position + light_position_object_space)) - light_position_object_space;\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "}\n";

    String mDirVs_glsl =
        "// Directional light extrude\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "    vec4 newpos = \n"
        "        (uv0.xxxx * (position + light_position_object_space)) - light_position_object_space;\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String mDirVs_glsles = glsles_prefix + mDirVs_glsl;

    // c4 is the light position/direction in these
    // c5 is extrusion distance

    String mPointVs_1_1Finite =
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r1.x, r0.x, c5.x\n"            // r1.x = (1 - wcoord) * extrudeDist
        "add r0.yzw, v0.xxyz, -c4.xxyz\n"
        "dp3 r0.x, r0.yzw, r0.yzw\n"        // r0.x = extrusionDir.sqrLength()
        "rsq r0.x, r0.x\n"                  // r0.x = 1 / extrusionDir.length()
        "mul r0.xyz, r0.x, r0.yzw\n"        // r0.xyz = extrusionDir.normalisedCopy()
        "mad r0.xyz, r1.x, r0.xyz, v0.xyz\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

    String mPointVs_4_0Finite =
        "// Point light shadow volume extrude - FINITE\n"
        "struct VS_OUTPUT\n"
        "{\n"
        "\tfloat4 Pos : SV_POSITION;\n"
        "};\n"
        "VS_OUTPUT vs_main (\n"
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space, // homogeneous, object space\n"
        "   uniform float    shadow_extrusion_distance // how far to extrude\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
		"    float3 extrusionDir = position.xyz - light_position_object_space.xyz;\n"
		"    extrusionDir = normalize(extrusionDir);\n"
		"\n"
        "    float4 newpos = float4(position.xyz +  \n"
        "        ((1 - wcoord.x) * shadow_extrusion_distance * extrusionDir), 1);\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "\n"
        "}\n";

    String mPointVs_glslFinite =
        "// Point light shadow volume extrude - FINITE\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance; // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "   vec3 extrusionDir = position.xyz - light_position_object_space.xyz;\n"
        "   extrusionDir = normalize(extrusionDir);\n"
        "   \n"
        "    vec4 newpos = vec4(position.xyz +  \n"
        "        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "}\n";

    String mPointVs_glslesFinite = glsles_prefix + mPointVs_glslFinite;

    String mDirVs_1_1Finite =
        "vs_1_1\n"
        "def c6, 1, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.x, c6.x, -v7.x\n"
        "mul r0.w, r0.x, c5.x\n"            // r0.w = (1 - wcoord) * extrudeDist
        "dp3 r1.w, c4.xyz, c4.xyz\n"        // r1.w = extrusionDir.sqrLength()
        "rsq r1.w, r1.w\n"                  // r1.w = 1 / extrusionDir.length()
        "mul r0.xyz, r1.w, -c4.xyz\n"       // r0.xyz = extrusionDir.normalisedCopy()
        "mad r0.xyz, r0.w, r0.xyz, v0.xyz;\n"
        "mov r0.w, c6.x\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

    String mDirVs_4_0Finite =
        "// Directional light extrude - FINITE\n"
        "struct VS_OUTPUT\n"
        "{\n"
        "\tfloat4 Pos : SV_POSITION;\n"
        "};\n"
        "VS_OUTPUT vs_main (\n"
        "    float4 position            : POSITION,\n"
        "    float  wcoord          : TEXCOORD0,\n"
        "\n"
        "    uniform float4x4 worldviewproj_matrix,\n"
        "    uniform float4   light_position_object_space, // homogeneous, object space\n"
        "   uniform float    shadow_extrusion_distance // how far to extrude\n"
        "    )\n"
        "{\n"
        "    // extrusion in object space\n"
        "    // vertex unmodified if w==1, extruded if w==0\n"
		"    float3 extrusionDir = - light_position_object_space.xyz;\n"
		"    extrusionDir = normalize(extrusionDir);\n"
		"\n"
		"    float4 newpos = float4(position.xyz + \n"
		"        ((1 - wcoord.x) * shadow_extrusion_distance * extrusionDir), 1);\n"
        "\n"
        "    VS_OUTPUT output = (VS_OUTPUT)0;\n"
        "    output.Pos = mul(worldviewproj_matrix, newpos);\n"
        "    return output;\n"
        "\n"
        "}\n";

    String mDirVs_glslFinite =
        "// Directional light extrude - FINITE\n"
        "attribute vec4 uv0;\n"
        "attribute vec4 position;\n\n"
        "uniform mat4 worldviewproj_matrix;\n"
        "uniform vec4 light_position_object_space; // homogenous, object space\n"
        "uniform float shadow_extrusion_distance;  // how far to extrude\n\n"
        "void main()\n"
        "{\n"
        "    // Extrusion in object space\n"
        "    // Vertex unmodified if w==1, extruded if w==0\n"
        "   vec3 extrusionDir = - light_position_object_space.xyz;\n"
        "   extrusionDir = normalize(extrusionDir);\n"
        "   \n"
        "    vec4 newpos = vec4(position.xyz +  \n"
        "        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);\n"
        "\n"
        "    gl_Position = worldviewproj_matrix * newpos;\n"
        "\n"
        "}\n";

    String mDirVs_glslesFinite = glsles_prefix + mDirVs_glslFinite;

    String mGeneralFs_4_0 =
        "struct VS_OUTPUT\n"
        "{\n"
        "\tfloat4 Pos : SV_POSITION;\n"
        "};\n"
        "uniform float4 colour;\n" // only needed for debug
        "float4 fs_main (VS_OUTPUT input): SV_Target\n"
        "{\n"
        "    return colour;\n"
        "}\n";

    String mGeneralFs_glsl =
        "uniform vec4 colour;" // only needed for debug
        "void main()\n"
        "{\n"
        "    gl_FragColor = colour;\n"
        "}\n";

    String mGeneralFs_glsles = glsles_prefix + mGeneralFs_glsl;

	 String mModulate_Fs_hlsl_4_0 =
        "uniform float4 shadowColor;\n"
        "float4 ShadowBlend_ps(float4 position : SV_POSITION) : SV_Target\n"
        "{\n"
        "   return shadowColor;\n"
        "}";


	 String mModulate_Vs_hlsl_4_0 =
        "void ShadowBlend_vs\n"
        "(\n"
        "in float4 inPos : POSITION,\n"
        "out float4 pos : SV_POSITION,\n"
        "uniform float4x4 worldViewProj\n"
        ")\n"
        "{\n"
        "   pos = mul(worldViewProj, inPos);\n"
        "}";

	 String mModulate_Fs_cg =
		 "uniform float4 shadowColor; \n"
		 "float4 ShadowBlend_ps() : COLOR\n"
		 "{\n"
         "   return shadowColor;\n"
		"}\n";

	 String mModulate_Vs_cg =
            "void ShadowBlend_vs\n"
            "(\n"
            "in float4 inPos : POSITION,\n"
            "out float4 pos : POSITION,\n"
            "uniform float4x4 worldViewProj\n"
            ")\n"
            "{\n"
            "   pos = mul(worldViewProj, inPos);\n"
            "}";

    String mModulate_Fs_glsl =
        "uniform vec4 shadowColor;\n"
        "\n"
        "void main() {\n"
        "    gl_FragColor = shadowColor;\n"
        "}";

    String mModulate_Vs_glsl =
        "uniform mat4 worldViewProj; \n"
        "attribute vec4 vertex; \n"
        "\n"
        "void main() {\n"
        "    gl_Position = worldViewProj*vertex; \n"
        "}";
}
namespace Ogre {
    const String ShadowVolumeExtrudeProgram::programNames[NUM_SHADOW_EXTRUDER_PROGRAMS] =
    {
		"Ogre/ShadowExtrudePointLight",
        "Ogre/ShadowExtrudeDirLight",
        "Ogre/ShadowExtrudePointLightFinite",
        "Ogre/ShadowExtrudeDirLightFinite"
    };

    HighLevelGpuProgramPtr ShadowVolumeExtrudeProgram::frgProgram;

    std::vector<GpuProgramPtr> ShadowVolumeExtrudeProgram::mPrograms;
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

	void ShadowVolumeExtrudeProgram::AddInternalProgram(String name, String source, String language, String entryPoint, String target, GpuProgramType type)
	{
		HighLevelGpuProgramPtr program = HighLevelGpuProgramManager::getSingleton()
			.createProgram(name, ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, language, type);

		program->setSource(source);
		program->setParameter("entry_point", entryPoint);
		if (language == "cg")
			program->setParameter("profiles", target);
		else
			program->setParameter("target", target);

		mPrograms.push_back(program);
	}

	void ShadowVolumeExtrudeProgram::initialiseModulationPassPrograms(void)
	{
		bool vs_4_0 = GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1");
		bool ps_4_0 = GpuProgramManager::getSingleton().isSyntaxSupported("ps_4_0_level_9_1");
		bool glsl = GpuProgramManager::getSingleton().isSyntaxSupported("glsl");
		bool glsles = GpuProgramManager::getSingleton().isSyntaxSupported("glsles");

		const String vsProgramName = "Ogre/ShadowBlendVP";
		const String fsProgramName = "Ogre/ShadowBlendFP";

		const String vsEntryPoint = "ShadowBlend_vs";
		const String fsEntryPoint = "ShadowBlend_ps";

		String language;
		String fsTarget;
		String vsTarget;
		String vsProgram;
		String fsProgram;

		if (glsl)
		{
			vsTarget = "glsl";
			fsTarget = "glsl";
			language = "glsl";
			vsProgram = mModulate_Vs_glsl;
			fsProgram = mModulate_Fs_glsl;
		}
		else if (glsles)
		{
            vsTarget = "glsles";
            fsTarget = "glsles";
            language = "glsles";
            vsProgram = glsles_prefix+mModulate_Vs_glsl;
            fsProgram = glsles_prefix+mModulate_Fs_glsl;
		}
		else
		{
			if (ps_4_0 && vs_4_0)
			{
				vsTarget = "vs_4_0_level_9_1";
				fsTarget = "ps_4_0_level_9_1";
				language = "hlsl";
				vsProgram = mModulate_Vs_hlsl_4_0;
				fsProgram = mModulate_Fs_hlsl_4_0;
			}
			else
			{
				vsTarget = "vs_2_0";
				fsTarget = "ps_2_0";
				language = "cg";
				vsProgram = mModulate_Vs_cg;
				fsProgram = mModulate_Fs_cg;
			}
		}

		//Add vertex program
		AddInternalProgram(vsProgramName, vsProgram, language, vsEntryPoint, vsTarget, GPT_VERTEX_PROGRAM);

		//Add fragment program
		AddInternalProgram(fsProgramName, fsProgram, language, fsEntryPoint, fsTarget, GPT_FRAGMENT_PROGRAM);
	}

    void ShadowVolumeExtrudeProgram::initialise(void)
    {
		if (mPrograms.empty())
		{
			String syntax;
			bool vertexProgramFinite[NUM_SHADOW_EXTRUDER_PROGRAMS] =
			{
				false, false,
				true,  true,
			};
			Light::LightTypes vertexProgramLightTypes[NUM_SHADOW_EXTRUDER_PROGRAMS] =
			{
				Light::LT_POINT, Light::LT_DIRECTIONAL,
				Light::LT_POINT, Light::LT_DIRECTIONAL,
			};

			// load hardware extrusion programs for point & dir lights
			if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_1_1"))
			{
				syntax = "vs_1_1";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0")
                  || GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_1")
                  || GpuProgramManager::getSingleton().isSyntaxSupported("vs_4_0_level_9_3"))
			{
				syntax = "vs_4_0";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("glsles"))
			{
				syntax = "glsles";
			}
			else if (GpuProgramManager::getSingleton().isSyntaxSupported("glsl"))
			{
				syntax = "glsl";
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Vertex programs are supposedly supported, but neither "
					"glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.",
					"SceneManager::initShadowVolumeMaterials");
			}
			// Create all programs
			for (unsigned short v = 0; v < NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
			{
				// Create debug extruders
				if (!GpuProgramManager::getSingleton().getByName(programNames[v],
				        ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME))
				{
					if (syntax == "vs_4_0")
					{
						HighLevelGpuProgramPtr vp =
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"hlsl", GPT_VERTEX_PROGRAM);
						vp->setSource(ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax,
							vertexProgramFinite[v], false));

						vp->setParameter("target", "vs_4_0_level_9_1"); // shared subset, to be usable from microcode cache on all devices
						vp->setParameter("entry_point", "vs_main");
						mPrograms.push_back(vp);

						if (!frgProgram)
						{
						    frgProgram =
								HighLevelGpuProgramManager::getSingleton().createProgram(
								        "Ogre/ShadowFrgProgram", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
								"hlsl", GPT_FRAGMENT_PROGRAM);
						    frgProgram->setSource(mGeneralFs_4_0);
						    frgProgram->setParameter("target", "ps_4_0_level_9_1"); // shared subset, to be usable from microcode cache on all devices
						    frgProgram->setParameter("entry_point", "fs_main");
						}
					}
					else if (syntax == "glsles")
					{
						HighLevelGpuProgramPtr vp =
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"glsles", GPT_VERTEX_PROGRAM);
						vp->setSource(ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax,
							vertexProgramFinite[v], false));
						vp->setParameter("target", syntax);
						mPrograms.push_back(vp);

						if (!frgProgram)
						{
							frgProgram =
								HighLevelGpuProgramManager::getSingleton().createProgram(
								"Ogre/ShadowFrgProgram", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
								"glsles", GPT_FRAGMENT_PROGRAM);
							frgProgram->setSource(glsles_prefix + mGeneralFs_glsl);
							frgProgram->setParameter("target", "glsles");
						}
					}
					else if (syntax == "glsl")
					{
						HighLevelGpuProgramPtr vp =
							HighLevelGpuProgramManager::getSingleton().createProgram(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							"glsl", GPT_VERTEX_PROGRAM);
						vp->setSource(ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax,
							vertexProgramFinite[v], false));
						vp->setParameter("target", syntax);
						mPrograms.push_back(vp);

						if (!frgProgram)
						{
						    frgProgram =
								HighLevelGpuProgramManager::getSingleton().createProgram(
								"Ogre/ShadowFrgProgram", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
								"glsl", GPT_FRAGMENT_PROGRAM);
						    frgProgram->setSource(mGeneralFs_glsl);
						    frgProgram->setParameter("target", "glsl");
						}
					}
					else
					{
						GpuProgramPtr vp =
							GpuProgramManager::getSingleton().createProgramFromString(
							programNames[v], ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME,
							ShadowVolumeExtrudeProgram::getProgramSource(
							vertexProgramLightTypes[v], syntax,
							vertexProgramFinite[v], false),
							GPT_VERTEX_PROGRAM, syntax);
						mPrograms.push_back(vp);
					}
				}
			}

			initialiseModulationPassPrograms();

			for(auto& prog : mPrograms)
			{
			    prog->load();
			}

			if(frgProgram)
			    frgProgram->load();
        }
    }
    //---------------------------------------------------------------------
    void ShadowVolumeExtrudeProgram::shutdown(void)
    {
        if (!mPrograms.empty())
        {
            for (unsigned short v = 0; v < NUM_SHADOW_EXTRUDER_PROGRAMS; ++v)
            {
                mPrograms[v]->getCreator()->remove(mPrograms[v]); // handle high level and low level programs
            }
            mPrograms.clear();

            if(frgProgram)
                frgProgram->getCreator()->remove(frgProgram);
            frgProgram.reset();
        }
    }
    //---------------------------------------------------------------------
    const String& ShadowVolumeExtrudeProgram::getProgramSource(
        Light::LightTypes lightType, const String &syntax, bool finite, bool debug)
    {
        if (lightType == Light::LT_DIRECTIONAL)
        {
            if (syntax == "vs_1_1")
            {
                return finite ? mDirVs_1_1Finite : mDirVs_1_1;
            }
            else if (syntax == "vs_4_0")
            {
                return finite ? mDirVs_4_0Finite : mDirVs_4_0;
			}
            else if (syntax == "glsl")
            {
                return finite ? mDirVs_glslFinite : mDirVs_glsl;
            }
            else if (syntax == "glsles")
            {
                return finite ? mDirVs_glslesFinite : mDirVs_glsles;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Vertex programs are supposedly supported, but neither "
                    "glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.",
                    "SceneManager::getProgramSource");
            }
        }
        else
        {
            if (syntax == "vs_1_1")
            {
                return finite ? mPointVs_1_1Finite : mPointVs_1_1;
            }
            else if (syntax == "vs_4_0")
            {
                return finite ? mPointVs_4_0Finite : mPointVs_4_0;
			}
            else if (syntax == "glsl")
            {
                return finite ? mPointVs_glslFinite : mPointVs_glsl;
			}
            else if (syntax == "glsles")
            {
                return finite ? mPointVs_glslesFinite : mPointVs_glsles;
			}
            else
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Vertex programs are supposedly supported, but neither "
                    "glsl, glsles, vs_1_1 nor vs_4_0 syntaxes are present.",
                    "SceneManager::getProgramSource");
            }
        }
    }
    //---------------------------------------------------------------------
    const GpuProgramPtr& ShadowVolumeExtrudeProgram::get(Light::LightTypes lightType, bool finite,
                                                         bool debug)
    {
        // note: we could use the debug flag to create special "debug" programs
        // however this is currently just one uniform in the fragment shader..
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
