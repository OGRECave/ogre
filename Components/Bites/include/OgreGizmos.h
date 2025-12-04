#ifndef OGRE_OGREGIZMOS_H
#define OGRE_OGREGIZMOS_H
#include "OgreSceneNode.h"
#include "OgreBitesPrerequisites.h"

namespace OgreBites
{
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

    void setHighlighted(Ogre::Entity* highlighted);

    Ogre::SceneNode* getObject()
    {
        return mGizmoNode->getParentSceneNode();
    }

    GizmoMode getMode()
    {
        return mMode;
    }

    Ogre::Vector3 computeDrag(Ogre::Ray ray);

protected:
    void createMesh(Ogre::SceneManager *manager, Ogre::String name);

    void createPlaneMesh(Ogre::SceneManager *manager, Ogre::String name);

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
};
}
#endif // OGRE_OGREGIZMOS_H