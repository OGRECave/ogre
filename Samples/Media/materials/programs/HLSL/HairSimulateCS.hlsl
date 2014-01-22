/*
----------------------------------------------------------
Hair Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

#define MAX_IMPLICITS 10
//constants that change frame to frame

//ATTENTION TO THESE. SET THEM CORRECTLY. ADD COMPUTE SHADER TO THE MATERIAL>
cbuffer cbPerFrame : register( b0 )
{
    row_major float4x4 additionalTransformation; //use this transform to transform all the vertices. We are using this to transform hair vertices back to the vicinity of the head after simulation is turned off and on again
    row_major float4x4 RootTransformation;
    row_major float4x4 currentHairTransformation;
    row_major float4x4 WorldToGrid;
    
    int g_bApplyHorizontalForce; //bool
    int g_bAddGravity; //bool
    int g_numConstraintIterations;
    int g_numCFIterations;

	float g_angularStiffness;
    float g_gravityStrength;
    float g_TimeStep;
    int g_integrate; 

	int g_bApplyAdditionalTransform;
	int g_restoreToDefault;
	float g_blendAxis; //for coordinate frame correction
	int g_addAngularForces;
}

//constants that are set only once
cbuffer cbOnceOnly  : register( b1 )
{
    int g_NumSphereImplicits;
    int g_NumCylinderImplicits;
    int g_NumSphereNoMoveImplicits;
    int padding4;
    
    row_major float4x4 CollisionSphereTransformations[MAX_IMPLICITS];
    row_major float4x4 CollisionSphereInverseTransformations[MAX_IMPLICITS];
    row_major float4x4 CollisionCylinderTransformations[MAX_IMPLICITS];
    row_major float4x4 CollisionCylinderInverseTransformations[MAX_IMPLICITS];
    row_major float4x4 SphereNoMoveImplicitInverseTransform[MAX_IMPLICITS];
}

RWStructuredBuffer<float4> particlePositions : register(u0); //particle positions, for the  solver 
RWStructuredBuffer<float4> previousParticlePositions : register(u1); //the partice positions from the last frame
RWStructuredBuffer<float4> g_coordinateFrames : register(u2); //the coordinate frames for the particles. we have to update these as the hair moves and also use them for simulation

Buffer<int> g_SimulationStrandOffsets : register(t0); //note - maybe this should have been a short along with the corresponding buffer and SRV
Buffer<float> particleDistanceConstraintLengths : register(t1);
Buffer<float> g_stiffness : register(t2); //TODO! these are angular forces, and might be something more..have to get rid of these by the end since will be computing everything in one shader
Buffer<float4> g_OriginalMasterStrand : register(t3);
Texture3D g_FluidVelocityTexture : register(t4);
Buffer<float2> particleAngularLengthAndStiffness : register(t5); 
Buffer<float3> g_OriginalVectors : register(t6); 

//data structures:
struct ConstraintsAngularHairVertex 
{
    float4 Position : POSITION;
    float Length : LENGTH;
    float Stiffness : STIFFNESS;
    uint vid : ID;
};

struct coordinateFrame4
{
    float4 xAxis : X_AXIS;
	float4 yAxis : Y_AXIS;
	float4 zAxis : Z_AXIS;
};

//samplers
SamplerState samLinearClamp : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp; 
};

//-----------------------------------------------------------------------------------------
#define SMALL_NUM 0.0001
void rotateVector(float3 rotationAxis, float theta, float3 prevVec, inout float3 newVec, float c, float s)
{
	float3 axisDifference = rotationAxis - prevVec;
    if( length(axisDifference)<SMALL_NUM || theta < SMALL_NUM )
	{
	    newVec = prevVec;
		return;
	}

	//note: dont need to calculate these since they are being passed already
	c = cos(theta);
    s = sin(theta);
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

//assumes x is already normalized, and x stays as it is
void GramSchmidtOrthoNormalize( float3 x, inout float3 y, inout float3 z )
{
	y = y - dot(x,y)*x;
	y = normalize(y);

	z = z - dot(x,z)*x - dot(y,z)*y;
	z = normalize(z);
}

coordinateFrame4 UpdateCoordinateFrame(coordinateFrame4 cf, float4 vertex1, float4 vertex2)
{
    if(length(cf.xAxis)<0.001)
        return cf; //this is the cf at the end and its supposed to be zero; no update needed
   
   
    float blendAxis = g_blendAxis;

    //find the x axis
    coordinateFrame4 output;
    output.xAxis.xyz = blendAxis*normalize(vertex2.xyz - vertex1.xyz) + (1-blendAxis)*cf.xAxis.xyz;
    output.xAxis.xyz = normalize(output.xAxis.xyz);
    output.xAxis.w = 0;
    
    //find the rotation  
    float3 xRotationVector = cross( normalize(output.xAxis.xyz), normalize(cf.xAxis.xyz));
    float sinTheta = length(xRotationVector);
	float cosTheta =  dot( normalize(output.xAxis.xyz), normalize(cf.xAxis.xyz));
    float theta = asin(sinTheta);
	xRotationVector = normalize(xRotationVector);
	
	//find the y and z axis
	float3 y,z;	
 	rotateVector(xRotationVector, theta, cf.yAxis.xyz, y, cosTheta, sinTheta);
    rotateVector(xRotationVector, theta, cf.zAxis.xyz, z, cosTheta, sinTheta);
    y = normalize(y);
    z = normalize(z);			  
    
	// we could also do this for extra assurance
	//GramSchmidtOrthoNormalize(output.xAxis.xyz, y, z );
	
	
	//assign y and z axis
	output.yAxis.xyz = y;
	output.zAxis.xyz = z;
	output.yAxis.w = output.zAxis.w = 0;

    return output;
}

coordinateFrame4 PropagateCoordinateFrame(coordinateFrame4 cf, coordinateFrame4 prevCF)
{
    if(length(cf.xAxis)<0.01)
        return cf; //this is the cf at the end and its supposed to be zero; no update needed
         
    coordinateFrame4 output;
    
    //x axis
    // assume this is already correct in the existing CF. this only works if called after update CF
    output.xAxis = cf.xAxis;
    
    //find the rotation  
    float3 xRotationVector = cross( normalize(output.xAxis.xyz), normalize(prevCF.xAxis.xyz));// the vector around which the x axis has rotated
    float sinTheta = length(xRotationVector); 
    float theta = asin(sinTheta); //the amount of rotation
	float cosTheta = dot(normalize(output.xAxis.xyz), normalize(prevCF.xAxis.xyz));
	//float theta = acos(cosTheta);
	xRotationVector = normalize(xRotationVector);

	//find the y and z axis
	float3 y,z;	
 	rotateVector(xRotationVector, theta, prevCF.yAxis.xyz, y, cosTheta, sinTheta);
    rotateVector(xRotationVector, theta, prevCF.zAxis.xyz, z, cosTheta, sinTheta);
    y = normalize(y);
    z = normalize(z);			  
    
	//assign y and z axis
	output.yAxis.xyz = y;
	output.zAxis.xyz = z;
	output.yAxis.w = output.zAxis.w = 0;

    return output; 
}

bool IsFree(float4 particle)
{
    if(particle.w >=0)
        return true;
    return false;      
}

float2 Responsiveness(float4 particle0, float4 particle1)
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

void DistanceConstraint(inout float4 particle0, inout float4 particle1, float targetDistance, float stiffness = 1.0)
{
    float3 delta = particle1.xyz - particle0.xyz;
    float distance = max(length(delta), 1e-7);
    float stretching = 1 - targetDistance / distance;
    delta = stretching * delta;
    float2 responsiveness = Responsiveness(particle0, particle1);
    particle0.xyz += responsiveness[0] * delta * stiffness;
    particle1.xyz -= responsiveness[1] * delta * stiffness;
}

void SatisfyAngularSpringConstraints(inout ConstraintsAngularHairVertex CAvertex0, inout ConstraintsAngularHairVertex CAvertex1, inout ConstraintsAngularHairVertex CAvertex2, inout ConstraintsAngularHairVertex CAvertex3)
{
    if( CAvertex2.vid != CAvertex1.vid )   
    {
         float stiffness = CAvertex0.Stiffness * g_angularStiffness;
         if( stiffness>0 )
             DistanceConstraint(CAvertex0.Position,CAvertex2.Position, CAvertex0.Length, stiffness );
    }
    if( CAvertex1.vid != CAvertex0.vid )
    {
        if( CAvertex3.vid != CAvertex2.vid )
        {
             float stiffness = CAvertex1.Stiffness * g_angularStiffness;
             if( stiffness>0 )
                 DistanceConstraint(CAvertex1.Position,CAvertex3.Position, CAvertex1.Length, stiffness );
        }        
    }
}

void SatisfyAngularSpringConstraints(inout ConstraintsAngularHairVertex CAvertex0, inout ConstraintsAngularHairVertex CAvertex1)
{
    if( CAvertex0.vid != CAvertex1.vid )   
    {
         float stiffness = CAvertex0.Stiffness * g_angularStiffness;
         if( stiffness>0 )
             DistanceConstraint(CAvertex0.Position,CAvertex1.Position, CAvertex0.Length, stiffness );
    }
}

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

//add forces and do verlet integration
float4 addForcesAndIntegrate(float4 position, float4 oldPosition, float4 force, int vertexID)
{  

    float4 outputPos = position;
    

    if(!IsFree(position))     
    { 
        //if this is a root, transform it by the transform of the scalp
        outputPos.xyz = mul(float4( outputPos.xyz, 1), RootTransformation).xyz;
        return outputPos; 
    }

	//transform all the vertices if needed
	if(g_bApplyAdditionalTransform)
	{
	    position.xyz = mul(float4( position.xyz, 1), additionalTransformation).xyz;
	    oldPosition.xyz = mul(float4( oldPosition.xyz, 1), additionalTransformation).xyz;
	}

    float4 originalPosition = g_OriginalMasterStrand.Load(vertexID);    
    float4 transformedPos = float4((mul(float4(originalPosition.xyz,1),currentHairTransformation)).xyz,originalPosition.w);  
    addObstacleCollisionResponse(transformedPos);         
        
    float internalSpeedCoefficient =        force.w*(0.05-0.02)+0.02;         
    
    float3 velocity = (1-internalSpeedCoefficient)*(position.xyz - oldPosition.xyz);
    
    float stiffness = force.w;
    
    //if velocity is very low increase the stiffness
    if( length(velocity)<0.01 )    
    {
        stiffness = clamp(stiffness*5.0,0,1);
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
        float4 posInGrid = mul(float4(outputPos.xyz, 1), WorldToGrid );
        posInGrid.x += 0.5;
        posInGrid.y += 0.5;
        posInGrid.z += 0.5;
        float3 texcoords = float3(posInGrid.x, 1.0-posInGrid.y, posInGrid.z);
        float3 g_windForce = (g_FluidVelocityTexture.SampleLevel(samLinearClamp,texcoords,0).xyz);
        g_windForce *= (1-stiffness)*(5) + 5;
        force.xyz += g_windForce;
    }
    
    //Gravity
    if( g_bAddGravity)
        force.xyz += float3(0, -gravityStrength, 0);
      
    
    //no move constraint-- keep the hair off the face
    //sphere no move constraint  
    //only do this to hair that are not very stiff already 
    
    {
        for(int i=0;i<g_NumSphereNoMoveImplicits;i++)    
            if( stiffness<0.9 && IsInsideSphere(position.xyz,SphereNoMoveImplicitInverseTransform[i] ) )
            {
    
                float3 vectorToOriginal = transformedPos.xyz - position.xyz;
                float lengthToOriginal = length(vectorToOriginal);
                if( lengthToOriginal>1.0)
                   vectorToOriginal /= lengthToOriginal;     
           
                force.xyz +=  vectorToOriginal*0.525; 
            }
    }  
     
    //verlet integration
    //float clampAmount = 1.0;
    //velocity.xyz = clamp(velocity.xyz, float3(-clampAmount,-clampAmount,-clampAmount), float3(clampAmount,clampAmount,clampAmount) );
    outputPos.xyz = position.xyz
                    + velocity.xyz
                    + force.xyz*g_TimeStep*g_TimeStep; 

    //this makes stiff hair even stiffer, and makes it go towards its base transformed pose. 
	//However, we only apply this force when the hair and the base transformed hair are "relatively" close (and we apply this more to hair we have already designated as stiff). 
	//Here we have huristically decided this value to be 1.6 units
    float staticky = force.w*0.0475;
	if(length(outputPos.xyz - transformedPos.xyz) < 1.6 )
        outputPos.xyz = (1-staticky)*outputPos.xyz + staticky*transformedPos.xyz;
    
    return outputPos;  
    
}

float3 TransformOldVectorToNewVector( float3 vM1X, float3 vM1Y, float3 vM1Z, float3 oldVector) 
{
    if(length(vM1X)<0.01) return oldVector;

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

void AddAngularForces(inout float4 force0, inout float4 force1, int threadReadIndex, float4 positionV1, float4 positionV2,float3 vM1X, float3 vM1Y, float3 vM1Z)
{
    float3 bPOriginal = g_OriginalVectors.Load(threadReadIndex);
    
    if(length(bPOriginal)==0) //this should never happen
    {
        return;    
    }
    
    float3 bP;
	
	//transform the original vector from the local coordinate frame of vertex v-1 to the world coordinate frame
	bP = TransformOldVectorToNewVector(vM1X,vM1Y,vM1Z,bPOriginal);
    
    float k = force0.w*9 + 1; //minimum stiffness is 1 and maximum is 10; empirically adjusted
    
	float3 b = positionV2.xyz - positionV1.xyz;

	float lengthbP = length(bP);
	float lengthB = length(b);
    float dotB_Bp = dot(bP,b);
	float constant = k/(2*lengthbP*lengthB);
	float3 bDoubleNorm = b/(lengthB*lengthB);

	force0.xyz += constant * ( -bP + dotB_Bp*bDoubleNorm );
	force1.xyz += constant * (  bP - dotB_Bp*bDoubleNorm );
}

//since the maximum length is 40 the size of the CTA is fine at 64
//the total amount of thread local storage is limited to 32KB
#define BLOCK_SIZE 64
groupshared float4 sharedPos[BLOCK_SIZE];
groupshared float sharedLength[BLOCK_SIZE];
groupshared coordinateFrame4 sharedCFs[BLOCK_SIZE];
groupshared float sharedAngularStiffness[BLOCK_SIZE];
groupshared float sharedAngularDistance[BLOCK_SIZE];
groupshared float4 sharedForce[BLOCK_SIZE];

//we need to know what the start index is for each strand
//we need to know what the total number of vertices is for each strand
//we need to have thread local storage assigned and use it
//need to know how to sync threads  

[numthreads(64,1,1)]
void UpdateParticlesSimulate(uint threadId        : SV_GroupIndex,
                             uint3 groupId        : SV_GroupID,
                             uint3 globalThreadId : SV_DispatchThreadID)
{
   
    int readStart = 0; //the offset into the particle attributes buffer for this particular patch
    if(groupId.x > 0) readStart =  g_SimulationStrandOffsets.Load( groupId - 1 );
    int n = g_SimulationStrandOffsets.Load( groupId ) - readStart;

    float4 originalPosition;

    int threadReadIndex = readStart + threadId;
	
    //read the attributes into shared memory
    if(threadId < n)
    {
        originalPosition = sharedPos[threadId] = particlePositions[threadReadIndex];
        sharedLength[threadId] = particleDistanceConstraintLengths[threadReadIndex]; 
        float2 landS =  particleAngularLengthAndStiffness[threadReadIndex];
        sharedAngularStiffness[threadId] = landS.y; 
		sharedAngularDistance[threadId] =  landS.x;
        sharedForce[threadId] = float4(0,0,0,0);
		sharedForce[threadId].w = g_stiffness[threadReadIndex];
    }
    
    //synchronize after reading the data into shared memory 
    GroupMemoryBarrierWithGroupSync();
 
    //read and update the coordinate frames
    //note that we need these coordinate frames for calculating angular forces in addition to needing to update these for rendering
    if(threadId < n)
    {
    	sharedCFs[threadId].xAxis = g_coordinateFrames[threadReadIndex*3];
        sharedCFs[threadId].yAxis = g_coordinateFrames[threadReadIndex*3+1];
	    sharedCFs[threadId].zAxis = g_coordinateFrames[threadReadIndex*3+2];
	    
        //update the coordinate frames
        if(threadId<n-1)
            sharedCFs[threadId] = UpdateCoordinateFrame(sharedCFs[threadId], sharedPos[threadId], sharedPos[threadId+1] );  
    }
                
    GroupMemoryBarrierWithGroupSync();    

     //propagate the coordinate frames - this part is only done by thread 0
	if(threadId==0)
	{
	    for(int i=1; i<n; i++)
            sharedCFs[i] = PropagateCoordinateFrame(sharedCFs[i],sharedCFs[i-1]);
	}
    GroupMemoryBarrierWithGroupSync();


    int half = floor(n/2.0f);
    int half2 = floor((n-1)/2.0f); 
	int half3 = floor((n-3)/2.0f); 
	int halfAng1 = max(0,half2-3); //being conservative in how many particles we apply the angular constraint to
	int halfAng2 = max(0,half3-3); //being conservative in how many particles we apply the angular constraint to
	float stiffness = 0.0f;

    // note: we are not using these forces since they are causing some jittering

	//accumulate angular forces
    //angular forces for the first subset
    int id1 = threadId*2;
    int id2 = threadId*2+1;
	if(g_addAngularForces)
    if(threadId<half && threadId>0)
        AddAngularForces(sharedForce[id1], sharedForce[id2], threadReadIndex, sharedPos[id1], sharedPos[id2], sharedCFs[id1-1].xAxis,  sharedCFs[id1-1].yAxis,  sharedCFs[id1-1].zAxis);
    
    GroupMemoryBarrierWithGroupSync();

    //angular forces for the second subset
    id1 = threadId*2+1;
    id2 = threadId*2+2;
	if(g_addAngularForces)
    if(threadId<half2)
        AddAngularForces(sharedForce[id1], sharedForce[id2], threadReadIndex, sharedPos[id1], sharedPos[id2], sharedCFs[id1-1].xAxis,  sharedCFs[id1-1].yAxis,  sharedCFs[id1-1].zAxis);
		
    GroupMemoryBarrierWithGroupSync();

    //add forces and integrate
    if(threadId < n)
    {  
		float4 prevPos;
		if(g_restoreToDefault)sharedPos[threadId] = prevPos = g_OriginalMasterStrand.Load(threadReadIndex);    
		else if(g_integrate) prevPos = previousParticlePositions[threadReadIndex];
		else prevPos = sharedPos[threadId];

        sharedPos[threadId] = addForcesAndIntegrate(sharedPos[threadId], prevPos, sharedForce[threadId], threadReadIndex); 
        previousParticlePositions[threadReadIndex] = originalPosition;        
    }    
    GroupMemoryBarrierWithGroupSync();
      
    //iterate through the constraints for a specified number of times
    for(int iteration=0; iteration<g_numConstraintIterations; iteration++)
    {
        //apply distance constraints to first subset
        if(threadId<half)
            DistanceConstraint(sharedPos[threadId*2],sharedPos[threadId*2+1],sharedLength[threadId*2].x);

        GroupMemoryBarrierWithGroupSync();

        //apply distance constraints to second subset
        if(threadId<half2)
            DistanceConstraint(sharedPos[threadId*2+1],sharedPos[threadId*2+2],sharedLength[threadId*2+1].x);

        GroupMemoryBarrierWithGroupSync();
        
        int idM2 = threadId%2;
		int idD2 = floor(threadId/2.0);
    	int id = idD2*4 + idM2;

        //apply the angular constraints to the first subset
        if(threadId<halfAng1)
		{
            stiffness = sharedAngularStiffness[id] * g_angularStiffness;
             if( stiffness>0 )
                 DistanceConstraint(sharedPos[id],sharedPos[id+2], sharedAngularDistance[id], stiffness );
		}
        GroupMemoryBarrierWithGroupSync();

        //apply the angular constraints to the second subset
        if(threadId<halfAng2)
		{
			id += 2;
            stiffness = sharedAngularStiffness[id] * g_angularStiffness;
             if( stiffness>0 )
                 DistanceConstraint(sharedPos[id],sharedPos[id+2], sharedAngularDistance[id], stiffness );
		}
        GroupMemoryBarrierWithGroupSync();
        
        //apply the collision constraints
        addObstacleCollisionResponse(sharedPos[threadId]);
        GroupMemoryBarrierWithGroupSync();
    }
                 
    //and finally write back the data to the global buffer
    if(threadId < n)
    {
        particlePositions[threadReadIndex] = sharedPos[threadId];
        
        g_coordinateFrames[threadReadIndex*3] = sharedCFs[threadId].xAxis;
        g_coordinateFrames[threadReadIndex*3+1] = sharedCFs[threadId].yAxis;
        g_coordinateFrames[threadReadIndex*3+2] = sharedCFs[threadId].zAxis; 
    }             
}
