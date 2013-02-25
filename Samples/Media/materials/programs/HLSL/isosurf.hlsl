cbuffer cBuffer0
{       
    float4x4 WorldViewProj;
    float4x4 origWorldViewIT;
	float4 Metaballs[2];
	float IsoValue = 1.0;
}

struct SampleData
{
	float4 Pos   : SV_POSITION;
	float3 N     : TEXCOORD0;
	float2 Field : TEXCOORD1;
};

struct SurfaceVertex
{
	float4 Pos	: SV_POSITION;
	float3 N	: TEXCOORD0;
};

static const int Num_Metaballs = 2;

// Size of the sampling grid
static const int3 SizeMask = { 63, 63, 63 };
static const int3 SizeShift = { 0, 6, 12 };

// Metaball function
// Returns metaball function value in .w and its gradient in .xyz

float4 Metaball(float3 Pos, float3 Center, float RadiusSq)
{
	float epsilon = 0.001;

	float3 Dist = Pos - Center;
	float InvDistSq = 1 / (dot(Dist, Dist) + epsilon);

	float4 o;
	o.xyz = -2 * RadiusSq * InvDistSq * InvDistSq * Dist;
	o.w = RadiusSq * InvDistSq;
	return o;
}

// Vertex shader
SampleData mainVS(float4 pos : POSITION)
{
	SampleData o;
	
	// Sum up contributions from all metaballs
	float4 Field = 0;
	for (int i = 0; i<2; i++) 
	{
		Field += Metaball(pos.xyz, Metaballs[i].xyz, Metaballs[i].w);
	}
	
	float3x3 WorldViewIT = { origWorldViewIT[0].xyz, origWorldViewIT[1].xyz, origWorldViewIT[2].xyz };
	
	// Transform position and normals
	pos.w = 1.0f;
	o.Pos = mul(WorldViewProj, pos);
	o.N = mul(WorldViewIT, Field.xyz);	// we want normals in world space
	o.Field.x = Field.w;
	
	// Generate in-out flags
	o.Field.y = (Field.w < IsoValue) ? 1 : 0;

//	o.Color = (Field*0.5+0.5) * (Field.w / 10.0);
	return o;
}

// Estimate where isosurface intersects grid edge with endpoints v0, v1
void CalcIntersection(float4 Pos0,
					  float3 N0,
					  float2 Field0,
					  float4 Pos1,
					  float3 N1,
					  float2 Field1,
					  inout TriangleStream<SurfaceVertex> OutputStream
					  )
{
	SurfaceVertex outputVertex;
	float t = (IsoValue - Field0.x) / (Field1.x - Field0.x);
	if ((Field0.x < IsoValue) && (Field1.x > Field0.x))
	{
		if (t > 0 && t < 1)
		{
			float4 Pos = lerp(Pos0, Pos1, t);
			float3 N = lerp(N0, N1, t);
			outputVertex.Pos = Pos;
			outputVertex.N = N;
			OutputStream.Append(outputVertex);
		}
	}
}
	
// Geometry shader
// input: line with adjacency (tetrahedron)
// outputs: zero, one or two triangles depending if isosurface intersects tetrahedron


[maxvertexcount(4)]
void mainGS(lineadj SampleData input[4], inout TriangleStream<SurfaceVertex> OutputStream)
{
	// construct index for this tetrahedron
	unsigned int index = (int(input[0].Field.y) << 3) |
				(int(input[1].Field.y) << 2) |
				(int(input[2].Field.y) << 1) |
				 int(input[3].Field.y);


	// don't bother if all vertices out or all vertices inside isosurface
	if (index > 0 && index < 15)
	{
		//Uber-compressed version of the edge table.
		unsigned int edgeListHex[] = 
			{0x0001cde0, 0x98b08c9d, 0x674046ce, 0x487bc480, 
			0x21301d2e, 0x139bd910, 0x26376e20, 0x3b700000};
		
		unsigned int edgeValFull = edgeListHex[index/2];
		unsigned int edgeVal = (index % 2 == 1) ? (edgeValFull & 0xFFFF) : ((edgeValFull >> 16) & 0xFFFF);
		int4 e0 = int4((edgeVal >> 14) & 0x3, (edgeVal >> 12) & 0x3, (edgeVal >> 10) & 0x3, (edgeVal >> 8) & 0x3);
		int4 e1 = int4((edgeVal >> 6) & 0x3, (edgeVal >> 4) & 0x3, (edgeVal >> 2) & 0x3, (edgeVal >> 0) & 0x3);
		
		CalcIntersection(input[e0.x].Pos, input[e0.x].N, input[e0.x].Field, input[e0.y].Pos, input[e0.y].N, input[e0.y].Field, OutputStream);
		CalcIntersection(input[e0.z].Pos, input[e0.z].N, input[e0.z].Field, input[e0.w].Pos, input[e0.w].N, input[e0.w].Field, OutputStream);
		CalcIntersection(input[e1.x].Pos, input[e1.x].N, input[e1.x].Field, input[e1.y].Pos, input[e1.y].N, input[e1.y].Field, OutputStream);

		// Emit additional triangle, if necessary
		if (e1.z != -1) {
			CalcIntersection(input[e1.z].Pos, input[e1.z].N, input[e1.z].Field, input[e1.w].Pos, input[e1.w].N, input[e1.w].Field, OutputStream);
		}
	}
}

// Pixel shader
float4 mainPS(SurfaceVertex surfaceVertex) : SV_TARGET
{
	//Sanitize input
	float3 N = normalize(surfaceVertex.N);
	float3 L = float3(0, 0, 1);
	//return float4(N*0.5+0.5, 1);
	float3 materials[2] = { float3(1, 1, 1), float3(0, 0, 0.5)};
    float nDotL = dot( N, L);
	float4 color = float4(abs(nDotL) * materials[nDotL < 0.0], 0.1);
	return color;
}