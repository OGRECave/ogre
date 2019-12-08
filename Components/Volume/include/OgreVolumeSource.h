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
#ifndef __Ogre_Volume_Source_H__
#define __Ogre_Volume_Source_H__

#include "OgreVector.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** \addtogroup Optional
    *  @{
    */
    /** \defgroup Volume Volume
    * %Volume Rendering with LOD aimed at terrain
    *  @{
    */

    /** Abstract class defining the density function.
    */
    class _OgreVolumeExport Source
    {
    protected:

        /** Gets the start vector of an intersection. This is needed for restricted volumes
        like discreet grids.
        @param ray
            The ray of the intersection.
        @param maxDistance
            The maximum distance to query the ray for intersections.
        @return
            The start point of the intersection ray.
        */
        virtual Vector3 getIntersectionStart(const Ray &ray, Real maxDistance) const;
        
        /** Gets the end vector of an intersection. This is needed for restricted volumes
        like discreet grids.
        @param ray
            The ray of the intersection.
        @param maxDistance
            The maximum distance to query the ray for intersections.
        @return
            The end point of the intersection ray.
        */
        virtual Vector3 getIntersectionEnd(const Ray &ray, Real maxDistance) const;

    public:

        /// The id of volume files.
        static const uint32 VOLUME_CHUNK_ID;

        /// The version of volume files.
        static const uint16 VOLUME_CHUNK_VERSION;

        /// The amount of items being written as one chunk during serialization.
        static const size_t SERIALIZATION_CHUNK_SIZE;
        
        /** Destructor.
        */
        virtual ~Source(void);

        /** Gets the density value and gradient at a specific position.
        @param position
            The position.
        @return
            A vector with x, y, z containing the gradient and w containing the density.
        */
        virtual Vector4 getValueAndGradient(const Vector3 &position) const = 0;

        /** Gets the density value at a specific position.
        @param position
            The position.
        @return
            The density.
        */
        virtual Real getValue(const Vector3 &position) const = 0;

        /** Serializes a volume source to a discrete grid file with deflated
        compression. To achieve better compression, all density values are clamped
        within a maximum absolute value of (to - from).length() / 16.0. The values
        are scanned in this inner-loop-order: z, x, y. y last because there is usually
        the least isosurface intersection to be expected in the use case of terrain and
        so more often the maximum density value hit. The values are written as 16 Bit
        floats to save space. Note that this process is not lossless, the tradeoff is
        between accuracy of the source-reproduction (smaller voxelWidth) and smaller
        filesize (bigger voxelWidth).
        @param from
            The start point to scan the volume.
        @param to
            The end point to scan the volume.
        @param voxelWidth
            The width of a single cube in the density grid.
        @param file
            The file to write the grid to.
        */
        void serialize(const Vector3 &from, const Vector3 &to, float voxelWidth, const String &file);
        
        /** Same as the other serialize function but with a user definable maximum absolute density value.
        @param from
            The start point to scan the volume.
        @param to
            The end point to scan the volume.
        @param voxelWidth
            The width of a single cube in the density grid.
        @param maxClampedAbsoluteDensity
            The maximum absolute density value to be written into the file, influencing the compression rate.
        @param file
            The file to write the grid to.
        */
        void serialize(const Vector3 &from, const Vector3 &to, float voxelWidth, Real maxClampedAbsoluteDensity, const String &file);

        /** Gets the first intersection of a ray with the volume.
        If you are using this together with the VolumeChunk:
        Beware of the possible scaling or other transformations you did on the Chunk! Do the inverse first
        on the ray origin. Example of a scaling with the factor 10: ray.setOrigin(ray.getOrigin() / (Real)10.0);
        @param ray
            The ray.
        @param result
            Will hold the intersection point if there is an intersection.
        @param scale
            The scaling of the volume compared to the world.
        @param maxIterations
            The maximum amount of iterations on the ray before giving up.
        @param maxDistance
            The maximum distance of the intersection point.
        */
        bool getFirstRayIntersection(const Ray &ray, Vector3 &result, Real scale = (Real)1.0, size_t maxIterations = 5000, Real maxDistance = (Real)10000.0) const;

        /** Gets a factor to come from volume coordinate to world coordinate.
        @return
            The factor, 1.0 in the default implementation.
        */
        Real getVolumeSpaceToWorldSpaceFactor(void) const;
    };

    /** @} */
    /** @} */
}
}

#endif
