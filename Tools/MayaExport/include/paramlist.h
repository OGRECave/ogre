////////////////////////////////////////////////////////////////////////////////
// paramlist.h
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

#ifndef PARAMLIST_H
#define PARAMLIST_H

#include "mayaExportLayer.h"

// Length units multipliers from Maya internal unit (cm)

#define CM2MM 10.0
#define CM2CM 1.0
#define CM2M  0.01
#define CM2IN 0.393701
#define CM2FT 0.0328084
#define CM2YD 0.0109361

namespace OgreMayaExporter
{
    class Submesh;

    typedef struct clipInfoTag
    {
        float start;                            //start time of the clip
        float stop;                             //end time of the clip
        float rate;                             //sample rate of anim curves, -1 means auto
        MString name;                           //clip name
    } clipInfo;

    typedef enum
    {
        NPT_CURFRAME,
        NPT_BINDPOSE
    } NeutralPoseType;

    typedef enum
    {
        TS_TEXCOORD,
        TS_TANGENT
    } TangentSemantic;

    /***** Class ParamList *****/
    class ParamList
    {
    public:
        // class members
        bool exportMesh, exportMaterial, exportAnimCurves, exportCameras, exportAll, exportVBA,
            exportVertNorm, exportVertCol, exportTexCoord, exportCamerasAnim,
            exportSkeleton, exportSkelAnims, exportBSAnims, exportVertAnims, exportBlendShapes, 
            exportWorldCoords, useSharedGeom, lightingOff, copyTextures, exportParticles,
            buildTangents, buildEdges, skelBB, bsBB, vertBB, 
            tangentsSplitMirrored, tangentsSplitRotated, tangentsUseParity;

        float lum;  // Length Unit Multiplier

        MString meshFilename, skeletonFilename, materialFilename, animFilename, camerasFilename, matPrefix,
            texOutputDir, particlesFilename;

        std::ofstream outMaterial, outAnim, outCameras, outParticles;

        MStringArray writtenMaterials;

        std::vector<clipInfo> skelClipList;
        std::vector<clipInfo> BSClipList;
        std::vector<clipInfo> vertClipList;

        NeutralPoseType neutralPoseType;
        TangentSemantic tangentSemantic;

        std::vector<Submesh*> loadedSubmeshes;
        std::vector<MDagPath> currentRootJoints;

        // constructor
        ParamList() {
            lum = 1.0;
            exportMesh = false;
            exportMaterial = false;
            exportSkeleton = false;
            exportSkelAnims = false;
            exportBSAnims = false;
            exportVertAnims = false;
            exportBlendShapes = false;
            exportAnimCurves = false;
            exportCameras = false;
            exportParticles = false;
            exportAll = false;
            exportWorldCoords = false;
            exportVBA = false;
            exportVertNorm = false;
            exportVertCol = false;
            exportTexCoord = false;
            exportCamerasAnim = false;
            useSharedGeom = false;
            lightingOff = false;
            copyTextures = false;
            skelBB = false;
            bsBB = false;
            vertBB = false;
            meshFilename = "";
            skeletonFilename = "";
            materialFilename = "";
            animFilename = "";
            camerasFilename = "";
            particlesFilename = "";
            matPrefix = "";
            texOutputDir = "";
            skelClipList.clear();
            BSClipList.clear();
            vertClipList.clear();
            neutralPoseType = NPT_CURFRAME;
            buildEdges = false;
            buildTangents = false;
            tangentsSplitMirrored = false;
            tangentsSplitRotated = false;
            tangentsUseParity = false;
            tangentSemantic = TS_TANGENT;
            loadedSubmeshes.clear();
            currentRootJoints.clear();
        }

        ParamList& operator=(ParamList& source) 
        {
            int i;
            lum = source.lum;
            exportMesh = source.exportMesh;
            exportMaterial = source.exportMaterial;
            exportSkeleton = source.exportSkeleton;
            exportSkelAnims = source.exportSkelAnims;
            exportBSAnims = source.exportBSAnims;
            exportVertAnims = source.exportVertAnims;
            exportBlendShapes = source.exportBlendShapes;
            exportAnimCurves = source.exportAnimCurves;
            exportCameras = source.exportCameras;
            exportAll = source.exportAll;
            exportWorldCoords = source.exportWorldCoords;
            exportVBA = source.exportVBA;
            exportVertNorm = source.exportVertNorm;
            exportVertCol = source.exportVertCol;
            exportTexCoord = source.exportTexCoord;
            exportCamerasAnim = source.exportCamerasAnim;
            exportParticles = source.exportParticles;
            useSharedGeom = source.useSharedGeom;
            lightingOff = source.lightingOff;
            copyTextures = source.copyTextures;
            skelBB = source.skelBB;
            bsBB = source.bsBB;
            vertBB = source.vertBB;
            meshFilename = source.meshFilename;
            skeletonFilename = source.skeletonFilename;
            materialFilename = source.materialFilename;
            animFilename = source.animFilename;
            camerasFilename = source.camerasFilename;
            particlesFilename = source.particlesFilename;
            matPrefix = source.matPrefix;
            texOutputDir = source.texOutputDir;
            buildEdges = source.buildEdges;
            buildTangents = source.buildTangents;
            tangentsSplitMirrored = source.tangentsSplitMirrored;
            tangentsSplitRotated = source.tangentsSplitRotated;
            tangentsUseParity = source.tangentsUseParity;
            tangentSemantic = source.tangentSemantic;
            skelClipList.resize(source.skelClipList.size());
            for (i=0; i< skelClipList.size(); i++)
            {
                skelClipList[i].name = source.skelClipList[i].name;
                skelClipList[i].start = source.skelClipList[i].start;
                skelClipList[i].stop = source.skelClipList[i].stop;
                skelClipList[i].rate = source.skelClipList[i].rate;
            }
            BSClipList.resize(source.BSClipList.size());
            for (i=0; i< BSClipList.size(); i++)
            {
                BSClipList[i].name = source.BSClipList[i].name;
                BSClipList[i].start = source.BSClipList[i].start;
                BSClipList[i].stop = source.BSClipList[i].stop;
                BSClipList[i].rate = source.BSClipList[i].rate;
            }
            vertClipList.resize(source.vertClipList.size());
            for (i=0; i< vertClipList.size(); i++)
            {
                vertClipList[i].name = source.vertClipList[i].name;
                vertClipList[i].start = source.vertClipList[i].start;
                vertClipList[i].stop = source.vertClipList[i].stop;
                vertClipList[i].rate = source.vertClipList[i].rate;
            }
            neutralPoseType = source.neutralPoseType;
            for (i=0; i<source.loadedSubmeshes.size(); i++)
                loadedSubmeshes.push_back(source.loadedSubmeshes[i]);
            for (i=0; i<source.currentRootJoints.size(); i++)
                currentRootJoints.push_back(source.currentRootJoints[i]);

            return *this;
        }

        // destructor
        ~ParamList() {
            if (outMaterial)
                outMaterial.close();
            if (outAnim)
                outAnim.close();
            if (outCameras)
                outCameras.close();
            if (outParticles)
                outParticles.close();
        }
        // method to pars arguments and set parameters
        void parseArgs(const MArgList &args);
        // method to open files for writing
        MStatus openFiles();
        // method to close open output files
        MStatus closeFiles();
    };

};  //end namespace

#endif