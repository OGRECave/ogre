#version 150

//
// Explanation of different particle types
//
#define PT_LAUNCHER 0 //Firework Launcher - launches a PT_SHELL every so many seconds
#define PT_SHELL    1 //Unexploded shell - flies from the origin and explodes into many PT_EMBERXs
#define PT_EMBER1   2 //basic particle - after it's emitted from the shell, it dies
#define PT_EMBER2   3 //after it's emitted, it explodes again into many PT_EMBER1s
#define PT_EMBER3   4 //just a differently colored ember1
#define P_SHELLLIFE 3.0
#define P_EMBER1LIFE 2.5
#define P_EMBER2LIFE 1.5
#define P_EMBER3LIFE 2.0

in vec4 position;
uniform float inTimer;
uniform float inType;
uniform vec3 inVelocity;

out block {
	vec3 	pos;
    vec4 	color;
	float	radius;
} ColoredFirework;

uniform mat4 worldViewProj;

//The vertex shader that prepares the fireworks for display
void main()
{
    //
    // Pass the point through
    //
    ColoredFirework.pos = position.xyz; //Multiply by world matrix?
    ColoredFirework.radius = 1.5;
    
    //  
    // calculate the color
    //
    if( inType == PT_LAUNCHER )
    {
        ColoredFirework.color = vec4(1,0.1,0.1,1);
        ColoredFirework.radius = 1.0;
    }
    else if( inType == PT_SHELL )
    {
        ColoredFirework.color = vec4(0.1,1,1,1);
        ColoredFirework.radius = 1.0;
    }
    else if( inType == PT_EMBER1 )
    {
        ColoredFirework.color = vec4(1,1,0.1,1);
        ColoredFirework.color *= (inTimer / P_EMBER1LIFE );
    }
    else if( inType == PT_EMBER2 )
    {
        ColoredFirework.color = vec4(1,0.1,1,1);
    }
    else if( inType == PT_EMBER3 )
    {
        ColoredFirework.color = vec4(1,0.1,0.1,1);
        ColoredFirework.color *= (inTimer / P_EMBER3LIFE );
    }
}
