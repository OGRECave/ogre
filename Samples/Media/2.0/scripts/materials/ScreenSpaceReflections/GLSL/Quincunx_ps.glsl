#version 330

uniform sampler2D tex;

in block
{
	vec2 uv0;
} inPs;

out vec4 fragColour;

in vec4 gl_FragCoord;

void main()
{
	vec4 finalColour = texelFetch( tex, ivec2( gl_FragCoord.xy ), 0 ).xyzw;

	finalColour += texelFetch( tex, ivec2( gl_FragCoord.xy ) + ivec2( -1,  1 ), 0 ).xyzw;
	finalColour += texelFetch( tex, ivec2( gl_FragCoord.xy ) + ivec2(  1,  1 ), 0 ).xyzw;
	finalColour += texelFetch( tex, ivec2( gl_FragCoord.xy ) + ivec2( -1, -1 ), 0 ).xyzw;
	finalColour += texelFetch( tex, ivec2( gl_FragCoord.xy ) + ivec2(  1, -1 ), 0 ).xyzw;

	finalColour /= 5.0;

	fragColour = finalColour;
}
