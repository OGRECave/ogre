#include "OgreMaxMaterialExport.h"
#include "max.h"
#include "plugapi.h"
#include "stdmat.h"
#include "impexp.h"
#include "IGame/IGame.h"

namespace OgreMax {

	MaterialExporter::MaterialExporter(const Config& config, MaterialMap& map) : m_materialMap(map), OgreMaxExporter(config)
	{
	}

	MaterialExporter::~MaterialExporter()
	{
	}


	bool MaterialExporter::buildMaterial(std::string& output) {

		std::stringstream of;
		of.precision(6);
		of << std::fixed;
		output = "";

		if (streamMaterial(of))
			output = of.str();
		else
			return false;

		return true;
	}

	//bool MaterialExporter::streamPass(std::ostream &of, Mtl *mtl) {
	bool MaterialExporter::streamPass(std::ostream &of, IGameMaterial *mtl) {
		of << "\t\tpass" << std::endl;
		of << "\t\t{" << std::endl;

		int subMtlCt = mtl->GetSubMaterialCount();

		Point4 val4;
		Point3 val3;
		PropType pt;
		IGameProperty* p = mtl->GetAmbientData();

		if (p) {
			pt = p->GetType();

			if (pt == IGAME_POINT3_PROP) {
				p->GetPropertyValue(val3);
				of << "\t\t\tambient " << val3.x << " " << val3.y << " " << val3.z << " " << std::endl;
			}

			if (pt == IGAME_POINT4_PROP) {
				p->GetPropertyValue(val4);
				of << "\t\t\tambient " << val4.x << " " << val4.y << " " << val4.z << " " << val4.w << " " << std::endl;
			}
		}

		p = mtl->GetDiffuseData();
		if (p) {
			pt = p->GetType();

			if (pt == IGAME_POINT3_PROP) {
				p->GetPropertyValue(val3);
				of << "\t\t\tdiffuse " << val3.x << " " << val3.y << " " << val3.z << " " << std::endl;
			}

			if (pt == IGAME_POINT4_PROP) {
				p->GetPropertyValue(val4);
				of << "\t\t\tdiffuse " << val4.x << " " << val4.y << " " << val4.z << " " << val4.w << " " << std::endl;
			}
		}

		p = mtl->GetSpecularData();
		if (p) {
			pt = p->GetType();

			if (pt == IGAME_POINT3_PROP) {
				p->GetPropertyValue(val3);
				of << "\t\t\tspecular " << val3.x << " " << val3.y << " " << val3.z << " " << std::endl;
			}

			if (pt == IGAME_POINT4_PROP) {
				p->GetPropertyValue(val4);
				of << "\t\t\tspecular " << val4.x << " " << val4.y << " " << val4.z << " " << val4.w << " " << std::endl;
			}
		}

		p = mtl->GetEmissiveData();
		if (p) {
			pt = p->GetType();

			if (pt == IGAME_POINT3_PROP) {
				p->GetPropertyValue(val3);
				of << "\t\t\temissive " << val3.x << " " << val3.y << " " << val3.z << " " << std::endl;
			}

			if (pt == IGAME_POINT4_PROP) {
				p->GetPropertyValue(val4);
				of << "\t\t\temissive " << val4.x << " " << val4.y << " " << val4.z << " " << val4.w << " " << std::endl;
			}
		}

		int numTexMaps = mtl->GetNumberOfTextureMaps();
		if (numTexMaps > 0) {

			for (int texMapIdx = 0; texMapIdx < numTexMaps; texMapIdx++) {
				IGameTextureMap* tmap = mtl->GetIGameTextureMap(texMapIdx);
				if (tmap) {
					of << "\n\t\t\ttexture_unit " << std::endl;
					of << "\t\t\t{" << std::endl;

					std::string bmap(tmap->GetBitmapFileName());
					bmap = bmap.substr(bmap.find_last_of('\\') + 1);
					of << "\t\t\t\ttexture " << bmap << std::endl;
					of << "\t\t\t}" << std::endl;
				}
			}
		}

		of << "\t\t}" << std::endl;

		return true;
	}
	
	bool MaterialExporter::buildMaterial(IGameMaterial *material, const std::string& matName, std::string &script) {

		std::stringstream of;

		of << "material " << matName << std::endl;
		of << std::showpoint;
		of << "{" << std::endl;

		of << "\ttechnique" << std::endl;
		of << "\t{" << std::endl;

		int numSubMtl = 0;
		
		if (material != NULL) {
			numSubMtl = material->GetSubMaterialCount();

			if (numSubMtl > 0) {
				int i;
				for (i=0; i<numSubMtl; i++) {
					streamPass(of, material->GetSubMaterial(i));
				}
			}
			else
				streamPass(of, material);
		}
		else {
			streamPass(of, material);
		}

		of << "\t}" << std::endl;
		of << "}" << std::endl;

		script = of.str();

		return true;
	}

	bool MaterialExporter::streamMaterial(std::ostream &of) {

		// serialize this information to the material file
		MaterialMap::iterator it = m_materialMap.begin();

		while (it != m_materialMap.end()) {
			std::string matName(it->first);
			IGameMaterial *mtl = it->second;

			of << "material " << matName << std::endl;
			of << std::showpoint;
			of << "{" << std::endl;

			of << "\ttechnique" << std::endl;
			of << "\t{" << std::endl;

			int numSubMtl = 0;
			
			if (mtl != NULL) {
				numSubMtl = mtl->GetSubMaterialCount();

				if (numSubMtl > 0) {
					int i;
					for (i=0; i<numSubMtl; i++) {
						streamPass(of, mtl->GetSubMaterial(i));
					}
				}
				else
					streamPass(of, mtl);
			}
			else {
				streamPass(of, mtl);
			}

			of << "\t}" << std::endl;
			of << "}" << std::endl;

			it++;
		}

		m_materialMap.clear();

		return true;
	}

}