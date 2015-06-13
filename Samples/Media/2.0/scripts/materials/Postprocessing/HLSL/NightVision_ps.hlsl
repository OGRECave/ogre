Texture2D RT				: register(t0);
Texture3D noiseVol			: register(t1);
SamplerState samplerState[2]: register(s0);

float4 main
(
	float2 TexCoord : TEXCOORD0,
	uniform float4 lum,
	uniform float time
) : SV_Target
{	
	float4 oC;
	oC = RT.Sample( samplerState[0], TexCoord );
	
	//obtain luminence value
	oC = dot(oC,lum);
	
	//add some random noise
	oC += 0.2 *(noiseVol.Sample( samplerState[1], float3(TexCoord*5,time) ))- 0.05;
	
	//add lens circle effect
	//(could be optimised by using texture)
	float dist = distance(TexCoord, float2(0.5,0.5));
	oC *= smoothstep(0.5,0.45,dist);
	
	//add rb to the brightest pixels
	oC.rb = max (oC.r - 0.75, 0)*4;
	
	return oC ;
}
