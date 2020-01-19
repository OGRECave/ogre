#ifdef GL_ES
#version 300 es
#else
#version 150
#endif

uniform mat4 worldViewProj;
in vec4 vertex;
in vec3 normal;
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
    // ray towards frustum far corners
    ray = normal;
}
