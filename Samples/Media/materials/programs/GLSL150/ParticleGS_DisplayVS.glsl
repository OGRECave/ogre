#version 150

//
// Explanation of different particle types
//
// Firework Launcher - launches a PT_SHELL every so many seconds.
#define PT_LAUNCHER 0 
// Unexploded shell - flies from the origin and explodes into many PT_EMBERX's.
#define PT_SHELL    1 
// Basic particle - after it's emitted from the shell, it dies.
#define PT_EMBER1   2 
// After it's emitted, it explodes again into many PT_EMBER1's.
#define PT_EMBER2   3 
// Just a differently colored ember1.
#define PT_EMBER3   4 
#define P_SHELLLIFE 3.0
#define P_EMBER1LIFE 2.5
#define P_EMBER2LIFE 1.5
#define P_EMBER3LIFE 2.0

in vec4 position;
// timer
in float uv0;
// type
in float uv1;
// velocity
in vec3 uv2;

out vec4 colour;

uniform mat4 worldView;
uniform mat4 proj;
uniform float height;

//The vertex shader that prepares the fireworks for display
void main()
{
    float inTimer = uv0;
    float inType = uv1;

    float radius = 1.5;
    
    gl_Position = worldView * position;

    //  
    // calculate the colour
    //
    if (inType == PT_LAUNCHER)
    {
        // red
        colour = vec4(1, 0.1, 0.1, 1);
        radius = 1.0;
    }
    else if (inType == PT_SHELL)
    {
        // cyan
        colour = vec4(0.1, 1, 1, 1);
        radius = 1.0;
    }
    else if (inType == PT_EMBER1)
    {
        // yellow
        colour = vec4(1, 1, 0.1, 1);
        colour *= (inTimer / P_EMBER1LIFE);
    }
    else if (inType == PT_EMBER2)
    {
        // fuschia
        colour = vec4(1, 0.1, 1, 1);
    }
    else if (inType == PT_EMBER3)
    {
        // red
        colour = vec4(1, 0.1, 0.1, 1);
        colour *= (inTimer / P_EMBER3LIFE);
    }

    vec4 tmp = proj * vec4(radius*2, 0, gl_Position.z, 1);
    gl_PointSize = tmp.x/tmp.w*height;
    gl_Position = proj*gl_Position;
}
