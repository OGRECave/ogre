#version 150

in vec3 vertex;
in float uv0; // Timer
in float uv1; // Type
in vec3 uv2;  // Velocity
// in float inTimer;
// in float inType;
// in vec3 inVelocity;


out block {
    vec3 Pos;
    float Timer;
    float Type;
    vec3 Vel;
} FireworkData;

// out vec3 Pos;
// out float Timer;
// out float Type;
// out vec3 Vel;

//The vertex shader that prepares the fireworks for display
void main()
{
    FireworkData.Pos  = vertex;
    FireworkData.Timer = uv0;
    FireworkData.Type = uv1;
    FireworkData.Vel = uv2;
    // Pos  = vertex;
    // Timer = uv0;
    // Type = uv1;
    // Vel = uv2;

    // FireworkData.Pos = vertex;
    // FireworkData.Timer = 1;
    // FireworkData.Type = 8;
    // FireworkData.Vel = vec3(3, 3, 3);
}
