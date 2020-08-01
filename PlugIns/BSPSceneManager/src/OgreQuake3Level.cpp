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
#include "OgreQuake3Level.h"
#include "OgreLogManager.h"
#include "OgreTextureManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Quake3Level::Quake3Level()
    {

    }
    //-----------------------------------------------------------------------
    void Quake3Level::loadHeaderFromStream(DataStreamPtr& inStream)
    {
        // Load just the header
        mChunk = MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(sizeof(bsp_header_t)));
        inStream->read(mChunk->getPtr(), sizeof(bsp_header_t));
        // Grab all the counts, header only
        initialise(true);
    }
    //-----------------------------------------------------------------------
    void Quake3Level::loadFromStream(const DataStreamPtr& stream)
    {
        mChunk = MemoryDataStreamPtr(OGRE_NEW MemoryDataStream(stream));
        initialise();

#if OGRE_DEBUG_MODE
        dumpContents();
#endif


    }

#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
    //-----------------------------------------------------------------------
   // byte swapping functions
   static void SwapFourBytes(uint32* dw)
   {
      uint32 tmp;
      tmp =  (*dw & 0x000000FF);
      tmp = ((*dw & 0x0000FF00) >> 0x08) | (tmp << 0x08);
      tmp = ((*dw & 0x00FF0000) >> 0x10) | (tmp << 0x08);
      tmp = ((*dw & 0xFF000000) >> 0x18) | (tmp << 0x08);
      memcpy (dw, &tmp, sizeof(uint32));
   }
   //-----------------------------------------------------------------------
   static void SwapFourBytesGrup (uint32* src, int size)
   {
      uint32* ptr = (uint32*)src;
      int i;
      for (i = 0; i < size/4; ++i) {
         SwapFourBytes (&ptr[i]);
      }
   }
#endif
   //-----------------------------------------------------------------------
    void Quake3Level::initialise(bool headerOnly)
    {
        mHeader = (bsp_header_t*)mChunk->getPtr();

        // Header counts
        initialiseCounts();
        // Data pointers
        if (headerOnly)
        {
            mLumpStart = 0;
        }
        else
        {
            mLumpStart = ((unsigned char*)mHeader) + sizeof(mHeader);
            initialisePointers();
        }


#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        // swap header
        SwapFourBytes ((uint32*)&mHeader->version);
#endif
    }
    //-----------------------------------------------------------------------
    void Quake3Level::initialiseCounts(void)
    {
        mNumEntities = getLumpSize(BSP_ENTITIES_LUMP);
        mNumElements = getLumpSize(BSP_ELEMENTS_LUMP) / sizeof(int);
        mNumFaces = getLumpSize(BSP_FACES_LUMP) / sizeof(bsp_face_t);
        mNumLeafFaces = getLumpSize(BSP_LFACES_LUMP) / sizeof(int);
        mNumLeaves = getLumpSize(BSP_LEAVES_LUMP) / sizeof(bsp_leaf_t);
        mNumLightmaps = getLumpSize(BSP_LIGHTMAPS_LUMP)/BSP_LIGHTMAP_BANKSIZE;
        mNumModels = getLumpSize(BSP_MODELS_LUMP) / sizeof(bsp_model_t);
        mNumNodes = getLumpSize(BSP_NODES_LUMP) / sizeof(bsp_node_t);
        mNumPlanes = getLumpSize(BSP_PLANES_LUMP)/sizeof(bsp_plane_t);
        mNumShaders = getLumpSize(BSP_SHADERS_LUMP)/sizeof(bsp_shader_t);
        mNumVertices = getLumpSize(BSP_VERTICES_LUMP)/sizeof(bsp_vertex_t);
        mNumLeafBrushes = getLumpSize(BSP_LBRUSHES_LUMP)/sizeof(int);
        mNumBrushes = getLumpSize(BSP_BRUSH_LUMP)/sizeof(bsp_brush_t);
        mNumBrushSides = getLumpSize(BSP_BRUSHSIDES_LUMP)/sizeof(bsp_brushside_t);
    }
    //-----------------------------------------------------------------------
    void Quake3Level::initialisePointers(void)
    {
        mEntities = (unsigned char*)getLump(BSP_ENTITIES_LUMP);
        mElements = (int*)getLump(BSP_ELEMENTS_LUMP);
        mFaces = (bsp_face_t*)getLump(BSP_FACES_LUMP);
        mLeafFaces = (int*)getLump(BSP_LFACES_LUMP);
        mLeaves = (bsp_leaf_t*)getLump(BSP_LEAVES_LUMP);
        mLightmaps = (unsigned char*)getLump(BSP_LIGHTMAPS_LUMP);
        mModels = (bsp_model_t*)getLump(BSP_MODELS_LUMP);
        mNodes = (bsp_node_t*)getLump(BSP_NODES_LUMP);
        mPlanes = (bsp_plane_t*) getLump(BSP_PLANES_LUMP);
        mShaders = (bsp_shader_t*) getLump(BSP_SHADERS_LUMP);
        mVis = (bsp_vis_t*)getLump(BSP_VISIBILITY_LUMP);
        mVertices = (bsp_vertex_t*) getLump(BSP_VERTICES_LUMP);
        mLeafBrushes = (int*)getLump(BSP_LBRUSHES_LUMP);
        mBrushes = (bsp_brush_t*) getLump(BSP_BRUSH_LUMP);
        mBrushSides = (bsp_brushside_t*) getLump(BSP_BRUSHSIDES_LUMP);
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        SwapFourBytesGrup ((uint32*)mElements, mNumElements*sizeof(int));
        SwapFourBytesGrup ((uint32*)mFaces, mNumFaces*sizeof(bsp_face_t));
        SwapFourBytesGrup ((uint32*)mLeafFaces, mNumLeafFaces*sizeof(int));
        SwapFourBytesGrup ((uint32*)mLeaves, mNumLeaves*sizeof(bsp_leaf_t));
        SwapFourBytesGrup ((uint32*)mModels, mNumModels*sizeof(bsp_model_t));
        SwapFourBytesGrup ((uint32*)mNodes, mNumNodes*sizeof(bsp_node_t));
        SwapFourBytesGrup ((uint32*)mPlanes, mNumPlanes*sizeof(bsp_plane_t));
        for (int i=0; i < mNumShaders; ++i) {
            SwapFourBytes((uint32*)&mShaders[i].surface_flags);
            SwapFourBytes((uint32*)&mShaders[i].content_flags);
        }   
        SwapFourBytes((uint32*)&mVis->cluster_count);
        SwapFourBytes((uint32*)&mVis->row_size);
        SwapFourBytesGrup ((uint32*)mVertices, mNumVertices*sizeof(bsp_vertex_t));
        SwapFourBytesGrup ((uint32*)mLeafBrushes, mNumLeafBrushes*sizeof(int));
        SwapFourBytesGrup ((uint32*)mBrushes,  mNumBrushes*sizeof(bsp_brush_t));
        SwapFourBytesGrup ((uint32*)mBrushSides, mNumBrushSides*sizeof(bsp_brushside_t));
#endif
    }
    //-----------------------------------------------------------------------
    void* Quake3Level::getLump(int lumpType)
    {
        if (mLumpStart)
        {
       
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            // swap lump offset
            SwapFourBytes ((uint32*)&mHeader->lumps[lumpType].offset);
#endif
            return (unsigned char*)mHeader + mHeader->lumps[lumpType].offset;
        }
        else
        {
            return 0;
        }
    }
    //-----------------------------------------------------------------------
    int Quake3Level::getLumpSize(int lumpType)
    {

#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        // swap lump size
        SwapFourBytes ((uint32*)&mHeader->lumps[lumpType].size);
#endif
        return mHeader->lumps[lumpType].size;
    }
    //-----------------------------------------------------------------------
    void Quake3Level::dumpContents(void)
    {
        std::ofstream of;
        of.open("Quake3Level.log");


        of << "Quake3 level statistics" << std::endl;
        of << "-----------------------" << std::endl;
        of << "Entities     : " << mNumEntities << std::endl;
        of << "Faces        : " << mNumFaces << std::endl;
        of << "Leaf Faces   : " << mNumLeafFaces << std::endl;
        of << "Leaves       : " << mNumLeaves << std::endl;
        of << "Lightmaps    : " << mNumLightmaps << std::endl;
        of << "Elements     : " << mNumElements << std::endl;
        of << "Models       : " << mNumModels << std::endl;
        of << "Nodes        : " << mNumNodes << std::endl;
        of << "Planes       : " << mNumPlanes << std::endl;
        of << "Shaders      : " << mNumShaders << std::endl;
        of << "Vertices     : " << mNumVertices << std::endl;
        of << "Vis Clusters : " << mVis->cluster_count << std::endl;

        of << std::endl;
        of << "-= Shaders =-";
        of << std::endl;
        for (int i = 0; i < mNumShaders; ++i)
        {
            of << "Shader " << i << ": " << mShaders[i].name << std::endl;
        }

        of << std::endl;
        of << "-= Entities =-";
        of << std::endl;
        char* strEnt = strtok((char*)mEntities, "\0");
        while (strEnt != 0)
        {
            of << strEnt << std::endl;
            strEnt = strtok(0, "\0");
        }




        of.close();
    }
    //-----------------------------------------------------------------------
    void Quake3Level::extractLightmaps(void) const
    {
        // Lightmaps are always 128x128x24 (RGB)
        unsigned char* pLightmap = mLightmaps;
        for (int i = 0; i < mNumLightmaps; ++i)
        {
            StringStream name;
            name << "@lightmap" << i;

            // Load, no mipmaps, brighten by factor 2.5
            DataStreamPtr stream(OGRE_NEW MemoryDataStream(pLightmap, 128 * 128 * 3, false));
            Image img; 
            img.loadRawData( stream, 128, 128, 1, PF_BYTE_RGB );
            TextureManager::getSingleton().loadImage( name.str(), 
                ResourceGroupManager::getSingleton().getWorldResourceGroupName(), img, TEX_TYPE_2D, 0, 4.0f );
            pLightmap += BSP_LIGHTMAP_BANKSIZE;
        }


    }
    //-----------------------------------------------------------------------


}
