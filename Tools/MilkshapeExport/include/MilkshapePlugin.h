/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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

