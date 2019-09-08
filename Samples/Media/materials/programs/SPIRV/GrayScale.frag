#version 430

layout(location = 0) uniform sampler2D RT;
layout(location = 0) in vec2 oUv0;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 greyscale = vec3(dot(texture(RT, oUv0).rgb, vec3(0.3, 0.59, 0.11)));
	fragColor = vec4(greyscale, 1.0);
}
