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
    const float yFix = !rt->requiresTextureFlipping() ? float(height) : -1.0f;

    return Vector4f(countX, countY, yFix, tileSizePx);
}

/// window-space depth (== gl_FragCoord.z) of an on-axis point at distance z
static float viewZToWindowDepth(const Camera* cam, float z)
{
    const Matrix4& P = cam->getProjectionMatrixWithRSDepth(); // incl. reverse-Z / API depth range
    const Vector4 clip = P * Vector4(0.0f, 0.0f, -z, 1.0f);
    const float ndc = float(clip.z / clip.w);

    RenderSystem* rs = Root::getSingleton().getRenderSystem();
    const float dMin = rs ? float(rs->getMinimumDepthInputValue()) : -1.0f;
    const float dMax = rs ? float(rs->getMaximumDepthInputValue()) :  1.0f;
    return (ndc - dMin) / (dMax - dMin);
}

void Froxelizer::updateDepthParams(const Camera* cam)
{
    const float sliceCount = float(MAX_FROXEL_SLICES);

    mZLightNear = std::max<float>(cam->getNearClipDistance() * 5.0f, 1e-4f);
    mZLightFar = mZLightNear * 20; // corresponds to the 5..100 range in filament

    // 1/z = a*d + b (exact for perspective)
    const float d0 = viewZToWindowDepth(cam, mZLightNear);
    const float d1 = viewZToWindowDepth(cam, mZLightFar);
    OgreAssertDbg(std::abs(d1 - d0) > 1e-6f, "degenerate depth range");
    const float a = (1.0f / mZLightFar - 1.0f / mZLightNear) / (d1 - d0);
    const float b = 1.0f / mZLightNear - a * d0;

    // slice 0 == [0, zLightNear], slices 1..N-1 exponential over [zLightNear, zLightFar]
    const float linearizer = std::log2(mZLightFar / mZLightNear) / (sliceCount - 1.0f);

    mDepthParams = Vector4f(mZLightFar * a, mZLightFar * b, -1.0f / linearizer, sliceCount);
}

/// viewSpaceZ < 0 == in front of the camera. Exact inverse of the shader formula.
int Froxelizer::findSliceZ(float viewSpaceZ) const
{
    const float sliceCount = mDepthParams[3];

    // recipViewZ == mZLightFar / (-viewSpaceZ)
    float s = std::log2(mZLightFar / -viewSpaceZ) * mDepthParams[2] + sliceCount;

    // light center behind the camera (or z == 0) -> first slice
    s = viewSpaceZ < 0.0f ? s : 0.0f;

    return int(Math::Clamp(s, 0.0f, sliceCount - 1.0f)); // clamp then truncate, like the shader
}

//-----------------------------------------------------------------------------
void Froxelizer::rebuildLayout(const Viewport* viewport)
{
    mTileParams = computeFroxelLayout(viewport, MAX_FROXEL_SLICES, MAX_FROXELS);

    mLastWidth  = viewport->getActualWidth();
    mLastHeight = viewport->getActualHeight();
}

//-----------------------------------------------------------------------------
bool Froxelizer::binLights(const Camera* cam, const LightList& lights)
{
    uint32 hash = HashCombine(0, mTileParams);
    hash = HashCombine(hash, mDepthParams);
    for (const Light* l : lights)
    {
        hash = HashCombine(hash, l->getDerivedPosition());
        hash = HashCombine(hash, l->getAttenuationRange());
    }

    if (hash == mLastHash)
        return false;
    mLastHash = hash;

    auto cx = int(mTileParams[0]), cy = int(mTileParams[1]), cz = int(mDepthParams[3]);
    const uint32 froxelCount = uint32(cx * cy * cz);

    mGrid.assign(MAX_FROXELS, 0); // all counts zero, so no lights if we bail out early

    if (lights.empty() || froxelCount == 0)
        return true;

    mFroxelLights.resize(froxelCount);
    for (auto& bucket : mFroxelLights)
    {
        bucket.clear();
        bucket.reserve(MAX_LIGHTS);
    }

    const Affine3& view = cam->getViewMatrix();

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

        if (centerVS.z - radius > 0.0f) // sphere entirely behind the camera
            continue;

        // sphere spans z in [c.z - r, c.z + r]; c.z + r is the near side
        int sliceMin = std::max(findSliceZ(float(centerVS.z + radius)) - 1, 0);
        int sliceMax = std::min(findSliceZ(float(centerVS.z - radius)) + 1, cz - 1);

        // --- Screen-space XY tile range via sphere projection ---
        RealRect lightRect; // NDC [-1,1], y up
        cam->projectSphere(Sphere(posWorld, radius), lightRect);

        // NDC -> tile coords (flip Y so tile 0 is at the top)
        const float w = float(mLastWidth), h = float(mLastHeight), ts = mTileParams[3];
        int tileXMin = int(std::floor(((lightRect.left * 0.5f + 0.5f) * w) / ts));
        int tileXMax = int(std::floor(((lightRect.right * 0.5f + 0.5f) * w) / ts));
        int tileYMin = int(std::floor(((-lightRect.top * 0.5f + 0.5f) * h) / ts));
        int tileYMax = int(std::floor(((-lightRect.bottom * 0.5f + 0.5f) * h) / ts));
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
    mRecords.assign(MAX_FROXEL_RECORDS/4, 0); // 4 light indices per word
    uint32 offset = 0;
    for (uint32 f = 0; f < froxelCount; ++f)
    {
        uint32 count  = uint32(mFroxelLights[f].size());
        if (offset + count > MAX_FROXEL_RECORDS)
            count = uint32(MAX_FROXEL_RECORDS - offset);

        mGrid[f] = (offset << 8) | (count & 0xFFu);
        for (uint32 i = 0; i < count; ++i)
        {
            mRecords[offset >> 2] |= uint32(mFroxelLights[f][i]) << ((offset & 3u) * 8u);
            ++offset;
        }

        if (offset >= MAX_FROXEL_RECORDS)
            break;
    }

    return true;
}

//-----------------------------------------------------------------------------
void Froxelizer::updateLayout(const Camera* cam, const Viewport* viewport)
{
    if (viewport->getActualWidth() != mLastWidth || viewport->getActualHeight() != mLastHeight)
        rebuildLayout(viewport);

    if (cam->getNearClipDistance() * 5 != mZLightNear)
        updateDepthParams(cam);
}

} // namespace Ogre