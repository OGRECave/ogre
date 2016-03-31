#version 430

layout(std140) uniform;

layout (rgb10_a2) uniform restrict writeonly image2D shadowMap;
uniform sampler2D heightMap;

layout( local_size_x = @value( threads_per_group_x ),
        local_size_y = @value( threads_per_group_y ),
        local_size_z = @value( threads_per_group_z ) ) in;

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

//Bresenham algorithm uniforms
uniform vec2 delta;

uniform ivec2 xyStep; //(y0 < y1) ? 1 : -1;
uniform int isSteep;

layout(binding = 0) uniform StartsBuffer
{
	ivec4 startXY[4096];
};

struct PerGroupData
{
	int iterations;
	float deltaErrorStart;
	float padding0;
	float padding1;
};

layout(binding = 1) uniform PerGroupDataBuffer
{
	PerGroupData perGroupData[4096];
};

//Rendering uniforms
uniform float heightDelta;

vec2 calcShadow( ivec2 xyPos, vec2 prevHeight )
{
	prevHeight.x -= heightDelta;
	prevHeight.y = prevHeight.y * 0.9 - heightDelta; //Used for the penumbra region

	float currHeight = texelFetch( heightMap, xyPos, 0 ).x;

	float shadowValue = smoothstep( prevHeight.y, prevHeight.x, currHeight );
	//float shadowValue = currHeight < prevHeight.x ? 0.0 : 1.0;
	prevHeight.x = currHeight >= prevHeight.x ? currHeight : prevHeight.x;
	prevHeight.y = currHeight >= prevHeight.x ? currHeight : prevHeight.y;

	//We store shadow's height in 10 bits, but the actual heightmap is in 16 bits.
	//If we have a height of 0.9775, it will translate to 1000.96 rounding to 1001
	//Thus when sampling, the terrain will be self shadowed by itself.
	//We need to floor, not round.
	vec2 flooredHeight = floor( prevHeight.xy * 1024.0 ) * 0.0009765625;

	imageStore( shadowMap, xyPos, vec4( shadowValue, flooredHeight.xy, 1.0 ) );

	return prevHeight;
}

void main()
{
	vec2 prevHeight = vec2( 0.0, 0.0 );
	float error = delta.x * 0.5 + perGroupData[gl_WorkGroupID.x].deltaErrorStart;

	int x, y;
	if( gl_GlobalInvocationID.x < 4096u )
	{
		x = startXY[gl_GlobalInvocationID.x].x;
		y = startXY[gl_GlobalInvocationID.x].y;
	}
	else
	{
		x = startXY[gl_GlobalInvocationID.x - 4096u].z;
		y = startXY[gl_GlobalInvocationID.x - 4096u].w;
	}

	int numIterations = perGroupData[gl_WorkGroupID.x].iterations;
	for( int i=0; i<numIterations; ++i )
	{
		if( isSteep != 0 )
			prevHeight = calcShadow( ivec2( y, x ), prevHeight );
		else
			prevHeight = calcShadow( ivec2( x, y ), prevHeight );

		error -= delta.y;
		if( error < 0 )
		{
			y += xyStep.y;
			error += delta.x;
		}

		x += xyStep.x;
	}
}

	// Bresenham's line algorithm (http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B)
//	const bool steep = (fabs(y1 - y0) > fabs(x1 - x0));
//	if(steep)
//	{
//	  std::swap(x0, y0);
//	  std::swap(x1, y1);
//	}

//	if(x0 > x1)
//	{
//	  std::swap(x0, x1);
//	  std::swap(y0, y1);
//	}

//	const float dx = x1 - x0;
//	const float dy = fabs(y1 - y0);

//	float error = dx / 2.0f;
//	const int ystep = (y0 < y1) ? 1 : -1;
//	int y = (int)y0;

//	const int maxX = (int)x1;

//	for(int x=(int)x0; x<maxX; x++)
//	{
//	  if(steep)
//	  {
//		  SetPixel(y,x, color);
//	  }
//	  else
//	  {
//		  SetPixel(x,y, color);
//	  }

//	  error -= dy;
//	  if(error < 0)
//	  {
//		  y += ystep;
//		  error += dx;
//	  }
//	}
