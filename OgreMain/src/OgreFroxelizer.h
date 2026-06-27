#ifndef OGRE_FROXELIZER_H
#define OGRE_FROXELIZER_H

#include "OgreLight.h"
#include "OgrePrerequisites.h"
#include "OgreVector.h"
#include <vector>

namespace Ogre
{

/** Builds a frustum-voxel ("froxel") grid for the active camera and bins
    lights into it, in the style of Filament's Froxelizer. The result feeds
    two GPU buffers (grid + records) consumed by clustered shading. */
struct Froxelizer
{
    Froxelizer() = default;

    /// Rebuild the layout and re-bin the lights if needed
    void update(const Camera* cam, const Viewport* viewport, const LightList& lights);

    // shader-facing scalars
    const Vector4f& getTileParams() const { return mTileParams; } // (countX, countY, yFix, tileSizePx)
    const Vector4f& getDepthParams() const { return mDepthParams; } // (scaleZ, biasZ, linZ, sliceCount)

    // GPU buffer data
    const std::vector<uint32>& getGrid() const { return mGrid; }       // (offset << 8) | count per froxel
    const std::vector<uint8>& getRecords() const { return mRecords; } // flat light-index list

    enum : uint32
    {
        /// 4096 froxels fit in a 16 KiB buffer, the minimum guaranteed in GLES 3.x and Vulkan 1.1
        MAX_FROXELS = 4096,
        MAX_FROXEL_SLICES = 16,
        /// 16384 light indices (uint8) fit in a 16 KiB buffer
        MAX_FROXEL_RECORDS = 16384,
        MAX_LIGHTS = 255 ///< @note stored as uint8 in the froxel records
    };

private:
    void rebuildLayout(const Camera* cam, const Viewport* viewport);
    void binLights(const Camera* cam, const LightList& lights);

    int mLastWidth = 0, mLastHeight = 0;

    Vector4f mTileParams = Vector4f(0);
    Vector4f mDepthParams = Vector4f(0);
    // fixed 4096 entries: (offset << 8) | count
    std::vector<uint32> mGrid;
    // one byte per light index (max 256 lights)
    std::vector<uint8> mRecords;
    // temporary per-froxel light lists
    std::vector<std::vector<uint8>> mFroxelLights;
};

} // namespace Ogre

#endif