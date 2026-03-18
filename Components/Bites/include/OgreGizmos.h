#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreBitesPrerequisites.h"
#include "OgreInput.h"
#include "OgreRay.h"

namespace OgreBites
{
enum GizmoOperation
{
    GO_TRANSLATE,
    GO_ROTATE,
    GO_SCALE,
};

/**
Class which applies a manipulable gizmo to a SceneNode
*/
class _OgreBitesExport Gizmo : public InputListener
{
public:
    Gizmo(Ogre::SceneNode* sceneNode, GizmoOperation op, Ogre::Viewport* viewport);

    void setOperation(GizmoOperation op);

    void setTargetNode(Ogre::SceneNode* target);

    GizmoOperation getOperation() { return mOperation; }

    bool mouseMoved(const MouseMotionEvent& evt) override;
    bool mousePressed(const MouseButtonEvent& evt) override;
    bool mouseReleased(const MouseButtonEvent& evt) override;

private:
    void scaleToParent();
    bool pickAxis(Ogre::Ray& ray);
    void highlightAxis(int axis);
    void startDrag(const Ogre::Ray& startRay, const Ogre::Vector3& cameraDir);
    void computeDrag(Ogre::Ray& ray, const Ogre::Vector3& cameraDir);
    void stopDrag();
    Ogre::Ray toLocalRay(const Ogre::Ray& worldRay) const;

    GizmoOperation mOperation;
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};

    // Gizmo nodes
    Ogre::SceneNode* mGizmoNode{};
    Ogre::SceneNode* mParentNode{};
    Ogre::Entity* mGizmoEntity{};

    // Scene info required for interaction
    Ogre::Viewport* mViewport{};

    // Drag state
    bool mDragging = false;
    int mActiveAxis = 0;
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

class _OgreBitesExport CameraGizmo : public InputListener
{
public:
    CameraGizmo(Ogre::Viewport* viewport);

    bool mouseMoved(const MouseMotionEvent& evt) override;
    bool mousePressed(const MouseButtonEvent& evt) override;
private:
    void highlightFace(int faceIndex);
    bool snapCamera(int faceIndex) const;
    int pickFace(float vx, float vy);
    void createMesh(Ogre::SceneManager* manager, Ogre::String name);

    Ogre::SceneNode* mGizmoNode{};
    Ogre::Camera* mCamera{};
    int mOldFaceIndex = -1;
    Ogre::SceneNode* mFaceNodes[6]{};
    Ogre::ManualObject* mGizmoObjects[6]{};
    float mOverlayLeft = 0.80f;
    float mOverlayTop = 0.0f;
    float mOverlayWidth = 0.20f;
    float mOverlayHeight = 0.20f;

    // Scene info required for interaction
    Ogre::Viewport* mViewport{};
};
} // namespace OgreBites
#endif // OGRE_OGREGIZMOS_H
