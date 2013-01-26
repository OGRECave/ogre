#version 150

in vec4 vertex;
uniform float inTimer;
uniform float inType;
uniform vec3 inVelocity;

out block {
	vec4 	Pos;
	float 	Timer;
    float 	Type;
	vec3	Vel;
} FireworkData;

//The vertex shader that prepares the fireworks for display
void main()
{
	FireworkData.Pos = vertex;
	FireworkData.Timer = inTimer;
	FireworkData.Type = inType;
	FireworkData.Vel = inVelocity;
}
