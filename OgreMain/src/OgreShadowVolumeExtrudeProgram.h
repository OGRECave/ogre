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

        static std::vector<GpuProgramPtr> mPrograms;
        static const String programNames[NUM_SHADOW_EXTRUDER_PROGRAMS];

        /// General purpose method to get any of the program sources
        static const String& getProgramSource(Light::LightTypes lightType, const String &syntax,
            bool finite, bool debug);

        /// Initialise the creation of these modulation pass programs
        static void initialiseModulationPassPrograms(void);
        /// Add and load high level gpu program
        static void AddInternalProgram(String name, String source, String language, String entryPoint, String target, GpuProgramType type);
    public:
        static HighLevelGpuProgramPtr frgProgram;

        /// Initialise the creation of these vertex programs
        static void initialise(void);
        /// Shutdown & destroy the vertex programs
        static void shutdown(void);

        static const GpuProgramPtr& get(Light::LightTypes lightType, bool finite, bool debug = false);
    };
    /** @} */
    /** @} */
}

#endif
