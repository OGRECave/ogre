#version 150

uniform sampler2D RT;
in vec2 oUv0;
out vec4 fragColour;

void main()
{
    float nColors = 8.0;
    float gamma = 0.6;

    vec4 texCol = vec4(texture(RT, oUv0));
    vec3 tc = texCol.xyz;
    tc = pow(tc, vec3(gamma));
    tc = tc * nColors;
    tc = floor(tc);
    tc = tc / nColors;
    tc = pow(tc, vec3(1.0 / gamma));
    fragColour = vec4(tc, texCol.w);
}
