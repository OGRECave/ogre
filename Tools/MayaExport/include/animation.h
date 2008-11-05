////////////////////////////////////////////////////////////////////////////////
// animation.h
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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "mayaExportLayer.h"

namespace OgreMayaExporter
{
	// Track type
	typedef enum { TT_SKELETON, TT_MORPH, TT_POSE } trackType;

	// Target
	typedef enum { T_MESH, T_SUBMESH } target;

	// Vertex position
	typedef struct vertexPositiontag
	{
		float x,y,z;
	} vertexPosition;

	// Vertex pose reference
	typedef struct vertexPoseReftag
	{
		int poseIndex;
		float poseWeight;
	} vertexPoseRef;

	// Vertex animation keyframe
	typedef struct vertexKeyframetag
	{
		float time;
		std::vector<vertexPosition> positions;
		std::vector<vertexPoseRef> poserefs;
	} vertexKeyframe;

	// Skeleton animation keyframe
	typedef struct skeletonKeyframeTag
	{
		float time;								//time of keyframe
		double tx,ty,tz;						//translation
		double angle,axis_x,axis_y,axis_z;		//rotation
		float sx,sy,sz;							//scale
	} skeletonKeyframe;

	// Blend shape data
	typedef struct vertexOffestTag
	{
		long index;
		float x,y,z;
	} vertexOffset;

	typedef struct poseTag
	{
		target poseTarget;
		long index;
		int blendShapeIndex;
		MString name;
		std::vector<vertexOffset> offsets;
	} pose;

	// A class for storing an animation track
	// each track can be either skeleton, morph or pose animation
	class Track
	{
	public:
		//constructor
		Track() {
			clear();
		};
		//destructor
		~Track() {
			clear();
		}
		//clear track data
		void clear() {
			m_type = TT_SKELETON;
			m_target = T_MESH;
			m_index = 0;
			m_bone = "";
			m_vertexKeyframes.clear();
			m_skeletonKeyframes.clear();
		}
		//add vertex animation keyframe
		void addVertexKeyframe(vertexKeyframe& k) {
			m_vertexKeyframes.push_back(k);
		}
		//add skeleton animation keyframe
		void addSkeletonKeyframe(skeletonKeyframe& k) {
			m_skeletonKeyframes.push_back(k);
		}

		//public members
		trackType m_type;
		target m_target;
		int m_index;
		MString m_bone;
		std::vector<vertexKeyframe> m_vertexKeyframes;
		std::vector<skeletonKeyframe> m_skeletonKeyframes;
	};
	

	// A class for storing animation information
	// an animation is a collection of different tracks
	class Animation
	{
	public:
		//constructor
		Animation() {
			clear();
		}
		//destructor
		~Animation() { 
			clear(); 
		};
		//clear animation data
		void clear() {
			m_name = "";
			m_length = 0;
			m_tracks.clear();
		};
		//add track
		void addTrack(Track& t) {
			m_tracks.push_back(t);
		}

		//public memebers
		MString m_name;
		float m_length;
		std::vector<Track> m_tracks;
	};

} // end namespace


#endif // ANIMATION_H