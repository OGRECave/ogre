/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreXSIMaterialExporter.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"

#include <xsi_shader.h>
#include <xsi_imageclip.h>
#include <xsi_image.h>


namespace Ogre {

	//-------------------------------------------------------------------------
	XsiMaterialExporter::XsiMaterialExporter()
	{

	}
	//-------------------------------------------------------------------------
	XsiMaterialExporter::~XsiMaterialExporter()
	{
		clearPassQueue();
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterials(MaterialMap& materials, 
		TextureProjectionMap& texProjMap, const String& filename, 
		bool copyTextures)
	{
		LogOgreAndXSI("** Begin OGRE Material Export **");
		
		mTextureProjectionMap = texProjMap;

		String texturePath;
		if (copyTextures)
		{
			// derive the texture path
			String::size_type pos = filename.find_last_of("\\");
			if (pos == String::npos)
			{
				pos = filename.find_last_of("/");			
			}
			if (pos != String::npos)
			{
				texturePath = filename.substr(0, pos + 1);
			}
		}
		
		mMatSerializer.clearQueue();

		for (MaterialMap::iterator m = materials.begin(); m != materials.end(); ++m)
		{
			exportMaterial(m->second, copyTextures, texturePath);
		}

		mMatSerializer.exportQueued(filename);

		LogOgreAndXSI("** OGRE Material Export Complete **");
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterial(MaterialEntry* matEntry, 
		bool copyTextures, const String& texturePath)
	{
		LogOgreAndXSI("Exporting " + matEntry->name);

		MaterialPtr mat = MaterialManager::getSingleton().create(
			matEntry->name,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Technique* t = mat->createTechnique();

		// collect the passes into our queue
		// XSI stores passes in reverse order, so invert them
		clearPassQueue();
		XSI::Shader shader(matEntry->xsiShader);
		PassEntry* passEntry = new PassEntry();
		mPassQueue.push_front(passEntry);
		while (1)
		{
			passEntry->shaders.Add(shader);

			XSI::CRef source = shader.GetParameter(L"previous").GetSource();
			if(!source.IsValid() || !source.IsA(XSI::siShaderID))
			{
				// finish
				break;
			}

			shader = XSI::Shader(source);
			// If we find a 'blending' parameter, we're on a new pass
			if (shader.GetParameter(L"blending").IsValid())
			{
				passEntry = new PassEntry();
				mPassQueue.push_front(passEntry); // push front to invert order
			}
		}


		// Now go through each pass and create OGRE version
		for (PassQueue::iterator p = mPassQueue.begin(); p != mPassQueue.end(); ++p)
		{
			PassEntry* passEntry = *p;
			Pass* pass = t->createPass();
			LogOgreAndXSI("Added Pass");

			// Need to pre-populate pass textures to match up transforms
			populatePassTextures(pass, passEntry, copyTextures, texturePath);
			// Do the rest
			for (int s = 0; s < passEntry->shaders.GetCount(); ++s)
			{
				XSI::Shader shader(passEntry->shaders[s]);
				populatePass(pass, shader);
			}

		}


		mMatSerializer.queueForExport(mat);
		

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::clearPassQueue(void)
	{
		for (PassQueue::iterator i = mPassQueue.begin(); i != mPassQueue.end(); ++i)
		{
			delete *i;
		}
		mPassQueue.clear();

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePass(Pass* pass, XSI::Shader& xsishader)
	{
		populatePassDepthCull(pass, xsishader);
		populatePassSceneBlend(pass, xsishader);
		populatePassLighting(pass, xsishader);
		populatePassTextureTransforms(pass, xsishader);
		populatePassCgPrograms(pass, xsishader);
		populatePassHLSLPrograms(pass, xsishader);
		populatePassD3DAssemblerPrograms(pass, xsishader);
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassCgPrograms(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"Cg_Program");
		if (param.IsValid())
		{
			// TODO
			// XSI can't reference external files which makes it v.difficult to 
			// re-use shaders - mod XSI plugin?
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassHLSLPrograms(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"HLSL_Program");
		if (param.IsValid())
		{
			// TODO
			// XSI can't reference external files which makes it v.difficult to 
			// re-use shaders - mod XSI plugin?
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassD3DAssemblerPrograms(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"Vertex_Shader");
		if (param.IsValid())
		{
			// TODO
			// XSI can't reference external files which makes it v.difficult to 
			// re-use shaders - mod XSI plugin?
		}
		param = xsishader.GetParameter(L"Pixel_Shader");
		if (param.IsValid())
		{
			// TODO
			// XSI can't reference external files which makes it v.difficult to 
			// re-use shaders - mod XSI plugin?
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassDepthCull(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"cullingmode");
		if (param.IsValid())
		{
			short xsiCull = param.GetValue();
			switch (xsiCull)
			{
			case 0:
				pass->setCullingMode(CULL_NONE);
				pass->setManualCullingMode(MANUAL_CULL_NONE);
				break;
			case 1:
				pass->setCullingMode(CULL_CLOCKWISE);
				pass->setManualCullingMode(MANUAL_CULL_BACK);
				break;
			case 2:
				pass->setCullingMode(CULL_ANTICLOCKWISE);
				pass->setManualCullingMode(MANUAL_CULL_FRONT);
				break;
					
			};
		}	

		param = xsishader.GetParameter(L"depthtest");
		if (param.IsValid())
		{
			bool depthTest = param.GetValue();
			pass->setDepthCheckEnabled(depthTest);
		}
		param = xsishader.GetParameter(L"depthwrite");
		if (param.IsValid())
		{
			bool depthWrite = param.GetValue();
			pass->setDepthWriteEnabled(depthWrite);
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassSceneBlend(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"blending");
		if (param.IsValid() && (bool)param.GetValue())
		{
			SceneBlendFactor src = SBF_ONE;
			SceneBlendFactor dst = SBF_ONE;
			
			param = xsishader.GetParameter(L"srcblendingfunction");
			if (param.IsValid())
			{
				src = convertSceneBlend(param.GetValue());
			}
			param = xsishader.GetParameter(L"dstblendingfunction");
			if (param.IsValid())
			{
				dst = convertSceneBlend(param.GetValue());
			}

			pass->setSceneBlending(src, dst);
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassLighting(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"Enable_Lighting");
		if (param.IsValid())
		{
			pass->setLightingEnabled(param.GetValue());

			ColourValue tmpColour;
			xsishader.GetColorParameterValue(L"Ambient", tmpColour.r, tmpColour.g, 
				tmpColour.b, tmpColour.a);
			pass->setAmbient(tmpColour);
			xsishader.GetColorParameterValue(L"Diffuse", tmpColour.r, tmpColour.g, 
				tmpColour.b, tmpColour.a);
			pass->setDiffuse(tmpColour);
			xsishader.GetColorParameterValue(L"Emissive", tmpColour.r, tmpColour.g, 
				tmpColour.b, tmpColour.a);
			pass->setSelfIllumination(tmpColour);
			xsishader.GetColorParameterValue(L"Specular", tmpColour.r, tmpColour.g, 
				tmpColour.b, tmpColour.a);
			pass->setSpecular(tmpColour);
			
			pass->setShininess(xsishader.GetParameter(L"Shininess").GetValue());
		}

		
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassTextures(Pass* pass, 
		PassEntry* passEntry, bool copyTextures, const String& targetFolder)
	{
		// We need to search all shaders back to the point we would change
		// passes, and add all the textures. This is because we don't know
		// where in the shaders the texture transforms might be, since they
		// are linked via 'target' not by being on the same object.
		mTextureUnitTargetMap.clear();

		for (int s = 0; s < passEntry->shaders.GetCount(); ++s)
		{
			XSI::Shader shader(passEntry->shaders[s]);
			TextureUnitState* tex = 0;

			String progID = XSItoOgre(shader.GetProgID());
			if (progID.find("OGL13Texture") != String::npos ||
				progID.find("DXTexture") != String::npos ||
				progID.find("OGLCom") != String::npos)
			{
				if (!shader.GetParameter(L"bottom").IsValid())
				{
					tex = add2DTexture(pass, shader, copyTextures, targetFolder);
				}
				else if (shader.GetParameter(L"bottom").IsValid())
				{
					tex = addCubicTexture(pass, shader, copyTextures, targetFolder);
				}
			}
			else
			{
				continue; // not a texture so skip the rest
			}


			// texture coordinate set
			XSI::Parameter param = shader.GetParameter(L"tspace_id");
			if (param.IsValid())
			{
				// this is a name, need to look up index
				tex->setTextureCoordSet(
					getTextureCoordIndex(XSItoOgre(XSI::CString(param.GetValue()))));
			}

			// filtering & anisotropy
			// DX and GL shaders deal differently
			if (progID.find("OGL") != String::npos)
			{
				populateOGLFiltering(tex, shader);
			}
			else if (progID.find("DX") != String::npos)
			{
				populateDXFiltering(tex, shader);
			}

			// colour operation
			param = shader.GetParameter(L"modulation");
			if (param.IsValid())
			{
				long colourop = ((LONG)param.GetValue());
				switch (colourop)
				{
				case 0:
					// modulate
					tex->setColourOperation(LBO_MODULATE);
					break;
				case 1:
					// decal
					tex->setColourOperation(LBO_ALPHA_BLEND);
					break;
				case 2:
					// blend
					tex->setColourOperation(LBO_MODULATE);
					break;
				case 3:
					// replace
					tex->setColourOperation(LBO_REPLACE);
					break;
				case 4:
					// add
					tex->setColourOperation(LBO_ADD);
					break;
				}

			}


			
		}
	}
	//-------------------------------------------------------------------------
	TextureUnitState* XsiMaterialExporter::add2DTexture(Pass* pass, XSI::Shader& shader, 
		bool copyTextures, const String& targetFolder)
	{
		// create texture unit state and map from target incase future xforms
		TextureUnitState* tex = pass->createTextureUnitState();

		XSI::Parameter param = shader.GetParameter(L"target"); // OGL
		if (!param.IsValid())
			param = shader.GetParameter(L"Texture_Target"); // DX

		long target = ((LONG)param.GetValue());
		mTextureUnitTargetMap.insert(
			TextureUnitTargetMap::value_type(target, tex));

		// Get image
		XSI::CRef src = shader.GetParameter(L"Texture").GetSource();
		if (!src.IsValid() || !src.IsA(XSI::siImageClipID))
		{
			// Try Texture_1 (OGL Combined)
			src = shader.GetParameter(L"Texture_1").GetSource();

		}
		if (src.IsValid() && src.IsA(XSI::siImageClipID))
		{
			XSI::ImageClip imgClip(src);
			String srcTextureName = 
				XSItoOgre(XSI::CString(imgClip.GetParameter(L"SourceFileName").GetValue()));

			String::size_type pos = srcTextureName.find_last_of("\\");
			if (pos == String::npos)
			{
				pos = srcTextureName.find_last_of("/");
			}
			String textureName = 
				srcTextureName.substr(pos+1, srcTextureName.size() - pos - 1);
			String destTextureName = targetFolder + textureName;

			// copy texture if required
			if (copyTextures)
			{
				copyFile(srcTextureName, destTextureName);
			}

			LogOgreAndXSI("Adding texture " + textureName);
			tex->setTextureName(textureName);

		}

		return tex;
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populateOGLFiltering(TextureUnitState* tex, 
		XSI::Shader& shader)
	{
		FilterOptions minFilter, mipFilter, magFilter;
		minFilter = FO_LINEAR;
		magFilter = FO_LINEAR;
		mipFilter = FO_POINT;
		XSI::Parameter param = shader.GetParameter(L"minfilter");
		if (param.IsValid())
		{
			// XSI OGL shader uses minfilter to determine mip too
			long filt = ((LONG)param.GetValue());
			switch(filt)
			{
			case 0:
				minFilter = FO_POINT;
				mipFilter = FO_NONE;
				break;
			case 1:
				minFilter = FO_LINEAR;
				mipFilter = FO_NONE;
				break;
			case 2:
				minFilter = FO_POINT;
				mipFilter = FO_POINT;
				break;
			case 3:
				minFilter = FO_POINT;
				mipFilter = FO_LINEAR;
				break;
			case 4:
				minFilter = FO_LINEAR;
				mipFilter = FO_POINT;
				break;
			case 5:
				minFilter = FO_LINEAR;
				mipFilter = FO_LINEAR;
				break;
			};

		}
		param = shader.GetParameter(L"magfilter");
		if (param.IsValid())
		{
			long filt = ((LONG)param.GetValue());
			switch(filt)
			{
			case 0:
				magFilter = FO_POINT;
				break;
			case 1:
				magFilter = FO_LINEAR;
				break;
			};
		}

		param = shader.GetParameter(L"anisotropy");
		if (param.IsValid())
		{
			long aniso = ((LONG)param.GetValue());
			if (aniso > 1)
			{
				// No specific aniso filtering option, so upgrade linear -> aniso
				if (minFilter == FO_LINEAR)
					minFilter = FO_ANISOTROPIC;
				if (magFilter == FO_LINEAR)
					magFilter = FO_ANISOTROPIC;
			}
			tex->setTextureAnisotropy(aniso);
		}

		tex->setTextureFiltering(minFilter, magFilter, mipFilter);
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populateDXFiltering(TextureUnitState* tex, 
		XSI::Shader& shader)
	{
		FilterOptions minFilter, mipFilter, magFilter;
		minFilter = FO_LINEAR;
		magFilter = FO_LINEAR;
		mipFilter = FO_POINT;
		XSI::Parameter param = shader.GetParameter(L"minfilter");
		if (param.IsValid())
		{
			// XSI DX shader has min/mag and mip, and has more options
			long filt = ((LONG)param.GetValue());
			switch(filt)
			{
			case 0:
				minFilter = FO_NONE;
				break;
			case 1:
				minFilter = FO_POINT;
				break;
			case 2:
				minFilter = FO_LINEAR;
				break;
			case 3:
			case 4: // we don't support cubic/gaussian, use aniso
			case 5:
				minFilter = FO_ANISOTROPIC;
				break;
			};
		}
		param = shader.GetParameter(L"magfilter");
		if (param.IsValid())
		{
			// XSI DX shader has mag/mag and mip, and has more options
			long filt = ((LONG)param.GetValue());
			switch(filt)
			{
			case 0:
				magFilter = FO_NONE;
				break;
			case 1:
				magFilter = FO_POINT;
				break;
			case 2:
				magFilter = FO_LINEAR;
				break;
			case 3:
			case 4: // we don't support cubic/gaussian, use aniso
			case 5:
				magFilter = FO_ANISOTROPIC;
				break;
			};
		}
		param = shader.GetParameter(L"mipfilter");
		if (param.IsValid())
		{
			// XSI DX shader has mip/mag and mip, and has more options
			long filt = ((LONG)param.GetValue());
			switch(filt)
			{
			case 0:
				mipFilter = FO_NONE;
				break;
			case 1:
				mipFilter = FO_POINT;
				break;
			case 2:
				mipFilter = FO_LINEAR;
				break;
			case 3:
			case 4: // we don't support cubic/gaussian, use aniso
			case 5:
				mipFilter = FO_ANISOTROPIC;
				break;
			};
		}
		// Aniso
		param = shader.GetParameter(L"anisotropy");
		if (param.IsValid())
		{
			long aniso = ((LONG)param.GetValue());
			tex->setTextureAnisotropy(aniso);
		}

		tex->setTextureFiltering(minFilter, magFilter, mipFilter);
	}
	//-------------------------------------------------------------------------
	TextureUnitState* XsiMaterialExporter::addCubicTexture(Pass* pass, XSI::Shader& shader, 
		bool copyTextures, const String& targetFolder)
	{
		// create texture unit state and map from target incase future xforms
		TextureUnitState* tex = pass->createTextureUnitState();

		XSI::Parameter param = shader.GetParameter(L"target"); // OGL
		if (!param.IsValid())
			param = shader.GetParameter(L"Texture_Target"); // DX

		long target = ((LONG)param.GetValue());
		mTextureUnitTargetMap.insert(
			TextureUnitTargetMap::value_type(target, tex));

		// Get images
		wchar_t* cubeFaceName[6] = {
			L"front", L"back", L"top", L"bottom", L"left", L"right" };

		String finalNames[6];


		for (int face = 0; face < 6; ++face)
		{
			XSI::CRef src = shader.GetParameter(cubeFaceName[face]).GetSource();
			if (src.IsValid() && src.IsA(XSI::siImageClipID))
			{
				XSI::ImageClip imgClip(src);
				String srcTextureName = 
					XSItoOgre(XSI::CString(imgClip.GetParameter(L"SourceFileName").GetValue()));

				String::size_type pos = srcTextureName.find_last_of("\\");
				if (pos == String::npos)
				{
					pos = srcTextureName.find_last_of("/");
				}
				finalNames[face] = 
					srcTextureName.substr(pos+1, srcTextureName.size() - pos - 1);
				String destTextureName = targetFolder + finalNames[face];

				// copy texture if required
				if (copyTextures)
				{
					copyFile(srcTextureName, destTextureName);
				}


				LogOgreAndXSI("Cubemap face: " + srcTextureName);
			}
		}

		LogOgreAndXSI("Adding cubic texture");
		// Cannot do combinedUVW for now, DevIL can't write DDS cubemap so 
		// go for separates (user can modify)
		tex->setCubicTextureName(finalNames, false);

		return tex;
		
	}
	//-------------------------------------------------------------------------
	unsigned short XsiMaterialExporter::getTextureCoordIndex(const String& tspace)
	{
		TextureProjectionMap::iterator i = mTextureProjectionMap.find(tspace);
		if (i != mTextureProjectionMap.end())
		{
			return i->second;
		}
		else
		{
			// default
			return 0;
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassTextureTransforms(Pass* pass, 
		XSI::Shader& shader)
	{
		// TODO
		// Check we have the right object
		XSI::Parameter param = shader.GetParameter(L"wrap_u");
		if (param.IsValid())
		{

			// addressing mode
			TextureUnitState::UVWAddressingMode uvwadd;
			uvwadd.u = convertAddressingMode(param.GetValue());
			// default other dimensions incase not supplied
			uvwadd.v = uvwadd.u;
			uvwadd.w = uvwadd.u;

			param = shader.GetParameter(L"wrap_v");
			if (param.IsValid())
			{
				uvwadd.v = convertAddressingMode(param.GetValue());
			}
			param = shader.GetParameter(L"wrap_w");
			if (param.IsValid())
			{
				uvwadd.w = convertAddressingMode(param.GetValue());
			}

			// transform
			bool usexform = false;
			Matrix4 xform = Matrix4::IDENTITY;
			param = shader.GetParameter(L"Transform");
			if (param.IsValid() && (bool)param.GetValue())
			{
				Quaternion qx, qy, qz, qfinal;
				qx.FromAngleAxis(Degree(shader.GetParameter(L"rotx").GetValue()),
					Vector3::UNIT_X);
				qy.FromAngleAxis(Degree(shader.GetParameter(L"roty").GetValue()), 
					Vector3::UNIT_Y);
				qz.FromAngleAxis(Degree(shader.GetParameter(L"rotz").GetValue()), 
					Vector3::UNIT_Z);
				qfinal = qx * qy * qz;

				Vector3 trans;
				trans.x = shader.GetParameter(L"trsx").GetValue();
				trans.y = shader.GetParameter(L"trsy").GetValue();
				trans.z = shader.GetParameter(L"trsz").GetValue();

				Matrix3 rot3x3, scale3x3;
				qfinal.ToRotationMatrix(rot3x3);
				scale3x3 = Matrix3::ZERO;
				scale3x3[0][0] = shader.GetParameter(L"sclx").GetValue();
				scale3x3[1][1] = shader.GetParameter(L"scly").GetValue();
				scale3x3[2][2] = shader.GetParameter(L"sclz").GetValue();

				xform = rot3x3 * scale3x3;
				xform.setTrans(trans);
				usexform = true;

			}
			

			// Look up texture unit(s) that are using this target
			long target = ((LONG)shader.GetParameter(L"Texture_Target").GetValue());
			TextureUnitTargetMap::iterator i = mTextureUnitTargetMap.find(target);
			while (i != mTextureUnitTargetMap.end() && i->first == target)
			{
				TextureUnitState* tex = i->second;
				tex->setTextureAddressingMode(uvwadd);
				if (usexform)
					tex->setTextureTransform(xform);

				// texgen (not texcoord_index as in OGRE!)
				// Can turn into 2 different calls
				param = shader.GetParameter(L"texcoord_index");
				if (param.IsValid())
				{
					long e = ((LONG)param.GetValue());
					if (e != 0)
					{
						// Not Explicit
						// details differ per DX/OGL
						if (XSItoOgre(shader.GetProgID()).find("DX") != String::npos)
						{
							convertTexGenDX(tex, e, shader);
						}
						else
						{
							convertTexGenOGL(tex, e, shader);
						}

					}

				}
				++i;
			}




		}
		param = shader.GetParameter(L"Additive_Transform");
		if (param.IsValid())
		{
			unsigned short target = shader.GetParameter(L"Texture_Coord_ID").GetValue();
			if (pass->getNumTextureUnitStates() > target)
			{
				TextureUnitState* tex = pass->getTextureUnitState(target);

				long uvType = ((LONG)shader.GetParameter(L"UV_Type").GetValue());
				if (uvType != 0)
				{
					double val1 = shader.GetParameter(L"Val1").GetValue();
					double val2 = shader.GetParameter(L"Val2").GetValue();
					long wave = ((LONG)shader.GetParameter(L"Wave").GetValue());
					WaveformType wft;
					switch (wave)
					{
					case 1:
						wft = WFT_SINE;
						break;
					case 2:
						wft = WFT_TRIANGLE;
						break;
					case 3:
						wft = WFT_SQUARE;
						break;
					case 4:
						wft = WFT_SAWTOOTH;
						break;
					case 5:
						wft = WFT_INVERSE_SAWTOOTH;
						break;
					}
					double base = shader.GetParameter(L"Base").GetValue();
					double amp = shader.GetParameter(L"Amplitude").GetValue();
					double phase = shader.GetParameter(L"Phase").GetValue();
					double freq = shader.GetParameter(L"Frequency").GetValue();

					switch(uvType)
					{
					case 1: 
						// translate
						tex->setTextureScroll(val1, val2);
						break;
					case 2: 
						// rotate
						tex->setTextureRotate(Degree(val1));
						break;
					case 3: 
						// scale
						tex->setTextureScale(val1, val2);
						break;
					case 4: 
						// scroll
						if (wave != 0)
						{
							tex->setTransformAnimation(TextureUnitState::TT_TRANSLATE_U, 
								wft, base, freq, phase, amp);
							tex->setTransformAnimation(TextureUnitState::TT_TRANSLATE_V, 
								wft, base, freq, phase, amp);
						}
						else
						{
							tex->setScrollAnimation(val1, val2);
						}
						break;
					case 5: 
						// turn
						if (wave != 0)
						{
							tex->setTransformAnimation(TextureUnitState::TT_ROTATE, 
								wft, base, freq, phase, amp);
						}
						else
						{
							tex->setRotateAnimation(val1 / 360.0f);
						}
						break;
					case 6: 
						// stretch (only wave)
						if (wave != 0)
						{
							tex->setTransformAnimation(TextureUnitState::TT_SCALE_U, 
								wft, base, freq, phase, amp);
							tex->setTransformAnimation(TextureUnitState::TT_SCALE_V, 
								wft, base, freq, phase, amp);
						}
						break;

					}
				}
			}

		}

		// if more than one entry for the same target is found, it is ok for the
		// latter to take precedence since this is what happens in XSI.

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::convertTexGenOGL(TextureUnitState* tex, 
		long xsiVal, XSI::Shader& shader)
	{
		switch(xsiVal)
		{
			// no 0
		case 1:
			// Object linear
			tex->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
			break;
		case 2:
			// Eye linear (texture projection)
			tex->setProjectiveTexturing(true);
			break;
		case 3:
			// Sphere map
			tex->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
			break;
		case 4:
			// Reflection map
			tex->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
			break;
		case 5:
			// Normal map
			tex->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
			break;

		};
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::convertTexGenDX(TextureUnitState* tex, 
		long xsiVal, XSI::Shader& shader)
	{
		switch(xsiVal)
		{
			// no 0
		case 1:
			// Normal in camera space
			tex->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
			break;
		case 2:
			// Position in camera space
			tex->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
			break;
		case 3:
			// Reflection vector in camera space
			tex->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
			break;

		};
	}
	//-------------------------------------------------------------------------
	TextureUnitState::TextureAddressingMode 
	XsiMaterialExporter::convertAddressingMode(short xsiVal)
	{
		// same for OGL and DX
		switch(xsiVal)
		{
		case 0:
			return TextureUnitState::TAM_WRAP;
		case 1:
			return TextureUnitState::TAM_MIRROR;
		case 2: 
			return TextureUnitState::TAM_CLAMP;
		case 3: 
			// border?
			return TextureUnitState::TAM_CLAMP;
		case 4:
			// mirror once
			return TextureUnitState::TAM_MIRROR;
		case 5: 
			// clamp to edge
			return TextureUnitState::TAM_CLAMP;

		};

		// Keep compiler happy
		return TextureUnitState::TAM_WRAP;
	}
	//-------------------------------------------------------------------------
	SceneBlendFactor XsiMaterialExporter::convertSceneBlend(short xsiVal)
	{
		switch(xsiVal)
		{
		case 0:
			return SBF_ZERO;
		case 1:
			return SBF_ONE;
		case 2:
			return SBF_DEST_COLOUR;
		case 3:
			return SBF_ONE_MINUS_DEST_COLOUR;
		case 4:
			return SBF_SOURCE_ALPHA;
		case 5:
			return SBF_ONE_MINUS_SOURCE_ALPHA;
		case 6: 
			return SBF_DEST_ALPHA;
		case 7:
			return SBF_ONE_MINUS_DEST_ALPHA;
		};

		return SBF_ZERO;
		
	}

}

