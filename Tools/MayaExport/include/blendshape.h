////////////////////////////////////////////////////////////////////////////////
// blendshape.h
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

#ifndef _BLENDSHAPE_H
#define _BLENDSHAPE_H

#include "mayaExportLayer.h"
#include "paramList.h"
#include "animation.h"
#include "vertex.h"

namespace OgreMayaExporter
{
	typedef struct
	{
		MPlugArray srcConnections;
		MPlugArray dstConnections;
	} weightConnections;

	typedef struct
	{
		int targetIndex;
		std::vector<pose> poses;
	} poseGroup;

	// Blend Shape Class
	class BlendShape
	{
	public:
		// Constructor
		BlendShape();
		// Destructor
		~BlendShape();
		// Clear blend shape data
		void clear();
		// Load blend shape deformer from Maya
		MStatus load(MObject &blendShapeObj);
		// Load blend shape poses for shared geometry
		MStatus loadPosesShared(MDagPath& meshDag,ParamList &params,std::vector<vertex> &vertices,
			long numVertices,long offset=0);
		// Load blend shape poses for a submesh
		MStatus loadPosesSubmesh(MDagPath& meshDag,ParamList &params,std::vector<vertex> &vertices,
			std::vector<long>& indices,long targetIndex=0);
		//load a blend shape animation track
		Track loadTrack(float start,float stop,float rate,ParamList& params,int targetIndex,int startPoseId);
		// Get blend shape deformer name
		MString getName();
		// Get blend shape poses
		stdext::hash_map<int, poseGroup>& getPoseGroups();
		// Set maya blend shape deformer envelope
		void setEnvelope(float envelope);
		// Restore maya blend shape deformer original envelope
		void restoreEnvelope();
		// Break connections to this blendshape
		void breakConnections();
		// Restore connections on this blendshape
		void restoreConnections();
		// Public members
		MFnBlendShapeDeformer* m_pBlendShapeFn;

	protected:
		// Internal methods
		//load a blend shape pose for shared geometry
		MStatus loadPoseShared(MDagPath& meshDag,ParamList& params,std::vector<vertex>& vertices,
			long numVertices,long offset,MString poseName, int blendShapeIndex);
		//load a blend shape pose for a submesh
		MStatus loadPoseSubmesh(MDagPath& meshDag,ParamList& params,std::vector<vertex>& vertices,
			std::vector<long>& indices,MString poseName,int targetIndex, int blendShapeIndex);
		//load a blend shape animation keyframe
		vertexKeyframe loadKeyframe(float time,ParamList& params,int targetIndex,int startPoseId);

		// Protected members
		//original values to restore after export
		float m_origEnvelope;
		std::vector<float> m_origWeights;
		//blend shape poses
		stdext::hash_map<int, poseGroup> m_poseGroups;
		//blend shape target (shared geometry or submesh)
		target m_target;
		//blend shape weights connections
		std::vector<weightConnections> m_weightConnections;
	};


}	// end namespace

#endif