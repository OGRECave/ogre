////////////////////////////////////////////////////////////////////////////////
// material.cpp
// Author       : Francesco Giordana
// Sponsored by : Anygma N.V. (http://www.nazooka.com)
// Start Date   : January 13, 2005
// Copyright    : (C) 2006 by Francesco Giordana
// Email        : fra.giordana@tiscali.it
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#include "material.h"

namespace OgreMayaExporter
{
	// Constructor
	Material::Material()
	{
		clear();
	}


	// Destructor
	Material::~Material()
	{
	}


	// Get material name
	MString& Material::name()
	{
		return m_name;
	}


	// Clear data
	void Material::clear()
	{
		m_name = "";
		m_type = MT_LAMBERT;
		m_lightingOff = false;
		m_isTransparent = false;
		m_isTextured = false;
		m_isMultiTextured = false;
		m_ambient = MColor(0,0,0,0);
		m_diffuse = MColor(0,0,0,0);
		m_specular = MColor(0,0,0,0);
		m_emissive = MColor(0,0,0,0);
		m_textures.clear();
	}


	// Load material data
	MStatus Material::load(MFnDependencyNode* pShader,MStringArray& uvsets,ParamList& params)
	{
		MStatus stat;
		clear();
		//read material name, adding the requested prefix
		MString tmpStr = params.matPrefix;
		if (tmpStr != "")
			tmpStr += "/";
		tmpStr += pShader->name();
		MStringArray tmpStrArray;
		tmpStr.split(':',tmpStrArray);
		m_name = "";
		for (int i=0; i<tmpStrArray.length(); i++)
		{
			m_name += tmpStrArray[i];
			if (i < tmpStrArray.length()-1)
				m_name += "_";
		}

		//check if we want to export with lighting off option
		m_lightingOff = params.lightingOff;

		// GET MATERIAL DATA

		// Check material type
		if (pShader->object().hasFn(MFn::kPhong))
		{
			stat = loadPhong(pShader);
		}
		else if (pShader->object().hasFn(MFn::kBlinn))
		{
			stat = loadBlinn(pShader);
		}
		else if (pShader->object().hasFn(MFn::kLambert))
		{
			stat = loadLambert(pShader);
		}
		else if (pShader->object().hasFn(MFn::kPluginHwShaderNode))
		{
			stat = loadCgFxShader(pShader);
		}
		else
		{
			stat = loadSurfaceShader(pShader);
		}

		// Get textures data
		MPlugArray colorSrcPlugs;
		MPlugArray texSrcPlugs;
		MPlugArray placetexSrcPlugs;
		if (m_isTextured)
		{
			// Translate multiple textures if material is multitextured
			if (m_isMultiTextured)
			{
				// Get layered texture node
				MFnDependencyNode* pLayeredTexNode = NULL;
				if (m_type == MT_SURFACE_SHADER)
					pShader->findPlug("outColor").connectedTo(colorSrcPlugs,true,false);
				else
					pShader->findPlug("color").connectedTo(colorSrcPlugs,true,false);
				for (int i=0; i<colorSrcPlugs.length(); i++)
				{
					if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
					{
						pLayeredTexNode = new MFnDependencyNode(colorSrcPlugs[i].node());
						continue;
					}
				}

				// Get inputs to layered texture
				MPlug inputsPlug = pLayeredTexNode->findPlug("inputs");

				// Scan inputs and export textures
				for (int i=inputsPlug.numElements()-1; i>=0; i--)
				{
					MFnDependencyNode* pTextureNode = NULL;
					// Search for a connected texture
					inputsPlug[i].child(0).connectedTo(colorSrcPlugs,true,false);
					for (int j=0; j<colorSrcPlugs.length(); j++)
					{
						if (colorSrcPlugs[j].node().hasFn(MFn::kFileTexture))
						{
							pTextureNode = new MFnDependencyNode(colorSrcPlugs[j].node());
							continue;
						}
					}

					// Translate the texture if it was found
					if (pTextureNode)
					{
						// Get blend mode
						TexOpType opType;
						short bm;
						inputsPlug[i].child(2).getValue(bm);
						switch(bm)
						{				
						case 0:
							opType = TOT_REPLACE;
							break;
						case 1:
							opType = TOT_ALPHABLEND;
							break;				
						case 4:
							opType = TOT_ADD;
							break;
						case 6:
							opType = TOT_MODULATE;
							break;
						default:
							opType = TOT_MODULATE;
						}

						stat = loadTexture(pTextureNode,opType,uvsets,params);
						delete pTextureNode;
						if (MS::kSuccess != stat)
						{
							std::cout << "Error loading layered texture\n";
							std::cout.flush();
							delete pLayeredTexNode;
							return MS::kFailure;
						}
					}
				}
				if (pLayeredTexNode)
					delete pLayeredTexNode;
			}
			// Else translate the single texture
			else
			{
				// Get texture node
				MFnDependencyNode* pTextureNode = NULL;
				if (m_type == MT_SURFACE_SHADER)
					pShader->findPlug("outColor").connectedTo(colorSrcPlugs,true,false);
				else
					pShader->findPlug("color").connectedTo(colorSrcPlugs,true,false);
				for (int i=0; i<colorSrcPlugs.length(); i++)
				{
					if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
					{
						pTextureNode = new MFnDependencyNode(colorSrcPlugs[i].node());
						continue;
					}
				}
				if (pTextureNode)
				{
					TexOpType opType = TOT_MODULATE;
					stat = loadTexture(pTextureNode,opType,uvsets,params);
					delete pTextureNode;
					if (MS::kSuccess != stat)
					{
						std::cout << "Error loading texture\n";
						std::cout.flush();
						return MS::kFailure;
					}
				}
			}
		}

		return MS::kSuccess;
	}


	// Load a surface shader
	MStatus Material::loadSurfaceShader(MFnDependencyNode *pShader)
	{
		m_type = MT_SURFACE_SHADER;
		MPlugArray colorSrcPlugs;
		// Check if material is textured
		pShader->findPlug("outColor").connectedTo(colorSrcPlugs,true,false);
		for (int i=0; i<colorSrcPlugs.length(); i++)
		{
			if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
			{
				m_isTextured = true;
				continue;
			}
			else if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
			{
				m_isTextured = true;
				m_isMultiTextured = true;
				continue;
			}
		}

		// Check if material is transparent
		float trasp;
		pShader->findPlug("outTransparencyR").getValue(trasp);
		if (pShader->findPlug("outTransparency").isConnected() || trasp>0.0f)
			m_isTransparent = true;

		// Get material colours
		if (m_isTextured)
			m_diffuse = MColor(1.0,1.0,1.0,1.0);
		else
		{
			pShader->findPlug("outColorR").getValue(m_diffuse.r);
			pShader->findPlug("outColorG").getValue(m_diffuse.g);
			pShader->findPlug("outColorB").getValue(m_diffuse.b);
			float trasp;
			pShader->findPlug("outTransparencyR").getValue(trasp);
			m_diffuse.a = 1.0 - trasp;
		}
		m_ambient = MColor(0,0,0,1);
		m_emissive = MColor(0,0,0,1);
		m_specular = MColor(0,0,0,1);
		return MS::kSuccess;
	}

	// Load a lambert shader
	MStatus Material::loadLambert(MFnDependencyNode *pShader)
	{
		MPlugArray colorSrcPlugs;
		m_type = MT_LAMBERT;
		MFnLambertShader* pLambert = new MFnLambertShader(pShader->object());
		// Check if material is textured
		pLambert->findPlug("color").connectedTo(colorSrcPlugs,true,false);
		for (int i=0; i<colorSrcPlugs.length(); i++)
		{
			if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
			{
				m_isTextured = true;
				continue;
			}
			else if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
			{
				m_isTextured = true;
				m_isMultiTextured = true;
				continue;
			}
		}

		// Check if material is transparent
		if (pLambert->findPlug("transparency").isConnected() || pLambert->transparency().r>0.0f)
			m_isTransparent = true;

		// Get material colours
		//diffuse colour
		if (m_isTextured)
			m_diffuse = MColor(1.0,1.0,1.0,1.0);
		else
		{
			m_diffuse = pLambert->color();
			m_diffuse.a = 1.0 - pLambert->transparency().r;
		}
		//ambient colour
		m_ambient = pLambert->ambientColor();
		//emissive colour
		m_emissive = pLambert->incandescence();
		//specular colour
		m_specular = MColor(0,0,0,1);
		delete pLambert;
		return MS::kSuccess;
	}

	// Load a phong shader
	MStatus Material::loadPhong(MFnDependencyNode *pShader)
	{
		MPlugArray colorSrcPlugs;
		m_type = MT_PHONG;
		MFnPhongShader* pPhong = new MFnPhongShader(pShader->object());
		// Check if material is textured
		pPhong->findPlug("color").connectedTo(colorSrcPlugs,true,false);
		for (int i=0; i<colorSrcPlugs.length(); i++)
		{
			if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
			{
				m_isTextured = true;
				continue;
			}
			else if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
			{
				m_isTextured = true;
				m_isMultiTextured = true;
				continue;
			}
		}

		// Check if material is transparent
		if (pPhong->findPlug("transparency").isConnected() || pPhong->transparency().r>0.0f)
			m_isTransparent = true;

		// Get material colours
		//diffuse colour
		if (m_isTextured)
			m_diffuse = MColor(1.0,1.0,1.0,1.0);
		else
		{
			m_diffuse = pPhong->color();
			m_diffuse.a = 1.0 - pPhong->transparency().r;
		}
		//ambient colour
		m_ambient = pPhong->ambientColor();
		//emissive colour
		m_emissive = pPhong->incandescence();
		//specular colour
		m_specular = pPhong->specularColor();
		m_specular.a = pPhong->cosPower()*1.28;
		delete pPhong;
		return MS::kSuccess;
	}

	// load a blinn shader
	MStatus Material::loadBlinn(MFnDependencyNode *pShader)
	{
		MPlugArray colorSrcPlugs;
		m_type = MT_BLINN;
		MFnBlinnShader* pBlinn = new MFnBlinnShader(pShader->object());
		// Check if material is textured
		pBlinn->findPlug("color").connectedTo(colorSrcPlugs,true,false);
		for (int i=0; i<colorSrcPlugs.length(); i++)
		{
			if (colorSrcPlugs[i].node().hasFn(MFn::kFileTexture))
			{
				m_isTextured = true;
				continue;
			}
			else if (colorSrcPlugs[i].node().hasFn(MFn::kLayeredTexture))
			{
				m_isTextured = true;
				m_isMultiTextured = true;
				continue;
			}
		}

		// Check if material is transparent
		if (pBlinn->findPlug("transparency").isConnected() || pBlinn->transparency().r>0.0f)
			m_isTransparent = true;

		// Get material colours
		//diffuse colour
		if (m_isTextured)
			m_diffuse = MColor(1.0,1.0,1.0,1.0);
		else
		{
			m_diffuse = pBlinn->color();
			m_diffuse.a = 1.0 - pBlinn->transparency().r;
		}
		//ambient colour
		m_ambient = pBlinn->ambientColor();
		//emissive colour
		m_emissive = pBlinn->incandescence();
		//specular colour
		m_specular = pBlinn->specularColor();
		m_specular.a = 128.0-(128.0*pBlinn->eccentricity());
		delete pBlinn;
		return MS::kSuccess;
	}

	// load a cgFx shader
	MStatus Material::loadCgFxShader(MFnDependencyNode *pShader)
	{
		m_type = MT_CGFX;
		// Create a default white lambert
		m_isTextured = false;
		m_isMultiTextured = false;
		m_diffuse = MColor(1.0,1.0,1.0,1.0);
		m_specular = MColor(0,0,0,1);
		m_emissive = MColor(0,0,0,1);
		m_ambient = MColor(0,0,0,1);
		return MS::kSuccess;
	}

	// Load texture data from a texture node
	MStatus Material::loadTexture(MFnDependencyNode* pTexNode,TexOpType& opType,MStringArray& uvsets,ParamList& params)
	{
		Texture tex;
		// Get texture filename
		MString filename, absFilename;
		MRenderUtil::exactFileTextureName(pTexNode->object(),absFilename);
		filename = absFilename.substring(absFilename.rindex('/')+1,absFilename.length()-1);
		MString command = "toNativePath(\"";
		command += absFilename;
		command += "\")";
		MGlobal::executeCommand(command,absFilename);
		tex.absFilename = absFilename;
		tex.filename = filename;
		tex.uvsetIndex = 0;
		tex.uvsetName = "";
		// Set texture operation type
		tex.opType = opType;
		// Get connections to uvCoord attribute of texture node
		MPlugArray texSrcPlugs;
		pTexNode->findPlug("uvCoord").connectedTo(texSrcPlugs,true,false);
		// Get place2dtexture node (if connected)
		MFnDependencyNode* pPlace2dTexNode = NULL;
		for (int j=0; j<texSrcPlugs.length(); j++)
		{
			if (texSrcPlugs[j].node().hasFn(MFn::kPlace2dTexture))
			{
				pPlace2dTexNode = new MFnDependencyNode(texSrcPlugs[j].node());
				continue;
			}
		}
		// Get uvChooser node (if connected)
		MFnDependencyNode* pUvChooserNode = NULL;
		if (pPlace2dTexNode)
		{
			MPlugArray placetexSrcPlugs;
			pPlace2dTexNode->findPlug("uvCoord").connectedTo(placetexSrcPlugs,true,false);
			for (int j=0; j<placetexSrcPlugs.length(); j++)
			{
				if (placetexSrcPlugs[j].node().hasFn(MFn::kUvChooser))
				{
					pUvChooserNode = new MFnDependencyNode(placetexSrcPlugs[j].node());
					continue;
				}
			}
		}
		// Get uvset index
		if (pUvChooserNode)
		{
			bool foundMesh = false;
			bool foundUvset = false;
			MPlug uvsetsPlug = pUvChooserNode->findPlug("uvSets");
			MPlugArray uvsetsSrcPlugs;
			for (int i=0; i<uvsetsPlug.evaluateNumElements() && !foundMesh; i++)
			{
				uvsetsPlug[i].connectedTo(uvsetsSrcPlugs,true,false);
				for (int j=0; j<uvsetsSrcPlugs.length() && !foundMesh; j++)
				{
					if (uvsetsSrcPlugs[j].node().hasFn(MFn::kMesh))
					{
						uvsetsSrcPlugs[j].getValue(tex.uvsetName);
						for (int k=0; k<uvsets.length() && !foundUvset; k++)
						{
							if (uvsets[k] == tex.uvsetName)
							{
								tex.uvsetIndex = k;
								foundUvset = true;
							}
						}
					}
				}
			}
		}
		// Get texture options from Place2dTexture node
		if (pPlace2dTexNode)
		{
			// Get address mode
			//U
			bool wrapU, mirrorU;
			pPlace2dTexNode->findPlug("wrapU").getValue(wrapU);
			pPlace2dTexNode->findPlug("mirrorU").getValue(mirrorU);
			if (mirrorU)
				tex.am_u = TAM_MIRROR;
			else if (wrapU)
				tex.am_u = TAM_WRAP;
			else
				tex.am_u = TAM_CLAMP;
			// V
			bool wrapV,mirrorV;
			pPlace2dTexNode->findPlug("wrapV").getValue(wrapV);
			pPlace2dTexNode->findPlug("mirrorV").getValue(mirrorV);
			if (mirrorV)
				tex.am_v = TAM_MIRROR;
			else if (wrapV)
				tex.am_v = TAM_WRAP;
			else
				tex.am_v = TAM_CLAMP;
			// Get texture scale
			double covU,covV;
			pPlace2dTexNode->findPlug("coverageU").getValue(covU);
			pPlace2dTexNode->findPlug("coverageV").getValue(covV);
			tex.scale_u = covU;
			if (fabs(tex.scale_u) < PRECISION)
				tex.scale_u = 0;
			tex.scale_v = covV;
			if (fabs(tex.scale_v) < PRECISION)
				tex.scale_v = 0;
			// Get texture scroll
			double transU,transV;
			pPlace2dTexNode->findPlug("translateFrameU").getValue(transU);
			pPlace2dTexNode->findPlug("translateFrameV").getValue(transV);
			tex.scroll_u = -0.5 * (covU-1.0)/covU - transU/covU;
			if (fabs(tex.scroll_u) < PRECISION)
				tex.scroll_u = 0;
			tex.scroll_v = 0.5 * (covV-1.0)/covV + transV/covV;
			if (fabs(tex.scroll_v) < PRECISION)
				tex.scroll_v = 0;
			// Get texture rotation
			double rot;
			pPlace2dTexNode->findPlug("rotateFrame").getValue(rot);
			tex.rot = -rot;
			if (fabs(rot) < PRECISION)
				tex.rot = 0;
		}
		// add texture to material texture list
		m_textures.push_back(tex);
		// free up memory
		if (pUvChooserNode)
			delete pUvChooserNode;
		if (pPlace2dTexNode)
			delete pPlace2dTexNode;

		return MS::kSuccess;
	}


	// Write material data to an Ogre material script file
	MStatus Material::writeOgreScript(ParamList &params)
	{
		//Start material description
		params.outMaterial << "material " << m_name.asChar() << "\n";
		params.outMaterial << "{\n";

		//Start technique description
		params.outMaterial << "\ttechnique\n";
		params.outMaterial << "\t{\n";

		//Start render pass description
		params.outMaterial << "\t\tpass\n";
		params.outMaterial << "\t\t{\n";
		//set lighting off option if requested
		if (m_lightingOff)
			params.outMaterial << "\t\t\tlighting off\n\n";
		//set phong shading if requested (default is gouraud)
		if (m_type == MT_PHONG)
			params.outMaterial << "\t\t\tshading phong\n";
		//ambient colour
		params.outMaterial << "\t\t\tambient " << m_ambient.r << " " << m_ambient.g << " " << m_ambient.b
			<< " " << m_ambient.a << "\n";
		//diffuse colour
		params.outMaterial << "\t\t\tdiffuse " << m_diffuse.r << " " << m_diffuse.g << " " << m_diffuse.b
			<< " " << m_diffuse.a << "\n";
		//specular colour
		params.outMaterial << "\t\t\tspecular " << m_specular.r << " " << m_specular.g << " " << m_specular.b
			<< " " << m_specular.a << "\n";
		//emissive colour
		params.outMaterial << "\t\t\temissive " << m_emissive.r << " " << m_emissive.g << " " 
			<< m_emissive.b << "\n";
		//if material is transparent set blend mode and turn off depth_writing
		if (m_isTransparent)
		{
			params.outMaterial << "\n\t\t\tscene_blend alpha_blend\n";
			params.outMaterial << "\t\t\tdepth_write off\n";
		}
		//write texture units
		for (int i=0; i<m_textures.size(); i++)
		{
			//start texture unit description
			params.outMaterial << "\n\t\t\ttexture_unit\n";
			params.outMaterial << "\t\t\t{\n";
			//write texture name
			params.outMaterial << "\t\t\t\ttexture " << m_textures[i].filename.asChar() << "\n";
			//write texture coordinate index
			params.outMaterial << "\t\t\t\ttex_coord_set " << m_textures[i].uvsetIndex << "\n";
			//write colour operation
			switch (m_textures[i].opType)
			{
			case TOT_REPLACE:
				params.outMaterial << "\t\t\t\tcolour_op replace\n";
				break;
			case TOT_ADD:
				params.outMaterial << "\t\t\t\tcolour_op add\n";
				break;
			case TOT_MODULATE:
				params.outMaterial << "\t\t\t\tcolour_op modulate\n";
				break;
			case TOT_ALPHABLEND:
				params.outMaterial << "\t\t\t\tcolour_op alpha_blend\n";
				break;
			}
			//write texture transforms
			params.outMaterial << "\t\t\t\tscale " << m_textures[i].scale_u << " " << m_textures[i].scale_v << "\n";
			params.outMaterial << "\t\t\t\tscroll " << m_textures[i].scroll_u << " " << m_textures[i].scroll_v << "\n";
			params.outMaterial << "\t\t\t\trotate " << m_textures[i].rot << "\n";
			//end texture unit desription
			params.outMaterial << "\t\t\t}\n";
		}

		//End render pass description
		params.outMaterial << "\t\t}\n";

		//End technique description
		params.outMaterial << "\t}\n";

		//End material description
		params.outMaterial << "}\n";

		//Copy textures to output dir if required
		if (params.copyTextures)
			copyTextures(params);

		return MS::kSuccess;
	}


	// Copy textures to path specified by params
	MStatus Material::copyTextures(ParamList &params)
	{
		for (int i=0; i<m_textures.size(); i++)
		{
			// Copy file texture to output dir
			MString command = "copy \"";
			command += m_textures[i].absFilename;
			command += "\" \"";
			command += params.texOutputDir;
			command += "\"";
			system(command.asChar());
		}
		return MS::kSuccess;
	}

};	//end namespace