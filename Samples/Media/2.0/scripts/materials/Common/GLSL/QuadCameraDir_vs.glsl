#version 330

in vec4 vertex;
in vec3 normal;
in vec2 uv0;
uniform mat4 worldViewProj;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 uv0;
	vec3 cameraDir;
} outVs;

void main()
{
	gl_Position = (worldViewProj * vertex).xyzw;
	outVs.uv0.xy		= uv0.xy;
	outVs.cameraDir.xyz	= normal.xyz;
}
