/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        CubeMapping.h
    \brief
        Specialisation of OGRE's framework application to show the
        cube mapping feature where a wrap-around environment is reflected
        off of an object.
		Extended with Perlin noise to show we can.
*/

#include "ExampleApplication.h"

#define ENTITY_NAME "CubeMappedEntity"
#define MESH_NAME "CubeMappedMesh"

#define MATERIAL_NAME "Examples/SceneCubeMap2"
#define SKYBOX_MATERIAL "Examples/SceneSkyBox2"

/* ==================================================================== */
/*    Perlin Noise data and algorithms - copied from Perlin himself :)  */
/* ==================================================================== */
#define lerp(t,a,b) ( (a)+(t)*((b)-(a)) )
#define fade(t) ( (t)*(t)*(t)*(t)*((t)*((t)*6-15)+10) )
double grad(int hash, double x, double y, double z) {
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	double u = h<8||h==12||h==13 ? x : y,   // INTO 12 GRADIENT DIRECTIONS.
		v = h<4||h==12||h==13 ? y : z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
int p[512]={
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

double noise3(double x, double y, double z) {
	int X = ((int)floor(x)) & 255,                  // FIND UNIT CUBE THAT
		Y = ((int)floor(y)) & 255,                  // CONTAINS POINT.
		Z = ((int)floor(z)) & 255;
	x -= floor(x);                                // FIND RELATIVE X,Y,Z
	y -= floor(y);                                // OF POINT IN CUBE.
	z -= floor(z);
	double u = fade(x),                                // COMPUTE FADE CURVES
		v = fade(y),                                // FOR EACH OF X,Y,Z.
		w = fade(z);
	int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
		B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

	return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
						grad(p[BA  ], x-1, y  , z   )), // BLENDED
					lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
						grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
				lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
						grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
					lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
						grad(p[BB+1], x-1, y-1, z-1 ))));
}

/* ==================================================================== */
/*                                 Main part                            */
/* ==================================================================== */

class CubeMapListener : public ExampleFrameListener
{
private:
	// main variables
	Real tm ;
	Real timeoutDelay ;
	SceneManager *mSceneMgr ;
	SceneNode *objectNode ;

	// mesh-specific data
	MeshPtr originalMesh ;
	MeshPtr clonedMesh ;

	Entity *objectEntity ;
	vector<MaterialPtr>::type clonedMaterials ;

	// configuration
	Real displacement ;
	Real density ;
	Real timeDensity ;
	bool noiseOn ;
	size_t currentMeshIndex ;
	StringVector availableMeshes ;
	size_t currentLBXindex ;
	LayerBlendOperationEx currentLBX ;
	size_t currentCubeMapIndex ;
	StringVector availableCubeMaps ;
	MaterialPtr material ;
	
	void _updatePositionNoise(int numVertices, float *dstVertices,
		float *defaultVertices)
	{
		for(int i=0;i<3*numVertices;i+=3) {
			double n = 1 + displacement * noise3(
				defaultVertices[i]/density + tm,
				defaultVertices[i+1]/density + tm,
				defaultVertices[i+2]/density + tm);
			dstVertices[i+0] = defaultVertices[i] * n ;
			dstVertices[i+1] = defaultVertices[i+1] * n ;
			dstVertices[i+2] = defaultVertices[i+2] * n ;
		}
	}
	
	float* _normalsGetCleared(VertexData *vertexData)
	{
		const VertexElement *normVE = vertexData->
			vertexDeclaration->findElementBySemantic(VES_NORMAL);
		HardwareVertexBufferSharedPtr normHVB = vertexData->
			vertexBufferBinding->getBuffer(normVE->getSource());
		float* normals = (float*) normHVB->lock(0, normHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_DISCARD);
		memset(normals, 0, normHVB->getSizeInBytes());
		return normals;
	}
	
	void _normalsSaveNormalized(VertexData *vertexData, float *normals)
	{
		const VertexElement *normVE = vertexData->
			vertexDeclaration->findElementBySemantic(VES_NORMAL);
		HardwareVertexBufferSharedPtr normHVB = vertexData->
			vertexBufferBinding->getBuffer(normVE->getSource());
		size_t numVertices = normHVB->getNumVertices();
		for(size_t i=0;i<numVertices;i++, normals+=3) {
			Vector3 n(normals[0], normals[1], normals[2]);
			n.normalise();
			normals[0] = n.x ;
			normals[1] = n.y ;
			normals[2] = n.z ;
		}
		normHVB->unlock();
	}
	
	void _updateVertexDataNoiseAndNormals(
			VertexData *dstData, 
			VertexData *orgData,
			IndexData *indexData,
			float *normals)
	{
		size_t i ;
		
		// Find destination vertex buffer
		const VertexElement *dstVEPos = dstData->
			vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr dstHVBPos = dstData->
			vertexBufferBinding->getBuffer(dstVEPos->getSource());
		// Find source vertex buffer 
		const VertexElement *orgVEPos = orgData->
			vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr orgHVBPos = orgData->
			vertexBufferBinding->getBuffer(orgVEPos->getSource());
		// Lock these buffers
		float *dstDataPos = (float*) dstHVBPos->lock(0, dstHVBPos->getSizeInBytes(),
			HardwareBuffer::HBL_DISCARD);
		float *orgDataPos = (float*) orgHVBPos->lock(0, orgHVBPos->getSizeInBytes(),
			HardwareBuffer::HBL_READ_ONLY);
		// make noise
		size_t numVertices = orgHVBPos->getNumVertices();
		for(i=0;i<3*numVertices;i+=3) {
			double n = 1 + displacement * noise3(
				orgDataPos[i]/density + tm,
				orgDataPos[i+1]/density + tm,
				orgDataPos[i+2]/density + tm);
			dstDataPos[i+0] = orgDataPos[i] * n ;
			dstDataPos[i+1] = orgDataPos[i+1] * n ;
			dstDataPos[i+2] = orgDataPos[i+2] * n ;
		}
		// Unlock original position buffer
		orgHVBPos->unlock();

		// calculate normals
		HardwareIndexBufferSharedPtr indexHB = indexData->indexBuffer ;
		unsigned short * vertexIndices = (unsigned short*) indexHB->lock(
			0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
		size_t numFaces = indexData->indexCount / 3 ;
		for(i=0 ; i<numFaces ; i++, vertexIndices+=3) {
			//~ int p0 = 0;
			//~ int p1 = 1;
			//~ int p2 = 2;
			int p0 = vertexIndices[0] ;
			int p1 = vertexIndices[1] ;
			int p2 = vertexIndices[2] ;

			//~ Vector3 v0(10,0,20);
			//~ Vector3 v1(30,0,20);
			//~ Vector3 v2(20,-1,50);
			Vector3 v0(dstDataPos[3*p0], dstDataPos[3*p0+1], dstDataPos[3*p0+2]);
			Vector3 v1(dstDataPos[3*p1], dstDataPos[3*p1+1], dstDataPos[3*p1+2]);
			Vector3 v2(dstDataPos[3*p2], dstDataPos[3*p2+1], dstDataPos[3*p2+2]);

			Vector3 diff1 = v1 - v2 ;
			Vector3 diff2 = v1 - v0 ;
			Vector3 fn = diff1.crossProduct(diff2);
#define _ADD_VECTOR_TO_REALS(ptr,vec) { *(ptr)+=vec.x; *((ptr)+1)+=vec.y; *((ptr)+2)+=vec.z; }
			_ADD_VECTOR_TO_REALS(normals+3*p0, fn);
			_ADD_VECTOR_TO_REALS(normals+3*p1, fn);
			_ADD_VECTOR_TO_REALS(normals+3*p2, fn);
#undef _ADD_VECTOR_TO_REALS
		}
		indexHB->unlock();

		// Unlock destination position buffer
		dstHVBPos->unlock();
	}

	void updateNoise()
	{
		float *sharedNormals = 0 ;
		for(int m=0;m<clonedMesh->getNumSubMeshes();m++) { // for each subMesh
			SubMesh *subMesh = clonedMesh->getSubMesh(m);
			SubMesh *orgSubMesh = originalMesh->getSubMesh(m);
			if (subMesh->useSharedVertices) {
				if (!sharedNormals) { // first of shared
					sharedNormals = _normalsGetCleared(clonedMesh->sharedVertexData);
				}
				_updateVertexDataNoiseAndNormals(
					clonedMesh->sharedVertexData, 
					originalMesh->sharedVertexData,
					subMesh->indexData,
					sharedNormals);
			} else {
				float* normals = _normalsGetCleared(subMesh->vertexData);
				_updateVertexDataNoiseAndNormals(
					subMesh->vertexData, 
					orgSubMesh->vertexData,
					subMesh->indexData,
					normals);
				_normalsSaveNormalized(subMesh->vertexData, normals);
			}
		}
		if (sharedNormals) {
			_normalsSaveNormalized(clonedMesh->sharedVertexData, sharedNormals);
		}
	}

	void clearEntity()
	{
		// delete cloned materials 
		for(unsigned int m=0;m<clonedMaterials.size();m++) {
			MaterialManager::getSingleton().remove(clonedMaterials[m]->getHandle()) ;
		}
		clonedMaterials.clear();
		
		// detach and destroy entity
		objectNode->detachAllObjects();
		mSceneMgr->destroyEntity(ENTITY_NAME);
		
		// destroy mesh as well, to reset its geometry
		MeshManager::getSingleton().remove(clonedMesh->getHandle());
		
		objectEntity = 0 ;
	}
	
	VertexData* _prepareVertexData(VertexData *orgVD)
	{
		if (!orgVD)
			return 0 ;

        // Hacky bit: reorganise vertex buffers to a buffer-per-element
        // Since this demo was written a while back to assume that
        // Really this demo should be replaced with a vertex program noise
        // distortion, but left the software for now since it's nice for older
        // card owners
        VertexDeclaration* newDecl = orgVD->vertexDeclaration->clone();
        const VertexDeclaration::VertexElementList& elems = newDecl->getElements();
        VertexDeclaration::VertexElementList::const_iterator di;
        unsigned short buf = 0;
        for (di = elems.begin(); di != elems.end(); ++di)
        {
            newDecl->modifyElement(buf, buf, 0, di->getType(), di->getSemantic(), di->getIndex()); 
            buf++;
        }
        orgVD->reorganiseBuffers(newDecl);


		VertexData* newVD = new VertexData();
		// copy things that do not change
		newVD->vertexCount = orgVD->vertexCount ;
		newVD->vertexStart = orgVD->vertexStart ;
		// now copy vertex buffers, looking in the declarations
		VertexDeclaration* newVDecl = newVD->vertexDeclaration ;
		VertexBufferBinding* newVBind = newVD->vertexBufferBinding ;
		// note: I assume various semantics are not shared among buffers
		const VertexDeclaration::VertexElementList& orgVEL = orgVD->vertexDeclaration->getElements() ;
		VertexDeclaration::VertexElementList::const_iterator veli, velend;
		velend = orgVEL.end();
		// For each declaration, prepare buffer
		for( veli = orgVEL.begin() ; veli != velend ; ++veli)
		{
			VertexElementSemantic ves = (*veli).getSemantic();
			int source = (*veli).getSource() ;
			HardwareVertexBufferSharedPtr orgBuf = orgVD->vertexBufferBinding->
				getBuffer( source );
			// check usage for the new buffer
			bool dynamic = false ;
			switch(ves) {
				case VES_NORMAL :
				case VES_POSITION :
					dynamic = true ;
					break ;
				case VES_BLEND_INDICES : 
				case VES_BLEND_WEIGHTS :
				case VES_DIFFUSE : 
				case VES_SPECULAR :
				case VES_TEXTURE_COORDINATES :
				default :
					dynamic = false ;
					break ;
			}
			if (dynamic) { // create a new dynamic buffer with write access
				HardwareVertexBufferSharedPtr newBuf = 
					HardwareBufferManager::getSingleton().createVertexBuffer(
						orgBuf->getVertexSize(), orgBuf->getNumVertices(),
						HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, 
						//~ HardwareBuffer::HBU_DYNAMIC, 
						true
						//~ false
						);
				newBuf->copyData(*orgBuf, 0, 0, orgBuf->getSizeInBytes(), true);
				newVBind->setBinding( source, newBuf );
			} else { // use the old one
				newVBind->setBinding( source, orgBuf );
			}
			// add element for declaration
			newVDecl->addElement(source, (*veli).getOffset(), (*veli).getType(),
				ves, (*veli).getIndex());
		}
		return newVD;
	}
	
	void prepareClonedMesh()
	{
		// we create new Mesh based on the original one, but changing
		// HBU flags (inside _prepareVertexData)
		clonedMesh = MeshManager::getSingleton().createManual(MESH_NAME, 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		clonedMesh->_setBounds(originalMesh->getBounds()); 
		clonedMesh->_setBoundingSphereRadius(originalMesh->getBoundingSphereRadius());
		//~ if (originalMesh->sharedVertexData)
			//~ clonedMesh->sharedVertexData = originalMesh->sharedVertexData->clone();
		clonedMesh->sharedVertexData = 
			_prepareVertexData(originalMesh->sharedVertexData);
		for(int sm=0;sm<originalMesh->getNumSubMeshes();sm++) {
			SubMesh *orgSM = originalMesh->getSubMesh(sm);
			SubMesh *newSM = clonedMesh->createSubMesh();
			if (orgSM->isMatInitialised()) {
				newSM->setMaterialName(orgSM->getMaterialName());
			}
			newSM->useSharedVertices = orgSM->useSharedVertices ;
			// prepare vertex data
			newSM->vertexData = _prepareVertexData(orgSM->vertexData);
			// reuse index data
			newSM->indexData->indexBuffer = orgSM->indexData->indexBuffer ;
			newSM->indexData->indexStart = orgSM->indexData->indexStart ;
			newSM->indexData->indexCount = orgSM->indexData->indexCount ;
		}
	}

	void prepareEntity(const String& meshName) 
	{
		if (objectEntity) {
			clearEntity();
		}
		
		// load mesh if necessary - note, I assume this is the only point
		// Mesh can get loaded, since I want to make sure about its HBU etc.
		originalMesh = MeshManager::getSingleton().getByName(meshName);
		if (originalMesh.isNull()) {
			originalMesh = MeshManager::getSingleton().load(meshName,
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				true, true); //so we can still read it
			if (originalMesh.isNull()) {
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
					"Can't find a mesh: '"+meshName+"'",
					"CubeMapListener::prepareEntity");
			}
		}
		
		
		prepareClonedMesh();

        // create an entity based on cloned mesh
		objectEntity = mSceneMgr->createEntity( ENTITY_NAME, MESH_NAME);
        objectEntity->setMaterialName( material->getName() );
        Pass* pass = material->getTechnique(0)->getPass(0);
		
		// go through subentities and set materials as required
		for(int m=0;m<clonedMesh->getNumSubMeshes();m++) {
			SubMesh *subMesh = clonedMesh->getSubMesh(m);
			SubEntity *subEntity = objectEntity->getSubEntity(m);
			// check if this submesh has material set
			if (subMesh->isMatInitialised()) {
				const String& matName = subMesh->getMaterialName();
				MaterialPtr subMat = 
                    MaterialManager::getSingleton().getByName(matName);
				if (!subMat.isNull()) { // clone material, add layers from global material
					subMat->load();
					MaterialPtr cloned = subMat->clone(
						"CubeMapTempMaterial#"+StringConverter::toString(m));
                    Pass* clonedPass = cloned->getTechnique(0)->getPass(0);
					// can't help it - have to do it :)
					if (meshName=="knot.mesh") {
						for(size_t tl=0;tl<clonedPass->getNumTextureUnitStates();tl++) {
							TextureUnitState *tlayer = clonedPass->getTextureUnitState(tl);
							tlayer->setScrollAnimation(1.0 , 0);
						}
					}
					// add layers
					for(size_t tl=0;tl<pass->getNumTextureUnitStates();tl++) {
						TextureUnitState *orgTL = pass->getTextureUnitState(tl);
						TextureUnitState *newTL = clonedPass->createTextureUnitState(
							orgTL->getTextureName());
						*newTL = *orgTL ;
						newTL->setColourOperationEx(currentLBX);
					}
					subEntity->setMaterialName(cloned->getName());
					clonedMaterials.push_back(cloned);
				} 
			} 
		}

		objectNode->attachObject(objectEntity);
		
		// update noise to avoid one frame w/o noise
		if (noiseOn)
			updateNoise();
	}
	
	void updateInfoDisplacement()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/Displacement")
			->setCaption("[1/2] Displacement: "+StringConverter::toString(displacement));		
	}
	void updateInfoDensity()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/Density")
			->setCaption("[3/4] Noise density: "+StringConverter::toString(density));		
	}
	void updateInfoTimeDensity()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/TimeDensity")
			->setCaption("[5/6] Time density: "+StringConverter::toString(timeDensity));
	}
	void setObject()
	{
		currentMeshIndex %= availableMeshes.size();
		const String& meshName = availableMeshes[currentMeshIndex];
		printf("Switching to object: %s\n", meshName.c_str());
		prepareEntity(meshName);
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/Object")
			->setCaption("[O] Object: "+meshName);
	}
	void setNoiseOn()
	{
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/Noise")
			->setCaption(String("[N] Noise: ")+ ((noiseOn)?"on":"off") );		
	}
	void setMaterialBlending()
	{
		currentLBXindex %= 5;
		String lbxName ;
#define _LAZYERU_(a,b,c) case a : currentLBX = b ; lbxName = c ; break ;
		switch (currentLBXindex) {
			_LAZYERU_(0, LBX_ADD, "ADD")
			_LAZYERU_(1, LBX_MODULATE, "MODULATE")
			_LAZYERU_(2, LBX_MODULATE_X2, "MODULATE X2")
			_LAZYERU_(3, LBX_MODULATE_X4, "MODULATE X4")
			_LAZYERU_(4, LBX_SOURCE1, "SOURCE1")
			_LAZYERU_(5, LBX_SOURCE2, "SOURCE2")
			// more?
		}
#undef _LAZYERU_		
		// reset entities, materials and so on
		prepareEntity(availableMeshes[currentMeshIndex]);
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/Material")
			->setCaption("[M] Material blend:"+lbxName);
	}
	void setCubeMap()
	{
		currentCubeMapIndex %= availableCubeMaps.size();
		unsigned int i ;
		String cubeMapName = availableCubeMaps[currentCubeMapIndex];
		Pass *pass = material->getTechnique(0)->getPass(0);
		for(i=0;i<(unsigned int)pass->getTextureUnitState(0)->getNumFrames();i++) {
			String oldTexName = pass->getTextureUnitState(0)->
				getFrameTextureName(i);
			TexturePtr oldTex = TextureManager::getSingleton().getByName(oldTexName);
			TextureManager::getSingleton().remove(oldTexName);
		}
		pass->getTextureUnitState(0)->setCubicTextureName(cubeMapName, true);
		
		MaterialPtr mat2 = 
			MaterialManager::getSingleton().getByName(SKYBOX_MATERIAL);
        Pass* pass2 = mat2->getTechnique(0)->getPass(0);
		for(i=0;i<(unsigned int)pass2->getTextureUnitState(0)->getNumFrames();i++) {
			String oldTexName = pass2->getTextureUnitState(0)->
				getFrameTextureName(i);
			TexturePtr oldTex = TextureManager::getSingleton().getByName(oldTexName);
			TextureManager::getSingleton().remove(oldTexName);
		}
		pass2->getTextureUnitState(0)->setCubicTextureName(cubeMapName, false);

		mSceneMgr->setSkyBox(true, SKYBOX_MATERIAL );

		prepareEntity(availableMeshes[currentMeshIndex]);
		OverlayManager::getSingleton().getOverlayElement("Example/CubeMapping/CubeMap")
			->setCaption("[C] CubeMap:"+cubeMapName);
	}
	
#define RANDOM_FROM(a,b) (((float)(rand() & 65535)) / 65536.0f * ((b)-(a)) + (a))
	void goRandom()
	{
		displacement = RANDOM_FROM(0.0f, 1.0f);
		updateInfoDisplacement();

		density = RANDOM_FROM(1.0f, 300.0f);
		updateInfoDensity();
		
		timeDensity = RANDOM_FROM(1.0f, 10.0f);
		updateInfoTimeDensity();
	}
	
#define MEDIA_FILENAME "media.cfg"
	void readConfig()
	{
        String media_filename(MEDIA_FILENAME);
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        media_filename = macBundlePath() + "/Contents/Resources/" + media_filename;
#endif
		ConfigFile cfg;
		cfg.load( media_filename );
		availableMeshes = cfg.getMultiSetting("Mesh");
		availableCubeMaps = cfg.getMultiSetting("CubeMap");
	}
	
public:
    CubeMapListener(RenderWindow* win, Camera* cam,
			SceneManager *sceneMgr, SceneNode *objectNode)
        : ExampleFrameListener(win, cam)
    {
		this->mSceneMgr = sceneMgr ;
		this->objectNode = objectNode ;

		tm = 0 ;
		timeoutDelay = 0 ;
		displacement = 0.1f;
		density = 50.0f;
		timeDensity = 5.0f;
		objectEntity = 0 ;
		
		material = MaterialManager::getSingleton().getByName(MATERIAL_NAME);

		if (material.isNull()) {
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
				"Can't find material: "+String(MATERIAL_NAME),
				"CubeMapListener::CubeMapListener");
		}
		
		readConfig();

		currentMeshIndex = 0 ;
		setObject();
		
		currentLBXindex = 0 ;
		setMaterialBlending();
		
		currentCubeMapIndex = 0 ;
		setCubeMap();
		
		noiseOn = true ;
		setNoiseOn();

		updateInfoDisplacement();
		updateInfoDensity();
		updateInfoTimeDensity();
    }
    virtual bool frameRenderingQueued(const FrameEvent& evt)
    {
		// Call default
		if( ExampleFrameListener::frameRenderingQueued(evt) == false )
			return false;

		tm += evt.timeSinceLastFrame / timeDensity ;

		if (noiseOn)
			updateNoise();

		objectNode->yaw(Degree(20*evt.timeSinceLastFrame));

        return true;
    }
	virtual bool processUnbufferedKeyInput(const FrameEvent& evt)
    {

		bool retval = ExampleFrameListener::processUnbufferedKeyInput(evt);

		Real changeSpeed = evt.timeSinceLastFrame ;
		
		// adjust keyboard speed with SHIFT (increase) and CONTROL (decrease)
		if (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)) {
			changeSpeed *= 10.0f ;
		}
		if (mKeyboard->isKeyDown(OIS::KC_LCONTROL)) { 
			changeSpeed /= 10.0f ;
		}
		
#define ADJUST_RANGE(_value,_keyPlus,_keyMinus,_minVal,_maxVal,_change,_macro) {\
	if (mKeyboard->isKeyDown(_keyPlus)) \
		{ _value+=_change ; if (_value>=_maxVal) _value = _maxVal ; _macro ; } ; \
	if (mKeyboard->isKeyDown(_keyMinus)) \
		{ _value-=_change; if (_value<=_minVal) _value = _minVal ; _macro ; } ; \
}
		
		ADJUST_RANGE(displacement, OIS::KC_2, OIS::KC_1, -2, 2, 0.1f*changeSpeed, updateInfoDisplacement()) ;

		ADJUST_RANGE(density, OIS::KC_4, OIS::KC_3, 0.1, 500, 10.0f*changeSpeed, updateInfoDensity()) ;

		ADJUST_RANGE(timeDensity, OIS::KC_6, OIS::KC_5, 1, 10, 1.0f*changeSpeed, updateInfoTimeDensity()) ;

#define SWITCH_VALUE(_key,_timeDelay, _macro) { \
		if (mKeyboard->isKeyDown(_key) && timeoutDelay==0) { \
			timeoutDelay = _timeDelay ; _macro ;} }
	
		timeoutDelay-=evt.timeSinceLastFrame ;
		if (timeoutDelay<=0)
			timeoutDelay = 0;

		SWITCH_VALUE(OIS::KC_O, 0.5f, currentMeshIndex++ ; setObject());

		SWITCH_VALUE(OIS::KC_N, 0.5f, noiseOn = !noiseOn ; setNoiseOn());

		SWITCH_VALUE(OIS::KC_M, 0.5f, currentLBXindex++ ; setMaterialBlending());

		SWITCH_VALUE(OIS::KC_C, 0.5f, currentCubeMapIndex++ ; setCubeMap());
		
		SWITCH_VALUE(OIS::KC_SPACE, 0.5f, goRandom());

		return retval ;
	}
} ;

class CubeMapApplication : public ExampleApplication
{
public:
    CubeMapApplication() {}

protected:
	SceneNode *objectNode;

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // First check that cube mapping is supported
        if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_CUBEMAPPING))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your card does not support cube mapping, so cannot "
                "run this demo. Sorry!", 
                "CubeMapApplication::createScene");
        }

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skybox
        mSceneMgr->setSkyBox(true, SKYBOX_MATERIAL );

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        objectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// show overlay
		Overlay* overlay = OverlayManager::getSingleton().getByName("Example/CubeMappingOverlay");    
		overlay->show();
	}

    void createFrameListener(void)
    {
        mFrameListener= new CubeMapListener(mWindow, mCamera, mSceneMgr, objectNode);
        mRoot->addFrameListener(mFrameListener);
    }
};
