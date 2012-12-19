#version 150

out vec4 fragColour;

uniform sampler2D RT;
uniform sampler3D noiseVol;

in vec2 oUv0;
uniform vec4 lum;
uniform float time;

void main()
{
	vec4 oC;
	oC = texture(RT, oUv0);
	
	//obtain luminence value
	oC = vec4(dot(oC,lum));
	
	//add some random noise
	oC += 0.2 *(texture(noiseVol, vec3(oUv0*5,time)))- 0.05;
	
	//add lens circle effect
	//(could be optimised by using texture)
	float dist = distance(oUv0, vec2(0.5,0.5));
	oC *= smoothstep(0.5,0.45,dist);
	
	//add rb to the brightest pixels
	oC.rb = vec2(max(oC.r - 0.75, 0)*4);
	
	fragColour = oC;
}
