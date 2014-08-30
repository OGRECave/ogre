////////////////////////////////////////////////////////////////////////////////
// ogreExporter.h
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
/*********************************************************************************
 * Description: This is a plugin for Maya, that allows the export of animated    *
 *              meshes in the OGRE file format. All meshes will be combined      *
 *              together to form a single OGRE mesh, each Maya mesh will be      *
 *              translated as a submesh. Multiple materials per mesh are allowed *
 *              each group of triangles sharing the same material will become    *
 *              a separate submesh. Skeletal animation and blendshapes are       *
 *              supported, or, alternatively, vertex animation as a sequence     *
 *              of morph targets.                                                *
 *              The export command can be run via script too, for instructions   *
 *              on its usage please refer to the Instructions.txt file.          *
 *********************************************************************************/
/*********************************************************************************
 * Note: The particles exporter is an extra module submitted by the OGRE         *
 *       community, it still has to be reviewed and fixed.                       *
 *********************************************************************************/

#ifndef OGRE_EXPORTER_H
#define OGRE_EXPORTER_H

#include "mesh.h"
#include "particles.h"
#include "mayaExportLayer.h"
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>

namespace OgreMayaExporter
{
    class OgreExporter : public MPxCommand
    {
    public:
        // Public methods
        //constructor
        OgreExporter();
        //destructor
        virtual ~OgreExporter();
        //override of MPxCommand methods
        static void* creator();
        MStatus doIt(const MArgList& args);
        bool isUndoable() const;

    protected:
        // Internal methods
        //analyses a dag node in Maya and translates it to the OGRE format, 
        //it is recursively applied until the whole dag nodes tree has been visited
        MStatus translateNode(MDagPath& dagPath);
        //writes animation data to an extra .anim file
        MStatus writeAnim(MFnAnimCurve& anim);
        //writes camera data to an extra .camera file
        MStatus writeCamera(MFnCamera& camera);
        //writes all translated data to a group of OGRE files
        MStatus writeOgreData();
        //cleans up memory and exits
        void exit();

    private:
        // private members
        MStatus stat;
        ParamList m_params;
        Mesh* m_pMesh;
        MaterialSet* m_pMaterialSet;
        MSelectionList m_selList;
        MTime m_curTime;
    };




    /*********************************************************************************************
    *                                  INLINE Functions                                         *
    *********************************************************************************************/
    // Standard constructor
    inline OgreExporter::OgreExporter()
        :m_pMesh(0), m_pMaterialSet(0)
    {
        MGlobal::displayInfo("Translating scene to OGRE format");
    }

    // Routine for creating the plug-in
    inline void* OgreExporter::creator()
    {
        return new OgreExporter();
    }

    // It tells that this command is not undoable
    inline bool OgreExporter::isUndoable() const
    {
        MGlobal::displayInfo("Command is not undoable");
        return false;
    }

}   //end namespace
#endif