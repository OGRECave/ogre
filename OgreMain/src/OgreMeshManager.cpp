/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgrePatchMesh.h"
#include "OgrePrefabFactory.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> MeshManager* Singleton<MeshManager>::msSingleton = 0;
    MeshManager* MeshManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    MeshManager& MeshManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    MeshManager::MeshManager():
    mBoundsPaddingFactor(0.01), mListener(0)
    {
        mBlendWeightsBaseElementType = VET_FLOAT1;
        mPrepAllMeshesForShadowVolumes = false;

        mLoadOrder = 350.0f;
        mResourceType = "Mesh";

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

    }
    //-----------------------------------------------------------------------
    MeshManager::~MeshManager()
    {
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::getByName(const String& name, const String& groupName)
    {
        return static_pointer_cast<Mesh>(getResourceByName(name, groupName));
    }
    //-----------------------------------------------------------------------
    void MeshManager::_initialise(void)
    {
        // Create prefab objects
        createManual("Ogre/Debug/AxesMesh", RGN_INTERNAL, &mPrefabLoader);
        createManual("Prefab_Sphere", RGN_INTERNAL, &mPrefabLoader);
        createManual("Prefab_Cube", RGN_INTERNAL, &mPrefabLoader);
        // Planes can never be manifold
        createManual("Prefab_Plane", RGN_INTERNAL, &mPrefabLoader)->setAutoBuildEdgeLists(false);
    }
    //-----------------------------------------------------------------------
    MeshManager::ResourceCreateOrRetrieveResult MeshManager::createOrRetrieve(
        const String& name, const String& group,
        bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params,
        HardwareBuffer::Usage vertexBufferUsage, 
        HardwareBuffer::Usage indexBufferUsage, 
        bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        ResourceCreateOrRetrieveResult res = 
            ResourceManager::createOrRetrieve(name,group,isManual,loader,params);
        MeshPtr pMesh = static_pointer_cast<Mesh>(res.first);
        // Was it created?
        if (res.second)
        {
            pMesh->setVertexBufferPolicy(vertexBufferUsage, vertexBufferShadowed);
            pMesh->setIndexBufferPolicy(indexBufferUsage, indexBufferShadowed);
        }
        return res;

    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::prepare( const String& filename, const String& groupName, 
        HardwareBuffer::Usage vertexBufferUsage, 
        HardwareBuffer::Usage indexBufferUsage, 
        bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        MeshPtr pMesh = static_pointer_cast<Mesh>(createOrRetrieve(filename,groupName,false,0,0,
                                         vertexBufferUsage,indexBufferUsage,
                                         vertexBufferShadowed,indexBufferShadowed).first);
        pMesh->prepare();
        return pMesh;
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::load( const String& filename, const String& groupName, 
        HardwareBuffer::Usage vertexBufferUsage, 
        HardwareBuffer::Usage indexBufferUsage, 
        bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        MeshPtr pMesh = static_pointer_cast<Mesh>(createOrRetrieve(filename,groupName,false,0,0,
                                         vertexBufferUsage,indexBufferUsage,
                                         vertexBufferShadowed,indexBufferShadowed).first);
        pMesh->load();
        return pMesh;
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::create (const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return static_pointer_cast<Mesh>(createResource(name,group,isManual,loader,createParams));
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::createManual( const String& name, const String& groupName, 
        ManualResourceLoader* loader)
    {
        // Don't try to get existing, create should fail if already exists
        return create(name, groupName, true, loader);
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::createPlane( const String& name, const String& groupName,
        const Plane& plane, Real width, Real height, int xsegments, int ysegments,
        bool normals, unsigned short numTexCoordSets, Real xTile, Real yTile, const Vector3& upVector,
        HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage,
        bool vertexShadowBuffer, bool indexShadowBuffer)
    {
        // Create manual mesh which calls back self to load
        MeshPtr pMesh = createManual(name, groupName, &mPrefabLoader);
        // Planes can never be manifold
        pMesh->setAutoBuildEdgeLists(false);
        // store parameters
        MeshBuildParams params;
        params.type = MBT_PLANE;
        params.plane = plane;
        params.width = width;
        params.height = height;
        params.xsegments = xsegments;
        params.ysegments = ysegments;
        params.normals = normals;
        params.numTexCoordSets = numTexCoordSets;
        params.xTile = xTile;
        params.yTile = yTile;
        params.upVector = upVector;
        params.vertexBufferUsage = vertexBufferUsage;
        params.indexBufferUsage = indexBufferUsage;
        params.vertexShadowBuffer = vertexShadowBuffer;
        params.indexShadowBuffer = indexShadowBuffer;
        mPrefabLoader.mMeshBuildParams[pMesh.get()] = params;

        // to preserve previous behaviour, load immediately
        pMesh->load();

        return pMesh;
    }
    
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::createCurvedPlane( const String& name, const String& groupName, 
        const Plane& plane, Real width, Real height, Real bow, int xsegments, int ysegments,
        bool normals, unsigned short numTexCoordSets, Real xTile, Real yTile, const Vector3& upVector,
            HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage,
            bool vertexShadowBuffer, bool indexShadowBuffer)
    {
        // Create manual mesh which calls back self to load
        MeshPtr pMesh = createManual(name, groupName, &mPrefabLoader);
        // Planes can never be manifold
        pMesh->setAutoBuildEdgeLists(false);
        // store parameters
        MeshBuildParams params;
        params.type = MBT_CURVED_PLANE;
        params.plane = plane;
        params.width = width;
        params.height = height;
        params.curvature = bow;
        params.xsegments = xsegments;
        params.ysegments = ysegments;
        params.normals = normals;
        params.numTexCoordSets = numTexCoordSets;
        params.xTile = xTile;
        params.yTile = yTile;
        params.upVector = upVector;
        params.vertexBufferUsage = vertexBufferUsage;
        params.indexBufferUsage = indexBufferUsage;
        params.vertexShadowBuffer = vertexShadowBuffer;
        params.indexShadowBuffer = indexShadowBuffer;
        mPrefabLoader.mMeshBuildParams[pMesh.get()] = params;

        // to preserve previous behaviour, load immediately
        pMesh->load();

        return pMesh;

    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::createCurvedIllusionPlane(
        const String& name, const String& groupName, const Plane& plane,
        Real width, Real height, Real curvature,
        int xsegments, int ysegments,
        bool normals, unsigned short numTexCoordSets,
        Real uTile, Real vTile, const Vector3& upVector,
        const Quaternion& orientation, 
        HardwareBuffer::Usage vertexBufferUsage, 
        HardwareBuffer::Usage indexBufferUsage,
        bool vertexShadowBuffer, bool indexShadowBuffer,
        int ySegmentsToKeep)
    {
        // Create manual mesh which calls back self to load
        MeshPtr pMesh = createManual(name, groupName, &mPrefabLoader);
        // Planes can never be manifold
        pMesh->setAutoBuildEdgeLists(false);
        // store parameters
        MeshBuildParams params;
        params.type = MBT_CURVED_ILLUSION_PLANE;
        params.plane = plane;
        params.width = width;
        params.height = height;
        params.curvature = curvature;
        params.xsegments = xsegments;
        params.ysegments = ysegments;
        params.normals = normals;
        params.numTexCoordSets = numTexCoordSets;
        params.xTile = uTile;
        params.yTile = vTile;
        params.upVector = upVector;
        params.orientation = orientation;
        params.vertexBufferUsage = vertexBufferUsage;
        params.indexBufferUsage = indexBufferUsage;
        params.vertexShadowBuffer = vertexShadowBuffer;
        params.indexShadowBuffer = indexShadowBuffer;
        params.ySegmentsToKeep = ySegmentsToKeep;
        mPrefabLoader.mMeshBuildParams[pMesh.get()] = params;

        // to preserve previous behaviour, load immediately
        pMesh->load();

        return pMesh;
    }

    //-----------------------------------------------------------------------
    void MeshManager::PrefabLoader::tesselate2DMesh(SubMesh* sm, unsigned short meshWidth, unsigned short meshHeight,
        bool doubleSided, HardwareBuffer::Usage indexBufferUsage, bool indexShadowBuffer)
    {
        // The mesh is built, just make a list of indexes to spit out the triangles
        unsigned short vInc, v, iterations;

        if (doubleSided)
        {
            iterations = 2;
            vInc = 1;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            vInc = 1;
            v = 0;
        }

        // Allocate memory for faces
        // Num faces, width*height*2 (2 tris per square), index count is * 3 on top
        sm->indexData->indexCount = (meshWidth-1) * (meshHeight-1) * 2 * iterations * 3;
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
            sm->indexData->indexCount, indexBufferUsage, indexShadowBuffer);

        unsigned short v1, v2, v3;
        //bool firstTri = true;
        HardwareIndexBufferSharedPtr ibuf = sm->indexData->indexBuffer;
        // Lock the whole buffer
        HardwareBufferLockGuard ibufLock(ibuf, HardwareBuffer::HBL_DISCARD);
        unsigned short* pIndexes = static_cast<unsigned short*>(ibufLock.pData);

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            unsigned short u = 0;
            unsigned short uInc = 1; // Start with moving +u
            unsigned short vCount = meshHeight - 1;

            while (vCount--)
            {
                unsigned short uCount = meshWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * meshWidth) + u;
                    v2 = (v * meshWidth) + u;
                    v3 = ((v + vInc) * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * meshWidth) + (u + uInc);
                    v2 = (v * meshWidth) + u;
                    v3 = (v * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;

                    // Next column
                    u += uInc;
                }
                // Next row
                v += vInc;
                u = 0;


            }

            // Reverse vInc for double sided
            v = meshHeight - 1;
            vInc = -vInc;

        }
    }
    //-------------------------------------------------------------------------
    void MeshManager::setListener(Ogre::MeshSerializerListener *listener)
    {
        mListener = listener;
    }
    //-------------------------------------------------------------------------
    MeshSerializerListener *MeshManager::getListener()
    {
        return mListener;
    }
    //-----------------------------------------------------------------------
    void MeshManager::PrefabLoader::loadResource(Resource* res)
    {
        Mesh* msh = static_cast<Mesh*>(res);

        // attempt to create a prefab mesh
        bool createdPrefab = PrefabFactory::createPrefab(msh);

        // the mesh was not a prefab..
        if(!createdPrefab)
        {
            // Find build parameters
            MeshBuildParamsMap::iterator ibld = mMeshBuildParams.find(res);
            if (ibld == mMeshBuildParams.end())
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Cannot find build parameters for " + res->getName(),
                    "MeshManager::loadResource");
            }
            MeshBuildParams& params = ibld->second;

            switch(params.type)
            {
            case MBT_PLANE:
                loadManualPlane(msh, params);
                break;
            case MBT_CURVED_ILLUSION_PLANE:
                loadManualCurvedIllusionPlane(msh, params);
                break;
            case MBT_CURVED_PLANE:
                loadManualCurvedPlane(msh, params);
                break;
            default:
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Unknown build parameters for " + res->getName(),
                    "MeshManager::loadResource");
            }
        }
    }

    //-----------------------------------------------------------------------
    void MeshManager::PrefabLoader::loadManualPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if ((params.xsegments + 1) * (params.ysegments + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();

        // Set up vertex data
        // Use a single shared buffer
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        VertexData* vertexData = pMesh->sharedVertexData;
        // Set up Vertex Declaration
        VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        size_t currOffset = 0;
        // We always need positions
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION).getSize();
        // Optional normals
        if(params.normals)
        {
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            // Assumes 2D texture coords
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }

        vertexData->vertexCount = (params.xsegments + 1) * (params.ysegments + 1);

        // Allocate vertex buffer
        HardwareVertexBufferSharedPtr vbuf = 
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                vertexDecl->getVertexSize(0), vertexData->vertexCount,
                params.vertexBufferUsage, params.vertexShadowBuffer);

        // Set up the binding (one source only)
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::loadManualPlane");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Lock the whole buffer
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pReal = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Real xTex = (1.0f * params.xTile) / params.xsegments;
        Real yTex = (1.0f * params.yTile) / params.ysegments;
        Vector3 vec;
        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE;
        Real maxSquaredLength = 0;
        bool firstTime = true;

        for (int y = 0; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pReal++ = vec.x;
                *pReal++ = vec.y;
                *pReal++ = vec.z;

                // Build bounds as we go
                if (firstTime)
                {
                    min = vec;
                    max = vec;
                    maxSquaredLength = vec.squaredLength();
                    firstTime = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSquaredLength = std::max(maxSquaredLength, vec.squaredLength());
                }

                if (params.normals)
                {
                    // Default normal is along unit Z
                    vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;

                    *pReal++ = vec.x;
                    *pReal++ = vec.y;
                    *pReal++ = vec.z;
                }

                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pReal++ = x * xTex;
                    *pReal++ = 1 - (y * yTex);
                }


            } // x
        } // y

        // Unlock
        vbufLock.unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, params.xsegments + 1, params.ysegments + 1, false, 
            params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(AxisAlignedBox(min, max), true);
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredLength));
    }
    //-----------------------------------------------------------------------
    void MeshManager::PrefabLoader::loadManualCurvedPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if ((params.xsegments + 1) * (params.ysegments + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();

        // Set options
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        pMesh->sharedVertexData->vertexStart = 0;
        VertexBufferBinding* bind = pMesh->sharedVertexData->vertexBufferBinding;
        VertexDeclaration* decl = pMesh->sharedVertexData->vertexDeclaration;

        pMesh->sharedVertexData->vertexCount = (params.xsegments + 1) * (params.ysegments + 1);

        size_t offset = 0;
        offset += decl->addElement(0, offset, VET_FLOAT3, VES_POSITION).getSize();
        if (params.normals)
        {
            offset += decl->addElement(0, 0, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            offset += decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }


        // Allocate memory
        HardwareVertexBufferSharedPtr vbuf = 
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                offset, pMesh->sharedVertexData->vertexCount, 
                params.vertexBufferUsage, params.vertexShadowBuffer);
        bind->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::createPlane");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Real xTex = (1.0f * params.xTile) / params.xsegments;
        Real yTex = (1.0f * params.yTile) / params.ysegments;
        Vector3 vec;

        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE;
        Real maxSqLen = 0;
        bool first = true;

        Real diff_x, diff_y, dist;

        for (int y = 0; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;

                // Here's where curved plane is different from standard plane.  Amazing, I know.
                diff_x = (x - ((params.xsegments) / 2)) / static_cast<Real>((params.xsegments));
                diff_y = (y - ((params.ysegments) / 2)) / static_cast<Real>((params.ysegments));
                dist = std::sqrt(diff_x*diff_x + diff_y * diff_y );
                vec.z = (-std::sin((1-dist) * (Math::PI/2)) * params.curvature) + params.curvature;

                // Transform by orientation and distance
                Vector3 pos = xform * vec;
                // Assign to geometry
                *pFloat++ = pos.x;
                *pFloat++ = pos.y;
                *pFloat++ = pos.z;

                // Record bounds
                if (first)
                {
                    min = max = vec;
                    maxSqLen = vec.squaredLength();
                    first = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSqLen = std::max(maxSqLen, vec.squaredLength());
                }

                if (params.normals)
                {
                    // This part is kinda 'wrong' for curved planes... but curved planes are
                    //   very valuable outside sky planes, which don't typically need normals
                    //   so I'm not going to mess with it for now. 

                    // Default normal is along unit Z
                    //vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;
                    vec.normalise();

                    *pFloat++ = vec.x;
                    *pFloat++ = vec.y;
                    *pFloat++ = vec.z;
                }

                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pFloat++ = x * xTex;
                    *pFloat++ = 1 - (y * yTex);
                }

            } // x
        } // y
        vbufLock.unlock();

        // Generate face list
        tesselate2DMesh(pSub, params.xsegments + 1, params.ysegments + 1, 
            false, params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(AxisAlignedBox(min, max), true);
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSqLen));

    }
    //-----------------------------------------------------------------------
    void MeshManager::PrefabLoader::loadManualCurvedIllusionPlane(Mesh* pMesh, MeshBuildParams& params)
    {
        if (params.ySegmentsToKeep == -1) params.ySegmentsToKeep = params.ysegments;

        if ((params.xsegments + 1) * (params.ySegmentsToKeep + 1) > 65536)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Plane tessellation is too high, must generate max 65536 vertices");
        SubMesh *pSub = pMesh->createSubMesh();


        // Set up vertex data
        // Use a single shared buffer
        pMesh->sharedVertexData = OGRE_NEW VertexData();
        VertexData* vertexData = pMesh->sharedVertexData;
        // Set up Vertex Declaration
        VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        size_t currOffset = 0;
        // We always need positions
        currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION).getSize();
        // Optional normals
        if(params.normals)
        {
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL).getSize();
        }

        for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
        {
            // Assumes 2D texture coords
            currOffset += vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i).getSize();
        }

        vertexData->vertexCount = (params.xsegments + 1) * (params.ySegmentsToKeep + 1);

        // Allocate vertex buffer
        HardwareVertexBufferSharedPtr vbuf = 
            pMesh->getHardwareBufferManager()->createVertexBuffer(
                vertexDecl->getVertexSize(0), vertexData->vertexCount,
                params.vertexBufferUsage, params.vertexShadowBuffer);

        // Set up the binding (one source only)
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vbuf);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Affine3 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Affine3::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = params.plane.normal;
        zAxis.normalise();
        yAxis = params.upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.squaredLength() == 0)
        {
            //upVector must be wrong
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::createPlane");
        }
        xAxis.normalise();

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard transform from origin
        xlate.setTrans(params.plane.normal * -params.plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Imagine a large sphere with the camera located near the top
        // The lower the curvature, the larger the sphere
        // Use the angle from viewer to the points on the plane
        // Credit to Aftershock for the general approach
        Real camPos;      // Camera position relative to sphere center

        // Derive sphere radius
        Vector3 vertPos;  // position relative to camera
        Real sphDist;      // Distance from camera to sphere along box vertex vector
        // Vector3 camToSph; // camera position to sphere
        Real sphereRadius;// Sphere radius
        // Actual values irrelevant, it's the relation between sphere radius and camera position that's important
        const Real SPHERE_RAD = 100.0;
        const Real CAM_DIST = 5.0;

        sphereRadius = SPHERE_RAD - params.curvature;
        camPos = sphereRadius - CAM_DIST;

        // Lock the whole buffer
        HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_DISCARD);
        float* pFloat = static_cast<float*>(vbufLock.pData);
        Real xSpace = params.width / params.xsegments;
        Real ySpace = params.height / params.ysegments;
        Real halfWidth = params.width / 2;
        Real halfHeight = params.height / 2;
        Vector3 vec, norm;
        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE;
        Real maxSquaredLength = 0;
        bool firstTime = true;

        for (int y = params.ysegments - params.ySegmentsToKeep; y < params.ysegments + 1; ++y)
        {
            for (int x = 0; x < params.xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pFloat++ = vec.x;
                *pFloat++ = vec.y;
                *pFloat++ = vec.z;

                // Build bounds as we go
                if (firstTime)
                {
                    min = vec;
                    max = vec;
                    maxSquaredLength = vec.squaredLength();
                    firstTime = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSquaredLength = std::max(maxSquaredLength, vec.squaredLength());
                }

                if (params.normals)
                {
                    // Default normal is along unit Z
                    norm = Vector3::UNIT_Z;
                    // Rotate
                    norm = params.orientation * norm;

                    *pFloat++ = norm.x;
                    *pFloat++ = norm.y;
                    *pFloat++ = norm.z;
                }

                // Generate texture coords
                // Normalise position
                // modify by orientation to return +y up
                vec = params.orientation.Inverse() * vec;
                vec.normalise();
                // Find distance to sphere
                sphDist = Math::Sqrt(camPos*camPos * (vec.y*vec.y-1.0f) + sphereRadius*sphereRadius) - camPos*vec.y;

                vec.x *= sphDist;
                vec.z *= sphDist;

                // Use x and y on sphere as texture coordinates, tiled
                Real s = vec.x * (0.01f * params.xTile);
                Real t = 1.0f - (vec.z * (0.01f * params.yTile));
                for (unsigned short i = 0; i < params.numTexCoordSets; ++i)
                {
                    *pFloat++ = s;
                    *pFloat++ = t;
                }


            } // x
        } // y

        // Unlock
        vbufLock.unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, params.xsegments + 1, params.ySegmentsToKeep + 1, false, 
            params.indexBufferUsage, params.indexShadowBuffer);

        pMesh->_setBounds(AxisAlignedBox(min, max), true);
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredLength));
    }
    //-----------------------------------------------------------------------
    PatchMeshPtr MeshManager::createBezierPatch(const String& name, const String& groupName,
            void* controlPointBuffer, VertexDeclaration *declaration, 
            size_t width, size_t height,
            size_t uMaxSubdivisionLevel, size_t vMaxSubdivisionLevel,
            PatchSurface::VisibleSide visibleSide, 
            HardwareBuffer::Usage vbUsage, HardwareBuffer::Usage ibUsage,
            bool vbUseShadow, bool ibUseShadow)
    {
        if (width < 3 || height < 3)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Bezier patch require at least 3x3 control points",
                "MeshManager::createBezierPatch");
        }

        MeshPtr pMesh = getByName(name, groupName);
        if (pMesh)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "A mesh called " + name + 
                " already exists!", "MeshManager::createBezierPatch");
        }
        PatchMesh* pm = OGRE_NEW PatchMesh(this, name, getNextHandle(), groupName);
        pm->define(controlPointBuffer, declaration, width, height,
            uMaxSubdivisionLevel, vMaxSubdivisionLevel, visibleSide, vbUsage, ibUsage,
            vbUseShadow, ibUseShadow);
        pm->load();
        ResourcePtr res(pm);
        addImpl(res);

        return static_pointer_cast<PatchMesh>(res);
    }
    //-----------------------------------------------------------------------
    void MeshManager::setPrepareAllMeshesForShadowVolumes(bool enable)
    {
        mPrepAllMeshesForShadowVolumes = enable;
    }
    //-----------------------------------------------------------------------
    bool MeshManager::getPrepareAllMeshesForShadowVolumes(void)
    {
        return mPrepAllMeshesForShadowVolumes;
    }
    //-----------------------------------------------------------------------
    VertexElementType MeshManager::getBlendWeightsBaseElementType() const
    {
        return mBlendWeightsBaseElementType;
    }
    //-----------------------------------------------------------------------
    void MeshManager::setBlendWeightsBaseElementType( VertexElementType vet )
    {
        switch ( vet )
        {
            case VET_UBYTE4_NORM:
            case VET_USHORT2_NORM:
            case VET_FLOAT1:
                mBlendWeightsBaseElementType = vet;
                break;
            default:
                OgreAssert(false, "Unsupported BlendWeightsBaseElementType");
                break;
        }
    }
    //-----------------------------------------------------------------------
    Real MeshManager::getBoundsPaddingFactor( void )
    {
        return mBoundsPaddingFactor;
    }
    //-----------------------------------------------------------------------
    void MeshManager::setBoundsPaddingFactor(Real paddingFactor)
    {
        mBoundsPaddingFactor = paddingFactor;
    }
    //-----------------------------------------------------------------------
    Resource* MeshManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        // no use for createParams here
        return OGRE_NEW Mesh(this, name, handle, group, isManual, loader);
    }
    //-----------------------------------------------------------------------

}
