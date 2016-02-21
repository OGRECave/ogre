#version 150

in vec4 vertex;
// in vec2 uv0;
out vec2 texCoord[5];

// uniform mat4 worldViewProj;

void main()
{
    // gl_Position = worldViewProj * vertex;

    vec2 inPos = sign(vertex.xy);
    gl_Position = vec4(inPos.xy, 0.0, 1.0);

    vec2 uv0 = (vec2(inPos.x, -inPos.y) + 1.0) * 0.5;

    texCoord[0] = uv0;

    const float size = 0.01;
    texCoord[1] = uv0 + vec2(0.0, 1.0) * size;
    texCoord[2] = uv0 + vec2(0.0, 2.0) * size;
    texCoord[3] = uv0 + vec2(0.0, -1.0) * size;
    texCoord[4] = uv0 + vec2(0.0, -2.0) * size;
}
