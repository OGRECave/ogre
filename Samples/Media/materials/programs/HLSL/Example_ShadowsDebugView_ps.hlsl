
void main_ps( out float4 outColour : COLOR0,
                in float2 uv0 : TEXCOORD0,
                uniform sampler2D diffuseMap : register(s0) )
{
    outColour = float4( tex2D( diffuseMap, uv0 ).xxx, 1.0f );
}
