#version 330

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform sampler2D RT;
uniform sampler3D noiseVol;

uniform vec4 lum;
uniform float time;

void main()
{
	vec4 oC;
	oC = texture(RT, inPs.uv0);
	
	//obtain luminence value
	oC = vec4(dot(oC,lum));
	
	//add some random noise
	oC += 0.2 *(texture(noiseVol, vec3(inPs.uv0*5,time)))- 0.05;
	
	//add lens circle effect
	//(could be optimised by using texture)
	float dist = distance(inPs.uv0, vec2(0.5,0.5));
	oC *= smoothstep(0.5,0.45,dist);
	
	//add rb to the brightest pixels
	oC.rb = vec2(max(oC.r - 0.75, 0)*4);
	
	fragColour = oC;
}
