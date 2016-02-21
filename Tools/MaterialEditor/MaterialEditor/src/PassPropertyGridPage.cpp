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
#pragma warning(disable:4800)

#include "PassPropertyGridPage.h"

#include <wx/propgrid/advprops.h>

#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgrePass.h"
#include "OgrePrerequisites.h"

#include "PassController.h"

using namespace Ogre;

BEGIN_EVENT_TABLE(PassPropertyGridPage, wxPropertyGridPage)
    EVT_PG_CHANGED(-1, PassPropertyGridPage::propertyChanged)
END_EVENT_TABLE()

PassPropertyGridPage::PassPropertyGridPage(PassController* controller)
: mController(controller)
{
}

PassPropertyGridPage::~PassPropertyGridPage()
{
}

void PassPropertyGridPage::populate()
{   
    createGeneralCategory();
    createReflectanceCategory();
    createPointCategory();
    createSceneBlendingCategory();
    createDepthCategory();
    createCullingCategory();
    createIlluminationCategory();
    createFogCategory();
    createAlphaRejectionCategory();
    createMiscCategory();
}

void PassPropertyGridPage::createGeneralCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("General")));

    // Name
    mNameId = Append(wxStringProperty(wxT("Name"), wxPG_LABEL, pass->getName()));
    SetPropertyHelpString(mNameId, wxT("Name of this Pass"));
}

void PassPropertyGridPage::createReflectanceCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Reflectance")));

    // Ambient
    ColourValue ambient = pass->getAmbient();
    mAmbientId = Append(wxColourProperty(wxT("Ambient"), wxPG_LABEL, wxColour((int)(255 * ambient.r), (int)(255 * ambient.g), (int)(255 * ambient.b))));
    SetPropertyHelpString(mAmbientId, wxT("Ambient colour reflectance"));

    // Diffuse
    ColourValue diffuse = pass->getDiffuse();
    mDiffuseId = Append(wxColourProperty(wxT("Diffuse"), wxPG_LABEL, wxColour((int)(255 * diffuse.r), (int)(255 * diffuse.g), (int)(255 * diffuse.b))));
    SetPropertyHelpString(mDiffuseId, wxT("Diffuse colour reflectance"));

    // Specular
    ColourValue specular = pass->getSpecular();
    mSpecularId = Append(wxColourProperty(wxT("Specular"), wxPG_LABEL, wxColour((int)(255 * specular.r), (int)(255 * specular.g), (int)(255 * specular.b))));
    SetPropertyHelpString(mSpecularId, wxT("Specular colour reflectance"));

    // Shininess
    mShininessId = Append(wxFloatProperty(wxT("Shininess"), wxPG_LABEL, pass->getShininess()));
    SetPropertyHelpString(mShininessId, wxT("Shininess, affecting the size of specular highlights"));
}


void PassPropertyGridPage::createPointCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Point"), wxPG_LABEL));

    // Point Size
    wxPGId pointSize = Append(wxParentProperty(wxT("Size"), wxPG_LABEL));
    mPointSizeId = AppendIn(pointSize, wxFloatProperty(wxT("Size"), wxPG_LABEL, pass->getPointSize()));
    SetPropertyHelpString(mPointSizeId, wxT("Point size, affecting the size of points when rendering a point list, or a list of point sprites"));

    // Point Sprites
    mPointSpritesId = Append(wxBoolProperty(wxT("Point Sprites"), wxPG_LABEL, pass->getPointSpritesEnabled()));

    // Point Attenuation
    mAttenuationId = Append(wxParentProperty(wxT("Attenuation"),wxPG_LABEL)); 
    SetPropertyHelpString(mAttenuationId, wxT("Determines how points are attenuated with distance"));
    mPointAttenuationId = AppendIn(mAttenuationId, wxBoolProperty(wxT("Enabled"), wxPG_LABEL, pass->isPointAttenuationEnabled()));
    mPointMinSizeId = AppendIn(mAttenuationId, wxFloatProperty(wxT("Min"), wxPG_LABEL, pass->getPointMinSize()));
    SetPropertyHelpString(mPointMinSizeId, wxT("Minimum point size, when point attenuation is in use"));
    mPointMaxSizeId = AppendIn(mAttenuationId, wxFloatProperty(wxT("Max"), wxPG_LABEL, pass->getPointMaxSize()));
    SetPropertyHelpString(mAttenuationId, wxT("Maximum point size, when point attenuation is in use"));
    mPointAttenuationConstantId = AppendIn(mAttenuationId, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getPointAttenuationConstant()));
    SetPropertyHelpString(mPointAttenuationConstantId, wxT("Constant coefficient of the point attenuation"));
    mPointAttenuationLinearId = AppendIn(mAttenuationId, wxFloatProperty(wxT("Linear"), wxPG_LABEL, pass->getPointAttenuationLinear()));
    SetPropertyHelpString(mPointAttenuationLinearId, wxT("Linear coefficient of the point attenuation"));
    mPointAttenuationQuadraticId = AppendIn(mAttenuationId, wxFloatProperty(wxT("Quadratic"), wxPG_LABEL, pass->getPointAttenuationQuadratic()));
    SetPropertyHelpString(mPointAttenuationQuadraticId, wxT("Quadratic coefficient of the point attenuation"));
}


void PassPropertyGridPage::createSceneBlendingCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Scene Blending")));

    wxPGChoices sbtChoices;
    sbtChoices.Add(wxT("N/A") -1);
    sbtChoices.Add(wxT("Transparent Alpha"), SBT_TRANSPARENT_ALPHA);
    sbtChoices.Add(wxT("Transparent Colour"), SBT_TRANSPARENT_COLOUR);
    sbtChoices.Add(wxT("Add"), SBT_ADD);
    sbtChoices.Add(wxT("Modulate"), SBT_MODULATE);
    sbtChoices.Add(wxT("Replace"), SBT_REPLACE);

    wxPGChoices sbfChoices;
    sbfChoices.Add(wxT("One"), SBF_ONE);
    sbfChoices.Add(wxT("Zero"), SBF_ZERO);
    sbfChoices.Add(wxT("Dest Colour"), SBF_DEST_COLOUR);
    sbfChoices.Add(wxT("Src Colour"), SBF_SOURCE_COLOUR);
    sbfChoices.Add(wxT("One Minus Dest Colour"), SBF_ONE_MINUS_DEST_COLOUR);
    sbfChoices.Add(wxT("One Minus Src Colour"), SBF_ONE_MINUS_SOURCE_COLOUR);
    sbfChoices.Add(wxT("Dest Alpha"), SBF_DEST_ALPHA);
    sbfChoices.Add(wxT("Source Alpha"), SBF_SOURCE_ALPHA);
    sbfChoices.Add(wxT("One Minus Dest Alpha"), SBF_ONE_MINUS_DEST_ALPHA);
    sbfChoices.Add(wxT("One Minus Source Alpha"), SBF_ONE_MINUS_SOURCE_ALPHA);

    // Scene Blend Type
    bool type = true;
    SceneBlendType blendType;
    SceneBlendFactor srcFactor = pass->getSourceBlendFactor();
    SceneBlendFactor destFactor = pass->getDestBlendFactor();
    if(srcFactor == SBF_SOURCE_ALPHA && destFactor == SBF_ONE_MINUS_SOURCE_ALPHA)
        blendType = SBT_TRANSPARENT_ALPHA;
    else if(srcFactor == SBF_SOURCE_COLOUR && destFactor == SBF_ONE_MINUS_SOURCE_COLOUR)
        blendType = SBT_TRANSPARENT_COLOUR;
    else if(srcFactor == SBF_DEST_COLOUR && destFactor == SBF_ZERO)
        blendType = SBT_MODULATE;
    else if(srcFactor == SBF_ONE && destFactor == SBF_ONE)
        blendType = SBT_ADD;
    else if(srcFactor == SBF_ONE && destFactor == SBF_ZERO)
        blendType= SBT_REPLACE;
    else type = false;

    mSceneBlendTypeId = Append(wxEnumProperty(wxT("Type"), wxPG_LABEL, sbtChoices, (type) ? blendType : 0));

    // Source Scene Blend Type
    mSrcSceneBlendTypeId = Append(wxEnumProperty(wxT("Src Factor"), wxPG_LABEL, sbfChoices, srcFactor));

    // Destination Scene Blend Type
    mDestSceneBlendTypeId = Append(wxEnumProperty(wxT("Dest Factor"), wxPG_LABEL, sbfChoices, destFactor));
}


void PassPropertyGridPage::createDepthCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Depth")));

    // Depth Check
    mDepthCheckId = Append(wxBoolProperty(wxT("Depth Check"), wxPG_LABEL, pass->getDepthCheckEnabled()));

    // Depth Write
    mDepthWriteId = Append(wxBoolProperty(wxT("Depth Write"), wxPG_LABEL, pass->getDepthWriteEnabled()));   

    //  Depth Function
    wxPGChoices compareFuncChoices;
    compareFuncChoices.Add(wxT("Fail"), CMPF_ALWAYS_FAIL);
    compareFuncChoices.Add(wxT("Pass"), CMPF_ALWAYS_PASS);
    compareFuncChoices.Add(wxT("<"), CMPF_LESS);
    compareFuncChoices.Add(wxT("<="), CMPF_LESS_EQUAL);
    compareFuncChoices.Add(wxT("=="), CMPF_EQUAL);
    compareFuncChoices.Add(wxT("!="), CMPF_NOT_EQUAL);
    compareFuncChoices.Add(wxT(">="), CMPF_GREATER_EQUAL);
    compareFuncChoices.Add(wxT(">"), CMPF_GREATER);

    mDepthFunctionId = Append(wxEnumProperty(wxT("Depth Function"), wxPG_LABEL, compareFuncChoices, pass->getDepthFunction()));

    mDepthBiasId = Append(wxParentProperty(wxT("Depth Bias"), wxPG_LABEL));

    // Constant Bias
    mDepthBiasConstantId = AppendIn(mDepthBiasId, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getDepthBiasConstant()));

    // Slope Bias
    mDepthBiasSlopeId = AppendIn(mDepthBiasId, wxFloatProperty(wxT("Slope Scale"), wxPG_LABEL, pass->getDepthBiasSlopeScale()));
}

void PassPropertyGridPage::createCullingCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Culling")));

    // Culling Mode
    wxPGChoices cullingModeChoices;
    cullingModeChoices.Add(wxT("None"), CULL_NONE);
    cullingModeChoices.Add(wxT("Clockwise"), CULL_CLOCKWISE);
    cullingModeChoices.Add(wxT("Counterclockwise"), CULL_ANTICLOCKWISE);

    Append(wxEnumProperty(wxT("Culling Mode"), wxPG_LABEL, cullingModeChoices, pass->getDepthFunction()));

    // Manual Culling Mode
    wxPGChoices manualCullingModeChoices;
    manualCullingModeChoices.Add(wxT("None"), MANUAL_CULL_NONE);
    manualCullingModeChoices.Add(wxT("Back"), MANUAL_CULL_BACK);
    manualCullingModeChoices.Add(wxT("Front"), MANUAL_CULL_FRONT);

    mManualCullingModeId = Append(wxEnumProperty(wxT("Manual Culling Mode"), wxPG_LABEL, manualCullingModeChoices, pass->getManualCullingMode()));
}

void PassPropertyGridPage::createIlluminationCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Illumination")));

    mLightingId = Append(wxBoolProperty(wxT("Lighting"), wxPG_LABEL, pass->getLightingEnabled()));  

    // Max Simultaneous Lights
    mMaxLightsId = Append(wxIntProperty(wxT("Max Lights"), wxPG_LABEL, pass->getMaxSimultaneousLights()));

    // Start Light
    mStartLightId = Append(wxIntProperty(wxT("Start Light"), wxPG_LABEL, pass->getStartLight()));

    // Light Iteration
    mIterationId = Append(wxBoolProperty(wxT("Iteration"), wxPG_LABEL, pass->getIteratePerLight()));

    // Shading Mode
    wxPGChoices shadingModeChoices;
    shadingModeChoices.Add(wxT("Flat"), SO_FLAT);
    shadingModeChoices.Add(wxT("Gouraud"), SO_GOURAUD);
    shadingModeChoices.Add(wxT("Phong"), SO_PHONG);

    mShadingModeId = Append(wxEnumProperty(wxT("Shading Mode"), wxPG_LABEL, shadingModeChoices, pass->getShadingMode()));

    // Self Illumination
    ColourValue selfIllum = pass->getSelfIllumination();
    mSelfIlluminationId = Append(wxColourProperty(wxT("Self Illumination"), wxPG_LABEL, wxColour((int)(255 * selfIllum.r), (int)(255 * selfIllum.g), (int)(255 * selfIllum.b))));

}

void PassPropertyGridPage::createFogCategory()
{
    const Pass* pass = mController->getPass();

    mOverrideSceneId = Append(wxPropertyCategory(wxT("Fog")));

    // Fog Enabled
    mFogOverrideId = Append(wxBoolProperty(wxT("Override Scene"), wxPG_LABEL, pass->getFogOverride())); 

    // Fog Mode
    wxPGChoices fogModeChoices;
    fogModeChoices.Add(wxT("None"), FOG_NONE);
    fogModeChoices.Add(wxT("EXP"), FOG_EXP);
    fogModeChoices.Add(wxT("EXP2"), FOG_EXP2);
    fogModeChoices.Add(wxT("Linear"), FOG_LINEAR);

    mFogModeId = Append(wxEnumProperty(wxT("Fog Mode"), wxPG_LABEL, fogModeChoices, pass->getFogMode()));
}

// Possibly better as a wxParentProperty within Misc?
void PassPropertyGridPage::createAlphaRejectionCategory()
{
    const Pass* pass = mController->getPass();
    Append(wxPropertyCategory(wxT("Alpha Rejection"), wxPG_LABEL));
        
    // Alpha Reject Func
    wxPGChoices compareFuncChoices;
    compareFuncChoices.Add(wxT("Fail"), CMPF_ALWAYS_FAIL);
    compareFuncChoices.Add(wxT("Pass"), CMPF_ALWAYS_PASS);
    compareFuncChoices.Add(wxT("<"), CMPF_LESS);
    compareFuncChoices.Add(wxT("<="), CMPF_LESS_EQUAL);
    compareFuncChoices.Add(wxT("=="), CMPF_EQUAL);
    compareFuncChoices.Add(wxT("!="), CMPF_NOT_EQUAL);
    compareFuncChoices.Add(wxT(">="), CMPF_GREATER_EQUAL);
    compareFuncChoices.Add(wxT(">"), CMPF_GREATER);
    mAlphaRejectFuncId = Append(wxEnumProperty(wxT("Function"), wxPG_LABEL, compareFuncChoices, pass->getAlphaRejectFunction()));
    
    // Alpha Reject Value
    mAlphaRejectValueId = Append(wxIntProperty(wxT("Value"), wxPG_LABEL, pass->getAlphaRejectValue()));
}

void PassPropertyGridPage::createMiscCategory()
{
    const Pass* pass = mController->getPass();

    Append(wxPropertyCategory(wxT("Misc")));

    // Colour Write
    mColourWriteId = Append(wxBoolProperty(wxT("Colour Write"), wxPG_LABEL, pass->getColourWriteEnabled()));    

    // Polygon Mode
    wxPGChoices polygonModeChoices;
    polygonModeChoices.Add(wxT("Points"), PM_POINTS);
    polygonModeChoices.Add(wxT("Wireframe"), PM_WIREFRAME);
    polygonModeChoices.Add(wxT("Solid"), PM_SOLID);

    mPolygonModeId = Append(wxEnumProperty(wxT("Polygon Mode"), wxPG_LABEL, polygonModeChoices, pass->getPolygonMode()));

    // Track Vertex Colour Type
    wxPGChoices vertexColourTypeChoices;
    vertexColourTypeChoices.Add(wxT("None"), TVC_NONE);
    vertexColourTypeChoices.Add(wxT("Ambient"), TVC_AMBIENT);
    vertexColourTypeChoices.Add(wxT("Diffuse"), TVC_DIFFUSE);
    vertexColourTypeChoices.Add(wxT("Specular"), TVC_SPECULAR);
    vertexColourTypeChoices.Add(wxT("Emissive"), TVC_EMISSIVE);

    mTrackVertexColourTypeId = Append(wxEnumProperty(wxT("Track Vertex Colour Type"), wxPG_LABEL, vertexColourTypeChoices, pass->getVertexColourTracking()));
}

void PassPropertyGridPage::propertyChanged(wxPropertyGridEvent& event)
{
    wxPGId id = event.GetProperty();
    if(id == mNameId)
    {
        mController->setName(event.GetPropertyValueAsString().c_str());
    }
    else if(id == mAmbientId)
    {
        // TODO
    }
    else if(id == mDiffuseId)
    {
        // TODO
    }
    else if(id == mSpecularId)
    {
        // TODO
    }
    else if(id == mShininessId)
    {
        mController->setShininess((Real)event.GetPropertyValueAsDouble());
    }
    else if(id == mPointSizeId)
    {
        mController->setPointSize((Real)event.GetPropertyValueAsDouble());
    }
    else if(id == mPointSpritesId)
    {
        mController->setPointSpritesEnabled(event.GetPropertyValueAsBool());
    }
    //else if(id == mAttenuationId)
    //{
    //}
    else if(id == mPointAttenuationId)
    {
        const Pass* pass = mController->getPass();
        mController->setPointAttenuation(event.GetPropertyValueAsBool(), 
            pass->getPointAttenuationConstant(), pass->getPointAttenuationLinear(), 
            pass->getPointAttenuationQuadratic());
    }
    else if(id == mPointMinSizeId)
    {
        mController->setPointMinSize((Real)event.GetPropertyValueAsDouble());
    }
    else if(id == mPointMaxSizeId)
    {
        mController->setPointMaxSize((Real)event.GetPropertyValueAsDouble());
    }
    else if(id == mPointAttenuationConstantId)
    {
        const Pass* pass = mController->getPass();
        mController->setPointAttenuation(pass->isPointAttenuationEnabled(),
            (Real)event.GetPropertyValueAsDouble(), pass->getPointAttenuationLinear(),
            pass->getPointAttenuationQuadratic());
    }
    else if(id == mPointAttenuationLinearId)
    {
        const Pass* pass = mController->getPass();
        mController->setPointAttenuation(pass->isPointAttenuationEnabled(), 
            pass->getPointAttenuationConstant(), (Real)event.GetPropertyValueAsDouble(), 
            pass->getPointAttenuationQuadratic());
    }
    else if(id == mPointAttenuationQuadraticId)
    {
        const Pass* pass = mController->getPass();
        mController->setPointAttenuation(pass->isPointAttenuationEnabled(), 
            pass->getPointAttenuationConstant(), pass->getPointAttenuationLinear(), 
            (Real)event.GetPropertyValueAsDouble());
    }
    else if(id == mSceneBlendTypeId)
    {
        mController->setSceneBlending((SceneBlendType)event.GetPropertyValueAsInt());
    }
    else if(id == mSrcSceneBlendTypeId)
    {
        mController->setSceneBlending((SceneBlendFactor)event.GetPropertyValueAsInt(), 
            mController->getPass()->getDestBlendFactor());
    }
    else if(id == mDestSceneBlendTypeId)
    {
        mController->setSceneBlending(mController->getPass()->getSourceBlendFactor(), 
            (SceneBlendFactor)event.GetPropertyValueAsInt());
    }
    else if(id == mDepthCheckId)
    {
        mController->setDepthCheckEnabled(event.GetPropertyValueAsBool());
    }
    else if(id == mDepthWriteId)
    {
        mController->setDepthWriteEnabled(event.GetPropertyValueAsBool());
    }
    else if(id == mDepthFunctionId)
    {
        mController->setDepthFunction((CompareFunction)event.GetPropertyValueAsInt());
    }
    //else if(id == mDepthBiasId)
    //{
    //}
    else if(id == mDepthBiasConstantId)
    {
        mController->setDepthBias(event.GetPropertyValueAsDouble(), mController->getPass()->getDepthBiasSlopeScale());
    }
    else if(id == mDepthBiasSlopeId)
    {
        mController->setDepthBias(mController->getPass()->getDepthBiasConstant(), event.GetPropertyValueAsDouble());
    }
    else if(id == mManualCullingModeId)
    {
        mController->setManualCullingMode((ManualCullingMode)event.GetPropertyValueAsInt());
    }
    else if(id == mLightingId)
    {
        mController->setLightingEnabled(event.GetPropertyValueAsBool());
    }
    else if(id == mMaxLightsId)
    {
        mController->setMaxSimultaneousLights(event.GetPropertyValueAsInt());
    }
    else if(id == mStartLightId)
    {
        mController->setStartLight(event.GetPropertyValueAsInt());
    }
    else if(id == mIterationId)
    {
        mController->setLightCountPerIteration(event.GetPropertyValueAsInt());
    }
    else if(id == mShadingModeId)
    {
        mController->setShadingMode((ShadeOptions)event.GetPropertyValueAsInt());
    }
    else if(id == mSelfIlluminationId)
    {
        // TODO
    }
    else if(id == mOverrideSceneId)
    {
        const Pass* pass = mController->getPass();
        mController->setFog(event.GetPropertyValueAsBool(), pass->getFogMode(), pass->getFogColour(),
            pass->getFogDensity(), pass->getFogStart(), pass->getFogEnd());
    }
    else if(id == mFogOverrideId)
    {
        const Pass* pass = mController->getPass();
        mController->setFog(event.GetPropertyValueAsBool(), pass->getFogMode(), pass->getFogColour(),
            pass->getFogDensity(), pass->getFogStart(), pass->getFogEnd());
    }
    else if(id == mFogModeId)
    {
        const Pass* pass = mController->getPass();
        mController->setFog(pass->getFogOverride(), (FogMode)event.GetPropertyValueAsInt(), pass->getFogColour(),
            pass->getFogDensity(), pass->getFogStart(), pass->getFogEnd());
    }
    else if(id == mAlphaRejectFuncId)
    {
        mController->setAlphaRejectFunction((CompareFunction)event.GetPropertyValueAsInt());
    }
    else if(id == mAlphaRejectValueId)
    {
        mController->setAlphaRejectValue(event.GetPropertyValueAsInt());
    }
    else if(id == mColourWriteId)
    {
        mController->setColourWriteEnabled(event.GetPropertyValueAsBool());
    }
    else if(id == mPolygonModeId)
    {
        mController->setPolygonMode((PolygonMode)event.GetPropertyValueAsInt());
    }
    else if(id == mTrackVertexColourTypeId)
    {
        mController->setVertexColourTracking((TrackVertexColourType)event.GetPropertyValueAsInt());
    }
}
















