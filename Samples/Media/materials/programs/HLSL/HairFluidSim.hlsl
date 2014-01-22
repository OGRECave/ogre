/*
----------------------------------------------------------
Hair Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

// These are the values used when filling the obstacle textures
#define OBSTACLE_EXTERIOR  1.0f
#define OBSTACLE_BOUNDARY  128.0f/255.0f
#define OBSTACLE_INTERIOR  0.0f

//--------------------------------------------------------------------------------------
// Shaders to implement a "stable fluids" style semi-Lagrangian solver for 3D smoke
//--------------------------------------------------------------------------------------
// It assumes the velocity and pressure grids are collocated
// It handles boundary conditions for static obstacles stored as an in/out voxel volume
// MACCORMACK is supported for smoke density advection
// The diffusion step is skipped
//--------------------------------------------------------------------------------------

#define FT_SMOKE  0
#define FT_FIRE   1
#define FT_LIQUID 2

// Textures
Texture2D Texture_inDensity;

Texture3D Texture_pressure;
Texture3D Texture_velocity;
Texture3D Texture_vorticity;
Texture3D Texture_divergence;

Texture3D Texture_phi;
Texture3D Texture_phi_hat;
Texture3D Texture_phi_next;
Texture3D Texture_levelset;

Texture3D Texture_obstacles;
Texture3D Texture_obstvelocity;


// Variables
shared cbuffer cb0
{
	//int         fluidType = FT_SMOKE;
	int        g_advectAsTemperature = 0;
	int        g_treatAsLiquidVelocity = 0;
	int         g_drawTextureNumber = 1;
	float       g_textureWidth;
	float       g_textureHeight;
	float       g_textureDepth;

	// NOTE: The spacing between simulation grid cells is \delta x  = 1, so it is omitted everywhere
	float       g_timestep                = 1.0f;
	float       g_decay                   = 1.0f; // this is the (1.0 - dissipation_rate). dissipation_rate >= 0 ==> g_decay <= 1
	float       g_viscosity               = 5e-6f;// kinematic g_viscosity
	float       g_vortConfinementScale    = 0.0f; // this is typically a small value >= 0
	float3      g_gravity                 = 0;    // note this is assumed to be given as pre-multiplied by the g_timestep, so it's really velocity: cells per step
	float       g_temperatureLoss         = 0.003;// a constant amount subtracted at every step when advecting a quatnity as tempterature

	float       g_radius;
	float3      g_center; 
	float4      g_color;

	float4      g_obstBoxVelocity = float4(0, 0, 0, 0);
	float3      g_obstBoxLBDcorner;
	float3      g_obstBoxRTUcorner;
}


//parameters for attenuating velocity based on porous obstacles.
//these values are not hooked into CPP code yet, and so this option is not used currently
shared cbuffer constants
{
	bool        doVelocityAttenuation = false; 
	float       maxDensityAmount = 0.7;
	float       maxDensityg_decay = 0.95;
	float       liquidHeight = 24;
	float       rho                     = 1.2f; // rho = density of the fluid
}

SamplerState samPointClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

// Structs
struct VS_INPUT_FLUIDSIM
{
    float3 position          : POSITION;    // 2D slice vertex coordinates in clip space
    float3 textureCoords0    : TEXCOORD;    // 3D cell coordinates (x,y,z in 0-dimension range)
};

struct VS_OUTPUT_FLUIDSIM
{
    float4 pos               : SV_Position;
    float3 cell0             : TEXCOORD0;
    float3 texcoords         : TEXCOORD1;
    float2 LR                : TEXCOORD2;
    float2 BT                : TEXCOORD3;
    float2 DU                : TEXCOORD4;
};

struct GS_OUTPUT_FLUIDSIM
{
    float4 pos               : SV_Position; // 2D slice vertex coordinates in homogenous clip space
    float3 cell0             : TEXCOORD0;   // 3D cell coordinates (x,y,z in 0-dimension range)
    float3 texcoords         : TEXCOORD1;   // 3D cell texcoords (x,y,z in 0-1 range)
    float2 LR                : TEXCOORD2;   // 3D cell texcoords for the Left and Right neighbors
    float2 BT                : TEXCOORD3;   // 3D cell texcoords for the Bottom and Top neighbors
    float2 DU                : TEXCOORD4;   // 3D cell texcoords for the Down and Up neighbors
    uint RTIndex             : SV_RenderTargetArrayIndex;  // used to choose the destination slice
};

#define LEFTCELL    float3 (input.LR.x, input.texcoords.y, input.texcoords.z)
#define RIGHTCELL   float3 (input.LR.y, input.texcoords.y, input.texcoords.z)
#define BOTTOMCELL  float3 (input.texcoords.x, input.BT.x, input.texcoords.z)
#define TOPCELL     float3 (input.texcoords.x, input.BT.y, input.texcoords.z)
#define DOWNCELL    float3 (input.texcoords.x, input.texcoords.y, input.DU.x)
#define UPCELL      float3 (input.texcoords.x, input.texcoords.y, input.DU.y)

// Vertex shaders
VS_OUTPUT_FLUIDSIM VS_GRID( VS_INPUT_FLUIDSIM input)
{
    VS_OUTPUT_FLUIDSIM output = (VS_OUTPUT_FLUIDSIM)0;

    output.pos = float4(input.position.x, input.position.y, input.position.z, 1.0);
    output.cell0 = float3(input.textureCoords0.x, input.textureCoords0.y, input.textureCoords0.z);
    output.texcoords = float3( (input.textureCoords0.x)/(g_textureWidth),
                              (input.textureCoords0.y)/(g_textureHeight), 
                              (input.textureCoords0.z+0.5)/(g_textureDepth));

    float x = output.texcoords.x;
    float y = output.texcoords.y;
    float z = output.texcoords.z;

    // compute single texel offsets in each dimension
    float invW = 1.0/g_textureWidth;
    float invH = 1.0/g_textureHeight;
    float invD = 1.0/g_textureDepth;

    output.LR = float2(x - invW, x + invW);
    output.BT = float2(y - invH, y + invH);
    output.DU = float2(z - invD, z + invD);

    return output;
}

[maxvertexcount (3)]
void GS_ARRAY(triangle VS_OUTPUT_FLUIDSIM In[3], inout TriangleStream<GS_OUTPUT_FLUIDSIM> triStream)
{
    GS_OUTPUT_FLUIDSIM Out;
    // cell0.z of the first vertex in the triangle determines the destination slice index
    Out.RTIndex = In[0].cell0.z;
    for(int v=0; v<3; v++)
    {
        Out.pos          = In[v].pos; 
        Out.cell0        = In[v].cell0;
        Out.texcoords    = In[v].texcoords;
        Out.LR           = In[v].LR;
        Out.BT           = In[v].BT;
        Out.DU           = In[v].DU;
        triStream.Append( Out );
    }
    triStream.RestartStrip( );
}

[maxvertexcount (2)]
void GS_ARRAY_LINE(line VS_OUTPUT_FLUIDSIM In[2], inout LineStream<GS_OUTPUT_FLUIDSIM> Stream)
{
    GS_OUTPUT_FLUIDSIM Out;
    // cell0.z of the first vertex in the line determines the destination slice index
    Out.RTIndex = In[0].cell0.z;
    for(int v=0; v<2; v++)
    {
        Out.pos          = In[v].pos; 
        Out.cell0        = In[v].cell0;
        Out.texcoords    = In[v].texcoords;
        Out.LR           = In[v].LR;
        Out.BT           = In[v].BT;
        Out.DU           = In[v].DU;

        Stream.Append( Out );
    }
    Stream.RestartStrip( );
}

// Helper functions
float4 GetObstVelocity( float3 cellTexCoords )
{
    return Texture_obstvelocity.SampleLevel(samPointClamp, cellTexCoords, 0);
}

bool IsNonEmptyCell( float3 cellTexCoords )
{
    return (Texture_obstacles.SampleLevel(samPointClamp, cellTexCoords, 0).r <= OBSTACLE_BOUNDARY);
}

bool IsNonEmptyNonBoundaryCell( float3 cellTexCoords )
{
    float obst = Texture_obstacles.SampleLevel(samPointClamp, cellTexCoords, 0).r;
    return  (obst < OBSTACLE_BOUNDARY);
}

bool IsBoundaryCell( float3 cellTexCoords )
{
    return (Texture_obstacles.SampleLevel(samPointClamp, cellTexCoords, 0).r == OBSTACLE_BOUNDARY);
}

float3 GetAdvectedPosTexCoords(GS_OUTPUT_FLUIDSIM input)
{
    float3 pos = input.cell0;

    pos -= g_timestep * Texture_velocity.SampleLevel( samPointClamp, input.texcoords, 0 ).xyz;

    return float3(pos.x/g_textureWidth, pos.y/g_textureHeight, (pos.z+0.5)/g_textureDepth);
}

bool IsOutsideSimulationDomain( float3 cellTexcoords )
{
    if( g_treatAsLiquidVelocity == 1)
    {
        if( Texture_levelset.SampleLevel(samPointClamp, cellTexcoords, 0 ).r <= 0 )
            return false;
        else
            return true;
    }
    else
    {
        return false;
    }
}

#define SAMPLE_NEIGHBORS( texture3d, input ) \
    float L = texture3d.SampleLevel( samPointClamp, LEFTCELL, 0).r;     \
    float R = texture3d.SampleLevel( samPointClamp, RIGHTCELL, 0).r;    \
    float B = texture3d.SampleLevel( samPointClamp, BOTTOMCELL, 0).r;   \
    float T = texture3d.SampleLevel( samPointClamp, TOPCELL, 0).r;      \
    float D = texture3d.SampleLevel( samPointClamp, DOWNCELL, 0).r;     \
    float U = texture3d.SampleLevel( samPointClamp, UPCELL, 0).r;

float3 Gradient( Texture3D texture3d, VS_OUTPUT_FLUIDSIM input )
{
    SAMPLE_NEIGHBORS( texture3d, input );
    return 0.5f * float3(R - L,  T - B,  U - D);
}

float3 Gradient( Texture3D texture3d, GS_OUTPUT_FLUIDSIM input )
{
    SAMPLE_NEIGHBORS( texture3d, input );
    return 0.5f * float3(R - L,  T - B,  U - D);
}

float3 Gradient( Texture3D texture3d, GS_OUTPUT_FLUIDSIM input, out bool isBoundary, float minSlope, out bool highEnoughSlope )
{
    SAMPLE_NEIGHBORS( texture3d, input );

    float LBD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.x, input.DU.x), 0).r;
    float LBC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.x, input.texcoords.z), 0).r;
    float LBU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.x, input.DU.y), 0).r;
    float LCD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.texcoords.y, input.DU.x), 0).r;
    //float LCC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.texcoords.y, input.texcoords.z), 0).r;
    float LCU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.texcoords.y, input.DU.y), 0).r;
    float LTD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.y, input.DU.x), 0).r;
    float LTC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.y, input.texcoords.z), 0).r;
    float LTU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.x, input.BT.y, input.DU.y), 0).r;

    float CBD = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.x, input.DU.x), 0).r;
    //float CBC = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.x, input.texcoords.z), 0).r;
    float CBU = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.x, input.DU.y), 0).r;
    //float CCD = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.texcoords.y, input.DU.x), 0).r;
    float CCC = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.texcoords.y, input.texcoords.z), 0).r;
    //float CCU = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.texcoords.y, input.DU.y), 0).r;
    float CTD = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.y, input.DU.x), 0).r;
    //float CTC = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.y, input.texcoords.z)), 0).r;
    float CTU = texture3d.SampleLevel( samPointClamp, float3 (input.texcoords.x, input.BT.y, input.DU.y), 0).r;

    float RBD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.x, input.DU.x), 0).r;
    float RBC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.x, input.texcoords.z), 0).r;
    float RBU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.x, input.DU.y), 0).r;
    float RCD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.texcoords.y, input.DU.x), 0).r;
    //float RCC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.texcoords.y, input.texcoords.z), 0).r;
    float RCU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.texcoords.y, input.DU.y), 0).r;
    float RTD = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.y, input.DU.x), 0).r;
    float RTC = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.y, input.texcoords.z), 0).r;
    float RTU = texture3d.SampleLevel( samPointClamp, float3 (input.LR.y, input.BT.y, input.DU.y), 0).r;

    // is this cell next to the LevelSet boundary
    float product = L * R * B * T * D * U;
    product *= LBD * LBC * LBU * LCD * LCU * LTD * LTC * LTU
        * CBD * CBU * CTD * CTU
        * RBD * RBC * RBU * RCD * RCU * RTD * RTC * RTU;
    isBoundary = product < 0;

    // is the slope high enough 
    highEnoughSlope = (abs(R - CCC) > minSlope) || (abs(L - CCC) > minSlope) ||
        (abs(T - CCC) > minSlope) || (abs(B - CCC) > minSlope) ||
        (abs(U - CCC) > minSlope) || (abs(D - CCC) > minSlope);

    return 0.5f * float3(R - L,  T - B,  U - D);
}

// Pixel shaders
float4 PS_ADVECT_MACCORMACK( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    if( IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return 0;

    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    // get advected new position
    float3 npos = input.cell0 - g_timestep * Texture_velocity.SampleLevel( samPointClamp, input.texcoords, 0 ).xyz;
    
    // convert new position to texture coordinates
    float3 nposTC = float3(npos.x/g_textureWidth, npos.y/g_textureHeight, (npos.z+0.5)/g_textureDepth);

    // find the texel corner closest to the semi-Lagrangian "particle"
    float3 nposTexel = floor( npos + float3( 0.5f, 0.5f, 0.5f ) );
    float3 nposTexelTC = float3( nposTexel.x/g_textureWidth, nposTexel.y/g_textureHeight, nposTexel.z/g_textureDepth);

    // ht (half-texel)
    float3 ht = float3(0.5f/g_textureWidth, 0.5f/g_textureHeight, 0.5f/g_textureDepth);

    // get the values of nodes that contribute to the interpolated value
    // (texel centers are at half-integer locations)
    float4 nodeValues[8];
    nodeValues[0] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3(-ht.x, -ht.y, -ht.z), 0 );
    nodeValues[1] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3(-ht.x, -ht.y,  ht.z), 0 );
    nodeValues[2] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3(-ht.x,  ht.y, -ht.z), 0 );
    nodeValues[3] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3(-ht.x,  ht.y,  ht.z), 0 );
    nodeValues[4] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3( ht.x, -ht.y, -ht.z), 0 );
    nodeValues[5] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3( ht.x, -ht.y,  ht.z), 0 );
    nodeValues[6] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3( ht.x,  ht.y, -ht.z), 0 );
    nodeValues[7] = Texture_phi.SampleLevel( samPointClamp, nposTexelTC + float3( ht.x,  ht.y,  ht.z), 0 );

    // determine a valid range for the result
    float4 phiMin = min(min(min(nodeValues[0], nodeValues [1]), nodeValues [2]), nodeValues [3]);
    phiMin = min(min(min(min(phiMin, nodeValues [4]), nodeValues [5]), nodeValues [6]), nodeValues [7]);
    
    float4 phiMax = max(max(max(nodeValues[0], nodeValues [1]), nodeValues [2]), nodeValues [3]);
    phiMax = max(max(max(max(phiMax, nodeValues [4]), nodeValues [5]), nodeValues [6]), nodeValues [7]);

    float4 ret;
    // Perform final MACCORMACK advection step:
    // You can use point sampling and keep Texture_phi_1_hat
    //  r = Texture_phi_1_hat.SampleLevel( samPointClamp, input.texcoords, 0 )
    // OR use bilerp to avoid the need to keep a separate texture for phi_n_1_hat
    ret = Texture_phi.SampleLevel( samLinear, nposTC, 0)
        + 0.5 * ( Texture_phi.SampleLevel( samPointClamp, input.texcoords, 0 ) -
                 Texture_phi_hat.SampleLevel( samPointClamp, input.texcoords, 0 ) );

    // clamp result to the desired range
    ret = max( min( ret, phiMax ), phiMin ) * g_decay;

    if(g_advectAsTemperature)
    {
         ret -= g_temperatureLoss * g_timestep;
         ret = clamp(ret,float4(0,0,0,0),float4(5,5,5,5));
    }

    return ret;
}

float4 PS_ADVECT( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    if( IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return 0;

    float g_decayAmount = g_decay;

    if(doVelocityAttenuation)
    {
        float obstacle = Texture_obstacles.SampleLevel(samPointClamp, input.texcoords.xyz, 0).r;
        if( obstacle <= OBSTACLE_BOUNDARY) return 0;
        g_decayAmount *= clamp((obstacle-maxDensityAmount)/(1 - maxDensityAmount),0,1)*(1-maxDensityg_decay)+maxDensityg_decay;
    }
    else if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    float3 npos = GetAdvectedPosTexCoords(input);
    float4 ret = Texture_phi.SampleLevel( samLinear, npos, 0) * g_decayAmount;
    

    
    if(g_advectAsTemperature)
    {
         ret -= g_temperatureLoss * g_timestep;
         ret = clamp(ret,float4(0,0,0,0),float4(5,5,5,5));
    }

    return ret;
}

// vorticity confinement shaders
float4 PS_VORTICITY( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    float4 L = Texture_velocity.SampleLevel( samPointClamp, LEFTCELL, 0 );
    float4 R = Texture_velocity.SampleLevel( samPointClamp, RIGHTCELL, 0 );
    float4 B = Texture_velocity.SampleLevel( samPointClamp, BOTTOMCELL, 0 );
    float4 T = Texture_velocity.SampleLevel( samPointClamp, TOPCELL, 0 );
    float4 D = Texture_velocity.SampleLevel( samPointClamp, DOWNCELL, 0 );
    float4 U = Texture_velocity.SampleLevel( samPointClamp, UPCELL, 0 );

    float4 vorticity;
    // using central differences: D0_x = (D+_x - D-_x) / 2
    vorticity.xyz = 0.5 * float3( (( T.z - B.z ) - ( U.y - D.y )) ,
                                 (( U.x - D.x ) - ( R.z - L.z )) ,
                                 (( R.y - L.y ) - ( T.x - B.x )) );
                                 
    return vorticity;
}

float4 PS_CONFINEMENT( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    if( IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return 0;

    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    float4 omega = Texture_vorticity.SampleLevel( samPointClamp, input.texcoords, 0 );

    // Potential optimization: don't find length multiple times - do once for the entire texture
    float omegaL = length( Texture_vorticity.SampleLevel( samPointClamp, LEFTCELL, 0 ) );
    float omegaR = length( Texture_vorticity.SampleLevel( samPointClamp, RIGHTCELL, 0 ) );
    float omegaB = length( Texture_vorticity.SampleLevel( samPointClamp, BOTTOMCELL, 0 ) );
    float omegaT = length( Texture_vorticity.SampleLevel( samPointClamp, TOPCELL, 0 ) );
    float omegaD = length( Texture_vorticity.SampleLevel( samPointClamp, DOWNCELL, 0 ) );
    float omegaU = length( Texture_vorticity.SampleLevel( samPointClamp, UPCELL, 0 ) );

    float3 eta = 0.5 * float3( omegaR - omegaL,
                              omegaT - omegaB,
                              omegaU - omegaD );

    eta = normalize( eta + float3(0.001, 0.001, 0.001) );

    float4 force;
    force.xyz = g_timestep * g_vortConfinementScale * float3( eta.y * omega.z - eta.z * omega.y,
                                            eta.z * omega.x - eta.x * omega.z,
                                            eta.x * omega.y - eta.y * omega.x );
    
    // Note: the result is added to the current velocity at each cell using "additive blending"
    return force;
}

//diffusion shaders
float4 PS_DIFFUSE( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    float4 phi0C  = Texture_phi.SampleLevel( samPointClamp, input.texcoords, 0 );
    
    float4 phin0L = Texture_phi_next.SampleLevel( samPointClamp, LEFTCELL, 0 );
    float4 phin0R = Texture_phi_next.SampleLevel( samPointClamp, RIGHTCELL, 0 );
    float4 phin0B = Texture_phi_next.SampleLevel( samPointClamp, BOTTOMCELL, 0 );
    float4 phin0T = Texture_phi_next.SampleLevel( samPointClamp, TOPCELL, 0 );
    float4 phin0D = Texture_phi_next.SampleLevel( samPointClamp, DOWNCELL, 0 );
    float4 phin0U = Texture_phi_next.SampleLevel( samPointClamp, UPCELL, 0 );

    float dT = g_timestep;
    float v = g_viscosity;
    float dX = 1;

    float4 phin1C = ( (phi0C * dX*dX) - (dT * v * ( phin0L + phin0R + phin0B + phin0T + phin0D + phin0T )) ) / ((6 * dT * v) + (dX*dX));
    
    return phin1C;
}

//pressure projection shaders
float PS_DIVERGENCE( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    float4 fieldL = Texture_velocity.SampleLevel( samPointClamp, LEFTCELL, 0 );
    float4 fieldR = Texture_velocity.SampleLevel( samPointClamp, RIGHTCELL, 0 );
    float4 fieldB = Texture_velocity.SampleLevel( samPointClamp, BOTTOMCELL, 0 );
    float4 fieldT = Texture_velocity.SampleLevel( samPointClamp, TOPCELL, 0 );
    float4 fieldD = Texture_velocity.SampleLevel( samPointClamp, DOWNCELL, 0 );
    float4 fieldU = Texture_velocity.SampleLevel( samPointClamp, UPCELL, 0 );

    if( IsBoundaryCell(LEFTCELL) )  fieldL = GetObstVelocity(LEFTCELL);
    if( IsBoundaryCell(RIGHTCELL) ) fieldR = GetObstVelocity(RIGHTCELL);
    if( IsBoundaryCell(BOTTOMCELL) )fieldB = GetObstVelocity(BOTTOMCELL);
    if( IsBoundaryCell(TOPCELL) )   fieldT = GetObstVelocity(TOPCELL);
    if( IsBoundaryCell(DOWNCELL) )  fieldD = GetObstVelocity(DOWNCELL);
    if( IsBoundaryCell(UPCELL) )    fieldU = GetObstVelocity(UPCELL);

    float divergence =  0.5 *
        ( ( fieldR.x - fieldL.x ) + ( fieldT.y - fieldB.y ) + ( fieldU.z - fieldD.z ) );

    return divergence;
}

float PS_SCALAR_JACOBI( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    float pCenter = Texture_pressure.SampleLevel( samPointClamp, input.texcoords, 0 );
    float bC = Texture_divergence.SampleLevel( samPointClamp, input.texcoords, 0 );

    float pL = Texture_pressure.SampleLevel( samPointClamp, LEFTCELL, 0 );
    float pR = Texture_pressure.SampleLevel( samPointClamp, RIGHTCELL, 0 );
    float pB = Texture_pressure.SampleLevel( samPointClamp, BOTTOMCELL, 0 );
    float pT = Texture_pressure.SampleLevel( samPointClamp, TOPCELL, 0 );
    float pD = Texture_pressure.SampleLevel( samPointClamp, DOWNCELL, 0 );
    float pU = Texture_pressure.SampleLevel( samPointClamp, UPCELL, 0 );

    if( IsBoundaryCell(LEFTCELL) )  pL = pCenter;
    if( IsBoundaryCell(RIGHTCELL) ) pR = pCenter;
    if( IsBoundaryCell(BOTTOMCELL) )pB = pCenter;
    if( IsBoundaryCell(TOPCELL) )   pT = pCenter; 
    if( IsBoundaryCell(DOWNCELL) )  pD = pCenter;  
    if( IsBoundaryCell(UPCELL) )    pU = pCenter;

    return( pL + pR + pB + pT + pU + pD - bC ) /6.0;
}

float4 PS_APPLY_AIR_PRESSURE ( GS_OUTPUT_FLUIDSIM input) : SV_Target
{
   if( IsOutsideSimulationDomain( input.texcoords ) )
        return float4(0.0, 0.0, 0.0, 1.0);

    return float4(0.0, 0.0, 0.0, 0.0);
}

float4 PS_PROJECT( GS_OUTPUT_FLUIDSIM input ): SV_Target
{
    if( IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return 0;

    if( IsBoundaryCell(input.texcoords.xyz) )
        return GetObstVelocity(input.texcoords.xyz);

    float pCenter = Texture_pressure.SampleLevel( samPointClamp, input.texcoords, 0 ); 
    float pL = Texture_pressure.SampleLevel( samPointClamp, LEFTCELL, 0 );
    float pR = Texture_pressure.SampleLevel( samPointClamp, RIGHTCELL, 0 );
    float pB = Texture_pressure.SampleLevel( samPointClamp, BOTTOMCELL, 0 );
    float pT = Texture_pressure.SampleLevel( samPointClamp, TOPCELL, 0 );
    float pD = Texture_pressure.SampleLevel( samPointClamp, DOWNCELL, 0 );
    float pU = Texture_pressure.SampleLevel( samPointClamp, UPCELL, 0 );

    float4 velocity;
    float3 obstV = float3(0,0,0);
    float3 vMask = float3(1,1,1);
    float3 v;

    float3 vLeft = GetObstVelocity(LEFTCELL);
    float3 vRight = GetObstVelocity(RIGHTCELL);
    float3 vBottom = GetObstVelocity(BOTTOMCELL);
    float3 vTop = GetObstVelocity(TOPCELL);
    float3 vDown = GetObstVelocity(DOWNCELL);
    float3 vUp = GetObstVelocity(UPCELL);

    if( IsBoundaryCell(LEFTCELL) )  { pL = pCenter; obstV.x = vLeft.x; vMask.x = 0; }
    if( IsBoundaryCell(RIGHTCELL) ) { pR = pCenter; obstV.x = vRight.x; vMask.x = 0; }
    if( IsBoundaryCell(BOTTOMCELL) ){ pB = pCenter; obstV.y = vBottom.y; vMask.y = 0; }
    if( IsBoundaryCell(TOPCELL) )   { pT = pCenter; obstV.y = vTop.y; vMask.y = 0; }
    if( IsBoundaryCell(DOWNCELL) )  { pD = pCenter; obstV.z = vDown.z; vMask.z = 0; }
    if( IsBoundaryCell(UPCELL) )    { pU = pCenter; obstV.z = vUp.z; vMask.z = 0; }

    v = ( Texture_velocity.SampleLevel( samPointClamp, input.texcoords, 0 ).xyz -
                 (1.0f/rho)*(0.5*float3( pR - pL, pT - pB, pU - pD )) );

    velocity.xyz = (vMask * v) + obstV;

    return velocity;
}

// liquid simulation shaders
float4 PS_SIGNED_DISTANCE_TO_LIQUIDHEIGHT( GS_OUTPUT_FLUIDSIM input) : SV_Target
{
    float distance = input.cell0.z - liquidHeight;    
    return distance;
}

float4 PS_INJECT_LIQUID( GS_OUTPUT_FLUIDSIM input) : SV_Target
{
    // This is a hack to prevent the liquid from "falling through the floor" when using large g_timesteps.
    // The idea is to blend between the initial and "known" balanced state and the current level set state

    float d = input.cell0.z - liquidHeight;
    
    //float collar = 1.0; //distance *above* the liquid to reinitialize
    //if( d > collar )
    //    return 0;
   
    float amount = 0.001;
    return amount * d;
}

float4 PS_REDISTANCING(  GS_OUTPUT_FLUIDSIM input ) : SV_Target 
{
    const float dt = 0.1f;

    float phiC = Texture_phi_next.SampleLevel( samPointClamp, input.texcoords, 0).r;
  
    // avoid redistancing near boundaries, where gradients are ill-defined
    if( (input.cell0.x < 3) || (input.cell0.x > (g_textureWidth-4)) ||
        (input.cell0.y < 3) || (input.cell0.y > (g_textureHeight-4)) ||
        (input.cell0.z < 3) || (input.cell0.z > (g_textureDepth-4)) )
        return phiC;

    bool isBoundary;
    bool hasHighSlope;
    float3 gradPhi = Gradient(Texture_phi_next, input, isBoundary, 1.01f, hasHighSlope);
    float normGradPhi = length(gradPhi);

    if( isBoundary || !hasHighSlope || ( normGradPhi < 0.01f ) )
        return phiC;


    //float signPhi = phiC > 0 ? 1 : -1;
    float phiC0 = Texture_phi.SampleLevel( samPointClamp, input.texcoords, 0).r;
    float signPhi = phiC0 / sqrt( (phiC0*phiC0) + 1);
    //float signPhi = phiC / sqrt( (phiC*phiC) + (normGradPhi*normGradPhi));

    float3 backwardsPos = input.cell0 - (gradPhi/normGradPhi) * signPhi * dt;
    float3 npos = float3(backwardsPos.x/g_textureWidth, backwardsPos.y/g_textureHeight, (backwardsPos.z+0.5f)/g_textureDepth);

    return Texture_phi_next.SampleLevel( samLinear, npos, 0).r + (signPhi * dt);
}

float4 PS_EXTRAPOLATE_VELOCITY( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    const float dt = 0.25f;
    // do not modify the velocity in the current location
    if( !IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return Texture_velocity.SampleLevel( samPointClamp, input.texcoords, 0);

    // we still keep fluid velocity as 0 in cells occupied by obstacles
    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;


    float3 normalizedGradLS = normalize( Gradient(Texture_levelset, input) );
    float3 backwardsPos = input.cell0 - normalizedGradLS * dt;
    float3 npos = float3(backwardsPos.x/g_textureWidth, backwardsPos.y/g_textureHeight, (backwardsPos.z+0.5f)/g_textureDepth);

    float4 ret = Texture_velocity.SampleLevel( samLinear, npos, 0);
    
    return ret;
}

//fluid sources and external forces
float4 PS_LIQUID_STREAM( GS_OUTPUT_FLUIDSIM input, uniform bool outputColor ) : SV_Target 
{
    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    float dist = length(input.cell0 - g_center);
    
    float4 result;

    if( outputColor )
        result.rgb = g_color;
    else
        result.rgb = (dist - g_radius);
    
    if(dist < g_radius)
        result.a = 1;
    else 
        result.a = 0;

    return result;
}

float4 PS_APPLY_GRAVITY( GS_OUTPUT_FLUIDSIM input) : SV_Target
{
    if( IsOutsideSimulationDomain(input.texcoords.xyz ) )
        return 0;

    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    return float4(g_gravity, 1.0);     
}

float4 PS_GAUSSIAN( GS_OUTPUT_FLUIDSIM input ) : SV_Target 
{
    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;

    float dist = length( input.cell0 - g_center ) * g_radius;
    float4 result;
    result.rgb = g_color;    // + sin(g_color.rgb*10.0+cell*5.0)*0.2;
    result.a = exp( -dist*dist );

    return result;
}

float4 PS_COPY_TEXURE( GS_OUTPUT_FLUIDSIM input ) : SV_Target 
{
    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;
     
    float4 texCol = Texture_inDensity.SampleLevel( samLinear, input.texcoords.xy,0);
    return texCol*g_color;
}

float4 PS_ADD_DERIVATIVE_VEL( GS_OUTPUT_FLUIDSIM input ) : SV_Target 
{
    if( IsNonEmptyCell(input.texcoords.xyz) )
        return 0;
    
    float pCenter = Texture_inDensity.SampleLevel( samLinear, input.texcoords.xy, 0 ); 
    float pL = Texture_inDensity.SampleLevel( samLinear, (LEFTCELL).xy, 0 );
    float pR = Texture_inDensity.SampleLevel( samLinear, (RIGHTCELL).xy, 0 );
    float pB = Texture_inDensity.SampleLevel( samLinear, (BOTTOMCELL).xy, 0 );
    float pT = Texture_inDensity.SampleLevel( samLinear, (TOPCELL).xy, 0 );
    
    float4 vel = float4((pL-pR)*pCenter,(pB-pT)*pCenter,pCenter,1);
    
    return vel*g_color; 
}

// obstacle texture initialization
bool PointIsInsideBox(float3 p, float3 LBUcorner, float3 RTDcorner)
{
    return ((p.x > LBUcorner.x) && (p.x < RTDcorner.x)
        &&  (p.y > LBUcorner.y) && (p.y < RTDcorner.y)
        &&  (p.z > LBUcorner.z) && (p.z < RTDcorner.z));
}

struct PSDrawBoxOut
{
    float4 obstacle : SV_TARGET0;
    float4 velocity : SV_TARGET1;
};

PSDrawBoxOut PS_DYNAMIC_OBSTACLE_BOX( GS_OUTPUT_FLUIDSIM input )
{
    PSDrawBoxOut voxel;
    float3 innerobstBoxLBDcorner = g_obstBoxLBDcorner + 1;
    float3 innerobstBoxRTUcorner = g_obstBoxRTUcorner - 1;
    // cells completely inside box = 0.5
    if(PointIsInsideBox(input.cell0, innerobstBoxLBDcorner, innerobstBoxRTUcorner))
    {
        voxel.obstacle = OBSTACLE_INTERIOR;
        voxel.velocity = 0;
        return voxel;
    }

    // cells in box boundary = 1.0
    if(PointIsInsideBox(input.cell0, g_obstBoxLBDcorner, g_obstBoxRTUcorner))
    {
        voxel.obstacle = OBSTACLE_BOUNDARY;
        voxel.velocity = float4(g_obstBoxVelocity.xyz,1);
        return voxel;
    }

    voxel.obstacle = OBSTACLE_EXTERIOR;
    voxel.velocity = 0;
    return voxel;
}

PSDrawBoxOut PS_STATIC_OBSTACLE( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    PSDrawBoxOut voxel;
    voxel.obstacle = OBSTACLE_BOUNDARY;
    voxel.velocity = 0;
    return voxel;
}

float4 PS_DRAW_TEXTURE( VS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    //("Phi as density"),
    if( g_drawTextureNumber == 1)
        return float4(abs(Texture_phi.SampleLevel(samLinear,input.texcoords,0).r), 0.0f, 0.0f, 1.0f);
    //("Phi as level set"),
    else if( g_drawTextureNumber == 2)
    {
        float levelSet = Texture_phi.SampleLevel(samLinear,input.texcoords,0).r/(g_textureDepth);
        float g_color = lerp(1.0f, 0.0f, abs(levelSet));
        if( levelSet < 0 )
            return float4(0.0f,  g_color, 0.0f, 1.0f);
        return float4(g_color, 0.0f, 0.0f, 1.0f);
    }
    //("Gradient of phi"),
    else if( g_drawTextureNumber == 3)
        return float4(Gradient(Texture_phi, input), 1.0f);
    //("Velocity Field"),
    else if( g_drawTextureNumber == 4)
        return float4(abs(Texture_velocity.SampleLevel(samLinear,input.texcoords,0).xyz),1.0f);
    //("Pressure Field"),
    else if ( g_drawTextureNumber == 5)
        return float4(abs(Texture_pressure.SampleLevel(samLinear,input.texcoords,0).xyz), 1.0f);
    //("Voxelized Obstacles"),
    else
    {
        float obstColor = (Texture_obstacles.SampleLevel(samLinear,input.texcoords,0).r - OBSTACLE_INTERIOR) / (OBSTACLE_EXTERIOR - OBSTACLE_INTERIOR);
        return float4(abs(Texture_obstvelocity.SampleLevel(samLinear,input.texcoords,0).xy), obstColor, 1.0f);
    }
}