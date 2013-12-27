float4 copy_4xFP16_ps( uniform sampler2D tex : register(s0), float2 uv : TEXCOORD0 ) : COLOR0
{
	return tex2D( tex, uv ).xyzw;
}