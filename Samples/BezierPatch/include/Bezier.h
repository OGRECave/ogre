/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/


/**
    \file 
        Bezier.h
    \brief
        Specialisation of OGRE's framework application to show off
        the bezier patch support.
*/

#include "ExampleApplication.h"

// Hack struct for test
PatchMeshPtr patch;
Pass* patchPass;

// Event handler to add ability to alter subdivision
class BezierListener : public ExampleFrameListener
{
protected:
public:
    BezierListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
        
    }

    bool frameRenderingQueued(const FrameEvent& evt)
    {
	if( ExampleFrameListener::frameRenderingQueued(evt) == false )
		return false;

        static Real timeLapse = 0.0f;
        static Real factor = 0.0;
        static bool wireframe = 0;


        timeLapse += evt.timeSinceLastFrame;

        // Prgressively grow the patch
        if (timeLapse > 1.0f)
        {
            factor += 0.2;

            if (factor > 1.0f) 
            {
                wireframe = !wireframe;
                //mCamera->setPolygonMode(wireframe ? PM_WIREFRAME : PM_SOLID);
                patchPass->setPolygonMode(wireframe ? PM_WIREFRAME : PM_SOLID);
                factor = 0.0f;

            }

            patch->setSubdivision(factor);
            mDebugText = "Bezier subdivision factor: " + StringConverter::toString(factor);
            timeLapse = 0.0f;

        }

        // Call default
        return true;
    }
};

       
class BezierApplication : public ExampleApplication
{
protected:
    VertexDeclaration* patchDecl;
    float* patchCtlPoints;

public:
    BezierApplication() : patchDecl(NULL), patchCtlPoints(NULL) { }
    ~BezierApplication()
    {
        if (patchCtlPoints)
            delete [] patchCtlPoints;

        // patch vertex declaration will be deleted automatically
    }

protected:

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    #pragma pack(push)
    #pragma pack(1)
#endif
    struct PatchVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
    #pragma pack(pop)
#endif

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-0.5, -0.5, 0);

        // Create patch
        patchDecl = HardwareBufferManager::getSingleton().createVertexDeclaration();
        patchDecl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        patchDecl->addElement(0, sizeof(float)*3, VET_FLOAT3, VES_NORMAL);
        patchDecl->addElement(0, sizeof(float)*6, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

        // Make a 3x3 patch for test
        patchCtlPoints = (float*)( new PatchVertex[9] );

        // Patch data
        PatchVertex *pVert = (PatchVertex*)patchCtlPoints;

        pVert->x = -500.0; pVert->y = 200.0; pVert->z = -500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = -750.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 1000.0; pVert->z = -500.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.0;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 0.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.5;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 0.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.5;
        pVert++;
        pVert->x = 500.0; pVert->y = -50.0; pVert->z = 0.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.5;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 1.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 500.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 1.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 200.0; pVert->z = 800.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 1.0;
        pVert++;


        patch = MeshManager::getSingleton().createBezierPatch(
            "Bezier1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            patchCtlPoints, patchDecl, 
            3, 3, 5, 5, PatchSurface::VS_BOTH);

        // Start patch at 0 detail
        patch->setSubdivision(0.0f);
        // Create entity based on patch
        Entity* patchEntity = mSceneMgr->createEntity("Entity1", "Bezier1");

        MaterialPtr pMat = MaterialManager::getSingleton().create("TextMat", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        pMat->getTechnique(0)->getPass(0)->createTextureUnitState( "BumpyMetal.jpg" );
        patchEntity->setMaterialName("TextMat");
		patchPass = pMat->getTechnique(0)->getPass(0);

        // Attach the entity to the root of the scene
        mSceneMgr->getRootSceneNode()->attachObject(patchEntity);

        mCamera->setPosition(500,500, 1500);
        mCamera->lookAt(0,200,-300);

    }
    void destroyScene(void)
    {
        // free up the pointer before we shut down OGRE
		mSceneMgr->clearScene();
		MeshManager::getSingleton().removeAll();
		patch.setNull();
    }
	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new BezierListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }

};
