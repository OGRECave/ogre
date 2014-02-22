////////////////////////////////////////////////////////////////////////////////
// material.h
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

#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "mayaExportLayer.h"
#include "paramList.h"

namespace OgreMayaExporter
{

    typedef enum {MT_SURFACE_SHADER,MT_LAMBERT,MT_PHONG,MT_BLINN,MT_CGFX} MaterialType;

    typedef enum {TOT_REPLACE,TOT_MODULATE,TOT_ADD,TOT_ALPHABLEND} TexOpType;

    typedef enum {TAM_CLAMP,TAM_BORDER,TAM_WRAP,TAM_MIRROR} TexAddressMode;

    class Texture
    {
    public:
        //constructor
        Texture() {
            scale_u = scale_v = 1;
            scroll_u = scroll_v = 0;
            rot = 0;
            am_u = am_v = TAM_CLAMP;
        }
        //destructor
        ~Texture(){};
    
        //public members
        MString filename;
        MString absFilename;
        TexOpType opType;
        MString uvsetName;
        int uvsetIndex;
        TexAddressMode am_u,am_v;
        double scale_u,scale_v;
        double scroll_u,scroll_v;
        double rot;
    };


    /***** Class Material *****/
    class Material
    {
    public:
        //constructor
        Material();
        //destructor
        ~Material();
        //get material name
        MString& name();
        //clear material data
        void clear();
        //load material data
        MStatus load(MFnDependencyNode* pShader,MStringArray& uvsets,ParamList& params);
        //load a specific material type
        MStatus loadSurfaceShader(MFnDependencyNode* pShader);
        MStatus loadLambert(MFnDependencyNode* pShader);
        MStatus loadPhong(MFnDependencyNode* pShader);
        MStatus loadBlinn(MFnDependencyNode* pShader);
        MStatus loadCgFxShader(MFnDependencyNode* pShader);
        //write material data to Ogre material script
        MStatus writeOgreScript(ParamList &params);
        //copy textures to path specified by params
        MStatus copyTextures(ParamList &params);
    public:
        //load texture data
        MStatus loadTexture(MFnDependencyNode* pTexNode,TexOpType& opType,MStringArray& uvsets,ParamList& params);

        MString m_name;
        MaterialType m_type;
        MColor m_ambient, m_diffuse, m_specular, m_emissive;
        bool m_lightingOff;
        bool m_isTransparent;
        bool m_isTextured;
        bool m_isMultiTextured;
        std::vector<Texture> m_textures;
    };

};  //end of namespace

#endif