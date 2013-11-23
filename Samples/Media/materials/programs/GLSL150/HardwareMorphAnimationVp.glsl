#version 150

in vec4 vertex;
in vec4 normal;
in vec4 uv0;
in vec4 uv1; // pos2

// out vec2 oUv;
out vec4 colour;

uniform mat4 worldViewProj;
uniform vec4 anim_t;

// hardware morph animation (no normals)
void main()
{
	// interpolate
	vec4 interp = vec4(vertex.xyz + anim_t.x * (uv1.xyz - vertex.xyz), 1.0);
	
	gl_Position = worldViewProj * interp;
	// oUv = uv0.xy;
	colour = vec4(1.0, 0.0, 0.0, 1.0);
}
