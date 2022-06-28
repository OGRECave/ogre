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
#include "OgreShaderExInstancedViewports.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreShaderProgramSet.h"
#include "OgreScriptCompiler.h"
#include "OgreTechnique.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRoot.h"

namespace Ogre {
namespace RTShader {


/************************************************************************/
/*                                                                      */
/************************************************************************/
String ShaderExInstancedViewports::Type                         = "SGX_InstancedViewports";

//-----------------------------------------------------------------------
#define SGX_LIB_INSTANCED_VIEWPORTS                         "SampleLib_InstancedViewports"
#define SGX_FUNC_INSTANCED_VIEWPORTS_TRANSFORM              "SGX_InstancedViewportsTransform"
#define SGX_FUNC_INSTANCED_VIEWPORTS_DISCARD_OUT_OF_BOUNDS  "SGX_InstancedViewportsDiscardOutOfBounds"


//-----------------------------------------------------------------------
ShaderExInstancedViewports::ShaderExInstancedViewports()
{
    mMonitorsCount              = Vector2(1.0, 1.0);            
    mMonitorsCountChanged       = true;
    mOwnsGlobalData             = false;
}

//-----------------------------------------------------------------------
const String& ShaderExInstancedViewports::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int ShaderExInstancedViewports::getExecutionOrder() const
{
    // We place this effect after texturing stage and before fog stage.
    return FFP_POST_PROCESS+1;
}

//-----------------------------------------------------------------------
void ShaderExInstancedViewports::copyFrom(const SubRenderState& rhs)
{
    const ShaderExInstancedViewports& rhsInstancedViewports = static_cast<const ShaderExInstancedViewports&>(rhs);
    
    // Copy all settings that affect this sub render state output code.
    mMonitorsCount = rhsInstancedViewports.mMonitorsCount;
    mMonitorsCountChanged = rhsInstancedViewports.mMonitorsCountChanged;
}

//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::preAddToRenderState( const RenderState* renderState, Pass* srcPass, Pass* dstPass )
{
    return srcPass->getParent()->getParent()->getName().find("SdkTrays") == Ogre::String::npos;
}
//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();


    // Resolve vertex shader output position in projective space.

    mVSInPosition = vsMain->resolveInputParameter(Parameter::SPC_POSITION_OBJECT_SPACE);

    mVSOriginalOutPositionProjectiveSpace = vsMain->resolveOutputParameter(Parameter::SPC_POSITION_PROJECTIVE_SPACE);

#define SPC_POSITION_PROJECTIVE_SPACE_AS_TEXCORD ((Parameter::Content)(Parameter::SPC_CUSTOM_CONTENT_BEGIN + 1))

    mVSOutPositionProjectiveSpace = vsMain->resolveOutputParameter(SPC_POSITION_PROJECTIVE_SPACE_AS_TEXCORD, GCT_FLOAT4);

    // Resolve ps input position in projective space.
    mPSInPositionProjectiveSpace = psMain->resolveInputParameter(mVSOutPositionProjectiveSpace);
    // Resolve vertex shader uniform monitors count
    mVSInMonitorsCount = vsProgram->resolveParameter(GCT_FLOAT2, "monitorsCount");

    // Resolve pixel shader uniform monitors count
    mPSInMonitorsCount = psProgram->resolveParameter(GCT_FLOAT2, "monitorsCount");


    // Resolve the current world & view matrices concatenated   
    mWorldViewMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_WORLDVIEW_MATRIX);

    // Resolve the current projection matrix
    mProjectionMatrix = vsProgram->resolveParameter(GpuProgramParameters::ACT_PROJECTION_MATRIX);
    
    
#define SPC_MONITOR_INDEX Parameter::SPC_TEXTURE_COORDINATE3
    // Resolve vertex shader  monitor index
    mVSInMonitorIndex = vsMain->resolveInputParameter(SPC_MONITOR_INDEX, GCT_FLOAT4);

#define SPC_MATRIX_R0 Parameter::SPC_TEXTURE_COORDINATE4
#define SPC_MATRIX_R1 Parameter::SPC_TEXTURE_COORDINATE5
#define SPC_MATRIX_R2 Parameter::SPC_TEXTURE_COORDINATE6
#define SPC_MATRIX_R3 Parameter::SPC_TEXTURE_COORDINATE7

    // Resolve vertex shader viewport offset matrix
    mVSInViewportOffsetMatrixR0 = vsMain->resolveInputParameter(SPC_MATRIX_R0, GCT_FLOAT4);
    mVSInViewportOffsetMatrixR1 = vsMain->resolveInputParameter(SPC_MATRIX_R1, GCT_FLOAT4);
    mVSInViewportOffsetMatrixR2 = vsMain->resolveInputParameter(SPC_MATRIX_R2, GCT_FLOAT4);
    mVSInViewportOffsetMatrixR3 = vsMain->resolveInputParameter(SPC_MATRIX_R3, GCT_FLOAT4);


    
    // Resolve vertex shader output monitor index.
    mVSOutMonitorIndex = vsMain->resolveOutputParameter(SPC_MONITOR_INDEX, GCT_FLOAT4);

    // Resolve ps input monitor index.
    mPSInMonitorIndex = psMain->resolveInputParameter(mVSOutMonitorIndex);

    return true;
}

//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);

    vsProgram->addDependency(FFP_LIB_COMMON);
    vsProgram->addDependency(SGX_LIB_INSTANCED_VIEWPORTS);
    
    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_INSTANCED_VIEWPORTS);
    
    return true;
}


//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* psMain = psProgram->getEntryPointFunction();  
    

    // Add vertex shader invocations.
    if (false == addVSInvocations(vsMain, FFP_VS_TRANSFORM + 1))
        return false;


    // Add pixel shader invocations.
    if (false == addPSInvocations(psMain, FFP_PS_PRE_PROCESS + 1))
        return false;
    
    return true;
}

//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::addVSInvocations( Function* vsMain, const int groupOrder )
{
    FunctionAtom* funcInvocation = NULL;
    
    funcInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_INSTANCED_VIEWPORTS_TRANSFORM, groupOrder);
    funcInvocation->pushOperand(mVSInPosition, Operand::OPS_IN);
    funcInvocation->pushOperand(mWorldViewMatrix, Operand::OPS_IN);
    funcInvocation->pushOperand(mProjectionMatrix, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInViewportOffsetMatrixR0, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInViewportOffsetMatrixR1, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInViewportOffsetMatrixR2, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInViewportOffsetMatrixR3, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInMonitorsCount, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSInMonitorIndex, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSOriginalOutPositionProjectiveSpace, Operand::OPS_OUT);
    vsMain->addAtomInstance(funcInvocation);

    // Output position in projective space.
    funcInvocation = OGRE_NEW AssignmentAtom( groupOrder);
    funcInvocation->pushOperand(mVSOriginalOutPositionProjectiveSpace, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSOutPositionProjectiveSpace, Operand::OPS_OUT);
    vsMain->addAtomInstance(funcInvocation);

    // Output monitor index.
    funcInvocation = OGRE_NEW AssignmentAtom( groupOrder);
    funcInvocation->pushOperand(mVSInMonitorIndex, Operand::OPS_IN);
    funcInvocation->pushOperand(mVSOutMonitorIndex, Operand::OPS_OUT);
    vsMain->addAtomInstance(funcInvocation);

    return true;
}

//-----------------------------------------------------------------------
bool ShaderExInstancedViewports::addPSInvocations( Function* psMain, const int groupOrder )
{
    FunctionInvocation* funcInvocation = NULL;

    funcInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_INSTANCED_VIEWPORTS_DISCARD_OUT_OF_BOUNDS, groupOrder);
    funcInvocation->pushOperand(mPSInMonitorsCount, Operand::OPS_IN);
    funcInvocation->pushOperand(mPSInMonitorIndex, Operand::OPS_IN);
    funcInvocation->pushOperand(mPSInPositionProjectiveSpace, Operand::OPS_IN);
    
    psMain->addAtomInstance(funcInvocation);

    return true;
}
//-----------------------------------------------------------------------
void ShaderExInstancedViewports::updateGpuProgramsParams(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList)
{
    if (mMonitorsCountChanged)
    {
        mVSInMonitorsCount->setGpuParameter(mMonitorsCount + Vector2(0.0001, 0.0001));
        mPSInMonitorsCount->setGpuParameter(mMonitorsCount + Vector2(0.0001, 0.0001));

        mMonitorsCountChanged = false;
    }   
}
//-----------------------------------------------------------------------
void ShaderExInstancedViewports::setMonitorsCount( const Vector2 monitorCount )
{
    mMonitorsCount = monitorCount;
    mMonitorsCountChanged = true;

    Ogre::VertexDeclaration* vertexDeclaration = Ogre::HardwareBufferManager::getSingleton().createVertexDeclaration();
    size_t offset = 0;
    offset = vertexDeclaration->getVertexSize(0);
    vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 3);
    offset = vertexDeclaration->getVertexSize(0);
    vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 4);
    offset = vertexDeclaration->getVertexSize(0);
    vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 5);
    offset = vertexDeclaration->getVertexSize(0);
    vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 6);
    offset = vertexDeclaration->getVertexSize(0);
    vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_TEXTURE_COORDINATES, 7);

    Ogre::HardwareVertexBufferSharedPtr vbuf =
        Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        vertexDeclaration->getVertexSize(0), monitorCount.x * monitorCount.y, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf->setInstanceDataStepRate(1);
    vbuf->setIsInstanceData(true);

    float * buf = (float *)vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
    for (float x = 0 ; x < monitorCount.x ; x++)
        for (float y = 0 ; y < monitorCount.y ; y++)
        {
            *buf = x; buf++;
            *buf = y; buf++;
            *buf = 0; buf++;
            *buf = 0; buf++;

            Ogre::Quaternion q;
            Ogre::Radian angle = Ogre::Degree(90 / ( monitorCount.x *  monitorCount.y) * (x + y * monitorCount.x) );
            q.FromAngleAxis(angle,Ogre::Vector3::UNIT_Y);
            q.normalise();
            Ogre::Matrix3 rotMat;
            q.ToRotationMatrix(rotMat);

            *buf = rotMat.GetColumn(0).x; buf++;
            *buf = rotMat.GetColumn(0).y; buf++;
            *buf = rotMat.GetColumn(0).z; buf++;
            *buf = x * -20; buf++;

            *buf = rotMat.GetColumn(1).x; buf++;
            *buf = rotMat.GetColumn(1).y; buf++;
            *buf = rotMat.GetColumn(1).z; buf++;
            *buf = 0; buf++;

            *buf = rotMat.GetColumn(2).x; buf++;
            *buf = rotMat.GetColumn(2).y; buf++;
            *buf = rotMat.GetColumn(2).z; buf++;
            *buf =  y * 20; buf++;

            *buf = 0; buf++;
            *buf = 0; buf++;
            *buf = 0; buf++;
            *buf = 1; buf++;
        }
    vbuf->unlock();

    mOwnsGlobalData = true;

    auto rs = Ogre::Root::getSingleton().getRenderSystem();
    rs->setGlobalInstanceVertexBuffer(vbuf);
    rs->setGlobalInstanceVertexBufferVertexDeclaration(vertexDeclaration);
    rs->setGlobalNumberOfInstances(monitorCount.x * monitorCount.y);
}
ShaderExInstancedViewports::~ShaderExInstancedViewports()
{
    if (!mOwnsGlobalData)
        return;

    auto rs = Ogre::Root::getSingleton().getRenderSystem();
    if (rs->getGlobalInstanceVertexBufferVertexDeclaration())
    {
        Ogre::HardwareBufferManager::getSingleton().destroyVertexDeclaration(
            rs->getGlobalInstanceVertexBufferVertexDeclaration());
    }

    rs->setGlobalInstanceVertexBufferVertexDeclaration(NULL);
    rs->setGlobalNumberOfInstances(1);
    rs->setGlobalInstanceVertexBuffer(Ogre::HardwareVertexBufferSharedPtr() );
}

//-----------------------------------------------------------------------
SubRenderState* ShaderExInstancedViewportsFactory::createInstance(ScriptCompiler* compiler, 
                                                         PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    SubRenderState* subRenderState = SubRenderStateFactory::createInstance();
    return subRenderState;                              
}
//-----------------------------------------------------------------------
void ShaderExInstancedViewportsFactory::writeInstance(MaterialSerializer* ser, 
                                             SubRenderState* subRenderState, 
                                             Pass* srcPass, Pass* dstPass)
{

}

//-----------------------------------------------------------------------
const String& ShaderExInstancedViewportsFactory::getType() const
{
    return ShaderExInstancedViewports::Type;
}


//-----------------------------------------------------------------------
SubRenderState* ShaderExInstancedViewportsFactory::createInstanceImpl()
{
    return OGRE_NEW ShaderExInstancedViewports;
}

//-----------------------------------------------------------------------

}
}

#endif
