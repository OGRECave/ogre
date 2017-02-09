#include "Vector3.h"
#include "lwo2mesh.h"
#include "Ogre.h"
#include "OgreMesh.h"
#include "OgreStringConverter.h"
#include "OgreDefaultHardwareBufferManager.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
#include <math.h>
#include <float.h>  // FLT_MIN, FLT_MAX
#include <libgen.h> // dirname(), basename().
#include <string.h> // strtok();
#include <string>   // string class
#endif

#define POLYLIMIT 0x5555
#define POINTLIMIT 0x5555

extern Mesh::LodDistanceList distanceList;
extern Real reduction;
extern bool flags[NUMFLAGS];
extern MaterialSerializer* materialSerializer;
extern char *matPrefix;

extern ostream& nl(ostream& os);

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX

/* We expect the caller to provide an arrays of chars for the output. */
void _splitpath( const char *_fn, char *_drive, char *_dir, char *_node, char *_ext ) {
       
   /* A crazy mix of both c and c++.. */
       
   const char *delimiters = ".";
   char buf[ _MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT + 5 ];
   char *exte, *ddir, *fnam, *_ext_tmp;

   strcpy( buf, _fn );
   strcpy( _drive, "" ); // _drive is always empth on linux.

   if ( String( buf ).empty() ) {
       strcpy( _node, "" );
       strcpy( _dir, "" );
       strcpy( _ext, "" );
       return;
   } 
   
   fnam = basename( buf );
   strcpy( _node, fnam );
   ddir = dirname( buf );
   strcpy( _dir, ddir );
   
   _ext_tmp = strtok( fnam, delimiters );
   while ( ( _ext_tmp = strtok( NULL, delimiters ) ) != NULL ) exte = _ext_tmp;
   strcpy( _ext, exte );

   _node[ strlen(_node) - strlen(_ext) - 1 ] = '\0';

}

/* We expect the caller to provide an array of chars for the output. */
void _makepath( char *_fn, const char *_drive, const char *_dir,
       const char *_node, const char *_ext ) {
   
   /* A crazy mix of both c and c++.. */

   std::string buf("");

   if ( _drive != NULL ) // On Linux, this is usually empty.
       buf += _drive;
       
   if ( _dir != NULL ) { // The directory part.
       if ( std::string(_dir).empty() )
           buf += ".";
       buf += _dir;
       buf += "/";
   }
   
   if ( _node != NULL ) // The filename without the extension.
       buf += _node;
   
   if ( _ext != NULL ) { // The extension.
       if ( std::string( _ext ).compare( 0, 1, "." ) != 0 )
           buf += ".";
       buf += _ext;
   }

   strcpy( _fn, buf.c_str() );
}

#endif

void Lwo2MeshWriter::doExportMaterials()
{
    char
        drive[ _MAX_DRIVE ],
        dir[ _MAX_DIR ],
        node[ _MAX_FNAME ],
        ext[ _MAX_EXT ],
        texname [128];

    unsigned int slength = 0;

    if (flags[RenameMaterials])
    {
        if (flags[UseInteractiveMethod])
        {
            for (unsigned int i = 0; i < object->surfaces.size(); ++i)
            {       
                lwSurface *surface = object->surfaces[i];
                cout << "Rename surface " << surface->name << " to: ";
                cin >> texname;
                surface->setname(texname);
            }
        } else {
            _splitpath( dest, drive, dir, node, ext );

            for (unsigned int i = 0; i < object->surfaces.size(); ++i)
            {
                lwSurface *surface = object->surfaces[i];
                if (flags[UsePrefixMethod])
                    strcpy(texname,matPrefix);
                else
                {
                    strcpy(texname,node);
                    strcat(texname,"_");
                }

                strcat(texname, surface->name);
                surface->setname(texname);
            }
        }
    }

    for (unsigned int i = 0; i < object->surfaces.size(); ++i)
    {       
        lwSurface *surface = object->surfaces[i];

        // Create deferred material so no load
        MaterialPtr ogreMat = MaterialManager::getSingleton().getByName(surface->name);
        
        if (!ogreMat)
        {
            ogreMat = MaterialManager::getSingleton().create(surface->name, 
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            
            ogreMat->setAmbient
            (
                surface->color.rgb[0],
                surface->color.rgb[1],
                surface->color.rgb[2]
            );
            
            ogreMat->setDiffuse
            (
                surface->diffuse.val * surface->color.rgb[0],
                surface->diffuse.val * surface->color.rgb[1],
                surface->diffuse.val * surface->color.rgb[2],
                1.0f
            );
            
            ogreMat->setSpecular
            (
                surface->specularity.val * surface->color.rgb[0],
                surface->specularity.val * surface->color.rgb[1],
                surface->specularity.val * surface->color.rgb[2],
                1.0f
            );
            
            ogreMat->setShininess(surface->glossiness.val);
            
            ogreMat->setSelfIllumination
            (
                surface->luminosity.val * surface->color.rgb[0],
                surface->luminosity.val * surface->color.rgb[1],
                surface->luminosity.val * surface->color.rgb[2]
            );

            unsigned int j;
            lwTexture *tex;
            int cindex;
            lwClip *clip;
            
            for (j = 0; j < surface->color.textures.size(); j++)
            {
                tex = surface->color.textures[j];
                cindex = tex->param.imap->cindex;
                clip = object->lwFindClip(cindex);
                
                if (clip)
                {
                    _splitpath( clip->source.still->name, drive, dir, node, ext );
                    _makepath( texname, 0, 0, node, ext );                  
                    ogreMat->getTechnique(0)->getPass(0)->createTextureUnitState(texname);
                }
            }

            for (j = 0; j < surface->transparency.val.textures.size(); j++)
            {
                tex = surface->transparency.val.textures[j];
                cindex = tex->param.imap->cindex;
                clip = object->lwFindClip(cindex);
                
                if (clip)
                {
                    _splitpath( clip->source.still->name, drive, dir, node, ext );
                    _makepath( texname, 0, 0, node, ext );                  
                    ogreMat->getTechnique(0)->getPass(0)->createTextureUnitState(texname);
                }
            }
            materialSerializer->queueForExport(ogreMat);
        }
    }
}

Skeleton *Lwo2MeshWriter::doExportSkeleton(const String &skelName, int l)
{
    vpolygons bones;
    bones.clear();
    bones.reserve(256);

    vpoints bonepoints;
    bonepoints.clear();
    bonepoints.reserve(512);

    if (l == -1)
    {
        for (l = 0; l < object->layers.size(); ++l)
        {
            copyPoints(-1, ID_BONE, object->layers[l]->points, bonepoints);
            copyPolygons(-1, ID_BONE, object->layers[l]->polygons, bones);
        }
    }
    else
    {
        copyPoints(-1, ID_BONE, object->layers[l]->points, bonepoints);
        copyPolygons(-1, ID_BONE, object->layers[l]->polygons, bones);
    }

    if (!bones.size()) return NULL; // no bones means no skeleton

    SkeletonPtr ogreskel = Ogre::SkeletonManager::getSingleton().create(skelName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    unsigned int i;
    // Create all the bones in turn
    for (i = 0; i < bones.size(); ++i)
    {
        lwPolygon* bone = bones[i];
        if (bone->vertices.size() != 2) continue; // a bone has only 2 sides

        Bone* ogreBone = ogreskel->createBone("Bone");

        Ogre::Vector3 bonePos(bone->vertices[0]->point->x, bone->vertices[0]->point->y, bone->vertices[0]->point->z);

        ogreBone->setPosition(bonePos);
        // Hmm, Milkshape has chosen a Euler angle representation of orientation which is not smart
        // Rotation Matrix or Quaternion would have been the smarter choice
        // Might we have Gimbal lock here? What order are these 3 angles supposed to be applied?
        // Grr, we'll try our best anyway...
        Quaternion qx, qy, qz, qfinal;
/*
        qx.FromAngleAxis(msBoneRot[0], Vector3::UNIT_X);
        qy.FromAngleAxis(msBoneRot[1], Vector3::UNIT_Y);
        qz.FromAngleAxis(msBoneRot[2], Vector3::UNIT_Z);
*/
        // Assume rotate by x then y then z
        qfinal = qz * qy * qx;
        ogreBone->setOrientation(qfinal);
    }
/*
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);

        if (strlen(bone->szParentName) == 0)
        {
        }
        else
        {
            Bone* ogrechild = ogreskel->getBone(bone->szName);
            Bone* ogreparent = ogreskel->getBone(bone->szParentName);

            if (ogrechild == 0)
            {
                continue;
            }
            if (ogreparent == 0)
            {
                continue;
            }
            // Make child
            ogreparent->addChild(ogrechild);
        }


    }

    // Create the Animation(s)
    doExportAnimations(pModel, ogreskel);

    // Create skeleton serializer & export
    SkeletonSerializer serializer;
    serializer.exportSkeleton(ogreskel, szFile);

    ogreMesh->_notifySkeleton(ogreskel);

    return ogreskel;
*/
    if (ogreskel)
        Ogre::SkeletonManager::getSingleton().remove(ogreskel->getHandle());

    return NULL;
}

#define POSITION_BINDING 0
#define NORMAL_BINDING 1
#define TEXCOORD_BINDING 2

VertexData *Lwo2MeshWriter::setupVertexData(unsigned short vertexCount, VertexData *oldVertexData, bool deleteOldVertexData)
{
    VertexData *vertexData = new VertexData();

    if (oldVertexData)
    {
        // Basic vertex info
        vertexData->vertexStart = oldVertexData->vertexStart;
        vertexData->vertexCount = oldVertexData->vertexCount + vertexCount;

        const VertexBufferBinding::VertexBufferBindingMap bindings = oldVertexData->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbend;
        vbend = bindings.end();

        for (vbi = bindings.begin(); vbi != vbend; ++vbi)
        {
            HardwareVertexBufferSharedPtr srcbuf = vbi->second;
            // create new buffer with the same settings
            HardwareVertexBufferSharedPtr dstBuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                    srcbuf->getVertexSize(), srcbuf->getNumVertices() + vertexCount, srcbuf->getUsage(), srcbuf->isSystemMemory());

            // copy data
            dstBuf->copyData(*srcbuf, 0, 0, srcbuf->getSizeInBytes(), true);

            // Copy binding
            vertexData->vertexBufferBinding->setBinding(vbi->first, dstBuf);
        }

        // Copy elements
        const VertexDeclaration::VertexElementList elems = oldVertexData->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator ei, eiend;
        eiend = elems.end();
        for (ei = elems.begin(); ei != eiend; ++ei)
        {
            vertexData->vertexDeclaration->addElement(
                ei->getSource(),
                ei->getOffset(),
                ei->getType(),
                ei->getSemantic(),
                ei->getIndex() );
        }
        if (deleteOldVertexData) delete oldVertexData;
    }
    else
    {
        vertexData->vertexCount = vertexCount;
        
        VertexBufferBinding* bind = vertexData->vertexBufferBinding;
        VertexDeclaration* decl = vertexData->vertexDeclaration;
        
        decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);
        HardwareVertexBufferSharedPtr pbuf = HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(POSITION_BINDING), vertexData->vertexCount, HardwareBuffer::HBU_DYNAMIC, false);
        bind->setBinding(POSITION_BINDING, pbuf);
        
        decl->addElement(NORMAL_BINDING, 0, VET_FLOAT3, VES_NORMAL);
        HardwareVertexBufferSharedPtr nbuf = HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(NORMAL_BINDING), vertexData->vertexCount, HardwareBuffer::HBU_DYNAMIC, false);
        bind->setBinding(NORMAL_BINDING, nbuf);
        
        decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);
        HardwareVertexBufferSharedPtr tbuf = HardwareBufferManager::getSingleton().createVertexBuffer(decl->getVertexSize(TEXCOORD_BINDING), vertexData->vertexCount, HardwareBuffer::HBU_DYNAMIC, false);
        bind->setBinding(TEXCOORD_BINDING, tbuf);
    }   
    return vertexData;
}

void Lwo2MeshWriter::copyPoints(int surfaceIndex, unsigned long polygontype, vpoints &sourcepoints, vpoints &destpoints)
{
    for (unsigned int i = 0; i < sourcepoints.size(); i++)
    {
        lwPoint *point = sourcepoints[i];
        
        for (unsigned int j = 0; j < point->polygons.size(); j++)
        {
            lwPolygon *polygon = point->polygons[j];
            if (polygon->type == polygontype)
                if (surfaceIndex == -1 || surfaceIndex == polygon->surfidx)
                {
                    destpoints.push_back(point);
                    break;
                }
        }
    }
}

void Lwo2MeshWriter::copyPolygons(int surfaceIndex, unsigned long polygontype, vpolygons &sourcepolygons, vpolygons &destpolygons)
{
    for (unsigned int i = 0; i < sourcepolygons.size(); i++)
    {
        lwPolygon *polygon = sourcepolygons[i];
        if (polygon->type == polygontype)
            if (surfaceIndex == -1 || surfaceIndex == polygon->surfidx)
                destpolygons.push_back(polygon);
    }
}

void Lwo2MeshWriter::copyDataToVertexData(vpoints &points,
                                        vpolygons &polygons,
                                        vvmaps &vmaps,
                                        IndexData *indexData,
                                        VertexData *vertexData,
                                        unsigned short vertexDataOffset)
{
    lwVMap *vmap = 0;
    unsigned int ni;
    
    HardwareVertexBufferSharedPtr pbuf = vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
    HardwareVertexBufferSharedPtr nbuf = vertexData->vertexBufferBinding->getBuffer(NORMAL_BINDING);
    HardwareVertexBufferSharedPtr tbuf = vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
    HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;

    float* pPos = static_cast<float*>(pbuf->lock(HardwareBuffer::HBL_DISCARD));
    float* pNor = static_cast<float*>(nbuf->lock(HardwareBuffer::HBL_DISCARD));
    float* pTex = static_cast<float*>(tbuf->lock(HardwareBuffer::HBL_DISCARD));
    unsigned short *pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));

    for (unsigned int p = 0; p < polygons.size(); p++)
    {
        lwPolygon *polygon = polygons[p];
        
        if (polygon->vertices.size() != 3) continue; // only copy triangles;

        for (unsigned int v = 0; v < polygon->vertices.size(); v++)
        {
            lwVertex *vertex = polygon->vertices[v];
            lwPoint *point = vertex->point;
            unsigned short i = getPointIndex(point, points);

            pIdx[p*3 + v] = vertexDataOffset + i;
            
            ni = (vertexDataOffset + i) * 3;
            
            pPos[ni] = vertex->point->x;
            pPos[ni + 1] = vertex->point->y;
            pPos[ni + 2] = vertex->point->z;
            
            pNor[ni] = vertex->normal.x;
            pNor[ni + 1] = vertex->normal.y;
            pNor[ni + 2] = vertex->normal.z;
            
            bool found = false;
            
            ni = (vertexDataOffset + i) * 2;
            
            for (unsigned int v = 0; v < point->vmaps.size(); v++)
            {
                for (unsigned int vr = 0; vr < vmaps.size(); vr++)
                {
                    vmap = vmaps[vr];
                    if (point->vmaps[v].vmap == vmap)
                    {
                        int n = point->vmaps[v].index;
                        
                        pTex[ni] = vmap->val[n][0];
                        pTex[ni + 1] = 1.0f - vmap->val[n][1];
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
    }
    pbuf->unlock();
    nbuf->unlock();
    tbuf->unlock();
    ibuf->unlock();
}

void Lwo2MeshWriter::prepLwObject(void)
{
    unsigned int l, p;
    
    for (l = 0; l < object->layers.size(); l++)
    {
        lwLayer *layer = object->layers[l];
        
#ifdef _DEBUG
        cout << "Triangulating layer " << l << ", Polygons before: " << layer->polygons.size();
#endif
        layer->triangulatePolygons();
#ifdef _DEBUG
        cout << ", Polygons after: " << layer->polygons.size() << endl;
#endif
        
        // Mirror x-coord for Ogre;
        for (p = 0; p < layer->points.size(); p++)
        {
            layer->points[p]->x *= -1.0f;
            layer->points[p]->polygons.clear();
        }
        // Unscrew the bounding box
        float x = layer->bboxmin.x * -1.0f;
        layer->bboxmin.x = layer->bboxmax.x * -1.0f;
        layer->bboxmax.x = x;
        
        for ( p = 0; p < layer->polygons.size(); p++ )
        {
            lwPolygon *polygon = layer->polygons[ p ];
            for (unsigned int j = 0; j < polygon->vertices.size(); j++ )
                polygon->vertices[ j ]->point->polygons.push_back(polygon);
        }   
        
        for (p = 0; p < layer->polygons.size(); p++)
            layer->polygons[p]->flip();
        
        layer->calculatePolygonNormals();
        layer->calculateVertexNormals();
    }
}

inline int Lwo2MeshWriter::getPointIndex(lwPoint *point, vpoints &points)
{
    for (unsigned int i = 0; i < points.size(); ++i)
        if (points[i] == point) return i;
        
    return -1;
}

inline String Lwo2MeshWriter::makeLayerFileName(char* dest, unsigned int l, char *layername)
{
    char
        drive[ _MAX_DRIVE ],
        dir[ _MAX_DIR ],
        node[ _MAX_FNAME ],
        ext[ _MAX_EXT ],
        buf[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 5 ];

    String LayerFileName;
    String TempName;

    _splitpath( dest, drive, dir, node, ext );

    TempName = String( node );
   
    if (layername) {
        TempName += ".";
        TempName += layername;
    } else {
        TempName += ".layer" + StringConverter::toString(l);
    }

    _makepath( buf, drive, dir, TempName.c_str(), ext );
    LayerFileName = String( buf );

    return LayerFileName;
}

inline String Lwo2MeshWriter::makeMaterialFileName(char* dest)
{
    char
        drive[ _MAX_DRIVE ],
        dir[ _MAX_DIR ],
        node[ _MAX_FNAME ],
        ext[ _MAX_EXT ],
        buf[ _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 5 ];

    String MaterialFileName;

    _splitpath( dest, drive, dir, node, ext );
    _makepath( buf, drive, dir, node, ".material" );
 
    const char *test = MaterialFileName.c_str();
    MaterialFileName = String( buf );

    return MaterialFileName;
}

inline void Lwo2MeshWriter::getTextureVMaps(vtextures &textures, vvmaps &svmaps, vvmaps &dvmaps)
{
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        lwTexture *texture = textures[i];
        
        if (texture->type == ID_IMAP && texture->param.imap)
        {
            char *mapname = texture->param.imap->vmap_name;
            if (mapname)
                for (unsigned int v = 0; v < svmaps.size(); v++)
                {
                    lwVMap *vmap = svmaps[v];
                    if (strcmp(mapname, vmap->name) == 0) dvmaps.push_back(vmap);
                }
        }
    }
    return;
}

bool Lwo2MeshWriter::writeLwo2Mesh(lwObject *nobject, char *ndest)
{
    object = nobject;
    dest = ndest;
    
    if (!object) return false;
    if (!object->layers.size()) return false;
    
    prepLwObject();
    
    vpoints points;
    vpolygons polygons;
    vvmaps vmaps;
    
    MeshSerializer meshserializer;

    if (flags[ExportMaterials])
    {
        doExportMaterials();
        materialSerializer->exportQueued(makeMaterialFileName(dest));
    }

    unsigned int ml = object->layers.size();

    bool SeparateLayers = flags[UseSeparateLayers] && ml > 1;

    if (!SeparateLayers) ogreMesh = Ogre::MeshManager::getSingleton().create(ndest, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Ogre::Vector3 boundingBoxMin(FLT_MAX, FLT_MAX, FLT_MAX);
    Ogre::Vector3 boundingBoxMax(FLT_MIN, FLT_MIN, FLT_MIN);


    for( unsigned int ol = 0; ol < ml; ++ol )
    {
        if (!object->layers[ol]->polygons.size())
            continue;

        Ogre::Vector3 currentMin(object->layers[ol]->bboxmin.x,
                                 object->layers[ol]->bboxmin.y,
                                 object->layers[ol]->bboxmin.z);
        Ogre::Vector3 currentMax(object->layers[ol]->bboxmax.x,
                                 object->layers[ol]->bboxmax.y,
                                 object->layers[ol]->bboxmax.z);

        if (SeparateLayers)
        {   
            ogreMesh = Ogre::MeshManager::getSingleton().create(ndest, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            ogreMesh->_setBounds(Ogre::AxisAlignedBox(currentMin, currentMax));
            ogreMesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(std::max(currentMin.squaredLength(), currentMax.squaredLength())));
        }
        else
        {
            boundingBoxMin.makeFloor(currentMin);
            boundingBoxMax.makeCeil(currentMax);
        }
        
        for (unsigned int s = 0; s < object->surfaces.size(); s++)
        {
            lwSurface *surface = object->surfaces[s];
            
            points.clear();
            polygons.clear();
            vmaps.clear();
            
            unsigned int l = ol;

            for( unsigned int il = 0; il < ml; ++il )
            {
                if (!SeparateLayers) l = il;

                copyPoints(s, ID_FACE, object->layers[l]->points, points);
                copyPolygons(s, ID_FACE, object->layers[l]->polygons, polygons);
                getTextureVMaps(surface->color.textures, object->layers[l]->vmaps, vmaps);

                if (SeparateLayers) break;
            }

            if (!polygons.size()) continue;             
            
            SubMesh *ogreSubMesh = ogreMesh->createSubMesh();
            ogreSubMesh->useSharedVertices = flags[UseSharedVertexData] && points.size() < POINTLIMIT;

            ogreSubMesh->indexData->indexCount = polygons.size() * 3;
            ogreSubMesh->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, ogreSubMesh->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
            ogreSubMesh->setMaterialName(surface->name);
            

            if (ogreSubMesh->useSharedVertices)
            {
                unsigned short vertexDataOffset = 0;
                if (ogreMesh->sharedVertexData) vertexDataOffset = ogreMesh->sharedVertexData->vertexCount;
                ogreMesh->sharedVertexData = setupVertexData(points.size(), ogreMesh->sharedVertexData);
                copyDataToVertexData(points, polygons, vmaps, ogreSubMesh->indexData, ogreMesh->sharedVertexData, vertexDataOffset);
            }
            else
            {
                ogreSubMesh->vertexData = setupVertexData(points.size());
                copyDataToVertexData(points, polygons, vmaps, ogreSubMesh->indexData, ogreSubMesh->vertexData);
            }
        }

        if (!SeparateLayers)
        {   
            ogreMesh->_setBounds(Ogre::AxisAlignedBox(boundingBoxMin, boundingBoxMax));
            ogreMesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(std::max(boundingBoxMin.squaredLength(), boundingBoxMax.squaredLength())));
        }
        
        String fname = SeparateLayers ? makeLayerFileName(dest, ol, object->layers[ol]->name) : dest;

        Skeleton *skeleton = 0;

        if (flags[ExportSkeleton])
            if (SeparateLayers)
                skeleton = doExportSkeleton(fname, ol);
            else
                if (!ol) skeleton = doExportSkeleton(fname, -1);

        if (flags[GenerateLOD])
        {
            ProgressiveMesh::VertexReductionQuota quota;

            if (flags[UseFixedMethod])
                quota = ProgressiveMesh::VRQ_CONSTANT;
            else
                quota = ProgressiveMesh::VRQ_PROPORTIONAL;
                    
            ogreMesh->generateLodLevels(distanceList, quota, reduction);
        }

        if (flags[GenerateEdgeLists])
        {
            ogreMesh->buildEdgeList();
        }

        if (flags[GenerateTangents])
        {
            ogreMesh->buildTangentVectors();
        }
        
        try
        {
            meshserializer.exportMesh(ogreMesh.get(), fname);
        }
        catch (...)
        {
            cout << "Could not export to file: " << fname << endl;
        }

        ogreMesh->unload();

        Ogre::MeshManager::getSingleton().remove(ogreMesh->getHandle());
        if (flags[ExportSkeleton] && skeleton) delete skeleton;

        if (!SeparateLayers) break;
    }

    return true;
}
