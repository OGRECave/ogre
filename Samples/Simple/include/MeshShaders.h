/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
#include "SdkSample.h"
#include "SamplePlugin.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_MeshShaders : public SdkSample
{
 public:
    Sample_MeshShaders()
    {
        mInfo["Title"] = "Mesh Shaders";
        mInfo["Description"] = "An example of using mesh shaders";
        mInfo["Thumbnail"] = "thumb_atomicc.png";
        mInfo["Category"] = "ShaderFeatures";
    }

    void testCapabilities(const RenderSystemCapabilities* caps) override
    {
        requireMaterial("Example/MeshShader");
    }

    void setupContent() override
    {
        mViewport->setBackgroundColour(ColourValue(0.3, 0.3, 0.3));

        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(0), 1000);

        auto rect = mSceneMgr->createEntity(SceneManager::PT_PLANE);
        rect->getMesh()->_setBounds(AxisAlignedBox::BOX_INFINITE);

        MaterialPtr mat = MaterialManager::getSingleton().getByName("Example/MeshShader");
        rect->setMaterial(mat);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(rect);
    }
};
