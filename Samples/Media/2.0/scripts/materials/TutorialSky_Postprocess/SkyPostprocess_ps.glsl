#version 330

uniform samplerCube skyCubemap;

in block
{
    vec3 cameraDir;
} inPs;

out vec3 fragColour;

void main()
{
    fragColour = texture( skyCubemap, inPs.cameraDir ).xyz;
}
