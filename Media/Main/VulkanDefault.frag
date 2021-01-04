#version 430

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, uv.xy);
}