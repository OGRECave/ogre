#version 150

uniform mat4 worldViewProj;
in vec4 vertex;
in vec2 uv0;
out vec2 oUv0;
out vec2 oUv1;
out vec2 oUv2;
out vec2 oUv3;
out vec4 pos;

void main()
{
	// Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
    gl_Position = worldViewProj * vertex;

    oUv0 = uv0;
    oUv1 = oUv0;
    oUv2 = oUv0;
    oUv3 = oUv0;
    pos = gl_Position;
}
