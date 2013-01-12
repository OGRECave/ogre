#version 150

uniform mat4 worldViewProj;
uniform vec3 farCorner;
in vec4 vertex;
out vec2 oUv0;
out vec3 ray;

void main()
{
	gl_Position = worldViewProj * vertex;

    // Clean up inaccuracies for the UV coords
    vec2 uv = sign(vertex).xy;
    // Convert to image space
    uv = (vec2(uv.x, -uv.y) + 1.0) * 0.5;
    oUv0 = uv;
    // Calculate the correct ray (modify XY parameters based on screen-space quad XY)
    ray = farCorner * vec3(sign(vertex.xy), 1);
}
