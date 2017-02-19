#include <metal_stdlib>
using namespace metal;

//struct PS_INPUT
//{
//	float2 uv0;
//};

fragment float4 main_metal
(
	//PS_INPUT inPs								[[stage_in]],
	float4 gl_FragCoord							[[position]],

	texture2d<float, access::read>	tex			[[texture(0)]]
)
{
	float4 finalColour = tex.read( uint2( gl_FragCoord.xy ), 0 ).xyzw;

	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2( -1,  1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  1,  1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2( -1, -1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  1, -1 ), 0 ).xyzw;

	finalColour /= 5.0;

	/*finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2( -1,  1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  0,  1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  1,  1 ), 0 ).xyzw;

	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2( -1, -1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  0, -1 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  1, -1 ), 0 ).xyzw;

	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2( -1,  0 ), 0 ).xyzw;
	finalColour += tex.read( uint2( gl_FragCoord.xy ) + uint2(  1,  0 ), 0 ).xyzw;

	finalColour /= 9.0;*/

	return finalColour;
}
