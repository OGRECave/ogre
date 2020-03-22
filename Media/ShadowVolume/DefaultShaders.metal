struct RasterizerData
{
  float4 pos [[position]];
  float2 uv;
};

struct Vertex
{
  float3 pos [[ attribute(0) ]];
  float2 uv [[ attribute(8) ]];
};

struct Uniform
{
  metal::float4x4 mvpMtx;
  metal::float4x4 texMtx;
};

vertex RasterizerData default_vp(Vertex in [[stage_in]],
                                 constant Uniform& u [[buffer(CONST_SLOT_START)]])
{
  RasterizerData out;
  out.pos = u.mvpMtx * float4(in.pos, 1);
  out.uv = (u.texMtx * float4(in.uv,1,1)).xy;
  return out;
}

fragment half4 default_fp(RasterizerData in [[stage_in]],
                          metal::texture2d<half> tex [[texture(0)]],
                          metal::sampler s [[sampler(0)]])
{
  return tex.sample(s, in.uv);
}