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

#ifndef __SHADOWVOLUMEEXTRUDEPROGRAM_H__
#define __SHADOWVOLUMEEXTRUDEPROGRAM_H__

#include "OgrePrerequisites.h"
#include "OgreLight.h"

namespace Ogre {
    /** Static class containing source for vertex programs for extruding shadow volumes
    @remarks
        This exists so we don't have to be dependent on an external media files.
        Assembler is used so we don't have to rely on particular plugins.
        The assembler contents of this file were generated from the following Cg:
    @code
        // Point light shadow volume extrude
        void shadowVolumeExtrudePointLight_vp (
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

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
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

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
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

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
            float4 position			: POSITION,
            float  wcoord			: TEXCOORD0,

            out float4 oPosition	: POSITION,

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
        static String mDirArbvp1;
        static String mDirVs_1_1;
		static String mDirVs_4_0;
        // same as above, except the color is set to 1 to enable debug volumes to be seen
        static String mPointArbvp1Debug;
        static String mPointVs_1_1Debug;
		static String mPointVs_4_0Debug;
        static String mDirArbvp1Debug;
        static String mDirVs_1_1Debug;
		static String mDirVs_4_0Debug;
		
        static String mPointArbvp1Finite;
        static String mPointVs_1_1Finite;
		static String mPointVs_4_0Finite;
        static String mDirArbvp1Finite;
        static String mDirVs_1_1Finite;
		static String mDirVs_4_0Finite;
        // same as above, except the color is set to 1 to enable debug volumes to be seen
        static String mPointArbvp1FiniteDebug;
        static String mPointVs_1_1FiniteDebug;
		static String mPointVs_4_0FiniteDebug;
        static String mDirArbvp1FiniteDebug;
        static String mDirVs_1_1FiniteDebug;
		static String mDirVs_4_0FiniteDebug;

		static String mGeneralFs_4_0;

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
        /// Shutdown & destroy the vertex programs
        static void shutdown(void);
        /// Get extruder program source for point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1(void) { return mPointArbvp1; }
        /// Get extruder program source for point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1(void) { return mPointVs_1_1; }
		/// Get extruder program source for point lights, compatible with vs_4_0
		static const String& getPointLightExtruderVs_4_0(void) { return mPointVs_4_0; }
        /// Get extruder program source for directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1(void) { return mDirArbvp1; }
        /// Get extruder program source for directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1(void) { return mDirVs_1_1; }
		/// Get extruder program source for directional lights, compatible with vs_4_0
		static const String& getDirectionalLightExtruderVs_4_0(void) { return mDirVs_4_0; }

        /// Get extruder program source for debug point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1Debug(void) { return mPointArbvp1Debug; }
        /// Get extruder program source for debug point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1Debug(void) { return mPointVs_1_1Debug; }
		/// Get extruder program source for debug point lights, compatible with vs_4_0
		static const String& getPointLightExtruderVs_4_0Debug(void) { return mPointVs_4_0Debug; }
        /// Get extruder program source for debug directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1Debug(void) { return mDirArbvp1Debug; }
        /// Get extruder program source for debug directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1Debug(void) { return mDirVs_1_1Debug; }
		/// Get extruder program source for debug directional lights, compatible with vs_4_0
		static const String& getDirectionalLightExtruderVs_4_0Debug(void) { return mDirVs_4_0Debug; }
        /// General purpose method to get any of the program sources
        static const String& getProgramSource(Light::LightTypes lightType, const String syntax, 
            bool finite, bool debug);

        static const String& getProgramName(Light::LightTypes lightType, bool finite, bool debug);


        /// Get FINITE extruder program source for point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1Finite(void) { return mPointArbvp1Finite; }
        /// Get FINITE extruder program source for point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1Finite(void) { return mPointVs_1_1Finite; }
		/// Get FINITE extruder program source for point lights, compatible with vs_4_0
		static const String& getPointLightExtruderVs_4_0Finite(void) { return mPointVs_4_0Finite; }
        /// Get FINITE extruder program source for directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1Finite(void) { return mDirArbvp1Finite; }
        /// Get FINITE extruder program source for directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1Finite(void) { return mDirVs_1_1Finite; }
		/// Get FINITE extruder program source for directional lights, compatible with vs_4_0
		static const String& getDirectionalLightExtruderVs_4_0Finite(void) { return mDirVs_4_0Finite; }

        /// Get FINITE extruder program source for debug point lights, compatible with arbvp1
        static const String& getPointLightExtruderArbvp1FiniteDebug(void) { return mPointArbvp1FiniteDebug; }
        /// Get extruder program source for debug point lights, compatible with vs_1_1
        static const String& getPointLightExtruderVs_1_1FiniteDebug(void) { return mPointVs_1_1FiniteDebug; }
		/// Get extruder program source for debug point lights, compatible with vs_4_0
		static const String& getPointLightExtruderVs_4_0FiniteDebug(void) { return mPointVs_4_0FiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with arbvp1
        static const String& getDirectionalLightExtruderArbvp1FiniteDebug(void) { return mDirArbvp1FiniteDebug; }
        /// Get FINITE extruder program source for debug directional lights, compatible with vs_1_1
        static const String& getDirectionalLightExtruderVs_1_1FiniteDebug(void) { return mDirVs_1_1FiniteDebug; }
		/// Get FINITE extruder program source for debug directional lights, compatible with vs_4_0
		static const String& getDirectionalLightExtruderVs_4_0FiniteDebug(void) { return mDirVs_4_0FiniteDebug; }



		
    };
}
#endif
