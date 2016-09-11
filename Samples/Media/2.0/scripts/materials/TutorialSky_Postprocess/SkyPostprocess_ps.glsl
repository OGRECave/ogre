#version 330

uniform samplerCube skyCubemap;

in block
{
    vec3 cameraDir;
} inPs;

out vec3 fragColour;

void main()
{
	//Cubemaps are left-handed
	fragColour = texture( skyCubemap, vec3( inPs.cameraDir.xy, -inPs.cameraDir.z ) ).xyz;
}
