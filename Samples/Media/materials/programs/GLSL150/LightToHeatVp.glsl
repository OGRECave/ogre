#version 150

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
