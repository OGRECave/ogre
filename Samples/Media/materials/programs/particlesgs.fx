//--------------------------------------------------------------------------------------
// File: ParticlesGS.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// this is the original shader from the dx sdk with a few minor changes so the sample will work.
//--------------------------------------------------------------------------------------
struct VSParticleIn
{
    float3 pos              : POSITION;         //position of the particle
    float3 vel              : TEXCOORD0;           //velocity of the particle
    float  Timer            : TEXCOORD1;            //timer for the particle
    uint   Type             : TEXCOORD2;             //particle type
};

struct VSParticleDrawOut
{
    float3 pos : POSITION;
    float4 color : COLOR0;
    float radius : RADIUS;
};

struct PSSceneIn
{
    float4 pos : SV_Position;
    float2 tex : TEXTURE0;
    float4 color : COLOR0;
};

cbuffer cb0
{
    float4x4 worldViewProj;
    float4x4 inverseView;
    float globalTime;
    float elapsedTime;
    float4 frameGravity;
    float secondsPerFirework = 1.0;
    float3 g_positions[4] =
    {
        float3( -1, 1, 0 ),
        float3( 1, 1, 0 ),
        float3( -1, -1, 0 ),
        float3( 1, -1, 0 ),
    };
    float2 g_texcoords[4] = 
    { 
        float2(0,1), 
        float2(1,1),
        float2(0,0),
        float2(1,0),
    };
};

#define NUM_EMBER_1S 30
#define NUM_EMBER_2S 15


Texture2D g_txDiffuse;
SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

Texture1D g_txRandom;
SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

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

//
// Vertex shader for drawing the point-sprite particles
//
VSParticleDrawOut VSScenemain(VSParticleIn input)
{
    VSParticleDrawOut output = (VSParticleDrawOut)0;
    
    //
    // Pass the point through
    //
    output.pos = input.pos;
    output.radius = 1.5;
    
    //  
    // calculate the color
    //
    if( input.Type == PT_LAUNCHER )
    {
        output.color = float4(1,0.1,0.1,1);
        output.radius = 1.0;
    }
    else if( input.Type == PT_SHELL )
    {
        output.color = float4(0.1,1,1,1);
        output.radius = 1.0;
    }
    else if( input.Type == PT_EMBER1 )
    {
        output.color = float4(1,1,0.1,1);
        output.color *= (input.Timer / P_EMBER1LIFE );
    }
    else if( input.Type == PT_EMBER2 )
    {
        output.color = float4(1,0.1,1,1);
    }
    else if( input.Type == PT_EMBER3 )
    {
        output.color = float4(1,0.1,0.1,1);
        output.color *= (input.Timer / P_EMBER3LIFE );
    }
    
    return output;
}

//
// Passthrough VS for the streamout GS
//
VSParticleIn VSPassThroughmain(VSParticleIn input)
{
    return input;
}

//
// Sample a random direction from our random texture
//
float3 RandomDir(float fOffset)
{
    float tCoord = (globalTime + fOffset) / 300.0;
    return g_txRandom.SampleLevel( g_samPoint, tCoord, 0 );
}

//
// Generic particle motion handler
//

void GSGenericHandler( VSParticleIn input, inout PointStream<VSParticleIn> ParticleOutputStream )
{
    input.pos += input.vel*elapsedTime;
    input.vel += frameGravity;
    input.Timer -= elapsedTime;
    ParticleOutputStream.Append( input );
}

//
// Launcher type particle handler
//
void GSLauncherHandler( VSParticleIn input, inout PointStream<VSParticleIn> ParticleOutputStream )
{
    if(input.Timer <= 0)
    {
        float3 vRandom = normalize( RandomDir( input.Type ) );
        //time to emit a new SHELL
        VSParticleIn output;
        output.pos = input.pos + input.vel*elapsedTime;
        output.vel = input.vel + vRandom*8.0;
        output.Timer = P_SHELLLIFE + vRandom.y*0.5;
        output.Type = PT_SHELL;
        ParticleOutputStream.Append( output );
        
        //reset our timer
        input.Timer = secondsPerFirework + vRandom.x*0.4;
    }
    else
    {
        input.Timer -= elapsedTime;
    }
    
    //emit ourselves to keep us alive
    ParticleOutputStream.Append( input );
    
}

//
// Shell type particle handler
//
void GSShellHandler( VSParticleIn input, inout PointStream<VSParticleIn> ParticleOutputStream )
{
    if(input.Timer <= 0)
    {
        VSParticleIn output;
        float3 vRandom = float3(0,0,0);
        
        //time to emit a series of new Ember1s  
        for(int i=0; i<NUM_EMBER_1S; i++)
        {
            vRandom = normalize( RandomDir( input.Type + i ) );
            output.pos = input.pos + input.vel*elapsedTime;
            output.vel = input.vel + vRandom*15.0;
            output.Timer = P_EMBER1LIFE;
            output.Type = PT_EMBER1;
            ParticleOutputStream.Append( output );
        }
        
        //find out how many Ember2s to emit
        for(int i=0; i<abs(vRandom.x)*NUM_EMBER_2S; i++)
        {
            vRandom = normalize( RandomDir( input.Type + i ) );
            output.pos = input.pos + input.vel*elapsedTime;
            output.vel = input.vel + vRandom*10.0;
            output.Timer = P_EMBER2LIFE + 0.4*vRandom.x;
            output.Type = PT_EMBER2;
            ParticleOutputStream.Append( output );
        }
        
    }
    else
    {
        GSGenericHandler( input, ParticleOutputStream );
    }
}

//
// Ember1 and Ember3 type particle handler
//
void GSEmber1Handler( VSParticleIn input, inout PointStream<VSParticleIn> ParticleOutputStream )
{
    if(input.Timer > 0)
    {
        GSGenericHandler( input, ParticleOutputStream );
    }
}

//
// Ember2 type particle handler
//
void GSEmber2Handler( VSParticleIn input, inout PointStream<VSParticleIn> ParticleOutputStream )
{
    if(input.Timer <= 0)
    {
        VSParticleIn output;
    
        //time to emit a series of new Ember3s  
        for(int i=0; i<10; i++)
        {
            output.pos = input.pos + input.vel*elapsedTime;
            output.vel = input.vel + normalize( RandomDir( input.Type + i ) )*10.0;
            output.Timer = P_EMBER3LIFE;
            output.Type = PT_EMBER3;
            ParticleOutputStream.Append( output );
        }
    }
    else
    {
        GSGenericHandler( input, ParticleOutputStream );
    }
}

//
// Main particle system handler... handler particles and streams them out to a vertex buffer
//
[maxvertexcount(128)]
void GSAdvanceParticlesMain(point VSParticleIn input[1], inout PointStream<VSParticleIn> ParticleOutputStream)
{
    if( input[0].Type == PT_LAUNCHER )
        GSLauncherHandler( input[0], ParticleOutputStream );
    else if ( input[0].Type == PT_SHELL )
        GSShellHandler( input[0], ParticleOutputStream );
    else if ( input[0].Type == PT_EMBER1 ||
              input[0].Type == PT_EMBER3 )
        GSEmber1Handler( input[0], ParticleOutputStream );
    else if( input[0].Type == PT_EMBER2 )
        GSEmber2Handler( input[0], ParticleOutputStream );
}

//
// GS for rendering point sprite particles.  Takes a point and turns it into 2 tris.
//
[maxvertexcount(4)]
void GSScenemain(point VSParticleDrawOut input[1], inout TriangleStream<PSSceneIn> SpriteStream)
{
    PSSceneIn output;
    
    //
    // Emit two new triangles
    //
    for(int i=0; i<4; i++)
    {
        float3 position = g_positions[i]*input[0].radius;
        position = mul( position, (float3x3)inverseView ) + input[0].pos;
        output.pos = mul( float4(position,1.0), worldViewProj );
        
        output.color = input[0].color;
        output.tex = g_texcoords[i];
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}

//
// PS for particles
//
float4 PSScenemain(PSSceneIn input) : SV_Target
{   
    return g_txDiffuse.Sample( g_samLinear, input.tex ) * input.color;
}

//
// RenderParticles - renders particles on the screen
//
technique10 RenderParticles
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSScenemain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSScenemain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSScenemain() ) );
        
        SetBlendState( AdditiveBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }  
}

//
// AdvanceParticles - advances the particle system one time step
//

GeometryShader gsStreamOut = ConstructGSWithSO( CompileShader( gs_4_0, GSAdvanceParticlesMain() ), "POSITION.xyz; NORMAL.xyz; TIMER.x; TYPE.x" );
technique10 AdvanceParticles
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, VSPassThroughmain() ) );
        SetGeometryShader( gsStreamOut );
        SetPixelShader( NULL );
        
        SetDepthStencilState( DisableDepth, 0 );
    }  
}
