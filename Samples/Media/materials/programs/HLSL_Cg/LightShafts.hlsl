// ---------------------------------------------------------------------------------------
// Light shafts shaders with shadows and noise support
// Inpirated on the ATI paper https://developer.amd.com/wordpress/media/2012/10/Mitchell_LightShafts.pdf
// Ogre3D implementation by Xavier Verguín González (xavyiy [at] gmail [dot] com) [Xavyiy]
// ---------------------------------------------------------------------------------------

// --------------------- Light shafts material ------------------------

void main_vp(
    // IN
    float4 iPosition	        : POSITION,
    // OUT
    out float4 oPosition		: POSITION,
    out float3 vPosition        : TEXCOORD0,
    out float4 oUV    		    : TEXCOORD1,
    // UNIFORM
    uniform float4x4 uWorldView,
    uniform float4x4 uWorldViewProj,
    uniform float4x4 uTexWorldViewProj)
{
    oPosition   = mul(uWorldViewProj, iPosition);
    vPosition   = mul(uWorldView, iPosition).xyz;
    oUV         = mul(uTexWorldViewProj, iPosition);
}

ps_2_0 float convertDepth(float d) { return d; }
arbfp1 float convertDepth(float d) { return d * 0.5 + 0.5; }

void main_fp(
    // IN
    float3 vPosition        : TEXCOORD0,
    float4 iUV    		    : TEXCOORD1,
    // OUT
    out float4 oColor		: COLOR,
    // UNIFORM
    uniform float4     uAttenuation,
    uniform float3    uLightPosition,
    uniform sampler2D uDepthMap  : register(s0),
    uniform sampler2D uCookieMap : register(s1),
    uniform sampler2D uNoiseMap	 : register(s2),
    uniform float Time)
{
    iUV = iUV / iUV.w;

    float Depth  = tex2D(uDepthMap,  iUV.xy).r;

    if (Depth < convertDepth(iUV.z))
    {
        oColor = float4(0,0,0,1);
    }
    else
    {
        float4 Cookie = tex2D(uCookieMap, iUV.xy);
        float2 Noise  = float2(tex2D(uNoiseMap,  iUV.xy - Time).r,
                               tex2D(uNoiseMap,  iUV.xy + Time).g);

        float noise  = Noise.x * Noise.y;
        float length_ = length(uLightPosition - vPosition);
        float atten  = 1.0 / (uAttenuation.y + uAttenuation.z*length_ + 20*uAttenuation.w*length_*length_);

        oColor = float4(Cookie.rgb * atten * noise , 1);
    }
}