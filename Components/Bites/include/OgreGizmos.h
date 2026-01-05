#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreBitesPrerequisites.h"
#include "OgreRay.h"
#include "OgreSceneNode.h"

namespace OgreBites
{

constexpr uint32_t QUERYFLAG_GIZMO = 1 << 1;

enum GizmoMode /// enum for different kinds of gizmo
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
    AXIS_X = 1 << 0, // 1
    AXIS_Y = 1 << 1, // 2
    AXIS_Z = 1 << 2, // 4

    AXIS_XY = AXIS_X | AXIS_Y, // 3
    AXIS_YZ = AXIS_Y | AXIS_Z, // 6
    AXIS_XZ = AXIS_X | AXIS_Z  // 5
};

/**
Class which applies a manipulable gizmo to a scene object (including the camera).
*/
class _OgreBitesExport Gizmo
{
public:
    Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode);

    void setMode(GizmoMode style);

    bool isGizmoEntity(Ogre::Entity* ent) const;

    void attachTo(Ogre::SceneNode* target);

    GizmoMode getMode() { return mMode; }

    bool pickAxis(Ogre::Ray& ray);

    void startDrag(const Ogre::Ray& startRay, const Ogre::Vector3& cameraDir);

    void computeDrag(Ogre::Ray& ray, const Ogre::Vector3& cameraDir);

    void stopDrag();

    bool isDragging() const;

protected:
    static void createMesh(Ogre::SceneManager* manager, Ogre::String name);

    static void createPlaneMesh(Ogre::SceneManager* manager, Ogre::String name);

    void scaleToParent();

    void highlightAxis(AXIS axis);

    static Ogre::Real rayLineDistance(const Ogre::Ray& ray, const Ogre::Vector3& lineOrigin,
                                      const Ogre::Vector3& lineDir, Ogre::Real lineLength);

    Ogre::Ray toLocalRay(const Ogre::Ray& worldRay) const;

    static bool pickRotateRing(const Ogre::Ray& ray, const Ogre::Vector3& center,
                               const Ogre::Vector3& axis,
                               Ogre::Real radius, Ogre::Real tolerance);

    static Ogre::Vector3 computePlaneHit(const Ogre::Ray& ray, const Ogre::Vector3& axis,
                                         const Ogre::Vector3& cameraDir, const Ogre::Vector3& planePoint);

    GizmoMode mMode;
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};

    // Gizmo nodes
    Ogre::SceneNode* mGizmoNode{};
    Ogre::SceneNode* mParentNode{};
    Ogre::SceneNode* mGizmoX{};
    Ogre::SceneNode* mGizmoY{};
    Ogre::SceneNode* mGizmoZ{};
    Ogre::Entity* mGizmoEntities[6]{};

    // Drag state
    bool mDragging = false;
    AXIS mActiveAxis = AXIS_NONE;
    Ogre::Vector3 mInitialObjectPos;
    Ogre::Quaternion mInitialObjectRot;
    Ogre::Vector3 mInitialObjectScale;
    Ogre::Vector3 mDragAxisLocal;
    Ogre::Vector3 mDragAxisWorld;
    Ogre::Vector3 mDragStartHitLocal;

    // Picking
    std::unordered_map<Ogre::Entity*, int> mEntityToAxis;
    int mOldGizmoAxis{};
};
} // namespace OgreBites
#endif // OGRE_OGREGIZMOS_H