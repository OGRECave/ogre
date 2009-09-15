/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/

 Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "TextureUnitPropertyGridPage.h"

#include <boost/bind.hpp>

#include <wx/propgrid/advprops.h>

#include "OgreCommon.h"
#include "OgreTextureUnitState.h"

#include "TextureUnitController.h"

using namespace Ogre;

BEGIN_EVENT_TABLE(TextureUnitPropertyGridPage, wxPropertyGridPage)
EVT_PG_CHANGED(-1, TextureUnitPropertyGridPage::propertyChanged)
END_EVENT_TABLE()

TextureUnitPropertyGridPage::TextureUnitPropertyGridPage(TextureUnitController* controller)
: mController(controller)
{
	mController->subscribe(TextureUnitController::TextureNameChanged, boost::bind(&TextureUnitPropertyGridPage::textureNameChanged, this, _1));
	mController->subscribe(TextureUnitController::CubicTextureNameChanged, boost::bind(&TextureUnitPropertyGridPage::cubicTextureNameChanged, this, _1));
	mController->subscribe(TextureUnitController::AnimatedTextureNameChanged, boost::bind(&TextureUnitPropertyGridPage::animatedTextureNameChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureDimensionsChanged, boost::bind(&TextureUnitPropertyGridPage::textureDimensionsChanged, this, _1));
	mController->subscribe(TextureUnitController::CurrentFrameChanged, boost::bind(&TextureUnitPropertyGridPage::currentFrameChanged, this, _1));
	mController->subscribe(TextureUnitController::FrameTextureNameChanged, boost::bind(&TextureUnitPropertyGridPage::frameTextureNameChanged, this, _1));
	mController->subscribe(TextureUnitController::FrameTextureNameAdded, boost::bind(&TextureUnitPropertyGridPage::frameTextureNameAdded, this, _1));
	mController->subscribe(TextureUnitController::FrameTextureNameRemoved, boost::bind(&TextureUnitPropertyGridPage::frameTextureNameRemoved, this, _1));
	mController->subscribe(TextureUnitController::BindingTypeChanged, boost::bind(&TextureUnitPropertyGridPage::bindingTypeChanged, this, _1));
	mController->subscribe(TextureUnitController::ContentTypeChanged, boost::bind(&TextureUnitPropertyGridPage::contentTypeChanged, this, _1));
	mController->subscribe(TextureUnitController::PixelFormatChanged, boost::bind(&TextureUnitPropertyGridPage::pixelFormatChanged, this, _1));
	mController->subscribe(TextureUnitController::NumMipmapsChanged, boost::bind(&TextureUnitPropertyGridPage::numMipmapsChanged, this, _1));
	mController->subscribe(TextureUnitController::AlphaChanged, boost::bind(&TextureUnitPropertyGridPage::alphaChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureCoordSetChanged, boost::bind(&TextureUnitPropertyGridPage::textureCoordSetChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureTransformChanged, boost::bind(&TextureUnitPropertyGridPage::textureTransformChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureScrollChanged, boost::bind(&TextureUnitPropertyGridPage::textureScrollChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureUScrollChanged, boost::bind(&TextureUnitPropertyGridPage::textureUScrollChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureVScrollChanged, boost::bind(&TextureUnitPropertyGridPage::textureVScrollChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureUScaleChanged, boost::bind(&TextureUnitPropertyGridPage::textureUScaleChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureVScaleChanged, boost::bind(&TextureUnitPropertyGridPage::textureVScaleChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureRotateChanged, boost::bind(&TextureUnitPropertyGridPage::textureRotateChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureAddressingModeChanged, boost::bind(&TextureUnitPropertyGridPage::textureAddressingModeChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureBorderColourChanged, boost::bind(&TextureUnitPropertyGridPage::textureBorderColourChanged, this, _1));
	mController->subscribe(TextureUnitController::ColourOperationExChanged, boost::bind(&TextureUnitPropertyGridPage::colourOperationExChanged, this, _1));
	mController->subscribe(TextureUnitController::ColourOpMultipassFallbackChanged, boost::bind(&TextureUnitPropertyGridPage::colourOpMultipassFallbackChanged, this, _1));
	mController->subscribe(TextureUnitController::AlphaOperationChanged, boost::bind(&TextureUnitPropertyGridPage::alphaOperationChanged, this, _1));
	//mController->subscribe(TextureUnitController::EffectAdded, boost::bind(&TextureUnitPropertyGridPage::effectAdded, this, _1));
	mController->subscribe(TextureUnitController::EnvironmentMapChanged, boost::bind(&TextureUnitPropertyGridPage::environmentMapChanged, this, _1));
	mController->subscribe(TextureUnitController::ScrollAnimationChanged, boost::bind(&TextureUnitPropertyGridPage::scrollAnimationChanged, this, _1));
	mController->subscribe(TextureUnitController::RotateAnimationChanged, boost::bind(&TextureUnitPropertyGridPage::rotateAnimationChanged, this, _1));
	mController->subscribe(TextureUnitController::TransformAnimationChanged, boost::bind(&TextureUnitPropertyGridPage::projectiveTexturingChanged, this, _1));
	mController->subscribe(TextureUnitController::ProjectiveTexturingChanged, boost::bind(&TextureUnitPropertyGridPage::projectiveTexturingChanged, this, _1));
	//mController->subscribe(TextureUnitController::EffectRemoved, boost::bind(&TextureUnitPropertyGridPage::effectRemoved, this, _1));
	//mController->subscribe(TextureUnitController::Blanked, boost::bind(&TextureUnitPropertyGridPage::blanked, this, _1));
	mController->subscribe(TextureUnitController::TextureFilteringChanged, boost::bind(&TextureUnitPropertyGridPage::textureFilteringChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureAnisotropyChanged, boost::bind(&TextureUnitPropertyGridPage::textureAnisotropyChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureMipMapBiasChanged, boost::bind(&TextureUnitPropertyGridPage::textureMipMapBiasChanged, this, _1));
	mController->subscribe(TextureUnitController::NameChanged, boost::bind(&TextureUnitPropertyGridPage::nameChanged, this, _1));
	mController->subscribe(TextureUnitController::TextureNameAliasChanged, boost::bind(&TextureUnitPropertyGridPage::textureNameAliasChanged, this, _1));
}

TextureUnitPropertyGridPage::~TextureUnitPropertyGridPage() 
{
}

void TextureUnitPropertyGridPage::populate() 
{
	const TextureUnitState* tus = mController->getTextureUnit();
	
	mTextureNameId = Append(wxStringProperty(wxT("Texture Name"), wxPG_LABEL, tus->getTextureName()));
	mCurrentFrameId = Append(wxIntProperty(wxT("Current Frame"), wxPG_LABEL, tus->getCurrentFrame()));
	
	wxPGChoices btChoices;
	btChoices.Add(wxT("Fragment"), TextureUnitState::BT_FRAGMENT);
	btChoices.Add(wxT("Vertex"), TextureUnitState::BT_VERTEX);
	mBindingTypeId = Append(wxEnumProperty(wxT("Binding Type"), wxPG_LABEL, btChoices, TextureUnitState::BT_FRAGMENT));
	
	wxPGChoices ctChoices;
	ctChoices.Add(wxT("Named"), TextureUnitState::CONTENT_NAMED);
	ctChoices.Add(wxT("Shadow"), TextureUnitState::CONTENT_SHADOW);
	mContentTypeId = Append(wxEnumProperty(wxT("Content Type"), wxPG_LABEL, ctChoices, TextureUnitState::CONTENT_NAMED));
	
	wxPGChoices pfChoices;
	pfChoices.Add(wxT("Unknown"), PF_UNKNOWN);
	pfChoices.Add(wxT("L8"), PF_L8);
	pfChoices.Add(wxT("BYTE_L"), PF_BYTE_L);   
	pfChoices.Add(wxT("L16"), PF_L16); 
	pfChoices.Add(wxT("SHORT_L"), PF_SHORT_L);   
	pfChoices.Add(wxT("A8"), PF_A8); 
	pfChoices.Add(wxT("BYTE_A"), PF_BYTE_A);   
	pfChoices.Add(wxT("A4L4"), PF_A4L4);  
	pfChoices.Add(wxT("BYTE_LA"), PF_BYTE_LA);  
	pfChoices.Add(wxT("R5G6B5"), PF_R5G6B5);   
	pfChoices.Add(wxT("B5G6R5"), PF_B5G6R5); 
	pfChoices.Add(wxT("R3G3B2"), PF_R3G3B2);  
	pfChoices.Add(wxT("A4R4G4B4"), PF_A4R4G4B4);  
	pfChoices.Add(wxT("A1R5G5B5"), PF_A1R5G5B5); 
	pfChoices.Add(wxT("R8G8B8"), PF_R8G8B8);  
	pfChoices.Add(wxT("B8G8R8"), PF_B8G8R8);  
	pfChoices.Add(wxT("A8R8G8B8"), PF_A8R8G8B8);  
	pfChoices.Add(wxT("A8B8G8R8"), PF_A8B8G8R8);  
	pfChoices.Add(wxT("B8G8R8A8"), PF_B8G8R8A8);  
	pfChoices.Add(wxT("R8G8B8A8"), PF_R8G8B8A8); 
	pfChoices.Add(wxT("X8R8G8B8"), PF_X8R8G8B8);  
	pfChoices.Add(wxT("X8B8G8R8"), PF_X8B8G8R8);  
	pfChoices.Add(wxT("BYTE_RGB"), PF_BYTE_RGB);  
	pfChoices.Add(wxT("BYTE_BGR"), PF_BYTE_BGR);  
	pfChoices.Add(wxT("BYTE_BGRA"), PF_BYTE_BGRA);  
	pfChoices.Add(wxT("BYTE_RGBA"), PF_BYTE_RGBA);  
	pfChoices.Add(wxT("A2R10G10B10"), PF_A2R10G10B10);  
	pfChoices.Add(wxT("A2B10G10R10"), PF_A2B10G10R10); 
	pfChoices.Add(wxT("DXT1"), PF_DXT1); 
	pfChoices.Add(wxT("DXT2"), PF_DXT2); 
	pfChoices.Add(wxT("DXT3"), PF_DXT3); 
	pfChoices.Add(wxT("DXT4"), PF_DXT4);
	pfChoices.Add(wxT("DXT5"), PF_DXT5); 
	pfChoices.Add(wxT("FLOAT16_R"), PF_FLOAT16_R);   
	pfChoices.Add(wxT("FLOAT16_RGB"), PF_FLOAT16_RGB);   
	pfChoices.Add(wxT("FLOAT16_RGBA"), PF_FLOAT16_RGBA);   
	pfChoices.Add(wxT("FLOAT32_R"), PF_FLOAT32_R);   
	pfChoices.Add(wxT("FLOAT32_RGB"), PF_FLOAT32_RGB);   
	pfChoices.Add(wxT("FLOAT32_RGBA"), PF_FLOAT32_RGBA);   
	pfChoices.Add(wxT("FLOAT16_GR"), PF_FLOAT16_GR);   
	pfChoices.Add(wxT("FLOAT32_GR"), PF_FLOAT32_GR);   
	pfChoices.Add(wxT("PF_DEPTH"), PF_DEPTH);   
	pfChoices.Add(wxT("SHORT_RGBA"), PF_SHORT_RGBA);   
	pfChoices.Add(wxT("SHORT_GR"), PF_SHORT_GR);   
	pfChoices.Add(wxT("SHORT_RGB"), PF_SHORT_RGB);   
	pfChoices.Add(wxT("PF_COUNT"), PF_COUNT); 
	mPixelFormatId = Append(wxEnumProperty(wxT("Desired Format"), wxPG_LABEL, pfChoices, PF_UNKNOWN));
	
	mNumMipMapsId = Append(wxIntProperty(wxT("Mip Maps")));
}

void TextureUnitPropertyGridPage::propertyChanged(wxPropertyGridEvent& event) 
{
	wxPGId id = event.GetProperty();
	if(id == mTextureNameId) {}
	else if(id == mCubicTextureNameId) {}
	else if(id == mAnimatedTextureNameId) {}
	else if(id == mTextureDimensionsId) {}
	else if(id == mCurrentFrameId) {}
	else if(id == mFrameTextureNameId) {}
	else if(id == mBindingTypeId) {}
	else if(id == mContentTypeId) {}
	else if(id == mPixelFormatId) {}
	else if(id == mNumMipMapsId) {}
	else if(id == mAlphaId) {}
	else if(id == mTextureCoordSetId) {}
	else if(id == mTextureTransformId) {}
	else if(id == mTextureScrollId) {}
	else if(id == mTextureUScrollId) {}
	else if(id == mTextureVScrollId) {}
	else if(id == mTextureUScaleId) {}
	else if(id == mTextureVScaleId) {}
	else if(id == mTextureRotateId) {}
	else if(id == mTextureAddressingModeId) {}
	else if(id == mTextureBorderColourId) {}
	else if(id == mColourOperationExId) {}
	else if(id == mColourOpMultipassFallbackId) {}
	else if(id == mAlphaOperationId) {}
	else if(id == mEnvironmentMapId) {}
	else if(id == mScrollAnimationId) {}
	else if(id == mRotateAnimationId) {}
	else if(id == mTransformAnimationId) {}
	else if(id == mProjectiveTexturingId) {}
	else if(id == mTextureFilteringId) {}
	else if(id == mTextureAnisotropyId) {}
	else if(id == mTextureMipMapBiasId) {}
	else if(id == mNameId) {}
	else if(id == mTextureNameAliasId) {}
}

void TextureUnitPropertyGridPage::textureNameChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::cubicTextureNameChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mCubicTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::animatedTextureNameChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mAnimatedTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureDimensionsChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureDimensionsId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::currentFrameChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mCurrentFrameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::frameTextureNameChanged(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mFrameTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::frameTextureNameAdded(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mFrameTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::frameTextureNameRemoved(EventArgs& args) 
{
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mFrameTextureNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::bindingTypeChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mBindingTypeId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::contentTypeChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mContentTypeId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::pixelFormatChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mPixelFormatId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::numMipmapsChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mNumMipMapsId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::alphaChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mAlphaId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureCoordSetChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureCoordSetId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureTransformChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureTransformId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureScrollChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureScrollId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureUScrollChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureUScrollId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureVScrollChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureVScrollId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureUScaleChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureUScaleId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureVScaleChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureVScaleId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureRotateChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureRotateId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureAddressingModeChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureAddressingModeId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureBorderColourChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureBorderColourId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::colourOperationExChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mColourOperationExId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::colourOpMultipassFallbackChanged(
		EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mColourOpMultipassFallbackId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::alphaOperationChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mAlphaOperationId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::environmentMapChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mEnvironmentMapId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::scrollAnimationChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mScrollAnimationId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::rotateAnimationChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mRotateAnimationId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::transformAnimationChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTransformAnimationId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::projectiveTexturingChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mProjectiveTexturingId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureFilteringChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureFilteringId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureAnisotropyChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureAnisotropyId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureMipMapBiasChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureMipMapBiasId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::nameChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mNameId);
	if (prop == NULL)
		return;
}

void TextureUnitPropertyGridPage::textureNameAliasChanged(EventArgs& args) {
	TextureUnitEventArgs tuea = dynamic_cast<TextureUnitEventArgs&>(args);
	TextureUnitController* tc = tuea.getController();

	wxPGProperty* prop = GetPropertyPtr(mTextureNameAliasId);
	if (prop == NULL)
		return;
}

