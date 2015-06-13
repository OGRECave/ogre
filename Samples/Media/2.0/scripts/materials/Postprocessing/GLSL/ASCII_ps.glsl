#version 330

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform sampler2D RT;
uniform sampler3D chars;

uniform vec2 numTiles;
uniform vec2 iNumTiles;
uniform vec2 iNumTiles2;
uniform vec4 lum;
uniform float charBias;

void main()
{
    vec3 local;

	//sample RT
	local.xy = mod(inPs.uv0, iNumTiles);
	vec2 middle = inPs.uv0 - local.xy;
	local.xy = local.xy * numTiles;
	
	//iNumTiles2 = iNumTiles / 2
	middle = middle + iNumTiles2;
	vec4 c = texture(RT, middle);
	
	//multiply luminance by charbias , beacause not all slices of the ascii
	//volume texture are used
	local.z = dot(c , lum)*charBias;
	
	//fix to brighten the dark pixels with small characters
	//c *= lerp(2.0,1.0, local.z);
	
	c *= texture(chars, local);
	fragColour = c;
}
