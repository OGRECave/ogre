/*
----------------------------------------------------------
Hair Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

//#include "Hair.h"
#define NUM_MRTS 8
#define NUM_PASSES 1
#define ARRAY_SIZE (NUM_PASSES * NUM_MRTS)

#define CSM_ZMAX 1.0
#define	CSM_HALF_NUM_TEX 4

#define MAX_IMPLICITS 10
#define oneOverSqrt2PI 0.39894228040143267
#define PI 3.1415926535897932384626433832795

#define SMALL_NUM 0.0001
#define EPSILON 0.0001

// These are the values used when filling the obstacle textures
#define OBSTACLE_EXTERIOR  1.0f
#define OBSTACLE_BOUNDARY  128.0f/255.0f
#define OBSTACLE_INTERIOR  0.0f

#define SHADOWS_VS

// PCF
#define PCF_RADIUS 5
#define PCF_INCREMENT 2
#define USE_ABSORPTION

//Deep Opacity Maps
#define SM_RADIUS 5
#define SM_INCREMENT 2
#define RGBA8_PRECISION_DENSITY (64.0/255.0)
#define INV_RGBA8_PRECISION_DENSITY (255.0/64.0)

#define g_NumInterpolatedAttributesMinusOne 1023
#define g_NumInterpolatedAttributes 1024

shared cbuffer constants
{
	//float g_widthMulC = 0.5;//      1;//0.5;
	float g_widthMulB = 0.5;//     1;//0.5;
	float g_widthMulGS = 0.95;//    0.8;
	float g_widthMul = 1.0; //this is for the depth prepass
	//tessellation hardware
	int g_maxPatchesPerTessellatedStrand; //g_maxPatchesPerTessellatedStrand = maxStrandLength/(strandVerticesInPatch-1). note that maxStrandLength will be differernt depending on whether or not we are using this variable for a tessellated strand vs a non-tessllated strand, and will be different based on the tessellation factor.
	float g_InterpolationLOD = 1.0;
	float fg_textureWidth =  1024.0f;
	float g_StrandWidthMultiplier = 25.0f;
	float4 g_arrowColor;
	
	//float g_kdMesh = 1;
	//float g_ksMesh = 0.1;
	//float g_specPowerMesh = 10;
	//float g_AmbientLightMesh = 0.3;

	float g_alpha = 1.0f;
	
	float g_lightBufferRes = 1.0f;
	
	//clumping
	float g_clumpWidth = 1.0f;
	float g_topWidth = 0.7;
	float g_bottomWidth = 0.05;	
	float g_lt = 0.7;
	float g_lv = 0.05;	
	
	//float       splatSize;
	//int         g_blurRadius;
	//float3      g_blurDirection;
	//float       g_blurSigma;
	//bool        g_useGradientBasedForce;
}

shared cbuffer cb0
{
	float g_ScreenWidth = 1280.0;
	float g_ScreenHeight = 1024.0;

	int g_bApplyAdditionalTransform;
	int g_bApplyAdditionalRenderingTransform;

	float g_SSg_textureWidth;
	float g_SSg_textureHeight;

	float g_ksP;
	float g_ksS;
	float g_kd;
	float g_ka;
	float g_specPowerPrimary;
	float g_specPowerSecondary;    
	float g_specPowerPrimarySparkles;
	float g_ksP_sparkles;
	float4 g_baseColor;
	float4 g_specColor;
	
	float g_fNumHairsLOD = 1;
	float g_fWidthHairsLOD = 1;	
	
	float g_maxLengthToRoot = 12; //this is the maximum length of any strand in the whole hairstyle 
	int g_useScalpTexture;
	
	int g_NumTotalWisps;
	int g_NumMaxStrandsPerWisp;
	
	float g_blendAxis; //for coordinate frame correction
	int g_doCurlyHair;
	float g_angularStiffness;
	int g_bApplyHorizontalForce = false;
	int g_bAddGravity = false;
	float g_TimeStep = 0.1; 
	float g_gravityStrength = 0.1f;
	int g_bSimulate = false;
	
	//wind
	float3 g_windForce = float3(-1,0,0);
	
	//wind fluid simulation
	int fluidg_textureWidth;
	int fluidg_textureHeight;
	int fluidg_textureDepth;
	
	//body collisions
	int g_NumSphereImplicits;
	int g_NumCylinderImplicits;
	int g_NumSphereNoMoveImplicits;
	
	//render the collision spheres
	int g_currentCollisionSphere;
	
	float g_thinning = 0.5;
	int g_TessellatedMasterStrandLengthMax;
	
	float       g_densityThreshold;
	// float       g_interHairForces; - not used
	
	float       g_textureHeight;
	float       g_textureWidth;
	float       g_textureDepth;
	
	// float       g_gridZStep;
	//float       g_gridZMin;
	//int         g_gridNumRTs;	
	
	int         g_rowWidth;
	int         g_colWidth;
	int         g_textureIndex;
	int         g_gridZIndex;
	
	int        g_useBlurTexture; // - not used
	int        g_bClearForces;
	int        g_useShadows;
	
	float g_SoftEdges; // - not used
	float g_SigmaA;
	
	float g_ZNear, g_ZFar;
}

#define SCALE_WIDTH_WITH_LOD

shared cbuffer collisionmatrixes
{
	row_major float4x4 CollisionSphereTransformations[MAX_IMPLICITS];
	row_major float4x4 CollisionSphereInverseTransformations[MAX_IMPLICITS];
	row_major float4x4 CollisionCylinderTransformations[MAX_IMPLICITS];
	row_major float4x4 CollisionCylinderInverseTransformations[MAX_IMPLICITS];

	row_major float4x4 SphereNoMoveImplicitTransform[MAX_IMPLICITS];
	row_major float4x4 SphereNoMoveImplicitInverseTransform[MAX_IMPLICITS];

	int g_UseSphereForBarycentricInterpolant[MAX_IMPLICITS];
	int g_UseCylinderForBarycentricInterpolant[MAX_IMPLICITS];
	int g_UseSphereNoMoveForBarycentricInterpolant[MAX_IMPLICITS];
}

shared cbuffer matrixes
{
	matrix ViewProjection;
	matrix WorldView;
	matrix WorldViewProjection;
	row_major float4x4 WorldToGrid;
	row_major float4x4 GridToWorld;
	row_major float4x4 RootTransformation;
	row_major float4x4 additionalTransformation;
	row_major float4x4 TotalTransformation;
	row_major float4x4 HairToWorldTransform;
	row_major float4x4 currentHairTransformation;
	row_major float4x4 currentHairTransformationInverse;
	
	float3 vLightPos;
	float3 LightPosition = float3(1, 1, 1);
	float3 EyePosition;
	float3 TransformedEyePosition;
	float3 vLightDir;   //TO DO SHADOWS: if world matrices (like HairToWorldTransform) are incorporated into shadows
						//               this vector might change to be in world space rather than hair space.
						//               in that case calculations using it will have to change, since they are currently 
						//               happening in hair space
	float3 vLightDirObjectSpace;
	
	matrix mWorldViewProj;
	matrix mLightView;
	matrix mLightProj;
	matrix mLightViewProj;
	matrix mLightViewProjI;
	matrix mLightViewProjClip2Tex;
}

//texture and buffer variables
Buffer<float2> g_Attributes : register(t0);
Buffer<float>  g_StrandSizes : register(t1);
Buffer<float2> g_StrandCircularCoordinates : register(t2);
Buffer<float>  g_StrandLengths : register(t3);
Buffer<float3> g_StrandColors : register(t4);
Buffer<float2> g_StrandCoordinates : register(t5);
Buffer<float2> g_strandDeviations : register(t6); 
Buffer<float2> g_TangentJitter : register(t7);
Buffer<float2> g_curlDeviations : register(t8);
//position, tangent, index, length, coordinate frame buffers
Buffer<float4> g_TessellatedMasterStrand : register(t9);
Buffer<float4> g_MasterStrand : register(t10);
Buffer<float4> g_OriginalMasterStrand : register(t11);
Buffer<float3> g_MasterStrandRootIndices : register(t12);
Buffer<float3> g_MasterStrandRootIndicesUntessellated : register(t13); 
Buffer<float4> g_MasterStrandLengths : register(t14);
Buffer<float4> g_MasterStrandLengthsUntessellated : register(t15);
Buffer<float3> g_OriginalMasterStrandRootIndices : register(t16);
Buffer<int>    g_tessellatedMasterStrandRootIndex : register(t17);
Buffer<float4> g_coordinateFrames : register(t18);
Buffer<float4> g_tessellatedCoordinateFrames : register(t19);
Buffer<float>  g_TessellatedLengthsToRoots : register(t20);
Buffer<float>  g_LengthsToRoots : register(t21);
Buffer<float4> g_TessellatedTangents : register(t22);
Buffer<float4> g_Forces : register(t23);
Buffer<float3> g_OriginalVectors : register(t24);
Buffer<int>    g_VertexNumber : register(t25);
Buffer<float>  g_Lengths : register(t26);
Buffer<float>    g_mStrandIndices : register(t27);
Buffer<float>    g_sStrandIndices : register(t28);
//precalculated positions and attributes
Buffer<float4> g_InterpolatedPositionAndWidth : register(t29);
Buffer<float4> g_InterpolatedIdAlphaTex : register(t30);
Buffer<float4> g_Interpolatedtangent : register(t31);
Buffer<float4> g_InterpolatedPositionAndWidthClump : register(t32);
Buffer<float4> g_InterpolatedIdAlphaTexClump : register(t33);
Buffer<float4> g_InterpolatedtangentClump : register(t34);
Buffer<int> g_SStrandsPerMasterStrandCumulative : register(t35);
Buffer<int> g_MStrandsPerWispCumulative : register(t35);

//CSM
Texture2DArray<float4> tCSM : register(t38);
//hair rendering
Texture2D hairTexture : register(t39);
Texture2DArray hairTextureArray : register(t40);
Texture2D specularNoise : register(t41);
Texture2D densityThicknessMapClump : register(t42);
Texture2D densityThicknessMapBarycentric : register(t43);
//textures
Texture2D g_CollisionsTexture : register(t44);
Texture3D g_FluidVelocityTexture : register(t45);
Texture2D g_SupersampledSceneColor : register(t46);
Texture2D g_sceneDepth : register(t47);
//interaction variables
Texture2DArray Texture_density : register(t48);
Texture2DArray Texture_density_Demux : register(t49);
Texture2DArray Texture_density_Blur_Temp : register(t50);
Texture2DArray Texture_density_Blur : register(t51);
Texture2DArray Texure_to_blur : register(t52);
Texture2DArray Texture_Voxelized_Obstacles : register(t53);
Texture2D tShadowMap : register(t54);
Texture2D tHairDepthMap : register(t59);
Texture2DArray tShadowMapArray : register(t60);

//Shading
Texture2D<float4> ShadingLookup1 : register(t54);
Texture2D<float4> ShadingLookup2 : register(t55);

Texture2D meshAOMap : register(t56);

StructuredBuffer<float4> g_MasterStrandSB : register(t57);
StructuredBuffer<float4> g_coordinateFramesSB : register(t58);

cbuffer onblend
{
    float4 g_Zmin[NUM_MRTS];
    float4 g_Zmax[NUM_MRTS];
    float4 g_Zi[ARRAY_SIZE];
    float4 g_Dz[ARRAY_SIZE];
}

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp; 
};

SamplerState samLinearWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap; 
};

SamplerState samPointClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp; 
};

SamplerComparisonState samShadow
{
	ComparisonFunc = GREATER_EQUAL;
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState samAniso
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

//    Hair simulation
//structs

struct HairVertex 
{
    float4 Position;
};

struct coordinateFrame
{
    float3 x;
	float3 y;
	float3 z;
};

struct CFVertex 
{
    float4 Position;
	float3 Color;
};

struct collisionImplicit
{
    float3 center;
	float3 rotation;
	float3 scale;
};

struct HairVertexPair
{
    float4 position : POSITION0;
    float4 oldPosition : POSITION1;        
};

struct HairVertexLength
{
    float4 Position : POSITION0;
    float Length : LENGTH0;        
};

struct HairVertexLengthStiffness
{
    float4 Position : POSITION0;
    float2 LengthStiffness : LENGTH0;        
};

struct ConstraintsHairVertex 
{
    float4 Position : POSITION;
    float Length : LENGTH;
    uint vid : ID;
};

struct ConstraintsAngularHairVertex 
{
    float4 Position : POSITION;
    float Length : LENGTH;
    float Stiffness : STIFFNESS;
    uint vid : ID;
};

struct HairVertexVID
{
    float4 Position : POSITION;
    uint vid : ID;
};

//helper functions
bool IsAnEnd(HairVertex vertex)
{
    return vertex.Position.w > 0;
}

bool IsAnEnd(float4 vertex)
{
    return vertex.w > 0;
}

bool IsFree(HairVertex particle)
{
    if(particle.Position.w >=0)
        return true;
    return false;      
}

bool IsFree(float4 particle)
{
    if(particle.w >=0)
        return true;
    return false;      
}

float2 Responsiveness(HairVertex particle0, HairVertex particle1)
{
    if (IsFree(particle0)) 
    {
        if (IsFree(particle1))
            return float2(0.5, 0.5);
        else
            return float2(1, 0);
    }
    else 
    {
        if (IsFree(particle1))
            return float2(0, 1);
        else
            return float2(0, 0);
    }
    
}

//collision sphere stuff
//assuming original obstacle was a unit radius sphere centered at origin
float3 SphereConstraint(float4 position,int i)
{
    if(!IsFree(position))
        return 0;

    //inverse transform the position
    float3 transformedPosition = (mul(float4(position.xyz,1),CollisionSphereInverseTransformations[i])).xyz;
    
    //find the force
    float3 force= float3(0,0,0);
    float len = length(transformedPosition);
    
    if(len < 1)
    {
        force = (1-len)*(transformedPosition/len);
    
        row_major float4x4 transform = CollisionSphereTransformations[i];
        //remove the translation
        transform._41 = 0;
        transform._42 = 0;
        transform._43 = 0;
        transform._44 = 1;
        force = (mul(float4(force,1),transform)).xyz;
    }

    return force;
}

float3 SphereConstraint(float4 position,row_major float4x4 InverseTransform,row_major float4x4 ForwardTransform )
{
    if(!IsFree(position))
        return 0;

    //inverse transform the position
    float3 transformedPosition = (mul(float4(position.xyz,1),InverseTransform)).xyz;
    
    //find the force
    float3 force= float3(0,0,0);
    float len = length(transformedPosition);
    
    if(len < 1)
    {
        force = (1-len)*(transformedPosition/len);
    
        row_major float4x4 transform = ForwardTransform;
        //remove the translation
        transform._41 = 0;
        transform._42 = 0;
        transform._43 = 0;
        transform._44 = 1;
        force = (mul(float4(force,1),transform)).xyz;
    }

    return force;
}

//assuming original obstacle was a unit radius sphere centered at origin
bool PartitionInOutBoundarySphere(float3 position, float boundary, int i, inout float returnVal )
{
    //inverse transform the position
    float3 transformedPosition = (mul(float4(position.xyz,1),CollisionSphereInverseTransformations[i])).xyz;

    float len = length(transformedPosition);
    float distanceToPush = 1 - len; 
    
    if(distanceToPush < 0)
    {    
        returnVal = OBSTACLE_EXTERIOR;
        return false;
    }
    if(distanceToPush > boundary)
    {    
        returnVal = OBSTACLE_INTERIOR;
        return true;
    }
    returnVal = OBSTACLE_BOUNDARY;    
    return true;
    
}

//this assumes a circle centered at the origin with a radius of 1
bool IsInsideSphere(float3 position, int i,float3 additionalScale)
{
    //inverse transform the position
    float3 transformedPosition = mul(float4(position,1),CollisionSphereInverseTransformations[i]).xyz;
    transformedPosition *= additionalScale;
    float len = length(transformedPosition);
    if(len<1)
        return true;
    return false;	             
}

//this assumes a sphere centered at the origin with a radius of 1
bool IsInsideSphere(float3 position, row_major float4x4 transform )
{
    //inverse transform the position
    float3 transformedPosition = mul(float4(position,1),transform).xyz;
    float len = length(transformedPosition);
    if(len<1)
        return true;
    return false;	             
}

//collision cylinder stuff
float3 CylinderConstraint(float4 position, int i) 
{
    if(!IsFree(position))
        return 0;
  
    //inverse transform the position
    float3 transformedPosition = (mul(float4(position.xyz,1),CollisionCylinderInverseTransformations[i])).xyz;
    
    float3 force = float3(0,0,0);
    float3 forceX = float3(0,0,0);
    float3 forceY = float3(0,0,0);
    
    if( abs(transformedPosition.y) < 0.5 )
    {
        float hmy = 0.5 - abs(transformedPosition.y);
        float cpy = transformedPosition.y;
                    
        transformedPosition.y = 0;
        float l = length(transformedPosition);
        if( l < 1)
        {
            if(cpy<0)
                forceX = float3(0,-hmy,0);
            else
                forceX = float3(0,hmy,0);      
            
            forceY = normalize(transformedPosition)*(1-l);
                
            row_major float4x4 transform = CollisionCylinderTransformations[i];
            //remove the translation
            transform._41 = 0;
            transform._42 = 0;
            transform._43 = 0;
            transform._44 = 1;
            
            forceX = (mul(float4(forceX,1),transform)).xyz;
            forceY = (mul(float4(forceY,1),transform)).xyz;
            
            if(length(forceX) < length(forceY))
                force = forceX;
            else
                force = forceY;                
        }
    }
     
    return force;
}

//with transformed cylinders the following partitions are not exactly correct
//the boundary will favor whichever axis is longer
//check if this makes any difference in the fluid sim code and change if necessary
bool PartitionInOutBoundaryCylinder(float3 position, float boundary, int i, inout float returnVal )
{
    //inverse transform the position
    float3 transformedPosition = (mul(float4(position.xyz,1),CollisionCylinderInverseTransformations[i])).xyz;
  
   
    if( abs(transformedPosition.y) < 0.5 )
    {
        float hmy = 0.5 - abs(transformedPosition.y);
        float cpy = transformedPosition.y;
                    
        transformedPosition.y = 0;
        float l = length(transformedPosition);
        if( l < 1)
        {
            
            if( hmy < 0.5 && hmy > 0 && hmy < (1-l))
            {
                if(abs(hmy)>boundary)
                {    
                    returnVal = OBSTACLE_INTERIOR;
                    return true;
                }
                else 
                {   
                    returnVal = OBSTACLE_BOUNDARY;    
                    return true;
                } 
            }  
            
            if( (1-l) > boundary)
            {    
                returnVal = OBSTACLE_INTERIOR;
                return true;
            }
            else 
            {   
                returnVal = OBSTACLE_BOUNDARY;    
                return true;
            }  
        }
    }
     
    returnVal = OBSTACLE_EXTERIOR;
    return false;
}

//this assumes a height of 1 and a radius of 1 cylinder centered at the origin and aligned along the y axis
bool IsInsideCylinder(float3 position, int i)
{
    float3 transformedPosition = mul(float4(position,1),CollisionCylinderInverseTransformations[i]).xyz;
    if( abs(transformedPosition.y) < 0.5 )
    {
        transformedPosition.y = 0;
        float l = length(transformedPosition);
        if( l < 1)
            return true;
    }
    return false;    
}

void DistanceConstraint(inout HairVertex particle0, inout HairVertex particle1, float targetDistance, float stiffness = 1.0)
{
    float3 delta = particle1.Position.xyz - particle0.Position.xyz;
    float distance = max(length(delta), 1e-7);
    float stretching = 1 - targetDistance / distance;
    delta = stretching * delta;
    float2 responsiveness = Responsiveness(particle0, particle1);
    particle0.Position.xyz += responsiveness[0] * delta * stiffness;
    particle1.Position.xyz -= responsiveness[1] * delta * stiffness;
}

//this code only works with demuxed textures, not the original density texture
float Sample2DTexArray(Texture2DArray tex, float3 posInGrid)
{
    float3 texcoords = float3(posInGrid.x, posInGrid.y, posInGrid.z * g_textureDepth);
    float density = abs(tex.SampleLevel(samLinearClamp,texcoords,0)).r; 
    return density; 
}

float3 SampleGradientTrilinear(Texture2DArray tex, float3 tc)
{
    float3 texelWidth = float3( 1.0/g_textureWidth, 1.0/g_textureHeight, 1.0/g_textureDepth );

    #define LEFTCELL    float3 (tc.x-texelWidth.x, tc.y, tc.z)
    #define RIGHTCELL   float3 (tc.x+texelWidth.x, tc.y, tc.z)
    #define BOTTOMCELL  float3 (tc.x, tc.y-texelWidth.y, tc.z)
    #define TOPCELL     float3 (tc.x, tc.y+texelWidth.y, tc.z)
    #define DOWNCELL    float3 (tc.x, tc.y, tc.z - texelWidth.z)
    #define UPCELL      float3 (tc.x, tc.y, tc.z + texelWidth.z)

    float texL = Sample2DTexArray( tex, LEFTCELL );
    float texR = Sample2DTexArray( tex, RIGHTCELL );
    float texB = Sample2DTexArray( tex, BOTTOMCELL );
    float texT = Sample2DTexArray( tex, TOPCELL );
    float texU = Sample2DTexArray( tex, UPCELL );
    float texD = Sample2DTexArray( tex, DOWNCELL );
    
    return float3(  texR - texL, texB - texT, texU - texD );
    
}

float4 visualizeDensity(float col)
{
    if(col>g_densityThreshold)
        return float4(1,0,0,1);
    col /= g_densityThreshold;    
    return float4(col.xxx,1);
}

//assumes x is already normalized, and x stays as it is
void GramSchmidtOrthoNormalize( float3 x, inout float3 y, inout float3 z )
{
	y = y - dot(x,y)*x;
	y = normalize(y);

	z = z - dot(x,z)*x - dot(y,z)*y;
	z = normalize(z);
}

//Vertex shaders
//add angular spring forces
HairVertexVID AddAngularForcesVS(HairVertex input , uint vertexID : SV_VertexID)
{
    HairVertexVID output;
    output.Position = input.Position;
    output.vid = vertexID;
    return output;
}

void addObstacleCollisionResponse(inout float4 position)
{
    int i;
    
    //spheres
    for(i=0;i<g_NumSphereImplicits;i++)
    {    position.xyz += SphereConstraint(position,i); }

    //cylinders
    for(i=0;i<g_NumCylinderImplicits;i++)
    {    position.xyz += CylinderConstraint(position,i); } 

}

//add forces to move hair out of obstacles
void SatisfyCollisionConstraintsVS(inout HairVertex input)
{
    addObstacleCollisionResponse(input.Position);          
}

//add forces and do verlet integration
HairVertex addForcesAndIntegrateVS(HairVertexPair particlePair, uint vertexID : SV_VertexID)
{  
    HairVertex outputPos;
    outputPos.Position = particlePair.position;
    
    if(!IsFree(particlePair.position))     
    { 
        //if this is a root, transform it by the transform of the scalp
        outputPos.Position.xyz = mul(float4( outputPos.Position.xyz, 1), RootTransformation).xyz;
        return outputPos; 
    }
    
    if(!g_bSimulate)
        return outputPos;

	//transform all the vertices if needed
	if(g_bApplyAdditionalTransform)
	{
	    particlePair.position.xyz = mul(float4( particlePair.position.xyz, 1), additionalTransformation).xyz;
	    particlePair.oldPosition.xyz = mul(float4( particlePair.oldPosition.xyz, 1), additionalTransformation).xyz;
	}

    float4 originalPosition = g_OriginalMasterStrand.Load(vertexID);    
    float4 transformedPos = float4((mul(float4(originalPosition.xyz,1),currentHairTransformation)).xyz,originalPosition.w);  
    addObstacleCollisionResponse(transformedPos);          
    
    float4 force = 0;
    force = g_Forces.Load( vertexID ); 
    
    float internalSpeedCoefficient =        force.w*(0.05-0.02)+0.02;         
    
    float3 velocity = (1-internalSpeedCoefficient)*(particlePair.position.xyz - particlePair.oldPosition.xyz);
    
    float stiffness = force.w;
    
    //if velocity is very low increase the stiffness
    if( length(velocity)<0.01 )    
    {
        stiffness = clamp(force.w*5.0,0,1);
        float stiffnessIncrease = stiffness - force.w;
        if( stiffnessIncrease > 0 )
            force *= stiffnessIncrease*6;  
    }
    if( length(velocity)>0.5) //if velocity is very high decrease the stiffness
        force *= 0.5;
    
    
    float gravityStrength =   (1-stiffness)*(1.2-0.07)+0.07;              
   
    gravityStrength *= g_gravityStrength; 
     
    //add wind force
    if( g_bApplyHorizontalForce)
    {
        float4 posInGrid = mul(float4(outputPos.Position.xyz, 1), WorldToGrid );
        posInGrid.x += 0.5;
        posInGrid.y += 0.5;
        posInGrid.z += 0.5;
        float3 texcoords = float3(posInGrid.x, 1.0-posInGrid.y, posInGrid.z);
        float3 g_windForce = (g_FluidVelocityTexture.SampleLevel(samLinearClamp,texcoords,0).xyz);
        g_windForce *= (1-stiffness)*(5) + 5;
        force.xyz += g_windForce;
    }
    
    //Gravity------------------------------------------------------------------------
    if( g_bAddGravity)
        force.xyz += float3(0, -gravityStrength, 0);
    
    //original curve attractor-------------------------------------------------------
    /*commenting out, this didnt work so nicely with the rest of the forces
    float3 vectorToOriginal = originalPosition - particlePair.position;
    float lengthToOriginal = length(vectorToOriginal);
    if( lengthToOriginal>1.0)
        vectorToOriginal /= lengthToOriginal;     
    force.xyz +=  vectorToOriginal*0.07;
    */     
     
    //no move constraint-- keep the hair off the face----------------------------------
    //sphere no move constraint  
    //only do this to hair that are not very stiff already 
    
    if(stiffness<0.9) 
    {
        for(int i=0;i<g_NumSphereNoMoveImplicits;i++)    
            if( IsInsideSphere(particlePair.position.xyz,SphereNoMoveImplicitInverseTransform[i] ) )
            {
    
                float3 vectorToOriginal = transformedPos.xyz - particlePair.position.xyz;
                float lengthToOriginal = length(vectorToOriginal);
                if( lengthToOriginal>1.0)
                   vectorToOriginal /= lengthToOriginal;     
           
                force.xyz +=  vectorToOriginal*0.525; 
            }
    }  
             
    //verlet integration
    //float clampAmount = 1.0;
    //velocity.xyz = clamp(velocity.xyz, float3(-clampAmount,-clampAmount,-clampAmount), float3(clampAmount,clampAmount,clampAmount) );
    outputPos.Position.xyz = particlePair.position.xyz
                             + velocity.xyz
                             + force.xyz*g_TimeStep*g_TimeStep; 

    //this makes stiff hair even stiffer, and makes it go towards its base transformed pose.
	//However, we only apply this force when the hair and the base transformed hair are "relatively" close. Here we have huristically decided this value to be 1.6 units
    float staticky = force.w*0.0475;
    if(length(outputPos.Position.xyz - transformedPos.xyz) < 1.6 )
	    outputPos.Position.xyz = (1-staticky)*outputPos.Position.xyz + staticky*transformedPos.xyz;
    
    return outputPos;  
    
}

//satisfy linear spring constraints
ConstraintsHairVertex SatisfySpringConstraintsVS(HairVertexLength input, uint vertexID : SV_VertexID)
{
    ConstraintsHairVertex output;
    output.Position = input.Position;
    output.vid = vertexID;
    output.Length = input.Length;
    return output;
}

ConstraintsAngularHairVertex SatisfyAngularSpringConstraintsVS(HairVertexLengthStiffness input, uint vertexID : SV_VertexID)
{
    ConstraintsAngularHairVertex output;
    output.Position = input.Position;
    output.vid = vertexID;
    output.Length = input.LengthStiffness.x;
    output.Stiffness = input.LengthStiffness.y;
    return output;
}

void restoreToDefaultPositionsVS(inout HairVertex input, uint vertexID : SV_VertexID)
{
    float4 originalPosition = g_OriginalMasterStrand.Load(vertexID);
    input.Position = originalPosition; 
}

//geometry shaders
float3 TransformOldVectorToNewVector( int index, float3 oldVector) 
{
	float3 vM1X = g_coordinateFrames.Load(index*3).xyz;

    if(length(vM1X)<0.01) return oldVector;
	
	float3 vM1Y = g_coordinateFrames.Load(index*3+1).xyz;
	float3 vM1Z = g_coordinateFrames.Load(index*3+2).xyz;
	
	float3 worldX = float3(1,0,0);
	float3 worldY = float3(0,1,0);
	float3 worldZ = float3(0,0,1);

    row_major float4x4 vM1ToWorld = float4x4
    (
        dot(worldX,vM1X), dot(worldY,vM1X), dot(worldZ,vM1X), 0,
        dot(worldX,vM1Y), dot(worldY,vM1Y), dot(worldZ,vM1Y), 0,
        dot(worldX,vM1Z), dot(worldY,vM1Z), dot(worldZ,vM1Z), 0,
		0,                0,                0,                1     

    );

    //transform from old to new coordinate frame
    float3 retVector = mul(float4(oldVector,1),vM1ToWorld).xyz;
	return retVector;
}

[MaxVertexCount(2)]
void AddAngularForcesGS(line HairVertexVID vertex[2], inout PointStream<HairVertex> stream)
{
    HairVertex vertex0,vertex1;
    vertex0.Position = vertex[0].Position;
    vertex1.Position = vertex[1].Position;

    //if g_ClearForces is set, first clear the forces, and then add to them
    if(g_bClearForces)
    {    
        vertex0.Position.xyz = float3(0,0,0); 
        vertex1.Position.xyz = float3(0,0,0);        
    }
    
    if( vertex[0].vid == vertex[1].vid )
    {
        stream.Append(vertex0);
        return;
    }

    float3 bPOriginal = g_OriginalVectors.Load(vertex[0].vid);
    
    if(length(bPOriginal)==0) //this should never happen
    {
        stream.Append(vertex0);
        stream.Append(vertex1); 
        return;    
    }
    
    float4 positionV1 = g_MasterStrand.Load(vertex[0].vid);
    float4 positionV2 = g_MasterStrand.Load(vertex[1].vid);

    float3 bP;

	//transform the original vector from the local coordinate frame of vertex v-1 to the world coordinate frame
	bP = TransformOldVectorToNewVector(vertex[0].vid-1,bPOriginal);
    //testing bP = TransformOldVectorToNewVector(vertex[0].vid,float3(1,0,0));
    
    float k = vertex0.Position.w*9 + 1; //minimum stiffness is 1 and maximum is 10; empirically adjusted
    
	float3 b = positionV2.xyz - positionV1.xyz;

	float lengthbP = length(bP);
	float lengthB = length(b);
    float dotB_Bp = dot(bP,b);
	float constant = k/(2*lengthbP*lengthB);
	float3 bDoubleNorm = b/(lengthB*lengthB);

	vertex0.Position.xyz += constant * ( -bP + dotB_Bp*bDoubleNorm );
	vertex1.Position.xyz += constant * (  bP - dotB_Bp*bDoubleNorm );

    
    stream.Append( vertex0 );
    stream.Append( vertex1 );
}

[MaxVertexCount(2)]
void SatisfySpringConstraintsGS(line ConstraintsHairVertex vertex[2], inout PointStream<HairVertex> stream)
{
    HairVertex vertex0,vertex1;
    vertex0.Position = vertex[0].Position;
    vertex1.Position = vertex[1].Position;
    
    if( vertex[0].vid == vertex[1].vid )
    {
        stream.Append(vertex0);
        return;
    }
    
    DistanceConstraint(vertex0,vertex1,vertex[0].Length);
    stream.Append( vertex0 );
    stream.Append( vertex1 );
}

[MaxVertexCount(4)]
void SatisfyAngularSpringConstraintsGS(lineadj ConstraintsAngularHairVertex vertex[4], inout PointStream<HairVertex> stream)
{
    HairVertex vertex0,vertex1,vertex2,vertex3;
    vertex0.Position = vertex[0].Position;
    vertex1.Position = vertex[1].Position;
    vertex2.Position = vertex[2].Position;
    vertex3.Position = vertex[3].Position;
    
    bool firstIsReal = false;
    bool secondIsReal = false;
    bool thirdIsReal = false;
      
    if( vertex[2].vid != vertex[1].vid )   
    {
         secondIsReal = true;
         float stiffness = vertex[0].Stiffness * g_angularStiffness;
         if( stiffness>0 )
             DistanceConstraint(vertex0,vertex2, vertex[0].Length, stiffness );
    }
    if( vertex[1].vid != vertex[0].vid )
    {
        firstIsReal = true;
        if( vertex[3].vid != vertex[2].vid )
        {
             thirdIsReal = true;
             float stiffness = vertex[1].Stiffness * g_angularStiffness;
             if( stiffness>0 )
                 DistanceConstraint(vertex1,vertex3, vertex[1].Length, stiffness );
        }        
    }
    
    stream.Append( vertex0 );
    if( firstIsReal )
        stream.Append( vertex1 );
    if( secondIsReal )   
        stream.Append( vertex2 );
    if( thirdIsReal )        
        stream.Append( vertex3 );
}

//coordinate frame update and rendering
struct coordinateFrame4
{
    float4 xAxis : X_AXIS;
	float4 yAxis : Y_AXIS;
	float4 zAxis : Z_AXIS;
};

struct coordinateFrame4x4
{
	coordinateFrame4 m_Arr[4];
};

struct coordinateFrameAndID
{
    float3 xAxis : X_AXIS;
	float3 yAxis : Y_AXIS;
	float3 zAxis : Z_AXIS;
	uint v   : VERTEX_ID;
};

struct coordinateFrameRender
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

void rotateVector(float3 rotationAxis, float theta, float3 prevVec, inout float3 newVec)
{
	float3 axisDifference = rotationAxis - prevVec;
    if( length(axisDifference)<SMALL_NUM || theta < SMALL_NUM )
	{
	    newVec = prevVec;
		return;
	}

	float c = cos(theta);
    float s = sin(theta);
	float t = 1 - c;
    float x = rotationAxis.x;
	float y = rotationAxis.y;
	float z = rotationAxis.z;

	row_major float4x4 rotationMatrix = float4x4 ( t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
	                                               t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
							                       t*x*z - s*y, t*y*z + s*x, t*z*z + c  , 0,
							                       0          , 0          , 0          , 1);
	
    newVec = mul( float4(prevVec,1),rotationMatrix).xyz;

}

coordinateFrame4 UpdateCoordinateFrameVS(coordinateFrame4 cf, uint vertexID : SV_VertexID)
{
    if(length(cf.xAxis)<0.001)
        return cf; //this is the cf at the end and its supposed to be zero; no update needed

    //find the x axis
    coordinateFrame4 output;
    float4 vertex1 = g_MasterStrand.Load(vertexID);
    float4 vertex2 = g_MasterStrand.Load(vertexID+1);    
    output.xAxis.xyz = g_blendAxis*normalize(vertex2.xyz - vertex1.xyz) + (1-g_blendAxis)*cf.xAxis.xyz;
    output.xAxis.xyz = normalize(output.xAxis.xyz);
    output.xAxis.w = 0;
    
    //find the rotation  
    float3 xRotationVector = cross( output.xAxis.xyz, normalize(cf.xAxis.xyz));
    float sinTheta = length(xRotationVector);
    float theta = asin(sinTheta);
	xRotationVector = normalize(xRotationVector);
	
	//find the y and z axis
	float3 y,z;	
 	rotateVector(xRotationVector, theta, cf.yAxis.xyz, y);
    rotateVector(xRotationVector, theta, cf.zAxis.xyz, z);
    y = normalize(y);
    z = normalize(z);			  
    
	/* we could also do this for extra assurance
	GramSchmidtOrthoNormalize(output.xAxis.xyz, y, z );
	*/
	
	//assign y and z axis
	output.yAxis.xyz = y;
	output.zAxis.xyz = z;
	output.yAxis.w = output.zAxis.w = 0;

    return output;
}

coordinateFrame4 PropagateCoordinateFrameVS(coordinateFrame4 cf, uint vertexID : SV_VertexID)
{
    if(length(cf.xAxis)<0.01)
        return cf; //this is the cf at the end and its supposed to be zero; no update needed

    //load the previous vertex's coordinate frame
    int prevID = vertexID-1;
    if(prevID == 0)
        return cf;
    
    
    prevID = vertexID-1;
    
    coordinateFrame4 prevCF;
    prevCF.xAxis = g_coordinateFrames.Load(prevID*3 );
    if(length(prevCF.xAxis)<0.01)
        return cf; //this is the root and it does not need update

    prevCF.yAxis = g_coordinateFrames.Load(prevID*3 + 1); 
    prevCF.zAxis = g_coordinateFrames.Load(prevID*3 + 2);
    
    coordinateFrame4 output;
    
    //x axis
    // assume this is already correct in the existing CF. this only works if called after update CF
    output.xAxis = cf.xAxis;
    
    //find the rotation  
    float3 xRotationVector = cross( output.xAxis.xyz, normalize(prevCF.xAxis.xyz));
    float sinTheta = length(xRotationVector);
    float theta = asin(sinTheta);
	xRotationVector = normalize(xRotationVector);

	//find the y and z axis
	float3 y,z;	
 	rotateVector(xRotationVector, theta, prevCF.yAxis.xyz, y);
    rotateVector(xRotationVector, theta, prevCF.zAxis.xyz, z);
    y = normalize(y);
    z = normalize(z);			  
    
	//assign y and z axis
	output.yAxis.xyz = y;
	output.zAxis.xyz = z;
	output.yAxis.w = output.zAxis.w = 0;

    return output; 
}

coordinateFrameAndID RenderCoordinateFrameVS(coordinateFrame4 cf, uint vertexID : SV_VertexID)
{
     coordinateFrameAndID output;
 
     output.xAxis = cf.xAxis.xyz;
     output.yAxis = cf.yAxis.xyz;
     output.zAxis = cf.zAxis.xyz;
 
     output.v = vertexID; 
     return output;
}

coordinateFrameAndID RenderCoordinateFrameVSUnTess(uint vertexID : SV_VertexID)
{
     coordinateFrameAndID output;
 
     output.xAxis = g_coordinateFrames.Load(vertexID*3).xyz;
     output.yAxis = g_coordinateFrames.Load(vertexID*3+1).xyz;
     output.zAxis = g_coordinateFrames.Load(vertexID*3+2).xyz;
 
     output.v = vertexID; 
     return output;
}

coordinateFrameAndID RenderCoordinateFrameVSUnTessSB(uint vertexID : SV_VertexID)
{
     coordinateFrameAndID output;
 
     output.xAxis = g_coordinateFramesSB[vertexID*3].xyz;
     output.yAxis = g_coordinateFramesSB[vertexID*3+1].xyz;
     output.zAxis = g_coordinateFramesSB[vertexID*3+2].xyz;
 
     output.v = vertexID; 
     return output;
}

void RenderCoordinateFrameAux(point coordinateFrameAndID cf[1], float4 vertex, inout LineStream<coordinateFrameRender> stream)
{
    coordinateFrameRender p;
    
    float axisScale = 0.2;
        
	//x-axis
	p.Position = vertex;
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(1,0,0);
	stream.Append(p);
	
	p.Position = float4(vertex.x + axisScale*cf[0].xAxis.x ,vertex.y + axisScale*cf[0].xAxis.y, vertex.z + axisScale*cf[0].xAxis.z, 1);
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(1,0,0);
	stream.Append(p);
    stream.RestartStrip();    
		
	//y-axis
	p.Position = vertex;
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(0,1,0);
	stream.Append(p);
	
	p.Position = float4(vertex.x + axisScale*cf[0].yAxis.x ,vertex.y + axisScale*cf[0].yAxis.y, vertex.z + axisScale*cf[0].yAxis.z, 1);
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(0,1,0);
	stream.Append(p);
    stream.RestartStrip();    

	//z-axis
	p.Position = vertex;
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(0,0,1);
	stream.Append(p);
	p.Position = float4(vertex.x + axisScale*cf[0].zAxis.x ,vertex.y + axisScale*cf[0].zAxis.y, vertex.z + axisScale*cf[0].zAxis.z, 1);
	p.Position = mul( float4(p.Position.xyz,1),ViewProjection);
	p.Color = float3(0,0,1);
	stream.Append(p);
    stream.RestartStrip();   
}

[MaxVertexCount(6)]
void RenderCoordinateFrameGSUnTessellated(point coordinateFrameAndID cf[1], inout LineStream<coordinateFrameRender> stream)
{
    //load the vertex point
    float4 vertex = g_MasterStrand.Load(cf[0].v);
    RenderCoordinateFrameAux(cf,vertex,stream);
}

[MaxVertexCount(6)]
void RenderCoordinateFrameGSUnTessellatedSB(point coordinateFrameAndID cf[1], inout LineStream<coordinateFrameRender> stream)
{
    //load the vertex point
     float4 vertex = g_MasterStrandSB[cf[0].v];
    RenderCoordinateFrameAux(cf,vertex,stream);
}

[MaxVertexCount(6)]
void RenderCoordinateFrameGSTessellated(point coordinateFrameAndID cf[1], inout LineStream<coordinateFrameRender> stream)
{
    //load the vertex point
    float4 vertex = g_TessellatedMasterStrand.Load(cf[0].v);
    RenderCoordinateFrameAux(cf,vertex,stream);
}

float4 RenderCoordinateFramePS(coordinateFrameRender input) : SV_Target
{
    return float4(input.Color,1); 
}

//    Hair rendering
//structs
struct HairAdjacencyVertex
{
    unsigned int controlVertexIndex : controlVertexIndex;
    float u : u;
};

struct HairVertex2
{
   float3 Position : Position;
   float2 scalpTexcoords : SCALPTEX;
   float shadow : SHADOW;
   float width : Width;
   float3 Tangent : Tangent;
   float tex : TEXALONGLENGTH;
   float dummy : MYDUMMY; // workaround for the bug in compiler (August 2009 SDK). does not compile without it
   int ID : ID;
};

struct HairVertexPWI
{
   float3 Position : Position;
   float width : Width;
   int ID : ID;
};

struct InterpolatedSOBarycentricVertex 
{
    float4 PositionAndWidth : PositionAndWidth;
    float4 IdAlphaTex : IdAlphaTex;
    float4 tangent : tangent;    
};
struct InterpolatedSOBarycentricVertexPosWidth 
{
    float4 PositionAndWidth : PositionAndWidth;
};
struct InterpolatedSOBarycentricVertexIDAlphaTex 
{
    float4 IdAlphaTex : IdAlphaTex;
};
struct InterpolatedSOBarycentricVertexTangent 
{
    float4 tangent : tangent;    
};

struct HairVertexPWIAT
{
   float3 Position : Position;
   float width : Width;
   int ID : ID;
   float tex : TEXALONGLENGTH;
};

struct HairCollisionVertex
{
    float4 Position : SV_Position; 
    float3 BarycenticPosition : BPos;
	float dummy : DUMMY; // added due to bug in compiler
    int vertexID : VertexID; 
};

struct HairCollisionVertexWithGS
{
    float4 Position : P; 
    float3 BarycenticPosition : BPos;
	float dummy : DUMMY; // added due to bug in compiler
    int vertexID : VertexID; 
};

struct HairVertexCollisions
{
   float3 Position : Position;
   int vertexID : vertexID;
   nointerpolation float2 texcoords : TEXCOORDS;
   int ID : ID;
}; 

struct HairVertex2Grid
{
   float3 Position : Position;
   int ID : ID;
};

struct HairPoint 
{
    float4 Position : SV_Position;
    float3 Tangent : TANGENT;
    float shadow : SHADOW;
    nointerpolation float2 scalpTexcoords : SCALP_TEXCOORDS; //these texcoords stay the same over the length of the hair and are not interpolated. 
    float2 tex: TEXTUREACROSSSTRAND;
    nointerpolation int ID : ID;
#ifndef SHADOWS_VS
    float3 wPos : WorldPosition;
#endif
};

struct HairPoint_LINES 
{
    float4 Position : SV_Position;
    float3 Tangent : TANGENT;
    float shadow : SHADOW;
    nointerpolation float2 scalpTexcoords : SCALP_TEXCOORDS; //these texcoords stay the same over the length of the hair and are not interpolated. 
    nointerpolation int ID : ID;     
   // float3 wPos : WorldPosition;
};

struct HairPointDepthPrepass
{
    float4 Position : SV_Position;
    float3 tex: TEXTUREACROSSSTRAND;
};

struct HairPointDepth 
{
    float4 Position : SV_Position;
    float depth : DEPTH;
};

struct HairPointDepthDensity 
{
    float4 Position : SV_Position;
    float4 posInGrid : POS_IN_GRID;
};

struct HairPointDepthSO 
{
    float4 Position : SV_Position;
    float3 wPos : WorldPosition;
    float4 Positionl : Positionl;
    float4 WICL : WICL;
    float4 texcoords : texcoords;
};   

struct HairPointCollisions
{
    float4 Position : SV_Position;
    int vertexID : vertexID;
    nointerpolation float2 texcoords : TEXCOORDS;
};

struct HairPointGrid 
{
    float4 Position : SV_Position;
    float distance  : DIST; 
};

struct simpleInPS
{
    float4 pos : SV_Position;
    float3 col : COLOR0;
};

//vertex shaders
void Passthrough(inout HairVertex vertex)
{
}

//tessellation
void TessellateVSBSplinePassThrough(inout HairAdjacencyVertex input)
{

}

//create coordinate frames for the tessellated hair
coordinateFrame4 TessellateVSBSplineCFs(HairAdjacencyVertex input , uint vertexID : SV_VertexID)
{ 
    coordinateFrame4 output;
    
    float4x4 basisMatrix = float4x4
    (
        -1/6.0,  3/6.0, -3/6.0,  1/6.0,
         3/6.0, -6/6.0,  3/6.0,  0,
        -3/6.0,  0,      3/6.0,  0,
         1/6.0,  4/6.0,  1/6.0,  0
    );
    
    float u = input.u;
    coordinateFrame4x4 controlCFs;
    
    if(u<0)
    {   
        u += 2;
        //controlCFs.m_Arr[1].xAxis = g_coordinateFrames.Load(input.controlVertexIndex*3 );
        controlCFs.m_Arr[1].yAxis = g_coordinateFrames.Load(input.controlVertexIndex*3 + 1); 
        controlCFs.m_Arr[1].zAxis = g_coordinateFrames.Load(input.controlVertexIndex*3 + 2);
               
        //controlCFs.m_Arr[2].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 );
        controlCFs.m_Arr[2].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 1 );
        controlCFs.m_Arr[2].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 2 );        
        
        //controlCFs.m_Arr[3].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 );
        controlCFs.m_Arr[3].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 1 );
        controlCFs.m_Arr[3].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 2 );
                        
        controlCFs.m_Arr[0] = controlCFs.m_Arr[1];       
    }
    else if(u>1)
    {
        u -= 2;
        //controlCFs.m_Arr[0].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3);
        controlCFs.m_Arr[0].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 1);
        controlCFs.m_Arr[0].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 2);                
        
        //controlCFs.m_Arr[1].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3);
        controlCFs.m_Arr[1].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 1); 
        controlCFs.m_Arr[1].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 2);
        
        //controlCFs.m_Arr[2].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3);
        controlCFs.m_Arr[2].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3 + 1); 
        controlCFs.m_Arr[2].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3 + 2);                
        
        controlCFs.m_Arr[3] = controlCFs.m_Arr[2];       
    }
    else
    {
          //controlCFs.m_Arr[0].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+0)*3);
          controlCFs.m_Arr[0].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+0)*3 + 1);
          controlCFs.m_Arr[0].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+0)*3 + 2);

          //controlCFs.m_Arr[1].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3);
          controlCFs.m_Arr[1].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 1);
          controlCFs.m_Arr[1].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+1)*3 + 2);

          //controlCFs.m_Arr[2].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3);
          controlCFs.m_Arr[2].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 1);
          controlCFs.m_Arr[2].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+2)*3 + 2);

          //controlCFs.m_Arr[3].xAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3);
          controlCFs.m_Arr[3].yAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3 + 1);
          controlCFs.m_Arr[3].zAxis = g_coordinateFrames.Load((input.controlVertexIndex+3)*3 + 2);

    }
    
    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);
    float3 x,y,z;
    
    x = normalize( (g_TessellatedMasterStrand.Load(vertexID+1)).xyz - (g_TessellatedMasterStrand.Load(vertexID)).xyz);
    
    int c;
    
    y = float3(0,0,0);
    y += basis[0] * controlCFs.m_Arr[0].yAxis.xyz;
    y += basis[1] * controlCFs.m_Arr[1].yAxis.xyz;
    y += basis[2] * controlCFs.m_Arr[2].yAxis.xyz;
    y += basis[3] * controlCFs.m_Arr[3].yAxis.xyz;

        
    z = float3(0,0,0);
    z += basis[0] * controlCFs.m_Arr[0].zAxis.xyz;
    z += basis[1] * controlCFs.m_Arr[1].zAxis.xyz;
    z += basis[2] * controlCFs.m_Arr[2].zAxis.xyz;
    z += basis[3] * controlCFs.m_Arr[3].zAxis.xyz;
        
   GramSchmidtOrthoNormalize( x, y, z );

   output.xAxis.xyz = x;
   output.yAxis.xyz = y;
   output.zAxis.xyz = z;
   output.xAxis.w = output.yAxis.w = output.zAxis.w = 0;


   return output;      
   
}

//create coordinate frames for the tessellated hair
coordinateFrame4 TessellateVSBSplineSBCFs(HairAdjacencyVertex input , uint vertexID : SV_VertexID)
{ 
    coordinateFrame4 output;
    
    float4x4 basisMatrix = float4x4
    (
        -1/6.0,  3/6.0, -3/6.0,  1/6.0,
         3/6.0, -6/6.0,  3/6.0,  0,
        -3/6.0,  0,      3/6.0,  0,
         1/6.0,  4/6.0,  1/6.0,  0
    );
    
    float u = input.u;
    coordinateFrame4x4 controlCFs;
    
    if(u<0)
    {   
        u += 2;
        //controlCFs.m_Arr[1].xAxis = g_coordinateFramesSB[input.controlVertexIndex*3 ];
        controlCFs.m_Arr[1].yAxis = g_coordinateFramesSB[input.controlVertexIndex*3 + 1]; 
        controlCFs.m_Arr[1].zAxis = g_coordinateFramesSB[input.controlVertexIndex*3 + 2];
               
        //controlCFs.m_Arr[2].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 ];
        controlCFs.m_Arr[2].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 1 ];
        controlCFs.m_Arr[2].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 2 ];        
        
        //controlCFs.m_Arr[3].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 ];
        controlCFs.m_Arr[3].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 1 ];
        controlCFs.m_Arr[3].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 2 ];
                        
        controlCFs.m_Arr[0] = controlCFs.m_Arr[1];       
    }
    else if(u>1)
    {
        u -= 2;
        //controlCFs.m_Arr[0].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3];
        controlCFs.m_Arr[0].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 1];
        controlCFs.m_Arr[0].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 2];                
        
        //controlCFs.m_Arr[1].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3];
        controlCFs.m_Arr[1].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 1]; 
        controlCFs.m_Arr[1].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 2];
        
        //controlCFs.m_Arr[2].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3];
        controlCFs.m_Arr[2].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3 + 1]; 
        controlCFs.m_Arr[2].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3 + 2];                
        
        controlCFs.m_Arr[3] = controlCFs.m_Arr[2];       
    }
    else
    {
          //controlCFs.m_Arr[0].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+0)*3];
          controlCFs.m_Arr[0].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+0)*3 + 1];
          controlCFs.m_Arr[0].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+0)*3 + 2];

          //controlCFs.m_Arr[1].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3];
          controlCFs.m_Arr[1].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 1];
          controlCFs.m_Arr[1].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+1)*3 + 2];

          //controlCFs.m_Arr[2].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3];
          controlCFs.m_Arr[2].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 1];
          controlCFs.m_Arr[2].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+2)*3 + 2];

          //controlCFs.m_Arr[3].xAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3];
          controlCFs.m_Arr[3].yAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3 + 1];
          controlCFs.m_Arr[3].zAxis = g_coordinateFramesSB[(input.controlVertexIndex+3)*3 + 2];

    }
    
    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);
    float3 x,y,z;
    
    x = normalize( (g_TessellatedMasterStrand.Load(vertexID+1)).xyz - (g_TessellatedMasterStrand.Load(vertexID)).xyz);
    
    int c;
    
    y = float3(0,0,0);
    y += basis[0] * controlCFs.m_Arr[0].yAxis.xyz;
    y += basis[1] * controlCFs.m_Arr[1].yAxis.xyz;
    y += basis[2] * controlCFs.m_Arr[2].yAxis.xyz;
    y += basis[3] * controlCFs.m_Arr[3].yAxis.xyz;

        
    z = float3(0,0,0);
    z += basis[0] * controlCFs.m_Arr[0].zAxis.xyz;
    z += basis[1] * controlCFs.m_Arr[1].zAxis.xyz;
    z += basis[2] * controlCFs.m_Arr[2].zAxis.xyz;
    z += basis[3] * controlCFs.m_Arr[3].zAxis.xyz;
        
   GramSchmidtOrthoNormalize( x, y, z );

   output.xAxis.xyz = x;
   output.yAxis.xyz = y;
   output.zAxis.xyz = z;
   output.xAxis.w = output.yAxis.w = output.zAxis.w = 0;

   return output;
}

struct LengthVertex
{
    float Length : LENGTH;
};

LengthVertex TessellateVSBSplineLengths(HairAdjacencyVertex input)
{ 
    LengthVertex output;
    output.Length = 0;

    float4x4 basisMatrix = float4x4
    (
        -1/6.0,  3/6.0, -3/6.0,  1/6.0,
         3/6.0, -6/6.0,  3/6.0,  0,
        -3/6.0,  0,      3/6.0,  0,
         1/6.0,  4/6.0,  1/6.0,  0
    );
    
    float u = input.u;
    float4 controlVertexLengths;
    
    if(u<0)
    {   
        u += 2;
        controlVertexLengths[0] = controlVertexLengths[1] = g_LengthsToRoots.Load(input.controlVertexIndex);
        controlVertexLengths[2] = g_LengthsToRoots.Load(input.controlVertexIndex + 1);
        controlVertexLengths[3] = g_LengthsToRoots.Load(input.controlVertexIndex + 2);        
    }
    else if(u>1)
    {
        u -= 2;
        controlVertexLengths[0] = g_LengthsToRoots.Load(input.controlVertexIndex + 1);
        controlVertexLengths[1] = g_LengthsToRoots.Load(input.controlVertexIndex + 2);
        controlVertexLengths[2] = controlVertexLengths[3] = g_LengthsToRoots.Load(input.controlVertexIndex + 3);        
    }
    else
    {
          controlVertexLengths[0] = g_LengthsToRoots.Load(input.controlVertexIndex );
          controlVertexLengths[1] = g_LengthsToRoots.Load(input.controlVertexIndex + 1);
          controlVertexLengths[2] = g_LengthsToRoots.Load(input.controlVertexIndex + 2);
          controlVertexLengths[3] = g_LengthsToRoots.Load(input.controlVertexIndex + 3);
    }
      
    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);
    [unroll] for (int c = 0; c < 4; ++c) 
        output.Length += basis[c] * controlVertexLengths[c];

    return output;      
}

//interpolation
int findClosestIndex(float3 coords)
{
    int closestIndex = 0;

    if(coords.y > coords.x)
    {
        if(coords.z > coords.y )
            closestIndex = 2;
        else
            closestIndex = 1;
    }
    else
    {   
        if(coords.z > coords.x )
            closestIndex = 2;
        else
            closestIndex = 0;
    }
    return closestIndex;
}

//render the interpolated vertices into a texture to detect which of them are intersecting the collision implicits
//and what the smallest vertexID is for each strand that does intersect
//normal multistrand interpolation, no clumping
HairVertexCollisions InterpolateVSMCollisions( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    HairVertexCollisions output;
    
    float fIndex = g_mStrandIndices.Load(InstanceID);
    int index = floor(fIndex);
    float3 masterStrandRoots = g_MasterStrandRootIndices.Load(index); 
    float4 lengths = g_MasterStrandLengths.Load( index );

    //if we have run over hair lengths
    if( (int)vertexID >= lengths.w )
    {    
        output.ID = -1;
        return output;
    }
    else
        output.ID = InstanceID;
              
    float3 coords;
    coords.xy = g_StrandCoordinates.Load(output.ID & g_NumInterpolatedAttributesMinusOne);
    coords.z = (1 - coords.x - coords.y);
   
    int3 vertexIndices;
    vertexIndices[0] = floor(masterStrandRoots.x) + vertexID;
    vertexIndices[1] = floor(masterStrandRoots.y) + vertexID;
    vertexIndices[2] = floor(masterStrandRoots.z) + vertexID;
    
    float3 lengthsToRoots;
    lengthsToRoots[0] = g_TessellatedLengthsToRoots.Load( vertexIndices[0] );
    lengthsToRoots[1] = g_TessellatedLengthsToRoots.Load( vertexIndices[1] );
    lengthsToRoots[2] = g_TessellatedLengthsToRoots.Load( vertexIndices[2] ); 
    
    
    //barycentric position---------------------------------------------------------------------   
    float3x4 masterVertexPosition;
    masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
    masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
    masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
    float3 BarycenticPosition =   coords.x  * masterVertexPosition[0].xyz
                      + coords.y  * masterVertexPosition[1].xyz
                      + coords.z  * masterVertexPosition[2].xyz;

    float BarycetricLengthToRoot =  coords.x  * lengthsToRoots[0]
                                  + coords.y  * lengthsToRoots[1]
                                  + coords.z  * lengthsToRoots[2];
     
    output.Position = BarycenticPosition;
    
    float fullYPixel = 1.0/g_NumTotalWisps;
    float fullXPixel = 1.0/g_NumMaxStrandsPerWisp;
    int interpHairNumber = frac(fIndex)*1000; //note: check that 1000 is defined in other places and checked accordingly
    output.texcoords.x = interpHairNumber*fullXPixel + fullXPixel/2.0;
    output.texcoords.y = 1.0 - (index*fullYPixel + fullYPixel/2.0);
    
    output.vertexID = vertexID;
       
    return output;
} 

//shadows
float4 k_v(const float k)
{
	return float4(k, k+1, k+2, k+3);
}

float4 Ck_v(const float k) 
{
	return PI * (2.0 * k_v(k) - 1.0);
}

float Absorption(float2 texcoord, float z, int2 offset)
{
	float z0 = tShadowMap.SampleLevel(samPointClamp, texcoord, 0, offset).x;
	if( z >= z0 )
	{
	    float d = max(z - z0, 0);
	    return 1 - exp(g_SigmaA * d);
	    //float d = clamp(g_SigmaA*(z0-z),0,1);
	   // return d;//1 - d;
	}
	return 0;
}

float DOMContribution(float z, float2 texcoord, int2 offset)
{
	float shadow = 0;
	
	//read the shadow map depth
	//float z0 = tHairDepthMap.SampleLevel(samPointClamp, texcoord, 0, offset);
	
	//read the DOM
	float4 opacitySlices = tShadowMap.SampleLevel(samPointClamp, texcoord, 0, offset);
	//the starting depth is embeded in the alpha channel
	float z0 = opacitySlices.w;
	opacitySlices *= INV_RGBA8_PRECISION_DENSITY * 2.0;
	opacitySlices.w = 0;
	
	//compute contribution from all the layers (IMPORTANT! assuming only one texture with four slices) 
	float4 W = max(0, 1.0 - abs(g_Zi[0] + z0 - z) / g_Dz[0]);
	shadow += dot(W, opacitySlices);	
		
//	if(z > g_Zi[0].w + z0) shadow = opacitySlices.z; 
		
	return min(1.0f,shadow);
	
}

float ShadowDOM(float2 texcoord, float z)
{
	float shadow = 0;
	float n = 0;
	[unroll]for (int dx = -SM_RADIUS; dx <= SM_RADIUS; dx+=SM_INCREMENT) 
	{ 
		[unroll]for (int dy = -SM_RADIUS; dy <= SM_RADIUS; dy+=SM_INCREMENT) 
		{ 
			shadow += DOMContribution(z,texcoord,int2(dx,dy));
			n++;
		}
	}
	
	shadow /= n;
    //shadow = exp(-shadow*g_SigmaA);	
	return shadow;	
}

float ShadowPCF(float2 texcoord, float z)
{
	float shadow = 0;
	float n = 0;
	[unroll]for (int dx = -PCF_RADIUS; dx <= PCF_RADIUS; dx+=PCF_INCREMENT) { 
		[unroll]for (int dy = -PCF_RADIUS; dy <= PCF_RADIUS; dy+=PCF_INCREMENT) { 

#ifdef USE_ABSORPTION
			shadow += Absorption(texcoord, z, int2(dx, dy));
#else
			shadow += tShadowMap.SampleCmpLevelZero(samShadow, texcoord, z, int2(dx, dy));
#endif
			n++;
		}
	}
	return shadow / n;
}

float NormalizeDepth(float z)
{
	return (z - g_ZNear) / (g_ZFar - g_ZNear) * CSM_ZMAX;
}

float WorldToDepth(float3 wPos)
{
	float z = mul(float4(wPos, 1), mLightView).z;
#ifdef USE_CSM	
	return NormalizeDepth(z);
#endif
    return z;
    	
}

float2 WorldToLightCoord(float3 wPos)
{
	return mul(float4(wPos, 1), mLightViewProjClip2Tex).xy;
}

//MULTISTRAND INTERPOLATION
//vertex shaders for multistrand interpolation with collision detection 
//vertex shaders for rendering, depth pre-pass and shadows

//determine position, width, ID
HairVertexPWI InterpolateVSCore0( uint InstanceID, uint vertexID, 
inout int vertexIndices0_output, inout int vertexIndices1_output, inout int vertexIndices2_output,
inout float3 coords_output, inout int closestIndex_output, 
inout float barycentricWeight_output, inout float clumpWeight_output, inout float2 texcoords_output,
inout float3 originalMasterStrandRoots_output, inout float lengthToRoot_output, inout float lengthsw_output)
{
    HairVertexPWI output = (HairVertexPWI)0;

    float fIndex = g_mStrandIndices.Load(InstanceID);
    int index = floor(fIndex);

    float3 masterStrandRoots = g_MasterStrandRootIndices.Load(index); 
    float4 lengths = g_MasterStrandLengths.Load( index ); //these are the lengths in number of vertices from root
    float3 originalMasterStrandRoots = g_OriginalMasterStrandRootIndices.Load(index);
    
    //if we have run over hair lengths
    if( (int)vertexID >= lengths.w )
    {    
        output.ID = -1;
        return output;
    }
    output.ID = InstanceID;

    float3 coords;
    coords.xy = g_StrandCoordinates.Load(output.ID & g_NumInterpolatedAttributesMinusOne);
    coords.z = (1 - coords.x - coords.y);
 
    //texcoords
    float3x2 rootTexcoords;
    rootTexcoords[0] = g_Attributes.Load( floor(originalMasterStrandRoots.x));
    rootTexcoords[1] = g_Attributes.Load( floor(originalMasterStrandRoots.y));
    rootTexcoords[2] = g_Attributes.Load( floor(originalMasterStrandRoots.z));       
    float2 BaryCentricTexcoords =   coords.x  * rootTexcoords[0].xy
                                  + coords.y  * rootTexcoords[1].xy
                                  + coords.z  * rootTexcoords[2].xy;

    //sample the density map, and if we are over the density dont create anymore hair
    float4 hairDensityThickness = densityThicknessMapBarycentric.SampleLevel(samLinear,BaryCentricTexcoords,0);  

    int3 vertexIndices;
    vertexIndices[0] = floor(masterStrandRoots.x) + vertexID;
    vertexIndices[1] = floor(masterStrandRoots.y) + vertexID;
    vertexIndices[2] = floor(masterStrandRoots.z) + vertexID;

    //length to root
    //needed for clumping, tapering the ends and for texture mapping along the strand
    float3 lengthsToRoots;
    lengthsToRoots[0] = g_TessellatedLengthsToRoots.Load( vertexIndices[0] );
    lengthsToRoots[1] = g_TessellatedLengthsToRoots.Load( vertexIndices[1] );
    lengthsToRoots[2] = g_TessellatedLengthsToRoots.Load( vertexIndices[2] ); 

    //barycentric position  
    float3x4 masterVertexPosition;
    masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
    masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
    masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
    float3 BarycenticPosition =   coords.x  * masterVertexPosition[0].xyz
                      + coords.y  * masterVertexPosition[1].xyz
                      + coords.z  * masterVertexPosition[2].xyz;

   float clumpWeight=0;
   float barycentricWeight = 1.0;
 
   int closestIndex =  findClosestIndex(coords);
   
    //if the hair strand below intersects an obstacle clamp to nearest guide hair
    
    float fullYPixel = 1.0/g_NumTotalWisps;
    float fullXPixel = 1.0/g_NumMaxStrandsPerWisp;
    int interpHairNumber = frac(fIndex)*100;
    float2 texcoordsCollision;
    texcoordsCollision.x = interpHairNumber*fullXPixel + fullXPixel/2.0;
    texcoordsCollision.y = 1.0 - (index*fullYPixel + fullYPixel/2.0);
    float collisions = g_CollisionsTexture.SampleLevel(samPointClamp,texcoordsCollision,0).r;
    int buffer = 60; //TODO: where did this come from?
    float2 texcoords;

    float BarycetricLengthToRoot =  coords.x  * lengthsToRoots[0]
                                  + coords.y  * lengthsToRoots[1]
                                  + coords.z  * lengthsToRoots[2];
    float lengthToRoot;

#if 1
	if(collisions < (float)(vertexID + buffer) )
    {
 		float2 clumpCoordinates = g_StrandCircularCoordinates.Load(output.ID & g_NumInterpolatedAttributesMinusOne); // this is completely wrong, but it does not lead to a big visual problem. THESE COORDINATES HAVE NOTHING IN COMMON WITH THE OTHER COORDINATES THAT WE HAVE FOR MULTISTRAND. IE BOTH HAIR ARE COMPLETELT DIFFERENT
       	float clumpLengthToRoot = lengthsToRoots[closestIndex];
        float2 ClumpTexcoords = rootTexcoords[closestIndex];

		//load the coordinate frames
		coordinateFrame4 cf;
		cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 1);
		cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 2);
	    
		//create the new position for the hair clump vertex
		float radius = g_clumpWidth * (g_topWidth*(1-clumpLengthToRoot) + g_bottomWidth*clumpLengthToRoot);
		float3 ClumpPosition = masterVertexPosition[closestIndex].xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;
       
        clumpWeight = (vertexID - collisions + buffer)/float(buffer);
        clumpWeight = clamp(clumpWeight,0,1);
        clumpWeight = sin((clumpWeight-0.5)*PI)*0.5 + 0.5;

	    barycentricWeight = 1.0-clumpWeight;
        output.Position = BarycenticPosition*barycentricWeight + ClumpPosition*clumpWeight;
        texcoords = BaryCentricTexcoords*barycentricWeight + ClumpTexcoords*clumpWeight;
        lengthToRoot = BarycetricLengthToRoot*barycentricWeight + clumpLengthToRoot*clumpWeight;
    }
    else
#endif
    {
        output.Position = BarycenticPosition;
        texcoords = BaryCentricTexcoords;
        lengthToRoot = BarycetricLengthToRoot;
    }
    
    if(vertexID==0) //rooting the interpolated hair
    {
        float3 xAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 ).xyz;
        output.Position.xyz -= 0.25*xAxis; 
    }
        
    //tapering the hair---------------------------------------------------------------------------
    //modulating width by the width denoted in the densityThickness texture-----------------------
    output.width = hairDensityThickness.r;
    float lt = lerp( g_lt, g_lt-0.2, hairDensityThickness.r );    
    if(lengthToRoot > lt)
        output.width *= ( 1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt) );
    
       
    vertexIndices0_output = vertexIndices[0];
    vertexIndices1_output = vertexIndices[1];
    vertexIndices2_output = vertexIndices[2];
    coords_output         = coords;
    closestIndex_output   = closestIndex; 
    barycentricWeight_output = barycentricWeight;
    clumpWeight_output    = clumpWeight;
    texcoords_output      = texcoords;
    originalMasterStrandRoots_output = originalMasterStrandRoots;
    lengthToRoot_output = lengthToRoot;
    lengthsw_output = lengths.w;
     
    return output;
}

//determine position, width, ID, alpha and tex
HairVertexPWIAT InterpolateVSCore( uint InstanceID, uint vertexID, 
inout int vertexIndices0_output, inout int vertexIndices1_output, inout int vertexIndices2_output,
inout float3 coords_output, inout int closestIndex_output, 
inout float barycentricWeight_output, inout float clumpWeight_output, inout float2 texcoords_output)
{
   
    //get the position, width and ID, and related values
    HairVertexPWI pwi = (HairVertexPWI)0;
    int3 vertexIndices;
    vertexIndices[0] = vertexIndices[1] = vertexIndices[2] = 0;
    float3 coords = float3(0,0,0);
    int closestIndex = 0;
    float barycentricWeight = 0;
    float clumpWeight = 0;
    float2 texcoords = float2(0,0);
    float3 originalMasterStrandRoots = float3(0,0,0);
    float lengthToRoot = 0;
    float lengthsw = 0;
    pwi = InterpolateVSCore0(InstanceID,vertexID,vertexIndices[0],vertexIndices[1],vertexIndices[2],coords,closestIndex,barycentricWeight,clumpWeight,texcoords,originalMasterStrandRoots,lengthToRoot,lengthsw); 


    HairVertexPWIAT output;
    output.Position = pwi.Position;
    output.width = pwi.width;
    output.ID = pwi.ID;
    output.tex = 0;    
    if(output.ID < 0 )
        return output;

       
    float3 totalLengths;
    totalLengths[0] = g_Lengths.Load( originalMasterStrandRoots.x );
    totalLengths[1] = g_Lengths.Load( originalMasterStrandRoots.y );
    totalLengths[2] = g_Lengths.Load( originalMasterStrandRoots.z ); 
                                      
    float BarycentricTotalLength =   coords.x  * totalLengths[0]
                                   + coords.y  * totalLengths[1]
                                   + coords.z  * totalLengths[2];
	float clumpTotalLength = totalLengths[closestIndex];
    float totalLength =     BarycentricTotalLength*barycentricWeight + clumpTotalLength*clumpWeight;

    //this texture goes from 0 to 1 for the longest hair in the hairstyle.
    //lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
    //totalLength is the actual total length of the strand
    //g_maxLengthToRoot is the total length of the longest strand
    output.tex = lengthToRoot*totalLength/g_maxLengthToRoot;
   
    
    //thinning the hair lengths---------------------------------------------------------------------
    float inLengthFrac = g_StrandLengths.Load(output.ID & g_NumInterpolatedAttributesMinusOne);
    float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;
    
    if( ( int(lengthToRoot*lengthsw) > int(maxLength*lengthsw) + 1 ) || ((int)vertexID > int(lengthsw) - 1) )
    {    output.ID = -1;
         return output;
    }
    if( (lengthToRoot > maxLength) || ((int)vertexID == int(lengthsw)-1) )
        output.width = EPSILON;    
       
        
    vertexIndices0_output = vertexIndices[0];
    vertexIndices1_output = vertexIndices[1];
    vertexIndices2_output = vertexIndices[2];
    coords_output         = coords;
    closestIndex_output   = closestIndex; 
    barycentricWeight_output = barycentricWeight;
    clumpWeight_output    = clumpWeight;
    texcoords_output      = texcoords; 
        
    return output;
}

HairVertexPWIAT InterpolateVSDepthPrepass( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    int vertexIndices[3] = {0,0,0};
    float3 coords = 0;
    int closestIndex = 0;
    float barycentricWeight = 0, clumpWeight = 0;
    float2 texcoords = 0;
    return InterpolateVSCore(InstanceID,vertexID,vertexIndices[0],vertexIndices[1],vertexIndices[2],coords,closestIndex,barycentricWeight,clumpWeight,texcoords);
}

HairVertexPWI InterpolateVSBMultiStrandDepthShadows( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    int vertexIndices[3];
    float3 coords;
    int closestIndex;
    float barycentricWeight;
    float clumpWeight;
    float2 texcoords;
    float3 originalMasterStrandRoots;
    float lengthToRoot;
    float lengthsw;
    return InterpolateVSCore0(InstanceID,vertexID,vertexIndices[0],vertexIndices[1],vertexIndices[2],coords,closestIndex,barycentricWeight,clumpWeight,texcoords,originalMasterStrandRoots,lengthToRoot,lengthsw); 
}

HairVertex2 InterpolateVSBarycentricTotalCore( uint InstanceID, uint vertexID )
{
    HairVertexPWIAT outputDepth;
    int vertexIndices[3];
    vertexIndices[0] = vertexIndices[1] = vertexIndices[2] = 0;
    float3 coords = float3(0,0,0);
    int closestIndex = 0;
    float barycentricWeight = 0;
    float clumpWeight = 0;
    float2 texcoords = float2(0,0);
    outputDepth = InterpolateVSCore(InstanceID,vertexID,vertexIndices[0],vertexIndices[1],vertexIndices[2],coords,closestIndex,barycentricWeight,clumpWeight,texcoords);
    
    HairVertex2 output = (HairVertex2)0;
    output.Position = outputDepth.Position;
    output.width = outputDepth.width;
    output.ID = outputDepth.ID;
    output.tex = outputDepth.tex;
    output.Tangent = float3(0,0,0);
    output.scalpTexcoords = 0;
    output.shadow = 0;
    if(output.ID < 0 )
        return output;
    
    float3x4 masterVertexTangents;
    masterVertexTangents[0] = g_TessellatedTangents.Load( vertexIndices[0] );
    masterVertexTangents[1] = g_TessellatedTangents.Load( vertexIndices[1] );
    masterVertexTangents[2] = g_TessellatedTangents.Load( vertexIndices[2] ); 
    float3 BarycentricTangent =  coords.x  * masterVertexTangents[0].xyz
                               + coords.y  * masterVertexTangents[1].xyz
                               + coords.z  * masterVertexTangents[2].xyz;
    float3 ClumpTangent = masterVertexTangents[closestIndex].xyz; //NOTE: we are loading from float3x4 - why are we doing a .xyz qualification here?
    output.Tangent =  BarycentricTangent*barycentricWeight +     ClumpTangent*clumpWeight;
    output.Tangent = normalize(output.Tangent);

    output.scalpTexcoords = texcoords;
 
    return output;
}

struct HS_CONTROL_POINT_OUTPUT_INTERP_B
{
	float3 tessellatedPosition : POSITION; 
	float4 tessellatedTangent : TANGENT;
	float tessellatedLengthToRoot : LENGTHTOROOT;
};

struct HS_CONTROL_POINTDEPTH_OUTPUT_INTERP_B
{
	float3 tessellatedPosition : POSITION;
	float tessellatedLengthToRoot : LENGTHTOROOT;
};

struct HS_CONTROL_POINTCOLLISIONS_OUTPUT_INTERP_B
{
	float3 tessellatedPosition : POSITION;
};

//this is a version of multistrand interpolation that takes all the vertex information as given, but does calculations only for a collisions, which needs only position and ID
HairCollisionVertex InterpolateVSBarycentricCollisions_withInput(HS_CONTROL_POINTCOLLISIONS_OUTPUT_INTERP_B vertex0, HS_CONTROL_POINTCOLLISIONS_OUTPUT_INTERP_B vertex1, HS_CONTROL_POINTCOLLISIONS_OUTPUT_INTERP_B vertex2, float4 lengths, float totalLengths[3], float2 rootTexcoords0, float2 rootTexcoords1, float2 rootTexcoords2, uint vertexID, uint InstanceID)
{
    HairCollisionVertex output = (HairCollisionVertex)0;
    float fIndex = g_mStrandIndices.Load(InstanceID);
    int index = floor(fIndex);

    //check if we are over the hair length, or if we are culled due to density--------------------------------------
    //if we have run over hair lengths, cull
    if( (float)vertexID >= lengths.w || vertexID < 2)
    {   
        output.Position = float4(0,0,-1000,1.0);    
        return output;
    }

    int ID = InstanceID; 

    float3 coords;
    coords.xy = g_StrandCoordinates.Load(ID & g_NumInterpolatedAttributesMinusOne);
    coords.z = (1 - coords.x - coords.y);

    //y: the wisp number
    float fullYPixel = 2.0/g_NumTotalWisps;
    float yPos = -1 + index*fullYPixel + fullYPixel/2.0;
    
    //x: the interpolated hair number 
    int interpHairNumber = frac(fIndex)*100;
    float fullXPixel = 2.0/g_NumMaxStrandsPerWisp;
    float xPos = -1 + interpHairNumber*fullXPixel + fullXPixel/2.0;
   
    //barycentric position---------------------------------------------------  
    float3 BarycenticPosition =   coords.x  * vertex0.tessellatedPosition.xyz
                                + coords.y  * vertex1.tessellatedPosition.xyz
                                + coords.z  * vertex2.tessellatedPosition.xyz;

    output.Position = float4(xPos,yPos,0.5,1.0);
    output.BarycenticPosition = BarycenticPosition;
    output.vertexID = vertexID;

    return output;
}

HairVertex2 InterpolateVSMultiStrand( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    return InterpolateVSBarycentricTotalCore(InstanceID,vertexID);
}

InterpolatedSOBarycentricVertex InterpolateVSMultiStrand_SO(uint vertexID : SV_VertexID )
{
   HairVertex2 hairvert = InterpolateVSBarycentricTotalCore
   (vertexID/g_TessellatedMasterStrandLengthMax,vertexID%g_TessellatedMasterStrandLengthMax);
     
    InterpolatedSOBarycentricVertex output;
    output.PositionAndWidth = float4(hairvert.Position,hairvert.width);
    output.IdAlphaTex = float4(asfloat(hairvert.ID),1,hairvert.tex,hairvert.scalpTexcoords.x);
    output.tangent = float4(hairvert.Tangent,hairvert.scalpTexcoords.y);
    return output;    
}

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[2] : SV_TessFactor;
	float2 texcoords : CCC;
	float totalLength : DDD;
	float hairDensity : EEE;
	int nHairsBefore : HAIRS_BEFORE;
	int tessellatedMasterStrandRootIndex : BBB;
	int MasterStrandLength : AAA;
};
struct BARY_HS_CONSTANT_DATA_OUTPUT
{
    float Edges[2] : SV_TessFactor;
	int nHairsBefore : AAA;
	float3 masterStrandRoots : BBB;
	float lengthsw : DDD;
	float3x2 rootTexcoords : FFF;
	float3 totalLengths : GGG;
};
struct BARYDEPTH_HS_CONSTANT_DATA_OUTPUT
{
    float Edges[2] : SV_TessFactor;
	int nHairsBefore : AAA;
	float3 masterStrandRoots : BBB;
	float lengthsw : DDD;
	float3x2 rootTexcoords : FFF;
};
struct BARYCOLLISIONS_HS_CONSTANT_DATA_OUTPUT
{
    float Edges[2] : SV_TessFactor;
	int nHairsBefore : AAA;
	float3 masterStrandRoots : BBB;
};

static const int NHAIRS_PER_PATCH = 64;
int g_iFirstPatchHair;
// if length of hair is more than 64 verts, we are rendering it using several patches (subhairs)
int g_iSubHairFirstVert; // this is index of first subshair vertex inside hair
static const int NSEGMENTS_PER_PATCH = 64; // this is number of subhair segments

struct DUMMY
{
	float dummy : DUMMY;
};

DUMMY InterpolateVS_DUMMY11()
{
	DUMMY dummy;
	dummy.dummy = 0;
	return dummy;
}

HairVertex2 InterpolateVSMultiStrand_LOAD(uint index : SV_VertexID )
{
    HairVertex2 output = (HairVertex2)0;
        
    float4 PositionAndWidth = g_InterpolatedPositionAndWidth.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTex.Load(index);
    float4 tangent = g_Interpolatedtangent.Load(index);
        
    
    output.Position = PositionAndWidth.xyz;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;

    output.scalpTexcoords = float2(IdAlphaTex.w,tangent.w);
    
    #ifdef SHADOWS_VS
    if(g_useShadows)
    {
        float3 Pos = mul(float4(output.Position.xyz, 1), HairToWorldTransform).xyz;
	    float2 texcoords = WorldToLightCoord(Pos);
	    float z = WorldToDepth(Pos);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex float2 texcoords = WorldToLightCoord(output.Position.xyz);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView float z = WorldToDepth(output.Position.xyz);
	    output.shadow = ShadowPCF(texcoords, z);
    }
    else
        output.shadow = 0;
    #endif
        
    output.width = PositionAndWidth.w;
    output.Tangent = tangent.xyz;
    output.ID = asint(IdAlphaTex.x);
    output.tex = IdAlphaTex.z;
    return output;
}

HairVertexPWI InterpolateVSMultiStrandDepthShadows_LOAD(uint index : SV_VertexID)
{
    HairVertexPWI output;
    
    float4 PositionAndWidth = g_InterpolatedPositionAndWidth.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTex.Load(index);
    
    output.Position = PositionAndWidth.xyz;
	if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;

    output.width = PositionAndWidth.w;
    output.ID = asint(IdAlphaTex.x);
    
    return output;
}

HairVertexPWIAT InterpolateVSDepthPrepass_LOAD(uint index : SV_VertexID )
{
    HairVertexPWIAT output;
    float4 PositionAndWidth = g_InterpolatedPositionAndWidth.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTex.Load(index);
    
    output.Position = PositionAndWidth.xyz;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;
    output.width = PositionAndWidth.w;
    output.ID = asint(IdAlphaTex.x);
    output.tex = IdAlphaTex.z ;
                
    return output;
}

HairPoint HairRenderInterpolatedCoreVS( uint vertexID, uint InstanceID, 
Buffer<float4> InterpolatedPositionAndWidth, Buffer<float4> InterpolatedIdAlphaTex, Buffer<float4> Interpolatedtangent, float widthMult)
{

    int index;
    bool odd = false;
    if((vertexID&1)==0)
    {
        odd = false;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + int(vertexID/2);
    }
    else
    {
        odd = true;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + int((vertexID-1)/2);
    }
    
    float4 PositionAndWidth = InterpolatedPositionAndWidth.Load(index);
    float4 IdAlphaTex = InterpolatedIdAlphaTex.Load(index);
    float4 vTangent = Interpolatedtangent.Load(index);
    
    HairPoint hairPoint = (HairPoint)0;
    hairPoint.scalpTexcoords = float2(IdAlphaTex.w,vTangent.w);
    
    #ifdef SHADOWS_VS
    if(g_useShadows)
    {
        float3 Pos = mul(float4(PositionAndWidth.xyz, 1), HairToWorldTransform).xyz;
	    float2 texcoords = WorldToLightCoord(Pos);
	    float z = WorldToDepth(Pos);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex float2 texcoords = WorldToLightCoord(output.Position.xyz);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView float z = WorldToDepth(output.Position.xyz);
	    hairPoint.shadow = ShadowPCF(texcoords, z);
	}
    else
        hairPoint.shadow = 0;
    #endif


    int ID = asint(IdAlphaTex.x);
    hairPoint.ID = ID;
    
    float strandWidth = g_StrandSizes.Load(ID & g_NumInterpolatedAttributesMinusOne) * widthMult;
    float3 worldPos =  PositionAndWidth.xyz; 
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, vTangent.xyz));
    hairPoint.Tangent = vTangent.xyz;

    float3 pos;
    if(!odd)
    {
        pos = worldPos - sideVec * 0.5 * strandWidth * PositionAndWidth.w;
        hairPoint.tex = float2(0,IdAlphaTex.z);
    }
    else
    {
       pos = worldPos + sideVec * 0.5 * strandWidth * PositionAndWidth.w;
       hairPoint.tex = float2(1,IdAlphaTex.z);
    }
    
#ifndef SHADOWS_VS
    hairPoint.wPos = pos;
#endif
    hairPoint.Position = mul( float4(pos,1.0),ViewProjection);
    
          
    return hairPoint;

}      

HairPointDepth HairShadowsInterpolatedCoreVS( uint vertexID, uint InstanceID, Buffer<float4> InterpolatedPositionAndWidth, 
Buffer<float4> InterpolatedIdAlphaTex, Buffer<float4> Interpolatedtangent)
{
    int index;
    bool odd = false;
    if((vertexID&1)==0)
    {
        odd = false;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + vertexID/2;
    }
    else
    {
        odd = true;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + (vertexID-1)/2;
    }    
    
    float4 PositionAndWidth = InterpolatedPositionAndWidth.Load(index);
    float4 vTangent = Interpolatedtangent.Load(index);
    int ID = InstanceID; 
    
    float strandWidth = g_StrandSizes.Load(ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulB;
    float3 worldPos =  PositionAndWidth.xyz; 
    float3 tangent = vTangent.xyz; //using the tangent of the vertex rather than calculating v1-v0
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));

    float3 pos;
    if(!odd)
        pos = worldPos - sideVec * 0.5 * strandWidth * PositionAndWidth.w;
    else
        pos = worldPos + sideVec * 0.5 * strandWidth * PositionAndWidth.w;

    float3 Pos = mul(float4(pos, 1), HairToWorldTransform).xyz;
    HairPointDepth hairPoint;
    hairPoint.depth = mul(float4(Pos, 1), mLightView).z;
    #ifdef USE_CSM	
	    hairPoint.depth = NormalizeDepth(hairPoint.depth);
    #endif    
    hairPoint.Position = mul(float4(Pos, 1), ViewProjection);
    
    return hairPoint;
}

HairPointDepthPrepass HairDepthPrepassInterpolatedCoreVS( uint vertexID, uint InstanceID, Buffer<float4> InterpolatedPositionAndWidth, 
Buffer<float4> InterpolatedIdAlphaTex, Buffer<float4> Interpolatedtangent) 
{
    int index;
    bool odd = false;
    if((vertexID&1)==0)
    {
        odd = false;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + int(vertexID/2);
    }
    else
    {
        odd = true;
        index = InstanceID*g_TessellatedMasterStrandLengthMax + int((vertexID-1)/2);
    }
    
    float4 PositionAndWidth = InterpolatedPositionAndWidth.Load(index);
    float4 IdAlphaTex = InterpolatedIdAlphaTex.Load(index);
    float4 vTangent = Interpolatedtangent.Load(index);
    
    HairPointDepthPrepass hairPoint = (HairPointDepthPrepass)0;
    int ID = asint(IdAlphaTex.x);
    
    float strandWidth = g_StrandSizes.Load(ID & g_NumInterpolatedAttributesMinusOne) * g_widthMul;
    float3 worldPos =  PositionAndWidth.xyz; 
    float3 tangent = vTangent.xyz; //using the tangent of the vertex rather than calculating v1-v0
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));

    float3 pos;
    if(!odd)
    {
        pos = worldPos - sideVec * 0.5 * strandWidth * PositionAndWidth.w;
        hairPoint.tex = float3(0,IdAlphaTex.z,ID & 3);
    }
    else
    {
       pos = worldPos + sideVec * 0.5 * strandWidth * PositionAndWidth.w;
       hairPoint.tex = float3(1,IdAlphaTex.z,ID & 3);
    }
    
    hairPoint.Position = mul( float4(pos,1.0),ViewProjection);
          
    return hairPoint;

}

HairCollisionVertex InterpolateVSMultiStrandCollisions( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    HairCollisionVertex output = (HairCollisionVertex)0;
    float fIndex = g_mStrandIndices.Load(InstanceID);
    int index = floor(fIndex);
    
    
    //check if we are over the hair length, or if we are culled due to density--------------------------------------
    float3 masterStrandRoots = g_MasterStrandRootIndices.Load(index); 
    float4 lengths = g_MasterStrandLengths.Load( index ); //these are the lengths in number of vertices from root
    float3 originalMasterStrandRoots = g_OriginalMasterStrandRootIndices.Load(index);
    //if we have run over hair lengths, cull
    if( (float)vertexID >= lengths.w || vertexID < 2)
    {   
        output.Position = float4(0,0,-1000,1.0);    
        return output;
    }                
    int ID = InstanceID; 
    float3 coords;
    coords.xy = g_StrandCoordinates.Load(ID & g_NumInterpolatedAttributesMinusOne);
    coords.z = (1 - coords.x - coords.y);
    float3x2 rootTexcoords;
    rootTexcoords[0] = g_Attributes.Load( floor(originalMasterStrandRoots.x));
    rootTexcoords[1] = g_Attributes.Load( floor(originalMasterStrandRoots.y));
    rootTexcoords[2] = g_Attributes.Load( floor(originalMasterStrandRoots.z));       
    float2 BaryCentricTexcoords =   coords.x  * rootTexcoords[0].xy
                                  + coords.y  * rootTexcoords[1].xy
                                  + coords.z  * rootTexcoords[2].xy;                                     
    //sample the density map, and if we are over the density dont create anymore hair
    float4 hairDensityThickness = densityThicknessMapBarycentric.SampleLevel(samLinear,BaryCentricTexcoords,0);
    
    
    //y: the wisp number
    float fullYPixel = 2.0/g_NumTotalWisps;
    float yPos = -1 + index*fullYPixel + fullYPixel/2.0;
    
    //x: the interpolated hair number 
    int interpHairNumber = frac(fIndex)*100;
    float fullXPixel = 2.0/g_NumMaxStrandsPerWisp;
    float xPos = -1 + interpHairNumber*fullXPixel + fullXPixel/2.0;
   
    //barycentric position---------------------------------------------------------------------   
    int3 vertexIndices;
    vertexIndices[0] = floor(masterStrandRoots.x) + min(vertexID,lengths.x);
    vertexIndices[1] = floor(masterStrandRoots.y) + min(vertexID,lengths.y);
    vertexIndices[2] = floor(masterStrandRoots.z) + min(vertexID,lengths.z);
    float3x4 masterVertexPosition;
    masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
    masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
    masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
    float3 BarycenticPosition =   coords.x  * masterVertexPosition[0].xyz
                      + coords.y  * masterVertexPosition[1].xyz
                      + coords.z  * masterVertexPosition[2].xyz;

    output.Position = float4(xPos,yPos,0.5,1.0);    
    output.vertexID = vertexID;
    output.BarycenticPosition = BarycenticPosition;
    
    return output;
} 

//SINGLESTRAND BASED INTERPOLATION
//vertex shaders for singlestrand based interpolation
//vertex shaders for rendering, depth pre-pass and shadows

//calculate the position, width and ID
HairVertexPWI InterpolateVSClumpCore0( uint InstanceID, uint vertexID, inout int vertexIndexOut, 
inout float2 texcoordsOut, inout int MasterStrandNumberOut, inout float lengthToRootOut, inout int MasterStrandLengthOut )
{
    HairVertexPWI output = (HairVertexPWI)0;
    float fIndex = g_sStrandIndices.Load(InstanceID);
    int MasterStrandNumber = floor(fIndex);
    int outputID = InstanceID;    

    int tessellatedMasterStrandRootIndex;
    if(MasterStrandNumber==0)
        tessellatedMasterStrandRootIndex = 0;
    else
        tessellatedMasterStrandRootIndex = g_tessellatedMasterStrandRootIndex.Load(MasterStrandNumber-1);    
    int MasterStrandLength = g_tessellatedMasterStrandRootIndex.Load(MasterStrandNumber) - tessellatedMasterStrandRootIndex;  

    //If the current vertexID is more than master strand length then return
    if((int)vertexID > MasterStrandLength-2)
    {
        output.ID = -1;
        return output;
    }	

    float2 texcoords = g_Attributes.Load(MasterStrandNumber);

    //--------------------------------------------------------------------------------------------
    //sample the density map, and if we are over the density dont create anymore hair
    float4 hairDensityThickness = densityThicknessMapClump.SampleLevel(samLinear,texcoords,0);

    int vertexIndex = tessellatedMasterStrandRootIndex+vertexID;
    float lengthToRoot = g_TessellatedLengthsToRoots.Load( vertexIndex );
    float2 clumpCoordinates = g_StrandCircularCoordinates.Load(outputID & g_NumInterpolatedAttributesMinusOne);
    float4 masterVertexPosition = g_TessellatedMasterStrand.Load(vertexIndex);

    //jitter the position slightly
    float2 jitter = g_strandDeviations.Load((outputID & g_NumInterpolatedAttributesMinusOne) * g_TessellatedMasterStrandLengthMax + vertexID); 
 
    if(g_doCurlyHair)
        jitter += g_curlDeviations.Load(MasterStrandNumber*g_TessellatedMasterStrandLengthMax + vertexID); 

    clumpCoordinates += jitter;

    //modulating width by the width denoted in the densityThickness texture----------------------
    output.width = hairDensityThickness.r;

    //load the coordinate frames
    coordinateFrame4 cf;
    cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 1); 
    cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 2);

    output.ID = outputID; 

    //create the new position for the hair clump vertex
    float radius = g_clumpWidth * ( g_topWidth*(1-lengthToRoot) + g_bottomWidth*lengthToRoot );    
    output.Position.xyz = masterVertexPosition.xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;

    if(vertexID==0)
    {
        cf.xAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 ); 
        output.Position.xyz -= 0.25*cf.xAxis.xyz; 
    }

    vertexIndexOut = vertexIndex;
    texcoordsOut = texcoords;
    MasterStrandNumberOut = MasterStrandNumber; 
    lengthToRootOut = lengthToRoot;
    MasterStrandLengthOut = MasterStrandLength;

    return output;
}

//calculate the position, width, ID, alpha and texture
HairVertexPWIAT InterpolateVSClumpCore(uint InstanceID, uint vertexID, inout int vertexIndexOut, inout float2 texcoordsOut)
{

    HairVertexPWI pwi;
    int vertexIndex = 0;
    float2 texcoords = float2(0,0);
    float lengthToRoot = 0;
    int MasterStrandNumber = 0;
    int MasterStrandLength = 0;
    pwi = InterpolateVSClumpCore0(InstanceID,vertexID,vertexIndex,texcoords,MasterStrandNumber,lengthToRoot,MasterStrandLength);
        
    HairVertexPWIAT output = (HairVertexPWIAT)0;
    output.Position = pwi.Position;
    output.width = pwi.width;
    output.ID = pwi.ID;
    if(output.ID<0) 
        return output;
    
    float totalLength = g_Lengths.Load( MasterStrandNumber );

    //thinning the hair lengths
    float inLengthFrac = g_StrandLengths.Load(output.ID & g_NumInterpolatedAttributesMinusOne);
    float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;
       
    if( ( int(lengthToRoot*MasterStrandLength) > int(maxLength*MasterStrandLength) + 1 ) || ((int)vertexID > MasterStrandLength-2) )
    {    output.ID = -1;
         return output;
    }  
    if( (lengthToRoot > maxLength) || ((int)vertexID == MasterStrandLength-2) )
        output.width = EPSILON;
 
        
    //tapering the hair
    float lt = g_lt*maxLength;
    if(lengthToRoot > lt)
        output.width *= ( 1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt) );

    //this texture goes from 0 to 1 for the longest hair in the hairstyle.
    //lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
    //totalLength is the actual total length of the strand
    //g_maxLengthToRoot is the real total length of the longest strand
    output.tex = lengthToRoot*totalLength/g_maxLengthToRoot;


    vertexIndexOut = vertexIndex;
    texcoordsOut = texcoords;
    
    return output;
}

HairVertex2 InterpolateVSClumpTotalCore(uint InstanceID, uint vertexID)
{
    HairVertexPWIAT outputDepth;
    int vertexIndex = 0;
    float2 texcoords = float2(0,0);
    outputDepth = InterpolateVSClumpCore(InstanceID,vertexID,vertexIndex,texcoords);

    HairVertex2 output = (HairVertex2)0;
    output.Position = outputDepth.Position;
    output.width = outputDepth.width;
    output.ID = outputDepth.ID;
    output.tex = outputDepth.tex;
    output.Tangent = float3(0,0,0);
    output.scalpTexcoords = 0;
    output.shadow = 0;
    if(output.ID<0) 
        return output;
        
    //tangent
    output.Tangent.xyz = normalize((g_TessellatedTangents.Load(vertexIndex)).xyz);
    
    //texcoords
    output.scalpTexcoords = texcoords;
    
    return output;
}

// each hull shader processes strands belonging to the same master strand
BARY_HS_CONSTANT_DATA_OUTPUT InterpolateConstHSMultiStrand(uint iWisp : SV_PrimitiveID)
{    
    BARY_HS_CONSTANT_DATA_OUTPUT output;

	// how many c-strands accompany this master strand?
	output.nHairsBefore = g_iFirstPatchHair;
	if (iWisp > 0)
	{
		output.nHairsBefore += g_MStrandsPerWispCumulative.Load(iWisp - 1);
	}
    output.Edges[0] = min(NHAIRS_PER_PATCH, g_MStrandsPerWispCumulative.Load(iWisp) - output.nHairsBefore);
    float4 lengths = g_MasterStrandLengths.Load(iWisp); //these are the lengths in number of vertices from root
	output.lengthsw = lengths.w;
	int iSegmentsToGo = (int)lengths.w - 1 - g_iSubHairFirstVert;
	output.Edges[1] = min(iSegmentsToGo, NSEGMENTS_PER_PATCH);
	output.masterStrandRoots = g_MasterStrandRootIndices.Load(iWisp);
	float3 originalMasterStrandRoots = g_OriginalMasterStrandRootIndices.Load(iWisp);
	output.rootTexcoords[0] = g_Attributes.Load( floor(originalMasterStrandRoots.x));
	output.rootTexcoords[1] = g_Attributes.Load( floor(originalMasterStrandRoots.y));
	output.rootTexcoords[2] = g_Attributes.Load( floor(originalMasterStrandRoots.z));       
	output.totalLengths[0] = g_Lengths.Load( originalMasterStrandRoots.x );
	output.totalLengths[1] = g_Lengths.Load( originalMasterStrandRoots.y );
	output.totalLengths[2] = g_Lengths.Load( originalMasterStrandRoots.z ); 

	// dynamic LOD in number of hairs
	output.Edges[0] = max(output.Edges[0] * g_fNumHairsLOD, 1);

    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(1)]
[patchconstantfunc("InterpolateConstHSMultiStrand")]
void InterpolateHSMultiStrand(InputPatch<DUMMY, 1> inputPatch)
{
}

// each hull shader processes c-strands belonging to the same master strand
BARYDEPTH_HS_CONSTANT_DATA_OUTPUT InterpolateConstHSMultiStrandDepth(uint iWisp : SV_PrimitiveID)
{    
    BARYDEPTH_HS_CONSTANT_DATA_OUTPUT output;

	output.nHairsBefore = g_iFirstPatchHair;
	if (iWisp > 0)
	{
		output.nHairsBefore += g_MStrandsPerWispCumulative.Load(iWisp - 1);
	}
    output.Edges[0] = min(NHAIRS_PER_PATCH, g_MStrandsPerWispCumulative.Load(iWisp) - output.nHairsBefore);
    float4 lengths = g_MasterStrandLengths.Load(iWisp); //these are the lengths in number of vertices from root
	output.lengthsw = lengths.w;
	int iSegmentsToGo = (int)lengths.w - 1 - g_iSubHairFirstVert;
	output.Edges[1] = min(iSegmentsToGo, NSEGMENTS_PER_PATCH);
	output.masterStrandRoots = g_MasterStrandRootIndices.Load(iWisp);
	float3 originalMasterStrandRoots = g_OriginalMasterStrandRootIndices.Load(iWisp);
	output.rootTexcoords[0] = g_Attributes.Load( floor(originalMasterStrandRoots.x));
	output.rootTexcoords[1] = g_Attributes.Load( floor(originalMasterStrandRoots.y));
	output.rootTexcoords[2] = g_Attributes.Load( floor(originalMasterStrandRoots.z));

	// dynamic LOD in number of hairs
	output.Edges[0] = max(output.Edges[0] * g_fNumHairsLOD, 1);

    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(1)]
[patchconstantfunc("InterpolateConstHSMultiStrandDepth")]
void InterpolateHSMultiStrandDepth(InputPatch<DUMMY, 1> inputPatch)
{
}

// each hull shader processes c-strands belonging to the same master strand
BARYCOLLISIONS_HS_CONSTANT_DATA_OUTPUT InterpolateConstHSMultiStrandCollisions(uint iWisp : SV_PrimitiveID)
{    
    BARYCOLLISIONS_HS_CONSTANT_DATA_OUTPUT output;

	output.nHairsBefore = g_iFirstPatchHair;
	if (iWisp > 0)
	{
		output.nHairsBefore += g_MStrandsPerWispCumulative.Load(iWisp - 1);
	}
    output.Edges[0] = min(NHAIRS_PER_PATCH, g_MStrandsPerWispCumulative.Load(iWisp) - output.nHairsBefore);
    float4 lengths = g_MasterStrandLengths.Load(iWisp); //these are the lengths in number of vertices from root
	int iSegmentsToGo = (int)lengths.w - 1 - g_iSubHairFirstVert;
	output.Edges[1] = min(iSegmentsToGo, NSEGMENTS_PER_PATCH);
	output.masterStrandRoots = g_MasterStrandRootIndices.Load(iWisp);

	// dynamic LOD in number of hairs
	output.Edges[0] = max(output.Edges[0] * g_fNumHairsLOD, 1);

    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(1)]
[patchconstantfunc("InterpolateConstHSMultiStrandCollisions")]
void InterpolateHSMultiStrandCollisions(InputPatch<DUMMY, 1> inputPatch)
{
}

// each hull shader processes c-strands belonging to the same master strand
HS_CONSTANT_DATA_OUTPUT InterpolateConstHSSingleStrand(uint iPatch : SV_PrimitiveID)
{    
    HS_CONSTANT_DATA_OUTPUT output;

	// how many c-strands accompany this master strand?
	output.nHairsBefore = g_iFirstPatchHair;
	if (iPatch > 0)
	{
		output.nHairsBefore += g_SStrandsPerMasterStrandCumulative.Load(iPatch - 1);
	}
    output.Edges[0] = min(NHAIRS_PER_PATCH, g_SStrandsPerMasterStrandCumulative.Load(iPatch) - output.nHairsBefore);
	output.MasterStrandLength = g_tessellatedMasterStrandRootIndex.Load(iPatch);
    output.tessellatedMasterStrandRootIndex = 0;
	if (iPatch > 0)
	{
        output.tessellatedMasterStrandRootIndex = g_tessellatedMasterStrandRootIndex.Load(iPatch - 1);
	}
	output.MasterStrandLength -= output.tessellatedMasterStrandRootIndex;
	output.Edges[1] = min(NSEGMENTS_PER_PATCH, output.MasterStrandLength - g_iSubHairFirstVert - 2);
	output.texcoords = g_Attributes.Load(iPatch);
    output.totalLength = g_Lengths.Load(iPatch);
    //--------------------------------------------------------------------------------------------
    //sample the density map, and if we are over the density dont create anymore hair
    float4 hairDensityThickness = densityThicknessMapClump.SampleLevel(samLinear,output.texcoords,0);
	output.hairDensity = hairDensityThickness.r;

	// dynamic LOD in number of hairs
	output.Edges[0] = max(output.Edges[0] * g_fNumHairsLOD, 1);

    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(1)]
[patchconstantfunc("InterpolateConstHSSingleStrand")]
void InterpolateHSSingleStrand(InputPatch<DUMMY, 1> inputPatch)
{
}

[domain("isoline")]
HairVertex2 InterpolateDSMultiStrand(OutputPatch<DUMMY, 1> inputPatch, BARY_HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, uint iWispID : SV_PrimitiveID)
{
	// how many hairs did we render before this master strand
	uint iHairInsideCurMasterStrand = (int)(uv.y * input.Edges[0] + 0.5);
	uint iVertexInsideCurHair = (int)(uv.x * input.Edges[1] + 0.5) + g_iSubHairFirstVert;

	uint InstanceID = iHairInsideCurMasterStrand + input.nHairsBefore;
	uint vertexID = iVertexInsideCurHair;

	HairVertexPWIAT outputDepth;

	//get the position, width and ID, and related values
	HairVertexPWI pwi = (HairVertexPWI)0;

	float fIndex = (float)iHairInsideCurMasterStrand / 1000.f;
	int index = iWispID;

	pwi.ID = InstanceID;

	float3 coords;
	coords.xy = g_StrandCoordinates.Load(pwi.ID & g_NumInterpolatedAttributesMinusOne);
	coords.z = (1 - coords.x - coords.y);

	//texcoords ----------------------------------------------------------------------------------

	float2 BaryCentricTexcoords = coords.x * input.rootTexcoords[0].xy
		+ coords.y * input.rootTexcoords[1].xy
		+ coords.z * input.rootTexcoords[2].xy;

	//--------------------------------------------------------------------------------------------
	//sample the density map, and if we are over the density dont create anymore hair
	float4 hairDensityThickness = densityThicknessMapBarycentric.SampleLevel(samLinear,BaryCentricTexcoords,0);  

	int3 vertexIndices;
	vertexIndices[0] = floor(input.masterStrandRoots.x) + vertexID;
	vertexIndices[1] = floor(input.masterStrandRoots.y) + vertexID;
	vertexIndices[2] = floor(input.masterStrandRoots.z) + vertexID;

	//length to root--------------------------------------------------------------------------------
	//needed for clumping, tapering the ends and for texture mapping along the strand
	float3 lengthsToRoots;
	lengthsToRoots[0] = g_TessellatedLengthsToRoots.Load( vertexIndices[0] );
	lengthsToRoots[1] = g_TessellatedLengthsToRoots.Load( vertexIndices[1] );
	lengthsToRoots[2] = g_TessellatedLengthsToRoots.Load( vertexIndices[2] ); 

	//barycentric position---------------------------------------------------------------------   
	float3x4 masterVertexPosition;
	masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
	masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
	masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
	float3 BarycenticPosition = coords.x  * masterVertexPosition[0].xyz
		+ coords.y * masterVertexPosition[1].xyz
		+ coords.z * masterVertexPosition[2].xyz;

	float clumpWeight=0;
	float barycentricWeight = 1.0;

	int closestIndex =  findClosestIndex(coords);

	//clamp-------------------------------------------------------------------------------------
	//if the hair strand below intersects an obstacle clamp to nearest guide hair

	float fullYPixel = 1.0/g_NumTotalWisps;
	float fullXPixel = 1.0/g_NumMaxStrandsPerWisp;
	int interpHairNumber = frac(fIndex)*100;
	float2 texcoordsCollision;
	texcoordsCollision.x = interpHairNumber*fullXPixel + fullXPixel/2.0;
	texcoordsCollision.y = 1.0 - (index*fullYPixel + fullYPixel/2.0);
	float collisions = g_CollisionsTexture.SampleLevel(samPointClamp,texcoordsCollision,0).r;
	int buffer = 60;
	float2 texcoords;

	float BarycetricLengthToRoot =  coords.x  * lengthsToRoots[0]
	+ coords.y  * lengthsToRoots[1]
	+ coords.z  * lengthsToRoots[2];
	float lengthToRoot;

#if 1
	if(collisions < (int)vertexID + buffer)
	{
		float2 clumpCoordinates = g_StrandCircularCoordinates.Load(pwi.ID & g_NumInterpolatedAttributesMinusOne); // this is completely wrong, but it does not lead to a big visual problem. THESE COORDINATES HAVE NOTHING IN COMMON WITH THE OTHER COORDINATES THAT WE HAVE FOR MULTISTRAND. IE BOTH HAIR ARE COMPLETELT DIFFERENT
		float clumpLengthToRoot = lengthsToRoots[closestIndex];
		float2 ClumpTexcoords = input.rootTexcoords[closestIndex];

		//load the coordinate frames
		coordinateFrame4 cf;
		cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 1);
		cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 2);

		//create the new position for the hair clump vertex
		float radius = g_clumpWidth * (g_topWidth*(1-clumpLengthToRoot) + g_bottomWidth*clumpLengthToRoot);
		float3 ClumpPosition = masterVertexPosition[closestIndex].xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;

		clumpWeight = (vertexID - collisions + buffer)/float(buffer);
		clumpWeight = clamp(clumpWeight,0,1);
		clumpWeight = sin((clumpWeight-0.5)*PI)*0.5 + 0.5;

		barycentricWeight = 1.0-clumpWeight;
		pwi.Position = BarycenticPosition*barycentricWeight + ClumpPosition*clumpWeight;
		texcoords = BaryCentricTexcoords*barycentricWeight + ClumpTexcoords*clumpWeight;
		lengthToRoot = BarycetricLengthToRoot*barycentricWeight + clumpLengthToRoot*clumpWeight;
	}
	else
#endif
	{
		pwi.Position = BarycenticPosition;
		texcoords = BaryCentricTexcoords;
		lengthToRoot = BarycetricLengthToRoot;
	}

	if(vertexID==0) //rooting the interpolated hair
	{
		float3 xAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 ).xyz;
		pwi.Position.xyz -= 0.25*xAxis; 
	}

	//tapering the hair---------------------------------------------------------------------------
	//modulating width by the width denoted in the densityThickness texture-----------------------
	pwi.width = hairDensityThickness.r;
	float lt = lerp( g_lt, g_lt-0.2, hairDensityThickness.r );    
	if(lengthToRoot > lt)
		pwi.width *= ( 1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt) );

#ifdef SCALE_WIDTH_WITH_LOD
    //float fNumHairsLOD = max(0.1,g_fNumHairsLOD);
    //pwi.width /= fNumHairsLOD;
	pwi.width *= g_fWidthHairsLOD;    
#endif

	outputDepth.Position = pwi.Position;
	outputDepth.width = pwi.width;
	outputDepth.ID = pwi.ID;
	outputDepth.tex = 0;    

	float BarycentricTotalLength = coords.x  * input.totalLengths[0]
	+ coords.y * input.totalLengths[1]
	+ coords.z * input.totalLengths[2];
	float clumpTotalLength = input.totalLengths[closestIndex];
	float totalLength = BarycentricTotalLength*barycentricWeight + clumpTotalLength*clumpWeight;

	//this texture goes from 0 to 1 for the longest hair in the hairstyle.
	//lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
	//totalLength is the actual total length of the strand
	//g_maxLengthToRoot is the total length of the longest strand
	outputDepth.tex = lengthToRoot*totalLength/g_maxLengthToRoot;

	//thinning the hair lengths---------------------------------------------------------------------
	float inLengthFrac = g_StrandLengths.Load(outputDepth.ID & g_NumInterpolatedAttributesMinusOne);
	float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;

	if( (lengthToRoot > maxLength) || ((int)vertexID == int(input.lengthsw)-1) )
		outputDepth.width = EPSILON;    

	HairVertex2 hairvert;
	hairvert.Position = outputDepth.Position;
	hairvert.width = outputDepth.width;
	hairvert.ID = outputDepth.ID;
	hairvert.tex = outputDepth.tex;
	hairvert.Tangent = float3(0,0,0);
	hairvert.scalpTexcoords = 0;
	hairvert.shadow = 0;

	float3x4 masterVertexTangents;
	masterVertexTangents[0] = g_TessellatedTangents.Load( vertexIndices[0] );
	masterVertexTangents[1] = g_TessellatedTangents.Load( vertexIndices[1] );
	masterVertexTangents[2] = g_TessellatedTangents.Load( vertexIndices[2] ); 
	float3 BarycentricTangent = coords.x * masterVertexTangents[0].xyz
		+ coords.y * masterVertexTangents[1].xyz
		+ coords.z * masterVertexTangents[2].xyz;
	float3 ClumpTangent = masterVertexTangents[closestIndex].xyz;
	hairvert.Tangent = BarycentricTangent*barycentricWeight + ClumpTangent*clumpWeight;
	hairvert.Tangent = normalize(hairvert.Tangent);

	hairvert.scalpTexcoords = texcoords;

	if(g_bApplyAdditionalRenderingTransform)
	    hairvert.Position.xyz = mul( float4(hairvert.Position.xyz,1.0),additionalTransformation).xyz;

#ifdef SHADOWS_VS
	if(g_useShadows)
	{
		float3 Pos = mul(float4(hairvert.Position, 1), HairToWorldTransform).xyz;
		float2 texcoords = WorldToLightCoord(Pos);
		float z = WorldToDepth(Pos);
		//TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex float2 texcoords = WorldToLightCoord(output.Position.xyz);
		//TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView float z = WorldToDepth(output.Position.xyz);
		hairvert.shadow = ShadowPCF(texcoords, z);
	}
	else
		hairvert.shadow = 0;
#endif

	return hairvert;    
}

[domain("isoline")]
HairVertexPWI InterpolateDSMultiStrandDepth(OutputPatch<DUMMY, 1> inputPatch, BARYDEPTH_HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, uint iWispID : SV_PrimitiveID)
{
	// how many hairs did we render before this master strand
    uint iHairInsideCurMasterStrand = (int)(uv.y * input.Edges[0] + 0.5);
	uint iVertexInsideCurHair = (int)(uv.x * input.Edges[1] + 0.5) + g_iSubHairFirstVert;

	uint InstanceID = iHairInsideCurMasterStrand + input.nHairsBefore;
	uint vertexID = iVertexInsideCurHair;

	HairVertexPWIAT outputDepth;

	float fIndex = (float)iHairInsideCurMasterStrand / 1000.f;
	int index = iWispID;

	float3 coords;
	coords.xy = g_StrandCoordinates.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
	coords.z = (1 - coords.x - coords.y);

	//texcoords ----------------------------------------------------------------------------------

	float2 BaryCentricTexcoords = coords.x * input.rootTexcoords[0].xy
		+ coords.y * input.rootTexcoords[1].xy
		+ coords.z * input.rootTexcoords[2].xy;

	//--------------------------------------------------------------------------------------------
	//sample the density map, and if we are over the density dont create anymore hair
	float4 hairDensityThickness = densityThicknessMapBarycentric.SampleLevel(samLinear,BaryCentricTexcoords,0);  

	int3 vertexIndices;
	vertexIndices[0] = floor(input.masterStrandRoots.x) + vertexID;
	vertexIndices[1] = floor(input.masterStrandRoots.y) + vertexID;
	vertexIndices[2] = floor(input.masterStrandRoots.z) + vertexID;

	//length to root--------------------------------------------------------------------------------
	//needed for clumping, tapering the ends and for texture mapping along the strand
	float3 lengthsToRoots;
	lengthsToRoots[0] = g_TessellatedLengthsToRoots.Load( vertexIndices[0] );
	lengthsToRoots[1] = g_TessellatedLengthsToRoots.Load( vertexIndices[1] );
	lengthsToRoots[2] = g_TessellatedLengthsToRoots.Load( vertexIndices[2] ); 

	//barycentric position---------------------------------------------------------------------   
	float3x4 masterVertexPosition;
	masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
	masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
	masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
	float3 BarycenticPosition = coords.x  * masterVertexPosition[0].xyz
		+ coords.y * masterVertexPosition[1].xyz
		+ coords.z * masterVertexPosition[2].xyz;

	float clumpWeight=0;
	float barycentricWeight = 1.0;

	int closestIndex =  findClosestIndex(coords);

	//clamp-------------------------------------------------------------------------------------
	//if the hair strand below intersects an obstacle clamp to nearest guide hair

	float fullYPixel = 1.0/g_NumTotalWisps;
	float fullXPixel = 1.0/g_NumMaxStrandsPerWisp;
	int interpHairNumber = frac(fIndex)*100;
	float2 texcoordsCollision;
	texcoordsCollision.x = interpHairNumber*fullXPixel + fullXPixel/2.0;
	texcoordsCollision.y = 1.0 - (index*fullYPixel + fullYPixel/2.0);
	float collisions = g_CollisionsTexture.SampleLevel(samPointClamp,texcoordsCollision,0).r;
	int buffer = 60;
	float2 texcoords;

	float BarycetricLengthToRoot =  coords.x * lengthsToRoots[0]
	+ coords.y * lengthsToRoots[1]
	+ coords.z * lengthsToRoots[2];
	float lengthToRoot;

    HairVertexPWI pwi;
	pwi.ID = InstanceID;
#if 1
	if(collisions < (int)vertexID + buffer)
	{
		float2 clumpCoordinates = g_StrandCircularCoordinates.Load(InstanceID & g_NumInterpolatedAttributesMinusOne); // this is completely wrong, but it does not lead to a big visual problem. THESE COORDINATES HAVE NOTHING IN COMMON WITH THE OTHER COORDINATES THAT WE HAVE FOR MULTISTRAND. IE BOTH HAIR ARE COMPLETELT DIFFERENT
		float clumpLengthToRoot = lengthsToRoots[closestIndex];
		float2 ClumpTexcoords = input.rootTexcoords[closestIndex];

		//load the coordinate frames
		coordinateFrame4 cf;
		cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 1);
		cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 + 2);

		//create the new position for the hair clump vertex
		float radius = g_clumpWidth * (g_topWidth*(1-clumpLengthToRoot) + g_bottomWidth*clumpLengthToRoot);
		float3 ClumpPosition = masterVertexPosition[closestIndex].xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;

		clumpWeight = (vertexID - collisions + buffer)/float(buffer);
		clumpWeight = clamp(clumpWeight,0,1);
		clumpWeight = sin((clumpWeight-0.5)*PI)*0.5 + 0.5;

		barycentricWeight = 1.0-clumpWeight;
		pwi.Position = BarycenticPosition*barycentricWeight + ClumpPosition*clumpWeight;
		texcoords = BaryCentricTexcoords*barycentricWeight + ClumpTexcoords*clumpWeight;
		lengthToRoot = BarycetricLengthToRoot*barycentricWeight + clumpLengthToRoot*clumpWeight;
	}
	else
#endif
	{
		pwi.Position = BarycenticPosition;
		texcoords = BaryCentricTexcoords;
		lengthToRoot = BarycetricLengthToRoot;
	}

	if(vertexID==0) //rooting the interpolated hair
	{
		float3 xAxis = g_tessellatedCoordinateFrames.Load( vertexIndices[closestIndex]*3 ).xyz;
		pwi.Position.xyz -= 0.25*xAxis; 
	}

	//tapering the hair---------------------------------------------------------------------------
	//modulating width by the width denoted in the densityThickness texture-----------------------
	pwi.width = hairDensityThickness.r;
	float lt = lerp( g_lt, g_lt-0.2, hairDensityThickness.r );    
	if(lengthToRoot > lt)
		pwi.width *= ( 1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt) );

#ifdef SCALE_WIDTH_WITH_LOD
//    float fNumHairsLOD = max(0.1,g_fNumHairsLOD);
//    pwi.width /= fNumHairsLOD;
   	pwi.width *= g_fWidthHairsLOD;
#endif

	//this texture goes from 0 to 1 for the longest hair in the hairstyle.
	//lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
	//totalLength is the actual total length of the strand
	//g_maxLengthToRoot is the total length of the longest strand

	//thinning the hair lengths---------------------------------------------------------------------
	float inLengthFrac = g_StrandLengths.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
	float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;

	if( (lengthToRoot > maxLength) || ((int)vertexID == int(input.lengthsw)-1) )
		pwi.width = EPSILON;    

    if(g_bApplyAdditionalRenderingTransform)
	    pwi.Position.xyz = mul( float4(pwi.Position.xyz,1.0),additionalTransformation).xyz;

	return pwi;
}

[domain("isoline")]
HairCollisionVertexWithGS InterpolateDSMultiStrandCollisionsWithGS(OutputPatch<DUMMY, 1> inputPatch, BARYCOLLISIONS_HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, uint iWispID : SV_PrimitiveID)
{
    HairCollisionVertex output = (HairCollisionVertex)0;

	// how many hairs did we render before this master strand
    uint iHairInsideCurMasterStrand = (int)(uv.y * input.Edges[0] + 0.5);
	uint iVertexInsideCurHair = (int)(uv.x * input.Edges[1] + 0.5) + g_iSubHairFirstVert;

	uint InstanceID = iHairInsideCurMasterStrand + input.nHairsBefore;
	uint vertexID = iVertexInsideCurHair;

	float fIndex = (float)iHairInsideCurMasterStrand / 1000.f;
	int index = iWispID;

	float3 coords;
	coords.xy = g_StrandCoordinates.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
	coords.z = (1 - coords.x - coords.y);

    //y: the wisp number
    float fullYPixel = 2.0/g_NumTotalWisps;
    float yPos = -1 + index*fullYPixel + fullYPixel/2.0;
    
    //x: the interpolated hair number 
    int interpHairNumber = frac(fIndex)*100;
    float fullXPixel = 2.0/g_NumMaxStrandsPerWisp;
    float xPos = -1 + interpHairNumber*fullXPixel + fullXPixel/2.0;

	//barycentric position 
	
	int3 vertexIndices;
	vertexIndices[0] = floor(input.masterStrandRoots.x) + vertexID;
	vertexIndices[1] = floor(input.masterStrandRoots.y) + vertexID;
	vertexIndices[2] = floor(input.masterStrandRoots.z) + vertexID;	
	
	float3x4 masterVertexPosition;
	masterVertexPosition[0] = g_TessellatedMasterStrand.Load( vertexIndices[0] );
	masterVertexPosition[1] = g_TessellatedMasterStrand.Load( vertexIndices[1] );
	masterVertexPosition[2] = g_TessellatedMasterStrand.Load( vertexIndices[2] );       
	float3 BarycenticPosition = coords.x  * masterVertexPosition[0].xyz
		+ coords.y * masterVertexPosition[1].xyz
		+ coords.z * masterVertexPosition[2].xyz;


    output.Position = float4(xPos,yPos,0.5,1.0);
    output.BarycenticPosition = BarycenticPosition;
    output.vertexID = vertexID;

    return output;
}

[domain("isoline")]
HairVertex2 InterpolateDSSingleStrand_NORMAL(OutputPatch<DUMMY, 1> inputPatch, HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, uint MasterStrandNumber : SV_PrimitiveID)
{
    HairVertex2 output;

	// how many hairs did we render before this master strand
    uint iHairInsideCurMasterStrand = (int)(uv.y * input.Edges[0] + 0.5);
	uint iVertexInsideCurHair = (int)(uv.x * input.Edges[1] + 0.5) + g_iSubHairFirstVert;

	uint InstanceID = input.nHairsBefore + iHairInsideCurMasterStrand;
	uint vertexID = iVertexInsideCurHair;

    int vertexIndex = input.tessellatedMasterStrandRootIndex+vertexID;
    float lengthToRoot = g_TessellatedLengthsToRoots.Load( vertexIndex );
    float2 clumpCoordinates = g_StrandCircularCoordinates.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
    float4 masterVertexPosition = g_TessellatedMasterStrand.Load(vertexIndex);

    //jitter the position slightly
    float2 jitter = g_strandDeviations.Load((InstanceID & g_NumInterpolatedAttributesMinusOne) * g_TessellatedMasterStrandLengthMax + vertexID);

    if(g_doCurlyHair)
        jitter += g_curlDeviations.Load(MasterStrandNumber*g_TessellatedMasterStrandLengthMax + vertexID); 

    clumpCoordinates += jitter;

    //load the coordinate frames
    coordinateFrame4 cf;
    cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 1); 
    cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 2);

    //create the new position for the hair clump vertex
    float radius = g_clumpWidth * ( g_topWidth*(1-lengthToRoot) + g_bottomWidth*lengthToRoot );
    output.Position.xyz = masterVertexPosition.xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;

    if(vertexID==0)
    {
        cf.xAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 ); 
        output.Position.xyz -= 0.25*cf.xAxis.xyz; 
    }

    output.width = input.hairDensity;

    //thinning the hair lengths
    float inLengthFrac = g_StrandLengths.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
    float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;

    if((lengthToRoot > maxLength) || ((int)vertexID == input.MasterStrandLength-2))
        output.width = EPSILON;

    //tapering the hair
    float lt = g_lt*maxLength;
    if(lengthToRoot > lt)
        output.width *= (1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt));


#ifdef SCALE_WIDTH_WITH_LOD
	output.width *= g_fWidthHairsLOD;    
#endif

    //this texture goes from 0 to 1 for the longest hair in the hairstyle.
    //lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
    //totalLength is the actual total length of the strand
    //g_maxLengthToRoot is the real total length of the longest strand
    output.tex = lengthToRoot*input.totalLength/g_maxLengthToRoot;

    //tangent
    output.Tangent.xyz = normalize((g_TessellatedTangents.Load(vertexIndex)).xyz);

    output.scalpTexcoords = input.texcoords;

    output.shadow = 0;

	if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;

    #ifdef SHADOWS_VS
    if(g_useShadows)
    {
        float3 Pos = mul(float4(output.Position.xyz, 1), HairToWorldTransform).xyz;
	    float2 texcoords = WorldToLightCoord(Pos);
	    float z = WorldToDepth(Pos);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex float2 texcoords = WorldToLightCoord(output.Position.xyz);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView float z = WorldToDepth(output.Position.xyz);
	    output.shadow = ShadowPCF(texcoords, z);
	}
    else
        output.shadow = 0;
	#endif

    output.ID = InstanceID;
	return output;    
}

[domain("isoline")]
HairVertexPWI InterpolateDSSingleStrand_DEPTH(OutputPatch<DUMMY, 1> inputPatch, HS_CONSTANT_DATA_OUTPUT input, float2 uv : SV_DomainLocation, uint MasterStrandNumber : SV_PrimitiveID)
{
	// how many hairs did we render before this master strand
    uint iHairInsideCurMasterStrand = (int)(uv.y * input.Edges[0] + 0.5);
	uint iVertexInsideCurHair = (int)(uv.x * input.Edges[1] + 0.5) + g_iSubHairFirstVert;

	HairVertex2 hairvert;
	uint InstanceID = input.nHairsBefore + iHairInsideCurMasterStrand;
	uint vertexID = iVertexInsideCurHair;

    HairVertexPWIAT outputDepth;

    HairVertexPWI pwi = (HairVertexPWI)0;

    int vertexIndex = input.tessellatedMasterStrandRootIndex+vertexID;
    float lengthToRoot = g_TessellatedLengthsToRoots.Load( vertexIndex );
    float2 clumpCoordinates = g_StrandCircularCoordinates.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
    float4 masterVertexPosition = g_TessellatedMasterStrand.Load(vertexIndex);

    //jitter the position slightly
    float2 jitter = g_strandDeviations.Load((InstanceID & g_NumInterpolatedAttributesMinusOne) * g_TessellatedMasterStrandLengthMax + vertexID);

    if(g_doCurlyHair)
        jitter += g_curlDeviations.Load(MasterStrandNumber*g_TessellatedMasterStrandLengthMax + vertexID); 

    clumpCoordinates += jitter;

    //load the coordinate frames
    coordinateFrame4 cf;
    cf.yAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 1); 
    cf.zAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 + 2);

    //create the new position for the hair clump vertex
    float radius = g_clumpWidth * ( g_topWidth*(1-lengthToRoot) + g_bottomWidth*lengthToRoot );
    pwi.Position.xyz = masterVertexPosition.xyz + cf.yAxis.xyz*clumpCoordinates.x*radius + cf.zAxis.xyz*clumpCoordinates.y*radius;

    if(vertexID==0)
    {
        cf.xAxis = g_tessellatedCoordinateFrames.Load( vertexIndex*3 ); 
        pwi.Position.xyz -= 0.25*cf.xAxis.xyz; 
    }

	outputDepth = (HairVertexPWIAT)0;
    outputDepth.width = input.hairDensity;

    //thinning the hair lengths
    float inLengthFrac = g_StrandLengths.Load(InstanceID & g_NumInterpolatedAttributesMinusOne);
    float maxLength = 1.0-g_thinning + g_thinning*inLengthFrac;

    if((lengthToRoot > maxLength) || ((int)vertexID == input.MasterStrandLength-2))
        outputDepth.width = EPSILON;

    //tapering the hair
    float lt = g_lt*maxLength;
    if(lengthToRoot > lt)
        outputDepth.width *= (1 + (lt*(1-g_lv)/(1-lt)) - (1-g_lv)*lengthToRoot/(1-lt));
        
#ifdef SCALE_WIDTH_WITH_LOD
	outputDepth.width *= g_fWidthHairsLOD;    
#endif        

    //this texture goes from 0 to 1 for the longest hair in the hairstyle.
    //lengthToRoot is the fractional distance of the current vertex from the root counted in number of vertices
    //totalLength is the actual total length of the strand
    //g_maxLengthToRoot is the real total length of the longest strand
    outputDepth.tex = lengthToRoot*input.totalLength/g_maxLengthToRoot;

    //tangent
    hairvert.Tangent.xyz = normalize((g_TessellatedTangents.Load(vertexIndex)).xyz);

	HairVertexPWI output;
	output.Position = pwi.Position;
	output.width = outputDepth.width;
	output.ID = InstanceID;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;

	return output;    
}

[maxvertexcount(1)]
void InterpolateGSClump_SO(point InterpolatedSOBarycentricVertex vertex[1],
						   inout PointStream<InterpolatedSOBarycentricVertex> OutputStream)
{
	OutputStream.Append(vertex[0]);
}
HairVertex2 InterpolateVSSingleStrand(uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID)
{
    return InterpolateVSClumpTotalCore(InstanceID,vertexID);
}

HairVertexPWIAT InterpolateVSClumpDepthPrepass(uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID)
{
    int vertexIndex;
    float2 texcoords;
    return InterpolateVSClumpCore(InstanceID,vertexID,vertexIndex,texcoords); 
}

HairVertexPWI InterpolateVSSingleStrandDepthShadows(uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID)
{
    int vertexIndex = 0;
    float2 texcoords = float2(0,0);
    float lengthToRoot = 0;
    int MasterStrandNumber = 0;
    int MasterStrandLength = 0;
    return InterpolateVSClumpCore0(InstanceID,vertexID,vertexIndex,texcoords,MasterStrandNumber,lengthToRoot,MasterStrandLength);

}

InterpolatedSOBarycentricVertex InterpolateVSSingleStrand_SO(uint vertexID : SV_VertexID)
{
    HairVertex2 hairvert = InterpolateVSClumpTotalCore
    (vertexID/g_TessellatedMasterStrandLengthMax,vertexID%g_TessellatedMasterStrandLengthMax);
    
    InterpolatedSOBarycentricVertex output;
    output.PositionAndWidth = float4(hairvert.Position,hairvert.width);
    output.IdAlphaTex = float4(asfloat(hairvert.ID),1,hairvert.tex,hairvert.scalpTexcoords.x);
    output.tangent = float4(hairvert.Tangent,hairvert.scalpTexcoords.y);
    return output;    
}

HairVertex2 InterpolateVSSingleStrand_LOAD(uint index : SV_VertexID)
{
    HairVertex2 output = (HairVertex2)0;
    float4 PositionAndWidth = g_InterpolatedPositionAndWidthClump.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTexClump.Load(index);
    float4 tangent = g_InterpolatedtangentClump.Load(index);
    
    output.Position = PositionAndWidth.xyz;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;
    output.scalpTexcoords = float2(IdAlphaTex.w,tangent.w);
    
    
    #ifdef SHADOWS_VS
    if(g_useShadows)
    {
        float3 Pos = mul(float4(output.Position.xyz, 1), HairToWorldTransform).xyz;
	    float2 texcoords = WorldToLightCoord(Pos);
	    float z = WorldToDepth(Pos);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex float2 texcoords = WorldToLightCoord(output.Position.xyz);
	    //TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView float z = WorldToDepth(output.Position.xyz);
	    output.shadow = ShadowPCF(texcoords, z);
	}
    else
        output.shadow = 0;
	#endif
    
    output.width = PositionAndWidth.w;
    output.Tangent = tangent.xyz;
    output.ID = asint(IdAlphaTex.x);
    output.tex = IdAlphaTex.z;
        
    return output;
}

HairVertexPWI InterpolateVSSingleStrandDepthShadows_LOAD(uint index : SV_VertexID )
{
    HairVertexPWI output;
    float4 PositionAndWidth = g_InterpolatedPositionAndWidthClump.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTexClump.Load(index);
    
    output.Position = PositionAndWidth.xyz;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;
    output.width = PositionAndWidth.w;
    output.ID = asint(IdAlphaTex.x);
    
    return output;
}

HairVertexPWIAT InterpolateVSClumpDepthPrepass_LOAD( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    HairVertexPWIAT output;
    int index = InstanceID*g_TessellatedMasterStrandLengthMax + vertexID;
    
    float4 PositionAndWidth = g_InterpolatedPositionAndWidthClump.Load(index);
    float4 IdAlphaTex = g_InterpolatedIdAlphaTexClump.Load(index);
    
    output.Position = PositionAndWidth.xyz;
    if(g_bApplyAdditionalRenderingTransform)
	    output.Position.xyz = mul( float4(output.Position.xyz,1.0),additionalTransformation).xyz;
    output.width = PositionAndWidth.w;
    output.ID = asint(IdAlphaTex.x);
    output.tex = IdAlphaTex.z ;
                
    return output;
}

//rendering to grid does not do any interpolation
HairVertex2Grid InterpolateVS2Grid( uint InstanceID : SV_InstanceID, uint vertexID : SV_VertexID )
{
    HairVertex2Grid output = (HairVertex2Grid)0;
    int index = InstanceID;
    float3 masterStrandRoots = g_MasterStrandRootIndicesUntessellated.Load(index); 
    float4 lengths = g_MasterStrandLengthsUntessellated.Load( index );
    

    //if we have run over hair lengths
    if( (int)vertexID >= lengths.w )
    {    
        output.ID = -1;
        return output;
    }
    else
        output.ID = InstanceID;
                

    float3 coords;
    coords.xy = g_StrandCoordinates.Load(output.ID & g_NumInterpolatedAttributesMinusOne);
    coords.z = (1 - coords.x - coords.y);
    
    //position   
    float3x4 masterVertexPosition;
    masterVertexPosition[0] = g_MasterStrand.Load( floor(masterStrandRoots.x) + vertexID );
    masterVertexPosition[1] = g_MasterStrand.Load( floor(masterStrandRoots.y) + vertexID );
    masterVertexPosition[2] = g_MasterStrand.Load( floor(masterStrandRoots.z) + vertexID );       
   
    float3 position =   coords.x  * masterVertexPosition[0].xyz
                      + coords.y  * masterVertexPosition[1].xyz
                      + coords.z  * masterVertexPosition[2].xyz;
    
    output.Position = position;        
    return output;
} 

//geometry shaders
struct HairVertexTangent
{
    float4 Position : Position;
    float4 Tangent : Tangent;
};

[MaxVertexCount(1)]
void TessellateGSBSpline(point HairAdjacencyVertex input[1], inout PointStream<HairVertexTangent> inStream )
{ 
    HairVertexTangent output;

    float4x4 basisMatrix = float4x4
    (
        -1/6.0,  3/6.0, -3/6.0,  1/6.0,
         3/6.0, -6/6.0,  3/6.0,  0,
        -3/6.0,  0,      3/6.0,  0,
         1/6.0,  4/6.0,  1/6.0,  0
    );
    
  
    float u = input[0].u;
    
    //positions -------------------------------------------------------------------------
    float4x4 controlVertices;
    
    if(u<0)
    {   
        u += 2;
        controlVertices[0] = controlVertices[1] = g_MasterStrand.Load(input[0].controlVertexIndex);
        controlVertices[2] = g_MasterStrand.Load(input[0].controlVertexIndex + 1);
        controlVertices[3] = g_MasterStrand.Load(input[0].controlVertexIndex + 2);        
    }
    else if(u>1)
    {
        u -= 2;
        controlVertices[0] = g_MasterStrand.Load(input[0].controlVertexIndex + 1);
        controlVertices[1] = g_MasterStrand.Load(input[0].controlVertexIndex + 2);
        controlVertices[2] = controlVertices[3] = g_MasterStrand.Load(input[0].controlVertexIndex + 3);        
    }
    else
    {
          controlVertices[0] = g_MasterStrand.Load(input[0].controlVertexIndex );
          controlVertices[1] = g_MasterStrand.Load(input[0].controlVertexIndex + 1);
          controlVertices[2] = g_MasterStrand.Load(input[0].controlVertexIndex + 2);
          controlVertices[3] = g_MasterStrand.Load(input[0].controlVertexIndex + 3);
    }
    
       
    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);
    output.Position = float4(0, 0, 0, controlVertices[0].w);
    [unroll] for (int c = 0; c < 4; ++c) 
        output.Position += basis[c] * controlVertices[c];
 
    
    //tangents ---------------------------------------------------------------------
    float3x3 basisMatrixQuadratic = float3x3
    (
        0.5, -1.0, 0.5,
       -1.0,  1.0, 0.0,
        0.5,  0.5, 0.0
    );    
    
    float3 basisTangents  = mul(float3(u * u, u, 1), basisMatrixQuadratic);
    
    const float3 tangents[3] = 
    {
        float3(controlVertices[1].xyz - controlVertices[0].xyz),
        float3(controlVertices[2].xyz - controlVertices[1].xyz),
        float3(controlVertices[3].xyz - controlVertices[2].xyz),        
    };
    
    output.Tangent = float4(0,0,0,0);
    [unroll] for (c = 0; c < 3; ++c) 
        output.Tangent.xyz += basisTangents[c] * tangents[c];
 
    inStream.Append(output); 
}

[MaxVertexCount(1)]
void TessellateGSBSplineSB(point HairAdjacencyVertex input[1], inout PointStream<HairVertexTangent> inStream )
{ 
    HairVertexTangent output;

    float4x4 basisMatrix = float4x4
    (
        -1/6.0,  3/6.0, -3/6.0,  1/6.0,
         3/6.0, -6/6.0,  3/6.0,  0,
        -3/6.0,  0,      3/6.0,  0,
         1/6.0,  4/6.0,  1/6.0,  0
    );

    float u = input[0].u;
    
    //positions
    float4x4 controlVertices;
    
    if(u<0)
    {   
        u += 2;
        controlVertices[0] = controlVertices[1] = g_MasterStrandSB[input[0].controlVertexIndex];
        controlVertices[2] = g_MasterStrandSB[input[0].controlVertexIndex + 1];
        controlVertices[3] = g_MasterStrandSB[input[0].controlVertexIndex + 2];        
    }
    else if(u>1)
    {
        u -= 2;
        controlVertices[0] = g_MasterStrandSB[input[0].controlVertexIndex + 1];
        controlVertices[1] = g_MasterStrandSB[input[0].controlVertexIndex + 2];
        controlVertices[2] = controlVertices[3] = g_MasterStrandSB[input[0].controlVertexIndex + 3];        
    }
    else
    {
          controlVertices[0] = g_MasterStrandSB[input[0].controlVertexIndex ];
          controlVertices[1] = g_MasterStrandSB[input[0].controlVertexIndex + 1];
          controlVertices[2] = g_MasterStrandSB[input[0].controlVertexIndex + 2];
          controlVertices[3] = g_MasterStrandSB[input[0].controlVertexIndex + 3];
    }
  
    float4 basis = mul(float4(u * u * u, u * u, u, 1), basisMatrix);
    output.Position = float4(0, 0, 0, controlVertices[0].w);
    [unroll] for (int c = 0; c < 4; ++c) 
        output.Position += basis[c] * controlVertices[c];
   
    //tangents ---------------------------------------------------------------------
    float3x3 basisMatrixQuadratic = float3x3
    (
        0.5, -1.0, 0.5,
       -1.0,  1.0, 0.0,
        0.5,  0.5, 0.0
    );    
    
    float3 basisTangents  = mul(float3(u * u, u, 1), basisMatrixQuadratic);
    
    const float3 tangents[3] = 
    {
        float3(controlVertices[1].xyz - controlVertices[0].xyz),
        float3(controlVertices[2].xyz - controlVertices[1].xyz),
        float3(controlVertices[3].xyz - controlVertices[2].xyz),        
    };
    
    output.Tangent = float4(0,0,0,0);
    [unroll] for (c = 0; c < 3; ++c) 
        output.Tangent.xyz += basisTangents[c] * tangents[c];
 
    inStream.Append(output);  
}

[MaxVertexCount(4)]
void InterpolateGSDepthPrepass(line HairVertexPWIAT vertex[2], inout TriangleStream<HairPointDepthPrepass> stream)
{
    if (vertex[0].ID < 0 || vertex[1].ID < 0)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulGS;
    
    HairPointDepthPrepass hairPoint;
    float3 tangent = (vertex[1].Position - vertex[0].Position);
    float height = length(tangent);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; //note: this should be the average of the two vertices, and then the other positions also need to change.
    
    
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));
    
            
    float4x3 pos;
    float3 width0 = sideVec * 0.5 * width * vertex[0].width;
    float3 width1 = sideVec * 0.5 * width * vertex[1].width;
    
    pos[0] = vertex[0].Position - width0;
    pos[1] = vertex[0].Position + width0;
    pos[2] = vertex[1].Position - width1;
    pos[3] = vertex[1].Position + width1;
    
#ifdef INTERPOLATION_LOD_GS
    float2x4 hpos;
    for (int i = 0; i < 2; i++) {
        hpos[i] = mul( float4(pos[i],1.0),ViewProjection);
        hpos[i].xy /= hpos[i].w;
        hpos[i].xy = hpos[i].xy * 0.5 + 0.5; // uv
        hpos[i].xy *= float2(g_ScreenWidth, g_ScreenHeight);
    }

    float w1 = length(hpos[1].xy - hpos[0].xy);
    float p = w1 / g_InterpolationLOD;
    if (w1 < g_InterpolationLOD) {
        float rand = (float)(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) / (float)g_NumInterpolatedAttributes;
        if (rand >= p) return;

        float scale = 1.0/p;
        width0 *= scale;
        width1 *= scale;
        
        pos[0] = vertex[0].Position - width0;
        pos[1] = vertex[0].Position + width0;
        pos[2] = vertex[1].Position - width1;
        pos[3] = vertex[1].Position + width1;        
    }
#endif
    
    float4x3 tex;
    int textureIndex = vertex[0].ID & 3;
    tex[0] = float3(0,vertex[0].tex,textureIndex);
    tex[1] = float3(1,vertex[0].tex,textureIndex);
    tex[2] = float3(0,vertex[1].tex,textureIndex);
    tex[3] = float3(1,vertex[1].tex,textureIndex);
    
    
    hairPoint.Position = mul( float4(pos[0],1.0),ViewProjection);
    hairPoint.tex = tex[0];
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[1],1.0),ViewProjection);
    hairPoint.tex = tex[1];
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[2],1.0),ViewProjection);
    hairPoint.tex = tex[2];
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[3],1.0),ViewProjection);
    hairPoint.tex = tex[3];
    stream.Append(hairPoint);

        
    stream.RestartStrip();    
}

[MaxVertexCount(4)]
void InterpolateGSDepthShadows(line HairVertexPWI vertex[2], inout TriangleStream<HairPointDepth> stream)
{
    if (vertex[0].ID < 0 || vertex[1].ID != vertex[0].ID)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulGS;

    HairPointDepth hairPoint;
    float3 tangent = (vertex[1].Position - vertex[0].Position);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; // note: this should be the average of the two vertices, and then the other positions also need to change.
    
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));
            
    float4x3 pos;
    float3 width0 = sideVec * 0.5 * width  * vertex[0].width;
    float3 width1 = sideVec * 0.5 * width  * vertex[1].width;
    
    pos[0] = vertex[0].Position - width0;
    pos[1] = vertex[0].Position + width0;
    pos[2] = vertex[1].Position - width1;
    pos[3] = vertex[1].Position + width1;
    
    float3 Pos;

    Pos = mul(float4(pos[0], 1), HairToWorldTransform).xyz;
    hairPoint.depth = mul(float4(Pos, 1), mLightView).z;
    hairPoint.Position = mul(float4(Pos, 1), ViewProjection);
    stream.Append(hairPoint);

    Pos = mul(float4(pos[1], 1), HairToWorldTransform).xyz;
    hairPoint.depth = mul(float4(Pos, 1), mLightView).z;
    hairPoint.Position = mul(float4(Pos, 1), ViewProjection);
    stream.Append(hairPoint);

    Pos = mul(float4(pos[2], 1), HairToWorldTransform).xyz;
    hairPoint.depth = mul(float4(Pos, 1), mLightView).z;
    hairPoint.Position = mul(float4(Pos, 1), ViewProjection);
    stream.Append(hairPoint);

    Pos = mul(float4(pos[3], 1), HairToWorldTransform).xyz;
    hairPoint.depth = mul(float4(Pos, 1), mLightView).z;
    hairPoint.Position = mul(float4(Pos, 1), ViewProjection);
    stream.Append(hairPoint);

    stream.RestartStrip(); 
}

[MaxVertexCount(1)]
void InterpolateGSMultiStrandCollisions(line HairCollisionVertexWithGS vertex[2], inout PointStream<HairCollisionVertex> stream)
{
    HairCollisionVertex hairPoint;
    hairPoint.Position = vertex[0].Position;
    hairPoint.BarycenticPosition = vertex[0].BarycenticPosition;
	hairPoint.dummy = 0.0f;
    hairPoint.vertexID = vertex[0].vertexID;
    stream.Append(hairPoint);
}

[maxvertexcount(4)]
void InterpolateGS(line HairVertex2 vertex[2], inout TriangleStream<HairPoint> stream)
{
    if (vertex[0].ID < 0 || vertex[0].ID != vertex[1].ID)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulGS;

    HairPoint hairPoint;
    float3 tangent = (vertex[1].Position - vertex[0].Position);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; // note: this should be the average of the two vertices, and then the other positions also need to change.
    
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));
    
    float4x3 pos;
    float3 width0 = sideVec * 0.5 * width * vertex[0].width;
    float3 width1 = sideVec * 0.5 * width * vertex[1].width;
    
    pos[0] = vertex[0].Position - width0;
    pos[1] = vertex[0].Position + width0;
    pos[2] = vertex[1].Position - width1;
    pos[3] = vertex[1].Position + width1;
    
#ifdef INTERPOLATION_LOD_GS
    float2x4 hpos;
    for (int i = 0; i < 2; i++) {
        hpos[i] = mul( float4(pos[i],1.0),ViewProjection);
        hpos[i].xy /= hpos[i].w;
        hpos[i].xy = hpos[i].xy * 0.5 + 0.5; // uv
        hpos[i].xy *= float2(g_ScreenWidth, g_ScreenHeight);
    }

    float w1 = length(hpos[1].xy - hpos[0].xy);
    float p = w1 / g_InterpolationLOD;
    if (w1 < g_InterpolationLOD) {
        float rand = (float)(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) / (float)g_NumInterpolatedAttributes;
        if (rand >= p) return;

        float scale = 1.0/p;
        width0 *= scale;
        width1 *= scale;
        
        pos[0] = vertex[0].Position - width0;
        pos[1] = vertex[0].Position + width0;
        pos[2] = vertex[1].Position - width1;
        pos[3] = vertex[1].Position + width1;        
    }
#endif
    
    float4x2 tex;
    tex[0] = float2(0,vertex[0].tex);
    tex[1] = float2(1,vertex[0].tex);
    tex[2] = float2(0,vertex[1].tex);
    tex[3] = float2(1,vertex[1].tex);       
    
    float2x3 tangents;
    tangents[0] = normalize(vertex[0].Tangent.xyz);
    tangents[1] = normalize(vertex[1].Tangent.xyz);
    hairPoint.scalpTexcoords = vertex[0].scalpTexcoords;
    hairPoint.ID = vertex[0].ID; 

    hairPoint.Position = mul( float4(pos[0],1.0),ViewProjection);
    hairPoint.tex = tex[0];
    hairPoint.shadow = vertex[0].shadow;
    hairPoint.Tangent = tangents[0];
#ifndef SHADOWS_VS
    hairPoint.wPos = pos[0];
#endif
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[1],1.0),ViewProjection);
    hairPoint.tex = tex[1];
    hairPoint.shadow = vertex[0].shadow;
#ifndef SHADOWS_VS
    hairPoint.wPos = pos[1];
#endif
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[2],1.0),ViewProjection);
    hairPoint.tex = tex[2];
    hairPoint.shadow = vertex[1].shadow;
    hairPoint.Tangent = tangents[1];
#ifndef SHADOWS_VS
    hairPoint.wPos = pos[2];
#endif
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[3],1.0),ViewProjection);
    hairPoint.tex = tex[3];
    hairPoint.shadow = vertex[1].shadow;
#ifndef SHADOWS_VS
    hairPoint.wPos = pos[3];
#endif
    stream.Append(hairPoint);

    stream.RestartStrip();    
}

[MaxVertexCount(4)]
void InterpolateGSDensity(line HairVertexPWI vertex[2], inout TriangleStream<HairPointDepthDensity> stream)
{
    if (vertex[0].ID < 0 || vertex[1].ID < 0)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulGS;

    HairPointDepthDensity hairPoint;
    float3 tangent = (vertex[1].Position - vertex[0].Position);
    float height = length(tangent);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; // note: this should be the average of the two vertices, and then the other positions also need to change.
    
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));
            
    float4x3 pos;
    float3 width0 = sideVec * 0.5 * width * vertex[0].width; 
    float3 width1 = sideVec * 0.5 * width * vertex[1].width; 
    
    pos[0] = vertex[0].Position - width0;
    pos[1] = vertex[0].Position + width0;
    pos[2] = vertex[1].Position - width1;
    pos[3] = vertex[1].Position + width1;
    
    hairPoint.posInGrid = mul(float4(pos[0], 1), WorldToGrid );
    hairPoint.Position = mul( float4(pos[0],1.0),ViewProjection);
    stream.Append(hairPoint);

    hairPoint.posInGrid = mul(float4(pos[1], 1), WorldToGrid );
    hairPoint.Position = mul( float4(pos[1],1.0),ViewProjection);
    stream.Append(hairPoint);

    hairPoint.posInGrid = mul(float4(pos[2], 1), WorldToGrid );
    hairPoint.Position = mul( float4(pos[2],1.0),ViewProjection);
    stream.Append(hairPoint);

    hairPoint.posInGrid = mul(float4(pos[3], 1), WorldToGrid );
    hairPoint.Position = mul( float4(pos[3],1.0),ViewProjection);
    stream.Append(hairPoint);
        
    stream.RestartStrip();    
}

[MaxVertexCount(4)]
void InterpolateGSMCollisions(line HairVertexCollisions vertex[2], inout TriangleStream<HairPointCollisions> stream)
{
    if (vertex[0].ID < 0 || vertex[1].ID < 0)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne) * g_widthMulGS;

    HairPointCollisions hairPoint;
    hairPoint.texcoords = vertex[0].texcoords;
    float3 tangent = (vertex[1].Position - vertex[0].Position);
    float height = length(tangent);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; // note: this should be the average of the two vertices, and then the other positions also need to change.
    
    float3 eyeVec = TransformedEyePosition - worldPos;
    float3 sideVec = normalize(cross(eyeVec, tangent));
            
    float4x3 pos;
    float3 width0 = sideVec * 0.5 * width; //note : multiply by the clump width if its anything other than 1
    float3 width1 = sideVec * 0.5 * width; //note : multiply by the clump width if its anything other than 1
    
    pos[0] = vertex[0].Position - width0;
    pos[1] = vertex[0].Position + width0;
    pos[2] = vertex[1].Position - width1;
    pos[3] = vertex[1].Position + width1;
    
    hairPoint.Position = mul( float4(pos[0],1.0),ViewProjection);
    hairPoint.vertexID = vertex[0].vertexID;
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[1],1.0),ViewProjection);
    hairPoint.vertexID = vertex[0].vertexID;
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[2],1.0),ViewProjection);
    hairPoint.vertexID = vertex[1].vertexID;
    stream.Append(hairPoint);

    hairPoint.Position = mul( float4(pos[3],1.0),ViewProjection);
    hairPoint.vertexID = vertex[1].vertexID;
    stream.Append(hairPoint);

    stream.RestartStrip();    
}

void ExpandFacingQuad(float3 eyePos, float3 eyeVec, float3 worldPos, float3 tangent, float height,float width, inout TriangleStream<HairPointGrid> stream)
{
    float3 sideVec = normalize(cross(eyeVec, tangent));
            
    float4x3 pos;
    pos[0] =  worldPos - (sideVec * 0.5*width);
    pos[1] = pos[0] + (sideVec * width);
    pos[2] = pos[0] + (tangent * height);
    pos[3] = pos[2] + (sideVec * width );
    
    HairPointGrid hpOut;

    hpOut.Position = mul( float4(pos[0],1.0),WorldViewProjection);
    hpOut.distance = hpOut.Position.z; 
    stream.Append(hpOut);

    hpOut.Position = mul( float4(pos[1],1.0),WorldViewProjection);
    hpOut.distance = hpOut.Position.z; 
    stream.Append(hpOut);

    hpOut.Position = mul( float4(pos[2],1.0),WorldViewProjection);
    hpOut.distance = hpOut.Position.z; 
    stream.Append(hpOut);

    hpOut.Position = mul( float4(pos[3],1.0),WorldViewProjection);
    hpOut.distance = hpOut.Position.z; 
    stream.Append(hpOut);
        
    stream.RestartStrip();
}

//note! doing too many rasterizations maybe
[MaxVertexCount(8)]
void InterpolateGS2Grid(line HairVertex2Grid vertex[2], inout TriangleStream<HairPointGrid> stream)
{
    if (vertex[0].ID < 0 || vertex[1].ID < 0)
        return;

    float width = g_StrandSizes.Load(vertex[0].ID & g_NumInterpolatedAttributesMinusOne)*g_StrandWidthMultiplier;

    float3 tangent = (vertex[1].Position - vertex[0].Position);
    float height = length(tangent);
    tangent = normalize(tangent);
    float3 worldPos =  vertex[0].Position; // note: this should be the average of the two vertices, and then the other positions also need to change.

    float3 eyeVec;
    
    eyeVec = EyePosition - worldPos;
    ExpandFacingQuad(EyePosition,eyeVec,worldPos,tangent,height,width,stream);
    eyeVec = float3(0,0,1) - worldPos;  //note:  find something better!
    ExpandFacingQuad(float3(0,0,1),eyeVec,worldPos,tangent,height,width,stream);
}

//unrolled loop for efficiency, make sure the amount of implicits dosnt exceed this!
int evaluateVertexInsideImplicits(float3 Position)  
{
     if(0<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[0])
                if(IsInsideSphere(Position,0,float3(1.2,1.2,1.2))) 
                    return 1;  
        
        if(1<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[1])
               if(IsInsideSphere(Position,1,float3(1.2,1.2,1.2))) 
                   return 1;     

        if(2<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[2])
                if(IsInsideSphere(Position,2,float3(1.2,1.2,1.2))) 
                    return 1;     
        
        if(3<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[3])
                if(IsInsideSphere(Position,3,float3(1.2,1.2,1.2))) 
                    return 1;     

        if(4<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[4])        
                if(IsInsideSphere(Position,4,float3(1.2,1.2,1.2))) 
                    return 1;     
        
        if(5<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[5])    
                if(IsInsideSphere(Position,5,float3(1.2,1.2,1.2))) 
                    return 1;     
 
         if(6<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[6])       
                if(IsInsideSphere(Position,6,float3(1.2,1.2,1.2))) 
                    return 1;     
        
        if(7<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[7])
                if(IsInsideSphere(Position,7,float3(1.2,1.2,1.2))) 
                    return 1;     

        if(8<g_NumSphereImplicits)
            if(g_UseSphereForBarycentricInterpolant[8])
                if(IsInsideSphere(Position,8,float3(1.2,1.2,1.2))) 
                    return 1;    

        if(0<g_NumCylinderImplicits)    
            if(g_UseCylinderForBarycentricInterpolant[0])
                if(IsInsideCylinder(Position,0)) 
                    return 1;
        if(1<g_NumCylinderImplicits)    
            if(g_UseCylinderForBarycentricInterpolant[1])        
                if(IsInsideCylinder(Position,1)) 
                    return 1;
        if(2<g_NumCylinderImplicits)    
            if(g_UseCylinderForBarycentricInterpolant[2])
                if(IsInsideCylinder(Position,2)) 
                    return 1;
        
        if(0<g_NumSphereNoMoveImplicits)
            if(g_UseSphereNoMoveForBarycentricInterpolant[0])
                if( IsInsideSphere(Position,SphereNoMoveImplicitInverseTransform[0] )) 
                    return 1;   
        return 0;
}

//find the position
//test if vertex is inside obstacle
//if its inside an obstacle, output vertexID
float4 InterpolatePSMultiStrandCollisions(HairCollisionVertex input) : SV_Target
{     
    float4 posInGrid = mul(float4(input.BarycenticPosition.xyz, 1), WorldToGrid );
    posInGrid.x += 0.5;
    posInGrid.y += 0.5;
    posInGrid.z += 0.5;
    float3 texcoords = float3(posInGrid.x, 1.0-posInGrid.y, posInGrid.z * g_textureDepth);
    float density = Texture_Voxelized_Obstacles.SampleLevel(samLinearClamp,texcoords,0).r;  
    if( density>g_densityThreshold*1.8 )
           return float4(input.vertexID,input.vertexID,input.vertexID,input.vertexID);
                
/*
// reading from a pre-voxelized texture of obstacles is much faster than evaluating them here
//   unrolling loop below, this was much faster than doing the implicit calculations 

    if(evaluateVertexInsideImplicits(BarycenticPosition))
        return float4(input.vertexID,input.vertexID,input.vertexID,input.vertexID);
*/        
    
     return float4(1000,1000,1000,1000);      
}

// visualizing density
float4 RenderPSDensity(HairPoint input) : SV_Target
{
	float4 posInGrid = mul(float4(1,1,1/*input.wPos.xyz*/, 1), WorldToGrid );
	
    posInGrid.x += 0.5;
    posInGrid.y += 0.5;
    posInGrid.z += 0.5;
      
    float3 texcoords = float3(posInGrid.x, 1.0 - posInGrid.y, posInGrid.z * g_textureDepth);
    float density;
    
    //using wrap mode to show where our texture accesses are wrong
    
    if(g_useBlurTexture)
        density = abs(Texture_density_Blur.SampleLevel(samLinearWrap,texcoords,0)).r; 
   	else
   	    density = abs(Texture_density_Demux.SampleLevel(samLinearWrap,texcoords,0)).r; 
   	
    return visualizeDensity(density);
}

float4 RenderPSDensitySmall(HairPointDepthDensity input) : SV_Target
{
	float4 posInGrid = input.posInGrid;
    
    posInGrid.x += 0.5;
    posInGrid.y += 0.5;
    posInGrid.z += 0.5;
      
    float3 texcoords = float3(posInGrid.x, 1.0 - posInGrid.y, posInGrid.z * g_textureDepth);
    float density;
    
    //using wrap mode to show where our texture accesses are wrong
    
    if(g_useBlurTexture)
        density = abs(Texture_density_Blur.SampleLevel(samLinearWrap,texcoords,0)).r; 
   	else
   	    density = abs(Texture_density_Demux.SampleLevel(samLinearWrap,texcoords,0)).r; 
   	
    return visualizeDensity(density);
}

float4 InterpolatePSMCollisions(HairPointCollisions input) : SV_Target
{
    
    float collisions = g_CollisionsTexture.SampleLevel(samLinearWrap,input.texcoords,0).r; 
    if(collisions < input.vertexID)
        return float4(collisions.x/10.0,0,0,1);
    else 
       return float4(0,0,0,1);
}

float SoftEdge(float x)
{
	float d = length(x * 2.0 - 1.0);
    return exp(-g_SoftEdges * d);
}

float4 RenderPSDepthPrepass( HairPointDepthPrepass input) : SV_Target
{
    float4 outputColor = float4(0,0,0,1);
    float4 hairSpecTex = hairTextureArray.Sample( samAniso, float3(input.tex.xyz) );
    
    if(hairSpecTex.x < 0.3)
        return float4(0,0,0,0);

    hairSpecTex.x = hairSpecTex.x + 0.5;     

 	outputColor.a *= hairSpecTex.x; 
 	
 	//outputColor.a = 1;
 	return outputColor;  
 	
// 	return float4(1,0,0,0.5);
}

//all lighting calculations here are done in object space
float4 RenderPS(HairPoint input) : SV_Target
{
    float ksP = g_ksP;
    float ksS = g_ksS;
    float kd = g_kd;
    float specPowerPrimary = g_specPowerPrimary;
    float specPowerSecondary = g_specPowerSecondary;    
    float ksP_sparkles = g_ksP_sparkles;
    float specPowerPrimarySparkles = g_specPowerPrimarySparkles;
    float ka = g_ka;
    
    int ID = input.ID;
    
    float4 outputColor;
    
    //input color from textures
#ifdef USE_TEXTURE_COLOR    

	#ifdef USETEXTURES
		float4 baseColor = (hairTexture.SampleLevel(samLinear,input.scalpTexcoords,0));  
	#else   
	    float4 baseColor = float4(g_StrandColors.Load(ID & g_NumInterpolatedAttributesMinusOne),1);
	#endif  
    float4 specColor = (g_specColor + baseColor)*0.5;
#else   
	float4 baseColor = g_baseColor;
    float4 specColor = g_specColor;
#endif    

    float2 tangentJitter = g_TangentJitter.Load(ID & g_NumInterpolatedAttributesMinusOne);
    //float2 tangentJitter = g_TangentJitter.Load(0);

   	outputColor.xyz = baseColor.xyz;
   	
    int textureArrayIndex = ID & 3;
    float4 hairSpecTex = hairTextureArray.Sample( samAniso, float3(input.tex.xy,textureArrayIndex) ); 
    float4 specNoise = specularNoise.SampleLevel(samAniso,float2(input.tex.x,input.tex.y),0);
    //float hairSpecTex = hairTextureArray.Sample( samAniso, float3(0,0,0) ); 
    //float specNoise = specularNoise.SampleLevel(samAniso,float2(0,0),0);

    hairSpecTex.x = hairSpecTex.x + 0.5;     
	
	//ksP *= hairSpecTex.x;
    //ksS *= hairSpecTex.x;
    kd *= hairSpecTex.x;
    
    //make the roots darker
    
    //if(1)
    if(input.tex.y < 0.025)
    {
        kd *= 0.75;
        ka *= 0.75; 
        ksP *= 0.75;
        ksS *= 0.75;
    }   
	
    // Local geometry
	float3 P = float3(1,1,1);
	//float3 T = float3(1,0,0);
	//float3 L = normalize(float3(1, 1, 1)); //directional light
    //float3 N = float3(0,1,0);
	
	//float3 P = input.wPos;
	float3 L = vLightDirObjectSpace;
	float3 V = normalize(TransformedEyePosition - P);
	float3 T = input.Tangent;
    //re-create the normal vector:
    float3 sideVec = normalize(cross(V, T));
    float3 N = normalize(cross(T,sideVec));

    
	T += 0.2 * ( N.xyz*(hairSpecTex.x - 0.5)  );
	T = normalize(T);
	
	float3 TPrimary = T;
	TPrimary += (0.3*tangentJitter.x)*N; //jitter each hair strand tangent. primary shifted towards tip
	TPrimary += 0.1*( N.xyz*(specNoise.x - 0.5)  );
	TPrimary = normalize(TPrimary);
	
	float3 TSecondary = T;
	TSecondary += (-0.3 + 0.4*tangentJitter.y)*N; //jitter each hair strand tangent. primary shifted towards tip
	TSecondary += 0.1*( N.xyz*(specNoise.x - 0.5)  );
	TSecondary = normalize(TSecondary);	


	
	
#ifdef SHADOWS_VS
	//float shadow = 0;
	float shadow = input.shadow;
#else
	float shadow = 0.0f;
	if(g_useShadows)
    {
		float3 Pos = mul(float4(input.wPos, 1), HairToWorldTransform).xyz;
		float2 texcoords = WorldToLightCoord(Pos);
		float z = WorldToDepth(Pos);
		//TO DO SHADOWS: incorporate the HairToWorldTransform in mLightViewProjClip2Tex : float2 texcoords = WorldToLightCoord(input.wPos);
		//TO DO SHADOWS: incorporate the HairToWorldTransform in mLightView : float z = WorldToDepth(input.wPos);	 
		shadow = ShadowPCF(texcoords, z);
    }
#endif

   //kajiya and kay lighting
    
    //diffuse
    float TdotL = dot( T , L);
    float sinTL = sqrt( 1 - TdotL*TdotL );

    //specular primary
    float TPdotL = dot( TPrimary , L);
    float sinTPL = sqrt( 1 - TPdotL*TPdotL );
    float TPdotE = dot( TPrimary , V);
    float sinTPE = sqrt( 1 - TPdotE*TPdotE );

    //specular secondary
    float TSdotL = dot( TSecondary , L);
    float sinTSL = sqrt( 1 - TSdotL*TSdotL );
    float TSdotE = dot( TSecondary , V);
    float sinTSE = sqrt( 1 - TSdotE*TSdotE );

    float lightDiffuse = 1-shadow;
    float lightSpec = pow(lightDiffuse,2);
    
    float specPrimary = abs((TPdotL*TPdotE + sinTPL*sinTPE));
    
    outputColor.xyz = ka*outputColor.xyz                       //ambient light
                      + lightDiffuse*(kd*sinTL*outputColor.xyz)  //diffuse light
                      + lightSpec*(ksP*pow( specPrimary,specPowerPrimary).xxx)  //primary specular light
                      + lightSpec*(ksS*pow( abs((TSdotL*TSdotE + sinTSL*sinTSE)),specPowerSecondary).xxx)*specColor.xyz //secondary specular light                      
                      + lightSpec*(ksP_sparkles*pow( specPrimary,specPowerPrimarySparkles).xxx) //sparkles
                      ;
                                      
    outputColor.a = g_alpha;

    return outputColor;
}

struct Depth_PSOut
{
	float Z: SV_Target0;
};

Depth_PSOut RenderDepthPSSmall(HairPointDepth input)
{
	Depth_PSOut output;
	output.Z = input.depth;
	return output;
}

void VoxelizeDensity3(inout float4 RT, float4 Zmin, float4 Zmax, float z)
{
	float A = RGBA8_PRECISION_DENSITY;
	if (z >= Zmin.x) RT.x = A;
	if (z >= Zmin.y) RT.y = A;
	if (z >= Zmin.z) RT.z = A;
	RT.w = 10000;	
}

//this version of deep shadow maps shader just does 4 layers.
//to do more layers go to the function RenderDepthPSSmall_DOM_MRT
float4 RenderDepthPSSmall_DOM(HairPointDepth input) : SV_Target
{
	float4 output = float4(0,0,0,0);
	
	float2 tex = float2(input.Position.x/g_lightBufferRes, input.Position.y/g_lightBufferRes);
	float startDepth = tHairDepthMap.SampleLevel(samPointClamp, tex.xy, 0 ).x;

	VoxelizeDensity3(output, g_Zmin[0], g_Zmax[0], input.depth - startDepth);
	output.w = startDepth;

	return output;
}

struct Opacity_PSOut
{
	float4 RT0 : SV_Target0;
	float4 RT1 : SV_Target1;
	float4 RT2 : SV_Target2;
	float4 RT3 : SV_Target3;
};

/*
void VoxelizeDensity(inout float4 RT, float4 Zmin, float4 Zmax, float z)
{
	float A = RGBA8_PRECISION_DENSITY;
	
	if (z >= Zmin.x) RT.x = A;
	if (z >= Zmin.y) RT.y = A;
	if (z >= Zmin.z) RT.z = A;
	if (z >= Zmin.w) RT.w = A;

}

Opacity_PSOut RenderDepthPSSmall_DOM_MRT(HairPointDepth input)
{
	Opacity_PSOut OUT = (Opacity_PSOut)0.0f;
	OUT.RT0.x = input.depth;
	float z = input.depth;
	float A = RGBA8_PRECISION_DENSITY;

	if (z >= g_Zmin[0].x && z < g_Zmax[0].x) OUT.RT0.y = A;
	if (z >= g_Zmin[0].y && z < g_Zmax[0].y) OUT.RT0.z = A;
	if (z >= g_Zmin[0].z && z < g_Zmax[0].z) OUT.RT0.w = A;

	VoxelizeDensity(OUT.RT1, g_Zmin[1], g_Zmax[1], z);
	VoxelizeDensity(OUT.RT2, g_Zmin[2], g_Zmax[2], z);
	VoxelizeDensity(OUT.RT3, g_Zmin[3], g_Zmax[3], z);

	return OUT;
}*/

struct PostProc_VSOut
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD;
};

//Vertex shader that generates a full screen triangle with texcoords
//To use draw 3 vertices with primitive type triangle strip
PostProc_VSOut FullScreenTriVS( uint id : SV_VertexID )
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    output.tex = float2( (id << 1) & 2, id & 2 );
    output.pos = float4( output.tex * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
    return output;
}

float4 VisDepthsPS ( PostProc_VSOut IN ) : SV_TARGET
{
    float4 val = tShadowMap.SampleLevel(samPointClamp, IN.tex.xy, 0 );
	float4 output = float4(0,0,0,0);

	output.x = (val.x - g_ZNear) / (g_ZFar - g_ZNear);

	return output;
}

float4 CopyTexturePS ( PostProc_VSOut IN ) : SV_TARGET
{
    float z0 = tHairDepthMap.SampleLevel(samPointClamp, IN.tex.xy, 0 ).x;
    return float4(0,0,0,z0);
}

struct MRT_PSOut
{
	float4 RT0 : SV_Target0;
	float4 RT1 : SV_Target1;
	float4 RT2 : SV_Target2;
	float4 RT3 : SV_Target3;
	float4 RT4 : SV_Target4;	
	float4 RT5 : SV_Target5;
	float4 RT6 : SV_Target6;
	float4 RT7 : SV_Target7;		
};

void addDensityToMRT(inout float4 outputCol,int rgbaIndex)
{
    if(rgbaIndex == 0)
        outputCol.x = 1; 
    else if(rgbaIndex == 1)
        outputCol.y = 1;
    else if(rgbaIndex == 2)
        outputCol.z = 1; 
    else 
        outputCol.w = 1; 
}

MRT_PSOut RenderPSForGridMRT(HairPointGrid input)
{
   MRT_PSOut output = (MRT_PSOut)0;
   
   if(input.distance<0 || input.distance>1)
       return output;
   
   int rtIndex = floor(input.distance * 8.0);
   int rgbaIndex = 0x3 & int(input.distance * 32.0);

   if(rtIndex==0)    
	   addDensityToMRT(output.RT0,rgbaIndex);
   if(rtIndex==1)
	   addDensityToMRT(output.RT1,rgbaIndex);
   if(rtIndex==2)
	   addDensityToMRT(output.RT2,rgbaIndex);
   if(rtIndex==3)
	   addDensityToMRT(output.RT3,rgbaIndex);
   if(rtIndex==4)
	   addDensityToMRT(output.RT4,rgbaIndex);
   if(rtIndex==5)
	   addDensityToMRT(output.RT5,rgbaIndex);
   if(rtIndex==6)
	   addDensityToMRT(output.RT6,rgbaIndex);		   
   if(rtIndex==7) 
	   addDensityToMRT(output.RT7,rgbaIndex); 
   
   return output;  
}

//    PCF for Mesh and floor rendering
SamplerComparisonState  FILT_PCF
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ComparisonFunc = Less_Equal;
};

float BoxFilterStart( float fWidth )  //Assumes filter is odd
{
    return ( ( fWidth - 1.0f ) / 2.0f );
}

float DOM_FILTER( float2 tex, float fragDepth, float filterWidth )
{
    //PreShader - This should all be optimized away by the compiler
    //====================================
    float fStartOffset = BoxFilterStart( filterWidth );
    float texOffset = 1.0f / fg_textureWidth;
    //====================================
    
    fragDepth -= 0.3f;
    tex -= fStartOffset * texOffset;
    
    float lit = 0.0f;
		for( int i = 0; i < filterWidth; ++i )
			for( int j = 0; j < filterWidth; ++j )
			{	
				lit += 1.0f - DOMContribution(fragDepth, float2( tex.x + i * texOffset, tex.y + j * texOffset ), int2(0,0) );//tex, int2(i*texOffset, j*texOffset));
			}
	return lit / ( filterWidth * filterWidth );
	
}

float PCF_FILTER( float2 tex, float fragDepth, float filterWidth, Texture2D textureIn )
{
    //PreShader - This should all be optimized away by the compiler
    //====================================
    float fStartOffset = BoxFilterStart( filterWidth );
    float texOffset = 1.0f / fg_textureWidth;
    //====================================
    
    fragDepth -= 0.3f;
    tex -= fStartOffset * texOffset;
    
    float lit = 0.0f;
		for( int i = 0; i < filterWidth; ++i )
			for( int j = 0; j < filterWidth; ++j )
			{	
				lit += textureIn.SampleCmpLevelZero( FILT_PCF,    float2( tex.x + i * texOffset, tex.y + j * texOffset ), fragDepth );
			}
	return lit / ( filterWidth * filterWidth );
}


float SHADOW_FILTER( float2 tex, float fragDepth, float filterWidth )
{
	return PCF_FILTER( tex, fragDepth, filterWidth, tShadowMap );
}

//    Mesh rendering
struct MeshVertexIn
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent  : TANGENT;
};

struct MeshVertexOut
{
    float4 Position : SV_Position;
    float3 Normal   : Normal;
    float2 TexCoord : TexCoord;
    float Depth     : DEPTH;
    float3 wpos : WPOS; 
};


struct MeshVertexOutShadows
{
    float4 Position : SV_Position;
    float3 Normal   : Normal;
    float2 TexCoord : TexCoord;
    float Depth     : DEPTH;
    float3 wpos : WPOS;
    float2 lightTexCoords : ltc;
};

struct PsCnDOutput
{
    float4 Color    : SV_Target0;
};

MeshVertexOut MeshVS(MeshVertexIn vertexIn)
{
    MeshVertexOut vertex;
    vertex.Position = mul(float4(vertexIn.Position.xyz, 1), ViewProjection);
    vertex.Normal = normalize(vertexIn.Normal);
    vertex.TexCoord = vertexIn.TexCoord;
    vertex.Depth = vertex.Position.z/vertex.Position.w;
    vertex.wpos =  mul(float4(vertexIn.Position.xyz, 1), TotalTransformation).xyz; 
    return vertex;
}

PsCnDOutput MeshPS(MeshVertexOut vertex)
{   
    PsCnDOutput ret;
    
    float3 ambientColor = float3(1.0f,1.0f,1.0f);
    if(g_useScalpTexture)
        ambientColor = hairTexture.Sample(samLinear, vertex.TexCoord).xyz;       
    else    
	{
		ambientColor = meshAOMap.Sample(samLinear, vertex.TexCoord).xyz;   
        ambientColor = clamp( ambientColor,0.6,1.0);
	}

    float kd=1.0;
    float ks=0.1;
    float3 L = vLightDir;
    float diffuseLighting   = dot(-vertex.Normal, L);
    diffuseLighting = 0.5f*(diffuseLighting)+0.5f;
  	float3 E = normalize(TransformedEyePosition - vertex.wpos);
  	float3 R = reflect(E,vertex.Normal);
    float specularLighting = pow(max(dot(R,L),0.0),5);
      
    ret.Color = float4((diffuseLighting*kd + specularLighting*ks)*ambientColor,1);
    
    return ret;
}

MeshVertexOutShadows MeshVSShadows(MeshVertexIn vertexIn)
{
    MeshVertexOutShadows vertex;
    vertex.Position = mul(float4(vertexIn.Position, 1), ViewProjection);
    vertex.Normal = normalize(vertexIn.Normal);
    vertex.TexCoord = vertexIn.TexCoord;
    vertex.Depth = vertex.Position.z/vertex.Position.w;
    float3 wpos = mul(float4(vertexIn.Position, 1), TotalTransformation).xyz; 
    vertex.wpos = wpos;
    vertex.lightTexCoords = mul(float4(wpos.xyz, 1), mLightViewProjClip2Tex).xy;
    return vertex;
}

PsCnDOutput MeshPSShadows(MeshVertexOutShadows vertex)
{   
    PsCnDOutput ret;
    
    float3 ambientColor = float3(1.0f,1.0f,1.0f);
    if(!g_useScalpTexture)    
        ambientColor = meshAOMap.Sample(samLinear, vertex.TexCoord).xyz;   
    ambientColor = clamp( ambientColor,0.6,1.0);
    float3 ambient = ambientColor*0.15;

    float lit = 1.0f;
	if(g_useShadows)
	{
		float fDepth = mul(float4(vertex.wpos, 1), mLightView).z;
        lit = SHADOW_FILTER( vertex.lightTexCoords, fDepth, 22 );
	}

    float kd=0.85;
    float ks=0.1;
    float3 L = vLightDir;
    float diffuseLighting   = dot(-vertex.Normal, L);
    diffuseLighting = 0.5f*(diffuseLighting)+0.5f;
    
    //and a little bit of static hero lighting for the face which never changes
    float3 L2 = float3(-0.102307, -0.729127, 0.676688);
    float key = 0.15*(ambient.x + saturate(dot(-vertex.Normal, L2))).x;

    
  	float3 E = normalize(TransformedEyePosition - vertex.wpos);
  	float3 R = reflect(E,vertex.Normal);
    float specularLighting = pow(max(dot(R,L),0.0),5);
      
    ret.Color = float4((diffuseLighting*kd + specularLighting*ks)*lit*ambientColor + key,1);
    
    return ret;
}

//render the plane
struct VSPlaneIn
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
};

struct PSIn
{
    float4 pos : SV_Position;
    float3 wNorm : wNorm;
    float3 wLight : wLight;
    float fDepth : fDepth;
    float2 lightTexCoords : ltc;
};

PSIn VSDrawPlane( VSPlaneIn input )
{
    PSIn output = (PSIn)0.0f;
    
    output.pos = mul(float4(input.pos.xyz, 1), ViewProjection);
    output.wNorm = input.norm;
  
    float3 lightVec = vLightPos.xyz - input.pos.xyz;
    output.wLight = normalize( lightVec );
    output.fDepth = mul(float4(input.pos.xyz, 1), mLightView).z;
    output.lightTexCoords = mul(float4(input.pos.xyz, 1), mLightViewProjClip2Tex).xy;

    return output;
}

float4 PSDrawPlane( uniform int g_useShadows, PSIn input ) : SV_Target
{
	float4 output = (float4)0.0f;
	
    float lit = 1.0f;
    
    if(g_useShadows)
        lit = SHADOW_FILTER( input.lightTexCoords, input.fDepth, 16 );

	float3 wLight = normalize( input.wLight );
	float3 wNormal;
	
    wNormal = normalize( input.wNorm );

	float diffuseLighting = 1.0f;
	diffuseLighting *= dot( normalize( wNormal ), wLight );
	//diffuseLighting = clamp((diffuseLighting - 0.75)*4,0,1);
	//diffuseLighting = clamp((diffuseLighting-0.5),0,1)*1.3684 + 0.3176f;
	return  float4( 1.0f, 1.0f, 1.0f, 1.0f ) * diffuseLighting * lit;
}

//render collision objects-----------------------------------------------
MeshVertexOut CollisionVS(MeshVertexIn vertexIn)
{
    MeshVertexOut vertex;
    float3 Pos = vertexIn.Position;
    bool render = true;
    if(render)
    { 
        Pos = mul(float4(Pos,1), CollisionSphereTransformations[g_currentCollisionSphere]).xyz;
        //Pos = mul(float4(Pos,1), SphereNoMoveImplicitTransform[g_currentCollisionSphere]).xyz;
        //Pos = mul(float4(Pos,1), CollisionCylinderTransformations[g_currentCollisionSphere]).xyz;
        Pos = mul(float4(Pos,1), HairToWorldTransform).xyz;
    }
    else
        Pos = float3(1000,1000,1000);
    vertex.Position = mul(float4(Pos, 1), ViewProjection);
    vertex.Normal = normalize(vertexIn.Normal);
    vertex.TexCoord = vertexIn.TexCoord;
    return vertex;
}

float4 CollisionPS(MeshVertexOut vertex) : SV_Target
{   
    return float4(1,0,0,1);
}

//note: the code for rendering skinned meshes to the shadow map is further below under RenderSkinnedMeshDepth
struct MeshDepthVertexOut
{
    float4 Position : SV_Position;
    float Depth : texcoord;
};

MeshDepthVertexOut MeshDepthVS(MeshVertexIn vertexIn)
{
	MeshDepthVertexOut vertex;
	//TO DO SHADOWS: put TotalTransformation inside the mLightViewProj matrix; then we dont need to multiply by TotalTransformation below
	float3 wPos = mul(float4(vertexIn.Position, 1), TotalTransformation).xyz;
    vertex.Position = mul(float4(wPos, 1), mLightViewProj);
    vertex.Depth = WorldToDepth(wPos);
    return vertex;
}

Depth_PSOut MeshDepthPS(MeshDepthVertexOut vertex)
{
	Depth_PSOut output;
	output.Z = vertex.Depth;
	return output;
}

float4 MeshDepthPS_DOM(MeshDepthVertexOut vertex) : SV_Target
{
	float4 output = float4(0,0,0,0);
	float2 tex = float2(vertex.Position.x/g_lightBufferRes, vertex.Position.y/g_lightBufferRes);
	float startDepth = tHairDepthMap.SampleLevel(samPointClamp, tex.xy, 0 ).x;
	VoxelizeDensity3(output, g_Zmin[0], g_Zmax[0], vertex.Depth - startDepth);
	output.w = startDepth;
	return output;
}

//    arrow and cube rendering
struct VS_INPUT_ARROW
{ 
    float3 Position          : POSITION;
};

struct VS_OUTPUT_ARROW
{
    float4 Position          : SV_POSITION;
    float Depth              : DEPTH;
};

//vertex shader for rendering the arrow 
VS_OUTPUT_ARROW VSArrow( VS_INPUT_ARROW input )
{
    VS_OUTPUT_ARROW output = (VS_OUTPUT_ARROW)0;
    output.Position = mul( float4(input.Position, 1), ViewProjection );
    output.Depth = output.Position.z/output.Position.w;
    return output;
}

//pixel shader for arrow
PsCnDOutput PSArrow(VS_OUTPUT_ARROW In)
{  
    PsCnDOutput ret;
    ret.Color = g_arrowColor;
//    ret.Depth = In.Depth;
    return ret;
}

//inter-hair interaction and wind hair interaction
//display hair density texture------------------------------------------------------------
struct VS_INPUT_POSITION_TEX 
{
    float4 Position  : POSITION;
    float3 Texcoords : TEXCOORD;
    
};

struct VS_OUTPUT_POSITION_TEX
{
    float4 Position  : SV_POSITION;
    float3 Texcoords : TEXCOORD0;

};

struct VS_INPUT_FLUIDSIM
{
    float3 position          : POSITION;    // 2D slice vertex coordinates in clip space
    float3 textureCoords0    : TEXCOORD;    // 3D cell coordinates (x,y,z in 0-dimension range)
};

struct VS_OUTPUT_FLUIDSIM
{
    float4 pos               : SV_Position;
    float3 texcoords         : TEXCOORD1;
    float rtindex            : rtindex;
};

struct GS_OUTPUT_FLUIDSIM
{
    float4 pos               : SV_Position; // 2D slice vertex coordinates in homogenous clip space
    float3 texcoords         : TEXCOORD1;   // 3D cell texcoords (x,y,z in 0-1 range)
    uint RTIndex             : SV_RenderTargetArrayIndex;  // used to choose the destination slice
};

VS_OUTPUT_POSITION_TEX VS_GRID( VS_INPUT_POSITION_TEX input)
{
    VS_OUTPUT_POSITION_TEX output = (VS_OUTPUT_POSITION_TEX)0;

    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1.0);
    output.Texcoords = float3( (input.Texcoords.x)/(g_textureWidth),
                               (input.Texcoords.y)/(g_textureHeight), 
                                input.Texcoords.z);                   
    return output;
}

VS_OUTPUT_FLUIDSIM VS_GRID_FLUIDSIM( VS_INPUT_FLUIDSIM input)
{
    VS_OUTPUT_FLUIDSIM output = (VS_OUTPUT_FLUIDSIM)0;

    output.pos = float4(input.position.x, input.position.y, input.position.z, 1.0);
    output.rtindex = input.textureCoords0.z;
    output.texcoords = float3( (input.textureCoords0.x)/(fluidg_textureWidth),
                               (input.textureCoords0.y)/(fluidg_textureHeight), 
                               (input.textureCoords0.z)/(fluidg_textureDepth)*g_textureDepth); 
    return output;
}

[maxvertexcount (3)]
void GS_ARRAY(triangle VS_OUTPUT_FLUIDSIM In[3], inout TriangleStream<GS_OUTPUT_FLUIDSIM> triStream)
{
    GS_OUTPUT_FLUIDSIM Out;
    Out.RTIndex = In[0].rtindex;

    Out.pos          = In[0].pos; 
    Out.texcoords    = In[0].texcoords;
    triStream.Append( Out );

    Out.pos          = In[1].pos; 
    Out.texcoords    = In[1].texcoords;
    triStream.Append( Out );

    Out.pos          = In[2].pos; 
    Out.texcoords    = In[2].texcoords;
    triStream.Append( Out );

    triStream.RestartStrip( );
}

float4 PS_DEMUX_TO_3D_OBSTACLE_TEX( GS_OUTPUT_FLUIDSIM input ) : SV_Target
{
    //-----------------------------------
    //find if the cell is inside an obstacle
        
    float returnVal;
    
    float4 posInWorld = mul( float4(input.texcoords.x-0.5, 0.5-input.texcoords.y,(input.texcoords.z/g_textureDepth) - 0.5,1),GridToWorld);
    
    for(int i=0;i<g_NumSphereImplicits;i++)
        if(PartitionInOutBoundarySphere(posInWorld.xyz,0.4,i,returnVal))
            return returnVal;
            
    for(i=0;i<g_NumCylinderImplicits;i++)
        if(PartitionInOutBoundaryCylinder(posInWorld.xyz,0.2,i,returnVal))
            return returnVal;        
   
    //-----------------------------------
    float hairDensity = OBSTACLE_EXTERIOR;
    
#ifdef USE_HAIR_DENSITY    
    //if we want to use this, doVelocityAttenuation also has to be set to true in fluidSim.fx    
        
    hairDensity = abs(Texture_density_Blur.SampleLevel(samLinearClamp,input.texcoords,0)).r; 
    //map hair density so that max hair goes to 0.7 and no hair goes to 1
    //this is inline with the definitions that fluid sim uses for its obstacles
    hairDensity /= g_densityThreshold; //0: no density, 1: full density
    hairDensity = clamp(hairDensity,0,1);
    hairDensity =  1 - hairDensity;
    hairDensity *= 0.3;
    hairDensity += 0.7;
#endif

    return hairDensity;                 
}

float4 PS_RESOLVE_SUPERSAMPLE( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    //simple bilerp   
    float4 color = g_SupersampledSceneColor.SampleLevel(samLinear,input.Texcoords.xy,0);
 
    //more taps: these dont cost a lot but improve the look
    float TexelX = 1.0/(g_SSg_textureWidth);
    float TexelY = 1.0/(g_SSg_textureHeight);
    color += g_SupersampledSceneColor.SampleLevel(samLinear,input.Texcoords.xy + float2(-0.9*TexelX, 0.4*TexelY),0);
    color += g_SupersampledSceneColor.SampleLevel(samLinear,input.Texcoords.xy + float2( 0.4*TexelX, 0.9*TexelY),0);
    color += g_SupersampledSceneColor.SampleLevel(samLinear,input.Texcoords.xy + float2( 0.9*TexelX,-0.4*TexelY),0);
    color += g_SupersampledSceneColor.SampleLevel(samLinear,input.Texcoords.xy + float2(-0.4*TexelX,-0.9*TexelY),0);           
    color /= 5.0;

    return color;
}

struct PsOutColorDepth
{
    float  Depth    : SV_Depth;
};

PsOutColorDepth PS_RECONSTRUCT_DEPTH( VS_OUTPUT_POSITION_TEX input )
{
    PsOutColorDepth ret;
    ret.Depth = g_sceneDepth.SampleLevel(samLinear,input.Texcoords.xy,0).x;
    return ret; 
}

float4 PS_DRAW_TEXTURE( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    if( uint(input.Position.x) % g_rowWidth == 0 || uint(input.Position.y) % g_colWidth == 0)
        return float4(1,0,0,1);
    
    int zSlice = floor(input.Texcoords.z/4.0);
    int rgba = 0x3 & int(input.Texcoords.z);

    float3 texcoords = float3(input.Texcoords.xy,zSlice);
    float4 density = abs(Texture_density.SampleLevel(samPointClamp,texcoords,0));
  
    if(rgba == 0)
        return visualizeDensity(density.r); 
    else if(rgba == 1)
        return visualizeDensity(density.g); 
    else if(rgba == 2)
        return visualizeDensity(density.b); 
    else
        return visualizeDensity(density.a); 
                 
}

float4 PS_DRAW_TEXTURE_DEMUX( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    if( uint(input.Position.x) % g_rowWidth == 0 || uint(input.Position.y) % g_colWidth == 0)
        return float4(1,0,0,1);
    
    float density = 0.0f;
    if(g_textureIndex == 1)
        density = abs(Texture_density_Demux.SampleLevel(samPointClamp,input.Texcoords.xyz,0)).r;
    else if(g_textureIndex == 2)
        density = abs(Texture_density_Blur_Temp.SampleLevel(samPointClamp,input.Texcoords.xyz,0)).r;
    else if(g_textureIndex == 3)
        density = abs(Texture_density_Blur.SampleLevel(samPointClamp,input.Texcoords.xyz,0)).r;    
    else
        return float4(1,1,0,1);   

    return visualizeDensity(density);                  
}

struct VS_INPUT_COLLISIONS
{
    float3 Position  : POSITION;
    float3 Texcoords : TEXCOORD;
    
};

VS_OUTPUT_POSITION_TEX VS_COLLISIONS( VS_INPUT_COLLISIONS input)
{
    VS_OUTPUT_POSITION_TEX output = (VS_OUTPUT_POSITION_TEX)0;

    output.Position = float4(input.Position.x, input.Position.y, input.Position.z, 1.0);
    output.Texcoords = float3(input.Texcoords.x,input.Texcoords.y,0);            
    return output;
}

float4 PS_DRAW_COLLISIONS_TEXTURE( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    float collisions = g_CollisionsTexture.SampleLevel(samLinearClamp,input.Texcoords.xy,0).r;   
    return float4(collisions.xxx/100.0,1); 
    
}

float4 PS_VOXELIZE_OBSTACLES( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    float4 posInWorld = mul( float4(input.Texcoords.x-0.5, 0.5-input.Texcoords.y, g_gridZIndex/g_textureDepth - 0.5,1),GridToWorld);
    
    if(evaluateVertexInsideImplicits(posInWorld.xyz))
         return g_densityThreshold*2 + 0.0001; 
                                    
    return float4(0,0,0,0);        
}

float4 PS_DEMUX( VS_OUTPUT_POSITION_TEX input ) : SV_Target
{
    float obstacleDensity = Texture_Voxelized_Obstacles.SampleLevel(samPointClamp,float3(input.Texcoords.xy,g_gridZIndex),0).r;
    if(obstacleDensity>0)
        return obstacleDensity;
   
    //-----------------------------------
    
    int zSlice = floor(g_gridZIndex/4.0);
    int rgba = 0x3 & g_gridZIndex;

    float3 texcoords = float3(input.Texcoords.xy,zSlice);
    float4 density = abs(Texture_density.SampleLevel(samPointClamp,texcoords,0));
  
    if(rgba == 0)
        return density.r; 
    else if(rgba == 1)
        return density.g; 
    else if(rgba == 2)
        return density.b; 
    else
        return density.a; 
                 
}