
Texture2D<float4>	tex : register(t0);

struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

float4 main
(
	PS_INPUT inPs,
	float4 gl_FragCoord : SV_Position
) : SV_Target
{
	float4 finalColour = tex.Load( int3( int2( gl_FragCoord.xy ), 0 ) ).xyzw;

	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2( -1,  1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  1,  1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2( -1, -1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  1, -1 ), 0 ) ).xyzw;

	finalColour /= 5.0;

	/*finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2( -1,  1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  0,  1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  1,  1 ), 0 ) ).xyzw;

	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2( -1, -1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  0, -1 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  1, -1 ), 0 ) ).xyzw;

	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2( -1,  0 ), 0 ) ).xyzw;
	finalColour += tex.Load( int3( int2( gl_FragCoord.xy ) + int2(  1,  0 ), 0 ) ).xyzw;

	finalColour /= 9.0;*/

	return finalColour;
}
