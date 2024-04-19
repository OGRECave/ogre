#include <Ogre.h>
#include <OgreDotSceneLoader.h>
#include <OgreComponents.h>

#ifdef OGRE_BUILD_COMPONENT_TERRAIN
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#endif

#include <pugixml.hpp>

using namespace Ogre;

#ifndef OGRE_BUILD_COMPONENT_TERRAIN
namespace Ogre {
    class TerrainGroup {}; // appease unique_ptr destructor
}
#endif

namespace
{
String getAttrib(const pugi::xml_node& XMLNode, const String& attrib, const String& defaultValue = "")
{
    if (auto anode = XMLNode.attribute(attrib.c_str()))
        return anode.value();
    else
        return defaultValue;
}

Real getAttribReal(const pugi::xml_node& XMLNode, const String& attrib, Real defaultValue = 0)
{
    if (auto anode = XMLNode.attribute(attrib.c_str()))
        return StringConverter::parseReal(anode.value());
    else
        return defaultValue;
}

bool getAttribBool(const pugi::xml_node& XMLNode, const String& attrib, bool defaultValue = false)
{
    if (auto anode = XMLNode.attribute(attrib.c_str()))
        return anode.as_bool();
    else
        return defaultValue;

    return false;
}

Vector3 parseVector3(const pugi::xml_node& XMLNode)
{
    return Vector3(StringConverter::parseReal(XMLNode.attribute("x").value()),
                   StringConverter::parseReal(XMLNode.attribute("y").value()),
                   StringConverter::parseReal(XMLNode.attribute("z").value()));
}

Quaternion parseQuaternion(const pugi::xml_node& XMLNode)
{
    //! @todo Fix this crap!

    Quaternion orientation;

    if (XMLNode.attribute("qw"))
    {
        orientation.w = StringConverter::parseReal(XMLNode.attribute("qw").value());
        orientation.x = StringConverter::parseReal(XMLNode.attribute("qx").value());
        orientation.y = StringConverter::parseReal(XMLNode.attribute("qy").value());
        orientation.z = StringConverter::parseReal(XMLNode.attribute("qz").value());
    }
    else if (XMLNode.attribute("axisX"))
    {
        Vector3 axis;
        axis.x = StringConverter::parseReal(XMLNode.attribute("axisX").value());
        axis.y = StringConverter::parseReal(XMLNode.attribute("axisY").value());
        axis.z = StringConverter::parseReal(XMLNode.attribute("axisZ").value());
        Real angle = StringConverter::parseReal(XMLNode.attribute("angle").value());

        orientation.FromAngleAxis(Radian(angle), axis);
    }
    else if (XMLNode.attribute("angleX"))
    {
        Matrix3 rot;
        rot.FromEulerAnglesXYZ(StringConverter::parseAngle(XMLNode.attribute("angleX").value()),
                               StringConverter::parseAngle(XMLNode.attribute("angleY").value()),
                               StringConverter::parseAngle(XMLNode.attribute("angleZ").value()));
        orientation.FromRotationMatrix(rot);
    }
    else if (XMLNode.attribute("x"))
    {
        orientation.x = StringConverter::parseReal(XMLNode.attribute("x").value());
        orientation.y = StringConverter::parseReal(XMLNode.attribute("y").value());
        orientation.z = StringConverter::parseReal(XMLNode.attribute("z").value());
        orientation.w = StringConverter::parseReal(XMLNode.attribute("w").value());
    }
    else if (XMLNode.attribute("w"))
    {
        orientation.w = StringConverter::parseReal(XMLNode.attribute("w").value());
        orientation.x = StringConverter::parseReal(XMLNode.attribute("x").value());
        orientation.y = StringConverter::parseReal(XMLNode.attribute("y").value());
        orientation.z = StringConverter::parseReal(XMLNode.attribute("z").value());
    }

    return orientation;
}

ColourValue parseColour(pugi::xml_node& XMLNode)
{
    return ColourValue(StringConverter::parseReal(XMLNode.attribute("r").value()),
                       StringConverter::parseReal(XMLNode.attribute("g").value()),
                       StringConverter::parseReal(XMLNode.attribute("b").value()),
                       XMLNode.attribute("a") != NULL ? StringConverter::parseReal(XMLNode.attribute("a").value()) : 1);
}

struct DotSceneCodec : public Codec
{
    String magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const override { return ""; }
    String getType() const override { return "scene"; }
    void decode(const DataStreamPtr& stream, const Any& output) const override
    {
        DataStreamPtr _stream(stream);
        DotSceneLoader loader;
        loader.load(_stream, ResourceGroupManager::getSingleton().getWorldResourceGroupName(),
                    any_cast<SceneNode*>(output));
    }

    void encodeToFile(const Any& input, const String& outFileName) const override
    {
        DotSceneLoader loader;
        loader.exportScene(any_cast<SceneNode*>(input), outFileName);
    }
};

} // namespace

DotSceneLoader::DotSceneLoader() : mSceneMgr(0), mBackgroundColour(ColourValue::Black) {}

DotSceneLoader::~DotSceneLoader() {}

void DotSceneLoader::load(DataStreamPtr& stream, const String& groupName, SceneNode* rootNode)
{
    m_sGroupName = groupName;
    mSceneMgr = rootNode->getCreator();

    pugi::xml_document XMLDoc; // character type defaults to char

    auto result = XMLDoc.load_buffer(stream->getAsString().c_str(), stream->size());
    if (!result)
    {
        LogManager::getSingleton().logError("DotSceneLoader - " + String(result.description()));
        return;
    }

    // Grab the scene node
    auto XMLRoot = XMLDoc.child("scene");

    // Validate the File
    if (!XMLRoot.attribute("formatVersion"))
    {
        LogManager::getSingleton().logError("DotSceneLoader - Invalid .scene File. Missing <scene formatVersion='x.y' >");
        return;
    }

    // figure out where to attach any nodes we create
    mAttachNode = rootNode;

    // Process the scene
    processScene(XMLRoot);
}

void DotSceneLoader::processScene(pugi::xml_node& XMLRoot)
{
    // Process the scene parameters
    String version = getAttrib(XMLRoot, "formatVersion", "unknown");

    String message = "[DotSceneLoader] Parsing dotScene file with version " + version;
    if (XMLRoot.attribute("sceneManager"))
        message += ", scene manager " + String(XMLRoot.attribute("sceneManager").value());
    if (XMLRoot.attribute("minOgreVersion"))
        message += ", min. Ogre version " + String(XMLRoot.attribute("minOgreVersion").value());
    if (XMLRoot.attribute("author"))
        message += ", author " + String(XMLRoot.attribute("author").value());

    LogManager::getSingleton().logMessage(message);

    // Process environment (?)
    if (auto pElement = XMLRoot.child("environment"))
        processEnvironment(pElement);

    // Process nodes (?)
    if (auto pElement = XMLRoot.child("nodes"))
        processNodes(pElement);

    // Process externals (?)
    if (auto pElement = XMLRoot.child("externals"))
        processExternals(pElement);

    // Process userDataReference (?)
    if (auto pElement = XMLRoot.child("userData"))
        processUserData(pElement, mAttachNode->getUserObjectBindings());

    // Process light (?)
    if (auto pElement = XMLRoot.child("light"))
        processLight(pElement);

    // Process camera (?)
    if (auto pElement = XMLRoot.child("camera"))
        processCamera(pElement);

    // Process terrain (?)
    if (auto pElement = XMLRoot.child("terrainGroup"))
        processTerrainGroup(pElement);
}

void DotSceneLoader::processNodes(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Nodes...", LML_TRIVIAL);

    // Process node (*)
    for (auto pElement : XMLNode.children("node"))
    {
        processNode(pElement);
    }

    // Process position (?)
    if (auto pElement = XMLNode.child("position"))
    {
        mAttachNode->setPosition(parseVector3(pElement));
        mAttachNode->setInitialState();
    }

    // Process rotation (?)
    if (auto pElement = XMLNode.child("rotation"))
    {
        mAttachNode->setOrientation(parseQuaternion(pElement));
        mAttachNode->setInitialState();
    }

    // Process scale (?)
    if (auto pElement = XMLNode.child("scale"))
    {
        mAttachNode->setScale(parseVector3(pElement));
        mAttachNode->setInitialState();
    }
}

void DotSceneLoader::processExternals(pugi::xml_node& XMLNode)
{
    //! @todo Implement this
}

void DotSceneLoader::processEnvironment(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Environment...", LML_TRIVIAL);

    // Process camera (?)
    if (auto pElement = XMLNode.child("camera"))
        processCamera(pElement);

    // Process fog (?)
    if (auto pElement = XMLNode.child("fog"))
        processFog(pElement);

    // Process skyBox (?)
    if (auto pElement = XMLNode.child("skyBox"))
        processSkyBox(pElement);

    // Process skyDome (?)
    if (auto pElement = XMLNode.child("skyDome"))
        processSkyDome(pElement);

    // Process skyPlane (?)
    if (auto pElement = XMLNode.child("skyPlane"))
        processSkyPlane(pElement);

    // Process colourAmbient (?)
    if (auto pElement = XMLNode.child("colourAmbient"))
        mSceneMgr->setAmbientLight(parseColour(pElement));

    // Process colourBackground (?)
    if (auto pElement = XMLNode.child("colourBackground"))
        mBackgroundColour = parseColour(pElement);
}

void DotSceneLoader::processTerrainGroup(pugi::xml_node& XMLNode)
{
#ifdef OGRE_BUILD_COMPONENT_TERRAIN
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Terrain Group...", LML_TRIVIAL);

    Real worldSize = getAttribReal(XMLNode, "worldSize");
    int mapSize = StringConverter::parseInt(XMLNode.attribute("size").value());
    int compositeMapDistance = StringConverter::parseInt(XMLNode.attribute("tuningCompositeMapDistance").value());
    int maxPixelError = StringConverter::parseInt(XMLNode.attribute("tuningMaxPixelError").value());

    auto terrainGlobalOptions = TerrainGlobalOptions::getSingletonPtr();
    OgreAssert(terrainGlobalOptions, "TerrainGlobalOptions not available");

    terrainGlobalOptions->setMaxPixelError((Real)maxPixelError);
    terrainGlobalOptions->setCompositeMapDistance((Real)compositeMapDistance);

    auto terrainGroup = std::make_shared<TerrainGroup>(mSceneMgr, Terrain::ALIGN_X_Z, mapSize, worldSize);
    terrainGroup->setOrigin(Vector3::ZERO);
    terrainGroup->setResourceGroup(m_sGroupName);

    // Process terrain pages (*)
    for (auto pPageElement : XMLNode.children("terrain"))
    {
        int pageX = StringConverter::parseInt(pPageElement.attribute("x").value());
        int pageY = StringConverter::parseInt(pPageElement.attribute("y").value());

        terrainGroup->defineTerrain(pageX, pageY, pPageElement.attribute("dataFile").value());
    }
    terrainGroup->loadAllTerrains(true);

    terrainGroup->freeTemporaryResources();

    mAttachNode->getUserObjectBindings().setUserAny("TerrainGroup", terrainGroup);
#else
    OGRE_EXCEPT(Exception::ERR_INVALID_CALL, "recompile with Terrain component");
#endif
}

void DotSceneLoader::processLight(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Light: " + name, LML_TRIVIAL);

    // Create the light
    Light* pLight = mSceneMgr->createLight(name);
    if (pParent)
        pParent->attachObject(pLight);

    String sValue = getAttrib(XMLNode, "type");
    if (sValue == "point")
        pLight->setType(Light::LT_POINT);
    else if (sValue == "directional")
        pLight->setType(Light::LT_DIRECTIONAL);
    else if (sValue == "spot")
        pLight->setType(Light::LT_SPOTLIGHT);
    else if (sValue == "radPoint")
        pLight->setType(Light::LT_POINT);
    else if (sValue == "rect")
        pLight->setType(Light::LT_RECTLIGHT);

    pLight->setVisible(getAttribBool(XMLNode, "visible", true));
    pLight->setCastShadows(getAttribBool(XMLNode, "castShadows", true));
    pLight->setPowerScale(getAttribReal(XMLNode, "powerScale", 1.0));

    // Process colourDiffuse (?)
    if (auto pElement = XMLNode.child("colourDiffuse"))
        pLight->setDiffuseColour(parseColour(pElement));

    // Process colourSpecular (?)
    if (auto pElement = XMLNode.child("colourSpecular"))
        pLight->setSpecularColour(parseColour(pElement));

    if (sValue != "directional")
    {
        // Process lightRange (?)
        if (auto pElement = XMLNode.child("lightRange"))
            processLightRange(pElement, pLight);

        // Process lightAttenuation (?)
        if (auto pElement = XMLNode.child("lightAttenuation"))
            processLightAttenuation(pElement, pLight);
    }

    if (sValue == "rect")
    {
        // Process lightSourceSize (?)
        if (auto pElement = XMLNode.child("lightSourceSize"))
            processLightSourceSize(pElement, pLight);
	}

    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, pLight->getUserObjectBindings());
}

void DotSceneLoader::processCamera(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Camera: " + name, LML_TRIVIAL);

    // Real fov = getAttribReal(XMLNode, "fov", 45);
    Real aspectRatio = getAttribReal(XMLNode, "aspectRatio", 1.3333);
    String projectionType = getAttrib(XMLNode, "projectionType", "perspective");

    // Create the camera
    Camera* pCamera = mSceneMgr->createCamera(name);

    // construct a scenenode is no parent
    if (!pParent)
        pParent = mAttachNode->createChildSceneNode(name);

    pParent->attachObject(pCamera);

    // Set the field-of-view
    //! @todo Is this always in degrees?
    // pCamera->setFOVy(Degree(fov));

    // Set the aspect ratio
    pCamera->setAspectRatio(aspectRatio);

    // Set the projection type
    if (projectionType == "perspective")
        pCamera->setProjectionType(PT_PERSPECTIVE);
    else if (projectionType == "orthographic")
        pCamera->setProjectionType(PT_ORTHOGRAPHIC);

    // Process clipping (?)
    if (auto pElement = XMLNode.child("clipping"))
    {
        Real nearDist = getAttribReal(pElement, "near");
        pCamera->setNearClipDistance(nearDist);

        Real farDist = getAttribReal(pElement, "far");
        pCamera->setFarClipDistance(farDist);
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, static_cast<MovableObject*>(pCamera)->getUserObjectBindings());
}

void DotSceneLoader::processNode(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Construct the node's name
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Node: " + name, LML_TRIVIAL);

    // Create the scene node
    SceneNode* pNode;
    if (name.empty())
    {
        // Let Ogre choose the name
        if (pParent)
            pNode = pParent->createChildSceneNode();
        else
            pNode = mAttachNode->createChildSceneNode();
    }
    else
    {
        // Provide the name
        if (pParent)
            pNode = pParent->createChildSceneNode(name);
        else
            pNode = mAttachNode->createChildSceneNode(name);
    }

    // Process other attributes

    // Process position (?)
    if (auto pElement = XMLNode.child("position"))
    {
        pNode->setPosition(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process rotation (?)
    if (auto pElement = XMLNode.child("rotation"))
    {
        pNode->setOrientation(parseQuaternion(pElement));
        pNode->setInitialState();
    }

    // Process scale (?)
    if (auto pElement = XMLNode.child("scale"))
    {
        pNode->setScale(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process lookTarget (?)
    if (auto pElement = XMLNode.child("lookTarget"))
        processLookTarget(pElement, pNode);

    // Process trackTarget (?)
    if (auto pElement = XMLNode.child("trackTarget"))
        processTrackTarget(pElement, pNode);

    // Process node (*)
    for (auto pElement : XMLNode.children("node"))
    {
        processNode(pElement, pNode);
    }

    // Process entity (*)
    for (auto pElement : XMLNode.children("entity"))
    {
        processEntity(pElement, pNode);
    }

    // Process light (*)
    for (auto pElement : XMLNode.children("light"))
    {
        processLight(pElement, pNode);
    }

    // Process camera (*)
    for (auto pElement : XMLNode.children("camera"))
    {
        processCamera(pElement, pNode);
    }

    // Process particleSystem (*)
    for (auto pElement : XMLNode.children("particleSystem"))
    {
        processParticleSystem(pElement, pNode);
    }

    // Process billboardSet (*)
    for (auto pElement : XMLNode.children("billboardSet"))
    {
        processBillboardSet(pElement, pNode);
    }

    // Process plane (*)
    for (auto pElement : XMLNode.children("plane"))
    {
        processPlane(pElement, pNode);
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, pNode->getUserObjectBindings());

    // Process node animations (?)
    if (auto pElement = XMLNode.child("animations"))
        processNodeAnimations(pElement, pNode);
}

void DotSceneLoader::processLookTarget(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    //! @todo Is this correct? Cause I don't have a clue actually

    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Look Target, nodeName: " + nodeName, LML_TRIVIAL);

    Node::TransformSpace relativeTo = Node::TS_PARENT;
    String sValue = getAttrib(XMLNode, "relativeTo");
    if (sValue == "local")
        relativeTo = Node::TS_LOCAL;
    else if (sValue == "parent")
        relativeTo = Node::TS_PARENT;
    else if (sValue == "world")
        relativeTo = Node::TS_WORLD;

    // Process position (?)
    Vector3 position;
    if (auto pElement = XMLNode.child("position"))
        position = parseVector3(pElement);

    // Process localDirection (?)
    Vector3 localDirection = Vector3::NEGATIVE_UNIT_Z;
    if (auto pElement = XMLNode.child("localDirection"))
        localDirection = parseVector3(pElement);

    // Setup the look target
    try
    {
        if (!nodeName.empty())
        {
            SceneNode* pLookNode = mSceneMgr->getSceneNode(nodeName);
            position = pLookNode->_getDerivedPosition();
        }

        pParent->lookAt(position, relativeTo, localDirection);
    }
    catch (const Exception& e)
    {
        LogManager::getSingleton().logError("DotSceneLoader - " + e.getDescription());
    }
}

void DotSceneLoader::processTrackTarget(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Track Target, nodeName: " + nodeName, LML_TRIVIAL);

    // Process localDirection (?)
    Vector3 localDirection = Vector3::NEGATIVE_UNIT_Z;
    if (auto pElement = XMLNode.child("localDirection"))
        localDirection = parseVector3(pElement);

    // Process offset (?)
    Vector3 offset = Vector3::ZERO;
    if (auto pElement = XMLNode.child("offset"))
        offset = parseVector3(pElement);

    // Setup the track target
    try
    {
        SceneNode* pTrackNode = mSceneMgr->getSceneNode(nodeName);
        pParent->setAutoTracking(true, pTrackNode, localDirection, offset);
    }
    catch (const Exception& e)
    {
        LogManager::getSingleton().logError("DotSceneLoader - " + e.getDescription());
    }
}

void DotSceneLoader::processEntity(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Entity: " + name, LML_TRIVIAL);

    String meshFile = getAttrib(XMLNode, "meshFile");
	String staticGeometry = getAttrib(XMLNode, "static");
	String instancedManager = getAttrib(XMLNode, "instanced");
	String material = getAttrib(XMLNode, "material");
    bool castShadows = getAttribBool(XMLNode, "castShadows", true);
    bool visible = getAttribBool(XMLNode, "visible", true);

    // Create the entity
	MovableObject* pEntity = 0;

    try
    {
		// If the Entity is instanced then the creation path is different
		if (!instancedManager.empty())
		{
			LogManager::getSingleton().logMessage("[DotSceneLoader] Adding entity: " + name + " to Instance Manager: " + instancedManager, LML_TRIVIAL);

			// Load the Mesh to get the material name of the first submesh
			Ogre::MeshPtr mesh = MeshManager::getSingletonPtr()->load(meshFile, m_sGroupName);

			// Get the material name of the entity
			if(!material.empty())
				pEntity = mSceneMgr->createInstancedEntity(material, instancedManager);
			else
				pEntity = mSceneMgr->createInstancedEntity(mesh->getSubMesh(0)->getMaterialName(), instancedManager);

			pParent->attachObject(static_cast<InstancedEntity*>(pEntity));
		}
		else
		{
			pEntity = mSceneMgr->createEntity(name, meshFile, m_sGroupName);

			static_cast<Entity*>(pEntity)->setCastShadows(castShadows);
			static_cast<Entity*>(pEntity)->setVisible(visible);

			if (!material.empty())
				static_cast<Entity*>(pEntity)->setMaterialName(material);

			// If the Entity belongs to a Static Geometry group then it doesn't get attached to a node
			// * TODO * : Clean up nodes without attached entities or children nodes? (should be done afterwards if the hierarchy is being processed)
			if (!staticGeometry.empty())
			{
				LogManager::getSingleton().logMessage("[DotSceneLoader] Adding entity: " + name + " to Static Group: " + staticGeometry, LML_TRIVIAL);
				mSceneMgr->getStaticGeometry(staticGeometry)->addEntity(static_cast<Entity*>(pEntity), pParent->_getDerivedPosition(), pParent->_getDerivedOrientation(), pParent->_getDerivedScale());
			}
			else
			{
				LogManager::getSingleton().logMessage("[DotSceneLoader] pParent->attachObject(): " + name, LML_TRIVIAL);
				pParent->attachObject(static_cast<Entity*>(pEntity));
			}
		}
    }
    catch (const Exception& e)
    {
        LogManager::getSingleton().logError("DotSceneLoader - " + e.getDescription());
        return;
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, pEntity->getUserObjectBindings());
}

void DotSceneLoader::processParticleSystem(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Particle System: " + name, LML_TRIVIAL);

    String templateName = getAttrib(XMLNode, "template");

    if (templateName.empty())
        templateName = getAttrib(XMLNode, "file"); // compatibility with old scenes

    // Create the particle system
    try
    {
        ParticleSystem* pParticles = mSceneMgr->createParticleSystem(name, templateName);
        pParent->attachObject(pParticles);
    }
    catch (const Exception& e)
    {
        LogManager::getSingleton().logError("DotSceneLoader - " + e.getDescription());
    }
}

void DotSceneLoader::processBillboardSet(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    //! @todo Implement this
}

void DotSceneLoader::processPlane(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Plane: " + name, LML_TRIVIAL);

    Real distance = getAttribReal(XMLNode, "distance");
    Real width = getAttribReal(XMLNode, "width");
    Real height = getAttribReal(XMLNode, "height");
    int xSegments = StringConverter::parseInt(getAttrib(XMLNode, "xSegments"));
    int ySegments = StringConverter::parseInt(getAttrib(XMLNode, "ySegments"));
    int numTexCoordSets = StringConverter::parseInt(getAttrib(XMLNode, "numTexCoordSets"));
    Real uTile = getAttribReal(XMLNode, "uTile");
    Real vTile = getAttribReal(XMLNode, "vTile");
    String material = getAttrib(XMLNode, "material");
    bool hasNormals = getAttribBool(XMLNode, "hasNormals");
    Vector3 normal = parseVector3(XMLNode.child("normal"));
    Vector3 up = parseVector3(XMLNode.child("upVector"));

    Plane plane(normal, distance);
    MeshPtr res =
        MeshManager::getSingletonPtr()->createPlane(name + "mesh", m_sGroupName, plane, width, height, xSegments,
                                                    ySegments, hasNormals, numTexCoordSets, uTile, vTile, up);
    Entity* ent = mSceneMgr->createEntity(name, name + "mesh");

    ent->setMaterialName(material);

    pParent->attachObject(ent);
}

void DotSceneLoader::processFog(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Fog...", LML_TRIVIAL);

    // Process attributes
    Real expDensity = getAttribReal(XMLNode, "density", 0.001);
    Real linearStart = getAttribReal(XMLNode, "start", 0.0);
    Real linearEnd = getAttribReal(XMLNode, "end", 1.0);

    FogMode mode = FOG_NONE;
    String sMode = getAttrib(XMLNode, "mode");
    if (sMode == "none")
        mode = FOG_NONE;
    else if (sMode == "exp")
        mode = FOG_EXP;
    else if (sMode == "exp2")
        mode = FOG_EXP2;
    else if (sMode == "linear")
        mode = FOG_LINEAR;
    else
        mode = (FogMode)StringConverter::parseInt(sMode);

    // Process colourDiffuse (?)
    ColourValue colourDiffuse = ColourValue::White;

    if (auto pElement = XMLNode.child("colour"))
        colourDiffuse = parseColour(pElement);

    // Setup the fog
    mSceneMgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
}

void DotSceneLoader::processSkyBox(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing SkyBox...", LML_TRIVIAL);

    // Process attributes
    String material = getAttrib(XMLNode, "material", "BaseWhite");
    Real distance = getAttribReal(XMLNode, "distance", 5000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

    // Process rotation (?)
    Quaternion rotation = Quaternion::IDENTITY;

    if (auto pElement = XMLNode.child("rotation"))
        rotation = parseQuaternion(pElement);

    // Setup the sky box
    mSceneMgr->setSkyBox(true, material, distance, drawFirst, rotation, m_sGroupName);
}

void DotSceneLoader::processSkyDome(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing SkyDome...", LML_TRIVIAL);

    // Process attributes
    String material = XMLNode.attribute("material").value();
    Real curvature = getAttribReal(XMLNode, "curvature", 10);
    Real tiling = getAttribReal(XMLNode, "tiling", 8);
    Real distance = getAttribReal(XMLNode, "distance", 4000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if (!active)
        return;

    // Process rotation (?)
    Quaternion rotation = Quaternion::IDENTITY;
    if (auto pElement = XMLNode.child("rotation"))
        rotation = parseQuaternion(pElement);

    // Setup the sky dome
    mSceneMgr->setSkyDome(true, material, curvature, tiling, distance, drawFirst, rotation, 16, 16, -1, m_sGroupName);
}

void DotSceneLoader::processSkyPlane(pugi::xml_node& XMLNode)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing SkyPlane...", LML_TRIVIAL);

    // Process attributes
    String material = getAttrib(XMLNode, "material");
    Real planeX = getAttribReal(XMLNode, "planeX", 0);
    Real planeY = getAttribReal(XMLNode, "planeY", -1);
    Real planeZ = getAttribReal(XMLNode, "planeZ", 0);
    Real planeD = getAttribReal(XMLNode, "planeD", 5000);
    Real scale = getAttribReal(XMLNode, "scale", 1000);
    Real bow = getAttribReal(XMLNode, "bow", 0);
    Real tiling = getAttribReal(XMLNode, "tiling", 10);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

    // Setup the sky plane
    Plane plane;
    plane.normal = Vector3(planeX, planeY, planeZ);
    plane.d = planeD;
    mSceneMgr->setSkyPlane(true, plane, material, scale, tiling, drawFirst, bow, 1, 1, m_sGroupName);
}

void DotSceneLoader::processLightRange(pugi::xml_node& XMLNode, Light* pLight)
{
    // Process attributes
    Real inner = getAttribReal(XMLNode, "inner");
    Real outer = getAttribReal(XMLNode, "outer");
    Real falloff = getAttribReal(XMLNode, "falloff", 1.0);

    // Setup the light range
    pLight->setSpotlightRange(Radian(inner), Radian(outer), falloff);
}

void DotSceneLoader::processLightAttenuation(pugi::xml_node& XMLNode, Light* pLight)
{
    // Process attributes
    Real range = getAttribReal(XMLNode, "range");
    Real constant = getAttribReal(XMLNode, "constant");
    Real linear = getAttribReal(XMLNode, "linear");
    Real quadratic = getAttribReal(XMLNode, "quadratic");

    // Setup the light attenuation
    pLight->setAttenuation(range, constant, linear, quadratic);
}

void DotSceneLoader::processLightSourceSize(pugi::xml_node& XMLNode, Light* pLight)
{
    // Process attributes
    Real width = getAttribReal(XMLNode, "width");
    Real height = getAttribReal(XMLNode, "height");

    // Setup the light range
    pLight->setSourceSize(width, height);
}

void DotSceneLoader::processUserData(pugi::xml_node& XMLNode, UserObjectBindings& userData)
{
    // Process node (*)
    for (auto pElement : XMLNode.children("property"))
    {
        String name = getAttrib(pElement, "name");
        String type = getAttrib(pElement, "type");
        String data = getAttrib(pElement, "data");

        Any value;
        if (type == "bool")
            value = StringConverter::parseBool(data);
        else if (type == "float")
            value = StringConverter::parseReal(data);
        else if (type == "int")
            value = StringConverter::parseInt(data);
        else
            value = data;

        userData.setUserAny(name, value);
    }
}

void DotSceneLoader::processNodeAnimations(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Node Animations for SceneNode: " + pParent->getName(), LML_TRIVIAL);

    // Process node animations (*)
    for (auto pElement : XMLNode.children("animation"))
    {
        processNodeAnimation(pElement, pParent);
    }
}

void DotSceneLoader::processNodeAnimation(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process node animation (*)

    // Construct the animation name
    String name = getAttrib(XMLNode, "name");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Node Animation: " + name, LML_TRIVIAL);

    Real length = getAttribReal(XMLNode, "length");

    Animation* anim = mSceneMgr->createAnimation(name, length);

    bool enable = getAttribBool(XMLNode, "enable", false);
    bool loop = getAttribBool(XMLNode, "loop", false);

    String interpolationMode = getAttrib(XMLNode, "interpolationMode");

    if (interpolationMode == "linear")
        anim->setInterpolationMode(Animation::IM_LINEAR);
    else if (interpolationMode == "spline")
        anim->setInterpolationMode(Animation::IM_SPLINE);
    else
        LogManager::getSingleton().logError("DotSceneLoader - Invalid interpolationMode: " + interpolationMode);

    String rotationInterpolationMode = getAttrib(XMLNode, "rotationInterpolationMode");

    if (rotationInterpolationMode == "linear")
        anim->setRotationInterpolationMode(Animation::RIM_LINEAR);
    else if (rotationInterpolationMode == "spherical")
        anim->setRotationInterpolationMode(Animation::RIM_SPHERICAL);
    else
        LogManager::getSingleton().logError("DotSceneLoader - Invalid rotationInterpolationMode: " + rotationInterpolationMode);

    // create a track to animate the camera's node
    NodeAnimationTrack* track = anim->createNodeTrack(0, pParent);

    // Process keyframes (*)
    for (auto pElement : XMLNode.children("keyframe"))
    {
        processKeyframe(pElement, track);
    }

    // create a new animation state to track this
    auto animState = mSceneMgr->createAnimationState(name);
    animState->setEnabled(enable);
    animState->setLoop(loop);
}

void DotSceneLoader::processKeyframe(pugi::xml_node& XMLNode, NodeAnimationTrack* pTrack)
{
    // Process node animation keyframe (*)
    Real time = getAttribReal(XMLNode, "time");

    LogManager::getSingleton().logMessage("[DotSceneLoader] Processing Keyframe: " + StringConverter::toString(time), LML_TRIVIAL);

    auto keyframe = pTrack->createNodeKeyFrame(time);

    // Process translation (?)
    if (auto pElement = XMLNode.child("position")) {
        Vector3 translation = parseVector3(pElement);
        keyframe->setTranslate(translation);
    }

    // Process rotation (?)
    //Quaternion rotation = Quaternion::IDENTITY;
    if (auto pElement = XMLNode.child("rotation")) {
        Quaternion rotation = parseQuaternion(pElement);
        keyframe->setRotation(rotation);
    }

    // Process scale (?)
    //Vector3 scale = parseVector3(XMLNode.child("scale"));
    if (auto pElement = XMLNode.child("scale")) {
        Vector3 scale = parseVector3(pElement);
        keyframe->setScale(scale);
    }
}

void DotSceneLoader::exportScene(SceneNode* rootNode, const String& outFileName)
{
    pugi::xml_document XMLDoc; // character type defaults to char
    auto comment = XMLDoc.append_child(pugi::node_comment);
    comment.set_value(StringUtil::format(" exporter: Plugin_DotScene %d.%d.%d ", OGRE_VERSION_MAJOR,
                                         OGRE_VERSION_MINOR, OGRE_VERSION_PATCH)
                          .c_str());
    auto scene = XMLDoc.append_child("scene");
    scene.append_attribute("formatVersion") = "1.1";
    scene.append_attribute("sceneManager") = rootNode->getCreator()->getTypeName().c_str();

    auto nodes = scene.append_child("nodes");

    for(auto c : rootNode->getChildren())
        writeNode(nodes, static_cast<SceneNode*>(c));

    //writeNode(nodes, rootNode);

    XMLDoc.save_file(outFileName.c_str());
}

static void write(pugi::xml_node& node, const Vector3& v)
{
    node.append_attribute("x") = StringConverter::toString(v.x).c_str();
    node.append_attribute("y") = StringConverter::toString(v.y).c_str();
    node.append_attribute("z") = StringConverter::toString(v.z).c_str();
}

static void write(pugi::xml_node& node, const ColourValue& c)
{
    node.append_attribute("r") = StringConverter::toString(c.r).c_str();
    node.append_attribute("g") = StringConverter::toString(c.g).c_str();
    node.append_attribute("b") = StringConverter::toString(c.b).c_str();
    node.append_attribute("a") = StringConverter::toString(c.a).c_str();
}

void DotSceneLoader::writeNode(pugi::xml_node& parentXML, const SceneNode* n)
{
    auto nodeXML = parentXML.append_child("node");
    if(!n->getName().empty())
        nodeXML.append_attribute("name") = n->getName().c_str();

    auto pos = nodeXML.append_child("position");
    write(pos, n->getPosition());

    auto scale = nodeXML.append_child("scale");
    write(scale, n->getScale());

    auto rot = nodeXML.append_child("rotation");
    rot.append_attribute("qw") = StringConverter::toString(n->getOrientation().w).c_str();
    rot.append_attribute("qx") = StringConverter::toString(n->getOrientation().x).c_str();
    rot.append_attribute("qy") = StringConverter::toString(n->getOrientation().y).c_str();
    rot.append_attribute("qz") = StringConverter::toString(n->getOrientation().z).c_str();

    for(auto mo : n->getAttachedObjects())
    {
        if(auto c = dynamic_cast<Camera*>(mo))
        {
            auto camera = nodeXML.append_child("camera");
            camera.append_attribute("name") = c->getName().c_str();
            auto clipping = camera.append_child("clipping");
            clipping.append_attribute("near") = StringConverter::toString(c->getNearClipDistance()).c_str();
            clipping.append_attribute("far") = StringConverter::toString(c->getFarClipDistance()).c_str();
            continue;
        }

        if (auto l = dynamic_cast<Light*>(mo))
        {
            auto light = nodeXML.append_child("light");
            light.append_attribute("name") = l->getName().c_str();
            light.append_attribute("castShadows") = StringConverter::toString(l->getCastShadows()).c_str();

            if(!l->isVisible())
                light.append_attribute("visible") = "false";

            auto diffuse = light.append_child("colourDiffuse");
            write(diffuse, l->getDiffuseColour());
            auto specular = light.append_child("colourSpecular");
            write(specular, l->getSpecularColour());
            switch (l->getType())
            {
            case Light::LT_POINT:
                light.append_attribute("type") = "point";
                break;
            case Light::LT_DIRECTIONAL:
                light.append_attribute("type") = "directional";
                break;
            case Light::LT_SPOTLIGHT:
                light.append_attribute("type") = "spot";
                break;
            case Light::LT_RECTLIGHT:
                light.append_attribute("type") = "rect";
                break;
            }

            if(l->getType() != Light::LT_DIRECTIONAL)
            {
                auto range = light.append_child("lightRange");
                range.append_attribute("inner") =
                    StringConverter::toString(l->getSpotlightInnerAngle()).c_str();
                range.append_attribute("outer") =
                    StringConverter::toString(l->getSpotlightOuterAngle()).c_str();
                range.append_attribute("falloff") =
                    StringConverter::toString(l->getSpotlightFalloff()).c_str();
                auto atten = light.append_child("lightAttenuation");
                atten.append_attribute("range") =
                    StringConverter::toString(l->getAttenuationRange()).c_str();
                atten.append_attribute("constant") =
                    StringConverter::toString(l->getAttenuationConstant()).c_str();
                atten.append_attribute("linear") =
                    StringConverter::toString(l->getAttenuationLinear()).c_str();
                atten.append_attribute("quadratic") =
                    StringConverter::toString(l->getAttenuationQuadric()).c_str();
            }

            continue;
        }

        if(auto e = dynamic_cast<Entity*>(mo))
        {
            auto entity = nodeXML.append_child("entity");
            entity.append_attribute("name") = e->getName().c_str();
            entity.append_attribute("meshFile") = e->getMesh()->getName().c_str();

            if(!e->isVisible())
                entity.append_attribute("visible") = "false";

            // Heuristic: assume first submesh is representative
            auto sub0mat = e->getSubEntity(0)->getMaterial();
            if(sub0mat != e->getMesh()->getSubMesh(0)->getMaterial())
                entity.append_attribute("material") = sub0mat->getName().c_str();
            continue;
        }

        LogManager::getSingleton().logWarning("DotSceneLoader - unsupported MovableType " +
                                            mo->getMovableType());
    }

    // recurse
    for(auto c : n->getChildren())
        writeNode(nodeXML, static_cast<SceneNode*>(c));
}

const Ogre::String& DotScenePlugin::getName() const {
    static Ogre::String name = "DotScene Loader";
    return name;
}

void DotScenePlugin::initialise() {
    mCodec = new DotSceneCodec();
    Codec::registerCodec(mCodec);
}

void DotScenePlugin::shutdown() {
    Codec::unregisterCodec(mCodec);
    delete mCodec;
}

#ifndef OGRE_STATIC_LIB
    extern "C" _OgreDotScenePluginExport void dllStartPlugin();
    extern "C" _OgreDotScenePluginExport void dllStopPlugin();

    static DotScenePlugin plugin;

    extern "C" _OgreDotScenePluginExport void dllStartPlugin()
    {
        Ogre::Root::getSingleton().installPlugin(&plugin);
    }
    extern "C" _OgreDotScenePluginExport void dllStopPlugin()
    {
        Ogre::Root::getSingleton().uninstallPlugin(&plugin);
    }
#endif
