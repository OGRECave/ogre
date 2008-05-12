/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
