/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __ShaderPrerequisites_H__
#define __ShaderPrerequisites_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

namespace Ogre 
{

/// Forward declarations from Ogre namespace.
class Technique;
class Pass;
class Renderable;
class Viewport;
class RenderObjectListener;
class TextureUnitState;
class Frustum;
class ScriptTranslator;
class ScriptCompiler;
class PropertyAbstractNode;
class MaterialSerializer;

typedef GeneralAllocatedObject  RTShaderSystemAlloc;

namespace RTShader 
{

/// Forward declarations from RTShader namespace.
class RenderState;
class TargetRenderState;
class SubRenderState;
class SubRenderStateAccessor;
class SubRenderStateFactory;
class ProgramManager;
class Program;
class ProgramProcessor;
class ProgramSet;
class RenderState;
class Parameter;
class UniformParameter;
class Function;
class FunctionStageRef;
class FFPRenderStateBuilder;
class ShaderGenerator;
class SGMaterialSerializerListener;
class ProgramWriterFactory;
class ProgramWriterManager;

typedef shared_ptr<Parameter>        ParameterPtr;
typedef shared_ptr<UniformParameter> UniformParameterPtr;
typedef std::vector<ParameterPtr>  ShaderParameterList;

// Vertex shader output parameters compact policy.
enum VSOutputCompactPolicy
{   
    VSOCP_LOW       = 0,        // VS Outputs will be compacted just in case the maximum slot count exceeded.
    VSOCP_MEDIUM    = 1,        // VS Outputs will be compacted always without parameter splits.
    VSOCP_HIGH      = 2         // VS Outputs will be compacted always including parameter splits.
};

}
}

#include "OgreRTShaderExports.h"
#include "OgreRTShaderConfig.h"


#endif

