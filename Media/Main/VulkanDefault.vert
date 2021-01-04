#version 430

#include <OgreUnifiedShader.h>

//layout(row_major) uniform;
layout(binding = 0) uniform OgreUniforms
{
    mat4 modelViewProj;
    mat4 texMtx;
};

IN(vec4 position, POSITION)
IN(vec3 normal, NORMAL)
IN(vec3 uv0, TEXCOORD0)

OUT(vec3 uv, 0)

void main() {
    gl_Position = modelViewProj*position;
    uv = (texMtx*vec4(uv0, 1)).xyz;
}