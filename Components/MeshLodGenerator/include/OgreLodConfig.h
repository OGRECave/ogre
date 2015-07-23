/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef __LogConfig_H_
#define __LogConfig_H_

#include "OgreLodPrerequisites.h"
#include "OgreDistanceLodStrategy.h"

namespace Ogre
{

    struct _OgreLodExport ProfiledEdge
    {
        Vector3 src; // Vertex identifier
        Vector3 dst; // Direction of collapse
        Real cost; // Requested collapse cost
    };

    typedef vector<ProfiledEdge>::type LodProfile;

    /**
     * @brief Structure for automatic Lod configuration.
     */
    struct _OgreLodExport LodLevel
    {
        /**
         * @brief Type of the reduction.
         *
         * Note: The vertex count is determined by unique vertices per submesh.
         * A mesh may have duplicate vertices with same position.
         */
        enum VertexReductionMethod
        {
            /**
             * @brief Percentage of vertexes to be removed from each submesh.
             *
             * Valid range is a number between 0.0 and 1.0
             */
            VRM_PROPORTIONAL,

            /**
             * @brief Exact vertex count to be removed from each submesh.
             *
             * Pass only integers or it will be rounded.
             */
            VRM_CONSTANT,

            /**
             * @brief Reduces the vertices, until the cost is bigger then the given value.
             *
             * Collapse cost is equal to the amount of artifact the reduction causes.
             * This generates the best Lod output, but the collapse cost depends on implementation.
             */
            VRM_COLLAPSE_COST
        };

        /**
         * @brief Distance to swap the Lod.
         *
         * This depends on LodStrategy.
         */
        Real distance;

        /**
         * @brief Reduction method to use.
         *
         * @see ProgressiveMeshGenerator::VertexReductionMethod
         */
        VertexReductionMethod reductionMethod;

        /**
         * @brief The value, which depends on reductionMethod.
         */
        Real reductionValue;

        /**
         * @brief Set's a mesh as the Lod Level for given distance.
         *
         * This allows to generate the Lod levels in third party editors.
         * The mesh should have the same submeshes, same bones and animations, like the original mesh.
         * If you use this parameter, the reduction value and method will be ignored.
         * Using manual mesh is less efficient, because it needs separated vertex buffers.
         */
        String manualMeshName;

        /**
         * @brief This is set by ProgressiveMeshGenerator::build() function.
         *
         * Use Mesh::getNumLodLevels() for generated Lod count.
         */
        size_t outUniqueVertexCount;

        /**
         * @brief Whether the Lod level generation was skipped, because it has same vertex count as the previous Lod level.
         */
        bool outSkipped;
    };

    struct _OgreLodExport LodConfig
    {
        v1::MeshPtr mesh; /// The mesh which we want to reduce.
        LodStrategy* strategy; /// Lod strategy to use.

        typedef vector<LodLevel>::type LodLevelList;
        LodLevelList levels; /// Info about Lod levels

        LodConfig(v1::MeshPtr & _mesh, LodStrategy * _strategy = DistanceLodStrategy::getSingletonPtr());
        LodConfig();

        // Helper functions:
        void createManualLodLevel(Ogre::Real distance, const String& manualMeshName);
        void createGeneratedLodLevel(Ogre::Real distance,
                                     Real reductionValue,
                                     LodLevel::VertexReductionMethod reductionMethod = LodLevel::VRM_PROPORTIONAL);

        struct _OgreLodExport Advanced
        {
            /// Whether you want to process it immediatelly on main thread or you want to use Ogre::WorkQueue.
            /// If you use workqueue the generator will return immediately. After processed in background,
            /// the LodWorkQueueInjector will inject it in frameEnd event when rendering next frame.
            /// Ready LODs can also be injected by calling Root::getSingleton().getWorkQueue()->processResponses().
            /// (disabled by default)
            bool useBackgroundQueue;
            /// If enabled, it allows up to 50% smaller index buffers by storing once shared faces with frame shifting.
            /// There is no performance disadvantage! (enabled by default)
            bool useCompression;
            /// Use vertex normals to improve quality. Bit slower to generate, but it has better quality most of the time.
            /// (enabled by default)
            bool useVertexNormals;
            /// Faces inside a house can't be seen from far away. Weightening outside allows to remove those internal faces.
            /// It makes generation smaller and it is not 100% accurate. Set it to 0.0 to disable.
            /// (disabled by default)
            Ogre::Real outsideWeight;
            /// If outsideWeight is enabled, this will set the angle how deep the algorithm can walk inside the mesh.
            /// This value is an acos number between -1 and 1. (by default it is 0 which means 90 degree)
            Ogre::Real outsideWalkAngle;
            /// If the algorithm makes errors, you can fix it, by adding the edge to the profile.
            LodProfile profile;
            Advanced();
        } advanced;
    };
}
#endif
