struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
    
    
};

Texture2D<float4> sceneTexture	: register(t0);
Texture2D<float4> distortionTexture	: register(t1);

SamplerState samplerState0		: register(s0);

uniform float u_DistortionStrenght;

float4 main
(
	PS_INPUT inPs
) : SV_Target
{
	//Sample vector from distortion pass
	float4 dVec = distortionTexture.Sample(samplerState0, inPs.uv0);
    
	// Distortion texture is in range [0-1] so we will make it to range [-1.0 - 1.0] so we will get left-right vector and up-down vector
    dVec.xy = (dVec.xy - 0.5)*2.0;
    
	//Calculate base coordinate offset using texture color values, alpha and strenght
    float2 offset = dVec.xy * dVec.w * u_DistortionStrenght;
    
	//We will add more realism by offsetting each color channel separetly for different amount.
	//That is because violet/blue light has shorter wave lenght than red light so they refract differently.
	
    float2 ofR = offset*0.80; //Red channel refracts 80% of offset
    float2 ofG = offset*0.90; //Green channel refracts 90% of offset
    float2 ofB = offset; //Blue channel refracts 100% of offset
    
	//We have to sample three times to get each color channel.
	// Distortion is made by offsetting UV coordinates with each channel offset value.
    float cR = sceneTexture.Sample(samplerState0, inPs.uv0 + ofR).r;
    float cG = sceneTexture.Sample(samplerState0, inPs.uv0 + ofG).g;
    float cB = sceneTexture.Sample(samplerState0, inPs.uv0 + ofB).b;
    
	// Combine colors to result
    float4 color = float4(cR, cG, cB, 1.0);
    
	return color;
}
