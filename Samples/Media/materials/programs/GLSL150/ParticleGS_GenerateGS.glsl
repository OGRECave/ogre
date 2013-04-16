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

//These two were originally shader params, but they caused runtime errors
#define NUM_EMBER_1S 30
#define NUM_EMBER_2S 15
#define NUM_EMBER_3S 10
//This one was originally a variant, but this also causes runtime errors
//#define MAX_EMBER_2S 15.0

in block {
	vec4 	Pos;
	float 	Timer;
    float 	Type;
	vec3	Vel;
} FireworkData[];

layout(points) in;
layout(points, max_vertices = 1) out;

uniform sampler1D randomTex;
uniform vec3 frameGravity;
uniform float globalTime;
uniform float elapsedTime;
uniform float secondsPerFirework;
out vec4 colour;
//
// Generic particle motion handler
//

void GSGenericHandler( vec3 Pos, vec3 Vel, float Timer, float Type,
						float elapsedTime, 
						vec3 frameGravity)
{
    gl_Position.xyz = Pos + (Vel * elapsedTime);
    Vel += frameGravity;
    Timer -= elapsedTime;
	if (Pos.y > -100)
	{
		EmitVertex();//Pos : POSITION, Vel : TEXCOORD2, Timer : TEXCOORD0, Type : TEXCOORD1);
	}
}

//
// Sample a random direction from our random texture
//
vec3 RandomDir(float fOffset, float globalTime, sampler1D randomTex)
{
    float tCoord = (globalTime + fOffset) / 300.0;
    return texture(randomTex, tCoord).rgb;
}

//
// Launcher type particle handler
//
void GSLauncherHandler( vec3 Pos, vec3 Vel, float Timer, float Type, 
						float elapsedTime, 
						float globalTime, 
						sampler1D randomTex,
						float secondsPerFirework)
{
    if(Timer <= 0)
    {
        vec3 vRandom = normalize( RandomDir( Type, globalTime, randomTex) );
		//Give it more of an up bias
        vRandom = normalize(vRandom + vec3(0,2.5,0)); 
		
		//time to emit a new SHELL
        gl_Position.xyz = Pos + Vel*elapsedTime;
        vec3 outputVel = Vel + vRandom*35.0;
        float  outputTimer = P_SHELLLIFE + vRandom.y*0.5;
        float outputType = PT_SHELL;
        EmitVertex();//(outputPos : POSITION, outputVel : TEXCOORD2, outputTimer : TEXCOORD0, outputType : TEXCOORD1);
        
        //reset our timer
        Timer = secondsPerFirework + vRandom.x*0.4;
    }
    else
    {
        Timer -= elapsedTime;
    }
    
    //emit ourselves to keep us alive
    EmitVertex();//( Pos : POSITION, Vel : TEXCOORD2, Timer : TEXCOORD0, Type : TEXCOORD1);
}

//
// Shell type particle handler
//	
void GSShellHandler( vec3 Pos, vec3 Vel, float Timer, float Type,
					float elapsedTime, 
					float globalTime, 
					sampler1D randomTex,
					vec3 frameGravity)
{
    if(Timer <= 0)
    {
        vec3 outputPos;
		vec3 outputVel;
		float outputTimer;
		float outputType;
		
        vec3 vRandom = vec3(0,0,0);
        
        //time to emit a series of new Ember1s  
        for(int i=0; i<NUM_EMBER_1S; i++)
        {
            vRandom = normalize( RandomDir( Type+i, globalTime, randomTex ) );
            gl_Position.xyz = Pos + Vel*elapsedTime;
            outputVel = Vel + vRandom*15.0;
            outputTimer = P_EMBER1LIFE;
            outputType = PT_EMBER1;
            EmitVertex();//(outputPos : POSITION, outputTimer : TEXCOORD0, outputType : TEXCOORD1, outputVel : TEXCOORD2);
        }
        
        //find out how many Ember2s to emit
		//Not doing this because it causes a runtime error
		//int numEmber2s = abs(vRandom.x)*MAX_EMBER_2S;
        for(int i=0; i<NUM_EMBER_2S; i++)
        {
            vRandom = normalize( RandomDir( Type, globalTime, randomTex) );
            gl_Position.xyz = Pos + Vel*elapsedTime;
            outputVel = Vel + vRandom*10.0;
            outputTimer = P_EMBER2LIFE + 0.4*vRandom.x;
            outputType = PT_EMBER2;
            EmitVertex();//(outputPos : POSITION, outputVel : TEXCOORD2, outputTimer : TEXCOORD0, outputType : TEXCOORD1);
        }
        
    }
    else
    {
        GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity );
    }
}

//
// Ember1 and Ember3 type particle handler
//
void GSEmber1Handler( vec3 Pos, vec3 Vel, float Timer, float Type,
						float elapsedTime, 
						vec3 frameGravity)
{
    if(Timer > 0)
    {
        GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity );
    }
}

//
// Ember2 type particle handler
//
void GSEmber2Handler( vec3 Pos, vec3 Vel, float Timer, float Type,
						float elapsedTime, 
						float globalTime, 
						sampler1D randomTex,
						vec3 frameGravity)
{
    if(Timer <= 0)
    {
		vec3 outputPos;
		vec3 outputVel;
		float outputTimer;
		float outputType;
    
        //time to emit a series of new Ember3s  
        for(int i=0; i<NUM_EMBER_3S; i++)
        {
            gl_Position.xyz = Pos + Vel*elapsedTime;
            outputVel = Vel + normalize( RandomDir( Type + i, globalTime, randomTex ) )*10.0;
            outputTimer = P_EMBER3LIFE;
            outputType = PT_EMBER3;
            EmitVertex();
        }
    }
    else
    {
         GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity );
    }
}

void main()
{
	if( FireworkData[0].Type == PT_LAUNCHER )
        GSLauncherHandler( FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type, 
							elapsedTime, globalTime, randomTex, secondsPerFirework);
	else if ( FireworkData[0].Type == PT_SHELL )
        GSShellHandler( FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type, 
							elapsedTime, globalTime, randomTex, frameGravity);
	else if ( FireworkData[0].Type == PT_EMBER1 ||
              FireworkData[0].Type == PT_EMBER3 )
        GSEmber1Handler( FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type, 
							elapsedTime, frameGravity);
    else if( FireworkData[0].Type == PT_EMBER2 )
        GSEmber2Handler( FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type, 
							elapsedTime, globalTime, randomTex, frameGravity);
    colour = vec4(1.0,1.0,0.0,1.0);
	EndPrimitive();
}
