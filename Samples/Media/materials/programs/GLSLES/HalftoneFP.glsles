#version 300 es

precision mediump int;
precision mediump float;

in vec4 pos;
in vec2 oUv0;

uniform vec2 numTiles;
uniform vec2 iNumTiles;
uniform vec2 iNumTiles2;
uniform vec4 lum;
uniform sampler2D RT;
uniform lowp sampler3D noise;

out vec4 fragColour;

void main()
{
	vec3 local;
	local.xy = mod(oUv0, iNumTiles);
	vec2 middle = oUv0 - local.xy;
	local.xy = local.xy * numTiles;
	middle +=  iNumTiles2;
	local.z = dot(texture(RT, middle), lum);
	vec4 c = vec4(texture(noise,local).r);
	fragColour = c;
}
