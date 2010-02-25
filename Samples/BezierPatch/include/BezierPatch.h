#ifndef __BezierPatch_H__
#define __BezierPatch_H__

#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_BezierPatch : public SdkSample
{
public:

	Sample_BezierPatch()
	{
		mInfo["Title"] = "Bezier Patch";
		mInfo["Description"] = "A demonstration of the Bezier patch support.";
		mInfo["Thumbnail"] = "thumb_bezier.png";
		mInfo["Category"] = "Geometry";
	}

	void checkBoxToggled(CheckBox* box)
	{
		mPatchPass->setPolygonMode(box->isChecked() ? PM_WIREFRAME : PM_SOLID);
	}

	void sliderMoved(Slider* slider)
	{
		mPatch->setSubdivision(slider->getValue());
	}

protected:

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#	pragma pack(push, 1)
#endif
    struct PatchVertex
	{
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#	pragma pack(pop)
#endif

	void setupContent()
	{
		// setup some basic lighting for our scene
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->createLight()->setPosition(100, 100, 100);

		// define the control point vertices for our patch
		PatchVertex verts[9] =
		{
			{-50, -35, -50, -0.5, 0.5, 0.0, 0.0, 0.0},
			{  0,   0, -50,  0.0, 0.5, 0.0, 0.5, 0.0},
			{ 50,  35, -50,  0.5, 0.5, 0.0, 1.0, 0.0},
			{-50,   0,   0, -0.5, 0.5, 0.0, 0.0, 0.5},
			{  0,   0,   0,  0.0, 0.5, 0.0, 0.5, 0.5},
			{ 50,   0,   0,  0.5, 0.5, 0.0, 1.0, 0.5},
			{-50,  35,  50, -0.5, 0.5, 0.0, 0.0, 1.0},
			{  0,   0,  50,  0.0, 0.5, 0.0, 0.5, 1.0},
			{ 50, -35,  50,  0.5, 0.5, 0.0, 1.0, 1.0}
		};

		// specify a vertex format declaration for our patch: 3 floats for position, 3 floats for normal, 2 floats for UV
        VertexDeclaration* decl = HardwareBufferManager::getSingleton().createVertexDeclaration();
        decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        decl->addElement(0, sizeof(float) * 3, VET_FLOAT3, VES_NORMAL);
        decl->addElement(0, sizeof(float) * 6, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

		// create a patch mesh using vertices and declaration
        mPatch = MeshManager::getSingleton().createBezierPatch("patch",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, (float*)verts, decl, 3, 3, 5, 5, PatchSurface::VS_BOTH);

        mPatch->setSubdivision(0);   // start at 0 detail

		// create a patch entity from the mesh, give it a material, and attach it to the origin
        Entity* ent = mSceneMgr->createEntity("Patch", "patch");
		ent->setMaterialName("Examples/BumpyMetal");
        mSceneMgr->getRootSceneNode()->attachObject(ent);

		// save the main pass of the material so we can toggle wireframe on it
		mPatchPass = ent->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0);

		// use an orbit style camera
		mCameraMan->setStyle(CS_ORBIT);
		mCameraMan->setYawPitchDist(Degree(0), Degree(30), 250);

		mTrayMgr->showCursor();

		// create slider to adjust detail and checkbox to toggle wireframe
		mTrayMgr->createThickSlider(TL_TOPLEFT, "Detail", "Detail", 120, 44, 0, 1, 6);
		mTrayMgr->createCheckBox(TL_TOPLEFT, "Wireframe", "Wireframe", 120);
	}

    void cleanupContent()
    {
		mPatchPass->setPolygonMode(PM_SOLID);
		MeshManager::getSingleton().remove(mPatch->getHandle());
    }

	PatchMeshPtr mPatch;
	Pass* mPatchPass;
};

#endif
