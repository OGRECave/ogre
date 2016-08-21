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

#include "OgreMeshManager2.h"

#include "OgreMesh2.h"
#include "OgreSubMesh2.h"
#include "OgreMatrix4.h"
#include "OgrePatchMesh.h"
#include "OgreException.h"

#include "OgrePrefabFactory.h"

namespace Ogre
{
    template<> MeshManager* Singleton<MeshManager>::msSingleton = 0;
    //-----------------------------------------------------------------------
    MeshManager* MeshManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    MeshManager& MeshManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    MeshManager::MeshManager() :
        mVaoManager( 0 ),
        mBoundsPaddingFactor( 0.01 )/*,
        mListener( 0 )*/
    {
        mLoadOrder = 300.0f;
        mResourceType = "Mesh2";

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
        return getResourceByName(name, groupName).staticCast<Mesh>();
    }
    //-----------------------------------------------------------------------
    void MeshManager::_initialise(void)
    {
    }
    //-----------------------------------------------------------------------
    void MeshManager::_setVaoManager( VaoManager *vaoManager )
    {
        mVaoManager = vaoManager;
    }
    //-----------------------------------------------------------------------
    MeshManager::ResourceCreateOrRetrieveResult MeshManager::createOrRetrieve(
        const String& name, const String& group,
        bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params,
        BufferType vertexBufferType,
        BufferType indexBufferType,
        bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        ResourceCreateOrRetrieveResult res = 
            ResourceManager::createOrRetrieve(name,group,isManual,loader,params);
        MeshPtr pMesh = res.first.staticCast<Mesh>();
        // Was it created?
        if (res.second)
        {
            pMesh->setVertexBufferPolicy(vertexBufferType, vertexBufferShadowed);
            pMesh->setIndexBufferPolicy(indexBufferType, indexBufferShadowed);
        }
        return res;

    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::prepare( const String& filename, const String& groupName, 
                                  BufferType vertexBufferType,
                                  BufferType indexBufferType,
                                  bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        MeshPtr pMesh = createOrRetrieve( filename, groupName, false, 0, 0,
                                          vertexBufferType, indexBufferType,
                                          vertexBufferShadowed, indexBufferShadowed ).
                        first.staticCast<Mesh>();
        pMesh->prepare();
        return pMesh;
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::load( const String& filename, const String& groupName, 
                               BufferType vertexBufferType,
                               BufferType indexBufferType,
                               bool vertexBufferShadowed, bool indexBufferShadowed)
    {
        MeshPtr pMesh = createOrRetrieve( filename, groupName, false, 0, 0,
                                          vertexBufferType, indexBufferType,
                                          vertexBufferShadowed, indexBufferShadowed ).
                        first.staticCast<Mesh>();
        pMesh->load();
        return pMesh;
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::create( const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return createResource(name,group,isManual,loader,createParams).staticCast<Mesh>();
    }
    //-----------------------------------------------------------------------
    MeshPtr MeshManager::createManual( const String& name, const String& groupName,
                                       ManualResourceLoader* loader )
    {
        // Don't try to get existing, create should fail if already exists
        if( !this->getResourceByName( name, groupName ).isNull() )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_DUPLICATE_ITEM,
                         "v2 Mesh with name '" + name + "' already exists.",
                         "MeshManager::createManual" );
        }
        return create(name, groupName, true, loader);
    }
    //-------------------------------------------------------------------------
    /*void MeshManager::setListener(MeshSerializerListener *listener)
    {
        mListener = listener;
    }
    //-------------------------------------------------------------------------
    MeshSerializerListener *MeshManager::getListener()
    {
        return mListener;
    }*/
    //-----------------------------------------------------------------------
    Real MeshManager::getBoundsPaddingFactor(void)
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
        return OGRE_NEW Mesh(this, name, handle, group, mVaoManager, isManual, loader);
    }
    //-----------------------------------------------------------------------
}
