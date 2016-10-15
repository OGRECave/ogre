#version 330

in vec3 vertex;

uniform mat4 worldViewProj;
uniform vec4 worldScaledMatrix[3];
uniform vec3 probeCameraPosScaled;

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
	outVs.posLS.x = dot( worldScaledMatrix[0], vec4( vertex.xyz, 1.0 ) );
	outVs.posLS.y = dot( worldScaledMatrix[1], vec4( vertex.xyz, 1.0 ) );
	outVs.posLS.z = dot( worldScaledMatrix[2], vec4( vertex.xyz, 1.0 ) );
	outVs.posLS = outVs.posLS - probeCameraPosScaled;
	outVs.posLS.z = -outVs.posLS.z; //Left handed
}
