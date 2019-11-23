#include <Ogre.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreDotSceneLoader.h>
#include <OgreSceneLoaderManager.h>
#include <OgreComponents.h>

#include <pugixml.hpp>

using namespace Ogre;

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

        orientation.FromAngleAxis(Angle(angle), axis);
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
} // namespace

DotSceneLoader::DotSceneLoader() : mSceneMgr(0), mBackgroundColour(ColourValue::Black)
{
    SceneLoaderManager::getSingleton().registerSceneLoader("DotScene", {".scene"}, this);
}

DotSceneLoader::~DotSceneLoader()
{
    SceneLoaderManager::getSingleton().unregisterSceneLoader("DotScene");
}

void DotSceneLoader::load(DataStreamPtr& stream, const String& groupName, SceneNode* rootNode)
{
    m_sGroupName = groupName;
    mSceneMgr = rootNode->getCreator();

    pugi::xml_document XMLDoc; // character type defaults to char

    auto result = XMLDoc.load_buffer(stream->getAsString().c_str(), stream->size());
    if (!result)
    {
        LogManager::getSingleton().stream(LML_CRITICAL) << "[DotSceneLoader] " << result.description();
        return;
    }

    // Grab the scene node
    auto XMLRoot = XMLDoc.child("scene");

    // Validate the File
    if (!XMLRoot.attribute("formatVersion"))
    {
        LogManager::getSingleton().logError("[DotSceneLoader] Invalid .scene File. Missing <scene formatVersion='x.y' >");
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
    Real worldSize = getAttribReal(XMLNode, "worldSize");
    int mapSize = StringConverter::parseInt(XMLNode.attribute("size").value());
    int compositeMapDistance = StringConverter::parseInt(XMLNode.attribute("tuningCompositeMapDistance").value());
    int maxPixelError = StringConverter::parseInt(XMLNode.attribute("tuningMaxPixelError").value());

    auto terrainGlobalOptions = TerrainGlobalOptions::getSingletonPtr();
    OgreAssert(terrainGlobalOptions, "TerrainGlobalOptions not available");

    terrainGlobalOptions->setMaxPixelError((Real)maxPixelError);
    terrainGlobalOptions->setCompositeMapDistance((Real)compositeMapDistance);

    mTerrainGroup.reset(new TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z, mapSize, worldSize));
    mTerrainGroup->setOrigin(Vector3::ZERO);
    mTerrainGroup->setResourceGroup(m_sGroupName);

    // Process terrain pages (*)
    for (auto pPageElement : XMLNode.children("terrain"))
    {
        int pageX = StringConverter::parseInt(pPageElement.attribute("x").value());
        int pageY = StringConverter::parseInt(pPageElement.attribute("y").value());

        mTerrainGroup->defineTerrain(pageX, pageY, pPageElement.attribute("dataFile").value());
    }
    mTerrainGroup->loadAllTerrains(true);

    mTerrainGroup->freeTemporaryResources();
#else
    OGRE_EXCEPT(Exception::ERR_INVALID_CALL, "recompile with Terrain component");
#endif
}

void DotSceneLoader::processLight(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");

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
    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, pLight->getUserObjectBindings());
}

void DotSceneLoader::processCamera(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
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
}

void DotSceneLoader::processLookTarget(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    //! @todo Is this correct? Cause I don't have a clue actually

    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

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
    catch (Exception& /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a look target!");
    }
}

void DotSceneLoader::processTrackTarget(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

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
    catch (Exception& /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a track target!");
    }
}

void DotSceneLoader::processEntity(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String meshFile = getAttrib(XMLNode, "meshFile");
    bool castShadows = getAttribBool(XMLNode, "castShadows", true);

    // Create the entity
    Entity* pEntity = 0;
    try
    {
        pEntity = mSceneMgr->createEntity(name, meshFile, m_sGroupName);
        pEntity->setCastShadows(castShadows);
        pParent->attachObject(pEntity);

        if (auto material = XMLNode.attribute("material"))
            pEntity->setMaterialName(material.value());
    }
    catch (Exception& /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error loading an entity!", LML_CRITICAL);
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode.child("userData"))
        processUserData(pElement, pEntity->getUserObjectBindings());
}

void DotSceneLoader::processParticleSystem(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String templateName = getAttrib(XMLNode, "template");

    if (templateName.empty())
        templateName = getAttrib(XMLNode, "file"); // compatibility with old scenes

    // Create the particle system
    try
    {
        ParticleSystem* pParticles = mSceneMgr->createParticleSystem(name, templateName);
        pParent->attachObject(pParticles);
    }
    catch (Exception& /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error creating a particle system!");
    }
}

void DotSceneLoader::processBillboardSet(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    //! @todo Implement this
}

void DotSceneLoader::processPlane(pugi::xml_node& XMLNode, SceneNode* pParent)
{
    String name = getAttrib(XMLNode, "name");
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
    // Process attributes
    String material = getAttrib(XMLNode, "material", "BaseWhite");
    Real distance = getAttribReal(XMLNode, "distance", 5000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if (!active)
        return;

    // Process rotation (?)
    Quaternion rotation = Quaternion::IDENTITY;

    if (auto pElement = XMLNode.child("rotation"))
        rotation = parseQuaternion(pElement);

    // Setup the sky box
    mSceneMgr->setSkyBox(true, material, distance, drawFirst, rotation, m_sGroupName);
}

void DotSceneLoader::processSkyDome(pugi::xml_node& XMLNode)
{
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
    pLight->setSpotlightRange(Angle(inner), Angle(outer), falloff);
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

const Ogre::String& DotScenePlugin::getName() const {
    static Ogre::String name = "DotScene Loader";
    return name;
}

void DotScenePlugin::initialise() {
    mDotSceneLoader = new Ogre::DotSceneLoader();
}

void DotScenePlugin::shutdown() {
    delete mDotSceneLoader;
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
