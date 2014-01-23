/*
----------------------------------------------------------
Terrain Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

#include "TerrainTessellationINoise.hlsl"
#include "TerrainTessellationCommon.hlsl"

// Work-around for an optimization rule problem in the June 2010 HLSL Compiler (9.29.952.3111).
// Without this, we get cracks in the terrain.
// see http://support.microsoft.com/kb/2448404
// and http://blogs.msdn.com/b/chuckw/archive/2010/10/27/june-2010-hlsl-compiler-issue-with-tessellation.aspx
#if D3DX_VERSION == 0xa2b
#pragma ruledisable 0x0802405f
#endif

Texture2D g_CoarseHeightMap;
Texture2D g_CoarseGradientMap;
Texture2D g_DetailNoiseTexture;
Texture2D g_DetailNoiseGradTexture;

shared cbuffer UniformVariables
{
	int g_DebugShowTiles;
	int g_DebugShowPatches;
	float g_DetailNoiseScale;
	float g_fDisplacementHeight;
	float g_tileSize;
	float2 g_DetailUVScale;
	int3   g_FractalOctaves;
	float3 g_TextureWorldOffset;
};

cbuffer TessellationBuffer
{
    float2 g_screenSize; //Tessellation factors: x=edge, y=inside, z=MinDistance, w=range
	float g_tessellatedTriWidth;
};

cbuffer MatrixBuffer
{
	float  fDisplacementScale;
	float3 g_EyePos;      // world-space eye position
	float3 g_ViewDir;     // world-space eye direction
	// The proj matrix does not vary between the LOD and view-centre versions.  Only the view matrix varies.
	matrix g_WorldViewProj;
	matrix g_WorldViewLOD, g_WorldViewProjLOD;
	matrix g_Proj;	
};

struct VS_CONTROL_POINT_OUTPUT
{
	float3 vPosition        : POSITION;
    float2 vWorldXZ         : TEXCOORD1;
	Adjacency adjacency     : ADJACENCY_SIZES;
};

struct MeshVertex
{
    float4 vPosition        : SV_Position;
    float2 vWorldXZ         : TEXCOORD1;
    float3 vNormal          : NORMAL;
	float3 debugColour      : COLOR;
};

// There's no vertex data.  Position and UV coords are created purely from vertex and instance IDs.
void ReconstructPosition1(AppVertex input, out float3 pos, out int2 intUV)
{
    float iv = floor(input.VertexId * RECIP_CONTROL_VTX_PER_TILE_EDGE);
    float iu = input.VertexId - iv * CONTROL_VTX_PER_TILE_EDGE;
    float u = iu / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
    float v = iv / (CONTROL_VTX_PER_TILE_EDGE - 1.0);

	// Shrink tiles slightly to show gaps between them.
	float size = g_tileSize;
	if (g_DebugShowTiles)
		size *= 0.98;

	pos = float3(u * size + input.position.x, 0, v * size + input.position.y);
	intUV = int2(iu,iv);
}

/*void ReconstructPosition(AppVertex input, out float3 pos)
{
	int2 dummy;
	ReconstructPosition(input, pos, dummy);
}*/

// There's no vertex data.  Position and UV coords are created purely from vertex and instance IDs.
void ReconstructPosition2(AppVertexTessellation input, out float3 pos, out int2 intUV)
{
    float iv = floor(input.VertexId * RECIP_CONTROL_VTX_PER_TILE_EDGE);
    float iu = input.VertexId - iv * CONTROL_VTX_PER_TILE_EDGE;
    float u = iu / (CONTROL_VTX_PER_TILE_EDGE - 1.0);
    float v = iv / (CONTROL_VTX_PER_TILE_EDGE - 1.0);

	// Shrink tiles slightly to show gaps between them.
	float size = g_tileSize;
	if (g_DebugShowTiles)
		size *= 0.98;

	pos = float3(u * size + input.position.x, 0, v * size + input.position.y);
	intUV = int2(iu,iv);
}

/*void ReconstructPosition(AppVertexTessellation input, out float3 pos)
{
	int2 dummy;
	ReconstructPosition(input, pos, dummy);
}*/

// A very simple, regular procedural terrain for debugging cracks etc.
float debugSineHills(float2 uv)
{
	const float HORIZ_SCALE = 4 * 3.14159, VERT_SCALE = 1;
	uv *= HORIZ_SCALE;
	return VERT_SCALE * (sin(uv.x) + 1) * (sin(uv.y) + 1) - 0.5;
}

float DetailUVScale()
{
	const float DETAIL_UV_SCALE = pow(2, g_FractalOctaves.x + g_FractalOctaves.y + g_FractalOctaves.z - 5);
	return DETAIL_UV_SCALE;
}

float2 DetailNoiseSampleCoords(float2 uv, float2 g_DetailUVScale, float3 g_TextureWorldOffset)
{
	// Texture coords have to be offset by the eye's 2D world position.
	const float2 texOffset = float2(g_TextureWorldOffset.x, -g_TextureWorldOffset.z);

	const float2 detailUV = (texOffset + WORLD_UV_REPEATS * uv) * g_DetailUVScale.x;		// TBD: is WORLD_UV_REPEATS spurious here?
	return detailUV;
}

float ScaleDetailNoise(float coarse, float detail, float g_DetailNoiseScale)
{
	// The 8-bit texture needs a scale + bias.
	detail = 2.0 * detail - 1.0;

	// Note the detail is modulated by the height of the coarse sample, ridge octave style.
	return g_DetailNoiseScale * detail * saturate(coarse);
}

float2 ScaleDetailGrad(float2 grad, float g_DetailNoiseScale)
{
	return g_DetailNoiseScale * ((2 * grad) - 1);		// 2x-1 scale and bias
}

float SampleLevelDetailNoise(float2 uv, float coarse, float g_DetailNoiseScale, float2 g_DetailUVScale, float3 g_TextureWorldOffset)
{
	const int mip = 0;
	float detail = g_DetailNoiseTexture.SampleLevel(SamplerRepeatLinear, DetailNoiseSampleCoords(uv,g_DetailUVScale,g_TextureWorldOffset), mip).r;
	return ScaleDetailNoise(coarse, detail, g_DetailNoiseScale);
}

float2 SampleDetailGradOctaves(float2 uv, float g_DetailNoiseScale, float2 g_DetailUVScale, float3 g_TextureWorldOffset)
{
	// There is a 50x scale built into the texture in order to make better use of the range.  This scale
	// is hardcoded by the GUI in the normal map generator.  Compensate here with a 1/50.
	const float TEX_SCALE = 0.02;
	float2 grad = TEX_SCALE * ScaleDetailGrad(g_DetailNoiseGradTexture.Sample(SamplerRepeatMaxAniso, DetailNoiseSampleCoords(uv,g_DetailUVScale,g_TextureWorldOffset)).ra,g_DetailNoiseScale);
	return grad;
}

float2 worldXZtoHeightUV(float2 worldXZ)
{
	// [-8,8] -> [0,1]  TBD: Ought to depend on world size though.
	return worldXZ / 16 + 0.5;
}

float2 heightUVToWorldXZ(float2 uv)
{
	// [0,1] -> [-8,8]  TBD: Ought to depend on world size though.
	return 16 * (uv - 0.5);
}

// Wrappers for displacement map sampling allow us to substitute a 100% procedural surface.
// VS and DS sampling have to agree and use common code.
float SampleHeightForVS(Texture2D coarseTex, SamplerState coarseSampler, float2 worldXZ, int2 offset, float g_DetailNoiseScale, float2 g_DetailUVScale /*, int3 g_FractalOctaves*/, float3 g_TextureWorldOffset)
{
	const float2 uv = worldXZtoHeightUV(worldXZ);
	const int mipLevel = 0;
	
	// You can implement purely procedural terrain here, evaluating the height fn on-the-fly.
	// But for any complex function, with loads of octaves, it's faster to sample from a texture.
	// return debugSineHills(uvCol);						// Simple test shape.
	// return 1.5 * hybridTerrain(uvCol, g_FractalOctaves);	// Procedurally sampled fractal terrain.
	
	// Fractal terrain sampled from texture.  This is a render target that is updated when the view 
	// point changes.  There are two levels of textures here.  1) The low-res ridge noise map that is 
	// initialized by the deformation effect.  2) A detail texture that contains 5 octaves of 
	// repeating fBm.
	float coarse = coarseTex.SampleLevel(coarseSampler, uv, mipLevel, offset).r;	// coarse

	return VERTICAL_SCALE * (coarse + SampleLevelDetailNoise(uv, coarse, g_DetailNoiseScale, g_DetailUVScale, g_TextureWorldOffset));		// detail
}

float SampleHeightForVS(Texture2D tex, SamplerState sam, float2 worldXZ, float g_DetailNoiseScale,float2 g_DetailUVScale/*, int3 g_FractalOctaves*/, float3 g_TextureWorldOffset)
{
	int2 offset = 0;
	return SampleHeightForVS(tex, sam, worldXZ, offset, g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
}

MeshVertex VTFDisplacementVS(AppVertex input)
{
    MeshVertex output = (MeshVertex) 0;
    float3 displacedPos;
	int2 intUV;
	ReconstructPosition1(input, displacedPos, intUV);
    
    float z  = g_fDisplacementHeight * SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, displacedPos.xz, int2(1,1), g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
    float z2 = g_fDisplacementHeight * SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, displacedPos.xz, int2(2,1), g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
    float z3 = g_fDisplacementHeight * SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, displacedPos.xz, int2(1,2), g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
    
    float3 normal = float3(z2 - z, 1.0 / CONTROL_VTX_PER_TILE_EDGE, z3 - z);
    
	displacedPos.y += z;

    output.vPosition = mul( float4( displacedPos, 1.0f ), g_WorldViewProj );
	output.vWorldXZ = displacedPos.xz;
    output.vNormal = normalize(normal);
	output.debugColour = float3(1, 0.1, 0);
	output.vNormal = float3(1,1,1);
	//output.g_DebugShowPatches = g_DebugShowPatches;
	//output.g_DetailNoiseScale = g_DetailNoiseScale;
	//output.g_fDisplacementHeight = g_fDisplacementHeight;
	//output.g_DetailUVScale = g_DetailUVScale;
	/*output.g_FractalOctaves = g_FractalOctaves; */
	//output.g_TextureWorldOffset = g_TextureWorldOffset;
    // For debugging, darken a chequer board pattern of tiles to highlight tile boundaries.
	if (g_DebugShowPatches)
	    output.vNormal *= (0.5 * (((uint) intUV.x + (uint) intUV.y) % 2) + 0.5);

    return output;
}

VS_CONTROL_POINT_OUTPUT HwTessellationPassThruVS( AppVertexTessellation input )
{
    VS_CONTROL_POINT_OUTPUT output;
	int2 intUV;
	ReconstructPosition2(input, output.vPosition, intUV);

	float z = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, output.vPosition.xz, g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
	output.vPosition.y += z;
	output.vWorldXZ = output.vPosition.xz;
	output.adjacency = input.adjacency;
	//output.g_DetailNoiseScale = g_DetailNoiseScale;
	//output.g_DetailUVScale = g_DetailUVScale;
	/*output.g_FractalOctaves = g_FractalOctaves; */
	//output.g_TextureWorldOffset = g_TextureWorldOffset;
	
	return output;
}

//--------------------------------------------------------------------------------------
// Constant data function for the Terrain tessellation.  This is executed once per patch.
//--------------------------------------------------------------------------------------
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4]        : SV_TessFactor;
	float Inside[2]       : SV_InsideTessFactor;
    float2 vWorldXZ[4]    : TEXCOORD4;
	float3 debugColour[5] : COLOR;			// 5th is centre
};

struct HS_OUTPUT
{
    float3 vPosition : POSITION;
};

// These are one colour for each tessellation level and linear graduations between.
static const float3 DEBUG_COLOURS[6] = 
{ 
	float3(0,0,1),  //  2 - blue
	float3(0,1,1),	//  4 - cyan
	float3(0,1,0),	//  8 - green
	float3(1,1,0),	// 16 - yellow
	float3(1,0,1),	// 32 - purple
	float3(1,0,0),	// 64 - red
};

float ClipToScreenSpaceTessellation(float4 clip0, float4 clip1)
{
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= g_screenSize;
	clip1.xy *= g_screenSize;

	const float d = distance(clip0, clip1);

	// g_tessellatedTriWidth is desired pixels per tri edge
	return clamp(d / g_tessellatedTriWidth, 0,64);
}

// Project an edge into clip space and return the number of triangles that are required to fit across it
// in screenspace.
float EdgeToScreenSpaceTessellation(float3 p0, float3 p1)
{
	float4 clip0 = mul(float4(p0,1), g_WorldViewProjLOD);
	float4 clip1 = mul(float4(p1,1), g_WorldViewProjLOD);
	return ClipToScreenSpaceTessellation(clip0, clip1);
}

// Project a sphere into clip space and return the number of triangles that are required to fit across the 
// screenspace diameter.  (For convenience of the caller, we expect two edge end points and use the mid point as centre.)
float SphereToScreenSpaceTessellation(float3 p0, float3 p1, float diameter)
{
	float3 centre = 0.5 * (p0+p1);
	float4 view0 = mul(float4(centre,1), g_WorldViewLOD);
	float4 view1 = view0;
	view1.x += WORLD_SCALE * diameter;

	float4 clip0 = mul(view0, g_Proj);
	float4 clip1 = mul(view1, g_Proj);
	return ClipToScreenSpaceTessellation(clip0, clip1);
}

// Lifted from Tim's Island demo code.
bool inFrustum(const float3 pt, const float3 eyePos, const float3 viewDir, float margin)
{
	// conservative frustum culling
	float3 eyeToPt = pt - eyePos;
	float3 patch_to_camera_direction_vector = viewDir * dot(eyeToPt, viewDir) - eyeToPt;
	float3 patch_center_realigned = pt + normalize(patch_to_camera_direction_vector) * min(margin, length(patch_to_camera_direction_vector));
	float4 patch_screenspace_center = mul(float4(patch_center_realigned, 1.0), g_WorldViewProjLOD);

	if(((patch_screenspace_center.x/patch_screenspace_center.w > -1.0) && (patch_screenspace_center.x/patch_screenspace_center.w < 1.0) &&
		(patch_screenspace_center.y/patch_screenspace_center.w > -1.0) && (patch_screenspace_center.y/patch_screenspace_center.w < 1.0) &&
		(patch_screenspace_center.w>0)) || (length(pt-eyePos) < margin))
	{
		return true;
	}

	return false;
}

// The adjacency calculations ensure that neighbours have tessellations that agree.
// However, only power of two sizes *seem* to get correctly tessellated with no cracks.
float SmallerNeighbourAdjacencyClamp(float tess)
{
	// Clamp to the nearest larger power of two.  Any power of two works; larger means that we don't lose detail.
	// Output is [4,64].
	float logTess = ceil(log2(tess));
	float t = pow(2, logTess);

	// Our smaller neighbour's min tessellation is pow(2,1) = 2.  As we are twice its size, we can't go below 4.
	return max(4, t);
}

float LargerNeighbourAdjacencyClamp(float tess)
{
	// Clamp to the nearest larger power of two.  Any power of two works; larger means that we don't lose detail.
	float logTess = ceil(log2(tess));
	float t = pow(2, logTess);

	// Our larger neighbour's max tessellation is 64; as we are half its size, our tessellation must max out
	// at 32, otherwise we could be over-tessellated relative to the neighbour.  Output is [2,32].
	return clamp(t, 2,32);
}

void MakeVertexHeightsAgree(inout float3 p0, inout float3 p1/*, float g_DetailNoiseScale, float g_DetailUVScale*/)
{
	// This ought to work: if the adjacency has repositioned a vertex in XZ, we need to re-acquire its height.
	// However, causes an internal fxc error.  Again! :-(
	//float h0 = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, p0.xz, g_DetailNoiseScale, g_DetailUVScale);
	//float h1 = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, p0.xz, g_DetailNoiseScale, g_DetailUVScale);
	//p0.y = h0;
	//p1.y = h1;

	// Instead set both vertex heights to zero.  It's the only way I can think to agree with the neighbours
	// when sampling is broken in fxc.
	p0.y = p1.y = 0;
}

float SmallerNeighbourAdjacencyFix(float3 p0, float3 p1, float diameter)
{
	MakeVertexHeightsAgree(p0, p1);
	float t = SphereToScreenSpaceTessellation(p0, p1, diameter);
	return SmallerNeighbourAdjacencyClamp(t);
}

float LargerNeighbourAdjacencyFix(float3 p0, float3 p1, int patchIdx, float diameter)
{
	// We move one of the corner vertices in 2D (x,z) to match where the corner vertex is 
	// on our larger neighbour.  We move p0 or p1 depending on the even/odd patch index.
	//
	// Larger neighbour
	// +-------------------+
	// +---------+
	// p0   Us   p1 ---->  +		Move p1
	// |    0    |    1    |		patchIdx % 2 
	//
	//           +---------+
	// +  <----  p0   Us   p1		Move p0
	// |    0    |    1    |		patchIdx % 2 
	//
	if (patchIdx % 2)
		p0 += (p0 - p1);
	else
		p1 += (p1 - p0);

	// Having moved the vertex in (x,z), its height is no longer correct.
	MakeVertexHeightsAgree(p0, p1);

	// Half the tessellation because the edge is twice as long.
	float t = 0.5 * SphereToScreenSpaceTessellation(p0, p1, 2*diameter);
	return LargerNeighbourAdjacencyClamp(t);
}	

// This constant hull shader is executed once per patch.
HS_CONSTANT_DATA_OUTPUT TerrainScreenspaceLODConstantsHS(InputPatch<VS_CONTROL_POINT_OUTPUT, 4> ip, 
														 uint PatchID : SV_PrimitiveID)
{    
    HS_CONSTANT_DATA_OUTPUT Output = (HS_CONSTANT_DATA_OUTPUT) 0;

	const float3 centre = 0.25 * (ip[0].vPosition + ip[1].vPosition + ip[2].vPosition + ip[3].vPosition);
	const float  sideLen = max(abs(ip[1].vPosition.x - ip[0].vPosition.x), abs(ip[1].vPosition.x - ip[2].vPosition.x));		// assume square & uniform
	const float  diagLen = sqrt(2*sideLen*sideLen);

	//Output.g_DetailNoiseScale = ip[0].g_DetailNoiseScale;
	//Output.g_DetailUVScale = ip[0].g_DetailUVScale;
	/*Output.g_FractalOctaves = ip[0].g_FractalOctaves; */
	//Output.g_TextureWorldOffset = ip[0].g_TextureWorldOffset;
	
	if (!inFrustum(centre, g_EyePos / WORLD_SCALE, g_ViewDir, diagLen))
	{
		Output.Inside[0] = Output.Inside[1] = -1;
		Output.Edges[0]  = Output.Edges[1]  = Output.Edges[2] = Output.Edges[3] = -1;
	}
	else
	{
		// Alternative implementation left here for instruction purposes:
		// Examining displaced patch corners completely fails for patches that are seen edge-on where the patch has
		// significant interior displacement.  In screenspace, two patch dimensions are ~= 0.  However, once displaced
		// in the DS the screenspace size is significantly > 0.  I think this edge-based version simply doesn't work
		// well unless you have a good estimate of the max interior displacement (needs a max displacement per patch).
		//Output.Edges[0] = EdgeToScreenSpaceTessellation(ip[0].vPosition, ip[1].vPosition);
		//Output.Edges[3] = EdgeToScreenSpaceTessellation(ip[1].vPosition, ip[2].vPosition);
		//Output.Edges[2] = EdgeToScreenSpaceTessellation(ip[2].vPosition, ip[3].vPosition);
		//Output.Edges[1] = EdgeToScreenSpaceTessellation(ip[3].vPosition, ip[0].vPosition);

		// Alternatively: project a sphere centered on the patch edge mid-point (not patch bounding sphere - that
		// wouldn't work with adjacency fixes, etc).  This is independent of patch orientation and doesn't suffer 
		// from edges getting projected to zero size.  The flip side is that it over-tessellates flat, boring areas 
		// and uses more polygons overall.
		Output.Edges[0] = SphereToScreenSpaceTessellation(ip[0].vPosition, ip[1].vPosition, sideLen);
		Output.Edges[3] = SphereToScreenSpaceTessellation(ip[1].vPosition, ip[2].vPosition, sideLen);
		Output.Edges[2] = SphereToScreenSpaceTessellation(ip[2].vPosition, ip[3].vPosition, sideLen);
		Output.Edges[1] = SphereToScreenSpaceTessellation(ip[3].vPosition, ip[0].vPosition, sideLen);

		// Edges that need adjacency adjustment are identified by the per-instance ip[0].adjacency 
		// scalars, in *conjunction* with a patch ID that puts them on the edge of a tile.
		int2 patchXY;
		patchXY.y = PatchID / PATCHES_PER_TILE_EDGE;
		patchXY.x = PatchID - patchXY.y * PATCHES_PER_TILE_EDGE;

		// Identify patch edges that are adjacent to a patch of a different size.  The size difference
		// is encoded in ip[n].adjacency, either 0.5, 1.0 or 2.0.
		// neighbourMinusX refers to our adjacent neighbour in the direction of -ve x.  The value 
		// is the neighbour's size relative to ours.  Similarly for plus and Y, etc.  You really
		// need a diagram to make sense of the adjacency conditions in the if statements. :-(
		// These four ifs deal with neighbours that are smaller.
		if (ip[0].adjacency.neighbourMinusX < 0.55 && patchXY.x == 0)
			Output.Edges[0] = SmallerNeighbourAdjacencyFix(ip[0].vPosition, ip[1].vPosition, sideLen);
		if (ip[0].adjacency.neighbourMinusY < 0.55 && patchXY.y == 0)
			Output.Edges[1] = SmallerNeighbourAdjacencyFix(ip[3].vPosition, ip[0].vPosition, sideLen);
		if (ip[0].adjacency.neighbourPlusX < 0.55 && patchXY.x == PATCHES_PER_TILE_EDGE-1)
			Output.Edges[2] = SmallerNeighbourAdjacencyFix(ip[2].vPosition, ip[3].vPosition, sideLen);
		if (ip[0].adjacency.neighbourPlusY < 0.55 && patchXY.y == PATCHES_PER_TILE_EDGE-1)
			Output.Edges[3] = SmallerNeighbourAdjacencyFix(ip[1].vPosition, ip[2].vPosition, sideLen);

		// Deal with neighbours that are larger than us.
		if (ip[0].adjacency.neighbourMinusX > 1 && patchXY.x == 0)
			Output.Edges[0] = LargerNeighbourAdjacencyFix(ip[0].vPosition, ip[1].vPosition, patchXY.y, sideLen);
		if (ip[0].adjacency.neighbourMinusY > 1 && patchXY.y == 0)
			Output.Edges[1] = LargerNeighbourAdjacencyFix(ip[0].vPosition, ip[3].vPosition, patchXY.x, sideLen);	// NB: irregular index pattern - it's correct.
		if (ip[0].adjacency.neighbourPlusX > 1 && patchXY.x == PATCHES_PER_TILE_EDGE-1)
			Output.Edges[2] = LargerNeighbourAdjacencyFix(ip[3].vPosition, ip[2].vPosition, patchXY.y, sideLen);
		if (ip[0].adjacency.neighbourPlusY > 1 && patchXY.y == PATCHES_PER_TILE_EDGE-1)
			Output.Edges[3] = LargerNeighbourAdjacencyFix(ip[1].vPosition, ip[2].vPosition, patchXY.x, sideLen);	// NB: irregular index pattern - it's correct.

		// Use average of edge points for interior - visually looks OK.  
		// fxc bug if we assign different complex expressions to Inside[0] and [1].
		Output.Inside[1] = 0.5 * (Output.Edges[0] + Output.Edges[2]);
		Output.Inside[0] = 0.5 * (Output.Edges[1] + Output.Edges[3]);
	    
		Output.vWorldXZ[0] = ip[0].vWorldXZ;
		Output.vWorldXZ[1] = ip[1].vWorldXZ;
		Output.vWorldXZ[2] = ip[2].vWorldXZ;
		Output.vWorldXZ[3] = ip[3].vWorldXZ;

		Output.debugColour[0] = DEBUG_COLOURS[clamp(log2(Output.Edges[0]),  0,5)];
		Output.debugColour[1] = DEBUG_COLOURS[clamp(log2(Output.Edges[1]),  0,5)];
		Output.debugColour[2] = DEBUG_COLOURS[clamp(log2(Output.Edges[2]),  0,5)];
		Output.debugColour[3] = DEBUG_COLOURS[clamp(log2(Output.Edges[3]),  0,5)];
		Output.debugColour[4] = DEBUG_COLOURS[clamp(log2(Output.Inside[0]), 0,5)];
	}

    return Output;
}

// The hull shader is called once per output control point, which is specified with
// outputcontrolpoints.

// The input to the hull shader comes from the vertex shader

// The output from the hull shader will go to the domain shader.
// The tessellation factor, topology, and partition mode will go to the fixed function
// tessellator stage to calculate the UVW and domain points.
[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("TerrainScreenspaceLODConstantsHS")]
HS_OUTPUT TerrainScreenspaceLODHS(InputPatch<VS_CONTROL_POINT_OUTPUT, 4> p, uint i : SV_OutputControlPointID)
{
	// The VS displaces y for LOD calculations.  We drop it here so as not to displace twice in the DS.
    HS_OUTPUT Output;
    Output.vPosition = float3(p[i].vPosition.x, 0, p[i].vPosition.z);
    return Output;
}

float3 bilerpColour(float3 c0, float3 c1, float3 c2, float3 c3, float2 UV)
{
    float3 left  = lerp(c0, c1, UV.y);
    float3 right = lerp(c2, c3, UV.y);
    float3 result = lerp(left, right, UV.x);
	return result;
}

float3 lerpDebugColours(float3 cIn[5], float2 uv)
{
	if (uv.x < 0.5 && uv.y < 0.5)
		return bilerpColour(0.5* (cIn[0] + cIn[1]), cIn[0], cIn[1], cIn[4], 2 * uv);
	else if (uv.x < 0.5 && uv.y >= 0.5)
		return bilerpColour(cIn[0], 0.5* (cIn[0] + cIn[3]), cIn[4], cIn[3], 2 * (uv-float2(0,0.5)));
	else if (uv.x >= 0.5 && uv.y < 0.5)
		return bilerpColour(cIn[1], cIn[4], 0.5* (cIn[2] + cIn[1]), cIn[2], 2 * (uv-float2(0.5,0)));
	else // x >= 0.5 && y >= 0.5
		return bilerpColour(cIn[4], cIn[3], cIn[2], 0.5* (cIn[2] + cIn[3]), 2 * (uv-float2(0.5,0.5)));
}

// Templates please!!!
float2 Bilerp(float2 v0, float2 v1, float2 v2, float2 v3, float2 i)
{
    float2 bottom = lerp(v0, v3, i.x);
    float2 top    = lerp(v1, v2, i.x);
    float2 result = lerp(bottom, top, i.y);
	return result;
}

float3 Bilerp(float3 v0, float3 v1, float3 v2, float3 v3, float2 i)
{
    float3 bottom = lerp(v0, v3, i.x);
    float3 top    = lerp(v1, v2, i.x);
    float3 result = lerp(bottom, top, i.y);
	return result;
}

float3 TessellatedWorldPos(HS_CONSTANT_DATA_OUTPUT input, 
                    float2 UV : SV_DomainLocation,
                    const OutputPatch<HS_OUTPUT, 4> terrainQuad,
					float g_DetailNoiseScale,
					float2 g_DetailUVScale,
					float g_fDisplacementHeight
					/*, int3 g_FractalOctaves*/
					, float3 g_TextureWorldOffset
					)
{
    // bilerp the position
    float3 worldPos = Bilerp(terrainQuad[0].vPosition, terrainQuad[1].vPosition, terrainQuad[2].vPosition, terrainQuad[3].vPosition, UV);
    
	const int mipLevel   = 0;

	float height = SampleHeightForVS(g_CoarseHeightMap, SamplerClampLinear, worldPos.xz, g_DetailNoiseScale, g_DetailUVScale/*, g_FractalOctaves*/, g_TextureWorldOffset);
    worldPos.y += g_fDisplacementHeight * height;

	return worldPos;
}

// The domain shader is run once per vertex and calculates the final vertex's position
// and attributes.  It receives the UVW from the fixed function tessellator and the
// control point outputs from the hull shader.  Since we are using the DirectX 11
// Tessellation pipeline, it is the domain shader's responsibility to calculate the
// final SV_POSITION for each vertex.

// The input SV_DomainLocation to the domain shader comes from fixed function
// tessellator.  And the OutputPatch comes from the hull shader.  From these, you
// must calculate the final vertex position, color, texcoords, and other attributes.

// The output from the domain shader will be a vertex that will go to the video card's
// rasterization pipeline and get drawn to the screen.
[domain("quad")]
MeshVertex TerrainDisplaceDS( HS_CONSTANT_DATA_OUTPUT input, 
                    float2 UV : SV_DomainLocation,
                    const OutputPatch<HS_OUTPUT, 4> terrainQuad,
					uint PatchID : SV_PrimitiveID,
					uniform int g_DebugShowPatches,
					uniform float g_fDisplacementHeight)
{
    MeshVertex Output = (MeshVertex) 0;

	const float3 worldPos = TessellatedWorldPos(input, UV, terrainQuad, g_DetailNoiseScale, g_DetailUVScale, g_fDisplacementHeight/*, g_FractalOctaves*/, g_TextureWorldOffset);
    Output.vPosition = mul(float4(worldPos.xyz,1), g_WorldViewProj);
	Output.debugColour = lerpDebugColours(input.debugColour, UV);
	Output.vWorldXZ = worldPos.xz;
	Output.vNormal = float3(1,1,1);
	//Output.g_DebugShowPatches = g_DebugShowPatches;
	//Output.g_DetailNoiseScale = input.g_DetailNoiseScale;
	//Output.g_fDisplacementHeight = g_fDisplacementHeight;
	//Output.g_DetailUVScale = input.g_DetailUVScale;
	//Output.g_TextureWorldOffset = input.g_TextureWorldOffset;
	
	// For debugging, darken a chequer board pattern of tiles to highlight tile boundaries.
	if (g_DebugShowPatches)
	{
		const uint patchY = PatchID / PATCHES_PER_TILE_EDGE;
		const uint patchX = PatchID - patchY * PATCHES_PER_TILE_EDGE;
		Output.vNormal *= (0.5 * ((patchX + patchY) % 2) + 0.5);
	}

	return Output;    
}


//--------------------------------------------------------------------------------------
// Smooth shading pixel shader section
//--------------------------------------------------------------------------------------
Texture2D g_TerrainColourTexture1;
Texture2D g_TerrainColourTexture2;

float HighContrast(float i)
{
	//return 1.3 * (i-0.1);
	//return 1.9 * (i-0.2); // higher
	return i;
}

float3 SampleDetailNormal(float2 worldXZ, float g_DetailNoiseScale, float2 g_DetailUVScale, float g_CoarseSampleSpacing)
{
	// The detail displacement is scaled by the coarse displacement's height.  So we need 
	// to fetch the height to scale the normal.  (The scale computation could perhaps be
	// moved up to the DS.)
	const float2 uv = worldXZtoHeightUV(worldXZ);
	float coarse = g_CoarseHeightMap.Sample(SamplerClampLinear, uv).r;

	const float vScale = saturate(coarse) * WORLD_SCALE * VERTICAL_SCALE;

	// The MIP-mapping doesn't seem to work very well.  Maybe I need to think more carefully about
	// anti-aliasing the normal function?
	float2 grad = SampleDetailGradOctaves(uv, g_DetailNoiseScale, g_DetailUVScale,g_TextureWorldOffset);
	return normalize(float3(-vScale * grad.x, g_CoarseSampleSpacing * WORLD_UV_REPEATS_RECIP * g_DetailUVScale.y, vScale * grad.y));
}

float DebugCracksPattern(MeshVertex input)
{
	// Dark grey and black - to better show any cracks.
	if (g_DebugShowPatches)
		return 0.1 * (input.vNormal.x - 0.5);
	else 
		return 0;
}

float4 SmoothShadePS(MeshVertex input,
					 uniform float g_CoarseSampleSpacing) : SV_Target
{
	//return DebugCracksPattern(input);

	// Not sure about the arbitrary 2x.  It looks right visually.  Maybe there's a constant somewhere in
	// the texture sizes that I overlooked?!?
	const float ARBITRARY_FUDGE = 2;
	const float2 grad = g_CoarseGradientMap.Sample(SamplerRepeatLinear, worldXZtoHeightUV(input.vWorldXZ)).rg;
	const float vScale = ARBITRARY_FUDGE * g_fDisplacementHeight * WORLD_SCALE * VERTICAL_SCALE;
	const float3 coarseNormal = normalize(float3(vScale * grad.x, g_CoarseSampleSpacing, -vScale * grad.y));
	const float3 detailNormal = SampleDetailNormal(input.vWorldXZ, g_DetailNoiseScale,g_DetailUVScale,g_CoarseSampleSpacing);
	const float3 normal = normalize(coarseNormal + detailNormal);

	// Texture coords have to be offset by the eye's 2D world position.  Why the 2x???
	const float2 texUV = input.vWorldXZ + 2 * float2(g_TextureWorldOffset.x, -g_TextureWorldOffset.z);;

	// We apply two textures at vastly different scales: macro and micro detail.
	float3 macroDetail = g_TerrainColourTexture1.Sample(SamplerRepeatMaxAniso, texUV).xyz;				// we know that this is grey only
	float4 microDetail = g_TerrainColourTexture2.Sample(SamplerRepeatMedAniso, texUV * 50);
	float  gaussian = g_NoiseTexture.Sample(SamplerRepeatPoint, texUV * 50.0 / 256.0).x;
	float  uniformRandom = frac(gaussian * 10);
	float  randomizedDetail = 1;

	// Randomly choose between four versions of the micro texture in RGBA.  This sort of 
	// blending is representative of game-engine terrain shaders (and the perf).
	if (uniformRandom >= 0.75)
		randomizedDetail = microDetail.x;
	else if (uniformRandom >= 0.5)
		randomizedDetail = microDetail.y;
	else if (uniformRandom >= 0.25)
		randomizedDetail = microDetail.z;
	else
		randomizedDetail = microDetail.w;

	// Some of the lunar reference photos have a brownish tint.  IMO, it looks slightly better here.
	const float3 colour = float3(0.988, 0.925, 0.847);

	// This light direction approximately matches the pre-baked direction in the NASA photos.
	const float3 lightDir = normalize(float3(-0.74,0.45,-0.15));
	float lit = saturate(HighContrast(dot(lightDir, normal)));

	// Chequer pattern still comes down from the DS.
	if (g_DebugShowPatches)
		lit *= input.vNormal.x;

	//return lit;
	return float4(colour * macroDetail * randomizedDetail * lit, 1);
}