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

in vec3 position;
// timer
in float uv0;
// type
in float uv1;
// velocity
in vec3 uv2;

out block {
    vec3 pos;
    vec4 color;
    float radius;
} ColoredFirework;

uniform mat4 worldViewProj;

//The vertex shader that prepares the fireworks for display
void main()
{
    float inTimer = uv0;
    float inType = uv1;
    
    //
    // Pass the point through
    //
    ColoredFirework.pos = position; // Multiply by world matrix?
    ColoredFirework.radius = 1.5;
    
    //  
    // calculate the color
    //
    if (inType == PT_LAUNCHER)
    {
        // red
        ColoredFirework.color = vec4(1, 0.1, 0.1, 1);
        ColoredFirework.radius = 1.0;
    }
    else if (inType == PT_SHELL)
    {
        // cyan
        ColoredFirework.color = vec4(0.1, 1, 1, 1);
        ColoredFirework.radius = 1.0;
    }
    else if (inType == PT_EMBER1)
    {
        // yellow
        ColoredFirework.color = vec4(1, 1, 0.1, 1);
        ColoredFirework.color *= (inTimer / P_EMBER1LIFE);
    }
    else if (inType == PT_EMBER2)
    {
        // fuschia
        ColoredFirework.color = vec4(1, 0.1, 1, 1);
    }
    else if (inType == PT_EMBER3)
    {
        // red
        ColoredFirework.color = vec4(1, 0.1, 0.1, 1);
        ColoredFirework.color *= (inTimer / P_EMBER3LIFE);
    }
}
