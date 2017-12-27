#ifndef GL_ES
#version 120
#else
#version 100
#endif

// The MIT License
// Copyright (c) 2016-2017 Mohamad Moneimne and Contributors

attribute vec4 position;
#ifdef HAS_NORMALS
attribute vec4 normal;
#endif
#ifdef HAS_TANGENTS
attribute vec4 tangent;
#endif
#ifdef HAS_UV
attribute vec2 uv0;
#endif

uniform mat4 u_MVPMatrix;
uniform mat4 u_ModelMatrix;

varying vec3 v_Position;
varying vec2 v_UV;

#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
varying mat3 v_TBN;
#else
varying vec3 v_Normal;
#endif
#endif


void main()
{
  vec4 pos = u_ModelMatrix * position;
  v_Position = vec3(pos.xyz) / pos.w;

  #ifdef HAS_NORMALS
  #ifdef HAS_TANGENTS
  vec3 normalW = normalize(vec3(u_ModelMatrix * vec4(normal.xyz, 0.0)));
  vec3 tangentW = normalize(vec3(u_ModelMatrix * vec4(tangent.xyz, 0.0)));
  vec3 bitangentW = cross(normalW, tangentW) * tangent.w;
  v_TBN = mat3(tangentW, bitangentW, normalW);
  #else // HAS_TANGENTS != 1
  v_Normal = normalize(vec3(u_ModelMatrix * vec4(normal.xyz, 0.0)));
  #endif
  #endif

  #ifdef HAS_UV
  v_UV = uv0;
  #else
  v_UV = vec2(0.,0.);
  #endif

  gl_Position = u_MVPMatrix * position; // needs w for proper perspective correction
}


