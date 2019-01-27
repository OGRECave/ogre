// oceanHLSL_Cg.vert
// vertex program for Ocean water simulation
// 04 Aug 2005
// adapted for Ogre by nfz
// original shader source from Render Monkey 1.6 Reflections Refractions.rfx
// can be used in both Cg and HLSL compilers

// 06 Aug 2005: moved uvw calculation from fragment program into vertex program 

struct VS_OUTPUT {
   float4 Pos:    POSITION;
   float3 uvw:    TEXCOORD0;
   float3 normal: TEXCOORD1;
   float3 vVec:   TEXCOORD2;
};

VS_OUTPUT main(float4 Pos: POSITION, float3 normal: NORMAL,
	uniform float4x4 worldViewProj_matrix,
	uniform float3 scale,
	uniform float2 waveSpeed,
	uniform float noiseSpeed,
	uniform float time_0_X,
	uniform float3 eyePosition

)
{
   VS_OUTPUT Out;

   Out.Pos = mul(worldViewProj_matrix, Pos);
   
   // uvw is the calculated uvw coordinates based on vertex position
   Out.uvw = Pos.xyz * scale;
   Out.uvw.xz += waveSpeed * time_0_X;
   Out.uvw.y += Out.uvw.z + noiseSpeed * time_0_X;
   
   //  the view vector needs to be in vertex space
   Out.vVec = Pos.xyz - eyePosition;
   Out.normal = normal;

   return Out;
}
