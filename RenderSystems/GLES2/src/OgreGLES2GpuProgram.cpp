/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLES2GpuProgram.h"
#include "OgreLogManager.h"

using namespace Ogre;

GLenum getGLShaderType(GpuProgramType programType)
{
	switch (programType)
	{
		case GPT_VERTEX_PROGRAM:
		default:
			return GL_VERTEX_SHADER;
		case GPT_FRAGMENT_PROGRAM:
			return GL_FRAGMENT_SHADER;
	}
}

GLES2GpuProgram::GLES2GpuProgram(ResourceManager* creator, const String& name, 
    ResourceHandle handle, const String& group, bool isManual, 
    ManualResourceLoader* loader) 
    : GpuProgram(creator, name, handle, group, isManual, loader)
{
    if (createParamDictionary("GLES2GpuProgram"))
    {
        setupBaseParamDictionary();
    }
}

GLES2GpuProgram::~GLES2GpuProgram()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unload(); 
}

GLuint GLES2GpuProgram::getAttributeIndex(VertexElementSemantic semantic, uint index)
{
	return getFixedAttributeIndex(semantic, index);
}

GLuint GLES2GpuProgram::getFixedAttributeIndex(VertexElementSemantic semantic, uint index)
{
	// Some drivers (e.g. OS X on nvidia) incorrectly determine the attribute binding automatically
	// and end up aliasing existing built-ins. So avoid! Fixed builtins are: 

	//  a  builtin				custom attrib name
	// ----------------------------------------------
	//	0  gl_Vertex			vertex
	//  1  n/a					blendWeights		
	//	2  gl_Normal			normal
	//	3  gl_Color				colour
	//	4  gl_SecondaryColor	secondary_colour
	//	5  gl_FogCoord			fog_coord
	//  7  n/a					blendIndices
	//	8  gl_MultiTexCoord0	uv0
	//	9  gl_MultiTexCoord1	uv1
	//	10 gl_MultiTexCoord2	uv2
	//	11 gl_MultiTexCoord3	uv3
	//	12 gl_MultiTexCoord4	uv4
	//	13 gl_MultiTexCoord5	uv5
	//	14 gl_MultiTexCoord6	uv6, tangent
	//	15 gl_MultiTexCoord7	uv7, binormal
	switch(semantic)
	{
        case VES_POSITION:
            return 0;
        case VES_BLEND_WEIGHTS:
            return 1;
        case VES_NORMAL:
            return 2;
        case VES_DIFFUSE:
            return 3;
        case VES_SPECULAR:
            return 4;
        case VES_BLEND_INDICES:
            return 7;
        case VES_TEXTURE_COORDINATES:
            return 8 + index;
        case VES_TANGENT:
            return 14;
        case VES_BINORMAL:
            return 15;
        default:
            assert(false && "Missing attribute!");
            return 0;
	};

}

bool GLES2GpuProgram::isAttributeValid(VertexElementSemantic semantic, uint index)
{
	// default implementation
	switch(semantic)
	{
		case VES_POSITION:
		case VES_NORMAL:
		case VES_DIFFUSE:
		case VES_SPECULAR:
		case VES_TEXTURE_COORDINATES:
			return false;
		case VES_BLEND_WEIGHTS:
		case VES_BLEND_INDICES:
		case VES_BINORMAL:
		case VES_TANGENT:
			return true; // with default binding
	}

    return false;
}
