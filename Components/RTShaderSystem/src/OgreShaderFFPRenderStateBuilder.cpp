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
#include "OgreShaderPrecompiledHeaders.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS

namespace Ogre {

namespace RTShader {


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildRenderState(ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState)
{
    renderState->reset();

    // Build transformation sub state.
    buildFFPSubRenderState(FFP_TRANSFORM, FFPTransform::Type, sgPass, renderState);

    // Build colour sub state.
    buildFFPSubRenderState(FFP_COLOUR, FFPColour::Type, sgPass, renderState);

    // Build lighting sub state.
#ifndef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
    buildFFPSubRenderState(FFP_LIGHTING, FFPLighting::Type, sgPass, renderState);
#else
    buildFFPSubRenderState(FFP_LIGHTING, PerPixelLighting::Type, sgPass, renderState);
#endif
    // Build texturing sub state.
    buildFFPSubRenderState(FFP_TEXTURING, FFPTexturing::Type, sgPass, renderState);
    
    // Build fog sub state.
    buildFFPSubRenderState(FFP_FOG, FFPFog::Type, sgPass, renderState);

    buildFFPSubRenderState(FFP_ALPHA_TEST, FFPAlphaTest::Type, sgPass, renderState);
	
    // Resolve colour stage flags.
    resolveColourStageFlags(sgPass, renderState);

}


//-----------------------------------------------------------------------------
static SubRenderState* getSubStateByOrder(int subStateOrder, const RenderState* renderState)
{
    for (auto curSubRenderState : renderState->getTemplateSubRenderStateList())
    {
        if (curSubRenderState->getExecutionOrder() == subStateOrder)
        {
            SubRenderState* clone;

            clone = ShaderGenerator::getSingleton().createSubRenderState(curSubRenderState->getType());
            *clone = *curSubRenderState;

            return clone;
        }
    }

    return NULL;
}
SubRenderState* FFPRenderStateBuilder::getCustomFFPSubState(ShaderGenerator::SGPass* sgPass, int subStateOrder)
{
    SubRenderState* customSubState = NULL;

    // Try to override with custom render state of this pass.
    if(auto customRenderState = sgPass->getCustomRenderState())
        customSubState = getSubStateByOrder(subStateOrder, customRenderState);

    // Case no custom sub state of this pass found, try to override with global scheme state.
    if (customSubState == NULL)
    {
        const String& schemeName = sgPass->getParent()->getDestinationTechniqueSchemeName();
        const RenderState* renderStateGlobal = ShaderGenerator::getSingleton().getRenderState(schemeName);

        customSubState = getSubStateByOrder(subStateOrder, renderStateGlobal);
    }

    return customSubState;
}
//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildFFPSubRenderState(int subRenderStateOrder, const String& subRenderStateType,
                                                ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState)
{
    SubRenderState* subRenderState = getCustomFFPSubState(sgPass, subRenderStateOrder);

    if (subRenderState == NULL)
    {
        subRenderState = ShaderGenerator::getSingleton().createSubRenderState(subRenderStateType);
    }

    if (subRenderState->preAddToRenderState(renderState, sgPass->getSrcPass(), sgPass->getDstPass()))
    {
        renderState->addSubRenderStateInstance(subRenderState);
    }
    else
    {
        ShaderGenerator::getSingleton().destroySubRenderState(subRenderState);
    }
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::resolveColourStageFlags( ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState )
{
    const SubRenderStateList& subRenderStateList = renderState->getTemplateSubRenderStateList();
    FFPColour* colourSubState = NULL;

    // Find the colour sub state.
    for (SubRenderStateListConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
    {
        SubRenderState* curSubRenderState = *it;

        if (curSubRenderState->getType() == FFPColour::Type)
        {
            colourSubState = static_cast<FFPColour*>(curSubRenderState);
            break;
        }
    }
    
    for (SubRenderStateListConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
    {
        SubRenderState* curSubRenderState = *it;

        // Add vertex shader specular lighting output in case of specular enabled.
        if (curSubRenderState->getType() == FFPLighting::Type && colourSubState != NULL)
        {
            colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_DIFFUSE);

            Pass* srcPass = sgPass->getSrcPass();

            if (srcPass->getShininess() > 0.0 &&
                srcPass->getSpecular() != ColourValue::Black)
            {
                colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_SPECULAR);              
            }   
            break;
        }
    }
}



}
}

#endif
