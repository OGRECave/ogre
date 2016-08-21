#version 330

uniform sampler2D depthTexture;

vec4 gl_FragCoord;
out float gl_FragDepth;

void main()
{
	float fDepth0 = texelFetch( depthTexture, ivec2(gl_FragCoord.xy * 2.0), 0 ).x;
	float fDepth1 = texelFetch( depthTexture, ivec2(gl_FragCoord.xy * 2.0) + ivec2( 0, 1 ), 0 ).x;
	float fDepth2 = texelFetch( depthTexture, ivec2(gl_FragCoord.xy * 2.0) + ivec2( 1, 0 ), 0 ).x;
	float fDepth3 = texelFetch( depthTexture, ivec2(gl_FragCoord.xy * 2.0) + ivec2( 1, 1 ), 0 ).x;

	//gl_FragDepth = texelFetch( depthTexture, ivec2(gl_FragCoord.xy * 2.0), 0 ).x;
	gl_FragDepth = max( max( fDepth0, fDepth1 ), max( fDepth2, fDepth3 ) );
}
