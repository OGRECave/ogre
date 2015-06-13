#version 330

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform vec2 numTiles;
uniform vec2 iNumTiles;
uniform vec2 iNumTiles2;
uniform vec4 lum;
uniform sampler2D RT;
uniform sampler3D noise;

void main()
{
	vec3 local;
	local.xy = mod(inPs.uv0, iNumTiles);
	vec2 middle = inPs.uv0 - local.xy;
	local.xy = local.xy * numTiles;
	middle +=  iNumTiles2;
	local.z = dot(texture(RT, middle), lum);
	vec4 c = vec4(texture(noise,local).r);
	fragColour = c;
}
