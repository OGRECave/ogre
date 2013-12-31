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
#ifndef _TEXTUREUNITPROPERTYGRIDPAGE_H_
#define _TEXTUREUNITPROPERTYGRIDPAGE_H_

#include <wx/wx.h>
#include <wx/propgrid/manager.h>

class TextureUnitController;
class EventArgs;

class TextureUnitPropertyGridPage : public wxPropertyGridPage
{
public:
	TextureUnitPropertyGridPage(TextureUnitController* controller);
	virtual ~TextureUnitPropertyGridPage();

	virtual void populate();

	void textureNameChanged(EventArgs& args);
	void cubicTextureNameChanged(EventArgs& args);
	void animatedTextureNameChanged(EventArgs& args);
	void textureDimensionsChanged(EventArgs& args);
	void currentFrameChanged(EventArgs& args);
	void frameTextureNameChanged(EventArgs& args);
	void frameTextureNameAdded(EventArgs& args);
	void frameTextureNameRemoved(EventArgs& args);
	void bindingTypeChanged(EventArgs& args);
	void contentTypeChanged(EventArgs& args);
	void pixelFormatChanged(EventArgs& args);
	void numMipmapsChanged(EventArgs& args);
	void alphaChanged(EventArgs& args);
	void textureCoordSetChanged(EventArgs& args);
	void textureTransformChanged(EventArgs& args);
	void textureScrollChanged(EventArgs& args);
	void textureUScrollChanged(EventArgs& args);
	void textureVScrollChanged(EventArgs& args);
	void textureUScaleChanged(EventArgs& args);
	void textureVScaleChanged(EventArgs& args);
	void textureRotateChanged(EventArgs& args);
	void textureAddressingModeChanged(EventArgs& args);
	void textureBorderColourChanged(EventArgs& args);
	void colourOperationExChanged(EventArgs& args);
	void colourOpMultipassFallbackChanged(EventArgs& args);
	void alphaOperationChanged(EventArgs& args);
	void environmentMapChanged(EventArgs& args);
	void scrollAnimationChanged(EventArgs& args);
	void rotateAnimationChanged(EventArgs& args);
	void transformAnimationChanged(EventArgs& args);
	void projectiveTexturingChanged(EventArgs& args);
	void textureFilteringChanged(EventArgs& args);
	void textureAnisotropyChanged(EventArgs& args);
	void textureMipMapBiasChanged(EventArgs& args);
	void nameChanged(EventArgs& args);
	void textureNameAliasChanged(EventArgs& args);

protected:
	virtual void propertyChanged(wxPropertyGridEvent& event);

	TextureUnitController* mController;

	wxPGId mTextureNameId;
    wxPGId mCubicTextureNameId;
    wxPGId mAnimatedTextureNameId;
    wxPGId mTextureDimensionsId;
    wxPGId mCurrentFrameId;
    wxPGId mFrameTextureNameId;
    wxPGId mBindingTypeId;
    wxPGId mContentTypeId;
    wxPGId mPixelFormatId;
    wxPGId mNumMipMapsId;
    wxPGId mAlphaId;
    wxPGId mTextureCoordSetId;
    wxPGId mTextureTransformId;
    wxPGId mTextureScrollId;
    wxPGId mTextureUScrollId;
    wxPGId mTextureVScrollId;
    wxPGId mTextureUScaleId;
    wxPGId mTextureVScaleId;
    wxPGId mTextureRotateId;
    wxPGId mTextureAddressingModeId;
    wxPGId mTextureBorderColourId;
    wxPGId mColourOperationExId;
    wxPGId mColourOpMultipassFallbackId;
    wxPGId mAlphaOperationId;
    wxPGId mEnvironmentMapId;
    wxPGId mScrollAnimationId;
    wxPGId mRotateAnimationId;
    wxPGId mTransformAnimationId;
    wxPGId mProjectiveTexturingId;
    wxPGId mTextureFilteringId;
    wxPGId mTextureAnisotropyId;
    wxPGId mTextureMipMapBiasId;
    wxPGId mNameId;
    wxPGId mTextureNameAliasId;


	DECLARE_EVENT_TABLE();
};

#endif // _TEXTUREUNITPROPERTYGRIDPAGE_H_
