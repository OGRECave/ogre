cbuffer MatrixBuffer
{
	matrix worldViewProj;
};

struct v2p
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

//////////////////////////////////////////////
//				 CASTER PASS				//
//				    HEAT					//
//////////////////////////////////////////////

v2p HeatCaster_vp(
              // in
              float4 vPos: POSITION,
              float4 vNormal: NORMAL,
              uniform float3 eyePosition        // object space
            )
{
   v2p output;
   float4 eyeDir = float4(eyePosition - vPos.xyz, 0);
   eyeDir = normalize(eyeDir);
   output.position = mul( worldViewProj, vPos );
   output.texCoord = clamp( dot( vNormal, eyeDir ), 0, 1 );
   return output;
}

float4 HeatCaster_fp(
                        v2p input
                     ) : SV_Target
{
   return float4(input.texCoord.x,input.texCoord.x,input.texCoord.x,input.texCoord.x);
}

//////////////////////////////////////////////
//				 CASTER PASS				//
//				    COLD					//
//////////////////////////////////////////////

v2p ColdCaster_vp(
              // in
              float4 vPos: POSITION,
              float4 vNormal: NORMAL,
              uniform float3 eyePosition        // object space
            )
{
   v2p output;
   float4 eyeDir = float4(eyePosition - vPos.xyz, 0);
   eyeDir = normalize(eyeDir);
   output.position = mul( worldViewProj, vPos );
   output.texCoord = clamp( dot( vNormal, eyeDir ), 0, 1 );
   return output;
}

float4 ColdCaster_fp(
                        // input from vp
                        v2p input
                     ) : SV_Target
{
   return float4(input.texCoord.x / 2,input.texCoord.x / 2,input.texCoord.x / 2,input.texCoord.x / 2);
}


//////////////////////////////////////////////
//	   PASS 1 - Light to heat conversion	//
//////////////////////////////////////////////

v2p LightToHeat_vp(
                    // in
                    float4 inPos: POSITION,
                    uniform float flipping
                   )
{
    v2p output;
    output.position = float4(inPos.x, flipping * inPos.y, 0.0f, 1.0f);
    inPos.xy = sign(inPos.xy);
    output.texCoord = (float2(inPos.x, -inPos.y) + 1.0f)/2.0f;
	return output;
}

SamplerState g_samLinear
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState g_samVolume
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
	AddressW = Wrap;
};

float4 LightToHeat_fp(
                        // input from vp
                        v2p inp,
                                         
                        // params
                        uniform float4 random_fractions,
                        uniform float4 heatBiasScale,
                        uniform float4 depth_modulator,
						uniform Texture2D Input,       // output of HeatVisionCaster_fp (NdotV)
						uniform Texture2D NoiseMap,
						uniform Texture2D HeatLookup						
                     ) : SV_Target
{
   float  depth, heat, interference;

   //  Output constant color:
   depth = Input.Sample(g_samLinear, inp.texCoord );
   depth *= (depth * depth_modulator.x);

   heat  = (depth * heatBiasScale.y);

//   if (depth > 0)
   {
		interference = -0.5 + NoiseMap.Sample(g_samVolume, inp.texCoord.xy + float2( random_fractions.x, random_fractions.y ) );
		interference *= interference;
		interference *= 1 - heat;
		heat += interference;//+ heatBiasScale.x;
   }

/*
	heatBias isn't used for now
   if (heat > 0)
       heat += heatBiasScale.x;
*/

   // Clamp UVs
   heat  = max( 0.005, min( 0.995, heat ) );
   float4 outColor = HeatLookup.Sample(g_samLinear, float2( heat, 0.f ) );
   return outColor;
}


//////////////////////////////////////////////
// PASS 2 - add simple blur (final pass)	//
//////////////////////////////////////////////

v2p Blur_vp(
                    // in
                    float4 inPos: POSITION,
                    uniform float flipping
                   )
{
    v2p output;
    output.position = float4(inPos.x, flipping * inPos.y, 0.0f, 1.0f);
    inPos.xy = sign(inPos.xy);
    output.texCoord  = (float2(inPos.x, -inPos.y) + 1.0f)/2.0f;
	return output;
}

// ps_2_0
float4 Blur_fp(
                    // input from vp
                    v2p inp,
                    // parameters
					uniform Texture2D Input,
                    uniform float4 blurAmount
                    ) : SV_Target
{
   int i;
   float4 tmpOutColor;
   float  diffuseGlowFactor;
   const float2 offsets[4] = 
   {
/*
		// hazy blur
		-1.8, -1.8,
		-1.8, 1.8,
		1.8, -1.8,
		1.8, 1.8
*/
/*
		// less-hazy blur
	  -1.0,  2.0,
	  -1.0, -1.0,
	   1.0, -1.0,
	   1.0,  1.0
*/
/*
      -0.326212, -0.405805,
      -0.840144, -0.073580,
      -0.695914,  0.457137,
      -0.203345,  0.620716
*/

	  -0.3,  0.4,
	  -0.3, -0.4,
	   0.3, -0.4,
	   0.3,  0.4

   };

   tmpOutColor = Input.Sample(g_samLinear, inp.texCoord );	// UV coords are in image space

   // calculate glow amount
   diffuseGlowFactor = 0.0113f * (2.0 - max( tmpOutColor.r, tmpOutColor.g ));

   // basic blur filter
   for (i = 0; i < 4; i++) {
      tmpOutColor += Input.Sample(g_samLinear, inp.texCoord.xy + blurAmount.x * diffuseGlowFactor * offsets[i] );
   }

   tmpOutColor *= 0.25;

   // TIPS (old-skool strikes again!)
   // Pay attention here! If you use the "out float4 outColor" directly
   // in your steps while creating the output color (like you remove
   // the "tmpOutColor" var and just use the "outColor" directly)
   // your pixel-color output IS CHANGING EACH TIME YOU DO AN ASSIGNMENT TOO!
   // A temporary variable, instead, acts like a per-pixel double buffer, and
   // best of all, lead to better performance.
   float4 outColor = tmpOutColor;
   return outColor;
}