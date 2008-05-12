// oceanHLSL_Cg.frag
// fragment program for Ocean water simulation
// 04 Aug 2005
// adapted for Ogre by nfz
// original shader source from Render Monkey 1.6 Reflections Refractions.rfx
// can be used in both Cg and HLSL compilers

// 06 Aug 2005: moved uvw calculation from fragment program into vertex program 

float4 main(float3 uvw: TEXCOORD0, float3 normal: TEXCOORD1, float3 vVec: TEXCOORD2,
	uniform float fadeBias,
	uniform float fadeExp,
	uniform float4 waterColor,
	uniform sampler Noise,
	uniform sampler skyBox

) : COLOR
{

   float3 noisy = tex3D(Noise, uvw).xyz;

   // convert to Signed noise 
   float3 bump = 2 * noisy - 1;
   bump.xz *= 0.15;
   // Make sure the normal always points upwards
   // note that Ogres y axis is vertical (RM Z axis is vertical)
   bump.y = 0.8 * abs(bump.y) + 0.2;
   // Offset the surface normal with the bump
   bump = normalize(normal + bump);

   // Find the reflection vector
   float3 normView = normalize(vVec);
   float3 reflVec = reflect(normView, bump);
   // Ogre has z flipped for cubemaps
   reflVec.z = -reflVec.z;
   float4 refl = texCUBE(skyBox, reflVec);

   // set up for fresnel calc
   float lrp = 1 - dot(-normView, bump);
   
   // Interpolate between the water color and reflection for fresnel effect
   return lerp(waterColor, refl, saturate(fadeBias + pow(lrp, fadeExp)));

}
