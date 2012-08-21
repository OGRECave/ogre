#include "SamplePlugin.h"
#include "VolumeTerrain.h"

#include "OgreVolumeCSGSource.h"
#include "OgreVolumeCacheSource.h"
#include "OgreVolumeTextureSource.h"
#include "OgreVolumeIsoSurface.h"
#include "OgreVolumeUtils.h"

using namespace Ogre;
using namespace OgreBites;
using namespace Ogre::Volume;

void Sample_VolumeTerrain::setupMCDisplay(size_t i)
{
    LogManager::getSingleton().stream() << "MC index " << i;

    if (mMCDisplay)
    {
        mSceneMgr->getRootSceneNode()->removeChild(mMCDisplay);
    }
    
    Vector3 corners[8] = {
        Vector3((Real)0.0, (Real)0.0, (Real)0.0),
        Vector3((Real)10.0, (Real)0.0, (Real)0.0),
        Vector3((Real)10.0, (Real)0.0, (Real)10.0),
        Vector3((Real)0.0, (Real)0.0, (Real)10.0),
        Vector3((Real)0.0, (Real)10.0, (Real)0.0),
        Vector3((Real)10.0, (Real)10.0, (Real)0.0),
        Vector3((Real)10.0, (Real)10.0, (Real)10.0),
        Vector3((Real)0.0, (Real)10.0, (Real)10.0)
    };

    Vector4 values[8] = {
        (i & 1) ? Vector4(-1, -1, -1, (Real)1.0) : Vector4(-1, -1, -1, (Real)-1.0),
        (i & 2) ? Vector4(1, -1, -1, (Real)1.0) : Vector4(1, -1, -1, (Real)-1.0),
        (i & 4) ? Vector4(1, -1, 1, (Real)1.0) : Vector4(1, -1, 1, (Real)-1.0),
        (i & 8) ? Vector4(-1, -1, 1, (Real)1.0) : Vector4(-1, -1, 1, (Real)-1.0),
        (i & 16) ? Vector4(-1, 1, -1, (Real)1.0) : Vector4(-1, 1, -1, (Real)-1.0),
        (i & 32) ? Vector4(1, 1, -1, (Real)1.0) : Vector4(1, 1, -1, (Real)-1.0),
        (i & 64) ? Vector4(1, 1, 1, (Real)1.0) : Vector4(1, 1, 1, (Real)-1.0),
        (i & 128) ? Vector4(-1, 1, 1, (Real)1.0) : Vector4(-1, -1, 1, (Real)-1.0)
    };

    String names[8] = {
        "MCSphere0",
        "MCSphere1",
        "MCSphere2",
        "MCSphere3",
        "MCSphere4",
        "MCSphere5",
        "MCSphere6",
        "MCSphere7"
    };


    mMCDisplay = mSceneMgr->getRootSceneNode()->createChildSceneNode();

    // Spheres
    size_t spheresCount = 0;
    for (size_t corner = 0; corner < 8; ++corner)
    {
        if (values[corner].w != (Real)-1.0)
        {
            spheresCount++;
            SceneNode *n = mMCDisplay->createChildSceneNode();
            n->translate(corners[corner]);
            n->scale(Vector3((Real)0.005));
            mSceneMgr->destroyEntity(names[corner]);
            Entity * mcSphere = mSceneMgr->createEntity(names[corner], "sphere.mesh");
            mcSphere->setMaterial(MaterialManager::getSingleton().getByName("justWhite"));
            n->attachObject(mcSphere);
        }
    }

    // Cube lines
    mSceneMgr->destroyEntity("MCCube");
    MeshManager::getSingleton().remove("MCCubeMesh");
    ManualObject* manual = mSceneMgr->createManualObject();
    manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);
    size_t baseIndex = 0;
    Utils::addCubeToManualObject(manual, corners[0], corners[1], corners[2], corners[3],
        corners[4], corners[5], corners[6], corners[7], baseIndex);
    manual->end();
    manual->convertToMesh("MCCubeMesh");
    Entity *cube = mSceneMgr->createEntity("MCCube", "MCCubeMesh");
    mMCDisplay->attachObject(cube);
    
    // MC triangles
    if (spheresCount != 0 && spheresCount != 8)
    {
        IsoSurface is(0);
        MeshBuilder mb;
        is.addMarchingCubesTriangles(corners, values, &mb);
        mSceneMgr->destroyEntity("MCTriangles");
        Entity *mc = mb.generateWithManualObject(mSceneMgr, "MCTriangles", "yellowWire");
        mMCDisplay->attachObject(mc);
    }
}

class CallbackTest : public MeshBuilderCallback
{
public:
    virtual void getTriangles(const VecVertex &vertices, const VecIndices &indices)
    {
        LogManager::getSingleton().stream() << "Triangles: " << (indices.size() / 3);
    }
};

void Sample_VolumeTerrain::setupContent(void)
{
    setupControls();
    
    mcConfig = 0;
    
    /*
    // Some kind of spheres plane
    CSGSphereSource *spheres[10][10];
    CSGUnionSource *unions[99];
    Source *prev;
    Real radius = (Real)5.0;
    size_t i = 0;
    for (size_t z = 0; z < 10; ++z)
    {
        for (size_t x = 0; x < 10; ++x)
        {
            spheres[z][x] = new CSGSphereSource(radius, Vector3(x * radius, (Real)0.0, z * radius));
            if (x > 0 || z > 0)
            {
                unions[i] = new CSGUnionSource(prev, spheres[z][x]);
                prev = unions[i];
                i++;
            }
            else 
            {
                prev = spheres[z][x];
            }
        }
    }
    Source *src = unions[98];
    Vector3 to((Real)45.0, (Real)45.0, (Real)45.0);
    */
    // Four big spheres
    /*CSGSphereSource a((Real)10.0, Vector3((Real)(22.5 - 7.0), (Real)22.5, (Real)22.5));
    CSGSphereSource b((Real)10.0, Vector3((Real)(22.5 + 7.0), (Real)22.5, (Real)22.5));
    CSGUnionSource unionSrc(&a, &b);
    Source *src = &unionSrc;
    Vector3 to((Real)45.0, (Real)45.0, (Real)45.0);*/

    /*CSGSphereSource c((Real)10.0, Vector3((Real)(22.5 - 7.0), (Real)22.5, (Real)22.5 + 7.0));
    CSGSphereSource d((Real)10.0, Vector3((Real)(22.5 + 7.0), (Real)22.5, (Real)22.5 + 7.0));
    CSGUnionSource unionSrcA(&a, &b);
    CSGUnionSource unionSrcB(&c, &d);
    CSGUnionSource unionSrc(&unionSrcA, &unionSrcB);
    Source *src = &unionSrc;
    Vector3 to((Real)45.0, (Real)45.0, (Real)45.0);*/

    // Just one big sphere
    /*CSGSphereSource sphere((Real)15.0, Vector3((Real)(21.0), (Real)21.0, (Real)21.0));
    Real size = (Real)42.0;
    Vector3 to(size);
    Source *src = &sphere;*/
    
    Timer t;
    Real size = (Real)256.0;
    /*//size = (Real)64.0;
    TextureSource textureSource("bigVolumeTerrain1.dds", size, size, size, true, false, true);
    LogManager::getSingleton().stream() << "Loaded volume data in " << t.getMilliseconds() << "ms";;
    //CSGScaleSource scale(&textureSource, (Real)4.0);
    CacheSource cache(&textureSource);
    //Source *src = &textureSource;
    //Source *src = &cache;
    //Source *src = &scale;*/
    Vector3 to(size);

    // Add some spheres to this
    /*CSGSphereSource sphere2((Real)5.0, Vector3((Real)(103.0), (Real)146.0, (Real)86.0));
    CSGSphereSource sphere3((Real)5.0, Vector3((Real)(105.0), (Real)147.5, (Real)84.5));
    CSGSphereSource sphere4((Real)5.0, Vector3((Real)(109.5), (Real)149.0, (Real)84.0));
    CSGSphereSource sphere6((Real)5.0, Vector3((Real)(112.0), (Real)150.0, (Real)83.0));
    CSGSphereSource sphere7((Real)5.0, Vector3((Real)(114.5), (Real)151.0, (Real)82.5));
    CSGSphereSource sphere9((Real)5.0, Vector3((Real)(119.5), (Real)152.0, (Real)80.0));
    CSGSphereSource sphere8((Real)5.0, Vector3((Real)(124.5), (Real)152.0, (Real)78.0));
    CSGSphereSource sphere5((Real)5.0, Vector3((Real)(130.0), (Real)151.0, (Real)76.0));
    CSGSphereSource sphere1((Real)5.0, Vector3((Real)(133.0), (Real)150.0, (Real)75.0));
    CSGUnionSource union1(&sphere1, &sphere2);
    CSGUnionSource union2(&union1, &sphere3);
    CSGUnionSource union3(&union2, &sphere4);
    CSGUnionSource union4(&union3, &sphere5);
    CSGUnionSource union5(&union4, &sphere6);
    CSGUnionSource union6(&union5, &sphere7);
    CSGUnionSource union7(&union6, &sphere8);
    CSGUnionSource union8(&union7, &sphere9);

    CSGUnionSource unionWithTerrain(&union8, &textureSource);
    Source *src = &unionWithTerrain;*/


    /*Real size = (Real)64.0;
    TextureSource textureSource("sphere.dds", size, size, size, false, false, true);
    LogManager::getSingleton().stream() << "Loaded volume data in " << t.getMilliseconds() << "ms";
    Source *src = &textureSource;
    Vector3 to(size);*/

    mCamera->setPosition(to);
    mCamera->lookAt((Real)5.5, (Real)5.5, (Real)0.0);
    mCamera->setNearClipDistance((Real)0.5);

    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    //Entity* triplanarTest = mSceneMgr->createEntity("TriplanarTest", "triplanarTest.mesh");
    //mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(triplanarTest);

    // Light
    /*Light* pointLight = mSceneMgr->createLight("pointLight0");
    pointLight->setType(Light::LT_POINT);
    pointLight->setPosition(Vector3(750));
    pointLight->setDiffuseColour(1.0, 1.0, 1.0);
    pointLight->setSpecularColour(0.0, 0.0, 0.0);*/

    Light* directionalLight0 = mSceneMgr->createLight("directionalLight0");
    directionalLight0->setType(Light::LT_DIRECTIONAL);
    directionalLight0->setDirection(Vector3(1, -1, 1));
    directionalLight0->setDiffuseColour(1, 0.98, 0.73);
    directionalLight0->setSpecularColour(0.1, 0.1, 0.1);
    /*Light* directionalLight1 = mSceneMgr->createLight("directionalLight1");
    directionalLight1->setType(Light::LT_DIRECTIONAL);
    directionalLight1->setDirection(Vector3(-1, -1, -1));
    directionalLight1->setDiffuseColour(0.2, 0.2, 0.2);
    directionalLight1->setSpecularColour(0.0, 0.0, 0.0);*/
    
    /*Light* spotLight = mSceneMgr->createLight("spotLight0");
    spotLight->setType(Light::LT_SPOTLIGHT);
    spotLight->setPosition(Vector3(100, 300, 100));
    spotLight->setDiffuseColour(1.0, 0.0, 0.0);
    spotLight->setSpecularColour(0.0, 0.0, 0.0);
    spotLight->setDirection(0, -1, 0);
    spotLight->setSpotlightFalloff(1.0);
    spotLight->setSpotlightInnerAngle(Radian(Degree(1)));
    spotLight->setSpotlightOuterAngle(Radian(Degree(10)));*/

    /*pointLight = mSceneMgr->createLight("pointLight2");
    pointLight->setType(Light::LT_POINT);
    pointLight->setPosition(Vector3(0, 1000, 1000));
    pointLight->setDiffuseColour(0.0, 1.0, 0.0);
    pointLight->setSpecularColour(0.0, 0.0, 0.0);*/
   
    // Volume
    Chunk::setMaxPixelError((Real)40.0);
    mVolumeRoot = new Chunk();
    mVolumeRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VolumeParent");
    CallbackTest cb;

    ChunkParameters parameters;
    /*parameters.sceneManager = mSceneMgr;
    parameters.src = src;
    parameters.baseError = (Real)2.0; // 2.5;
    parameters.errorMultiplicator = (Real)0.85; // 0.85;
    parameters.createOctreeVisualization = false; // true;
    parameters.createDualGridVisualization = false; // true;
    parameters.skirtFactor = (Real)0.6;*/

    parameters.lodCallback = &cb;
    parameters.lodCallbackLod = 5;

    //mVolumeRoot->load(mVolumeRootNode, Vector3::ZERO, to, 4, &parameters);

    mVolumeRoot->load(mVolumeRootNode, mSceneMgr, "volumeTerrain1.cfg", &cb, 5);
    
    /*parameters.sceneManager = mSceneMgr;
    parameters.src = src;
    parameters.baseError = (Real)0.5; // 2.5;
    parameters.errorMultiplicator = (Real)0.75; // 0.85;
    parameters.createOctreeVisualization = true; // true;
    parameters.createDualGridVisualization = true; // true;
    parameters.skirtFactor = (Real)5.5;
    mVolumeRoot->load(mVolumeRootNode, Vector3::ZERO, to, 2, &parameters);*/

    t.reset();
    mVolumeRoot->setMaterial("triplanarReference");
    //mVolumeRoot->setChunkMaterial("justWhite");
    LogManager::getSingleton().stream() << "Loaded material in " << t.getMilliseconds() << "ms";
    //mVolumeRootNode->scale(Vector3((Real)2.0));

    // Bye spheres plane
    /*
    for (size_t z = 0; z < 10; ++z)
    {
        for (size_t x = 0; x < 10; ++x)
        {
            delete spheres[z][x];
        }
    }
    for (size_t i = 0; i < 99; ++i)
    {
        delete unions[i];
    }*/
}

void Sample_VolumeTerrain::setupControls(void)
{
    mTrayMgr->showCursor();
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
        setDragLook(true);
#endif
    mCameraMan->setTopSpeed((Real)25.0);

    // make room for the controls
    mTrayMgr->showLogo(TL_TOPRIGHT);
    mTrayMgr->showFrameStats(TL_TOPRIGHT);
    mTrayMgr->toggleAdvancedFrameStats();
    
    // checkboxes
    CheckBox *checkbox = mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowOctree", "Show Octree");
    //checkbox->setChecked(true);
    checkbox = mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowDualgrid", "Show Dualgrid");
    //checkbox->setChecked(true);
    checkbox = mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowTerrain", "Show Terrain");
    checkbox->setChecked(true);

    // a console
    mTrayMgr->createCheckBox(TL_TOPLEFT, "ShowConsole", "Show Console");
    mConsole = mTrayMgr->createTextBox(TL_TOPLEFT, "Console", "Console", (Real)600.0, (Real)400.0);
    mConsole->hide();
    mTrayMgr->removeWidgetFromTray("Console");
    LogManager::getSingleton().getDefaultLog()->addListener(this);
    LogManager::getSingleton().logMessage("Welcome to the Volume Renderer sample!");
    mTrayMgr->hideAll();
    mTrayMgr->showAll();
}

void Sample_VolumeTerrain::checkBoxToggled(CheckBox* checkBox)
{
    if (checkBox->getName() == "ShowConsole")
    {
        if (checkBox->isChecked())
        {
            if (mConsoleCounter >= 18)
            {
                mConsole->setScrollPercentage((Real)1.0);
            }
            mConsole->show();
            mTrayMgr->moveWidgetToTray("Console", TL_TOPLEFT);
        }
        else
        {
            mConsole->hide();
            mTrayMgr->removeWidgetFromTray("Console");
        }
    }
    else if (checkBox->getName() == "ShowOctree")
    {
        if (mVolumeRoot)
        {
            mVolumeRoot->showOctree(checkBox->isChecked());
        }
    }
    else if (checkBox->getName() == "ShowDualgrid")
    {
        if (mVolumeRoot)
        {
            mVolumeRoot->showDualGrid(checkBox->isChecked());
        }
    }
    else if (checkBox->getName() == "ShowTerrain")
    {
        if (mVolumeRoot)
        {
            mVolumeRoot->setVisible(checkBox->isChecked());
        }
    }
}

void Sample_VolumeTerrain::cleanupContent(void)
{
    mConsoleCounter = 0;
    mVolumeRoot = 0; // dito
    mVolumeRootNode = 0; // dito
    mAllHidden = false;
    LogManager::getSingleton().getDefaultLog()->removeListener(this);
}

Sample_VolumeTerrain::Sample_VolumeTerrain() : mConsoleCounter(0), mVolumeRoot(0), mAllHidden(false)
{
    mInfo["Title"] = "Volume Terrain";
    mInfo["Description"] = "GSoC 2012 Volume Terrain sample";
    mInfo["Thumbnail"] = "thumb_volumeterrain.png";
    mInfo["Category"] = "Geometry";
}

void Sample_VolumeTerrain::messageLogged(const String& message, LogMessageLevel lml, bool maskDebug, const String &logName, bool& skipThisMessage)
{
    if (!skipThisMessage)
    {
        mConsole->appendText(message + "\n");
        mConsoleCounter++;
    }
}

bool Sample_VolumeTerrain::keyPressed(const OIS::KeyEvent& evt)
{
    if (evt.key == OIS::KC_H)
    {
        if (mAllHidden)
        {
            mTrayMgr->showAll();
        }
        else
        {
            mTrayMgr->hideAll();
        }
        mAllHidden = !mAllHidden;
    }
    if (evt.key == OIS::KC_M)
    {
        setupMCDisplay(mcConfig++);
    }
    return SdkSample::keyPressed(evt);
}

#ifndef OGRE_STATIC_LIB

SamplePlugin* sp;
Sample* s;


extern "C" _OgreSampleExport void dllStartPlugin()
{
    s = new Sample_VolumeTerrain();
    sp = OGRE_NEW SamplePlugin(s->getInfo()["Title"] + " Sample");
    sp->addSample(s);
    Root::getSingleton().installPlugin(sp);
}

extern "C" _OgreSampleExport void dllStopPlugin()
{
    Root::getSingleton().uninstallPlugin(sp); 
    OGRE_DELETE sp;
    delete s;
}

#endif
