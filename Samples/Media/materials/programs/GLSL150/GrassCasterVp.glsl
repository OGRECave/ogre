#version 150

uniform mat4 worldViewProj;
uniform vec4 offset;
uniform vec4 texelOffsets;

in vec4 position;
in vec4 uv0;

out vec4 oUv0;
out vec2 oDepth;

//////////////////////// GRASS SHADOW CASTER
// Shadow caster vertex program.
void main()
{
	vec4 mypos = position;
	vec4 factor = vec4(1.0, 1.0, 1.0, 1.0) - uv0.yyyy;
	mypos = mypos + offset * factor;
	gl_Position = worldViewProj * mypos;

	// fix pixel / texel alignment
	gl_Position.xy += texelOffsets.zw * gl_Position.w;
    
	oDepth.x = gl_Position.z;
	oDepth.y = gl_Position.w;
  
  	oUv0 = uv0;
}
