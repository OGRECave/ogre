#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreSceneNode.h"

namespace OgreBites
{
enum GizmoMode   /// enum for different kinds of gizmo
{
    G_NONE,
    G_CAMERA,
    G_TRANSLATE,
    G_ROTATE,
    G_SCALE,
};

/**
Class which applies a manipulable gizmo to a scene object (including the camera).
*/
class Gizmo
{
public:
    Gizmo(Ogre::SceneNode* sceneNode, GizmoMode style);

    void setObject(Ogre::SceneNode* sceneObject);

    void setMode(GizmoMode style);

    Ogre::SceneNode* getObject()
    {
        return mSceneNode;
    }

    GizmoMode getStyle()
    {
        return mMode;
    }

protected:
    void createMesh(Ogre::SceneManager *manager, Ogre::String name);

    void createPlaneMesh(Ogre::SceneManager *manager, Ogre::String name);

    Ogre::SceneNode* mSceneNode{};
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};
    GizmoMode mMode;
};
}
#endif // OGRE_OGREGIZMOS_H