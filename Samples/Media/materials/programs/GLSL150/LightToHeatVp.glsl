#version 150

//TODO Use precalculated uv?
// in vec4 vertex;
// in vec2 uv0;
// out vec2 uv;

// uniform mat4 worldViewProj;

// void main()
// {
//     gl_Position = worldViewProj * vertex;
//     uv = uv0;
// }

in vec4 position;
uniform float flipping;
out vec2 uv;

void main()
{
    vec4 inPos = position;
    gl_Position = vec4(inPos.x, flipping * inPos.y, 0.0, 1.0);
    inPos.xy = sign(inPos.xy);
    uv = (vec2(inPos.x, -inPos.y) + 1.0) * 0.5;
}
