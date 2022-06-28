#include "OgreUnifiedShader.h"

struct RasterizerData
{
  vec4 pos [[position]];
  vec2 uv;
};

struct Vertex
{
  IN(vec3 pos, POSITION);
  IN(vec2 uv, TEXCOORD0);
};

struct Uniform
{
  mat4 mvpMtx;
  mat4 texMtx;
};

// first 15 slots are reserved for the vertex attributes
#define UNIFORM_INDEX_START 16

vertex RasterizerData default_vp(Vertex in [[stage_in]],
                                 constant Uniform& u [[buffer(UNIFORM_INDEX_START)]])
{
  RasterizerData out;
  out.pos = u.mvpMtx * vec4(in.pos, 1);
  out.uv = (u.texMtx * vec4(in.uv,1,1)).xy;
  return out;
}

fragment half4 default_fp(RasterizerData in [[stage_in]],
                          metal::texture2d<half> tex [[texture(0)]],
                          metal::sampler s [[sampler(0)]])
{
  return tex.sample(s, in.uv);
}