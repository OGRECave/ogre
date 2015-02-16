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
#include "MeshSerializerTests.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreVertexIndexData.h"
#include "OgreEdgeListBuilder.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreSubMesh.h"
#include "OgreMeshSerializer.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreFileSystem.h"
#include "OgreConfigFile.h"
#include "OgreOldSkeletonManager.h"
#include "OgreSkeletonSerializer.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreMaterialManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreSkeleton.h"

#include "UnitTestSuite.h"

//#define I_HAVE_LOT_OF_FREE_TIME

// To run XML test, you need to symlink all files (except main.cpp) from XMLConverter tool to the Test_Ogre component!
// You also need to set TIXML_USE_STL macro globally.
// #define OGRE_TEST_XMLSERIALIZER

#ifdef OGRE_TEST_XMLSERIALIZER
// #define TIXML_USE_STL
#include "OgreXMLMeshSerializer.h"
#endif

#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
#include "OgreMeshLodGenerator.h"
#include "OgreLodConfig.h"
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include "macUtils.h"
#endif

// Register the test suite
CPPUNIT_TEST_SUITE_REGISTRATION(MeshSerializerTests);

//--------------------------------------------------------------------------
void MeshSerializerTests::setUp()
{
    mErrorFactor = 0.05;

    mFSLayer = OGRE_NEW_T(Ogre::FileSystemLayer, Ogre::MEMCATEGORY_GENERAL)(OGRE_VERSION_NAME);

    OGRE_NEW ResourceGroupManager();
    OGRE_NEW LodStrategyManager();
    OGRE_NEW DefaultHardwareBufferManager();
    OGRE_NEW MeshManager();
    OGRE_NEW OldSkeletonManager();
    ArchiveManager* archiveMgr = OGRE_NEW ArchiveManager();
    archiveMgr->addArchiveFactory(OGRE_NEW FileSystemArchiveFactory());

    MaterialManager* matMgr = OGRE_NEW MaterialManager();
    matMgr->initialise();

    // Load resource paths from config file
    ConfigFile cf;
    String resourcesPath;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    resourcesPath = mFSLayer->getConfigFilePath("resources.cfg");
#else
    resourcesPath = mFSLayer->getConfigFilePath("bin/resources.cfg");
#endif

    // Go through all sections & settings in the file
    cf.load(resourcesPath);
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap* settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            if (typeName == "FileSystem") {
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
        }
    }

    mMesh = MeshManager::getSingleton().load("knot.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    getResourceFullPath(mMesh, mMeshFullPath);
    if (!copyFile(mMeshFullPath + ".bak", mMeshFullPath)) {
        // If there is no backup, create one.
        copyFile(mMeshFullPath, mMeshFullPath + ".bak");
    }
    mSkeletonFullPath = "";
    mSkeleton = OldSkeletonManager::getSingleton().load("jaiqua.skeleton", "Popular").staticCast<Skeleton>();
    getResourceFullPath(mSkeleton, mSkeletonFullPath);
    if (!copyFile(mSkeletonFullPath + ".bak", mSkeletonFullPath)) {
        // If there is no backup, create one.
        copyFile(mSkeletonFullPath, mSkeletonFullPath + ".bak");
    }

    mMesh->reload();

#ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR
    {
        MeshLodGenerator().generateAutoconfiguredLodLevels(mMesh);
    }
#endif /* ifdef OGRE_BUILD_COMPONENT_MESHLODGENERATOR */

    mOrigMesh = mMesh->clone(mMesh->getName() + ".orig.mesh", mMesh->getGroup());
}
//--------------------------------------------------------------------------
void MeshSerializerTests::tearDown()
{
    // Copy back original file.
    if (!mMeshFullPath.empty()) {
        copyFile(mMeshFullPath + ".bak", mMeshFullPath);
    }
    if (!mSkeletonFullPath.empty()) {
        copyFile(mSkeletonFullPath + ".bak", mSkeletonFullPath);
    }
    if (!mMesh.isNull()) {
        mMesh->unload();
        mMesh.setNull();
    }
    if (!mOrigMesh.isNull()) {
        mOrigMesh->unload();
        mOrigMesh.setNull();
    }
    if (!mSkeleton.isNull()) {
        mSkeleton->unload();
        mSkeleton.setNull();
    }    
    
    OGRE_DELETE MeshManager::getSingletonPtr();
    OGRE_DELETE OldSkeletonManager::getSingletonPtr();
    OGRE_DELETE DefaultHardwareBufferManager::getSingletonPtr();
    OGRE_DELETE ArchiveManager::getSingletonPtr();    
    OGRE_DELETE MaterialManager::getSingletonPtr();
    OGRE_DELETE LodStrategyManager::getSingletonPtr();
    OGRE_DELETE ResourceGroupManager::getSingletonPtr();
    OGRE_DELETE_T(mFSLayer, FileSystemLayer, Ogre::MEMCATEGORY_GENERAL);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_clone()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    MeshPtr cloneMesh = mMesh->clone(mMesh->getName() + ".clone.mesh", mMesh->getGroup());
    assertMeshClone(mMesh.get(), cloneMesh.get());
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh(MeshVersion version)
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    MeshSerializer serializer;
    serializer.exportMesh(mOrigMesh.get(), mMeshFullPath, version);
    mMesh->reload();
    assertMeshClone(mOrigMesh.get(), mMesh.get(), version);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testSkeleton_Version_1_8()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    if (!mSkeleton.isNull()) {
        SkeletonSerializer skeletonSerializer;
        skeletonSerializer.exportSkeleton(mSkeleton.get(), mSkeletonFullPath, SKELETON_VERSION_1_8);
        mSkeleton->reload();
    }
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testSkeleton_Version_1_0()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    if (!mSkeleton.isNull()) {
        SkeletonSerializer skeletonSerializer;
        skeletonSerializer.exportSkeleton(mSkeleton.get(), mSkeletonFullPath, SKELETON_VERSION_1_0);
        mSkeleton->reload();
    }
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_10()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    testMesh(MESH_VERSION_LATEST);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_8()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    testMesh(MESH_VERSION_1_8);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_41()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    testMesh(MESH_VERSION_1_7);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_4()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    testMesh(MESH_VERSION_1_4);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_3()
{
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    testMesh(MESH_VERSION_1_0);
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_Version_1_2()
{
#ifdef I_HAVE_LOT_OF_FREE_TIME
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    // My sandboxing test. Takes a long time to complete!
    // Runs on all meshes and exports all to every LoD version.
    char* groups [] = { "Popular", "General", "Tests" };
    for (int i = 0; i < 3; i++) {
        StringVectorPtr meshes = ResourceGroupManager::getSingleton().findResourceNames(groups[i], "*.mesh");
        StringVector::iterator it, itEnd;
        it = meshes->begin();
        itEnd = meshes->end();
        for (; it != itEnd; it++) {
            try {
                mMesh = MeshManager::getSingleton().load(*it, groups[i]);
            }
            catch(std::exception e)
            {
                // OutputDebugStringA(e.what());
            }
            getResourceFullPath(mMesh, mMeshFullPath);
            if (!copyFile(mMeshFullPath + ".bak", mMeshFullPath)) {
                // If there is no backup, create one.
                copyFile(mMeshFullPath, mMeshFullPath + ".bak");
            }
            mOrigMesh = mMesh->clone(mMesh->getName() + ".orig.mesh", mMesh->getGroup());
            testMesh_XML();
            testMesh(MESH_VERSION_1_10);
            testMesh(MESH_VERSION_1_8);
            testMesh(MESH_VERSION_1_7);
            testMesh(MESH_VERSION_1_4);
            testMesh(MESH_VERSION_1_0);
        }
        meshes = ResourceGroupManager::getSingleton().findResourceNames(groups[i], "*.skeleton");
        it = meshes->begin();
        itEnd = meshes->end();
        for (; it != itEnd; it++) {
            mSkeleton = SkeletonManager::getSingleton().load(*it, groups[i]);
            getResourceFullPath(mSkeleton, mSkeletonFullPath);
            if (!copyFile(mSkeletonFullPath + ".bak", mSkeletonFullPath)) {
                // If there is no backup, create one.
                copyFile(mSkeletonFullPath, mSkeletonFullPath + ".bak");
            }
            SkeletonSerializer skeletonSerializer;
            skeletonSerializer.exportSkeleton(mSkeleton.get(), mSkeletonFullPath, SKELETON_VERSION_1_8);
            mSkeleton->reload();
            skeletonSerializer.exportSkeleton(mSkeleton.get(), mSkeletonFullPath, SKELETON_VERSION_1_0);
            mSkeleton->reload();
        }
    }
#endif /* ifdef I_HAVE_LOT_OF_FREE_TIME */
}
//--------------------------------------------------------------------------
void MeshSerializerTests::testMesh_XML()
{
#ifdef OGRE_TEST_XMLSERIALIZER
    UnitTestSuite::getSingletonPtr()->startTestMethod(__FUNCTION__);

    XMLMeshSerializer serializerXML;
    serializerXML.exportMesh(mOrigMesh.get(), mMeshFullPath + ".xml");
    mMesh = MeshManager::getSingleton().create(mMesh->getName() + ".test.mesh", mMesh->getGroup());
    serializerXML.importMesh(mMeshFullPath + ".xml", VET_COLOUR_ABGR, mMesh.get());
    assertMeshClone(mOrigMesh.get(), mMesh.get());
#endif
}
//--------------------------------------------------------------------------
void MeshSerializerTests::assertMeshClone(Mesh* a, Mesh* b, MeshVersion version /*= MESH_VERSION_LATEST*/)
{
    // TODO: Compare skeleton
    // TODO: Compare animations
    // TODO: Compare pose animations

    // CPPUNIT_ASSERT(a->getGroup() == b->getGroup());
    // CPPUNIT_ASSERT(a->getName() == b->getName());

#ifndef OGRE_TEST_XMLSERIALIZER
    // XML serializer fails on these!
    CPPUNIT_ASSERT(isEqual(a->getBoundingSphereRadius(), b->getBoundingSphereRadius()));
    CPPUNIT_ASSERT(isEqual(a->getBounds().getMinimum(), b->getBounds().getMinimum()));
    CPPUNIT_ASSERT(isEqual(a->getBounds().getMaximum(), b->getBounds().getMaximum()));
#else
    StringStream str;
    Real val1 = a->getBoundingSphereRadius();
    Real val2 = b->getBoundingSphereRadius();
    Real diff = (val1 > val2) ? (val1 / val2) : (val2 / val1);
    if (diff > 1.1) {
        str << "bound sphere diff: " << diff << std::endl;
    }
    val1 = a->getBounds().getMinimum().length();
    val2 = b->getBounds().getMinimum().length();
    diff = (val1 > val2) ? (val1 / val2) : (val2 / val1);
    if (diff > 1.1) {
        str << "bound min diff: " << diff << std::endl;
    }
    val1 = a->getBounds().getMaximum().length();
    val2 = b->getBounds().getMaximum().length();
    diff = (val1 > val2) ? (val1 / val2) : (val2 / val1);
    if (diff > 1.1) {
        str << "bound max diff: " << diff << std::endl;
    }
    if (!str.str().empty()) {
        StringStream str2;
        str2 << std::endl << "Mesh name: " << b->getName() << std::endl;
        str2 << str.str();
        std::cout << str2.str();
        // OutputDebugStringA(str2.str().c_str());
    }
#endif /* ifndef OGRE_TEST_XMLSERIALIZER */

    // AutobuildEdgeLists is not saved to mesh file. You need to set it after loading a mesh!
    // CPPUNIT_ASSERT(a->getAutoBuildEdgeLists() == b->getAutoBuildEdgeLists());
    CPPUNIT_ASSERT(isHashMapClone(a->getSubMeshNameMap(), b->getSubMeshNameMap()));

    assertVertexDataClone(a->sharedVertexData, b->sharedVertexData);
    CPPUNIT_ASSERT(a->getCreator() == b->getCreator());
    CPPUNIT_ASSERT(a->getIndexBufferUsage() == b->getIndexBufferUsage());
    CPPUNIT_ASSERT(a->getSharedVertexDataAnimationIncludesNormals() == b->getSharedVertexDataAnimationIncludesNormals());
    CPPUNIT_ASSERT(a->getSharedVertexDataAnimationType() == b->getSharedVertexDataAnimationType());
    CPPUNIT_ASSERT(a->getVertexBufferUsage() == b->getVertexBufferUsage());
    CPPUNIT_ASSERT(a->hasVertexAnimation() == b->hasVertexAnimation());

#ifndef OGRE_TEST_XMLSERIALIZER
    CPPUNIT_ASSERT(a->isEdgeListBuilt() == b->isEdgeListBuilt()); // <== OgreXMLSerializer is doing post processing to generate edgelists!
#endif // !OGRE_TEST_XMLSERIALIZER

    if ((a->getNumLodLevels() > 1 || b->getNumLodLevels() > 1) &&
        ((version < MESH_VERSION_1_8 || (!isLodMixed(a) && !isLodMixed(b))) && // mixed lod only supported in v1.10+
         (version < MESH_VERSION_1_4 || (a->getLodStrategyName() != DistanceLodStrategy::getSingletonPtr()->getName() &&
                                         b->getLodStrategyName() != DistanceLodStrategy::getSingletonPtr()->getName())))) { // Lod Strategy only supported in v1.41+
        CPPUNIT_ASSERT(a->getNumLodLevels() == b->getNumLodLevels());
        CPPUNIT_ASSERT(a->hasManualLodLevel() == b->hasManualLodLevel());
        CPPUNIT_ASSERT(a->getLodStrategyName() == b->getLodStrategyName());

        int numLods = a->getNumLodLevels();
        for (int i = 0; i < numLods; i++) {
            if (version != MESH_VERSION_1_0 && a->getAutoBuildEdgeLists() == b->getAutoBuildEdgeLists()) {
                assertEdgeDataClone(a->getEdgeList(i), b->getEdgeList(i));
            } else if (a->getLodLevel(i).edgeData != NULL && b->getLodLevel(i).edgeData != NULL) {
                assertEdgeDataClone(a->getLodLevel(i).edgeData, b->getLodLevel(i).edgeData);
            }
            assertLodUsageClone(a->getLodLevel(i), b->getLodLevel(i));
        }
    }

    CPPUNIT_ASSERT(a->getNumSubMeshes() == b->getNumSubMeshes());
    int numLods = std::min(a->getNumLodLevels(), b->getNumLodLevels());
    int numSubmeshes = a->getNumSubMeshes();
    for (int i = 0; i < numSubmeshes; i++) {
        SubMesh* aSubmesh = a->getSubMesh(i);
        SubMesh* bSubmesh = b->getSubMesh(i);

        CPPUNIT_ASSERT(aSubmesh->getMaterialName() == bSubmesh->getMaterialName());
        CPPUNIT_ASSERT(aSubmesh->isMatInitialised() == bSubmesh->isMatInitialised());
        CPPUNIT_ASSERT(aSubmesh->useSharedVertices == bSubmesh->useSharedVertices);
        CPPUNIT_ASSERT(aSubmesh->getVertexAnimationIncludesNormals() == bSubmesh->getVertexAnimationIncludesNormals());
        CPPUNIT_ASSERT(aSubmesh->getVertexAnimationType() == bSubmesh->getVertexAnimationType());
        CPPUNIT_ASSERT(aSubmesh->getTextureAliasCount() == bSubmesh->getTextureAliasCount());
        CPPUNIT_ASSERT(isContainerClone(aSubmesh->blendIndexToBoneIndexMap, bSubmesh->blendIndexToBoneIndexMap));
        // TODO: Compare getBoneAssignments and getTextureAliases
        for (int n = 0; n < numLods; n++) {
            if (a->_isManualLodLevel(n)) {
                continue;
            }
            RenderOperation aop, bop;
            aSubmesh->_getRenderOperation(aop, n);
            bSubmesh->_getRenderOperation(bop, n);
            assertIndexDataClone(aop.indexData, bop.indexData);
            assertVertexDataClone(aop.vertexData, bop.vertexData);
            CPPUNIT_ASSERT(aop.operationType == bop.operationType);
            CPPUNIT_ASSERT(aop.useIndexes == bop.useIndexes);
        }
    }
}
//--------------------------------------------------------------------------
bool MeshSerializerTests::isLodMixed(const Mesh* pMesh)
{
    if (!pMesh->hasManualLodLevel()) {
        return false;
    }

    unsigned short numLods = pMesh->getNumLodLevels();
    for (unsigned short i = 1; i < numLods; ++i) {
        if (!pMesh->_isManualLodLevel(i)) {
            return true;
        }
    }

    return false;
}
//--------------------------------------------------------------------------
void MeshSerializerTests::assertVertexDataClone(VertexData* a, VertexData* b, MeshVersion version /*= MESH_VERSION_LATEST*/)
{
    CPPUNIT_ASSERT((a == NULL) == (b == NULL));
    if (a) {
        // compare bindings
        {
            const VertexBufferBinding::VertexBufferBindingMap& aBindings = a->vertexBufferBinding->getBindings();
            const VertexBufferBinding::VertexBufferBindingMap& bBindings = b->vertexBufferBinding->getBindings();
            CPPUNIT_ASSERT(aBindings.size() == bBindings.size());
            typedef VertexBufferBinding::VertexBufferBindingMap::const_iterator bindingIterator;
            bindingIterator aIt = aBindings.begin();
            bindingIterator aEndIt = aBindings.end();
            bindingIterator bIt = bBindings.begin();
            for (; aIt != aEndIt; aIt++, bIt++) {
                CPPUNIT_ASSERT(aIt->first == bIt->first);
                CPPUNIT_ASSERT((aIt->second.get() == NULL) == (bIt->second.get() == NULL));
                if (a) {
                    CPPUNIT_ASSERT(aIt->second->getManager() == bIt->second->getManager());
                    CPPUNIT_ASSERT(aIt->second->getNumVertices() == bIt->second->getNumVertices());
                }
            }
        }

        {
            const VertexDeclaration::VertexElementList& aElements = a->vertexDeclaration->getElements();
            const VertexDeclaration::VertexElementList& bElements = a->vertexDeclaration->getElements();
            CPPUNIT_ASSERT(aElements.size() == bElements.size());
            typedef VertexDeclaration::VertexElementList::const_iterator bindingIterator;
            bindingIterator aIt = aElements.begin();
            bindingIterator aEndIt = aElements.end();
            bindingIterator bIt;
            for (; aIt != aEndIt; aIt++) {
                bIt = std::find(bElements.begin(), bElements.end(), *aIt);
                CPPUNIT_ASSERT(bIt != bElements.end());

#ifndef OGRE_TEST_XMLSERIALIZER
                const VertexElement& aElem = *aIt;
                const VertexElement& bElem = *bIt;
                HardwareVertexBufferSharedPtr abuf = a->vertexBufferBinding->getBuffer(aElem.getSource());
                HardwareVertexBufferSharedPtr bbuf = b->vertexBufferBinding->getBuffer(bElem.getSource());
                unsigned char* avertex = static_cast<unsigned char*>(abuf->lock(HardwareBuffer::HBL_READ_ONLY));
                unsigned char* bvertex = static_cast<unsigned char*>(bbuf->lock(HardwareBuffer::HBL_READ_ONLY));
                size_t avSize = abuf->getVertexSize();
                size_t bvSize = bbuf->getVertexSize();
                size_t elemSize = VertexElement::getTypeSize(aElem.getType());
                unsigned char* avEnd = avertex + a->vertexCount * avSize;
                bool error = false;
                for (; avertex < avEnd; avertex += avSize, bvertex += bvSize) {
                    float* afloat, * bfloat;
                    aElem.baseVertexPointerToElement(avertex, &afloat);
                    bElem.baseVertexPointerToElement(bvertex, &bfloat);
                    error |= (memcmp(afloat, bfloat, elemSize) != 0);
                }
                abuf->unlock();
                bbuf->unlock();
                CPPUNIT_ASSERT(!error && "Content of vertex buffer differs!");
#endif /* ifndef OGRE_TEST_XMLSERIALIZER */
            }
        }

        CPPUNIT_ASSERT(a->vertexStart == b->vertexStart);
        CPPUNIT_ASSERT(a->vertexCount == b->vertexCount);
        CPPUNIT_ASSERT(a->hwAnimDataItemsUsed == b->hwAnimDataItemsUsed);

        // Compare hwAnimationData
        {
            const VertexData::HardwareAnimationDataList& aAnimData = a->hwAnimationDataList;
            const VertexData::HardwareAnimationDataList& bAnimData = b->hwAnimationDataList;
            CPPUNIT_ASSERT(aAnimData.size() == bAnimData.size());
            typedef VertexData::HardwareAnimationDataList::const_iterator bindingIterator;
            bindingIterator aIt = aAnimData.begin();
            bindingIterator aEndIt = aAnimData.end();
            bindingIterator bIt = bAnimData.begin();
            for (; aIt != aEndIt; aIt++, bIt++) {
                CPPUNIT_ASSERT(aIt->parametric == bIt->parametric);
                CPPUNIT_ASSERT(aIt->targetBufferIndex == bIt->targetBufferIndex);
            }
        }
    }
}
//--------------------------------------------------------------------------
template<typename T>
bool MeshSerializerTests::isContainerClone(T& a, T& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}
//--------------------------------------------------------------------------
template<typename K, typename V>
bool MeshSerializerTests::isHashMapClone(const OGRE_HashMap<K, V>& a, const OGRE_HashMap<K, V>& b)
{
    // if you recreate a HashMap with same elements, then iteration order may differ!
    // So isContainerClone is not always working on HashMap.
    if (a.size() != b.size()) {
        return false;
    }
    typename OGRE_HashMap<K, V>::const_iterator it, itFind, itEnd;
    it = a.begin();
    itEnd = a.end();
    for (; it != itEnd; it++) {
        itFind = b.find(it->first);
        if (itFind == b.end() || itFind->second != it->second) {
            return false;
        }
    }
    return true;
}
//--------------------------------------------------------------------------
void MeshSerializerTests::assertIndexDataClone(IndexData* a, IndexData* b, MeshVersion version /*= MESH_VERSION_LATEST*/)
{
    CPPUNIT_ASSERT((a == NULL) == (b == NULL));
    if (a) {
        CPPUNIT_ASSERT(a->indexCount == b->indexCount);
        // CPPUNIT_ASSERT(a->indexStart == b->indexStart);
        CPPUNIT_ASSERT((a->indexBuffer.get() == NULL) == (b->indexBuffer.get() == NULL));
        if (!a->indexBuffer.isNull()) {
            CPPUNIT_ASSERT(a->indexBuffer->getManager() == b->indexBuffer->getManager());
            // CPPUNIT_ASSERT(a->indexBuffer->getNumIndexes() == b->indexBuffer->getNumIndexes());
            CPPUNIT_ASSERT(a->indexBuffer->getIndexSize() == b->indexBuffer->getIndexSize());
            CPPUNIT_ASSERT(a->indexBuffer->getType() == b->indexBuffer->getType());

            char* abuf = (char*) a->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
            char* bbuf = (char*) b->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY);
            size_t size = a->indexBuffer->getIndexSize();
            char* astart = abuf + a->indexStart * size;
            char* bstart = bbuf + b->indexStart * size;
            CPPUNIT_ASSERT(memcmp(astart, bstart, a->indexCount * size) == 0);
            a->indexBuffer->unlock();
            b->indexBuffer->unlock();
        }
    }
}
//--------------------------------------------------------------------------
void MeshSerializerTests::assertEdgeDataClone(EdgeData* a, EdgeData* b, MeshVersion version /*= MESH_VERSION_LATEST*/)
{
    CPPUNIT_ASSERT((a == NULL) == (b == NULL));
    if (a) {
        CPPUNIT_ASSERT(a->isClosed == b->isClosed);
        CPPUNIT_ASSERT(isContainerClone(a->triangleFaceNormals, b->triangleFaceNormals));
        CPPUNIT_ASSERT(isContainerClone(a->triangleLightFacings, b->triangleLightFacings));
        // TODO: Compare triangles and edgeGroups in more detail.
        CPPUNIT_ASSERT(a->triangles.size() == b->triangles.size());
        CPPUNIT_ASSERT(a->edgeGroups.size() == b->edgeGroups.size());
    }
}
//--------------------------------------------------------------------------
void MeshSerializerTests::assertLodUsageClone(const MeshLodUsage& a, const MeshLodUsage& b, MeshVersion version /*= MESH_VERSION_LATEST*/)
{
    CPPUNIT_ASSERT(a.manualName == b.manualName);
    CPPUNIT_ASSERT(isEqual(a.userValue, b.userValue));
    CPPUNIT_ASSERT(isEqual(a.value, b.value));
}
//--------------------------------------------------------------------------
void MeshSerializerTests::getResourceFullPath(const ResourcePtr& resource, String& outPath)
{
    ResourceGroupManager& resourceGroupMgr = ResourceGroupManager::getSingleton();
    String group = resource->getGroup();
    String name = resource->getName();
    FileInfo* info = NULL;
    FileInfoListPtr locPtr = resourceGroupMgr.listResourceFileInfo(group);
    FileInfoList::iterator it, itEnd;
    it = locPtr->begin();
    itEnd = locPtr->end();
    for (; it != itEnd; it++) {
        if (stricmp(name.c_str(), it->filename.c_str()) == 0) {
            info = &*it;
            break;
        }
    }
    if(!info) {
        outPath = name;
        return;
    }
    outPath = info->archive->getName();
    if (outPath[outPath .size()-1] != '/' && outPath[outPath .size()-1] != '\\') {
        outPath += '/';
    }
    outPath += info->path;
    if (outPath[outPath .size()-1] != '/' && outPath[outPath .size()-1] != '\\') {
        outPath += '/';
    }
    outPath += info->filename;

    assert(info->archive->getType() == "FileSystem");
}
//--------------------------------------------------------------------------
bool MeshSerializerTests::copyFile(const String& srcPath, const String& dstPath)
{
    std::ifstream src(srcPath.c_str(), std::ios::binary);
    if (!src.is_open()) {
        return false;
    }
    std::ofstream dst(dstPath.c_str(), std::ios::binary);
    if (!dst.is_open()) {
        return false;
    }

    dst << src.rdbuf();
    return true;
}
//--------------------------------------------------------------------------
bool MeshSerializerTests::isEqual(Real a, Real b)
{
    Real absoluteError = std::abs(a * mErrorFactor);
    return ((a - absoluteError) <= b) && ((a + absoluteError) >= b);
}
//--------------------------------------------------------------------------
bool MeshSerializerTests::isEqual(const Vector3& a, const Vector3& b)
{
    return isEqual(a.x, b.x) && isEqual(a.y, b.y) && isEqual(a.z, b.z);
}
//--------------------------------------------------------------------------
