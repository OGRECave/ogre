attribute vec4 position;
attribute vec3 normal;

varying vec4 oUv0;

uniform mat4 worldViewProj;

void main()
{
    gl_Position = worldViewProj * position;
    oUv0 = vec4(normal, 1.0);
}
