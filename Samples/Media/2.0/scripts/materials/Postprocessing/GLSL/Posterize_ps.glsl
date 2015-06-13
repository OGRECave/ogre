#version 330

uniform sampler2D RT;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
    float nColors = 8.0;
    float gamma = 0.6;

	vec4 texCol = texture(RT, inPs.uv0);
    vec3 tc = texCol.xyz;
    tc = pow(tc, vec3(gamma));
    tc = tc * nColors;
    tc = floor(tc);
    tc = tc / nColors;
    tc = pow(tc, vec3(1.0 / gamma));
    fragColour = vec4(tc, texCol.w);
}
