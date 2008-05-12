/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __MilkshapePlugin_H__
#define __MilkshapePlugin_H__

#include "msPlugIn.h"
#include "windows.h"
#include "Ogre.h"


/** A plugin class for Milkshape3D to export to OGRE model formats. 
@remarks
    This class is the implementor of the exporting interface for Milkshape3D, to allow
    you to export your models to OGRE format from that tool.
    Note that this plugin delegates most of the detail of exporting the model to the
    generic model export framework.
*/
class MilkshapePlugin : public cMsPlugIn
{
    
    char mTitle[64];

public:
	MilkshapePlugin ();
    virtual ~MilkshapePlugin ();

public:
    /// As required by Milkshape
    int             GetType ();
    /// As required by Milkshape
    const char *    GetTitle ();
    /// As required by Milkshape
    int             Execute (msModel* pModel);

    /** Callback to process window events */
#if OGRE_ARCHITECTURE_64 == OGRE_ARCH_TYPE
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
#else
    static BOOL CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
#endif

    bool exportMaterials;
    bool generateLods;
    bool generateEdgeLists;
    bool generateTangents;
	Ogre::VertexElementSemantic tangentSemantic;
	bool tangentsSplitMirrored;
	bool tangentsSplitRotated;
	bool tangentsUseParity;
    unsigned short numLods;
    float lodDepthIncrement;
    Ogre::ProgressiveMesh::VertexReductionQuota lodReductionMethod;
    float lodReductionAmount;
    bool exportMesh;
    bool exportSkeleton;
    bool splitAnimations;
    float fps; 

protected:
    bool showOptions(void);
    void doExportMesh(msModel* pModel);
    void doExportMaterials(msModel* pModel);
    void doExportAnimations(msModel* pModel, Ogre::SkeletonPtr& skel);
    Ogre::SkeletonPtr doExportSkeleton(msModel* pModel, Ogre::MeshPtr& mesh); // Skeleton returned for deletion later
    bool locateSkeleton(Ogre::MeshPtr& mesh);
	Ogre::ColourValue msVec4ToColourValue(float prop[4]);
};

#endif

