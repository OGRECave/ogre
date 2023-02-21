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
    struct MeshCodec : public Codec
    {
        String magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const override { return ""; }
        String getType() const override { return "mesh"; }
        void decode(const DataStreamPtr& input, const Any& output) const override
        {
            Mesh* dst = any_cast<Mesh*>(output);
            MeshSerializer serializer;
            serializer.setListener(MeshManager::getSingleton().getListener());
            serializer.importMesh(input, dst);
        }
    };

    //-----------------------------------------------------------------------
    template<> MeshManager* Singleton<MeshManager>::msSingleton = 0;

    bool MeshManager::mBonesUseObjectSpace = true;
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

        mMeshCodec.reset(new MeshCodec());
        Codec::registerCodec(mMeshCodec.get());

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

        mPrefabLoader.reset(new PrefabFactory());
    }
    //-----------------------------------------------------------------------
    MeshManager::~MeshManager()
    {
        Codec::unregisterCodec(mMeshCodec.get());
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::getByName(const String& name, const String& groupName) const
    {
        return static_pointer_cast<Mesh>(getResourceByName(name, groupName));
    }
    //-----------------------------------------------------------------------
    void MeshManager::_initialise(void)
    {
        // Create prefab objects
        createManual("Prefab_Sphere", RGN_INTERNAL, mPrefabLoader.get());
        createManual("Prefab_Cube", RGN_INTERNAL, mPrefabLoader.get());
        // Planes can never be manifold
        createManual("Prefab_Plane", RGN_INTERNAL, mPrefabLoader.get())->setAutoBuildEdgeLists(false);
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
        MeshPtr pMesh = createManual(name, groupName, mPrefabLoader.get());
        // Planes can never be manifold
        pMesh->setAutoBuildEdgeLists(false);
        // store parameters
        MeshBuildParams params = {};
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

        pMesh->getUserObjectBindings().setUserAny("_MeshBuildParams", Any(params));

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
        MeshPtr pMesh = createManual(name, groupName, mPrefabLoader.get());
        // Planes can never be manifold
        pMesh->setAutoBuildEdgeLists(false);
        // store parameters
        MeshBuildParams params = {};
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

        pMesh->getUserObjectBindings().setUserAny("_MeshBuildParams", Any(params));

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
        MeshPtr pMesh = createManual(name, groupName, mPrefabLoader.get());
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

        pMesh->getUserObjectBindings().setUserAny("_MeshBuildParams", Any(params));

        // to preserve previous behaviour, load immediately
        pMesh->load();

        return pMesh;
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
