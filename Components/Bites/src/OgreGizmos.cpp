#include "../include/OgreGizmos.h"
#include "OgreManualObject.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"
#include "OgreSubEntity.h"
#include "OgreTechnique.h"
#include <OgreEntity.h>

namespace OgreBites
{
Gizmo::Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode) : mMode(G_NONE)
{
    Ogre::MaterialPtr gizmoMaterial = Ogre::MaterialManager::getSingletonPtr()->createOrRetrieve("Gizmo_Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first.staticCast<Ogre::Material>();
    setObject(sceneNode);
    gizmoMaterial->setReceiveShadows(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
    gizmoMaterial->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);
    gizmoMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_DIFFUSE);

    createMesh(sceneManager, "OgitorAxisGizmoMesh");
    createPlaneMesh(sceneManager, "OgitorAxisPlaneMesh");

    mGizmoNode = sceneNode->createChildSceneNode("scbWidgetNode", Ogre::Vector3(0,0,0),
                                                                             Ogre::Quaternion::IDENTITY);

    mGizmoX = mGizmoNode->createChildSceneNode("scbnwx", Ogre::Vector3(0,0,0), Ogre::Quaternion::IDENTITY);
    mGizmoY = mGizmoNode->createChildSceneNode("scbnwy", Ogre::Vector3(0,0,0), Ogre::Quaternion::IDENTITY);
    mGizmoZ = mGizmoNode->createChildSceneNode("scbnwz", Ogre::Vector3(0,0,0), Ogre::Quaternion::IDENTITY);

    Ogre::Quaternion q1;
    Ogre::Quaternion q2;

    q1.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0,0,1));
    q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(1,0,0));
    mGizmoY->setOrientation(q1 * q2);

    q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0,1,0));
    q2.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(1,0,0));
    mGizmoZ->setOrientation(q1 * q2);

    //Entities
    mGizmoEntities[0] = sceneManager->createEntity("scbwx", "OgitorAxisGizmoMesh", "General");
    mGizmoEntities[1] = sceneManager->createEntity("scbwy", "OgitorAxisGizmoMesh", "General");
    mGizmoEntities[2] = sceneManager->createEntity("scbwz", "OgitorAxisGizmoMesh", "General");
    mGizmoEntities[3] = sceneManager->createEntity("scbwt", "OgitorAxisPlaneMesh", "General");
    mGizmoEntities[4] = sceneManager->createEntity("scbwu", "OgitorAxisPlaneMesh", "General");
    mGizmoEntities[5] = sceneManager->createEntity("scbwv", "OgitorAxisPlaneMesh", "General");

    //XX arrows
    mGizmoEntities[0]->setCastShadows(false);
    mGizmoEntities[0]->setMaterialName("MAT_GIZMO_X");
    mGizmoEntities[0]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[0]->setQueryFlags(Ogre::QUERYFLAG_WIDGET);
    mGizmoX->attachObject(mGizmoEntities[0]);

    //YY arrows
    mGizmoEntities[1]->setCastShadows(false);
    mGizmoEntities[1]->setMaterialName("MAT_GIZMO_Y");
    mGizmoEntities[1]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[1]->setQueryFlags(QUERYFLAG_WIDGET);
    mGizmoY->attachObject(mGizmoEntities[1]);

    //ZZ arrows
    mGizmoEntities[2]->setCastShadows(false);
    mGizmoEntities[2]->setMaterialName("MAT_GIZMO_Z");
    mGizmoEntities[2]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[2]->setQueryFlags(QUERYFLAG_WIDGET);
    mGizmoZ->attachObject(mGizmoEntities[2]);

    //XY Plane
    mGizmoEntities[3]->setCastShadows(false);
    mGizmoEntities[3]->setMaterialName("MAT_GIZMO_XY");
    mGizmoEntities[3]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[3]->setQueryFlags(QUERYFLAG_WIDGET);
    mGizmoX->attachObject(mGizmoEntities[3]);

    //YZ Plane
    mGizmoEntities[4]->setCastShadows(false);
    mGizmoEntities[4]->setMaterialName("MAT_GIZMO_YZ");
    mGizmoEntities[4]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[4]->setQueryFlags(QUERYFLAG_WIDGET);
    mGizmoY->attachObject(mGizmoEntities[4]);

    //ZX Plane
    mGizmoEntities[5]->setCastShadows(false);
    mGizmoEntities[5]->setMaterialName("MAT_GIZMO_ZX");
    mGizmoEntities[5]->setRenderQueueGroup(Ogre::RENDER_QUEUE_SKIES_LATE);
    // mGizmoEntities[5]->setQueryFlags(QUERYFLAG_WIDGET);
    mGizmoZ->attachObject(mGizmoEntities[5]);

    // Call once to derive bounding boxes
    mGizmoEntities[0]->getWorldBoundingBox(true);
    mGizmoEntities[1]->getWorldBoundingBox(true);
    mGizmoEntities[2]->getWorldBoundingBox(true);

    mEntityToAxis[mGizmoEntities[0]] = AXIS_X;
    mEntityToAxis[mGizmoEntities[1]] = AXIS_Y;
    mEntityToAxis[mGizmoEntities[2]] = AXIS_Z;

    mEntityToAxis[mGizmoEntities[3]] = AXIS_XY;
    mEntityToAxis[mGizmoEntities[4]] = AXIS_YZ;
    mEntityToAxis[mGizmoEntities[5]] = AXIS_XZ;

    mGizmoNode->setVisible(true);
    setMode(mode);
}

void Gizmo::setHighlighted(Ogre::Entity* entity)
{
    if (!entity)
    {
        mOldGizmoAxis = AXIS_NONE;
        mGizmoEntities[0]->setMaterialName("MAT_GIZMO_X");
        mGizmoEntities[1]->setMaterialName("MAT_GIZMO_Y");
        mGizmoEntities[2]->setMaterialName("MAT_GIZMO_Z");
        mGizmoEntities[3]->setMaterialName("MAT_GIZMO_XY");
        mGizmoEntities[4]->setMaterialName("MAT_GIZMO_YZ");
        mGizmoEntities[5]->setMaterialName("MAT_GIZMO_ZX");

        mHighlighted = nullptr;
        return;
    }
    auto it = mEntityToAxis.find(entity);
    if (it == mEntityToAxis.end())
        return;

    int axis = it->second;

    // Nothing changed?
    if (axis == mOldGizmoAxis)
        return;

    mOldGizmoAxis = axis;
    mHighlighted = entity;

    mGizmoEntities[0]->setMaterialName((axis & AXIS_X) ?
        "MAT_GIZMO_X_L" : "MAT_GIZMO_X");
    mGizmoEntities[1]->setMaterialName((axis & AXIS_Y) ?
        "MAT_GIZMO_Y_L" : "MAT_GIZMO_Y");
    mGizmoEntities[2]->setMaterialName((axis & AXIS_Z) ?
        "MAT_GIZMO_Z_L" : "MAT_GIZMO_Z");
    mGizmoEntities[3]->setMaterialName((axis == AXIS_XY) ?
        "MAT_GIZMO_XY_L" : "MAT_GIZMO_XY");
    mGizmoEntities[4]->setMaterialName((axis == AXIS_YZ) ?
        "MAT_GIZMO_YZ_L" : "MAT_GIZMO_YZ");
    mGizmoEntities[5]->setMaterialName((axis == AXIS_XZ) ?
        "MAT_GIZMO_ZX_L" : "MAT_GIZMO_ZX");
}


void Gizmo::setObject(Ogre::SceneNode* sceneObject)
{
    mGizmoNode = sceneObject;
}

void Gizmo::setMode(GizmoMode mode)
{
    if (mMode == mode)
        return;
    mMode = mode;
    Ogre::Entity* wx = mGizmoEntities[0];
    Ogre::Entity* wy = mGizmoEntities[1];
    Ogre::Entity* wz = mGizmoEntities[2];

    mGizmoEntities[3]->getSubEntity(0)->setVisible(false);
    mGizmoEntities[4]->getSubEntity(0)->setVisible(false);
    mGizmoEntities[5]->getSubEntity(0)->setVisible(false);

    wx->getSubEntity(0)->setVisible(true);
    wx->getSubEntity(1)->setVisible(false);
    wx->getSubEntity(2)->setVisible(false);
    wx->getSubEntity(3)->setVisible(false);
    wx->getSubEntity(4)->setVisible(false);
    wy->getSubEntity(0)->setVisible(true);
    wy->getSubEntity(1)->setVisible(false);
    wy->getSubEntity(2)->setVisible(false);
    wy->getSubEntity(3)->setVisible(false);
    wy->getSubEntity(4)->setVisible(false);
    wz->getSubEntity(0)->setVisible(true);
    wz->getSubEntity(1)->setVisible(false);
    wz->getSubEntity(2)->setVisible(false);
    wz->getSubEntity(3)->setVisible(false);
    wz->getSubEntity(4)->setVisible(false);
    switch(mode)
    {
    case G_TRANSLATE:
        {
            wx->getSubEntity(2)->setVisible(true);
            wy->getSubEntity(2)->setVisible(true);
            wz->getSubEntity(2)->setVisible(true);
            mGizmoEntities[3]->getSubEntity(0)->setVisible(true);
            mGizmoEntities[4]->getSubEntity(0)->setVisible(true);
            mGizmoEntities[5]->getSubEntity(0)->setVisible(true);
            break;
        }
    case G_ROTATE:
        {
            wx->getSubEntity(0)->setVisible(false);
            wy->getSubEntity(0)->setVisible(false);
            wz->getSubEntity(0)->setVisible(false);
            wx->getSubEntity(1)->setVisible(true);
            wy->getSubEntity(1)->setVisible(true);
            wz->getSubEntity(1)->setVisible(true);
            wx->getSubEntity(3)->setVisible(true);
            wy->getSubEntity(3)->setVisible(true);
            wz->getSubEntity(3)->setVisible(true);
            break;
        }
    case G_SCALE:
        {
            wx->getSubEntity(4)->setVisible(true);
            wy->getSubEntity(4)->setVisible(true);
            wz->getSubEntity(4)->setVisible(true);
            mGizmoEntities[3]->getSubEntity(0)->setVisible(true);
            mGizmoEntities[4]->getSubEntity(0)->setVisible(true);
            mGizmoEntities[5]->getSubEntity(0)->setVisible(true);
            break;
        }
    }

}

Ogre::Vector3 computeDrag(const Ogre::Ray& ray)
{
    if (!mActiveGizmo || !mParentObject)
        return mInitialObjectPos;

    // Determine axis
    Ogre::Vector3 axis;
    if (mActiveGizmo == mGizmoEntities[0]) axis = Ogre::Vector3::UNIT_X;
    else if (mActiveGizmo == mGizmoEntities[1]) axis = Ogre::Vector3::UNIT_Y;
    else if (mActiveGizmo == mGizmoEntities[2]) axis = Ogre::Vector3::UNIT_Z;
    else return mInitialObjectPos;

    // Construct a plane perpendicular to the axis, passing through initial object position
    Ogre::Plane plane(axis.crossProduct(mCamera->getDerivedDirection()), mInitialObjectPos);

    // Intersect ray with plane
    std::pair<bool, Ogre::Real> intersection = ray.intersects(plane);
    if (!intersection.first)
        return mInitialObjectPos;

    Ogre::Vector3 hitPoint = ray.getPoint(intersection.second);

    // Project delta onto axis
    Ogre::Vector3 delta = hitPoint - mInitialObjectPos;
    Ogre::Real amount = delta.dotProduct(axis);

    return mInitialObjectPos + axis * amount;
}



void Gizmo::createMesh(Ogre::SceneManager *manager, Ogre::String name)
{
    Ogre::ManualObject *mMesh = manager->createManualObject("AxisGizmoManualObject");

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_LINE_LIST);
    mMesh->position(0, 0, 0);
    mMesh->position(3, 0, 0);

    mMesh->index(0);
    mMesh->index(1);
    mMesh->end();

    float const radius = 0.22f;
    float const accuracy = 8;
    float MPI = Ogre::Math::PI;

    float division = (MPI / 2.0f) / 16.0f;
    float start = division * 3;
    float end = division * 14;


    int index_pos = 0;

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_LINE_STRIP);

    for(float theta = start; theta < end; theta += division)
    {
        mMesh->position(0, 3.0f * cos(theta), 3.0f * sin(theta));
        mMesh->index(index_pos++);
    }

    mMesh->end();

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position(2.85f,     0,     0);

    for(float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        mMesh->position(2.95f, radius * cos(theta), radius * sin(theta));
    }
    mMesh->position(3.45f,     0,     0);

    for(int inside = 1;inside < 16;inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for(int outside = 1;outside < 16;outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    mMesh->end();

    //ROTATE GIZMO

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    Ogre::Quaternion q1;
    q1.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3(0,0,1));
    Ogre::Quaternion q2;
    q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3(0,1,0));

    Ogre::Vector3 translate1(0, 3.0f * cos(end), 3.0f * sin(end));
    Ogre::Vector3 translate2(0, 3.0f * cos(start), 3.0f * sin(start) - 0.25f);

    Ogre::Vector3 pos(-0.3f,     0,     0);
    mMesh->position(q1 * pos + translate1);

    for(float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
        mMesh->position(q1 * pos + translate1);
    }
    pos = Ogre::Vector3(0.3f, 0 , 0);
    mMesh->position(q1 * pos + translate1);

    pos = Ogre::Vector3(-0.3f,     0,     0);
    mMesh->position(q2 * pos + translate2);

    for(float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        pos = Ogre::Vector3(-0.3f, radius * cos(theta), radius * sin(theta));
        mMesh->position(q2 * pos + translate2);
    }
    pos = Ogre::Vector3(0.3f, 0 , 0);
    mMesh->position(q2 * pos + translate2);

    for(int inside = 1;inside < 16;inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for(int outside = 1;outside < 16;outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    for(int inside = 19;inside < 34;inside++)
    {
        mMesh->index(18);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(18);
    mMesh->index(34);
    mMesh->index(19);

    for(int outside = 19;outside < 34;outside++)
    {
        mMesh->index(35);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(35);
    mMesh->index(34);
    mMesh->index(19);

    mMesh->end();

    //SCALE GIZMO

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position(2.85f,     0,     0);

    for(float theta = 0; theta < 2 * MPI; theta += MPI / accuracy)
    {
        mMesh->position(2.85f, radius * cos(theta), radius * sin(theta));
    }
    mMesh->position(3.45f,     0,     0);

    mMesh->position(3.40f,  0.20f,  0.20f);
    mMesh->position(3.40f,  0.20f, -0.20f);
    mMesh->position(3.40f, -0.20f, -0.20f);
    mMesh->position(3.40f, -0.20f,  0.20f);
    mMesh->position(3.50f,  0.20f,  0.20f);
    mMesh->position(3.50f,  0.20f, -0.20f);
    mMesh->position(3.50f, -0.20f, -0.20f);
    mMesh->position(3.50f, -0.20f,  0.20f);

    for(int inside = 1;inside < 16;inside++)
    {
        mMesh->index(0);
        mMesh->index(inside);
        mMesh->index(inside + 1);
    }
    mMesh->index(0);
    mMesh->index(16);
    mMesh->index(1);

    for(int outside = 1;outside < 16;outside++)
    {
        mMesh->index(17);
        mMesh->index(outside);
        mMesh->index(outside + 1);
    }
    mMesh->index(17);
    mMesh->index(16);
    mMesh->index(1);

    mMesh->index(18);
    mMesh->index(19);
    mMesh->index(20);
    mMesh->index(18);
    mMesh->index(20);
    mMesh->index(21);

    mMesh->index(22);
    mMesh->index(23);
    mMesh->index(24);
    mMesh->index(22);
    mMesh->index(24);
    mMesh->index(25);

    mMesh->index(18);
    mMesh->index(22);
    mMesh->index(25);
    mMesh->index(18);
    mMesh->index(25);
    mMesh->index(21);

    mMesh->index(19);
    mMesh->index(23);
    mMesh->index(24);
    mMesh->index(19);
    mMesh->index(24);
    mMesh->index(20);

    mMesh->index(18);
    mMesh->index(22);
    mMesh->index(23);
    mMesh->index(18);
    mMesh->index(23);
    mMesh->index(19);

    mMesh->index(21);
    mMesh->index(20);
    mMesh->index(24);
    mMesh->index(21);
    mMesh->index(24);
    mMesh->index(25);

    mMesh->end();

    mMesh->convertToMesh(name, "General");

    manager->destroyManualObject(mMesh);
}

void Gizmo::createPlaneMesh(Ogre::SceneManager *manager, Ogre::String name)
{
    Ogre::ManualObject *mMesh = manager->createManualObject("OgitorAxisPlaneGizmoManualObject");

    mMesh->begin("AxisGizmo_Material", Ogre::RenderOperation::OT_TRIANGLE_LIST);

    mMesh->position( 0, 1, 0);
    mMesh->position( 1, 1, 0);
    mMesh->position( 1, 0, 0);
    mMesh->position( 0, 0, 0);

    mMesh->index(0);
    mMesh->index(1);
    mMesh->index(2);
    mMesh->index(0);
    mMesh->index(2);
    mMesh->index(3);

    mMesh->end();

    mMesh->convertToMesh(name, "General");

    manager->destroyManualObject(mMesh);
}
}