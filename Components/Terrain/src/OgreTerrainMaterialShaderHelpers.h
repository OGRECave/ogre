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

#ifndef __Ogre_TerrainMaterialShaderHelpers_H__
#define __Ogre_TerrainMaterialShaderHelpers_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrainMaterialGeneratorA.h"

namespace Ogre
{
    typedef TerrainMaterialGeneratorA::SM2Profile SM2Profile;

    /// Interface definition for helper class to generate shaders
    class ShaderHelper : public TerrainAlloc
    {
    public:
        ShaderHelper(bool glsl) : mShadowSamplerStartHi(0), mShadowSamplerStartLo(0), mIsGLSL(glsl) {}
        virtual ~ShaderHelper() {}
        bool isVertexCompressionSupported() { return !mIsGLSL; }
        virtual HighLevelGpuProgramPtr generateVertexProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        virtual HighLevelGpuProgramPtr generateFragmentProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        virtual void updateParams(const SM2Profile* prof, const MaterialPtr& mat, const Terrain* terrain, bool compositeMap);
    protected:
        virtual String getVertexProgramName(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        virtual String getFragmentProgramName(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        virtual HighLevelGpuProgramPtr createVertexProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt) = 0;
        virtual HighLevelGpuProgramPtr createFragmentProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt) = 0;
        virtual void generateVertexProgramSource(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        virtual void generateFragmentProgramSource(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        virtual void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream) = 0;
        virtual void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream) = 0;
        virtual void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream) = 0;
        virtual void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream) = 0;
        virtual void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream) = 0;
        virtual void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream) = 0;
        virtual void defaultVpParams(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const HighLevelGpuProgramPtr& prog);
        virtual void defaultFpParams(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const HighLevelGpuProgramPtr& prog);
        virtual void updateVpParams(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params);
        virtual void updateFpParams(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, const GpuProgramParametersSharedPtr& params);
        static String getChannel(uint idx);

        size_t mShadowSamplerStartHi;
        size_t mShadowSamplerStartLo;
        bool mIsGLSL;
    };

    /// Utility class to help with generating shaders for Cg / HLSL.
    struct ShaderHelperCg : public ShaderHelper
    {
        ShaderHelperCg();
    protected:
        bool mSM4Available;
        HighLevelGpuProgramPtr createVertexProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        HighLevelGpuProgramPtr createFragmentProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream);
        void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream);
        void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        uint generateVpDynamicShadowsParams(uint texCoordStart, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateVpDynamicShadows(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadowsHelpers(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadowsParams(uint* texCoord, uint* sampler, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadows(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
    };

    /// Utility class to help with generating shaders for GLSL.
    struct ShaderHelperGLSL : public ShaderHelper
    {
        ShaderHelperGLSL();
    protected:
        bool mIsGLES;
        HighLevelGpuProgramPtr createVertexProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        HighLevelGpuProgramPtr createFragmentProgram(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt);
        void generateVpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpHeader(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateVpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream);
        void generateFpLayer(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, uint layer, StringStream& outStream);
        void generateVpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpFooter(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        uint generateVpDynamicShadowsParams(uint texCoordStart, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateVpDynamicShadows(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadowsHelpers(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadowsParams(uint* texCoord, uint* sampler, const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
        void generateFpDynamicShadows(const SM2Profile* prof, const Terrain* terrain, TechniqueType tt, StringStream& outStream);
    };
}
#endif

