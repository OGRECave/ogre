#ifndef __BillboardChain_H__
#define __BillboardChain_H__

#include "SdkSample.h"
#include "OgreBillboardChain.h"
#include <iostream>

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_BillboardChain : public SdkSample
{
public:

    Sample_BillboardChain()
    {
        std::cerr << "*** Created ***" << std::endl;
        mInfo["Title"] = "Billboard Chain";
        mInfo["Description"] = "Illustrate BillboardChain bug";
        mInfo["Category"] = "Geometry";
    }

protected:

    void setupContent()
    {
        // setup some basic lighting for our scene
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->getRootSceneNode()
            ->createChildSceneNode(Vector3(100, 100, 100))
            ->attachObject(mSceneMgr->createLight());

        auto mat = Ogre::MaterialManager::getSingleton().create("BillboardChainMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        mat->setReceiveShadows(false);
        auto t = mat->getTechnique(0);
        t->setLightingEnabled(false);
        t->setSceneBlending(Ogre::SBT_REPLACE);
        t->setDepthWriteEnabled(true);

        auto chain = mSceneMgr->createBillboardChain("chain");
        chain->setMaterialName(mat->getName(), mat->getGroup());
        mSceneMgr->getRootSceneNode()->attachObject(chain);

        BillboardChain::Element e;
        e.position = Vector3(0.0, 0.0, 0.0);
        e.width = 0.1;
        e.colour = ColourValue::Red;
        chain->addChainElement(0, e);
        e.position = Vector3(1.0, 1.0, 1.0);
        chain->addChainElement(0, e);

        // use an orbit style camera
        mCameraMan->setStyle(CS_ORBIT);
        mCameraMan->setYawPitchDist(Degree(0), Degree(30), 10);

        mTrayMgr->showCursor();
    }

    void cleanupContent()
    {
    }

};

#endif
