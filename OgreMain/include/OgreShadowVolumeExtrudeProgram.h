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

#ifndef __SHADOWVOLUMEEXTRUDEPROGRAM_H__
#define __SHADOWVOLUMEEXTRUDEPROGRAM_H__

#include "OgrePrerequisites.h"
#include "OgreLight.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Static class containing source for vertex programs for extruding shadow volumes
    @remarks
        This exists so we don't have to be dependent on an external media files.
        Assembler is used so we don't have to rely on particular plugins.
        The assembler contents of this file were generated from the following Cg:
    @code
        // Point light shadow volume extrude
        void shadowVolumeExtrudePointLight_vp (
            float4 position         : POSITION,
            float  wcoord           : TEXCOORD0,

            out float4 oPosition    : POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos // homogeneous, object space
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            float4 newpos = 
                (wcoord.xxxx * lightPos) + 
                float4(position.xyz - lightPos.xyz, 0);

            oPosition = mul(worldViewProjMatrix, newpos);

        }

        // Directional light extrude
        void shadowVolumeExtrudeDirLight_vp (
            float4 position         : POSITION,
            float  wcoord           : TEXCOORD0,

            out float4 oPosition    : POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos // homogenous, object space
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            float4 newpos = 
                (wcoord.xxxx * (position + lightPos)) - lightPos;

            oPosition = mul(worldViewProjMatrix, newpos);

        }
        // Point light shadow volume extrude - FINITE
        void shadowVolumeExtrudePointLightFinite_vp (
            float4 position         : POSITION,
            float  wcoord           : TEXCOORD0,

            out float4 oPosition    : POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos, // homogeneous, object space
            uniform float    extrusionDistance // how far to extrude
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            float3 extrusionDir = position.xyz - lightPos.xyz;
            extrusionDir = normalize(extrusionDir);
            
            float4 newpos = float4(position.xyz +  
                ((1 - wcoord.x) * extrusionDistance * extrusionDir), 1);

            oPosition = mul(worldViewProjMatrix, newpos);

        }

        // Directional light extrude - FINITE
        void shadowVolumeExtrudeDirLightFinite_vp (
            float4 position         : POSITION,
            float  wcoord           : TEXCOORD0,

            out float4 oPosition    : POSITION,

            uniform float4x4 worldViewProjMatrix,
            uniform float4   lightPos, // homogeneous, object space
            uniform float    extrusionDistance // how far to extrude
            )
        {
            // extrusion in object space
            // vertex unmodified if w==1, extruded if w==0
            // -ve lightPos is direction
            float4 newpos = float4(position.xyz - 
                (wcoord.x * extrusionDistance * lightPos.xyz), 1);

            oPosition = mul(worldViewProjMatrix, newpos);

        }       
    @endcode
    */
    class _OgreExport ShadowVolumeExtrudeProgram : public ShadowDataAlloc
    {
    private:
        static String mPointArbvp1;
        static String mPointVs_1_1;
        static String mPointVs_4_0;
        static String mPointVs_glsl;
        static String mPointVs_glsles;
        static String mDirArbvp1;
        static String mDirVs_1_1;
        static String mDirVs_4_0;
        static String mDirVs_glsl;
        static String mDirVs_glsles;
        // same as above, except the color is set to 1 to enable debug volumes to be seen
        static String mPointArbvp1Debug;
        static String mPointVs_1_1Debug;
        static String mPointVs_4_0Debug;
        static String mPointVs_glslDebug;
        static String mPointVs_glslesDebug;
        static String mDirArbvp1Debug;
        static String mDirVs_1_1Debug;
        static String mDirVs_4_0Debug;
        static String mDirVs_glslDebug;
        static String mDirVs_glslesDebug;
        
        static String mPointArbvp1Finite;
        static String mPointVs_1_1Finite;
        static String mPointVs_4_0Finite;
        static String mPointVs_glslFinite;
        static String mPointVs_glslesFinite;
        static String mDirArbvp1Finite;
        static String mDirVs_1_1Finite;
        static String mDirVs_4_0Finite;
        static String mDirVs_glslFinite;
        static String mDirVs_glslesFinite;
        // same as above, except the color is set to 1 to enable debug volumes to be seen
        static String mPointArbvp1FiniteDebug;
        static String mPointVs_1_1FiniteDebug;
        static String mPointVs_4_0FiniteDebug;
        static String mPointVs_glslFiniteDebug;
        static String mPointVs_glslesFiniteDebug;
        static String mDirArbvp1FiniteDebug;
        static String mDirVs_1_1FiniteDebug;
        static String mDirVs_4_0FiniteDebug;
        static String mDirVs_glslFiniteDebug;
        static String mDirVs_glslesFiniteDebug;

        static String mGeneralFs_4_0;
        static String mGeneralFs_glsl;
        static String mGeneralFs_glsles;

		static String mModulate_Fs_hlsl_4_0;
		static String mModulate_Vs_hlsl_4_0;
		static String mModulate_Fs_cg;
		static String mModulate_Vs_cg;
		static String mModulate_Fs_glsl;
		static String mModulate_Vs_glsl;
		
		


        static bool mInitialised;

    public:
#define OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS 8
        enum Programs
        {
            // Point light extruder, infinite distance
            POINT_LIGHT = 0,
            // Point light extruder, infinite distance, debug mode
            POINT_LIGHT_DEBUG = 1,
            // Directional light extruder, infinite distance
            DIRECTIONAL_LIGHT = 2,
            // Directional light extruder, infinite distance, debug mode
            DIRECTIONAL_LIGHT_DEBUG = 3,
            // Point light extruder, finite distance
            POINT_LIGHT_FINITE = 4,
            // Point light extruder, finite distance, debug mode
            POINT_LIGHT_FINITE_DEBUG = 5,
            // Directional light extruder, finite distance
            DIRECTIONAL_LIGHT_FINITE = 6,
            // Directional light extruder, finite distance, debug mode
            DIRECTIONAL_LIGHT_FINITE_DEBUG = 7

        };
        static const String programNames[OGRE_NUM_SHADOW_EXTRUDER_PROGRAMS];
        static String frgProgramName;

        /// Initialise the creation of these vertex programs
        static void initialise(void);
		/// Initialise the creation of these modulation pass programs
		static void initialiseModulationPassPrograms(void);
		/// Add and load high level gpu program
		static void AddInternalProgram(String name, String source, String language, String entryPoint, String target, GpuProgramType type);
        /// Shutdown & destroy the vertex programs
        static void shutdown(void);
        /// Get extruder program source for point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1(void) { return mPointArbvp1; }
        /// Get extruder program source for point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1(void) { return mPointVs_1_1; }
        /// Get extruder program source for point lights, compatible with vs_4_0
        static const String& getPointLightExtruderVs_4_0(void) { return mPointVs_4_0; }
        /// Get extruder program source for point lights, compatible with glsl
        static const String& getPointLightExtruderVs_glsl(void) { return mPointVs_glsl; }
        /// Get extruder program source for point lights, compatible with glsles
        static const String& getPointLightExtruderVs_glsles(void) { return mPointVs_glsles; }
        /// Get extruder program source for directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1(void) { return mDirArbvp1; }
        /// Get extruder program source for directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1(void) { return mDirVs_1_1; }
        /// Get extruder program source for directional lights, compatible with vs_4_0
        static const String& getDirectionalLightExtruderVs_4_0(void) { return mDirVs_4_0; }
        /// Get extruder program source for directional lights, compatible with glsl
        static const String& getDirectionalLightExtruderVs_glsl(void) { return mDirVs_glsl; }
        /// Get extruder program source for directional lights, compatible with glsles
        static const String& getDirectionalLightExtruderVs_glsles(void) { return mDirVs_glsles; }

        /// Get extruder program source for debug point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1Debug(void) { return mPointArbvp1Debug; }
        /// Get extruder program source for debug point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1Debug(void) { return mPointVs_1_1Debug; }
        /// Get extruder program source for debug point lights, compatible with vs_4_0
        static const String& getPointLightExtruderVs_4_0Debug(void) { return mPointVs_4_0Debug; }
        /// Get extruder program source for debug point lights, compatible with glsl
        static const String& getPointLightExtruderVs_glslDebug(void) { return mPointVs_glslDebug; }
        /// Get extruder program source for debug point lights, compatible with glsles
        static const String& getPointLightExtruderVs_glslesDebug(void) { return mPointVs_glslesDebug; }
        /// Get extruder program source for debug directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1Debug(void) { return mDirArbvp1Debug; }
        /// Get extruder program source for debug directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1Debug(void) { return mDirVs_1_1Debug; }
        /// Get extruder program source for debug directional lights, compatible with vs_4_0
        static const String& getDirectionalLightExtruderVs_4_0Debug(void) { return mDirVs_4_0Debug; }
        /// Get extruder program source for debug directional lights, compatible with glsl
        static const String& getDirectionalLightExtruderVs_glslDebug(void) { return mDirVs_glslDebug; }
        /// Get extruder program source for debug directional lights, compatible with glsles
        static const String& getDirectionalLightExtruderVs_glslesDebug(void) { return mDirVs_glslesDebug; }
        /// General purpose method to get any of the program sources
        static const String& getProgramSource(Light::LightTypes lightType, const String &syntax,
            bool finite, bool debug);

        static const String& getProgramName(Light::LightTypes lightType, bool finite, bool debug);


        /// Get FINITE extruder program source for point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1Finite(void) { return mPointArbvp1Finite; }
        /// Get FINITE extruder program source for point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1Finite(void) { return mPointVs_1_1Finite; }
        /// Get FINITE extruder program source for point lights, compatible with vs_4_0
        static const String& getPointLightExtruderVs_4_0Finite(void) { return mPointVs_4_0Finite; }
        /// Get FINITE extruder program source for point lights, compatible with glsl
        static const String& getPointLightExtruderVs_glslFinite(void) { return mPointVs_glslFinite; }
        /// Get FINITE extruder program source for point lights, compatible with glsles
        static const String& getPointLightExtruderVs_glslesFinite(void) { return mPointVs_glslesFinite; }
        /// Get FINITE extruder program source for directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1Finite(void) { return mDirArbvp1Finite; }
        /// Get FINITE extruder program source for directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1Finite(void) { return mDirVs_1_1Finite; }
        /// Get FINITE extruder program source for directional lights, compatible with vs_4_0
        static const String& getDirectionalLightExtruderVs_4_0Finite(void) { return mDirVs_4_0Finite; }
        /// Get FINITE extruder program source for directional lights, compatible with glsl
        static const String& getDirectionalLightExtruderVs_glslFinite(void) { return mDirVs_glslFinite; }
        /// Get FINITE extruder program source for directional lights, compatible with glsles
        static const String& getDirectionalLightExtruderVs_glslesFinite(void) { return mDirVs_glslesFinite; }

        /// Get FINITE extruder program source for debug point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1FiniteDebug(void) { return mPointArbvp1FiniteDebug; }
        /// Get extruder program source for debug point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1FiniteDebug(void) { return mPointVs_1_1FiniteDebug; }
        /// Get extruder program source for debug point lights, compatible with vs_4_0
        static const String& getPointLightExtruderVs_4_0FiniteDebug(void) { return mPointVs_4_0FiniteDebug; }
        /// Get extruder program source for debug point lights, compatible with glsl
        static const String& getPointLightExtruderVs_glslFiniteDebug(void) { return mPointVs_glslFiniteDebug; }
        /// Get extruder program source for debug point lights, compatible with glsles
        static const String& getPointLightExtruderVs_glslesFiniteDebug(void) { return mPointVs_glslesFiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1FiniteDebug(void) { return mDirArbvp1FiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1FiniteDebug(void) { return mDirVs_1_1FiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with vs_4_0
        static const String& getDirectionalLightExtruderVs_4_0FiniteDebug(void) { return mDirVs_4_0FiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with glsl
        static const String& getDirectionalLightExtruderVs_glslFiniteDebug(void) { return mDirVs_glslFiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with glsles
        static const String& getDirectionalLightExtruderVs_glslesFiniteDebug(void) { return mDirVs_glslesFiniteDebug; }

    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
