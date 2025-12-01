#ifndef OGRE_GIZMOS_H
#define OGRE_GIZMOS_H

#include "OgreGizmos.h"
#include "SdkSample.h"

using namespace Ogre;
using namespace OgreBites;

class _OgreSampleClassExport Sample_Gizmos : public SdkSample
{
    public:

    Sample_Gizmos()
    {
        mInfo["Title"] = "Gizmos";
        mInfo["Description"] = "A demo of gizmos for manipulating a scene.";
        mInfo["Thumbnail"] = "thumb_cel.png";
        mInfo["Category"] = "Other";
    }

    protected:

    void setupContent() override
    {
        mViewport->setBackgroundColour(ColourValue::White);

        // set our camera to orbit around the origin and show cursor
        mCameraMan->setStyle(CS_ORBIT);
        mTrayMgr->showCursor();

        // attach the light to a pivot node
        mLightPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // create a basic point light with an offset
        Light* light = mSceneMgr->createLight();
        mLightPivot->createChildSceneNode(Vector3(20, 40, 50))->attachObject(light);

        // create our model, give it the shader material, and place it at the origin
        SceneNode* gizmoParent = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mGizmo = new Gizmo(getSceneManager(), gizmoParent, G_ROTATE);

        // create a checkbox to toggle light movement
        mTranslate = mTrayMgr->createButton(TL_TOPLEFT, "Translate", "Translate");
        mRotate = mTrayMgr->createButton(TL_TOPLEFT, "Rotate", "Rotate");
        mScale = mTrayMgr->createButton(TL_TOPLEFT, "Scale", "Scale");
    }

    bool mousePressed(const OgreBites::MouseButtonEvent& evt) override
    {
        pickObject(evt.x, evt.y);
        return true;
    }

    private:

     Ogre::SubEntity* pickObject(float x, float y)
    {
        Ogre::Ray ray = Sample::mCamera->getCameraToViewportRay(x, y);

        Ogre::RaySceneQuery* query = mSceneMgr->createRayQuery(ray);
        query->setSortByDistance(true);

        Ogre::RaySceneQueryResult& hits = query->execute();
        for (auto& h : hits)
        {
            if (!h.movable) continue;
            Ogre::Entity* ent = dynamic_cast<Ogre::Entity*>(h.movable);
            if (!ent) continue;

            // Now refine per SubMesh
            SubEntity* picked = pickSubEntity(ent, ray);
            if (picked)
                return picked;
        }
        return nullptr;
    }

    Ogre::SubEntity* pickSubEntity(Ogre::Entity* entity, const Ogre::Ray& ray)
     {
         Ogre::Matrix4 transform = entity->_getParentNodeFullTransform();
         Ogre::Real closest = std::numeric_limits<float>::max();
         Ogre::SubEntity* best = nullptr;

         Ogre::MeshPtr mesh = entity->getMesh();

         for (unsigned i = 0; i < entity->getNumSubEntities(); ++i)
         {
             Ogre::SubMesh* sm = mesh->getSubMesh(i);

             // Raycast this submesh
             Ogre::Real distance;
             if (raycastSubMesh(ray, sm, transform, distance))
             {
                 if (distance < closest)
                 {
                     closest = distance;
                     best = entity->getSubEntity(i);
                 }
             }
         }

         return best;
     }


    bool raycastSubMesh(const Ogre::Ray& ray,
                        Ogre::SubMesh* submesh,
                        const Ogre::Matrix4& worldTransform,
                        Ogre::Real& outDistance)
    {
        // Load vertex data (can be shared between submeshes)
        Ogre::VertexData* vData = submesh->useSharedVertices ?
                                  submesh->parent->sharedVertexData :
                                  submesh->vertexData;

        Ogre::HardwareVertexBufferSharedPtr vbuf =
            vData->vertexBufferBinding->getBuffer(0);

        unsigned char* vertex =
            static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        size_t vSize = vbuf->getVertexSize();

        // Index data
        Ogre::IndexData* iData = submesh->indexData;

        Ogre::HardwareIndexBufferSharedPtr ibuf = iData->indexBuffer;
        bool use32 = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned char* indices =
            static_cast<unsigned char*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

        // Access vertex positions
        const Ogre::VertexElement* posElem =
            vData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

        Ogre::Real closest = std::numeric_limits<Ogre::Real>::max();
        Ogre::Vector3* pos = new Ogre::Vector3[3];

        // Iterate triangles
        for (size_t i = 0; i < iData->indexCount; i += 3)
        {
            unsigned int i0, i1, i2;

            if (use32)
            {
                uint32_t* b = (uint32_t*)indices;
                i0 = b[i + 0];
                i1 = b[i + 1];
                i2 = b[i + 2];
            }
            else
            {
                uint16_t* b = (uint16_t*)indices;
                i0 = b[i + 0];
                i1 = b[i + 1];
                i2 = b[i + 2];
            }

            // Read positions
            unsigned char* v;

            v = vertex + i0 * vSize;
            posElem->baseVertexPointerToElement(v, pos[0].ptr());
            v = vertex + i1 * vSize;
            posElem->baseVertexPointerToElement(v, pos[1].ptr());
            v = vertex + i2 * vSize;
            posElem->baseVertexPointerToElement(v, pos[2].ptr());

            // Transform to world space
            pos[0] = worldTransform * pos[0];
            pos[1] = worldTransform * pos[1];
            pos[2] = worldTransform * pos[2];

            // Ray to triangle intersection
            std::pair<bool, Ogre::Real> hit =
                Ogre::Math::intersects(ray, pos[0], pos[1], pos[2], true, false);

            if (hit.first && hit.second < closest)
            {
                closest = hit.second;
            }
        }

        delete[] pos;

        vbuf->unlock();
        ibuf->unlock();

        if (closest < std::numeric_limits<Ogre::Real>::max())
        {
            outDistance = closest;
            return true;
        }
        return false;
    }

    void buttonHit( OgreBites::Button* button )
    {
        Ogre::String name = button->getName();
        if (name == "Translate")
        {
            mGizmo->setMode(G_TRANSLATE);
        }
        else if (name == "Rotate")
        {
            mGizmo->setMode(G_ROTATE);
        }
        else if (name == "Scale")
        {
            mGizmo->setMode(G_SCALE);
        }
    }

    // custom shader parameter bindings
    enum ShaderParam { SP_SHININESS = 1, SP_DIFFUSE, SP_SPECULAR };

    Gizmo* mGizmo;
    SceneNode* mLightPivot;
    Button* mTranslate;
    Button* mRotate;
    Button* mScale;
};
#endif // OGRE_GIZMOS_H