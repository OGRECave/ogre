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

#ifndef __OgreShaderPrecompiledHeaders__
#define __OgreShaderPrecompiledHeaders__

#include <algorithm> // for std::sort

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreSceneManager.h"
#include "OgreViewport.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreTextureManager.h"

#include "OgreShaderFunction.h"
#include "OgreShaderFunctionAtom.h"
#include "OgreShaderGenerator.h"
#include "OgreShaderProgram.h"
#include "OgreShaderProgramSet.h"
#include "OgreShaderProgramManager.h"
#include "OgreShaderProgramProcessor.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderMaterialSerializerListener.h"
#include "OgreShaderProgramWriterManager.h"

#include "OgreShaderFFPRenderState.h"
#include "OgreShaderFFPRenderStateBuilder.h"
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPLighting.h"
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderFFPFog.h"
#include "OgreShaderFFPAlphaTest.h"

#include "OgreShaderExPerPixelLighting.h"
#include "OgreShaderExNormalMapLighting.h"
#include "OgreShaderExIntegratedPSSM3.h"
#include "OgreShaderExLayeredBlending.h"
#include "OgreShaderExHardwareSkinningTechnique.h"
#include "OgreShaderExHardwareSkinning.h"
#include "OgreShaderExLinearSkinning.h"
#include "OgreShaderExDualQuaternionSkinning.h"
#include "OgreShaderExTextureAtlasSampler.h"
#include "OgreShaderExTriplanarTexturing.h"
#include "OgreShaderExGBuffer.h"

#include "OgreShaderCGProgramProcessor.h"
#include "OgreShaderHLSLProgramProcessor.h"
#include "OgreShaderGLSLProgramProcessor.h"
#include "OgreShaderGLSLESProgramProcessor.h"

#include "OgreShaderProgramWriter.h"
#include "OgreShaderProgramWriterManager.h"
#include "OgreShaderCGProgramWriter.h"
#include "OgreShaderHLSLProgramWriter.h"
#include "OgreShaderGLSLProgramWriter.h"
#include "OgreShaderGLSLESProgramWriter.h"

#endif 
