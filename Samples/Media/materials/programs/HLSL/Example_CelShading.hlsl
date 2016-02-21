
/* Cel shading vertex program for single-pass rendering
   In this program, we want to calculate the diffuse and specular
   ramp components, and the edge factor (for doing simple outlining)
   For the outlining to look good, we need a pretty well curved model.
*/

struct vertex_in
{
	float4 position	: POSITION;
	float3 normal		: NORMAL;
};

struct fragment_in
{
    float4 oPosition : SV_POSITION;
	float  diffuse		 : TEXCOORD0;
	float  specular	 : TEXCOORD1;
	float  edge		 : TEXCOORD2;
};

fragment_in main_vp(
			vertex_in input,
			 // parameters
			 uniform float3 lightPosition, // object space
			 uniform float3 eyePosition,   // object space
			 uniform float4  shininess,
			 uniform float4x4 worldViewProj)
{
	fragment_in output;
	// calculate output position
	output.oPosition = mul(worldViewProj, input.position);

	// calculate light vector
	float3 N = normalize(input.normal);
	float3 L = normalize(lightPosition - input.position.xyz);
	
	// Calculate diffuse component
	output.diffuse = max(dot(N, L) , 0);

	// Calculate specular component
	float3 E = normalize(eyePosition - input.position.xyz);
	float3 H = normalize(L + E);
	output.specular = pow(max(dot(N, H), 0), shininess).x;
	// Mask off specular if diffuse is 0
	if (output.diffuse == 0) output.specular = 0;

	// Edge detection, dot eye and normal vectors
	output.edge = max(dot(N, E), 0);
	
	return output;
}

SamplerState SampleType;

float4 main_fp( fragment_in input,
			 
			 uniform float4 diffuse,
			 uniform float4 specular,
			 
			 uniform Texture1D diffuseRamp,
			 uniform Texture1D specularRamp,
			 uniform Texture1D edgeRamp) : SV_Target
{
	// Step functions from textures
	input.diffuse = diffuseRamp.Sample(SampleType, input.diffuse).x;
	input.specular = specularRamp.Sample(SampleType, input.specular).x;
	input.edge = edgeRamp.Sample(SampleType, input.edge).x;

	return input.edge * ((diffuse * input.diffuse) + 
					(specular * input.specular));
}