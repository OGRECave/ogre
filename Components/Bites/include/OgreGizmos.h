#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreBitesPrerequisites.h"
#include "OgreCameraMan.h"
#include "OgreRay.h"

namespace OgreBites
{

constexpr uint32_t QUERYFLAG_GIZMO = 1 << 1;

enum GizmoMode /// enum for different kinds of gizmo
{
    G_NONE,
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

inline AXIS operator|(AXIS a, AXIS b)
{
    return static_cast<AXIS>(static_cast<int>(a) | static_cast<int>(b));
}

inline AXIS& operator|=(AXIS& a, AXIS b)
{
    a = a | b;
    return a;
}


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

private:
    static void createMesh(Ogre::SceneManager* manager, const Ogre::String& name,
                           const Ogre::ColourValue& xColor, const Ogre::ColourValue& yColor,
                           const Ogre::ColourValue& zColor,
                           const Ogre::ColourValue& xyColor, const Ogre::ColourValue& yzColor,
                           const Ogre::ColourValue& zxColor);

    void scaleToParent();

    void highlightAxis(AXIS axis);

    Ogre::Ray toLocalRay(const Ogre::Ray& worldRay) const;

    static Ogre::Vector3 computePlaneHit(const Ogre::Ray& ray, const Ogre::Vector3& axis,
                                         const Ogre::Vector3& cameraDir, const Ogre::Vector3& planePoint);

    GizmoMode mMode;
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};

    // Gizmo nodes
    Ogre::SceneNode* mGizmoNode{};
    Ogre::SceneNode* mParentNode{};
    Ogre::Entity* mGizmoEntity{};

    // Drag state
    bool mDragging = false;
    AXIS mActiveAxis = AXIS_NONE;
    Ogre::Vector3 mInitialObjectPos;
    Ogre::Quaternion mInitialObjectRot;
    Ogre::Vector3 mInitialObjectScale;
    Ogre::Vector3 mDragAxisLocal;
    Ogre::Vector3 mDragAxisWorld;
    Ogre::Vector3 mDragStartHitLocal;
    Ogre::Vector3 mDragLastHitLocal;
    Ogre::Radian mDragAccumulatedAngle{0};
    Ogre::Quaternion mDragGizmoOrientation = Ogre::Quaternion::IDENTITY;
    Ogre::Vector3 mDragGizmoCenter = Ogre::Vector3::ZERO;
    Ogre::Vector3 mDragPlaneNormalLocal;
    bool mDragUsePlane = false;

    // Picking
    int mOldGizmoAxis{};
};

class _OgreBitesExport CameraGizmo
{
public:
    CameraGizmo(
        Ogre::SceneNode* mSceneNode,
        CameraMan* cameraMan);

    void highlightFace(Ogre::ManualObject* face);

    void snapCamera(Ogre::ManualObject* face) const;

    void updateOrientation();

    Ogre::ManualObject* pickFace(float vx, float vy);

protected:
    void createMesh(Ogre::SceneManager* manager, Ogre::String name);

    Ogre::SceneNode* mGizmoNode{};
    Ogre::Camera* mCamera{};
    CameraMan* mCameraMan{};
    Ogre::SceneNode* mCameraNode{};
    int mOldFaceIndex = -1;
    Ogre::SceneNode* mFaceNodes[6]{};
    Ogre::ManualObject* mGizmoObjects[6]{};
    float mOverlayLeft = 0.80f;
    float mOverlayTop = 0.0f;
    float mOverlayWidth = 0.20f;
    float mOverlayHeight = 0.20f;
};
} // namespace OgreBites
#endif // OGRE_OGREGIZMOS_H
