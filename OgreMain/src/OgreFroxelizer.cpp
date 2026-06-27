#include "OgreStableHeaders.h"

#include "OgreFroxelizer.h"
#include "OgreViewport.h"

namespace Ogre
{
/// Compute the froxel layout (XY tile count + tile size) for a given viewport and buffer budget
static Vector4f computeFroxelLayout(const Viewport* viewport, int sliceCount, uint32 bufferEntryCount)
{
    int width = std::max(1, viewport->getActualWidth());
    int height = std::max(1, viewport->getActualHeight());
    sliceCount = std::max(2, sliceCount);

    // Number of froxels in the XY plane; the Z slices consume buffer entries:
    //     froxelPlaneCount = bufferEntryCount / sliceCount
    const size_t froxelPlaneCount = bufferEntryCount / sliceCount;

    // Goal: countX * countY <= froxelPlaneCount with near-square froxels:
    //     countX / countY ~= aspect  =>  countY <= sqrt(froxelPlaneCount / aspect)
    const float aspect = float(width) / float(height);

    uint32 countYTmp = uint32(std::sqrt(double(froxelPlaneCount) / double(aspect)));
    countYTmp = std::max<uint32>(1u, countYTmp);
    uint32 countXTmp = std::max<uint32>(1u, uint32(froxelPlaneCount) / countYTmp);

    // Square froxel edge length in pixels (round up to the larger ratio so the
    // resulting tile count never exceeds the budget)
    uint32 tileSizePx = uint32(std::ceil(std::max(float(width) / float(countXTmp), float(height) / float(countYTmp))));
    tileSizePx = std::max<uint32>(1u, tileSizePx);

    // Final tile count derived from the final tile size
    uint32 countX = (width + tileSizePx - 1) / tileSizePx;
    uint32 countY = (height + tileSizePx - 1) / tileSizePx;

    const RenderTarget* rt = viewport->getTarget();
    const float yFix = rt->requiresTextureFlipping() ? float(height) : -1.0f;

    return Vector4f(countX, countY, yFix, tileSizePx);
}

/// Compute the logarithmic view-space Z slicing parameters used for froxel lighting.
static Vector4f getFroxelZParams(const Frustum* frustum, uint32 sliceCount)
{
    sliceCount = std::max(2u, sliceCount);

    float n = frustum->getNearClipDistance();
    float f = frustum->getBoundingRadius(); // always finite!

    // always standard depth [0,1]; shader flips depth itself for reversed-z
    const float scaleZ = -f * (f - n) / (f * n);
    const float biasZ  =  f / n;
    const float linZ   = -(float)sliceCount / std::log2(f / n); // negative

    return Vector4f(scaleZ, biasZ, linZ, sliceCount);
}

//-----------------------------------------------------------------------------
void Froxelizer::rebuildLayout(const Camera* cam, const Viewport* viewport)
{
    mTileParams = computeFroxelLayout(viewport, MAX_FROXEL_SLICES, MAX_FROXELS);
    mDepthParams = getFroxelZParams(cam, MAX_FROXEL_SLICES);

    mLastWidth  = viewport->getActualWidth();
    mLastHeight = viewport->getActualHeight();
}

//-----------------------------------------------------------------------------
void Froxelizer::binLights(const Camera* cam, const LightList& lights)
{
    auto cx = int(mTileParams[0]), cy = int(mTileParams[1]), cz = int(mDepthParams[3]);
    const uint32 froxelCount = uint32(cx * cy * cz);

    mGrid.assign(MAX_FROXELS, 0);
    mRecords.clear();

    if (lights.empty() || froxelCount == 0)
        return;

    mFroxelLights.resize(froxelCount);
    for (auto& bucket : mFroxelLights)
        bucket.clear();

    const Affine3& view = cam->getViewMatrix();

    // SAME sources as getFroxelZParams() -> consistent slicing with the shader
    const float zNear    = cam->getNearClipDistance();
    const float zFar     = cam->getBoundingRadius();          // finite
    const float invLogFN = 1.0f / std::log2(zFar / zNear);

    // inverse of shader: slice = sliceCount * log2(L/n) / log2(f/n)
    auto zToSlice = [&](float z) -> int {
        z = Math::Clamp(z, zNear, zFar);
        float s = float(cz) * std::log2(z / zNear) * invLogFN;
        return Math::Clamp(int(std::floor(s)), 0, cz - 1);
    };

    const uint8 lightCount = std::min<uint32>(lights.size(), MAX_LIGHTS);
    for (uint8 li = 0; li < lightCount; ++li)
    {
        const Light* l = lights[li];

        // Directional lights cover the whole frustum -> handle outside the cluster grid
        if (l->getType() == Light::LT_DIRECTIONAL)
            continue;

        const Real    radius   = l->getAttenuationRange();
        const Vector3 posWorld = l->getDerivedPosition();
        const Vector3 centerVS = view * posWorld; // view space (looking down -Z)

        // --- Z slice range from positive linear depth ---
        float zMinLin = -centerVS.z - radius;
        float zMaxLin = -centerVS.z + radius;
        if (zMaxLin <= zNear)
            continue; // entirely behind the near plane

        int sliceMin = zToSlice(zMinLin);
        int sliceMax = zToSlice(zMaxLin);

        // --- Screen-space XY tile range via sphere projection ---
        Real nl, nt, nr, nb; // NDC [-1,1], y up
        cam->projectSphere(Sphere(posWorld, radius), &nl, &nt, &nr, &nb);

        // NDC -> tile coords (flip Y so tile 0 is at the top)
        int tileXMin = int(std::floor((nl * 0.5f + 0.5f) * cx));
        int tileXMax = int(std::floor((nr * 0.5f + 0.5f) * cx));
        int tileYMin = int(std::floor((-nt * 0.5f + 0.5f) * cy));
        int tileYMax = int(std::floor((-nb * 0.5f + 0.5f) * cy));
        if (tileXMin > tileXMax) std::swap(tileXMin, tileXMax);
        if (tileYMin > tileYMax) std::swap(tileYMin, tileYMax);

        // Reject lights entirely outside the screen boundaries
        if (tileXMax < 0 || tileXMin >= cx || tileYMax < 0 || tileYMin >= cy)
            continue;

        tileXMin = Math::Clamp(tileXMin, 0, cx - 1);
        tileXMax = Math::Clamp(tileXMax, 0, cx - 1);
        tileYMin = Math::Clamp(tileYMin, 0, cy - 1);
        tileYMax = Math::Clamp(tileYMax, 0, cy - 1);

        // --- Assign to the froxels inside the conservative AABB ---
        for (int z = sliceMin; z <= sliceMax; ++z)
            for (int y = tileYMin; y <= tileYMax; ++y)
                for (int x = tileXMin; x <= tileXMax; ++x)
                {
                    const uint32 idx = (z * cy + y) * cx + x;
                    auto& bucket = mFroxelLights[idx];
                    if (bucket.size() < MAX_LIGHTS) // count is stored in 8 bits
                        bucket.push_back(li);
                }
    }

    // --- Flatten into grid (offset << 8 | count) + records ---
    for (uint32 f = 0; f < froxelCount; ++f)
    {
        uint32 offset = uint32(mRecords.size());
        uint32 count  = uint32(mFroxelLights[f].size());
        if (mRecords.size() + count > MAX_FROXEL_RECORDS)
            count = uint32(MAX_FROXEL_RECORDS - mRecords.size());

        mGrid[f] = (offset << 8) | (count & 0xFFu);
        mRecords.insert(mRecords.end(), mFroxelLights[f].begin(), mFroxelLights[f].begin() + count);

        if (mRecords.size() >= MAX_FROXEL_RECORDS)
            break;
    }
}

//-----------------------------------------------------------------------------
void Froxelizer::update(const Camera* cam, const Viewport* viewport, const LightList& lights)
{
    bool layoutChanged = viewport->getActualWidth() != mLastWidth || viewport->getActualHeight() != mLastHeight;

    if (layoutChanged)
        rebuildLayout(cam, viewport);

    binLights(cam, lights);
}

} // namespace Ogre