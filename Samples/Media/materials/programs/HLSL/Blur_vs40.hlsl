struct VS_OUTPUT {
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main(float4 Pos: POSITION){
   VS_OUTPUT Out;

   // Clean up inaccuracies
   Pos.xy = sign(Pos.xy);

   Out.Pos = float4(Pos.xy, 0, 1);
   // Image-space
   Out.texCoord.x = 0.5 * (1 + Pos.x);
   Out.texCoord.y = 0.5 * (1 - Pos.y);

   return Out;
}

