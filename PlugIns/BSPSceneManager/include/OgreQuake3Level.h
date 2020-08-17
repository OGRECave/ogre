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
#ifndef __Quake3Level_H__
#define __Quake3Level_H__

#include "OgreBspPrerequisites.h"
#include "OgreQuake3Types.h"
#include "OgreDataStream.h"


namespace Ogre {
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup BSPSceneManager
    *  @{
    */
    /** Support for loading and extracting data from a Quake3 level file.
    This class implements the required methods for opening Quake3 level files
    and extracting the pertinent data within. Ogre supports BSP based levels
    through it's own BspLevel class, which is not specific to any file format,
    so this class is here to source that data from the Quake3 format.
    Quake3 levels include far more than just data for rendering - typically the
    <strong>leaves</strong> of the tree are used for rendering, and <strong>brushes,</strong>
    are used to define convex hulls made of planes for collision detection. There are also
    <strong>entities</strong> which define non-visual elements like player start
    points, triggers etc and <strong>models</strong> which are used for movable
    scenery like doors and platforms. <strong>Shaders</strong> meanwhile are textures
    with extra effects and 'content flags' indicating special properties like
    water or lava.
    I will try to support as much of this as I can in Ogre, but I won't duplicate
    the structure or necessarily use the same terminology. Quake3 is designed for a very specific
    purpose and code structure, whereas Ogre is designed to be more flexible,
    so for example I'm likely to separate game-related properties like surface flags
    from the generics of materials in my implementation.
    This is a utility class only - a single call to loadFromChunk should be
    enough. You should not expect the state of this object to be consistent
    between calls, since it uses pointers to memory which may no longer
    be valid after the original call. This is why it has no accessor methods
    for reading it's internal state.
    */
    class Quake3Level : public ResourceAlloc
    {
    public:
        Quake3Level();

        /** Load just the header information from a Quake3 file. 
        @remarks
            This method loads just the header information from the 
            Quake3 file, in order to estimate the loading time.
        */
        void loadHeaderFromStream(DataStreamPtr& inStream);

        /** Reads Quake3 bsp data from a stream as read from the file.
            Since ResourceManagers generally locate data in a variety of
            places they typically manipulate them as a chunk of data, rather than
            a file pointer since this is unsupported through compressed archives.
            Quake3 files are made up of a header (which contains version info and
            a table of the contents) and 17 'lumps' i.e. sections of data,
            the offsets to which are kept in the table of contents. The 17 types
            are predefined (You can find them in OgreQuake3Types.h)

            @param inStream Stream containing Quake3 data
        */
        void loadFromStream(const DataStreamPtr& inStream);

        /* Extracts the embedded lightmap texture data and loads them as textures.
           Calling this method makes the lightmap texture data embedded in
           the .bsp file available to the renderer. Lightmaps are extracted
           and loaded as Texture objects (subclass specific to RenderSystem
           subclass) and are named "@lightmap1", "@lightmap2" etc.
        */
        void extractLightmaps(void) const;

        /** Utility function read the header and set up pointers. */
        void initialise(bool headerOnly = false);
        /** Utility function read the header and set up counters. */
        void initialiseCounts(void);
        /** Utility function read the header and set up pointers. */
        void initialisePointers(void);

        /** Utility function to return a pointer to a lump. */
        void* getLump(int lumpType);
        int getLumpSize(int lumpType);

        /** Debug method. */
        void dumpContents(void);

        // Internal storage
        // This is ALL temporary. Don't rely on it being static
        MemoryDataStreamPtr mChunk;

        // NB no brushes, fog or local lightvolumes yet
        bsp_header_t* mHeader;
        unsigned char* mLumpStart;

        int* mElements; // vertex indexes for faces
        int mNumElements;

        void* mEntities;
        int mNumEntities;

        bsp_model_t* mModels;
        int mNumModels;

        bsp_node_t* mNodes;
        int mNumNodes;

        bsp_leaf_t* mLeaves;
        int mNumLeaves;

        int* mLeafFaces;     // Indexes to face groups by leaf
        int mNumLeafFaces;

        bsp_plane_t* mPlanes;
        int mNumPlanes;

        bsp_face_t* mFaces;      // Groups of faces
        int mNumFaces;

        bsp_vertex_t* mVertices;
        int mNumVertices;

        bsp_shader_t* mShaders;
        int mNumShaders;

        unsigned char* mLightmaps;
        int mNumLightmaps;

        bsp_vis_t* mVis;

        bsp_brush_t* mBrushes;
        int mNumBrushes;

        bsp_brushside_t* mBrushSides;
        int mNumBrushSides;

        int* mLeafBrushes;      // Groups of indexes to brushes by leaf
        int mNumLeafBrushes;



    };
    /** @} */
    /** @} */
}


#endif
