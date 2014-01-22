float4 main (float4 pos			: SV_POSITION, 
			 float2 oUv			: TEXCOORD0,
			 float4 colour		: COLOR) : SV_Target
{
	return colour;
}

float4 mainCg (float4 pos : SV_POSITION, float4 colour0 : COLOR) : SV_Target
{
    return colour0;
}

float4 mainForAmbientOneTexture (float4 pos : SV_POSITION, float2 oUv : TEXCOORD0, float4 colour0 : COLOR, uniform float4 colour) : SV_Target
{
	return colour;
}