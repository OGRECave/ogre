#version 150

in vec3 vertex;
out vec4 Position0;
// uniform float inTimer;
// uniform float inType;
// uniform vec3 inVelocity;

// out block {
//     vec4        Pos;
//     // float       Timer;
//     // float       Type;
//     // vec3        Vel;
// } FireworkData;

//The vertex shader that prepares the fireworks for display
void main()
{
    // gl_Position = vertex;
    Position0 = vec4(vertex,0);
    // FireworkData.Pos = vertex;
    // FireworkData.Timer = inTimer;
    // FireworkData.Type = inType;
    // FireworkData.Vel = inVelocity;

    // FireworkData.Pos = vertex;
    // FireworkData.Timer = 0;
    // FireworkData.Type = 0;
    // FireworkData.Vel = vec3(3,3,3);
}
