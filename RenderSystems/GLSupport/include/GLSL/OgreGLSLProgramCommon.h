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

#ifndef RENDERSYSTEMS_GLSUPPORT_INCLUDE_GLSL_OGREGLSLPROGRAMCOMMON_H_
#define RENDERSYSTEMS_GLSUPPORT_INCLUDE_GLSL_OGREGLSLPROGRAMCOMMON_H_

#include "OgreGLSupportPrerequisites.h"
#include "OgreConfig.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreGLSLShaderCommon.h"
#include "OgreHardwareUniformBuffer.h"

namespace Ogre
{
/// Structure used to keep track of named uniforms in the linked program object
struct GLUniformReference
{
    /// GL location handle
    int mLocation;
    /// Which type of program params will this value come from?
    GpuProgramType mSourceProgType;
    /// The constant definition it relates to
    const GpuConstantDefinition* mConstantDef;
};
typedef std::vector<GLUniformReference> GLUniformReferenceList;
typedef GLUniformReferenceList::iterator GLUniformReferenceIterator;

typedef std::vector<HardwareUniformBufferSharedPtr> GLUniformBufferList;
typedef GLUniformBufferList::iterator GLUniformBufferIterator;

class GLSLProgramCommon
{
public:
    GLSLProgramCommon(GLSLShaderCommon* vertexShader);
    virtual ~GLSLProgramCommon() {}

    void extractLayoutQualifiers(void);

    /** Sets whether the linked program includes the required instructions
        to perform skeletal animation.
        @remarks
        If this is set to true, OGRE will not blend the geometry according to
        skeletal animation, it will expect the vertex program to do it.
    */
    void setSkeletalAnimationIncluded(bool included) { mSkeletalAnimation = included; }

    /** Returns whether the linked program includes the required instructions
        to perform skeletal animation.
        @remarks
         If this returns true, OGRE will not blend the geometry according to
         skeletal animation, it will expect the vertex program to do it.
                                                                         */
    bool isSkeletalAnimationIncluded(void) const { return mSkeletalAnimation; }

    /// Get the GL Handle for the program object
    uint getGLProgramHandle(void) const { return mGLProgramHandle; }

    /** Makes a program object active by making sure it is linked and then putting it in use.
     */
    virtual void activate(void) = 0;

    /// query if the program is using the given shader
    virtual bool isUsingShader(GLSLShaderCommon* shader) const = 0;

    /** Updates program object uniforms using data from GpuProgramParameters.
        Normally called by GLSLShader::bindParameters() just before rendering occurs.
    */
    virtual void updateUniforms(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType) = 0;

    /** Updates program object uniform blocks using data from GpuProgramParameters.
        Normally called by GLSLShader::bindParameters() just before rendering occurs.
    */
    virtual void updateUniformBlocks(GpuProgramParametersSharedPtr params, uint16 mask, GpuProgramType fromProgType) = 0;

    /** Updates program object uniforms using data from pass iteration GpuProgramParameters.
        Normally called by GLSLShader::bindMultiPassParameters() just before multi pass rendering occurs.
    */
    virtual void updatePassIterationUniforms(GpuProgramParametersSharedPtr params) = 0;

    /** Get the fixed attribute bindings normally used by GL for a semantic. */
    static int32 getFixedAttributeIndex(VertexElementSemantic semantic, uint index);

    /**
     * use alternate vertex attribute layout using only 8 vertex attributes
     *
     * For "Vivante GC1000" and "VideoCore IV" (notably in Raspberry Pi) on GLES2
     */
    static void useTightAttributeLayout();
protected:
    /// Container of uniform references that are active in the program object
    GLUniformReferenceList mGLUniformReferences;
    /// Container of uniform buffer references that are active in the program object
    GLUniformBufferList mGLUniformBufferReferences;

    /// Linked vertex shader.
    GLSLShaderCommon* mVertexShader;

    /// Flag to indicate that uniform references have already been built
    bool mUniformRefsBuilt;
    /// GL handle for the program object
    uint mGLProgramHandle;
    /// Flag indicating that the program or pipeline object has been successfully linked
    int mLinked;
    /// Flag indicating skeletal animation is being performed
    bool mSkeletalAnimation;
    /// A value to define the case we didn't look for the attributes since the contractor
    static const int NULL_CUSTOM_ATTRIBUTES_INDEX = -2;
    /// A value to define the attribute has not been found (this is also the result when glGetAttribLocation fails)
    static const int NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX = -1;

    /// An array to hold the attributes indexes
    int mCustomAttributesIndexes[VES_COUNT][OGRE_MAX_TEXTURE_COORD_SETS];

    /// Compiles and links the vertex and fragment programs
    virtual void compileAndLink(void) = 0;

    static VertexElementSemantic getAttributeSemanticEnum(const String& type);
    static const char * getAttributeSemanticString(VertexElementSemantic semantic);

    /// Name / attribute list
    struct CustomAttribute
    {
        const char* name;
        int32 attrib;
        VertexElementSemantic semantic;
    };
    static CustomAttribute msCustomAttributes[17];
};

} /* namespace Ogre */

#endif /* RENDERSYSTEMS_GLSUPPORT_INCLUDE_GLSL_OGREGLSLPROGRAMCOMMON_H_ */
