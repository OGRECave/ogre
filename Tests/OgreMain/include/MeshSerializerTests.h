#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "OgreLogManager.h"
#include "OgreMesh.h"
#include "OgreMeshSerializer.h"

using namespace Ogre;

class MeshSerializerTests : public CppUnit::TestFixture
{
// CppUnit macros for setting up the test suite
CPPUNIT_TEST_SUITE(MeshSerializerTests);
CPPUNIT_TEST(testMesh_XML);
CPPUNIT_TEST(testSkeleton_Version_1_8);
CPPUNIT_TEST(testSkeleton_Version_1_0);
CPPUNIT_TEST(testMesh_clone);
CPPUNIT_TEST(testMesh_Version_1_9);
CPPUNIT_TEST(testMesh_Version_1_8);
CPPUNIT_TEST(testMesh_Version_1_41);
CPPUNIT_TEST(testMesh_Version_1_4);
CPPUNIT_TEST(testMesh_Version_1_3);
CPPUNIT_TEST(testMesh_Version_1_2);
CPPUNIT_TEST_SUITE_END();
protected:
MeshPtr mMesh;
MeshPtr mOrigMesh;
String mMeshFullPath;
String mSkeletonFullPath;
SkeletonPtr mSkeleton;
Real errorFactor;
public:
void setUp();
void tearDown();
void testSkeleton_Version_1_8();
void testSkeleton_Version_1_0();
void testMesh_clone();
void testMesh_Version_1_9();
void testMesh_Version_1_8();
void testMesh_Version_1_41();
void testMesh_Version_1_4();
void testMesh_Version_1_3();
void testMesh_Version_1_2();
void testMesh_XML();
void testMesh(MeshVersion version);
void assertMeshClone(Mesh* a, Mesh* b, MeshVersion version = MESH_VERSION_LATEST);
void assertVertexDataClone(VertexData* a, VertexData* b, MeshVersion version = MESH_VERSION_LATEST);
void assertIndexDataClone(IndexData* a, IndexData* b, MeshVersion version = MESH_VERSION_LATEST);
void assertEdgeDataClone(EdgeData* a, EdgeData* b, MeshVersion version = MESH_VERSION_LATEST);
void assertLodUsageClone(const MeshLodUsage& a, const MeshLodUsage& b, MeshVersion version = MESH_VERSION_LATEST);

template<typename T>
bool isContainerClone(T& a, T& b);
template<typename K, typename V>
bool isHashMapClone(const HashMap<K, V>& a, const HashMap<K, V>& b);

void getResourceFullPath(const ResourcePtr& resource, String& outPath);
bool copyFile(const String& srcPath, const String& dstPath);
bool isLodMixed(const Mesh* pMesh);
bool isEqual(Real a, Real b);
bool isEqual(const Vector3& a, const Vector3& b);
};
