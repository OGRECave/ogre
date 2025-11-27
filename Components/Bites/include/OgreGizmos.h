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

/**
Class which applies a manipulable gizmo to a scene object (including the camera).
*/
class _OgreBitesExport Gizmo
{
public:
    Gizmo(Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode, GizmoMode mode);
    void setObject(Ogre::SceneNode* sceneObject);

    void setMode(GizmoMode style);

    Ogre::SceneNode* getObject()
    {
        return mGizmoNode->getParentSceneNode();
    }

    GizmoMode getMode()
    {
        return mMode;
    }

protected:
    void createMesh(Ogre::SceneManager *manager, Ogre::String name);

    void createPlaneMesh(Ogre::SceneManager *manager, Ogre::String name);

    Ogre::SceneNode* mGizmoNode{};
    Ogre::SceneNode* mGizmoX{};
    Ogre::SceneNode* mGizmoY{};
    Ogre::SceneNode* mGizmoZ{};
    Ogre::Entity* mGizmoEntities[6]{};
    std::unique_ptr<Ogre::ManualObject> mGizmoObj{};
    GizmoMode mMode;
};
}
#endif // OGRE_OGREGIZMOS_H