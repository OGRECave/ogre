#include "../include/OgreGizmos.h"
#include "OgreManualObject.h"
#include "OgreMaterialManager.h"
#include "OgreSceneManager.h"

namespace OgreBites
{
Gizmo::Gizmo(Ogre::SceneNode* sceneNode, GizmoMode mode) : mMode(G_CAMERA)
{
    Ogre::MaterialPtr gizmoMaterial = Ogre::MaterialManager::getSingletonPtr()->createOrRetrieve("Gizmo_Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first.staticCast<Ogre::Material>();
    setObject(sceneNode);
    setMode(mode);
};

void Gizmo::setObject(Ogre::SceneNode* sceneObject)
{
    mSceneNode = sceneObject;
}

void Gizmo::setMode(GizmoMode mode)
{
    mMode = mode;

}
void Gizmo::createMesh(Ogre::SceneManager *manager, Ogre::String name)
{
    // // Create object
    // mGizmoObj = std::make_unique<Ogre::ManualObject>("gizmo");
    // mGizmoObj->setCastShadows(false);
    // mSceneNode->attachObject(mGizmoObj.get());
    //
    // mGizmoObj->begin("baseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_STRIP);
    // mGizmoObj->colour((Ogre::Real)0.0, (Ogre::Real)1.0, (Ogre::Real)0.0);
    //
    //
    // // rendering cube, only using 14 vertices
    // const Ogre::Vector3 cube_strip[14] = {
    //     {-1.f, 1.f, 1.f},   // Front-top-left
    //     {1.f, 1.f, 1.f},    // Front-top-right
    //     {-1.f, -1.f, 1.f},  // Front-bottom-left
    //     {1.f, -1.f, 1.f},   // Front-bottom-right
    //     {1.f, -1.f, -1.f},  // Back-bottom-right
    //     {1.f, 1.f, 1.f},    // Front-top-right
    //     {1.f, 1.f, -1.f},   // Back-top-right
    //     {-1.f, 1.f, 1.f},   // Front-top-left
    //     {-1.f, 1.f, -1.f},  // Back-top-left
    //     {-1.f, -1.f, 1.f},  // Front-bottom-left
    //     {-1.f, -1.f, -1.f}, // Back-bottom-left
    //     {1.f, -1.f, -1.f},  // Back-bottom-right
    //     {-1.f, 1.f, -1.f},  // Back-top-left
    //     {1.f, 1.f, -1.f}    // Back-top-right
    // };
    //
    // for (const auto& vtx : cube_strip)
    // {
    //     mGizmoObj->position(vtx);
    // }
    //
    // mGizmoObj->end();

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

    mMesh->convertToMesh(name, "GizmoResources");

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

    mMesh->convertToMesh(name, "EditorResources");

    manager->destroyManualObject(mMesh);
}
}