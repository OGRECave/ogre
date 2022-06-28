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

//
// Generic particle motion handler
//

struct VSParticleIn
{
	float3 pos   : POSITION;
	float timer  : TEXCOORD0;
	float type	 : TEXCOORD1;
	float3 vel   : TEXCOORD2;
};

struct VSParticleDrawOut
{
    float4 pos : POSITION;
	float4 color : COLOR0;
	float2 radius : TEXCOORD0;
};

struct PSSceneIn
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD0;
    float4 color : COLOR0;
};

SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
};

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

void GSGenericHandler(  VSParticleIn input
					  , float elapsedTime
					  , float3 frameGravity
					  , inout PointStream<VSParticleIn> OutputStream
					  )
{
    input.pos.xyz += input.vel * elapsedTime;
	input.vel += frameGravity;
	input.timer -= elapsedTime;
	if (input.pos.y > -100)
	{
		OutputStream.Append(input);
	}
}

//
// Sample a random direction from our random texture
//
float3 RandomDir(float fOffset, float globalTime, Texture1D randomTex)
{
    float2 tCoord;
	tCoord.x = (globalTime + fOffset) / 300.0;
	tCoord.y = 0.5;
    return randomTex.SampleLevel(g_samPoint, tCoord.x, 0).rgb;
}

//
// Launcher type particle handler
//
void GSLauncherHandler( VSParticleIn input,
						float elapsedTime, 
						float globalTime, 
						Texture1D randomTex,
						float secondsPerFirework, 
						inout PointStream<VSParticleIn> outputStream)
{
    if(input.timer <= 0)
    {
		VSParticleIn gsOutput;
        float3 vRandom = normalize( RandomDir( input.type, globalTime, randomTex) );
		//Give it more of an up bias
        vRandom = normalize(vRandom + float3(0,2.5,0)); 
		
		//time to emit a new SHELL
        gsOutput.pos.xyz = input.pos.xyz + input.vel*elapsedTime;
        gsOutput.vel = input.vel + vRandom*35.0;
        gsOutput.timer = P_SHELLLIFE + vRandom.y*0.5;
        gsOutput.type = PT_SHELL;

		outputStream.Append(gsOutput);
		        
        //reset our timer
        input.timer = secondsPerFirework + vRandom.x*0.4;
    }
    else
    {
        input.timer -= elapsedTime;
    }
    
    //emit ourselves to keep us alive
    outputStream.Append(input);
}

//
// Shell type particle handler
//	
void GSShellHandler( VSParticleIn input,
					float elapsedTime, 
					float globalTime, 
					Texture1D randomTex,
					float3 frameGravity,
					inout PointStream<VSParticleIn> outputStream)
{
    if(input.timer <= 0)
    {
		VSParticleIn gsOutput;
		
        float3 vRandom = float3(0,0,0);
        
        //time to emit a series of new Ember1s  
		{
        for(int i=0; i<NUM_EMBER_1S; i++)
        {
            vRandom = normalize( RandomDir( input.type+i, globalTime, randomTex ) );
            gsOutput.pos.xyz = input.pos.xyz + input.vel*elapsedTime;
            gsOutput.vel = input.vel + vRandom*15.0;
            gsOutput.timer = P_EMBER1LIFE;
            gsOutput.type = PT_EMBER1;
            
			outputStream.Append(gsOutput);
        }
		}
        
        //find out how many Ember2s to emit
		//Not doing this because it causes a runtime error
		//int numEmber2s = abs(vRandom.x)*MAX_EMBER_2S;
		{
        for(int i=0; i<NUM_EMBER_2S; i++)
        {
            vRandom = normalize( RandomDir( input.type, globalTime, randomTex) );
            gsOutput.pos.xyz = input.pos.xyz + input.vel*elapsedTime;
            gsOutput.vel = input.vel + vRandom*10.0;
            gsOutput.timer = P_EMBER2LIFE + 0.4*vRandom.x;
            gsOutput.type = PT_EMBER2;

			outputStream.Append(gsOutput);
        }
		}
    }
    else
    {
        GSGenericHandler(input, elapsedTime, frameGravity, outputStream);
    }
}

//
// Ember1 and Ember3 type particle handler
//
void GSEmber1Handler(VSParticleIn input, float elapsedTime, float3 frameGravity, inout PointStream<VSParticleIn> outputStream)
{
    if(input.timer > 0)
    {
        GSGenericHandler(input, elapsedTime, frameGravity, outputStream);
    }
}

//
// Ember2 type particle handler
//
void GSEmber2Handler(VSParticleIn input
					 , float elapsedTime
					 , float globalTime
					 , Texture1D randomTex
					 , float3 frameGravity
					 , inout PointStream<VSParticleIn> outputStream)
{
	VSParticleIn gsOutput;
    if(input.timer <= 0)
    {
        //time to emit a series of new Ember3s  
        for(int i=0; i<NUM_EMBER_3S; i++)
        {
            gsOutput.pos.xyz = input.pos.xyz + input.vel*elapsedTime;
            gsOutput.vel = input.vel + normalize( RandomDir( input.type + i, globalTime, randomTex ) )*10.0;
            gsOutput.timer = P_EMBER3LIFE;
            gsOutput.type = PT_EMBER3;

			outputStream.Append(gsOutput);
        }
    }
    else
    {
         GSGenericHandler(input, elapsedTime, frameGravity, outputStream);
    }
}

//The vertex shader that prepares the fireworks for display
VSParticleIn GenerateParticles_VS(VSParticleIn input)
{
	VSParticleIn output = input;
	return output;
}

[maxvertexcount(64)]
void GenerateParticles_GS(point VSParticleIn input[1], inout PointStream<VSParticleIn> ParticleOutputStream
	, uniform Texture1D randomTex
	, uniform float3 gravity
	, uniform float globalTime
	, uniform float elapsedTime
	, uniform float secondsPerFirework
	)
{
    float3 frameGravity = gravity * elapsedTime;
	if( input[0].type == PT_LAUNCHER )
        GSLauncherHandler( input[0], elapsedTime, globalTime, randomTex, secondsPerFirework, ParticleOutputStream);
	else if ( input[0].type == PT_SHELL )
        GSShellHandler( input[0], elapsedTime, globalTime, randomTex, frameGravity, ParticleOutputStream);
	else if ( input[0].type == PT_EMBER1 ||input[0].type == PT_EMBER3 )
       GSEmber1Handler( input[0], elapsedTime, frameGravity, ParticleOutputStream);
    else if( input[0].type == PT_EMBER2 )
        GSEmber2Handler( input[0], elapsedTime, globalTime, randomTex, frameGravity, ParticleOutputStream);
}

//The vertex shader that prepares the fireworks for display
VSParticleDrawOut DisplayParticles_VS(VSParticleIn input,
                                        uniform float4x4 worldView,
                                        uniform float4x4 proj)
{
	VSParticleDrawOut output;
    //
    // Pass the point through
    //
    output.pos = mul(worldView, float4(input.pos,1));
    output.radius = float2(1.5, 1.5);
    
    //  
    // calculate the color
    //
    if( input.type == PT_LAUNCHER )
    {
        output.color = float4(1,0.1,0.1,1);
        output.radius = float2(1.0, 1.0);
    }
    else if( input.type == PT_SHELL )
    {
        output.color = float4(0.1,1,1,1);
        output.radius = float2(1.0, 1.0);
    }
    else if( input.type == PT_EMBER1 )
    {
        output.color = float4(1,1,0.1,1);
        output.color *= (input.timer / P_EMBER1LIFE );
    }
    else if( input.type == PT_EMBER2 )
    {
        output.color = float4(1,0.1,1,1);
    }
    else if( input.type == PT_EMBER3 )
    {
        output.color = float4(1,0.1,0.1,1);
        output.color *= (input.timer / P_EMBER3LIFE );
    }
    else
    {
        output.color = float4(0,0,0,0);
    }

    float4 tmp = mul(proj, float4(output.radius, output.pos.z, 1));
    output.radius = tmp.xy; // no w divison here, no viewport normalization in GS
    output.pos = mul(proj, output.pos);

    return output;
}

//The geometry shader that prepares the fireworks for display
[maxvertexcount(4)]
void DisplayParticles_GS( point VSParticleDrawOut input[1]
						, inout TriangleStream<PSSceneIn> SpriteStream)
{
	float2 g_positions[4] = { float2( -1, 1 ), float2( -1, -1 ), float2( 1, 1 ), float2( 1, -1 ) };
    float2 g_texcoords[4] = { float2(0,1), float2(1,1), float2(0,0), float2(1,0) };

	 PSSceneIn output;
	
	//
    // Emit two new triangles
    //
    for(int i=0; i<4; i++)
    {
        output.pos = input[0].pos - float4(g_positions[i]*input[0].radius, 0, 0);
		output.tex = g_texcoords[i];
		output.color = input[0].color;
		SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}

//The pixels shaders that colors the fireworks
float4 DisplayParticles_PS(	PSSceneIn input
						  , uniform Texture2D diffuseTex) : SV_Target
{
	return diffuseTex.Sample(g_samLinear, input.tex) * input.color;
}