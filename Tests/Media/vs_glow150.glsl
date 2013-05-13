#version 150

uniform float size_value;
uniform float time;
uniform mat4 worldViewProjMatrix;

in vec4 vertex;
in vec3 normal;

void main(void)
{
    vec3 Pos = vertex.xyz + (size_value * (1.0 + (sin(time * 5.0) + 1.0) / 15.0 ) * normal );

    gl_Position = worldViewProjMatrix * vec4(Pos,1.0);
}
