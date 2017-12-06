
RWTexture2D<unorm float4> shadowMap		: register(u0);
Texture2D<float> heightMap;

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

//Bresenham algorithm uniforms
uniform float2 delta;

uniform int2 xyStep; //(y0 < y1) ? 1 : -1;
uniform int isSteep;

cbuffer StartsBuffer : register(b0)
{
	int4 startXY[4096];
};

struct PerGroupData
{
	int iterations;
	float deltaErrorStart;
	float padding0;
	float padding1;
};


cbuffer PerGroupDataBuffer : register(b1)
{
	PerGroupData perGroupData[4096];
};

//Rendering uniforms
uniform float heightDelta;

float2 calcShadow( int2 xyPos, float2 prevHeight )
{
	prevHeight.x -= heightDelta;
	prevHeight.y = prevHeight.y * 0.985 - heightDelta; //Used for the penumbra region

	float currHeight = heightMap.Load( int3( xyPos, 0 ) );

	float shadowValue = smoothstep( prevHeight.y, prevHeight.x, currHeight + 0.001 );
	prevHeight.x = currHeight >= prevHeight.x ? currHeight : prevHeight.x;
	prevHeight.y = currHeight >= prevHeight.y ? currHeight : prevHeight.y;

	//We store shadow's height in 10 bits, but the actual heightmap is in 16 bits.
	//If we have a height of 0.9775, it will translate to 999.98 rounding to 1000
	//Thus when sampling, the objects on top of the terrain will be shadowed by the
	//terrain at that spot. Thus we subtract 1 from the height, and add 1 to
	//invHeightLength for a smooth gradient (which also prevents div. by zero).
	float2 roundedHeight = floor( saturate( prevHeight.xy ) * 1023.0 + 0.5 ) - 1.0;
	float invHeightLength = 1.0 / (roundedHeight.x - roundedHeight.y + 1); //+1 Avoids div. by zero
	roundedHeight.y *= 0.000977517;

	shadowMap[xyPos] = float4( shadowValue, roundedHeight.y, invHeightLength, 1.0 );
	
	return prevHeight;
}

[numthreads(@value( threads_per_group_x ), @value( threads_per_group_y ), @value( threads_per_group_z ))]
void main( uint3 gl_GlobalInvocationID : SV_DispatchThreadId, uint3 gl_WorkGroupID : SV_GroupID )
{
	float2 prevHeight = float2( 0.0, 0.0 );
	float error = delta.x * 0.5 + perGroupData[gl_WorkGroupID.x].deltaErrorStart;

	int x, y;
	if( gl_GlobalInvocationID.x < 4096u )
	{
		x = startXY[gl_GlobalInvocationID.x].x;
		y = startXY[gl_GlobalInvocationID.x].y;
	}
	else
	{
		//Due to alignment nightmares, instead of doing startXY[8192];
		//we perform startXY[4096] and store the values in .zw instead of .xy
		//It only gets used if the picture is very big. This branch is coherent as
		//long as 4096 is multiple of threads_per_group_x.
		x = startXY[gl_GlobalInvocationID.x - 4096u].z;
		y = startXY[gl_GlobalInvocationID.x - 4096u].w;
	}
	
	int numIterations = perGroupData[gl_WorkGroupID.x].iterations;
	for( int i=0; i<numIterations; ++i )
	{
		if( isSteep )
			prevHeight = calcShadow( int2( y, x ), prevHeight );
		else
			prevHeight = calcShadow( int2( x, y ), prevHeight );

		error -= delta.y;
		if( error < 0 )
		{
			y += xyStep.y;
			error += delta.x;
		}
		
		x += xyStep.x;
	}
}
