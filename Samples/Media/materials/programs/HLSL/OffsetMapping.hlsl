/* Offset mapping including a shadow element and multiple lights in one pass */ 
void integratedshadows_vp(float4 position   : POSITION, 
              float3 normal      : NORMAL, 
              float2 uv         : TEXCOORD0, 
              float3 tangent     : TANGENT0, 
              // outputs 
              out float4 oPosition    : POSITION, 
              out float2 oUv          : TEXCOORD0, 
              out float3 oLightDir    : TEXCOORD1, // tangent space 
             out float3 oEyeDir       : TEXCOORD2, // tangent space 
			 out float3 oSpotDirection : TEXCOORD3, // tangent space
              out float3 oLightDir1    : TEXCOORD4, // tangent space 
              out float3 oSpotDirection1    : TEXCOORD5, // tangent space 
			 out float4 oShadowUV1 : TEXCOORD6,
			 out float4 oShadowUV2 : TEXCOORD7,
              // parameters 
              uniform float4 lightPosition, // object space 
              uniform float4 lightPosition1, // object space 
              uniform float3 eyePosition,   // object space 
 			  uniform float3 spotDirection, // object space
 			  uniform float3 spotDirection1, // object space
              uniform float4x4 worldViewProj,
			  uniform float4x4 worldMatrix,
			  uniform float4x4 texViewProj1,
			  uniform float4x4 texViewProj2) 
{  
   // calculate output position 
   oPosition = mul(worldViewProj, position); 

   float4 worldPos = mul(worldMatrix, position);
   oShadowUV1 = mul(texViewProj1, worldPos);
   oShadowUV2 = mul(texViewProj2, worldPos);
   
   

   // pass the main uvs straight through unchanged 
   oUv = uv; 

   // calculate tangent space light vector 
   // Get object space light direction 
   float3 lightDir = normalize(lightPosition.xyz -  (position * lightPosition.w));
   float3 lightDir1 = normalize(lightPosition1.xyz -  (position * lightPosition1.w));
   float3 eyeDir = eyePosition - position.xyz; 
    
   // Calculate the binormal (NB we assume both normal and tangent are 
   // already normalised) 
   // NB looks like nvidia cross params are BACKWARDS to what you'd expect 
   // this equates to NxT, not TxN 
   float3 binormal = cross(tangent, normal); 
    
   // Form a rotation matrix out of the vectors 
   float3x3 rotation = float3x3(tangent, binormal, normal); 
    
   // Transform the light vector according to this matrix 
   lightDir = normalize(mul(rotation, lightDir)); 
   lightDir1 = normalize(mul(rotation, lightDir1)); 
   eyeDir = normalize(mul(rotation, eyeDir)); 

   oLightDir = lightDir; 
   oLightDir1 = lightDir1; 
   oEyeDir = eyeDir; 
   oSpotDirection = normalize(mul(rotation, -spotDirection));
   oSpotDirection1 = normalize(mul(rotation, -spotDirection1));
}

// Expand a range-compressed vector
float3 expand(float3 v)
{
	return (v - 0.5) * 2;
}

void integratedshadows_fp(
	float2 uv : TEXCOORD0,
	float3 lightDir : TEXCOORD1,
	float3 eyeDir : TEXCOORD2,
	float3 spotDir : TEXCOORD3,
	float3 lightDir1 : TEXCOORD4,
	float3 spotDir1 : TEXCOORD5,
	float4 shadowUV1 : TEXCOORD6,
	float4 shadowUV2 : TEXCOORD7,
	uniform float3 lightDiffuse,
	uniform float4 scaleBias,
    uniform float4 spotParams,
	uniform float3 lightDiffuse1,
    uniform float4 spotParams1,
	uniform sampler2D normalHeightMap : register(s0),
	uniform sampler2D diffuseMap : register(s1),
	uniform sampler2D shadowMap1 : register(s2),
	uniform sampler2D shadowMap2 : register(s3),
	out float4 oColour : COLOR)
{
	// get the height using the tex coords
	float height = tex2D(normalHeightMap, uv).a;

	// scale and bias factors	
	float scale = scaleBias.x;
	float bias = scaleBias.y;

	// calculate displacement	
	float displacement = (height * scale) + bias;
	
	float3 uv2 = float3(uv, 1);

	float3 scaledEyeDir = eyeDir * displacement;
	
	// calculate the new tex coord to use for normal and diffuse
	float2 newTexCoord = (scaledEyeDir + uv2).xy;
	
	// get the new normal and diffuse values
	float3 normal = expand(tex2D(normalHeightMap, newTexCoord).xyz);
	float3 diffuse = tex2D(diffuseMap, newTexCoord).xyz;
	
	float3 col1 = diffuse * saturate(dot(normal, lightDir)) * lightDiffuse;
	// factor in spotlight angle
	float rho = saturate(dot(spotDir, lightDir));
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	float spotFactor = pow(
		saturate(rho - spotParams.y) / (spotParams.x - spotParams.y), spotParams.z);
	col1 = col1 * spotFactor;
	float3 col2 = diffuse * saturate(dot(normal, lightDir1)) * lightDiffuse1;
	// factor in spotlight angle
	rho = saturate(dot(spotDir1, lightDir1));
	// factor = (rho - cos(outer/2) / cos(inner/2) - cos(outer/2)) ^ falloff
	spotFactor = pow(
		saturate(rho - spotParams1.y) / (spotParams1.x - spotParams1.y), spotParams1.z);
	col2 = col2 * spotFactor;

	// shadow textures
	col1 = col1 * tex2Dproj(shadowMap1, shadowUV1);
	col2 = col2 * tex2Dproj(shadowMap2, shadowUV2);

	oColour = float4(col1 + col2, 1);

}

