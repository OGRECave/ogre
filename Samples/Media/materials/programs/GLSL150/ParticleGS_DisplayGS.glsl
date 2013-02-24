#version 150

in block {
	vec3 	pos;
    vec4 	color;
	float	radius;
} ColoredFirework[];

out block {
	vec4 	pos;
    vec4 	color;
	vec2	texcoord;
} Firework;

uniform mat4 inverseView;
uniform mat4 worldViewProj;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

//The geometry shader that prepares the fireworks for display
void main()
{
	vec3 g_positions[4] = vec3[4](vec3(-1, 1, 0), vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0));
    vec2 g_texcoords[4] = vec2[4](vec2(0, 1), vec2(1, 1), vec2(0, 0), vec2(1, 0));

    //
    // Emit two new triangles
    //
    for(int i=0; i<4; i++)
    {
		vec3 position = -g_positions[i] * ColoredFirework[0].radius;
        position = mat3(inverseView) * position + ColoredFirework[0].pos;
        gl_Position = worldViewProj * vec4(position, 1.0);

        Firework.pos = gl_Position;
        Firework.color = ColoredFirework[0].color;
        Firework.texcoord = g_texcoords[i];
        EmitVertex();
    }
    EndPrimitive();
}
