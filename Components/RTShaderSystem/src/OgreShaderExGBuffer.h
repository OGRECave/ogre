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
#ifndef _ShaderGBuffer_
#define _ShaderGBuffer_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderParameter.h"
#include "OgreShaderSubRenderState.h"

namespace Ogre
{
namespace RTShader
{

/** \addtogroup Optional
 *  @{
 */
/** \addtogroup RTShader
 *  @{
 */

/** Transform sub render state implementation of writing to GBuffers
 */
class GBuffer : public SubRenderState
{

    // Interface.
public:
    enum TargetLayout
    {
        TL_DEPTH,
        TL_NORMAL,
        TL_VIEWPOS,
        TL_NORMAL_VIEWDEPTH,
        TL_DIFFUSE_SPECULAR,
    };
    typedef std::vector<TargetLayout> TargetBuffers;

    /**
    @see SubRenderState::getType.
    */
    const String& getType() const override;

    /**
    @see SubRenderState::getExecutionOrder.
    */
    int getExecutionOrder() const override;

    /**
    @see SubRenderState::copyFrom.
    */
    void copyFrom(const SubRenderState& rhs) override;

    /**
    @see SubRenderState::createCpuSubPrograms.
    */
    bool createCpuSubPrograms(ProgramSet* programSet) override;

    bool preAddToRenderState(const RenderState* renderState, Pass* srcPass, Pass* dstPass) override;

    void setParameter(const String& name, const Any& value) override;

    static String Type;

private:
    static void addViewPosInvocations(ProgramSet* programSet, const ParameterPtr& out, bool depthOnly);
    static void addDepthInvocations(ProgramSet* programSet, const ParameterPtr& out);
    static void addNormalInvocations(ProgramSet* programSet, const ParameterPtr& out) ;
    static void addDiffuseSpecularInvocations(ProgramSet* programSet, const ParameterPtr& out);

    TargetBuffers mOutBuffers;
};

/**
A factory that enables creation of GBuffer instances.
@remarks Sub class of SubRenderStateFactory
*/
class GBufferFactory : public SubRenderStateFactory
{
public:
    /**
    @see SubRenderStateFactory::getType.
    */
    const String& getType() const override;

    /**
    @see SubRenderStateFactory::createInstance.
    */
    SubRenderState* createInstance(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass,
                                           SGScriptTranslator* translator) override;

    /**
    @see SubRenderStateFactory::writeInstance.
    */
    void writeInstance(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) override;

protected:
    /**
    @see SubRenderStateFactory::createInstanceImpl.
    */
    SubRenderState* createInstanceImpl() override;
};

/** @} */
/** @} */

} // namespace RTShader
} // namespace Ogre

#endif
#endif
