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

#include "OgreSPIRVShaderFactory.h"
#include "OgreLogManager.h"
#include "OgreGLSLExtSupport.h"


namespace Ogre {

static GLenum getGLShaderType(GpuProgramType programType)
{
    switch (programType)
    {
    case GPT_VERTEX_PROGRAM:
        return GL_VERTEX_SHADER;
    case GPT_HULL_PROGRAM:
        return GL_TESS_CONTROL_SHADER;
    case GPT_DOMAIN_PROGRAM:
        return GL_TESS_EVALUATION_SHADER;
    case GPT_GEOMETRY_PROGRAM:
        return GL_GEOMETRY_SHADER;
    case GPT_FRAGMENT_PROGRAM:
        return GL_FRAGMENT_SHADER;
    case GPT_COMPUTE_PROGRAM:
        return GL_COMPUTE_SHADER;
    }

    return 0;
}

SPIRVShader::SPIRVShader(ResourceManager* creator, const String& name, ResourceHandle handle, const String& group,
                         bool isManual, ManualResourceLoader* loader)
    : GLSLShader(creator, name, handle, group, isManual, loader)
{
    if (createParamDictionary("SPIRVGpuProgram"))
    {
        setupBaseParamDictionary();
    }
}

SPIRVShader::~SPIRVShader()
{
    // have to call this here reather than in Resource destructor
    // since calling virtual methods in base destructors causes crash
    unloadHighLevel();
}

const String& SPIRVShader::getLanguage(void) const
{
    static String language = "spirv";
    return language;
}

void SPIRVShader::loadFromSource(void)
{
    OGRE_CHECK_GL_ERROR(mGLShaderHandle = glCreateShader(getGLShaderType(mType)));

    OGRE_CHECK_GL_ERROR(glShaderBinary(1, &mGLShaderHandle, GL_SHADER_BINARY_FORMAT_SPIR_V, mSource.data(), mSource.size()));

    OGRE_CHECK_GL_ERROR(glSpecializeShader(mGLShaderHandle, "main", 0, NULL, NULL));

    // Check for compile errors
    int compiled = 0;
    OGRE_CHECK_GL_ERROR(glGetShaderiv(mGLShaderHandle, GL_COMPILE_STATUS, &compiled));

    if (compiled) return;

    String compileInfo = getObjectInfo(mGLShaderHandle);

    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, compileInfo);
}

SPIRVShaderFactory::SPIRVShaderFactory()
{

}

SPIRVShaderFactory::~SPIRVShaderFactory()
{

}

const String& SPIRVShaderFactory::getLanguage(void) const
{
    static String language = "spirv";
    return language;
}

HighLevelGpuProgram* SPIRVShaderFactory::create(ResourceManager* creator, const String& name, ResourceHandle handle,
                                                const String& group, bool isManual, ManualResourceLoader* loader)
{
    return OGRE_NEW SPIRVShader(creator, name, handle, group, isManual, loader);
}

void SPIRVShaderFactory::destroy(HighLevelGpuProgram* prog) { delete prog; }
}
