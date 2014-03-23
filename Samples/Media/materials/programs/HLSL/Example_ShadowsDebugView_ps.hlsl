
void main_ps(	in float depth : TEXCOORD0,
				out float4 outColour : COLOR0 )
{
	outColour = float4( depth.xxx, 1 );
}