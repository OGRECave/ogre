////////////////////////////////////////////////////////////////////////////////
// materialSet.h
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

#ifndef _MATERIALSET_H
#define _MATERIALSET_H

#include "_singleton.h"
#include "material.h"
#include "mayaExportLayer.h"

namespace OgreMayaExporter
{
	class MaterialSet : public Singleton<MaterialSet>
	{
	public:
		//constructor
		MaterialSet(){
			//create a default material
			m_pDefaultMat = new Material();
			m_pDefaultMat->m_type = MT_LAMBERT;
			m_pDefaultMat->m_name = "defaultLambert";
			m_pDefaultMat->m_ambient = MColor(0,0,0,1);
			m_pDefaultMat->m_emissive = MColor(0,0,0,1);
			m_pDefaultMat->m_diffuse = MColor(0.5,0.5,0.5,1);
		};
		//destructor
		~MaterialSet(){
			clear();
			if (m_pDefaultMat)
				delete m_pDefaultMat;
		}
		//clear
		void clear(){
			for (int i=0; i<m_materials.size(); i++)
				delete m_materials[i];
			m_materials.clear();
		}
		//add material
		void addMaterial(Material* pMat){
			bool found = false;
			for (int i=0; i<m_materials.size() && !found; i++)
			{
				if (m_materials[i]->name() == pMat->name())
				{
					found = true;
					delete pMat;
				}
			}
			if (!found)
				m_materials.push_back(pMat);
		}
		//get material
		Material* getMaterial(const MString& name){
			for (int i=0; i<m_materials.size(); i++)
			{
				if (m_materials[i]->name() == name)
					return m_materials[i];
			}
			return NULL;
		};
		//get default material
		Material* getDefaultMaterial()
		{
			return m_pDefaultMat;
		};
		//get material set
		static MaterialSet& getSingleton(){
			assert(ms_Singleton);  
			return (*ms_Singleton);
		};
		static MaterialSet* getSingletonPtr(){
			return ms_Singleton;
		};
		//write materials to Ogre XML
		MStatus writeOgreScript(ParamList &params){
			MStatus stat;
			for (int i=0; i<m_materials.size(); i++)
			{
				stat = m_materials[i]->writeOgreScript(params);
				if (MS::kSuccess != stat)
				{
					MString msg = "Error writing material ";
					msg += m_materials[i]->name();
					msg += ", aborting operation";
					MGlobal::displayInfo(msg);
				}
			}
			return MS::kSuccess;
		};

	protected:
		std::vector<Material*> m_materials;
		Material* m_pDefaultMat;
	};

};	//end namespace

#endif