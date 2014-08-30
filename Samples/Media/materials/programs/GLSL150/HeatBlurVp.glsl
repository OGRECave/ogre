#version 150

// in vec4 vertex;
// in vec2 uv0;
out vec2 uv;

uniform vec4 position;
uniform float flipping;
// uniform mat4 worldViewProj;

void main()
{
    vec4 inPos = position;
    gl_Position = vec4(inPos.x, flipping * inPos.y, 0.0, 1.0);
    inPos.xy = sign(inPos.xy);
    uv  = (vec2(inPos.x, -inPos.y) + 1.0) / 2.0;

    // gl_Position = worldViewProj * vertex;
    // uv  = uv0;
}
