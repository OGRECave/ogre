/*
    Lwo2MeshWriter based on the MilkShape exporter
    Dennis Verbeek ( dverbeek@hotmail.com )

    Linux port by Magnus Møller Petersen ( magnus@moaner.dk )

    doExportSkeleton is unfinished
*/

#ifndef _LWO2MESH_H_
#define _LWO2MESH_H_

#include "lwObject.h"
#include "Ogre.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX

/* GNU libc has no equivalent to _splitpath() and _makepath(), so we'll write my
 * own using a combination of string functions and dirname() / basname().
 */

// I've pulled the following values from the top of my head.
#define _MAX_DRIVE 256
#define _MAX_FNAME 256
#define _MAX_DIR   256
#define _MAX_EXT   256

// Function prototypes.
void _splitpath( const char *_fn, char *_drive, char *_dir, char *_node, char *_ext );
void _makepath( char *_fn, const char *_drive, const char *_dir, const char *_node,
        const char *_ext );

#endif

using namespace Ogre;

enum Parameters
{
    InfoOnly,
    PrintVMaps,
    UseSharedVertexData,
    UseSeparateLayers,
    GenerateLOD,
    GenerateEdgeLists,
    GenerateTangents,
    UseFixedMethod,
    ExportMaterials,
    RenameMaterials,
    UseInteractiveMethod,
    UseObjectMethod,
    UsePrefixMethod,
    ExportSkeleton,
    HasNormals,
    MakeNewSubMesh,
    LinearCopy
};

#define NUMFLAGS 17

class Lwo2MeshWriter
{
public: 
    bool writeLwo2Mesh(lwObject *nobject, char *ndest);
private:
    void prepLwObject(void);

    void doExportMaterials(void);

    Skeleton *doExportSkeleton(const String &skelName, int layer);

    VertexData *setupVertexData(unsigned short vertexCount, VertexData *oldVertexData = 0, bool deleteOldVertexData = true);
    void copyPoints(int surfaceIndex, unsigned long polygontype, vpoints &sourcepoints, vpoints &destpoints);
    void copyPolygons(int surfaceIndex, unsigned long polygontype, vpolygons &sourcepolygons, vpolygons &destpolygons);
    void copyDataToVertexData(vpoints &points,
        vpolygons &polygons,
        vvmaps &vmaps,
        IndexData *indexData,
        VertexData *vertexData,
        unsigned short vertexDataOffset = 0);

    inline int getPointIndex(lwPoint *point, vpoints &points);
    inline void getTextureVMaps(vtextures &textures, vvmaps &svmaps, vvmaps &dvmaps);

    inline String makeLayerFileName(char* dest, unsigned int l, char *layername);
    inline String makeMaterialFileName(char* dest);

    char *dest;
    lwObject *object;
    MeshPtr ogreMesh;
    
    unsigned int nLayers;
    unsigned int nSurfaces;
    
    unsigned int numPolygons;
    unsigned int *numLayerPolygons;
    unsigned int *numLayerSurfacePolygons;
    unsigned int *numSurfacePolygons;
    
    unsigned int vertexCount;
    unsigned int *numLayerVertices;
    unsigned int *numLayerSurfaceVertices;
    unsigned int *numSurfaceVertices;
};

#endif // _LWO2MESH_H_

