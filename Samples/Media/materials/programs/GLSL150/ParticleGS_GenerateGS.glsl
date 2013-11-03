#version 150

//
// Explanation of different particle types.
//
// Firework Launcher - launches a PT_SHELL every so many seconds.
#define PT_LAUNCHER 0 
// Unexploded shell - flies from the origin and explodes into many PT_EMBERX's.
#define PT_SHELL    1 
// Basic particle - after it's emitted from the shell, it dies.
#define PT_EMBER1   2 
// After it's emitted, it explodes again into many PT_EMBER1's.
#define PT_EMBER2   3 
// Just a different colored ember1.
#define PT_EMBER3   4
#define P_SHELLLIFE 3.0
#define P_EMBER1LIFE 2.5
#define P_EMBER2LIFE 1.5
#define P_EMBER3LIFE 2.0

// These two were originally shader params, but they caused runtime errors.
#define NUM_EMBER_1S 30
#define NUM_EMBER_2S 15
#define NUM_EMBER_3S 10
// This one was originally a variant, but also causes runtime errors.
//#define MAX_EMBER_2S 15.0

layout(points) in;
layout(points, max_vertices = 60) out;

in block {
    vec3 Pos;
    float Timer;
    float Type;
    vec3 Vel;
} FireworkData[];

out vec3 outputPos;
out float outputTimer;
out float outputType;
out vec3 outputVel;

uniform sampler1D randomTexture;
uniform vec3 frameGravity;
uniform float globalTime;
uniform float elapsedTime;
uniform float secondsPerFirework;

//
// Generic particle motion handler
//
void GSGenericHandler( vec3 Pos, vec3 Vel, float Timer, float Type,
                       float elapsedTime,
                       vec3 frameGravity)
{
    Vel += frameGravity;
    Timer -= elapsedTime;

    if (Pos.y > -100)
    {
        outputPos = Pos + (Vel * elapsedTime);
        outputTimer = Timer;
        outputType = Type;
        outputVel = Vel;
        EmitVertex();
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
    if (Timer <= 0)
    {
        vec3 vRandom = normalize(RandomDir(Type, globalTime, randomTex));
        // Give it more of an up bias.
        vRandom = normalize(vRandom + vec3(0, 2.5, 0));

        // Time to emit a new SHELL.
        outputPos = Pos + Vel * elapsedTime;
        outputVel = Vel + vRandom * 35.0;
        outputTimer = P_SHELLLIFE + vRandom.y * 0.5;
        outputType = PT_SHELL;
        EmitVertex();

        // Reset our timer.
        Timer = secondsPerFirework + vRandom.x * 0.4;
    }
    else
    {
        Timer -= elapsedTime;
    }

    // Emit ourselves to keep us alive.
    outputPos = Pos;
    outputVel = Vel;
    outputTimer = Timer;
    outputType = Type;
    EmitVertex();
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
    if (Timer <= 0)
    {
        vec3 vRandom = vec3(0,0,0);

        // Time to emit a series of new Ember1s.
        for (int i = 0; i < NUM_EMBER_1S; i++)
        {
            vRandom = normalize(RandomDir(Type + i, globalTime, randomTex));
            outputPos = Pos + Vel * elapsedTime;
            outputVel = Vel + vRandom * 15.0;
            outputTimer = P_EMBER1LIFE;
            outputType = PT_EMBER1;
            EmitVertex();
        }

        // Find out how many Ember2s to emit.
        // Not doing this because it causes a runtime error.
        //int numEmber2s = abs(vRandom.x)*MAX_EMBER_2S;
        for (int i = 0; i < NUM_EMBER_2S; i++)
        {
            vRandom = normalize(RandomDir(Type, globalTime, randomTex));
            outputPos = Pos + Vel * elapsedTime;
            outputVel = Vel + vRandom * 10.0;
            outputTimer = P_EMBER2LIFE + 0.4 * vRandom.x;
            outputType = PT_EMBER2;
            EmitVertex();
        }

    }
    else
    {
        GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity);
    }
}

//
// Ember1 and Ember3 type particle handler.
//
void GSEmber1Handler( vec3 Pos, vec3 Vel, float Timer, float Type,
                      float elapsedTime,
                      vec3 frameGravity)
{
    if (Timer > 0)
    {
        GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity);
    }
}

//
// Ember2 type particle handler.
//
void GSEmber2Handler( vec3 Pos, vec3 Vel, float Timer, float Type,
                      float elapsedTime,
                      float globalTime,
                      sampler1D randomTex,
                      vec3 frameGravity)
{ 
    if (Timer <= 0)
    {
        // Time to emit a series of new Ember3's.
        for (int i = 0; i < NUM_EMBER_3S; i++)
        {
            outputPos = Pos + Vel * elapsedTime;
            outputVel = Vel + normalize(RandomDir(Type + i, globalTime, randomTex)) * 10.0;
            outputTimer = P_EMBER3LIFE;
            outputType = PT_EMBER3;
            EmitVertex();
        }
    }
    else
    {
        GSGenericHandler(Pos, Vel, Timer, Type, elapsedTime, frameGravity);
    }
}

void main()
{
    if (FireworkData[0].Type == PT_LAUNCHER)
        GSLauncherHandler(FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type,
                           elapsedTime, globalTime, randomTexture, secondsPerFirework);
    else if (FireworkData[0].Type == PT_SHELL)
        GSShellHandler(FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type,
                        elapsedTime, globalTime, randomTexture, frameGravity);
    else if (FireworkData[0].Type == PT_EMBER1 ||
             FireworkData[0].Type == PT_EMBER3)
        GSEmber1Handler(FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type,
                         elapsedTime, frameGravity);
    else if (FireworkData[0].Type == PT_EMBER2)
        GSEmber2Handler(FireworkData[0].Pos.xyz, FireworkData[0].Vel, FireworkData[0].Timer, FireworkData[0].Type,
                         elapsedTime, globalTime, randomTexture, frameGravity);
    EndPrimitive();

    // // gl_Position.xyz = FireworkData[0].Pos;// + FireworkData[0].Vel * elapsedTime;
    // // gl_Position.w = 1;
    // outputPos = FireworkData[0].Pos + 1;// + FireworkData[0].Vel * elapsedTime;
    // // outputPos = Pos[0];// + FireworkData[0].Vel * elapsedTime;
    // //gl_Position = vec4(10, 10, 10, 1);
    // // outputTimer = P_SHELLLIFE + 0.5;
    // outputTimer = FireworkData[0].Timer;
    // // outputTimer = Timer[0];
    // outputType = FireworkData[0].Type;
    // // outputType = Type[0];
    // outputVel = FireworkData[0].Vel;
    // // outputVel = Vel[0];
    // //outputVel = vec3(4, 5, 35.0);
    // EmitVertex();
    // EndPrimitive();
    // // // gl_Position = vec4(10, 20, 30, 1);
    // outputPos = vec3(10, 20, 30);
    // outputTimer = P_EMBER3LIFE;
    // outputType = PT_SHELL;
    // outputVel = vec3(40, 50, 60);
    // EmitVertex();
    // EndPrimitive();
}
