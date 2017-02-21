#version 330

uniform sampler2DMS tex;

in block
{
	vec2 uv0;
} inPs;

out float fragColour;

in vec4 gl_FragCoord;

void main()
{
	fragColour = texelFetch( tex, ivec2( gl_FragCoord.xy ), 0 ).x;
}
