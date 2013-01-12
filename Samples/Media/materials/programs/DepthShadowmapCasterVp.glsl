uniform mat4 worldViewProj;
uniform vec4 texelOffsets;
uniform vec4 depthRange;

varying vec2 depth;

void main()
{
	gl_Position = ftransform();

	// fix pixel / texel alignment
	gl_Position.xy += texelOffsets.zw * gl_Position.w;
	// linear depth storage
	// offset / scale range output
#if LINEAR_RANGE
	depth.x = (gl_Position.z - depthRange.x) * depthRange.w;
#else
	depth.x = gl_Position.z;
#endif
	depth.y = gl_Position.w;


}

