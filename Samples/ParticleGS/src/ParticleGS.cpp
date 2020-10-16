/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

  You may use this sample code for anything you like, it is not covered by the
  same license as the rest of the engine.
  -----------------------------------------------------------------------------
*/
/*
  -----------------------------------------------------------------------------
  Filename:    ParticleGS.cpp
  Description: Demonstrates the use of the geometry shader and render to vertex
  buffer to create a particle system that is entirely calculated on the GPU.
  Partial implementation of ParticlesGS example from Microsoft's DirectX 10
  SDK : http://msdn.microsoft.com/en-us/library/ee416421.aspx
  -----------------------------------------------------------------------------
*/

#include "ParticleGS.h"

using namespace Ogre;
using namespace OgreBites;

namespace OgreBites {

#ifdef LOG_GENERATED_BUFFER
struct FireworkParticle
{
    float pos[3];
    float timer;
    float type;
    float vel[3];
};
#endif

    Sample_ParticleGS::Sample_ParticleGS()
    {
        mInfo["Title"] = "Geometry Shader Particle System";
        mInfo["Description"] = "A demo of particle systems using geometry shaders and render to vertex buffers.";
        mInfo["Thumbnail"] = "thumb_particlegs.png";
        mInfo["Category"] = "Effects";
    }

    void Sample_ParticleGS::createProceduralParticleSystem()
    {
        mParticleSystem = static_cast<ProceduralManualObject*>
            (mSceneMgr->createMovableObject("ParticleGSEntity", ProceduralManualObjectFactory::FACTORY_TYPE_NAME));
        MaterialPtr mat = MaterialManager::getSingleton().getByName("Ogre/ParticleGS/Display", "General");
        mParticleSystem->setMaterial(mat);

        // Generate the geometry that will seed the particle system.
        ManualObject* particleSystemSeed = mSceneMgr->createManualObject("ParticleSeed");
        // This needs to be the initial launcher particle.
        particleSystemSeed->begin("Ogre/ParticleGS/Display", RenderOperation::OT_POINT_LIST);
        particleSystemSeed->position(0,0,0); // Position
        particleSystemSeed->textureCoord(1); // Timer
        particleSystemSeed->textureCoord(0); // Type
        particleSystemSeed->textureCoord(0,0,0); // Velocity
        particleSystemSeed->end();

        // Generate the RenderToBufferObject.
        RenderToVertexBufferSharedPtr r2vb =
            HardwareBufferManager::getSingleton().createRenderToVertexBuffer();
        r2vb->setRenderToBufferMaterialName("Ogre/ParticleGS/Generate");

        r2vb->setOperationType(RenderOperation::OT_POINT_LIST);
        r2vb->setMaxVertexCount(16000);
        r2vb->setResetsEveryUpdate(false);
        VertexDeclaration* vertexDecl = r2vb->getVertexDeclaration();
        // Define input / feedback variables.
        size_t offset = 0;
        // Position
        offset += vertexDecl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
        // Timer
        offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 0).getSize();
        // Type
        offset += vertexDecl->addElement(0, offset, VET_FLOAT1, VES_TEXTURE_COORDINATES, 1).getSize();
        // Velocity
        vertexDecl->addElement(0, offset, VET_FLOAT3, VES_TEXTURE_COORDINATES, 2).getSize();

        // Apply the random texture.
        TexturePtr randomTexture = RandomTools::generateRandomVelocityTexture();
        r2vb->getRenderToBufferMaterial()->getBestTechnique()->getPass(0)->
            getTextureUnitState("RandomTexture")->setTextureName(
                randomTexture->getName(), randomTexture->getTextureType());

        // Bind the two together.
        mParticleSystem->setRenderToVertexBuffer(r2vb);
        mParticleSystem->setManualObject(particleSystemSeed);

        // GpuProgramParametersSharedPtr geomParams = mParticleSystem->
        //     getRenderToVertexBuffer()->getRenderToBufferMaterial()->
        //     getBestTechnique()->getPass(0)->getGeometryProgramParameters();
        // if (geomParams->_findNamedConstantDefinition("randomTexture"))
        // {
        //     geomParams->setNamedConstant("randomTexture", 0);
        // }

        // Set bounds.
        AxisAlignedBox aabb;
        aabb.setMinimum(-100,-100,-100);
        aabb.setMaximum(100,100,100);
        mParticleSystem->setBoundingBox(aabb);
    }

    void Sample_ParticleGS::testCapabilities(const RenderSystemCapabilities* caps)
    {
        if (!caps->hasCapability(RSC_GEOMETRY_PROGRAM))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support geometry programs, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_ParticleGS::createScene");
        }
        if (!caps->hasCapability(RSC_HWRENDER_TO_VERTEX_BUFFER))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Your render system / hardware does not support render to vertex buffers, "
                        "so you cannot run this sample. Sorry!",
                        "Sample_ParticleGS::createScene");
        }
    }

    void Sample_ParticleGS::setupContent(void)
    {
        mCameraNode->setPosition(0,35,-100);
        mCameraNode->lookAt(Vector3(0,35,0), Node::TS_PARENT);

        mSceneMgr->setAmbientLight(ColourValue(0.7, 0.7, 0.7));

        mProceduralManualObjectFactory = OGRE_NEW ProceduralManualObjectFactory();
        Root::getSingleton().addMovableObjectFactory(mProceduralManualObjectFactory);

        createProceduralParticleSystem();
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mParticleSystem);
        // mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mParticleSystem->getManualObject());

        // Add an ogre head to the scene.
        SceneNode* ogreHeadSN = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        Entity *ogreHead = mSceneMgr->createEntity("head", "ogrehead.mesh");
        ogreHeadSN->scale(0.1,0.1,0.1);
        ogreHeadSN->yaw(Degree(180));
        ogreHeadSN->attachObject(ogreHead);

        // Add a plane to the scene.
        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
                                                1500,1500,20,20,true,1,60,60,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("Examples/Rockwall");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,95,0))->attachObject(pPlaneEnt);
    }

    void Sample_ParticleGS::cleanupContent()
    {
        Root::getSingleton().removeMovableObjectFactory(mProceduralManualObjectFactory);
        OGRE_DELETE mProceduralManualObjectFactory;
        mProceduralManualObjectFactory = 0;

        MeshManager::getSingleton().remove("Myplane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }

#ifdef LOG_GENERATED_BUFFER
    bool Sample_ParticleGS::frameEnded(const FrameEvent& evt)
    {
        // This will only work if the vertex buffer usage is dynamic
        // (see R2VB implementation).
        LogManager::getSingleton().getDefaultLog()->stream() <<
            "Particle system for frame " <<     Root::getSingleton().getNextFrameNumber();
        RenderOperation renderOp;
        mParticleSystem->getRenderToVertexBuffer()->getRenderOperation(renderOp);
        const HardwareVertexBufferSharedPtr& vertexBuffer =
            renderOp.vertexData->vertexBufferBinding->getBuffer(0);

        assert(vertexBuffer->getVertexSize() == sizeof(FireworkParticle));
        FireworkParticle* particles = static_cast<FireworkParticle*>
            (vertexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
        int vertexCount = renderOp.vertexData->vertexCount;

        for (size_t i = 0; i < renderOp.vertexData->vertexCount; i++)
        {
            FireworkParticle& p = particles[i];
            LogManager::getSingleton().getDefaultLog()->stream() <<
                "FireworkParticle " << i + 1 << " : " <<
                "Position : " << p.pos[0] << " " << p.pos[1] << " " << p.pos[2] << " , " <<
                "Timer : " << p.timer << " , " <<
                "Type : " << p.type << " , " <<
                "Velocity : " << p.vel[0] << " " << p.vel[1] << " " << p.vel[2];
        }
        
        vertexBuffer->unlock();
        return SdkSample::frameEnded(evt);
    }
#endif
}
