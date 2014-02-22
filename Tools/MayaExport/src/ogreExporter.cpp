////////////////////////////////////////////////////////////////////////////////
// ogreExporter.cpp
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

#include "OgreExporter.h"

namespace OgreMayaExporter
{
    OgreExporter::~OgreExporter()
    {
        exit();
    }
    // Restore the scene to a state previous to the export, clean up memory and exit
    void OgreExporter::exit()
    {
        // Restore active selection list
        MGlobal::setActiveSelectionList(m_selList);
        // Restore current time
        MAnimControl::setCurrentTime(m_curTime);
        // Free memory
        delete m_pMesh;
        m_pMesh = 0;
        delete m_pMaterialSet;
        m_pMaterialSet = 0;
        // Close output files
        m_params.closeFiles();
        std::cout.flush();
    }

    // Execute the command
    MStatus OgreExporter::doIt(const MArgList& args)
    {
        // clean up
        delete m_pMesh;
        delete m_pMaterialSet;

        // Parse the arguments.
        m_params.parseArgs(args);
        // Create output files
        m_params.openFiles();
        // Create a new empty mesh
        m_pMesh = new Mesh();
        // Create a new empty material set
        m_pMaterialSet = new MaterialSet();
        // Save current time for later restore
        m_curTime = MAnimControl::currentTime();
        // Save active selection list for later restore
        MGlobal::getActiveSelectionList(m_selList);
        /**************************** LOAD DATA **********************************/
        if (m_params.exportAll)
        {   // We are exporting the whole scene
            std::cout << "Export the whole scene\n";
            std::cout.flush();
            MItDag dagIter;
            MFnDagNode worldDag (dagIter.root());
            MDagPath worldPath;
            worldDag.getPath(worldPath);
            stat = translateNode(worldPath);
        }
        else
        {   // We are translating a selection
            std::cout << "Export selected objects\n";
            std::cout.flush();
            // Get the selection list
            MSelectionList activeList;
            stat = MGlobal::getActiveSelectionList(activeList);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error retrieving selection list\n";
                std::cout.flush();
                exit();
                return MS::kFailure;
            }
            MItSelectionList iter(activeList);

            for ( ; !iter.isDone(); iter.next())
            {                               
                MDagPath dagPath;
                stat = iter.getDagPath(dagPath);
                stat = translateNode(dagPath); 
            }                           
        }

        // Load vertex animations
        if (m_params.exportVertAnims)
            m_pMesh->loadAnims(m_params);

        // Load blend shapes
        if (m_params.exportBlendShapes)
            m_pMesh->loadBlendShapes(m_params);

        // Restore skeleton to correct pose
        if (m_pMesh->getSkeleton())
            m_pMesh->getSkeleton()->restorePose();

        // Load skeleton animation (do it now, so we have loaded all needed joints)
        if (m_pMesh->getSkeleton() && m_params.exportSkelAnims)
        {
            // Load skeleton animations
            m_pMesh->getSkeleton()->loadAnims(m_params);
        }

        /**************************** WRITE DATA **********************************/
        stat = writeOgreData();

        std::cout << "Export completed succesfully\n";
        std::cout.flush();
        exit();

        return MS::kSuccess;
    }


    /**************************** TRANSLATE A NODE **********************************/
    // Method for iterating over nodes in a dependency graph from top to bottom
    MStatus OgreExporter::translateNode(MDagPath& dagPath)
    {
        if (m_params.exportAnimCurves)
        {
            MObject dagPathNode = dagPath.node();
            MItDependencyGraph animIter( dagPathNode,
                MFn::kAnimCurve,
                MItDependencyGraph::kUpstream,
                MItDependencyGraph::kDepthFirst,
                MItDependencyGraph::kNodeLevel,
                &stat );

            if (stat)
            {
                for (; !animIter.isDone(); animIter.next())
                {
                    MObject anim = animIter.thisNode(&stat);
                    MFnAnimCurve animFn(anim,&stat);
                    std::cout << "Found animation curve: " << animFn.name().asChar() << "\n";
                    std::cout << "Translating animation curve: " << animFn.name().asChar() << "...\n";
                    std::cout.flush();
                    stat = writeAnim(animFn);
                    if (MS::kSuccess == stat)
                    {
                        std::cout << "OK\n";
                        std::cout.flush();
                    }
                    else
                    {
                        std::cout << "Error, Aborting operation\n";
                        std::cout.flush();
                        return MS::kFailure;
                    }
                }
            }
        }
        if (dagPath.hasFn(MFn::kMesh)&&(m_params.exportMesh||m_params.exportMaterial||m_params.exportSkeleton)
            && (dagPath.childCount() == 0))
        {   // we have found a mesh shape node, it can't have any children, and it contains
            // all the mesh geometry data
            MDagPath meshDag = dagPath;
            MFnMesh meshFn(meshDag);
            if (!meshFn.isIntermediateObject())
            {
                std::cout << "Found mesh node: " << meshDag.fullPathName().asChar() << "\n";
                std::cout << "Loading mesh node " << meshDag.fullPathName().asChar() << "...\n";
                std::cout.flush();
                stat = m_pMesh->load(meshDag,m_params);
                if (MS::kSuccess == stat)
                {
                    std::cout << "OK\n";
                    std::cout.flush();
                }
                else
                {
                    std::cout << "Error, mesh skipped\n";
                    std::cout.flush();
                }
            }
        }
        else if (dagPath.hasFn(MFn::kCamera)&&(m_params.exportCameras) && (!dagPath.hasFn(MFn::kShape)))
        {   // we have found a camera shape node, it can't have any children, and it contains
            // all information about the camera
            MFnCamera cameraFn(dagPath);
            if (!cameraFn.isIntermediateObject())
            {
                std::cout <<  "Found camera node: "<< dagPath.fullPathName().asChar() << "\n";
                std::cout <<  "Translating camera node: "<< dagPath.fullPathName().asChar() << "...\n";
                std::cout.flush();
                stat = writeCamera(cameraFn);
                if (MS::kSuccess == stat)
                {
                    std::cout << "OK\n";
                    std::cout.flush();
                }
                else
                {
                    std::cout << "Error, Aborting operation\n";
                    std::cout.flush();
                    return MS::kFailure;
                }
            }
        }
        else if ( ( dagPath.apiType() == MFn::kParticle ) && m_params.exportParticles )
        {   // we have found a set of particles
            MFnDagNode fnNode(dagPath);
            if (!fnNode.isIntermediateObject())
            {
                std::cout <<  "Found particles node: "<< dagPath.fullPathName().asChar() << "\n";
                std::cout <<  "Translating particles node: "<< dagPath.fullPathName().asChar() << "...\n";
                std::cout.flush();
                Particles particles;
                particles.load(dagPath,m_params);
                stat = particles.writeToXML(m_params);
                if (MS::kSuccess == stat)
                {
                    std::cout << "OK\n";
                    std::cout.flush();
                }
                else
                {
                    std::cout << "Error, Aborting operation\n";
                    std::cout.flush();
                    return MS::kFailure;
                }
            }
        }
        // look for meshes and cameras within the node's children
        for (uint i=0; i<dagPath.childCount(); i++)
        {
            MObject child = dagPath.child(i);
             MDagPath childPath = dagPath;
            stat = childPath.push(child);
            if (MS::kSuccess != stat)
            {
                std::cout << "Error retrieving path to child " << i << " of: " << dagPath.fullPathName().asChar();
                std::cout.flush();
                return MS::kFailure;
            }
            stat = translateNode(childPath);
            if (MS::kSuccess != stat)
                return MS::kFailure;
        }
        return MS::kSuccess;
    }



    /********************************************************************************************************
    *                       Method to translate a single animation curve                                   *
    ********************************************************************************************************/
    MStatus OgreExporter::writeAnim(MFnAnimCurve& anim)
    {
        m_params.outAnim << "anim " << anim.name().asChar() << "\n";
        m_params.outAnim <<"{\n";
        m_params.outAnim << "\t//Time   /    Value\n";

        for (uint i=0; i<anim.numKeys(); i++)
            m_params.outAnim << "\t" << anim.time(i).as(MTime::kSeconds) << "\t" << anim.value(i) << "\n";

        m_params.outAnim << "}\n\n";
        return MS::kSuccess;
    }



    /********************************************************************************************************
    *                           Method to translate a single camera                                        *
    ********************************************************************************************************/
    MStatus OgreExporter::writeCamera(MFnCamera& camera)
    {
        MPlug plug;
        MPlugArray srcplugarray;
        double dist;
        MAngle angle;
        MFnTransform* cameraTransform = NULL;
        MFnAnimCurve* animCurve = NULL;
        // get camera transform
        for (int i=0; i<camera.parentCount(); i++)
        {
            if (camera.parent(i).hasFn(MFn::kTransform))
            {
                cameraTransform = new MFnTransform(camera.parent(i));
                continue;
            }
        }
        // start camera description
        m_params.outCameras << "camera " << cameraTransform->partialPathName().asChar() << "\n";
        m_params.outCameras << "{\n";

        //write camera type
        m_params.outCameras << "\ttype ";
        if (camera.isOrtho())
            m_params.outCameras << "ortho\n";
        else
            m_params.outCameras << "persp\n";

        // write translation data
        m_params.outCameras << "\ttranslation\n";
        m_params.outCameras << "\t{\n";
        //translateX
        m_params.outCameras << "\t\tx ";
        plug = cameraTransform->findPlug("translateX");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i < srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to translateX attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(dist);
            m_params.outCameras << "= " << dist << "\n";
        }
        //translateY
        m_params.outCameras << "\t\ty ";
        plug = cameraTransform->findPlug("translateY");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i< srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to translateY attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(dist);
            m_params.outCameras << "= " << dist << "\n";
        }
        //translateZ
        m_params.outCameras << "\t\tz ";
        plug = cameraTransform->findPlug("translateZ");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i< srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to translateZ attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(dist);
            m_params.outCameras << "= " << dist << "\n";
        }
        m_params.outCameras << "\t}\n";

        // write rotation data
        m_params.outCameras << "\trotation\n";
        m_params.outCameras << "\t{\n";
        m_params.outCameras << "\t\tx ";
        //rotateX
        plug = cameraTransform->findPlug("rotateX");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i< srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to rotateX attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(angle);
            m_params.outCameras << "= " << angle.asDegrees() << "\n";
        }
        //rotateY
        m_params.outCameras << "\t\ty ";
        plug = cameraTransform->findPlug("rotateY");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i< srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to rotateY attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(angle);
            m_params.outCameras << "= " << angle.asDegrees() << "\n";
        }
        //rotateZ
        m_params.outCameras << "\t\tz ";
        plug = cameraTransform->findPlug("rotateZ");
        if (plug.isConnected() && m_params.exportCamerasAnim)
        {
            plug.connectedTo(srcplugarray,true,false,&stat);
            for (int i=0; i< srcplugarray.length(); i++)
            {
                if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
                {
                    if (animCurve)
                        delete animCurve;
                    animCurve = new MFnAnimCurve(srcplugarray[i].node());
                    continue;
                }
                else if (i == srcplugarray.length()-1)
                {
                    std::cout << "Invalid link to rotateZ attribute\n";
                    return MS::kFailure;
                }
            }
            m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
        }
        else
        {
            plug.getValue(angle);
            m_params.outCameras << "= " << angle.asDegrees() << "\n";
        }
        m_params.outCameras << "\t}\n";

        // end camera description
        m_params.outCameras << "}\n\n";
        if (cameraTransform != NULL)
            delete cameraTransform;
        if (animCurve != NULL)
            delete animCurve;
        return MS::kSuccess;
    }

    /********************************************************************************************************
    *                           Method to write data to OGRE format                                         *
    ********************************************************************************************************/
    MStatus OgreExporter::writeOgreData()
    {
        // Create singletons
        Ogre::LogManager logMgr;
        Ogre::ResourceGroupManager rgm;
        Ogre::MeshManager meshMgr;
        Ogre::SkeletonManager skelMgr;
        Ogre::MaterialManager matMgr;
        Ogre::DefaultHardwareBufferManager hardwareBufMgr;

        // Create a log
        logMgr.createLog("ogreMayaExporter.log", true);

        // Write mesh binary
        if (m_params.exportMesh)
        {
            std::cout << "Writing mesh binary...\n";
            std::cout.flush();
            stat = m_pMesh->writeOgreBinary(m_params);
            if (stat != MS::kSuccess)
            {
                std::cout << "Error writing mesh binary file\n";
                std::cout.flush();
            }
        }

        // Write skeleton binary
        if (m_params.exportSkeleton)
        {
            if (m_pMesh->getSkeleton())
            {
                std::cout << "Writing skeleton binary...\n";
                std::cout.flush();
                stat = m_pMesh->getSkeleton()->writeOgreBinary(m_params);
                if (stat != MS::kSuccess)
                {
                    std::cout << "Error writing mesh binary file\n";
                    std::cout.flush();
                }
            }
        }
        
        // Write materials data
        if (m_params.exportMaterial)
        {
            std::cout << "Writing materials data...\n";
            std::cout.flush();
            stat  = m_pMaterialSet->writeOgreScript(m_params);
            if (stat != MS::kSuccess)
            {
                std::cout << "Error writing materials file\n";
                std::cout.flush();
            }
        }

        return MS::kSuccess;
    }

} // end namespace


// Routine for registering the command within Maya
MStatus initializePlugin( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin( obj, "OgreExporter", "7.0", "Any");
    status = plugin.registerCommand( "ogreExport", OgreMayaExporter::OgreExporter::creator );
    if (!status) {
        status.perror("registerCommand");
        return status;
    }
    
    return status;
}

// Routine for unregistering the command within Maya
MStatus uninitializePlugin( MObject obj)
{
    MStatus   status;
    MFnPlugin plugin( obj );
    status = plugin.deregisterCommand( "ogreExport" );
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }
    
    return status;
}

