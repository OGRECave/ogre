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
#ifndef _PASSPROPERTYGRIDPAGE_H_
#define _PASSPROPERTYGRIDPAGE_H_

#include <wx/wx.h>
#include <wx/propgrid/manager.h>

class PassController;

class PassPropertyGridPage : public wxPropertyGridPage
{
public:
	PassPropertyGridPage(PassController* controller);
	virtual ~PassPropertyGridPage();

	virtual void populate();

protected:
	virtual void createGeneralCategory();
	virtual void createReflectanceCategory();
	virtual void createPointCategory();
	virtual void createSceneBlendingCategory();
	virtual void createDepthCategory();
	virtual void createCullingCategory();
	virtual void createIlluminationCategory();
	virtual void createFogCategory();
	virtual void createAlphaRejectionCategory();
	virtual void createMiscCategory();

	virtual void propertyChanged(wxPropertyGridEvent& event);
	
	PassController* mController;

	wxPGId mNameId;

	wxPGId mAmbientId;
	wxPGId mDiffuseId;
	wxPGId mSpecularId;
	wxPGId mShininessId;

	wxPGId mPointSizeId;
	wxPGId mPointSpritesId;
	wxPGId mAttenuationId;
	wxPGId mPointAttenuationId;
	wxPGId mPointMinSizeId;
	wxPGId mPointMaxSizeId;
	wxPGId mPointAttenuationConstantId;
	wxPGId mPointAttenuationLinearId;
	wxPGId mPointAttenuationQuadraticId;

	wxPGId mSceneBlendTypeId;
	wxPGId mSrcSceneBlendTypeId;
	wxPGId mDestSceneBlendTypeId;

	wxPGId mDepthCheckId;
	wxPGId mDepthWriteId;
	wxPGId mDepthFunctionId;
	wxPGId mDepthBiasId;
	wxPGId mDepthBiasConstantId;
	wxPGId mDepthBiasSlopeId;

	wxPGId mManualCullingModeId;

	wxPGId mLightingId;
	wxPGId mMaxLightsId;
	wxPGId mStartLightId;
	wxPGId mIterationId;
	wxPGId mShadingModeId;
	wxPGId mSelfIlluminationId;

	wxPGId mOverrideSceneId;
	wxPGId mFogOverrideId;
	wxPGId mFogModeId;

	wxPGId mAlphaRejectFuncId;
	wxPGId mAlphaRejectValueId;

	wxPGId mColourWriteId;
	wxPGId mPolygonModeId;
	wxPGId mTrackVertexColourTypeId;

	DECLARE_EVENT_TABLE();
};

#endif // _PASSPROPERTYGRIDPAGE_H_