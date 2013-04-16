// Simple blur filter

const float2 samples[8] = {
    {-1, 1},
    {-1, 0},
    {-1, -1},
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1}
};

float4 blur(

    in float2 texCoord: TEXCOORD0,
    uniform float sampleDistance: register(c0),
    uniform sampler Blur0: register(s0)

) : COLOR
{
   float4 sum = tex2D(Blur0, texCoord);
   for (int i = 0; i < 8; ++i){
      sum += tex2D(Blur0, texCoord + sampleDistance * samples[i]);
   }
   return sum / 9;
}



float4 blend
(
    in float2 texCoord: TEXCOORD0,

    uniform sampler Blur0 : register(s0),
    uniform sampler Blur1 : register(s1),

    uniform float focus: register(c0),
    uniform float range: register(c1)
) : COLOR
{
   float4 sharp = tex2D(Blur0, texCoord);
   float4 blur  = tex2D(Blur1, texCoord);

   // alpha channel of sharp RT has depth info
   return mix(sharp, blur, saturate(range * abs(focus - sharp.a)));
}
// Simple blur filter

const float2 samples[8] = {
    {-1, 1},
    {-1, 0},
    {-1, -1},
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1}
};

float4 blur(

    in float2 texCoord: TEXCOORD0,
    uniform float sampleDistance: register(c0),
    uniform sampler Blur0: register(s0)

) : COLOR
{
   float4 sum = tex2D(Blur0, texCoord);
   for (int i = 0; i < 8; ++i){
      sum += tex2D(Blur0, texCoord + sampleDistance * samples[i]);
   }
   return sum / 9;
}



float4 blend
(
    in float2 texCoord: TEXCOORD0,

    uniform sampler Blur0 : register(s0),
    uniform sampler Blur1 : register(s1),

    uniform float focus: register(c0),
    uniform float range: register(c1)
) : COLOR
{
   float4 sharp = tex2D(Blur0, texCoord);
   float4 blur  = tex2D(Blur1, texCoord);

   // alpha channel of sharp RT has depth info
   return mix(sharp, blur, saturate(range * abs(focus - sharp.a)));
}
