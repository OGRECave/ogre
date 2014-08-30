////////////////////////////////////////////////////////////////////////////////
// mayaExportLayer.h
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

#ifndef _MAYAEXPORTLAYER_H
#define _MAYAEXPORTLAYER_H

#define PRECISION 0.0001

// standard libraries
#include <math.h>
#include <vector>
#include <set>
#include <cassert>
#include <fstream>

#ifdef MAC_PLUGIN
    #include <ext/hash_map>
#else
    #include <hash_map>
#endif

// Maya API
#include <maya/MAngle.h>
#include <maya/MFnTransform.h>
#include <maya/MItDag.h>
#include <maya/MFnCamera.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnIkJoint.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MMatrix.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MPointArray.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>
#include <maya/MAnimUtil.h>
#include <maya/MRenderUtil.h>
#include <maya/MQuaternion.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MBoundingBox.h>
#include <maya/MDagModifier.h>

// OGRE API
#include "Ogre.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"

#pragma warning (disable : 4018)

#endif