////////////////////////////////////////////////////////////////////////////////
// paramlist.cpp
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

#include "paramlist.h"
#include <maya/MGlobal.h>

/***** Class ParamList *****/
// method to parse arguments from command line
namespace OgreMayaExporter
{
	void ParamList::parseArgs(const MArgList &args)
	{
		MStatus stat;
		// Parse arguments from command line
		for (uint i=0; i < args.length(); i++ )
		{
			if ((MString("-all") == args.asString(i,&stat)) && (MS::kSuccess == stat))
				exportAll = true;
			else if ((MString("-world") == args.asString(i,&stat)) && (MS::kSuccess == stat))
				exportWorldCoords = true;
			else if ((MString("-lu") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				MString lengthUnit = args.asString(++i,&stat);
				if (MString("pref") == lengthUnit)
				{
					MGlobal::executeCommand("currentUnit -q -l",lengthUnit,false);
				}
				if (MString("mm") == lengthUnit)
					lum = CM2MM;
				else if (MString("cm") == lengthUnit)
					lum = CM2CM;
				else if (MString("m") == lengthUnit)
					lum = CM2M;
				else if (MString("in") == lengthUnit)
					lum = CM2IN;
				else if (MString("ft") == lengthUnit)
					lum = CM2FT;
				else if (MString("yd") == lengthUnit)
					lum = CM2YD;
			}
			else if ((MString("-scale") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				float s = args.asDouble(++i,&stat);
				lum *= s;
			}
			else if ((MString("-mesh") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportMesh = true;
				meshFilename = args.asString(++i,&stat);
			}
			else if ((MString("-mat") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportMaterial = true;
				materialFilename = args.asString(++i,&stat);
			}
			else if ((MString("-matPrefix") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				matPrefix = args.asString(++i,&stat);
			}
			else if ((MString("-copyTex") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				copyTextures = true;
				texOutputDir = args.asString(++i,&stat);
			}
			else if ((MString("-lightOff") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				lightingOff = true;
			}
			else if ((MString("-skel") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportSkeleton = true;
				skeletonFilename = args.asString(++i,&stat);
			}
			else if ((MString("-skeletonAnims") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportSkelAnims = true;
			}
			else if ((MString("-vertexAnims") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertAnims = true;
			}
			else if ((MString("-blendShapes") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportBlendShapes = true;
			}
			else if ((MString("-BSAnims") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportBSAnims = true;
			}
			else if ((MString("-skelBB") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				skelBB = true;
			}
			else if ((MString("-bsBB") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				bsBB = true;
			}
			else if ((MString("-vertBB") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				vertBB = true;
			}
			else if ((MString("-animCur") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportAnimCurves = true;
				animFilename = args.asString(++i,&stat);
			}
			else if ((MString("-cam") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportCameras = true;
				camerasFilename = args.asString(++i,&stat);
			}
			else if ((MString("-v") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVBA = true;
			}
			else if ((MString("-n") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertNorm = true;
			}
			else if ((MString("-c") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertCol = true;
			}
			else if ((MString("-t") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportTexCoord = true;
			}
			else if ((MString("-edges") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				buildEdges = true;
			}
			else if ((MString("-tangents") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				buildTangents = true;
				MString tanSem = args.asString(++i,&stat);
				if (tanSem == "TEXCOORD")
					tangentSemantic = TS_TEXCOORD;
				else if (tanSem == "TANGENT")
					tangentSemantic = TS_TANGENT;
			}
			else if ((MString("-tangentsplitmirrored") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				tangentsSplitMirrored = true;
			}
			else if ((MString("-tangentsplitrotated") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				tangentsSplitRotated = true;
			}
			else if ((MString("-tangentuseparity") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				tangentsUseParity = true;
			}
			else if ((MString("-camAnim") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportCamerasAnim = true;
			}
			else if ((MString("-particles") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportParticles = true;
				particlesFilename = args.asString(++i,&stat);
			}
			else if ((MString("-shared") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				useSharedGeom = true;
			}
			else if ((MString("-np") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				MString npType = args.asString(++i,&stat);
				if (npType == "curFrame")
					neutralPoseType = NPT_CURFRAME;
				else if (npType == "bindPose")
					neutralPoseType = NPT_BINDPOSE;
			}
			else if ((MString("-skeletonClip") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				//get clip name
				MString clipName = args.asString(++i,&stat);
				//check if name is unique, otherwise skip the clip
				bool uniqueName = true;
				for (int k=0; k<skelClipList.size() && uniqueName; k++)
				{
					if (clipName == skelClipList[k].name)
						uniqueName = false;
				}
				//if the name is uniue, load the clip info
				if (uniqueName)
				{
					//get clip range
					MString clipRangeType = args.asString(++i,&stat);
					float startTime, stopTime;
					if (clipRangeType == "startEnd")
					{
						startTime = args.asDouble(++i,&stat);
						stopTime = args.asDouble(++i,&stat);
						MString rangeUnits = args.asString(++i,&stat);
						if (rangeUnits == "frames")
						{
							//range specified in frames => convert to seconds
							MTime t1(startTime, MTime::uiUnit());
							MTime t2(stopTime, MTime::uiUnit());
							startTime = t1.as(MTime::kSeconds);
							stopTime = t2.as(MTime::kSeconds);
						}
					}
					else
					{
						//range specified by time slider
						MTime t1 = MAnimControl::minTime();
						MTime t2 = MAnimControl::maxTime();
						startTime = t1.as(MTime::kSeconds);
						stopTime = t2.as(MTime::kSeconds);
					}
					// get sample rate
					float rate;
					MString sampleRateType = args.asString(++i,&stat);
					if (sampleRateType == "sampleByFrames")
					{
						// rate specified in frames
						int intRate = args.asInt(++i,&stat);
						MTime t = MTime(intRate, MTime::uiUnit());
						rate = t.as(MTime::kSeconds);
					}
					else
					{
						// rate specified in seconds
						rate = args.asDouble(++i,&stat);
					}
					//add clip info
					clipInfo clip;
					clip.name = clipName;
					clip.start = startTime;
					clip.stop = stopTime;
					clip.rate = rate;
					skelClipList.push_back(clip);
					std::cout << "skeleton clip " << clipName.asChar() << "\n";
					std::cout << "start: " << startTime << ", stop: " << stopTime << "\n";
					std::cout << "rate: " << rate << "\n";
					std::cout << "-----------------\n";
					std::cout.flush();
				}
				//warn of duplicate clip name
				else
				{
					std::cout << "Warning! A skeleton clip with name \"" << clipName.asChar() << "\" already exists\n";
					std::cout.flush();
				}
			}
			else if ((MString("-BSClip") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				//get clip name
				MString clipName = args.asString(++i,&stat);
				//check if name is unique, otherwise skip the clip
				bool uniqueName = true;
				for (int k=0; k<BSClipList.size() && uniqueName; k++)
				{
					if (clipName == BSClipList[k].name)
						uniqueName = false;
				}
				//if the name is uniue, load the clip info
				if (uniqueName)
				{
					//get clip range
					MString clipRangeType = args.asString(++i,&stat);
					float startTime, stopTime;
					if (clipRangeType == "startEnd")
					{
						startTime = args.asDouble(++i,&stat);
						stopTime = args.asDouble(++i,&stat);
						MString rangeUnits = args.asString(++i,&stat);
						if (rangeUnits == "frames")
						{
							//range specified in frames => convert to seconds
							MTime t1(startTime, MTime::uiUnit());
							MTime t2(stopTime, MTime::uiUnit());
							startTime = t1.as(MTime::kSeconds);
							stopTime = t2.as(MTime::kSeconds);
						}
					}
					else
					{
						//range specified by time slider
						MTime t1 = MAnimControl::minTime();
						MTime t2 = MAnimControl::maxTime();
						startTime = t1.as(MTime::kSeconds);
						stopTime = t2.as(MTime::kSeconds);
					}
					// get sample rate
					float rate;
					MString sampleRateType = args.asString(++i,&stat);
					if (sampleRateType == "sampleByFrames")
					{
						// rate specified in frames
						int intRate = args.asInt(++i,&stat);
						MTime t = MTime(intRate, MTime::uiUnit());
						rate = t.as(MTime::kSeconds);
					}
					else
					{
						// rate specified in seconds
						rate = args.asDouble(++i,&stat);
					}
					//add clip info
					clipInfo clip;
					clip.name = clipName;
					clip.start = startTime;
					clip.stop = stopTime;
					clip.rate = rate;
					BSClipList.push_back(clip);
					std::cout << "blend shape clip " << clipName.asChar() << "\n";
					std::cout << "start: " << startTime << ", stop: " << stopTime << "\n";
					std::cout << "rate: " << rate << "\n";
					std::cout << "-----------------\n";
					std::cout.flush();
				}
				//warn of duplicate clip name
				else
				{
					std::cout << "Warning! A blend shape clip with name \"" << clipName.asChar() << "\" already exists\n";
					std::cout.flush();
				}
			}
			else if ((MString("-vertexClip") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				//get clip name
				MString clipName = args.asString(++i,&stat);
				//check if name is unique, otherwise skip the clip
				bool uniqueName = true;
				for (int k=0; k<vertClipList.size() && uniqueName; k++)
				{
					if (clipName == vertClipList[k].name)
						uniqueName = false;
				}
				//if the name is uniue, load the clip info
				if (uniqueName)
				{
				//get clip range
					MString clipRangeType = args.asString(++i,&stat);
					float startTime, stopTime;
					if (clipRangeType == "startEnd")
					{
						startTime = args.asDouble(++i,&stat);
						stopTime = args.asDouble(++i,&stat);
						MString rangeUnits = args.asString(++i,&stat);
						if (rangeUnits == "frames")
						{
							//range specified in frames => convert to seconds
							MTime t1(startTime, MTime::uiUnit());
							MTime t2(stopTime, MTime::uiUnit());
							startTime = t1.as(MTime::kSeconds);
							stopTime = t2.as(MTime::kSeconds);
						}
					}
					else
					{
						//range specified by time slider
						MTime t1 = MAnimControl::minTime();
						MTime t2 = MAnimControl::maxTime();
						startTime = t1.as(MTime::kSeconds);
						stopTime = t2.as(MTime::kSeconds);
					}
					// get sample rate
					float rate;
					MString sampleRateType = args.asString(++i,&stat);
					if (sampleRateType == "sampleByFrames")
					{
						// rate specified in frames
						int intRate = args.asInt(++i,&stat);
						MTime t = MTime(intRate, MTime::uiUnit());
						rate = t.as(MTime::kSeconds);
					}
					else
					{
						// rate specified in seconds
						rate = args.asDouble(++i,&stat);
					}
					//add clip info
					clipInfo clip;
					clip.name = clipName;
					clip.start = startTime;
					clip.stop = stopTime;
					clip.rate = rate;
					vertClipList.push_back(clip);
					std::cout << "vertex clip " << clipName.asChar() << "\n";
					std::cout << "start: " << startTime << ", stop: " << stopTime << "\n";
					std::cout << "rate: " << rate << "\n";
					std::cout << "-----------------\n";
					std::cout.flush();
				}
				//warn of duplicate clip name
				else
				{
					std::cout << "Warning! A vertex animation clip with name \"" << clipName.asChar() << "\" already exists\n";
					std::cout.flush();
				}
			}
		}
	}


	// method to open output files for writing
	MStatus ParamList::openFiles()
	{
		MString msg;
		if (exportMaterial)
		{
			outMaterial.open(materialFilename.asChar());
			if (!outMaterial)
			{
				std::cout << "Error opening file: " << materialFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportAnimCurves)
		{
			outAnim.open(animFilename.asChar());
			if (!outAnim)
			{
				std::cout << "Error opening file: " << animFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportCameras)
		{
			outCameras.open(camerasFilename.asChar());
			if (!outCameras)
			{
				std::cout << "Error opening file: " << camerasFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportParticles)
		{
			outParticles.open(particlesFilename.asChar());
			if (!outParticles)
			{
				std::cout << "Error opening file: " << particlesFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		return MS::kSuccess;
	}

	// method to close open output files
	MStatus ParamList::closeFiles()
	{
		if (exportMaterial)
			outMaterial.close();
	
		if (exportAnimCurves)
			outAnim.close();
			
		if (exportCameras)
			outCameras.close();
		
		return MS::kSuccess;
	}

}	//end namespace