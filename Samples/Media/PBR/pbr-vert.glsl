#include <OgreUnifiedShader.h>

// The MIT License
// Copyright (c) 2016-2017 Mohamad Moneimne and Contributors

OGRE_UNIFORMS(
uniform mat4 u_MVPMatrix;
uniform mat4 u_ModelMatrix;
)

MAIN_PARAMETERS

IN(vec4 position, POSITION)
#ifdef HAS_NORMALS
IN(vec4 normal, NORMAL)
#endif
#ifdef HAS_TANGENTS
IN(vec4 tangent, TANGENT)
#endif
#ifdef HAS_UV
IN(vec2 uv0, TEXCOORD0)
#endif

OUT(vec3 v_Position, TEXCOORD0)
OUT(vec2 v_UV, TEXCOORD1)

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
OUT(mat3 v_TBN, TEXCOORD2)
#else
OUT(vec3 v_Normal, TEXCOORD2)
#endif
#endif


MAIN_DECLARATION
{
  vec4 pos = mul(u_ModelMatrix, position);
  v_Position = vec3(pos.xyz) / pos.w;

  #ifdef HAS_NORMALS
  #ifdef HAS_TANGENTS
  vec3 normalW = normalize(mul(u_ModelMatrix, vec4(normal.xyz, 0.0)).xyz);
  vec3 tangentW = normalize(mul(u_ModelMatrix, vec4(tangent.xyz, 0.0)).xyz);
  vec3 bitangentW = cross(normalW, tangentW) * tangent.w;
  v_TBN = mtxFromCols(tangentW, bitangentW, normalW);
  #else // HAS_TANGENTS != 1
  v_Normal = normalize(vec3(mul(u_ModelMatrix, vec4(normal.xyz, 0.0))));
  #endif
  #endif

  #ifdef HAS_UV
  v_UV = uv0;
  #else
  v_UV = vec2(0.,0.);
  #endif

  gl_Position = mul(u_MVPMatrix, position); // needs w for proper perspective correction
}


