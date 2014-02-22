////////////////////////////////////////////////////////////////////////////////
// blendshape.cpp
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

#include "blendshape.h"
#include "submesh.h"

namespace OgreMayaExporter
{
    // Constructor
    BlendShape::BlendShape()
    {
        m_pBlendShapeFn = NULL;
        clear();
    }

    // Destructor
    BlendShape::~BlendShape()
    {
        clear();
    }

    // Clear blend shape data
    void BlendShape::clear()
    {
        if (m_pBlendShapeFn)
            delete m_pBlendShapeFn;
        m_pBlendShapeFn = NULL;
        m_origEnvelope = 0;
        m_origWeights.clear();
        m_poseGroups.clear();
        poseGroup pg;
        pg.targetIndex = 0;
        m_poseGroups.insert(std::pair<int,poseGroup>(0, pg));
        m_target = T_MESH;
        m_weightConnections.clear();
    }

    // Load blend shape deformer from Maya
    MStatus BlendShape::load(MObject &blendShapeObj)
    {
        // Create a Fn for relative Maya blend shape deformer
        m_pBlendShapeFn = new MFnBlendShapeDeformer(blendShapeObj);
        // Save original envelope value for the deformer
        m_origEnvelope = m_pBlendShapeFn->envelope();
        // Save original target weights
        m_origWeights.clear();
        MIntArray indexList;
        m_pBlendShapeFn->weightIndexList(indexList);
        for (int i=0; i<indexList.length(); i++)
        {
            m_origWeights.push_back(m_pBlendShapeFn->weight(indexList[i]));
        }
        return MS::kSuccess;
    }

    // Load blend shape poses for shared geometry
    MStatus BlendShape::loadPosesShared(MDagPath& meshDag,ParamList &params,
        std::vector<vertex> &vertices,long numVertices,long offset)
    {
        MStatus stat;
        // Set blend shape target
        m_target = T_MESH;
        // Set blend shape deformer envelope to 1 to get target shapes
        m_pBlendShapeFn->setEnvelope(1);
        // Break connections on weights
        breakConnections();
        // Set weight to 0 for all targets
        MIntArray indexList;
        m_pBlendShapeFn->weightIndexList(indexList);
        for (int i=0; i<indexList.length(); i++)
        {
            stat = m_pBlendShapeFn->setWeight(indexList[i],0);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error setting weight " << indexList[i] << " to 0 on blendhape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
        }
        // Get pose names
        MStringArray poseNames;
        MString cmd = "aliasAttr -q " + m_pBlendShapeFn->name();
        MGlobal::executeCommand(cmd,poseNames,false,false);
        // Get all poses: set iteratively weight to 1 for current target shape and keep 0 for the other targets
        for (int i=0; i<indexList.length(); i++)
        {
            MString poseName = "pose" + i;
            // get pose name
            bool foundName = false;
            for (int j=1; j<poseNames.length() && !foundName; j+=2)
            {
                int idx = -1;
                sscanf(poseNames[j].asChar(),"weight[%d]",&idx);
                if (idx == i)
                {
                    poseName = poseNames[j-1];
                    foundName = true;
                    std::cout << "pose num: " << i << " name: " << poseName.asChar() << "\n";
                    std::cout.flush();
                }
            }
            // set weight to 1
            stat = m_pBlendShapeFn->setWeight(indexList[i],1);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error setting weight " << indexList[i] << " to 1 on blend shape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
            // load the pose
            stat = loadPoseShared(meshDag,params,vertices,numVertices,offset,poseName,i);
            if (stat != MS::kSuccess)
            {
                std::cout << "Failed loading target pose " << indexList[i] << "\n";
                std::cout << stat.errorString().asChar();
                std::cout.flush();
            }
            // set weight to 0
            stat = m_pBlendShapeFn->setWeight(indexList[i],0);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error resetting weight " << indexList[i] << " to 0 on blend shape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
        }
        // Set blend shape envelope to 0
        m_pBlendShapeFn->setEnvelope(0);
        // Restore targets weights
        for (int i=0; i<indexList.length(); i++)
        {
            m_pBlendShapeFn->setWeight(indexList[i],m_origWeights[i]);
        }
        // Restore connections on weights
        restoreConnections();
        return MS::kSuccess;
    }

    // Load blend shape poses for a submesh
    MStatus BlendShape::loadPosesSubmesh(MDagPath& meshDag,ParamList &params,
        std::vector<vertex> &vertices,std::vector<long>& indices,long targetIndex)
    {
        MStatus stat;
        // Set blend shape target
        m_target = T_SUBMESH;
        // Set blend shape deformer envelope to 1 to get target shapes
        m_pBlendShapeFn->setEnvelope(1);
        // Break connections on weights
        breakConnections();
        // Set weight to 0 for all targets
        MIntArray indexList;
        m_pBlendShapeFn->weightIndexList(indexList);
        for (int i=0; i<indexList.length(); i++)
        {
            stat = m_pBlendShapeFn->setWeight(indexList[i],0);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error setting weight " << indexList[i] << " to 0 on blendhape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
        }

        // Get pose names
        MStringArray poseNames;
        MString cmd = "aliasAttr -q " + m_pBlendShapeFn->name();
        MGlobal::executeCommand(cmd,poseNames,false,false);
        // Get all poses: set iteratively weight to 1 for current target shape and keep 0 for the other targets
        poseGroup pg;
        pg.targetIndex = targetIndex;
        m_poseGroups.insert(std::pair<int,poseGroup>(targetIndex,pg));
        for (int i=0; i<indexList.length(); i++)
        {
            MString poseName = "pose" + i;
            // get pose name
            bool foundName = false;
            for (int j=1; j<poseNames.length() && !foundName; j+=2)
            {
                int idx = -1;
                sscanf(poseNames[j].asChar(),"weight[%d]",&idx);
                if (idx == i)
                {
                    poseName = poseNames[j-1];
                    poseName += targetIndex;
                    foundName = true;
                    std::cout << "pose num: " << i << " name: " << poseName.asChar() << "\n";
                    std::cout.flush();
                }
            }
            // set weight to 1
            stat = m_pBlendShapeFn->setWeight(indexList[i],1);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error setting weight " << indexList[i] << " to 1 on blend shape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
            // load the pose
            stat = loadPoseSubmesh(meshDag,params,vertices,indices,poseName,targetIndex,i);
            if (stat != MS::kSuccess)
            {
                std::cout << "Failed loading target pose " << indexList[i] << "\n";
                std::cout << stat.errorString().asChar();
                std::cout.flush();
            }
            // set weight to 0
            stat = m_pBlendShapeFn->setWeight(indexList[i],0);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error resetting weight " << indexList[i] << " to 0 on blend shape deformer " << m_pBlendShapeFn->name().asChar() << "\n";
                std::cout << stat.errorString().asChar() << "\n";
                std::cout.flush();
            }
        }
        // Set blend shape envelope to 0
        m_pBlendShapeFn->setEnvelope(0);
        // Restore targets weights
        for (int i=0; i<indexList.length(); i++)
        {
            m_pBlendShapeFn->setWeight(indexList[i],m_origWeights[i]);
        }
        // Restore connections on weights
        restoreConnections();
        return MS::kSuccess;
    }

    // Load a single blend shape pose for shared geometry
    MStatus BlendShape::loadPoseShared(MDagPath& meshDag,ParamList &params,std::vector<vertex> &vertices,
        long numVertices,long offset,MString poseName,int blendShapeIndex)
    {
        // get the mesh Fn
        MFnMesh mesh(meshDag);
        // get pose group
        poseGroup& pg = m_poseGroups.find(0)->second;
        // create a new pose
        pose p;
        p.poseTarget = m_target;
        p.index = 0;
        p.blendShapeIndex = blendShapeIndex;
        p.name = poseName.asChar();
        // get vertex positions
        MFloatPointArray points;
        if (params.exportWorldCoords)
            mesh.getPoints(points,MSpace::kWorld);
        else
            mesh.getPoints(points,MSpace::kObject);
        // calculate vertex offsets
        for (int i=0; i<numVertices; i++)
        {
            vertexOffset vo;
            vertex v = vertices[offset+i];
            vo.x = points[v.index].x * params.lum - v.x;
            vo.y = points[v.index].y * params.lum - v.y;
            vo.z = points[v.index].z * params.lum - v.z;
            vo.index = offset+i;
            if (fabs(vo.x) < PRECISION)
                vo.x = 0;
            if (fabs(vo.y) < PRECISION)
                vo.y = 0;
            if (fabs(vo.z) < PRECISION)
                vo.z = 0;
            if ((vo.x!=0) || (vo.y!=0) || (vo.z!=0))
                p.offsets.push_back(vo);
        }
        // add pose to pose list
        if (p.offsets.size() > 0)
            pg.poses.push_back(p);
        if (params.bsBB)
        {
            // update bounding boxes of loaded submeshes
            for (int i=0; i<params.loadedSubmeshes.size(); i++)
            {
                MFnMesh mesh(params.loadedSubmeshes[i]->m_dagPath);
                MPoint min = mesh.boundingBox().min();
                MPoint max = mesh.boundingBox().max();
                MBoundingBox bbox(min,max);
                if (params.exportWorldCoords)
                    bbox.transformUsing(params.loadedSubmeshes[i]->m_dagPath.inclusiveMatrix());
                min = bbox.min() * params.lum;
                max = bbox.max() * params.lum;
                MBoundingBox newbbox(min,max);
                params.loadedSubmeshes[i]->m_boundingBox.expand(newbbox);
            }
        }
        // pose loaded succesfully
        return MS::kSuccess;
    }

    // Load a single blend shape pose for a submesh
    MStatus BlendShape::loadPoseSubmesh(MDagPath& meshDag,ParamList &params,std::vector<vertex>& vertices,
        std::vector<long>& indices,MString poseName,int targetIndex,int blendShapeIndex)
    {
        // get the mesh Fn
        MFnMesh mesh(meshDag);
        // get pose group
        poseGroup& pg = m_poseGroups.find(targetIndex)->second;
        // create a new pose
        pose p;
        p.poseTarget = m_target;
        p.index = targetIndex;
        p.blendShapeIndex = blendShapeIndex;
        p.name = poseName.asChar();
        // get vertex positions
        MFloatPointArray points;
        if (params.exportWorldCoords)
            mesh.getPoints(points,MSpace::kWorld);
        else
            mesh.getPoints(points,MSpace::kObject);
        // calculate vertex offsets
        for (int i=0; i<vertices.size(); i++)
        {
            long vertexIdx=indices[i];
            vertexOffset vo;
            vertex v = vertices[i];
            vo.x = points[v.index].x * params.lum - v.x;
            vo.y = points[v.index].y * params.lum - v.y;
            vo.z = points[v.index].z * params.lum - v.z;
            vo.index = i;
            if (fabs(vo.x) < PRECISION)
                vo.x = 0;
            if (fabs(vo.y) < PRECISION)
                vo.y = 0;
            if (fabs(vo.z) < PRECISION)
                vo.z = 0;
            if ((vo.x!=0) || (vo.y!=0) || (vo.z!=0))
                p.offsets.push_back(vo);
        }
        // add pose to pose list
        if (p.offsets.size() > 0)
            pg.poses.push_back(p);
        if (params.bsBB)
        {
            // update bounding boxes of loaded submeshes
            for (int i=0; i<params.loadedSubmeshes.size(); i++)
            {
                MFnMesh mesh(params.loadedSubmeshes[i]->m_dagPath);
                MPoint min = mesh.boundingBox().min();
                MPoint max = mesh.boundingBox().max();
                MBoundingBox bbox(min,max);
                if (params.exportWorldCoords)
                    bbox.transformUsing(params.loadedSubmeshes[i]->m_dagPath.inclusiveMatrix());
                min = bbox.min() * params.lum;
                max = bbox.max() * params.lum;
                MBoundingBox newbbox(min,max);
                params.loadedSubmeshes[i]->m_boundingBox.expand(newbbox);
            }
        }
        // pose loaded succesfully
        return MS::kSuccess;
    }



    // Load a blend shape animation track
    Track BlendShape::loadTrack(float start,float stop,float rate,ParamList& params,int targetIndex,int startPoseId)
    {
        MStatus stat;
        MString msg;
        std::vector<float> times;
        // Create a track for current clip
        Track t;
        t.m_type = TT_POSE;
        t.m_target = m_target;
        t.m_index = targetIndex;
        t.m_vertexKeyframes.clear();
        // Calculate times from clip sample rate
        times.clear();
        if (rate <= 0)
        {
            std::cout << "invalid sample rate for the clip (must be >0), we skip it\n";
            std::cout.flush();
            return t;
        }
        for (float time=start; time<stop; time+=rate)
            times.push_back(time);
        times.push_back(stop);
        // Get animation length
        float length=0;
        if (times.size() >= 0)
            length = times[times.size()-1] - times[0];
        if (length < 0)
        {
            std::cout << "invalid time range for the clip, we skip it\n";
            std::cout.flush();
            return t;
        }
        // Evaluate animation curves at selected times
        for (int i=0; i<times.size(); i++)
        {
            // Set time to wanted sample time
            MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
            // Load a keyframe at current time
            vertexKeyframe key = loadKeyframe(times[i]-times[0],params,targetIndex, startPoseId);
            // Add keyframe to joint track
            if (key.poserefs.size() > 0)
                t.addVertexKeyframe(key);
        }
        // Clip successfully loaded
        return t;
    }


    // Load a blend shape animation keyframe
    vertexKeyframe BlendShape::loadKeyframe(float time,ParamList& params,int targetIndex, int startPoseId)
    {
        poseGroup& pg = m_poseGroups.find(targetIndex)->second;
        // Create keyframe
        vertexKeyframe key;
        key.time = time;
        key.poserefs.clear();
        // Read weights of all poses at current time
        // Get blend shape deformer envelope
        float envelope = m_pBlendShapeFn->envelope();
        // Get weights of all targets
        MIntArray indexList;
        m_pBlendShapeFn->weightIndexList(indexList);
        for (int i=0; i<pg.poses.size(); i++)
        {
            pose& p = pg.poses[i];
            // Create a pose reference
            // Index of pose is relative to current blend shape
            vertexPoseRef poseref;
            poseref.poseIndex = startPoseId + i;
            poseref.poseWeight = envelope * m_pBlendShapeFn->weight(indexList[p.blendShapeIndex]);
            key.poserefs.push_back(poseref);
        }
        return key;
    }

    // Get blend shape deformer name
    MString BlendShape::getName()
    {
        return m_pBlendShapeFn->name();
    }

    // Get blend shape poses
    stdext::hash_map<int,poseGroup>& BlendShape::getPoseGroups()
    {
        return m_poseGroups;
    }

    // Set maya blend shape deformer envelope
    void BlendShape::setEnvelope(float envelope)
    {
        m_pBlendShapeFn->setEnvelope(envelope);
    }
    // Restore maya blend shape deformer original envelope
    void BlendShape::restoreEnvelope()
    {
        m_pBlendShapeFn->setEnvelope(m_origEnvelope);
    }
    // Break connections to this blendshape
    void BlendShape::breakConnections()
    {
        MStatus stat;
        MDagModifier dagModifier;
        // Clear the stored connections
        m_weightConnections.clear();
        // Save node connections and break them
        MPlug weightsPlug = m_pBlendShapeFn->findPlug("weight",true);
        for (int i=0; i<weightsPlug.evaluateNumElements(); i++)
        {
            MPlug wPlug = weightsPlug.elementByPhysicalIndex(i);
            MPlugArray srcConnections;
            MPlugArray dstConnections;
            wPlug.connectedTo(srcConnections,false,true);
            wPlug.connectedTo(dstConnections,true,false);
            weightConnections wcon;
            for (int j=0; j<srcConnections.length(); j++)
            {
                wcon.srcConnections.append(srcConnections[j]);
                dagModifier.disconnect(wPlug,srcConnections[j]);
                dagModifier.doIt();
            }
            for (int j=0; j<dstConnections.length(); j++)
            {
                wcon.dstConnections.append(dstConnections[j]);
                stat = dagModifier.disconnect(dstConnections[j],wPlug);
                if (MS::kSuccess != stat)
                {
                    std::cout << "Error trying to disconnect plug " << wPlug.name().asChar() << " and plug " << dstConnections[j].name().asChar() << "\n";
                    std::cout << stat.errorString().asChar() << "\n";
                    std::cout.flush();
                }
                stat = dagModifier.doIt();
                if (MS::kSuccess != stat)
                {
                    std::cout << "Error trying to disconnect plug " << wPlug.name().asChar() << " and plug " << dstConnections[j].name().asChar() << "\n";
                    std::cout << stat.errorString().asChar() << "\n";
                    std::cout.flush();
                }
            }
            m_weightConnections.push_back(wcon);
        }
    }


    // Restore connections on this blendshape
    void BlendShape::restoreConnections()
    {
        MDagModifier dagModifier;
        // Recreate stored connections on the weight attributes
        MPlug weightsPlug = m_pBlendShapeFn->findPlug("weight",true);
        for (int i=0; i<weightsPlug.evaluateNumElements(); i++)
        {
            MPlug wPlug = weightsPlug.elementByPhysicalIndex(i);
            weightConnections& wcon = m_weightConnections[i];
            for (int j=0; j<wcon.srcConnections.length(); j++)
            {
                dagModifier.connect(wPlug,wcon.srcConnections[j]);
                dagModifier.doIt();
            }
            for (int j=0; j<wcon.dstConnections.length(); j++)
            {
                dagModifier.connect(wcon.dstConnections[j],wPlug);
                dagModifier.doIt();
            }
        }
    }


} // end namespace