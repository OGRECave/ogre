#version 330

uniform sampler2D sceneTexture;
uniform sampler2D distortionTexture;



in block
{
	vec2 uv0;
} inPs;


uniform float u_DistortionStrenght;

out vec4 fragColour;

void main()
{
	//Sample vector from distortion pass
	vec4 dVec = texture( distortionTexture, inPs.uv0 );
    
	// Distortion texture is in range [0-1] so we will make it to range [-1.0 - 1.0] so we will get left-right vector and up-down vector
    dVec.xy = (dVec.xy - 0.5)*2.0;
    
	//Calculate base coordinate offset using texture color values, alpha and strenght
    vec2 offset = dVec.xy * dVec.w * u_DistortionStrenght;
    
	//We will add more realism by offsetting each color channel separetly for different amount.
	//That is because violet/blue light has shorter wave lenght than red light so they refract differently.
	
    vec2 ofR = offset*0.80; //Red channel refracts 80% of offset
    vec2 ofG = offset*0.90; //Green channel refracts 90% of offset
    vec2 ofB = offset; //Blue channel refracts 100% of offset
    
	//We have to sample three times to get each color channel.
	// Distortion is made by offsetting UV coordinates with each channel offset value.
    float cR = texture( sceneTexture, inPs.uv0 + ofR).r;
    float cG = texture( sceneTexture, inPs.uv0 + ofG).g;
    float cB = texture( sceneTexture, inPs.uv0 + ofB).b;
    
	// Combine colors to result
    fragColour = vec4(cR, cG, cB, 1.0);
}
