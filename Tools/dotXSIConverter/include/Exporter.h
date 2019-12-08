#ifndef __EXPORTER_H__
#define __EXPORTER_H__


#include "stdafx.h"
#include "SemanticLayer.h"
#include "OgrePrerequisites.h"
#include "OgreVector.h"
#include "OgreColourValue.h"
#include "OgreMesh.h"

using namespace Ogre;


/* Class for performing a mesh and skeleton export from dotXSI */

class Exporter {
    public:
        /* Constructor to initialize .XSI scene data */
        Exporter(CSLModel* Root);
        virtual ~Exporter();
        /* Function to perform an export to Ogre .mesh. */
        void exportMesh(std::string fileName, std::string skelName);
        /* Function to perform an export to Ogre .skeleton. */
        void exportBones(std::string fileName);
        
    protected:
        /* Internal recursive method for exporting a node in the scene */
        void exportCSLModel(Ogre::Mesh* pMesh, CSLModel* XSIModel);
        /* Export a submesh from the attached information */
        void exportSubMesh(Ogre::Mesh* pMesh, CSLMesh* XSIMesh);
        /* Recursive routine to traverse bone hierarchy tree */
        void recurseBones(Skeleton* pSkel, CSLModel* XSIModel);
        /* Exports animation tracks */
        void exportAnim(Skeleton* pSkel, CSLModel* XSIModel);
        
        /* XSI Scene */
        CSLModel* SceneRoot;
        int boneCount;
        std::string boneArray[OGRE_MAX_NUM_BONES];
        bool root;

        /* This class represents a unique vertex, identified from a unique combination of components. */

        class UniqueVertex {
            public:
                bool initialized;
                Ogre::Vector3 position;
                Ogre::Vector3 normal;
                Ogre::Vector2 uv[OGRE_MAX_TEXTURE_COORD_SETS];
                Ogre::RGBA color;
                /* The index of the next component with the same base details, but some variation. */
                size_t nextIndex;
                UniqueVertex();
                bool operator==(const UniqueVertex& rhs) const;
        };
        
        typedef std::vector<UniqueVertex> UniqueVertexList;
        // Unique vertex list
        UniqueVertexList mUniqueVertices;
        // Dynamic index list
        typedef std::vector<size_t> IndexList;
        IndexList mIndices;

        /* Function to start processing a polygon mesh. */
        void startPolygonMesh(size_t origPositionCount, size_t indexCount);
        /* Try to look up an existing vertex with the same information, or create new one. */
        size_t createOrRetrieveUniqueVertex(size_t originalPositionIndex, const UniqueVertex& vertex);
        /* Templatized method for writing indexes */
        template <typename T> void writeIndexes(T* buf);
        /* Create and fill a vertex buffer */
        void createVertexBuffer(VertexData* vd, unsigned short bufIdx);
};

#endif
