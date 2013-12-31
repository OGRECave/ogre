/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/


#ifndef _WATER_MESH_H_
#define _WATER_MESH_H_

#include "OgrePlatform.h"
#include "Ogre.h"

using namespace Ogre ;

class WaterMesh
{
private:
	MeshPtr mesh ;
	SubMesh *subMesh ; 
	float *vertexBuffers[3] ; // we need 3 vertex buffers
	int currentBuffNumber ;
	int complexity ;
	String meshName ;
	int numFaces ;
	int numVertices ;
	Vector3* vNormals ;

	HardwareVertexBufferSharedPtr posVertexBuffer ;
	HardwareVertexBufferSharedPtr normVertexBuffer ;
	HardwareVertexBufferSharedPtr texcoordsVertexBuffer ;
	HardwareIndexBufferSharedPtr indexBuffer ;

	Real lastTimeStamp ;
	Real lastAnimationTimeStamp;
	Real lastFrameTime ;
	
	void calculateFakeNormals();
	void calculateNormals();
public:
	WaterMesh(const String& meshName, Real planeSize, int complexity) ;
    
    virtual ~WaterMesh ();


	/** "pushes" a mesh at position [x,y]. Note, that x,y are float, hence 
	*	4 vertices are actually pushed
	*	@note 
	*		This should be replaced by push with 'radius' parameter to simulate
	*  		big objects falling into water
	*/
	void push(Real x, Real y, Real depth, bool absolute=false) ;

	/** gets height at given x and y, takes average value of the closes nodes */
	Real getHeight(Real x, Real y);

	/** updates mesh */
	void updateMesh(Real timeSinceLastFrame) ;
	
	Real PARAM_C ; // ripple speed 
	Real PARAM_D ; // distance
	Real PARAM_U ; // viscosity
	Real PARAM_T ; // time
	bool useFakeNormals ;
} ;

#endif
