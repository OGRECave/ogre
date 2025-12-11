#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreBitesPrerequisites.h"
#include "OgreRay.h"
#include "OgreSceneNode.h"

namespace OgreBites
{

constexpr uint32_t QUERYFLAG_GIZMO       = 1 << 1;

enum GizmoMode   /// enum for different kinds of gizmo
{
    G_NONE,
    G_AXIS,
    G_TRANSLATE,
    G_ROTATE,
    G_SCALE,
};

enum AXIS
{
    AXIS_NONE = 0,
    AXIS_X    = 1 << 0, // 1
    AXIS_Y    = 1 << 1, // 2
    AXIS_Z    = 1 << 2, // 4

    AXIS_XY   = AXIS_X | AXIS_Y, // 3
    AXIS_YZ   = AXIS_Y | AXIS_Z, // 6
    AXIS_XZ   = AXIS_X | AXIS_Z  // 5
};


/**
Class which applies a manipulable gizmo to a scene object (including the camera).
*/
class _OgreBitesExport Gizmo
{
public:
    Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode);
    void setObject(Ogre::SceneNode* sceneObject);

    void setMode(GizmoMode style);

    void attachTo(Ogre::SceneNode* target);

    void setHighlighted(Ogre::Entity* highlighted);

    Ogre::SceneNode* getObject()
    {
        return mGizmoNode->getParentSceneNode();
    }

    GizmoMode getMode()
    {
        return mMode;
    }

    void startDrag(Ogre::Entity* pickedGizmo, const Ogre::Ray& startRay, const Ogre::Vector3& cameraDir);

    void computeDrag(const Ogre::Ray& ray, const Ogre::Vector3& cameraDir);

    void stopDrag();

    bool isDragging();

protected:
    static void createMesh(Ogre::SceneManager *manager, Ogre::String name);

    static void createPlaneMesh(Ogre::SceneManager *manager, Ogre::String name);

    Ogre::Vector3 computePlaneHit(const Ogre::Ray& ray, const Ogre::Vector3& axis, const Ogre::Vector3& cameraDir,
                                  const Ogre::Vector3& planePoint);

    bool mDragging = false;
    Ogre::Ray mDragStartRay;
    Ogre::Entity* mActiveGizmo = nullptr;
    Ogre::Vector3 mInitialObjectPos;
    Ogre::SceneNode* mGizmoNode{};
    Ogre::SceneNode* mGizmoX{};
    Ogre::SceneNode* mGizmoY{};
    Ogre::SceneNode* mGizmoZ{};
    Ogre::Entity* mGizmoEntities[6]{};
    std::unordered_map<Ogre::Entity*, int> mEntityToAxis;
    Ogre::Entity* mHighlighted = nullptr;
    int mOldGizmoAxis{};
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};
    GizmoMode mMode;
    Ogre::Quaternion mInitialObjectRot;
    Ogre::Vector3 mInitialObjectScale;
    Ogre::Vector3 mDragStartHitPos;
    Ogre::Vector3 mDragAxis;
};
}
#endif // OGRE_OGREGIZMOS_H