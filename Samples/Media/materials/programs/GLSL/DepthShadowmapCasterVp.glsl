uniform mat4 worldViewProj;
uniform vec4 texelOffsets;
varying vec2 depth;

void main()
{
	gl_Position = ftransform();
	// fix pixel / texel alignment
	gl_Position.xy += texelOffsets.zw * gl_Position.w;
	depth = gl_Position.zw;
}

