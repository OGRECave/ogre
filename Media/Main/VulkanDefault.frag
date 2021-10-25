#version 430

#define USE_OGRE_FROM_FUTURE
#include <OgreUnifiedShader.h>

SAMPLER2D(texSampler, 2);

layout(location = 0) in vec3 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, uv.xy);
}