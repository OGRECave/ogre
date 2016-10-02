#version 330

in vec3 vertex;

uniform mat4 worldViewProj;
uniform mat4 localToProbeLocal;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec3 posLS;
} outVs;

void main()
{
	gl_Position = worldViewProj * vec4( vertex.xyz, 1.0 );
	outVs.posLS = ( localToProbeLocal * vec4( vertex.xyz, 1.0 ) ).xyz;
	outVs.posLS.z = -outVs.posLS.z; //Left handed
}
