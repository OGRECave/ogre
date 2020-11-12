#ifndef DOT_SCENELOADER_H
#define DOT_SCENELOADER_H

// Includes
#include <OgreDotScenePluginExports.h>
#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include <OgreResourceGroupManager.h>
#include <OgreSceneLoader.h>
#include <OgreString.h>
#include <OgrePlugin.h>
#include <OgreCodec.h>

namespace pugi
{
class xml_node;
}

// Forward declarations
namespace Ogre
{
class SceneManager;
class SceneNode;

class _OgreDotScenePluginExport DotSceneLoader : public Ogre::SceneLoader
{
public:
    DotSceneLoader();
    virtual ~DotSceneLoader();

    void load(Ogre::DataStreamPtr& stream, const Ogre::String& groupName, Ogre::SceneNode* rootNode);

    const Ogre::ColourValue& getBackgroundColour() { return mBackgroundColour; }

protected:
    void processScene(pugi::xml_node& XMLRoot);

    void processNodes(pugi::xml_node& XMLNode);
    void processExternals(pugi::xml_node& XMLNode);
    void processEnvironment(pugi::xml_node& XMLNode);
    void processTerrainGroup(pugi::xml_node& XMLNode);
    void processBlendmaps(pugi::xml_node& XMLNode);
    void processUserData(pugi::xml_node& XMLNode, Ogre::UserObjectBindings& userData);
    void processLight(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent = 0);
    void processCamera(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent = 0);

    void processNode(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent = 0);
    void processLookTarget(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);
    void processTrackTarget(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);
    void processEntity(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);
    void processParticleSystem(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);
    void processBillboardSet(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);
    void processPlane(pugi::xml_node& XMLNode, Ogre::SceneNode* pParent);

    void processFog(pugi::xml_node& XMLNode);
    void processSkyBox(pugi::xml_node& XMLNode);
    void processSkyDome(pugi::xml_node& XMLNode);
    void processSkyPlane(pugi::xml_node& XMLNode);

    void processLightRange(pugi::xml_node& XMLNode, Ogre::Light* pLight);
    void processLightAttenuation(pugi::xml_node& XMLNode, Ogre::Light* pLight);

    Ogre::SceneManager* mSceneMgr;
    Ogre::SceneNode* mAttachNode;
    Ogre::String m_sGroupName;
    Ogre::ColourValue mBackgroundColour;
};

class DotScenePlugin : public Plugin
{
    const String& getName() const;

    void install() {}
    void initialise();
    void shutdown();
    void uninstall() {}
private:
    Codec* mCodec;
};
} // namespace Ogre
#endif // DOT_SCENELOADER_H
