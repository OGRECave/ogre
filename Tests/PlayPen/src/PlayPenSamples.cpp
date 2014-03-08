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
#include "PlayPenSamples.h"
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#include "OgreMeshLodGenerator.h"
#include "OgreLodConfig.h"
#endif

//---------------------------------------------------------------------
PlayPen_testManualBlend::PlayPen_testManualBlend()
{
    mInfo["Title"] = "PlayPen: Manual Blend";
    mInfo["Description"] = "Manual blending";

}
void PlayPen_testManualBlend::setupContent()
{
    // create material
    MaterialPtr mat = MaterialManager::getSingleton().create("TestMat", 
        TRANSIENT_RESOURCE_GROUP).staticCast<Material>();
    Pass * p = mat->getTechnique(0)->getPass(0);
    p->setLightingEnabled(false);
    p->createTextureUnitState("Dirt.jpg");
    TextureUnitState* t = p->createTextureUnitState("ogrelogo.png");
    t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, 
        ColourValue::White, ColourValue::White, 0.75);

    Entity *planeEnt = mSceneMgr->createEntity(SceneManager::PT_PLANE);
    planeEnt->setName("Plane");
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(planeEnt);
    planeEnt->setMaterialName("TestMat");

    mCamera->setPosition(0,0,600);
    mCamera->lookAt(Vector3::ZERO);
}
//---------------------------------------------------------------------
PlayPen_testProjectSphere::PlayPen_testProjectSphere()
{
    mInfo["Title"] = "PlayPen: Project Sphere";
    mInfo["Description"] = "Projecting a sphere's bounds onto the camera";

}
void PlayPen_testProjectSphere::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue::White);


    Plane plane;
    plane.normal = Vector3::UNIT_Y;
    plane.d = 0;
    MeshManager::getSingleton().createPlane("Myplane",
        TRANSIENT_RESOURCE_GROUP, plane,
        4500,4500,10,10,true,1,5,5,Vector3::UNIT_Z);
    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("Examples/GrassFloor");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

    mProjectionSphere = new Sphere(Vector3(0, 2000, 0), 1500.0);

    ManualObject* debugSphere = mSceneMgr->createManualObject();
    debugSphere->setName("debugSphere");
    debugSphere->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
    for (int i = 0; i <= 20; ++i)
    {
        Vector3 basePos(mProjectionSphere->getRadius(), 0, 0);
        Quaternion quat;
        quat.FromAngleAxis(Radian(((float)i/(float)20)*Math::TWO_PI), Vector3::UNIT_Y);
        basePos = quat * basePos;
        debugSphere->position(basePos);
    }
    for (int i = 0; i <= 20; ++i)
    {
        Vector3 basePos(mProjectionSphere->getRadius(), 0, 0);
        Quaternion quat;
        quat.FromAngleAxis(Radian(((float)i/(float)20)*Math::TWO_PI), Vector3::UNIT_Z);
        basePos = quat * basePos;
        debugSphere->position(basePos);
    }
    debugSphere->end();

    SceneNode *sphereNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    sphereNode->setPosition(Vector3(0,2000,0));
    sphereNode->attachObject(debugSphere);

    MaterialPtr mat = MaterialManager::getSingleton().create("scissormat", 
        TRANSIENT_RESOURCE_GROUP).staticCast<Material>();
    Pass* p = mat->getTechnique(0)->getPass(0);
    p->setDepthWriteEnabled(false);
    p->setSceneBlending(SBT_TRANSPARENT_ALPHA);
    TextureUnitState* t = p->createTextureUnitState();
    t->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 
        ColourValue::Red);
    t->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, 0.5f);


    mScissorRect = mSceneMgr->createManualObject();
    mScissorRect->setName("mScissorRect");
    mScissorRect->setUseIdentityProjection(true);
    mScissorRect->setUseIdentityView(true);
    mScissorRect->setLocalAabb(Aabb::BOX_INFINITE);
    mScissorRect->begin(mat->getName());
    mScissorRect->position(Vector3::ZERO);
    mScissorRect->position(Vector3::ZERO);
    mScissorRect->position(Vector3::ZERO);
    mScissorRect->quad(0, 1, 2, 3);
    mScissorRect->end();
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mScissorRect);

    mCamera->setPosition(0,3000,5000);
    mCamera->lookAt(mProjectionSphere->getCenter());


}
bool PlayPen_testProjectSphere::frameStarted(const Ogre::FrameEvent& evt)
{
    Real left, top, right, bottom;
    mCamera->projectSphere(*mProjectionSphere, &left, &top, &right, &bottom);

    mScissorRect->beginUpdate(0);
    mScissorRect->position(left, top, 0);
    mScissorRect->position(left, bottom, 0);
    mScissorRect->position(right, bottom, 0);
    mScissorRect->position(right, top, 0);
    mScissorRect->quad(0,1,2,3);
    mScissorRect->end();

    return PlayPenBase::frameStarted(evt);

}

//---------------------------------------------------------------------
PlayPen_testCameraSetDirection::PlayPen_testCameraSetDirection()
: mUseParentNode(false)
, mUseFixedYaw(true)
, mFocus(100,200,-300)
{
    mInfo["Title"] = "PlayPen: Camera Set Direction";
    mInfo["Description"] = "Testing various settings for Camera::setDirection";

}
void PlayPen_testCameraSetDirection::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue::White);

    Entity* e = mSceneMgr->createEntity("1", "knot.mesh");
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(mFocus);
    node->attachObject(e);


    mCamera->setPosition(200,1000,1000);
    mCamera->lookAt(mFocus);

    mTrayMgr->createButton(OgreBites::TL_BOTTOM, "Look At", "Look At");
    mTrayMgr->createCheckBox(OgreBites::TL_BOTTOM, "tglParent", "Use Parent Node");
    OgreBites::CheckBox* chk = mTrayMgr->createCheckBox(OgreBites::TL_BOTTOM, "tglFixedYaw", "Use Fixed Yaw");
    chk->setChecked(true, false);
    mTrayMgr->showCursor();
    setDragLook(true);

    mParentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    mParentNode->setPosition(Vector3(1000, 2000, -1000));

}
void PlayPen_testCameraSetDirection::buttonHit(OgreBites::Button* button)
{
    mCamera->lookAt(mFocus);
}

void PlayPen_testCameraSetDirection::checkBoxToggled(OgreBites::CheckBox* box)
{
    if (box->getName() == "tglParent")
    {
        mUseParentNode = !mUseParentNode;

        if (mUseParentNode)
            mParentNode->attachObject(mCamera);
        else
            mParentNode->detachAllObjects();
    }
    else if (box->getName() == "tglFixedYaw")
    {
        mUseFixedYaw = !mUseFixedYaw;
        if (mUseFixedYaw)
            mCamera->setFixedYawAxis(true);
        else
            mCamera->setFixedYawAxis(false);

    }
}
#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
//---------------------------------------------------------------------
PlayPen_testManualLOD::PlayPen_testManualLOD()
{
    mInfo["Title"] = "PlayPen: Test Manual LOD";
    mInfo["Description"] = "Testing meshes with manual LODs assigned";
}
//---------------------------------------------------------------------
String PlayPen_testManualLOD::getLODMesh()
{
    MeshPtr msh1 = (MeshPtr)MeshManager::getSingleton().load("robot.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    LodConfig lodConfig(msh1);
    lodConfig.createManualLodLevel(5, "razor.mesh");
    lodConfig.createManualLodLevel(10, "sphere.mesh");
    MeshLodGenerator().generateLodLevels(lodConfig);

    return msh1->getName();

}
//---------------------------------------------------------------------
void PlayPen_testManualLOD::setupContent()
{
    String meshName = getLODMesh();

    Entity *ent;
    for (int i = 0; i < 5; ++i)
    {
        ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), meshName);
        // Add entity to the scene node
        SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->setPosition(Vector3(0,0,(i*50)-(5*50/2)));
        node->attachObject(ent);
    }
    AnimationState* anim = ent->getAnimationState("Walk");
    anim->setEnabled(true);
    mAnimStateList.push_back(anim);


    // Give it a little ambience with lights
    SceneNode *lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Light* l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("BlueLight");
    lnode->setPosition(-200,-80,-100);
    l->setDiffuseColour(0.5, 0.5, 1.0);

    lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("GreenLight");
    lnode->setPosition(0,0,-100);
    l->setDiffuseColour(0.5, 1.0, 0.5);

    // Position the camera
    mCamera->setPosition(100,50,100);
    mCamera->lookAt(-50,50,0);

    mSceneMgr->setAmbientLight(ColourValue::White);

}
//---------------------------------------------------------------------
PlayPen_testManualLODFromFile::PlayPen_testManualLODFromFile()
{
    mInfo["Title"] = "PlayPen: Test Manual LOD (file)";
    mInfo["Description"] = "Testing meshes with manual LODs assigned, loaded from a file";
}
//---------------------------------------------------------------------
String PlayPen_testManualLODFromFile::getLODMesh()
{
    MeshPtr msh1 = (MeshPtr)MeshManager::getSingleton().load("robot.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    LodConfig lodConfig(msh1);
    lodConfig.createManualLodLevel(5, "razor.mesh");
    lodConfig.createManualLodLevel(10, "sphere.mesh");
    MeshLodGenerator().generateLodLevels(lodConfig);

    // this time, we save this data to a file and re-load it

    MeshSerializer ser;
    const ResourceGroupManager::LocationList& ll = 
        ResourceGroupManager::getSingleton().getResourceLocationList(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    String prefix;
    for (ResourceGroupManager::LocationList::const_iterator i = ll.begin(); i != ll.end(); ++i)
    {
        if (StringUtil::endsWith((*i)->archive->getName(), "media"))
        {
            prefix = (*i)->archive->getName();
        }
    }
    ser.exportMesh(msh1.get(), prefix + "/testlod.mesh");

    MeshManager::getSingleton().removeAll();

    return "testlod.mesh";

}
#endif
//---------------------------------------------------------------------
PlayPen_testFullScreenSwitch::PlayPen_testFullScreenSwitch()
{
    mInfo["Title"] = "PlayPen: Test full screen";
    mInfo["Description"] = "Testing switching full screen modes without re-initialisation";

}
//---------------------------------------------------------------------
void PlayPen_testFullScreenSwitch::setupContent()
{
    m640x480w = mTrayMgr->createButton(TL_CENTER, "m640x480w", "640 x 480 (windowed)", 300);
    m640x480fs = mTrayMgr->createButton(TL_CENTER, "m640x480fs", "640 x 480 (fullscreen)", 300);
    m800x600w = mTrayMgr->createButton(TL_CENTER, "m800x600w", "800 x 600 (windowed)", 300);
    m800x600fs = mTrayMgr->createButton(TL_CENTER, "m800x600fs", "800 x 600 (fullscreen)", 300);
    m1024x768w = mTrayMgr->createButton(TL_CENTER, "m1024x768w", "1024 x 768 (windowed)", 300);
    m1024x768fs = mTrayMgr->createButton(TL_CENTER, "m1024x768fs", "1024 x 768 (fullscreen)", 300);

    mTrayMgr->showCursor();

}
//---------------------------------------------------------------------
void PlayPen_testFullScreenSwitch::buttonHit(OgreBites::Button* button)
{
    if (button == m640x480w)
        mWindow->setFullscreen(false, 640, 480);
    else if (button == m640x480fs)
        mWindow->setFullscreen(true, 640, 480);
    else if (button == m800x600w)
        mWindow->setFullscreen(false, 800, 600);
    else if (button == m800x600fs)
        mWindow->setFullscreen(true, 800, 600);
    else if (button == m1024x768w)
        mWindow->setFullscreen(false, 1024, 768);
    else if (button == m1024x768fs)
        mWindow->setFullscreen(true, 1024, 768);
}
//---------------------------------------------------------------------
PlayPen_testMorphAnimationWithNormals::PlayPen_testMorphAnimationWithNormals()
{
    mInfo["Title"] = "PlayPen: Morph anim (+normals)";
    mInfo["Description"] = "Testing morph animation with normals";
}
//---------------------------------------------------------------------
void PlayPen_testMorphAnimationWithNormals::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    Vector3 dir(-1, -1, 0.5);
    dir.normalise();
    SceneNode *lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Light* l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("light1");
    l->setType(Light::LT_DIRECTIONAL);
    l->setDirection(dir);

    MeshPtr mesh = MeshManager::getSingleton().load("sphere.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    
    String morphName = "testmorphwithnormals.mesh";
    mesh = mesh->clone(morphName);

    SubMesh* sm = mesh->getSubMesh(0);
    // Re-organise geometry since this mesh has no animation and all 
    // vertex elements are packed into one buffer
    VertexDeclaration* newDecl = 
        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(false, true, true);
    sm->vertexData->reorganiseBuffers(newDecl);
    // get the position buffer (which should now be separate);
    const VertexElement* posElem = 
        sm->vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
    HardwareVertexBufferSharedPtr origbuf = 
        sm->vertexData->vertexBufferBinding->getBuffer(
            posElem->getSource());

    // Create a new position & normal buffer with updated values
    HardwareVertexBufferSharedPtr newbuf = 
        HardwareBufferManager::getSingleton().createVertexBuffer(
            VertexElement::getTypeSize(VET_FLOAT3) * 2,
            sm->vertexData->vertexCount, 
            HardwareBuffer::HBU_STATIC, true);
    float* pSrc = static_cast<float*>(origbuf->lock(HardwareBuffer::HBL_READ_ONLY));
    float* pDst = static_cast<float*>(newbuf->lock(HardwareBuffer::HBL_DISCARD));

    // Make the sphere turn into a cube
    // Do this just by clamping each of the directions (we shrink it)
    float cubeDimension = 0.3f * mesh->getBoundingSphereRadius();
    size_t srcSkip = origbuf->getVertexSize() / sizeof(float) - 3;
    for (size_t v = 0; v < sm->vertexData->vertexCount; ++v)
    {
        // x/y/z position
        Vector3 pos;
        for (int d = 0; d < 3; ++d)
        {
            if (*pSrc >= 0)
            {
                pos.ptr()[d] = std::min(cubeDimension, *pSrc++);
            }
            else 
            {
                pos.ptr()[d] = std::max(-cubeDimension, *pSrc++);           
            }
            *pDst++ = pos.ptr()[d];
        }
        
        // normal
        // this should point along the major axis
        // unfortunately since vertices are not duplicated at edges there will be
        // some inaccuracy here but the most important thing is to add sharp edges
        Vector3 norm = pos.normalisedCopy();
        norm = norm.primaryAxis();
        *pDst++ = norm.x;
        *pDst++ = norm.y;
        *pDst++ = norm.z;

        pSrc += srcSkip;

    }

    origbuf->unlock();
    newbuf->unlock();
    
    // create a morph animation
    Animation* anim = mesh->createAnimation("testAnim", 10.0f);
    VertexAnimationTrack* vt = anim->createVertexTrack(1, sm->vertexData, VAT_MORPH);
    // re-use start positions for frame 0
    VertexMorphKeyFrame* kf = vt->createVertexMorphKeyFrame(0);
    kf->setVertexBuffer(origbuf);

    // Use translated buffer for mid frame
    kf = vt->createVertexMorphKeyFrame(4.0f);
    kf->setVertexBuffer(newbuf);

    // Pause there
    kf = vt->createVertexMorphKeyFrame(6.0f);
    kf->setVertexBuffer(newbuf);
    
    // re-use start positions for final frame
    kf = vt->createVertexMorphKeyFrame(10.0f);
    kf->setVertexBuffer(origbuf);

    // Export the mesh
    DataStreamPtr stream = Root::getSingleton().createFileStream(morphName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    MeshSerializer ser;
    ser.exportMesh(mesh.get(), stream);
    stream->close();
    
    // Unload old mesh to force reload
    MeshManager::getSingleton().remove(mesh->getHandle());
    mesh->unload();
    mesh.setNull();

    Entity* e = mSceneMgr->createEntity("test", morphName);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
    AnimationState* animState = e->getAnimationState("testAnim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);

    e = mSceneMgr->createEntity("test2", morphName);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(Vector3(200,0,0));
    node->attachObject(e);
    // test hardware morph
    e->setMaterialName("Examples/HardwareMorphAnimationWithNormals");
    animState = e->getAnimationState("testAnim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);

    mCamera->setNearClipDistance(0.5);
    mCamera->setPosition(0,100,-400);
    mCamera->lookAt(Vector3::ZERO);
    //mSceneMgr->setShowDebugShadows(true);

    Plane plane;
    plane.normal = Vector3::UNIT_Y;
    plane.d = 200;
    MeshManager::getSingleton().createPlane("Myplane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
        1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("2 - Default");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);
    
}

//---------------------------------------------------------------------
PlayPen_testMorphAnimationWithoutNormals::PlayPen_testMorphAnimationWithoutNormals()
{
    mInfo["Title"] = "PlayPen: Morph anim (-normals)";
    mInfo["Description"] = "Testing morph animation without normals";
}
//---------------------------------------------------------------------
void PlayPen_testMorphAnimationWithoutNormals::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    Vector3 dir(-1, -1, 0.5);
    dir.normalise();
    SceneNode *lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Light* l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("light1");
    l->setType(Light::LT_DIRECTIONAL);
    l->setDirection(dir);


    MeshPtr mesh = MeshManager::getSingleton().load("sphere.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    String morphName = "testmorphnonormals.mesh";
    mesh = mesh->clone(morphName);

    SubMesh* sm = mesh->getSubMesh(0);
    // Re-organise geometry since this mesh has no animation and all 
    // vertex elements are packed into one buffer
    VertexDeclaration* newDecl = 
        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(false, true, false);
    sm->vertexData->reorganiseBuffers(newDecl);
    // get the position buffer (which should now be separate);
    const VertexElement* posElem = 
        sm->vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
    HardwareVertexBufferSharedPtr origbuf = 
        sm->vertexData->vertexBufferBinding->getBuffer(
        posElem->getSource());

    // Create a new position & normal buffer with updated values
    HardwareVertexBufferSharedPtr newbuf = 
        HardwareBufferManager::getSingleton().createVertexBuffer(
        VertexElement::getTypeSize(VET_FLOAT3),
        sm->vertexData->vertexCount, 
        HardwareBuffer::HBU_STATIC, true);
    float* pSrc = static_cast<float*>(origbuf->lock(HardwareBuffer::HBL_READ_ONLY));
    float* pDst = static_cast<float*>(newbuf->lock(HardwareBuffer::HBL_DISCARD));

    // Make the sphere turn into a cube
    // Do this just by clamping each of the directions (we shrink it)
    float cubeDimension = 0.3f * mesh->getBoundingSphereRadius();
    for (size_t v = 0; v < sm->vertexData->vertexCount; ++v)
    {
        // x/y/z position
        Vector3 pos;
        for (int d = 0; d < 3; ++d)
        {
            if (*pSrc >= 0)
            {
                pos.ptr()[d] = std::min(cubeDimension, *pSrc++);
            }
            else 
            {
                pos.ptr()[d] = std::max(-cubeDimension, *pSrc++);           
            }
            *pDst++ = pos.ptr()[d];
        }

    }

    origbuf->unlock();
    newbuf->unlock();

    // create a morph animation
    Animation* anim = mesh->createAnimation("testAnim", 10.0f);
    VertexAnimationTrack* vt = anim->createVertexTrack(1, sm->vertexData, VAT_MORPH);
    // re-use start positions for frame 0
    VertexMorphKeyFrame* kf = vt->createVertexMorphKeyFrame(0);
    kf->setVertexBuffer(origbuf);

    // Use translated buffer for mid frame
    kf = vt->createVertexMorphKeyFrame(4.0f);
    kf->setVertexBuffer(newbuf);

    // Pause there
    kf = vt->createVertexMorphKeyFrame(6.0f);
    kf->setVertexBuffer(newbuf);

    // re-use start positions for final frame
    kf = vt->createVertexMorphKeyFrame(10.0f);
    kf->setVertexBuffer(origbuf);

    // Export the mesh
    DataStreamPtr stream = Root::getSingleton().createFileStream(morphName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    MeshSerializer ser;
    ser.exportMesh(mesh.get(), stream);
    stream->close();

    // Unload old mesh to force reload
    MeshManager::getSingleton().remove(mesh->getHandle());
    mesh->unload();
    mesh.setNull();

    Entity* e = mSceneMgr->createEntity("test", morphName);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
    AnimationState* animState = e->getAnimationState("testAnim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);

    e = mSceneMgr->createEntity("test2", morphName);
    SceneNode * node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(Vector3(200,0,0));
    node->attachObject(e);
    // test hardware morph
    e->setMaterialName("Examples/HardwareMorphAnimation");
    animState = e->getAnimationState("testAnim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);

    mCamera->setNearClipDistance(0.5);
    mCamera->setPosition(0,100,-400);
    mCamera->lookAt(Vector3::ZERO);
    //mSceneMgr->setShowDebugShadows(true);

    Plane plane;
    plane.normal = Vector3::UNIT_Y;
    plane.d = 200;
    MeshManager::getSingleton().createPlane("Myplane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
        1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("2 - Default");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

}
//---------------------------------------------------------------------
PlayPen_testPoseAnimationWithNormals::PlayPen_testPoseAnimationWithNormals()
{
    mInfo["Title"] = "PlayPen: Pose anim (+normals)";
    mInfo["Description"] = "Testing pose animation with normals";

}
//---------------------------------------------------------------------
void PlayPen_testPoseAnimationWithNormals::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    Vector3 dir(-1, -1, 0.5);
    dir.normalise();
    SceneNode *lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Light* l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("light1");
    l->setType(Light::LT_DIRECTIONAL);
    l->setDirection(dir);

    MeshPtr mesh = MeshManager::getSingleton().load("cube.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        
    String newName = "testposewithnormals.mesh";
    mesh = mesh->clone(newName);


    SubMesh* sm = mesh->getSubMesh(0);
    // Re-organise geometry since this mesh has no animation and all 
    // vertex elements are packed into one buffer
    VertexDeclaration* newDecl = 
        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(false, true, true);
    sm->vertexData->reorganiseBuffers(newDecl);

    // create 2 poses
    Pose* pose = mesh->createPose(1, "pose1");
    // Pose1 moves vertices 0, 1, 2 and 3 upward and pushes normals left
    Vector3 offset1(0, 50, 0);
    pose->addVertex(0, offset1, Vector3::NEGATIVE_UNIT_X);
    pose->addVertex(1, offset1, Vector3::NEGATIVE_UNIT_X);
    pose->addVertex(2, offset1, Vector3::NEGATIVE_UNIT_X);
    pose->addVertex(3, offset1, Vector3::NEGATIVE_UNIT_X);

    pose = mesh->createPose(1, "pose2");
    // Pose2 moves vertices 3, 4, and 5 to the right and pushes normals right
    // Note 3 gets affected by both
    Vector3 offset2(100, 0, 0);
    pose->addVertex(3, offset2, Vector3::UNIT_X);
    pose->addVertex(4, offset2, Vector3::UNIT_X);
    pose->addVertex(5, offset2, Vector3::UNIT_X);


    Animation* anim = mesh->createAnimation("poseanim", 20.0f);
    VertexAnimationTrack* vt = anim->createVertexTrack(1, sm->vertexData, VAT_POSE);
    
    // Frame 0 - no effect 
    vt->createVertexPoseKeyFrame(0);

    // Frame 1 - bring in pose 1 (index 0)
    VertexPoseKeyFrame* kf = vt->createVertexPoseKeyFrame(3);
    kf->addPoseReference(0, 1.0f);

    // Frame 2 - remove all 
    vt->createVertexPoseKeyFrame(6);

    // Frame 3 - bring in pose 2 (index 1)
    kf = vt->createVertexPoseKeyFrame(9);
    kf->addPoseReference(1, 1.0f);

    // Frame 4 - remove all
    vt->createVertexPoseKeyFrame(12);


    // Frame 5 - bring in pose 1 at 50%, pose 2 at 100% 
    kf = vt->createVertexPoseKeyFrame(15);
    kf->addPoseReference(0, 0.5f);
    kf->addPoseReference(1, 1.0f);

    // Frame 6 - bring in pose 1 at 100%, pose 2 at 50% 
    kf = vt->createVertexPoseKeyFrame(18);
    kf->addPoseReference(0, 1.0f);
    kf->addPoseReference(1, 0.5f);

    // Frame 7 - reset
    vt->createVertexPoseKeyFrame(20);


    // Export the mesh
    DataStreamPtr stream = Root::getSingleton().createFileStream(newName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    MeshSerializer ser;
    ser.exportMesh(mesh.get(), stream);
    stream->close();

    // Unload old mesh to force reload
    MeshManager::getSingleton().remove(mesh->getHandle());
    mesh->unload();
    mesh.setNull();

    Entity*  e;
    AnimationState* animState;
    // software pose
    e = mSceneMgr->createEntity("test2", newName);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(Vector3(150,0,0));
    node->attachObject(e);
    animState = e->getAnimationState("poseanim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);
    
    // test hardware pose
    e = mSceneMgr->createEntity("test", newName);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
    e->setMaterialName("Examples/HardwarePoseAnimationWithNormals");
    animState = e->getAnimationState("poseanim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);
    
    mCamera->setNearClipDistance(0.5);

    Plane plane;
    plane.normal = Vector3::UNIT_Y;
    plane.d = 200;
    MeshManager::getSingleton().createPlane("Myplane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
        1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("2 - Default");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

    mCamera->setPosition(0,-200,-300);
    mCamera->lookAt(0,0,0);
}

//---------------------------------------------------------------------
PlayPen_testPoseAnimationWithoutNormals::PlayPen_testPoseAnimationWithoutNormals()
{
    mInfo["Title"] = "PlayPen: Pose anim (-normals)";
    mInfo["Description"] = "Testing pose animation without normals";
}
//---------------------------------------------------------------------
void PlayPen_testPoseAnimationWithoutNormals::setupContent()
{
    mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
    Vector3 dir(-1, -1, 0.5);
    dir.normalise();
    SceneNode *lnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Light* l = mSceneMgr->createLight();
    lnode->attachObject(l);
    lnode->setName("light1");
    l->setType(Light::LT_DIRECTIONAL);
    l->setDirection(dir);

    MeshPtr mesh = MeshManager::getSingleton().load("cube.mesh", 
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        
    String newName = "testposenonormals.mesh";
    mesh = mesh->clone(newName);

    SubMesh* sm = mesh->getSubMesh(0);
    // Re-organise geometry since this mesh has no animation and all 
    // vertex elements are packed into one buffer
    VertexDeclaration* newDecl = 
        sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(false, true, false);
    sm->vertexData->reorganiseBuffers(newDecl);

    // create 2 poses
    Pose* pose = mesh->createPose(1, "pose1");
    // Pose1 moves vertices 0, 1, 2 and 3 upward 
    Vector3 offset1(0, 50, 0);
    pose->addVertex(0, offset1);
    pose->addVertex(1, offset1);
    pose->addVertex(2, offset1);
    pose->addVertex(3, offset1);

    pose = mesh->createPose(1, "pose2");
    // Pose2 moves vertices 3, 4, and 5 to the right
    // Note 3 gets affected by both
    Vector3 offset2(100, 0, 0);
    pose->addVertex(3, offset2);
    pose->addVertex(4, offset2);
    pose->addVertex(5, offset2);


    Animation* anim = mesh->createAnimation("poseanim", 20.0f);
    VertexAnimationTrack* vt = anim->createVertexTrack(1, sm->vertexData, VAT_POSE);
    
    // Frame 0 - no effect 
    vt->createVertexPoseKeyFrame(0);

    // Frame 1 - bring in pose 1 (index 0)
    VertexPoseKeyFrame* kf = vt->createVertexPoseKeyFrame(3);
    kf->addPoseReference(0, 1.0f);

    // Frame 2 - remove all 
    vt->createVertexPoseKeyFrame(6);

    // Frame 3 - bring in pose 2 (index 1)
    kf = vt->createVertexPoseKeyFrame(9);
    kf->addPoseReference(1, 1.0f);

    // Frame 4 - remove all
    vt->createVertexPoseKeyFrame(12);


    // Frame 5 - bring in pose 1 at 50%, pose 2 at 100% 
    kf = vt->createVertexPoseKeyFrame(15);
    kf->addPoseReference(0, 0.5f);
    kf->addPoseReference(1, 1.0f);

    // Frame 6 - bring in pose 1 at 100%, pose 2 at 50% 
    kf = vt->createVertexPoseKeyFrame(18);
    kf->addPoseReference(0, 1.0f);
    kf->addPoseReference(1, 0.5f);

    // Frame 7 - reset
    vt->createVertexPoseKeyFrame(20);

    // Export the mesh
    DataStreamPtr stream = Root::getSingleton().createFileStream(newName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
    MeshSerializer ser;
    ser.exportMesh(mesh.get(), stream);
    stream->close();

    // Unload old mesh to force reload
    MeshManager::getSingleton().remove(mesh->getHandle());
    mesh->unload();
    mesh.setNull();

    Entity*  e;
    AnimationState* animState;
    // software pose
    e = mSceneMgr->createEntity("test2", newName);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(Vector3(150,0,0));
    node->attachObject(e);
    animState = e->getAnimationState("poseanim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);
    
    // test hardware pose
    e = mSceneMgr->createEntity("test", newName);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
    e->setMaterialName("Examples/HardwarePoseAnimation");
    animState = e->getAnimationState("poseanim");
    animState->setEnabled(true);
    animState->setWeight(1.0f);
    mAnimStateList.push_back(animState);
    

    mCamera->setNearClipDistance(0.5);

    Plane plane;
    plane.normal = Vector3::UNIT_Y;
    plane.d = 200;
    MeshManager::getSingleton().createPlane("Myplane",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
        1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
    Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
    pPlaneEnt->setMaterialName("2 - Default");
    pPlaneEnt->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

    mCamera->setPosition(0,-200,-300);
    mCamera->lookAt(0,0,0);
}
