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
#include "MilkshapePlugin.h"
#include "Ogre.h"
#include "msLib.h"
#include "resource.h"
#include "OgreStringConverter.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreVertexIndexData.h"
#include "OgreResourceGroupManager.h"


//---------------------------------------------------------------------
MilkshapePlugin::MilkshapePlugin ()
{
    strcpy(mTitle, "OGRE Mesh / Skeleton...");
}
//---------------------------------------------------------------------
MilkshapePlugin::~MilkshapePlugin ()
{
    // do nothing
}
//---------------------------------------------------------------------
int MilkshapePlugin::GetType ()
{
    return(cMsPlugIn::eTypeExport|eNormalsAndTexCoordsPerTriangleVertex);
}
//---------------------------------------------------------------------
const char* MilkshapePlugin::GetTitle ()
{
    return mTitle;
}
//---------------------------------------------------------------------
int MilkshapePlugin::Execute (msModel* pModel)
{
    // Do nothing if no model selected
    if (!pModel)
        return -1;

	Ogre::LogManager logMgr;
	logMgr.createLog("msOgreExporter.log", true);
	logMgr.logMessage("OGRE Milkshape Exporter Log");
	logMgr.logMessage("---------------------------");
	Ogre::ResourceGroupManager resGrpMgr;

	//
    // check, if we have something to export
    //
    if (msModel_GetMeshCount (pModel) == 0)
    {
        ::MessageBox (NULL, "The model is empty!  Nothing exported!", "OGRE Export", MB_OK | MB_ICONWARNING);
        return 0;
    }

    if (!showOptions()) return 0;

    if (exportMesh)
    {
        doExportMesh(pModel);
    }

    return 0;

}
//---------------------------------------------------------------------
MilkshapePlugin *plugin;
#if OGRE_ARCHITECTURE_64 == OGRE_ARCH_TYPE
INT_PTR MilkshapePlugin::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#else
BOOL MilkshapePlugin::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
#endif
{
    HWND hwndDlgItem;
	int sel;

    switch (iMsg)
    {

    case WM_INITDIALOG:
        // Center myself
        int x, y, screenWidth, screenHeight;
        RECT rcDlg;
        GetWindowRect(hDlg, &rcDlg);
        screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
        screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

        x = (screenWidth / 2) - ((rcDlg.right - rcDlg.left) / 2);
        y = (screenHeight / 2) - ((rcDlg.bottom - rcDlg.top) / 2);

        MoveWindow(hDlg, x, y, (rcDlg.right - rcDlg.left),
            (rcDlg.bottom - rcDlg.top), TRUE);

        // Check mesh export
        hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MESH);
        SendMessage(hwndDlgItem, BM_SETCHECK, BST_CHECKED,0);

        // Set default LOD options
        hwndDlgItem = GetDlgItem(hDlg, IDC_NUM_LODS);
        SetWindowText(hwndDlgItem, "5");
        hwndDlgItem = GetDlgItem(hDlg, IDC_LOD_DEPTH);
        SetWindowText(hwndDlgItem, "500");
        hwndDlgItem = GetDlgItem(hDlg, IDC_LOD_VRQ);
        SetWindowText(hwndDlgItem, "25");
        hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_LOD_STYLE);
        SendMessage(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"percent");
        SendMessage(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"vertices");
        SendMessage(hwndDlgItem, CB_SETCURSEL, 0, 0);

        // Check edge lists
        hwndDlgItem = GetDlgItem(hDlg, IDC_EDGE_LISTS);
        SendMessage(hwndDlgItem, BM_SETCHECK, BST_CHECKED,0);

        // Check skeleton export
        hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_SKEL);
        SendMessage(hwndDlgItem, BM_SETCHECK, BST_CHECKED,0);

		// Set tangents option
		hwndDlgItem = GetDlgItem(hDlg, IDC_TANGENTS_TARGET);
		SendMessage(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"Tangent Semantic");
		SendMessage(hwndDlgItem, CB_ADDSTRING, 0, (LPARAM)"Texture Coord Semantic");
		SendMessage(hwndDlgItem, CB_SETCURSEL, 0, 0);



        // Set default FPS
        hwndDlgItem = GetDlgItem(hDlg, IDC_FPS);
        SetWindowText(hwndDlgItem, "24");


        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDOK:
                char val[20];

                // Set options
                hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MESH);
                plugin->exportMesh = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                hwndDlgItem = GetDlgItem(hDlg, IDC_GENERATE_LOD);
                plugin->generateLods = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;
                if (plugin->generateLods)
                {
                    hwndDlgItem = GetDlgItem(hDlg, IDC_NUM_LODS);
                    GetWindowText(hwndDlgItem, val, 20);
                    plugin->numLods = atoi(val);
                    if (!plugin->numLods)
                    {
                        MessageBox(hDlg, "Invalid number of LODs specified", "Validation error", MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LOD_DEPTH);
                    GetWindowText(hwndDlgItem, val, 20);
                    plugin->lodDepthIncrement = atof(val);
                    if (!plugin->lodDepthIncrement)
                    {
                        MessageBox(hDlg, "Invalid LOD depth increment specified", "Validation error", MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    hwndDlgItem = GetDlgItem(hDlg, IDC_LOD_VRQ);
                    GetWindowText(hwndDlgItem, val, 20);
                    plugin->lodReductionAmount = atof(val);
                    if (!plugin->lodReductionAmount)
                    {
                        MessageBox(hDlg, "Invalid LOD reduction amount specified", "Validation error", MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    hwndDlgItem = GetDlgItem(hDlg, IDC_CBO_LOD_STYLE);
                    sel = SendMessage(hwndDlgItem, CB_GETCURSEL,0,0);
                    if (sel == 0)
                    {
                        // percent
                        plugin->lodReductionMethod = Ogre::ProgressiveMesh::VRQ_PROPORTIONAL;
                        // adjust percent to parametric
                        plugin->lodReductionAmount *= 0.01;
                    }
                    else if (sel == 1)
                    {
                        // absolute
                        plugin->lodReductionMethod = Ogre::ProgressiveMesh::VRQ_CONSTANT;
                    }
                    else
                    {
                        MessageBox(hDlg, "Invalid LOD reduction method specified", "Validation error", MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }

                }

                hwndDlgItem = GetDlgItem(hDlg, IDC_EDGE_LISTS);
                plugin->generateEdgeLists = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                hwndDlgItem = GetDlgItem(hDlg, IDC_TANGENT_VECTORS);
                plugin->generateTangents = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;
				hwndDlgItem = GetDlgItem(hDlg, IDC_TANGENTS_TARGET);
				sel = SendMessage(hwndDlgItem, CB_GETCURSEL,0,0);
				if (sel == 0)
				{
					// tangents
					plugin->tangentSemantic = Ogre::VES_TANGENT;
				}
				else
				{
					// texture coords
					plugin->tangentSemantic = Ogre::VES_TEXTURE_COORDINATES;
				}
				hwndDlgItem = GetDlgItem(hDlg, IDC_TANG_SPLIT_MIRR);
				plugin->tangentsSplitMirrored = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;
				hwndDlgItem = GetDlgItem(hDlg, IDC_TANG_SPLIT_ROT);
				plugin->tangentsSplitRotated = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;
				hwndDlgItem = GetDlgItem(hDlg, IDC_4D_TANGENTS);
				plugin->tangentsUseParity = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_SKEL);
                plugin->exportSkeleton = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;


                hwndDlgItem = GetDlgItem(hDlg, IDC_SPLIT_ANIMATION);
                plugin->splitAnimations = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

				hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MATERIALS);
				plugin->exportMaterials = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

				hwndDlgItem = GetDlgItem(hDlg, IDC_FPS);
                GetWindowText(hwndDlgItem, val, 20);
                plugin->fps = atof(val);
                if (!plugin->fps)
                {
                    MessageBox(hDlg, "Invalid frame rate specified", "Validation error", MB_OK | MB_ICONEXCLAMATION);
                    return TRUE;
                }

                EndDialog(hDlg, TRUE);
                return TRUE;
            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return FALSE;
        }
    }

    return FALSE;

}

//---------------------------------------------------------------------
bool MilkshapePlugin::showOptions(void)
{
    HINSTANCE hInst = GetModuleHandle("msOGREExporter.dll");
    plugin = this;
    exportMesh = true;
    exportSkeleton = false;

	return (DialogBox(hInst, MAKEINTRESOURCE(IDD_OPTIONS), NULL, DlgProc) == TRUE);






}

void MilkshapePlugin::doExportMesh(msModel* pModel)
{


    // Create singletons
    Ogre::SkeletonManager skelMgr;
    Ogre::DefaultHardwareBufferManager defHWBufMgr;
	Ogre::LogManager& logMgr = Ogre::LogManager::getSingleton();
	Ogre::MeshManager meshMgr;


    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));

    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "mesh";
    char szFilter[128] = "OGRE .mesh Files (*.mesh)\0*.mesh\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Export to OGRE Mesh";

    if (!::GetSaveFileName (&ofn))
        return /*0*/;

    logMgr.logMessage("Creating Mesh object...");
    Ogre::MeshPtr ogreMesh = Ogre::MeshManager::getSingleton().create("export", 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    logMgr.logMessage("Mesh object created.");

    bool foundBoneAssignment = false;

    // No shared geometry
    int i;
	int wh, numbones;
	int intweight[3], intbones[3];
    size_t j;
    Ogre::Vector3 min, max, currpos;
    Ogre::Real maxSquaredRadius;
    bool first = true;
    for (i = 0; i < msModel_GetMeshCount (pModel); i++)
    {
        msMesh *pMesh = msModel_GetMeshAt (pModel, i);

        logMgr.logMessage("Creating SubMesh object...");
        Ogre::SubMesh* ogreSubMesh = ogreMesh->createSubMesh();
        logMgr.logMessage("SubMesh object created.");
        // Set material
        logMgr.logMessage("Getting SubMesh Material...");
        int matIdx = msMesh_GetMaterialIndex(pMesh);

        if (matIdx == -1)
        {
            // No material, use blank
            ogreSubMesh->setMaterialName("BaseWhite");
            logMgr.logMessage("No Material, using default 'BaseWhite'.");
        }
        else
        {

            msMaterial *pMat = msModel_GetMaterialAt(pModel, matIdx);
            ogreSubMesh->setMaterialName(pMat->szName);
            logMgr.logMessage("SubMesh Material Done.");
        }


        logMgr.logMessage("Setting up geometry...");
        // Set up mesh geometry
        ogreSubMesh->vertexData = new Ogre::VertexData();
        ogreSubMesh->vertexData->vertexCount = msMesh_GetVertexCount (pMesh);
        ogreSubMesh->vertexData->vertexStart = 0;
        Ogre::VertexBufferBinding* bind = ogreSubMesh->vertexData->vertexBufferBinding;
        Ogre::VertexDeclaration* decl = ogreSubMesh->vertexData->vertexDeclaration;
        // Always 1 texture layer, 2D coords
        #define POSITION_BINDING 0
        #define NORMAL_BINDING 1
        #define TEXCOORD_BINDING 2
        decl->addElement(POSITION_BINDING, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        decl->addElement(NORMAL_BINDING, 0, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
        decl->addElement(TEXCOORD_BINDING, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
        // Create buffers
        Ogre::HardwareVertexBufferSharedPtr pbuf = Ogre::HardwareBufferManager::getSingleton().
            createVertexBuffer(decl->getVertexSize(POSITION_BINDING), ogreSubMesh->vertexData->vertexCount,
                Ogre::HardwareBuffer::HBU_DYNAMIC, false);
        Ogre::HardwareVertexBufferSharedPtr nbuf = Ogre::HardwareBufferManager::getSingleton().
            createVertexBuffer(decl->getVertexSize(NORMAL_BINDING), ogreSubMesh->vertexData->vertexCount,
                Ogre::HardwareBuffer::HBU_DYNAMIC, false);
        Ogre::HardwareVertexBufferSharedPtr tbuf = Ogre::HardwareBufferManager::getSingleton().
            createVertexBuffer(decl->getVertexSize(TEXCOORD_BINDING), ogreSubMesh->vertexData->vertexCount,
                Ogre::HardwareBuffer::HBU_DYNAMIC, false);
        bind->setBinding(POSITION_BINDING, pbuf);
        bind->setBinding(NORMAL_BINDING, nbuf);
        bind->setBinding(TEXCOORD_BINDING, tbuf);

        ogreSubMesh->useSharedVertices = false;

        float* pPos = static_cast<float*>(
            pbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

        logMgr.logMessage("Doing positions and texture coords...");
        for (j = 0; j < ogreSubMesh->vertexData->vertexCount; ++j)
        {
            logMgr.logMessage("Doing vertex " + Ogre::StringConverter::toString(j));
            msVertex *pVertex = msMesh_GetVertexAt (pMesh, (int)j);
			msVertexEx *pVertexEx=msMesh_GetVertexExAt(pMesh, (int)j);
            msVec3 Vertex;
            msVertex_GetVertex (pVertex, Vertex);

            *pPos++ = Vertex[0];
            *pPos++ = Vertex[1];
            *pPos++ = Vertex[2];
            // Deal with bounds
            currpos = Ogre::Vector3(Vertex[0], Vertex[1], Vertex[2]);
            if (first)
            {
                min = max = currpos;
                maxSquaredRadius = currpos.squaredLength();
                first = false;
            }
            else
            {
                min.makeFloor(currpos);
                max.makeCeil(currpos);
                maxSquaredRadius = std::max(maxSquaredRadius, currpos.squaredLength());
            }

            int boneIdx = msVertex_GetBoneIndex(pVertex);
            if (boneIdx != -1)
            {
				foundBoneAssignment = true;
				numbones = 1;
				intbones[0] = intbones[1] = intbones[2] = -1;
				intweight[0] = intweight[1] = intweight[2] = 0;
				for(wh = 0; wh < 3; ++wh) 
				{
					intbones[wh] = msVertexEx_GetBoneIndices(pVertexEx, wh);
					if(intbones[wh] == -1) 
						break;

					++numbones;
					intweight[wh] = msVertexEx_GetBoneWeights(pVertexEx, wh);

				} // for(k)
				Ogre::VertexBoneAssignment vertAssign;
				vertAssign.boneIndex = boneIdx;
				vertAssign.vertexIndex = (unsigned int)j;
				if(numbones == 1) 
				{
					vertAssign.weight = 1.0;
				} // single assignment
				else 
				{
					vertAssign.weight=(Ogre::Real)intweight[0]/100.0;
				}
				ogreSubMesh->addBoneAssignment(vertAssign);
				if(numbones > 1) 
				{
					// this somewhat contorted logic is because the first weight [0] matches to the bone assignment
					// located with pVertex. The next two weights [1][2] match up to the first two bones found
					// with pVertexEx [0][1]. The weight for the fourth bone, if present, is the unassigned weight
					for(wh = 0; wh < 3; wh++) 
					{
						boneIdx = intbones[wh];
						if(boneIdx == -1) 
							break;
						vertAssign.boneIndex = boneIdx;
						vertAssign.vertexIndex = (unsigned int)j;
						if(wh == 2) 
						{ 
							// fourth weight is 1.0-(sumoffirstthreeweights)
							vertAssign.weight = 1.0-(((Ogre::Real)intweight[0]/100.0)+
								((Ogre::Real)intweight[1]/100.0)+((Ogre::Real)intweight[2]/100.0));
						}
						else 
						{
							vertAssign.weight=(Ogre::Real)intweight[wh+1];
						}
						ogreSubMesh->addBoneAssignment(vertAssign);
					} // for(k)
				} // if(numbones)
			}

        }
        pbuf->unlock();

        float* pTex = static_cast<float*>(
            tbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        logMgr.logMessage("Doing uvs, normals and indexes (v2)...");

        // Aargh, Milkshape uses stupid separate normal indexes for the same vertex like 3DS
        // Normals aren't described per vertex but per triangle vertex index
        // Pain in the arse, we have to do vertex duplication again if normals differ at a vertex (non smooth)
        // WHY don't people realise this format is a pain for passing to 3D APIs in vertex buffers?
        float* pNorm = static_cast<float*>(
            nbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        ogreSubMesh->indexData->indexCount = msMesh_GetTriangleCount (pMesh) * 3;
        // Always use 16-bit buffers, Milkshape can't handle more anyway
        Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
            createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
            ogreSubMesh->indexData->indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        ogreSubMesh->indexData->indexBuffer = ibuf;
        unsigned short *pIdx = static_cast<unsigned short*>(
            ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        for (j = 0; j < ogreSubMesh->indexData->indexCount; j+=3)
        {
            msTriangle *pTriangle = msMesh_GetTriangleAt (pMesh, (int)j/3);
			msTriangleEx *pTriangleEx=msMesh_GetTriangleExAt(pMesh, (int)j/3);
            word nIndices[3];
            msTriangle_GetVertexIndices (pTriangle, nIndices);
            msVec3 Normal;
            msVec2 uv;
            int k, vertIdx;

            for (k = 0; k < 3; ++k)
            {
                vertIdx = nIndices[k];
                // Face index
                pIdx[j+k] = vertIdx;

                // Vertex normals
                // For the moment, ignore any discrepancies per vertex
				msTriangleEx_GetNormal(pTriangleEx, k, &Normal[0]);
				msTriangleEx_GetTexCoord(pTriangleEx, k, &uv[0]);
				pTex[(vertIdx*2)]=uv[0];
				pTex[(vertIdx*2)+1]=uv[1];
                pNorm[(vertIdx*3)] = Normal[0];
                pNorm[(vertIdx*3)+1] = Normal[1];
                pNorm[(vertIdx*3)+2] = Normal[2];
            }

        } // Faces
        nbuf->unlock();
        ibuf->unlock();
        tbuf->unlock();

        // Now use Ogre's ability to reorganise the vertex buffers the best way
        Ogre::VertexDeclaration* newDecl = 
            ogreSubMesh->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                foundBoneAssignment, false);
        Ogre::BufferUsageList bufferUsages;
        for (size_t u = 0; u <= newDecl->getMaxSource(); ++u)
            bufferUsages.push_back(Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        ogreSubMesh->vertexData->reorganiseBuffers(newDecl, bufferUsages);


        logMgr.logMessage("Geometry done.");
    } // SubMesh

    // Set bounds
    ogreMesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(maxSquaredRadius));
    ogreMesh->_setBounds(Ogre::AxisAlignedBox(min, max), false);


    // Keep hold of a Skeleton pointer for deletion later
    // Mesh uses Skeleton pointer for skeleton name
    Ogre::SkeletonPtr pSkel;

    if (exportSkeleton && foundBoneAssignment)
    {
        // export skeleton, also update mesh to point to it
        pSkel = doExportSkeleton(pModel, ogreMesh);
    }
    else if (!exportSkeleton && foundBoneAssignment)
    {
        // We've found bone assignments, but skeleton is not to be exported
        // Prompt the user to find the skeleton
        if (!locateSkeleton(ogreMesh))
            return;

    }

    // Export
    logMgr.logMessage("Creating MeshSerializer..");
    Ogre::MeshSerializer serializer;
    logMgr.logMessage("MeshSerializer created.");

    // Generate LODs if required
    if (generateLods)
    {
        // Build LOD depth list
        Ogre::Mesh::LodDistanceList distList;
        float depth = 0;
        for (unsigned short depthidx = 0; depthidx < numLods; ++depthidx)
        {
            depth += lodDepthIncrement;
            distList.push_back(depth);
        }

        ogreMesh->generateLodLevels(distList, lodReductionMethod, lodReductionAmount);
    }

    if (generateEdgeLists)
    {
        ogreMesh->buildEdgeList();
    }

    if (generateTangents)
    {
		unsigned short src, dest;
		ogreMesh->suggestTangentVectorBuildParams(tangentSemantic, src, dest);
		ogreMesh->buildTangentVectors(tangentSemantic, src, dest, tangentsSplitMirrored, tangentsSplitRotated, tangentsUseParity);
    }

    // Export
    Ogre::String msg;
	msg  = "Exporting mesh data to file '" + Ogre::String(szFile) + "'";
    logMgr.logMessage(msg);
    serializer.exportMesh(ogreMesh.getPointer(), szFile);
    logMgr.logMessage("Export successful");

    Ogre::MeshManager::getSingleton().remove(ogreMesh->getHandle());
    if (!pSkel.isNull())
        Ogre::SkeletonManager::getSingleton().remove(pSkel->getHandle());

	if (exportMaterials && msModel_GetMaterialCount(pModel) > 0)
	{
		doExportMaterials(pModel);
	}
}


Ogre::SkeletonPtr MilkshapePlugin::doExportSkeleton(msModel* pModel, Ogre::MeshPtr& mesh)
{
    Ogre::LogManager &logMgr = Ogre::LogManager::getSingleton();
    Ogre::String msg;

    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));

    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "skeleton";
    char szFilter[128] = "OGRE .skeleton Files (*.skeleton)\0*.skeleton\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Export to OGRE Skeleton";

    if (!::GetSaveFileName (&ofn))
        return Ogre::SkeletonPtr();

    // Strip off the path
    Ogre::String skelName = szFile;
    size_t lastSlash = skelName.find_last_of("\\");
    skelName = skelName.substr(lastSlash+1);

    // Set up
    logMgr.logMessage("Trying to create Skeleton object");
    Ogre::SkeletonPtr ogreskel = Ogre::SkeletonManager::getSingleton().create(skelName, 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    logMgr.logMessage("Skeleton object created");

    // Complete the details

    // Do the bones
    int numBones = msModel_GetBoneCount(pModel);
	msg = "Number of bones: " + Ogre::StringConverter::toString(numBones);
    logMgr.logMessage(msg);

    int i;
    // Create all the bones in turn
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);
        Ogre::Bone* ogrebone = ogreskel->createBone(bone->szName);

        msVec3 msBonePos, msBoneRot;
        msBone_GetPosition(bone, msBonePos);
        msBone_GetRotation(bone, msBoneRot);

        Ogre::Vector3 bonePos(msBonePos[0], msBonePos[1], msBonePos[2]);
        ogrebone->setPosition(bonePos);
        // Hmm, Milkshape has chosen a Euler angle representation of orientation which is not smart
        // Rotation Matrix or Quaternion would have been the smarter choice
        // Might we have Gimbal lock here? What order are these 3 angles supposed to be applied?
        // Grr, we'll try our best anyway...
        Ogre::Quaternion qx, qy, qz, qfinal;
        qx.FromAngleAxis(Ogre::Radian(msBoneRot[0]), Ogre::Vector3::UNIT_X);
        qy.FromAngleAxis(Ogre::Radian(msBoneRot[1]), Ogre::Vector3::UNIT_Y);
        qz.FromAngleAxis(Ogre::Radian(msBoneRot[2]), Ogre::Vector3::UNIT_Z);

        // Assume rotate by x then y then z
        qfinal = qz * qy * qx;
        ogrebone->setOrientation(qfinal);

		Ogre::LogManager::getSingleton().stream()
			<< "Bone #" << i << ": " <<
            "Name='" << bone->szName << "' " <<
            "Position: " << bonePos << " " <<
            "Ms3d Rotation: {" << msBoneRot[0] << ", " << msBoneRot[1] << ", " << msBoneRot[2] << "} " <<
            "Orientation: " << qfinal;


    }
    // Now we've created all the bones, link them up
    logMgr.logMessage("Establishing bone hierarchy..");
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);

        if (strlen(bone->szParentName) == 0)
        {
            // Root bone
            msg = "Root bone detected: Name='" + Ogre::String(bone->szName) + "' Index=" 
				+ Ogre::StringConverter::toString(i);
            logMgr.logMessage(msg);
        }
        else
        {
            Ogre::Bone* ogrechild = ogreskel->getBone(bone->szName);
            Ogre::Bone* ogreparent = ogreskel->getBone(bone->szParentName);

            if (ogrechild == 0)
            {
                msg = "Error: could not locate child bone '" +
					Ogre::String(bone->szName) + "'";
                logMgr.logMessage(msg);
                continue;
            }
            if (ogreparent == 0)
            {
                msg = "Error: could not locate parent bone '"
					+ Ogre::String(bone->szParentName) + "'";
                logMgr.logMessage(msg);
                continue;
            }
            // Make child
            ogreparent->addChild(ogrechild);
        }


    }
    logMgr.logMessage("Bone hierarchy established.");

    // Create the Animation(s)
    doExportAnimations(pModel, ogreskel);



    // Create skeleton serializer & export
    Ogre::SkeletonSerializer serializer;
    msg = "Exporting skeleton to " + Ogre::String(szFile);
    logMgr.logMessage(msg);
    serializer.exportSkeleton(ogreskel.getPointer(), szFile);
    logMgr.logMessage("Skeleton exported");


    msg = "Linking mesh to skeleton file '" + skelName + "'";
    Ogre::LogManager::getSingleton().logMessage(msg);

    mesh->_notifySkeleton(ogreskel);

    return ogreskel;

}

bool MilkshapePlugin::locateSkeleton(Ogre::MeshPtr& mesh)
{
    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));

    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "skeleton";
    char szFilter[128] = "OGRE .skeleton Files (*.skeleton)\0*.skeleton\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Locate OGRE Skeleton (since you're not exporting it)";

    if (!::GetOpenFileName (&ofn))
        return false;

    // Strip off the path
    Ogre::String skelName = szFile;
    size_t lastSlash = skelName.find_last_of("\\");
    skelName = skelName.substr(lastSlash+1);

    Ogre::String msg = "Linking mesh to skeleton file '" + skelName + "'";
    Ogre::LogManager::getSingleton().logMessage(msg);

    // Create a dummy skeleton for Mesh to link to (saves it trying to load it)
    Ogre::SkeletonPtr pSkel = Ogre::SkeletonManager::getSingleton().create(skelName, 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::LogManager::getSingleton().logMessage("Dummy Skeleton object created for link.");

    mesh->_notifySkeleton(pSkel);

    return true;

}

struct SplitAnimationStruct
{
    int start;
    int end;
    Ogre::String name;
};

void MilkshapePlugin::doExportMaterials(msModel* pModel)
{
	Ogre::LogManager& logMgr = Ogre::LogManager::getSingleton();
	Ogre::MaterialManager matMgrSgl;
	Ogre::String msg;

    matMgrSgl.initialise();

	int numMaterials = msModel_GetMaterialCount(pModel);
	msg = "Number of materials: " + Ogre::StringConverter::toString(numMaterials);
	logMgr.logMessage(msg);

	OPENFILENAME ofn;
	memset (&ofn, 0, sizeof (OPENFILENAME));

	char szFile[MS_MAX_PATH];
	char szFileTitle[MS_MAX_PATH];
	char szDefExt[32] = "material";
	char szFilter[128] = "OGRE .material Files (*.material)\0*.material\0All Files (*.*)\0*.*\0\0";
	szFile[0] = '\0';
	szFileTitle[0] = '\0';

	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.lpstrDefExt = szDefExt;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MS_MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MS_MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrTitle = "Export to OGRE Material";

	if (!::GetSaveFileName (&ofn))
		return;

	// Strip off the path
	Ogre::String matName = szFile;
	size_t lastSlash = matName.find_last_of("\\");
	matName = matName.substr(lastSlash+1);

	// Set up
	logMgr.logMessage("Trying to create Material object");

	Ogre::MaterialSerializer matSer;

	for (int i = 0; i < numMaterials; ++i)
	{
		msMaterial *mat = msModel_GetMaterialAt(pModel, i);

		msg = "Creating material " + Ogre::String(mat->szName);
		logMgr.logMessage(msg);
        Ogre::MaterialPtr ogremat = matMgrSgl.create(mat->szName, 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		logMgr.logMessage("Created.");

		ogremat->setAmbient(msVec4ToColourValue(mat->Ambient));
		ogremat->setDiffuse(msVec4ToColourValue(mat->Diffuse));
		ogremat->setSpecular(msVec4ToColourValue(mat->Specular));
		ogremat->setShininess(mat->fShininess);

		if (0 < strlen(mat->szDiffuseTexture))
			ogremat->getTechnique(0)->getPass(0)->createTextureUnitState(mat->szDiffuseTexture);

        if (0 < strlen(mat->szAlphaTexture))
			ogremat->getTechnique(0)->getPass(0)->createTextureUnitState(mat->szAlphaTexture);


		matSer.queueForExport(ogremat);
	}

	msg = "Exporting materials to " + matName;
	logMgr.logMessage(msg);
	matSer.exportQueued(matName);
}

Ogre::ColourValue MilkshapePlugin::msVec4ToColourValue(float prop[4])
{
	Ogre::ColourValue colour;
	colour.r = prop[0];
	colour.g = prop[1];
	colour.b = prop[2];
	colour.a = prop[3];

	return colour;
}

void MilkshapePlugin::doExportAnimations(msModel* pModel, Ogre::SkeletonPtr& ogreskel)
{

    Ogre::LogManager& logMgr = Ogre::LogManager::getSingleton();
    std::vector<SplitAnimationStruct> splitInfo;
    Ogre::String msg;

    int numFrames = msModel_GetTotalFrames(pModel);
	msg = "Number of frames: " + Ogre::StringConverter::toString(numFrames);
    logMgr.logMessage(msg);

    if (splitAnimations)
    {
        // Explain
        msg = "You have chosen to create multiple discrete animations by splitting up the frames in "
			"the animation sequence. In order to do this, you must supply a simple text file "
            "describing the separate animations, which has a single line per animation in the format: \n\n" 
            "startFrame,endFrame,animationName\n\nFor example: \n\n"
            "1,20,Walk\n21,35,Run\n36,40,Shoot\n\n" 
            "..creates 3 separate animations (the frame numbers are inclusive)."
			"You must browse to this file in the next dialog.";
        MessageBox(0,msg.c_str(), "Splitting Animations",MB_ICONINFORMATION | MB_OK);
        // Prompt for a file which contains animation splitting info
        OPENFILENAME ofn;
        memset (&ofn, 0, sizeof (OPENFILENAME));
        
        char szFile[MS_MAX_PATH];
        char szFileTitle[MS_MAX_PATH];
        char szDefExt[32] = "skeleton";
        char szFilter[128] = "All Files (*.*)\0*.*\0\0";
        szFile[0] = '\0';
        szFileTitle[0] = '\0';

        ofn.lStructSize = sizeof (OPENFILENAME);
        ofn.lpstrDefExt = szDefExt;
        ofn.lpstrFilter = szFilter;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MS_MAX_PATH;
        ofn.lpstrFileTitle = szFileTitle;
        ofn.nMaxFileTitle = MS_MAX_PATH;
        ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        ofn.lpstrTitle = "Open animation split configuration file";

        if (!::GetOpenFileName (&ofn))
        {
            msg = "Splitting aborted, generating a single animation called 'Default'";
            MessageBox(0, msg.c_str(), "Info", MB_OK | MB_ICONWARNING);
            SplitAnimationStruct split;
            split.start = 1;
            split.end = numFrames;
            split.name = "Default";
            splitInfo.push_back(split);
        }
        else
        {
            // Read file
            Ogre::String sline;
            char line[256];
            SplitAnimationStruct newSplit;

            std::ifstream istr;
            istr.open(szFile);

            while (!istr.eof())
            {
                istr.getline(line, 256);
                sline = line;

                // Ignore blanks & comments
                if (sline == "" || sline.substr(0,2) == "//")
                    continue;

                // Split on ','
				std::vector<Ogre::String> svec = Ogre::StringUtil::split(line, ",\n");

                // Basic validation on number of elements
                if (svec.size() != 3)
                {
                    MessageBox(0, "Warning: corrupt animation details in file. You should look into this. ",
                        "Corrupt animations file", MB_ICONWARNING | MB_OK);
                    continue;
                }
                // Remove any embedded spaces
				Ogre::StringUtil::trim(svec[0]);
                Ogre::StringUtil::trim(svec[1]);
                Ogre::StringUtil::trim(svec[2]);
                // Create split info
                newSplit.start = atoi(svec[0].c_str());
                newSplit.end = atoi(svec[1].c_str());
                newSplit.name = svec[2];
                splitInfo.push_back(newSplit);

            }


        }

    }
    else
    {
        // No splitting
        SplitAnimationStruct split;
        split.start = 1;
        split.end = numFrames;
        split.name = "Default";
        splitInfo.push_back(split);
    }

    // Get animation length
    // Map frames -> seconds, this can be changed in speed of animation anyway



    int numBones = msModel_GetBoneCount(pModel);
    unsigned int frameTime;
    float realTime;

    std::vector<SplitAnimationStruct>::iterator animsIt;
    for (animsIt = splitInfo.begin(); animsIt != splitInfo.end(); ++animsIt)
    {
        SplitAnimationStruct& currSplit = *animsIt;

        // Create animation
        frameTime = currSplit.end - currSplit.start;
        realTime = frameTime / fps;
		Ogre::LogManager::getSingleton().stream()
			<< "Trying to create Animation object for animation "
			<<  currSplit.name << " For Frames " << currSplit.start << " to "
            << currSplit.end << " inclusive. ";

		Ogre::LogManager::getSingleton().stream()
			<< "Frame time = "
			<< frameTime << ", Seconds = " << realTime;

        Ogre::Animation *ogreanim =
            ogreskel->createAnimation(currSplit.name, realTime);
        logMgr.logMessage("Animation object created.");

        int i;
        // Create all the animation tracks
        for (i = 0; i < numBones; ++i)
        {

            msBone* bone = msModel_GetBoneAt(pModel, i);
            Ogre::Bone* ogrebone = ogreskel->getBone(bone->szName);

            // Create animation tracks
			msg = "Creating AnimationTrack for bone " + Ogre::StringConverter::toString(i);
            logMgr.logMessage(msg);

            Ogre::NodeAnimationTrack *ogretrack = ogreanim->createNodeTrack(i, ogrebone);
            logMgr.logMessage("Animation track created.");

            // OGRE uses keyframes which are both position and rotation
            // Milkshape separates them, but never seems to use the ability to
            // have a different # of pos & rot keys

            int numKeys = msBone_GetRotationKeyCount(bone);

            msg = "Number of keyframes: " + Ogre::StringConverter::toString(numKeys);
            logMgr.logMessage(msg);

            int currKeyIdx;
            msPositionKey* currPosKey;
            msRotationKey* currRotKey;
            for (currKeyIdx = 0; currKeyIdx < numKeys; ++currKeyIdx )
            {
                currPosKey = msBone_GetPositionKeyAt(bone, currKeyIdx);
                currRotKey = msBone_GetRotationKeyAt(bone, currKeyIdx);

                // Make sure keyframe is in current time frame (for splitting)
                if (currRotKey->fTime >= currSplit.start && currRotKey->fTime <= currSplit.end)
                {

                    msg = "Creating KeyFrame #" + Ogre::StringConverter::toString(currKeyIdx)
                        + " for bone #" + Ogre::StringConverter::toString(i);
                    logMgr.logMessage(msg);
                    // Create keyframe
                    // Adjust for start time, and for the fact that frames are numbered from 1
                    frameTime = currRotKey->fTime - currSplit.start;
                    realTime = frameTime / fps;
                    Ogre::TransformKeyFrame *ogrekey = ogretrack->createNodeKeyFrame(realTime);
                    logMgr.logMessage("KeyFrame created");

					Ogre::Vector3 kfPos;
					// Imported milkshape animations may not have positions 
					// for all rotation keys
					if ( currKeyIdx < bone->nNumPositionKeys ) {
						kfPos.x = currPosKey->Position[0];
						kfPos.y = currPosKey->Position[1];
						kfPos.z = currPosKey->Position[2];
					}
					else {
						kfPos.x = bone->Position[0];
						kfPos.y = bone->Position[1];
						kfPos.z = bone->Position[2];
					}
                    Ogre::Quaternion qx, qy, qz, kfQ;

					// Milkshape translations are local to own orientation, not parent
					kfPos = ogrebone->getOrientation() * kfPos;

                    ogrekey->setTranslate(kfPos);
                    qx.FromAngleAxis(Ogre::Radian(currRotKey->Rotation[0]), Ogre::Vector3::UNIT_X);
                    qy.FromAngleAxis(Ogre::Radian(currRotKey->Rotation[1]), Ogre::Vector3::UNIT_Y);
                    qz.FromAngleAxis(Ogre::Radian(currRotKey->Rotation[2]), Ogre::Vector3::UNIT_Z);
                    kfQ = qz * qy * qx;
                    ogrekey->setRotation(kfQ);

					Ogre::LogManager::getSingleton().stream()
						<< "KeyFrame details: Adjusted Frame Time=" << frameTime
						<< " Seconds: " << realTime << " Position=" << kfPos << " " 
						<< "Ms3d Rotation= {" << currRotKey->Rotation[0] << ", " 
						<< currRotKey->Rotation[1] << ", " << currRotKey->Rotation[2] 
						<< "} " << "Orientation=" << kfQ;
                } // keyframe creation

            } // keys
        } //Bones
    } // Animations




}

