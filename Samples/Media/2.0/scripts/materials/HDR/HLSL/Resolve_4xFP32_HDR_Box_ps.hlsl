float max3( float x, float y, float z ) { return max(x, max(y, z)); }

#if !MSAA_INITIALIZED
    #define MSAA_SUBSAMPLE_WEIGHT	0.25
    #define MSAA_NUM_SUBSAMPLES		4
#endif

// Apply this to tonemap linear HDR color "c" after a sample is fetched in the resolve.
// Note "c" 1.0 maps to the expected limit of low-dynamic-range monitor output.
//float3 tonemap( float3 c ) { return c * rcp(max3(c.r, c.g, c.b) + 1.0); }

// When the filter kernel is a weighted sum of fetched colors,
// it is more optimal to fold the weighting into the tonemap operation.
float3 tonemapWithWeight( float invLum, float3 c, float w )
{
    return c * (invLum * w * rcp( max3(c.r, c.g, c.b) * invLum + 1.0 ));
}

// Apply this to restore the linear HDR color before writing out the result of the resolve.
float3 tonemapInvert( float invLum, float3 c )
{
    float3 val = c * rcp( 1.0 - max3(c.r, c.g, c.b) );
    return val * rcp( invLum );
}

float4 loadWithToneMapAndWeight( float invLum, Texture2DMS<float4> tex, int2 iCoord, int subsample )
{
    float4 value = tex.Load( iCoord, subsample ).xyzw;
    value.xyz = tonemapWithWeight( invLum, value.xyz, MSAA_SUBSAMPLE_WEIGHT );
    value.w *= MSAA_SUBSAMPLE_WEIGHT;

    return value;
}

Texture2DMS<float4>	rt0         : register(t0);
Texture2D<float>	oldLumRt	: register(t1);

struct PS_INPUT
{
    float2 uv0			: TEXCOORD0;
};

float4 main
(
    PS_INPUT inPs,
    float4 gl_FragCoord : SV_Position
) : SV_Target
{
    int2 iFragCoord = int2( gl_FragCoord.xy );

    float oldInvLum = oldLumRt.Load( int3( 0, 0, 0 ) );

    float4 accumValue;

    accumValue  = loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 0 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 1 );

#if MSAA_NUM_SUBSAMPLES > 2
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 2 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 3 );
#endif

#if MSAA_NUM_SUBSAMPLES > 4
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 4 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 5 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 6 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 7 );
#endif

#if MSAA_NUM_SUBSAMPLES > 8
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 8 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 9 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 10 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 11 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 12 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 13 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 14 );
    accumValue += loadWithToneMapAndWeight( oldInvLum, rt0, iFragCoord, 15 );
#endif

    return float4( tonemapInvert( oldInvLum, accumValue.xyz ), accumValue.w );
}
