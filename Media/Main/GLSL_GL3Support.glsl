// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

// @public-api

#if __VERSION__ == 100
mat2 transpose(mat2 m)
{
  return mat2(m[0][0], m[1][0],
              m[0][1], m[1][1]);
}

mat3 transpose(mat3 m)
{
  return mat3(m[0][0], m[1][0], m[2][0],
              m[0][1], m[1][1], m[2][1],
              m[0][2], m[1][2], m[2][2]);
}

mat4 transpose(mat4 m)
{
  return mat4(m[0][0], m[1][0], m[2][0], m[3][0],
              m[0][1], m[1][1], m[2][1], m[3][1],
              m[0][2], m[1][2], m[2][2], m[3][2],
              m[0][3], m[1][3], m[2][3], m[3][3]);
}
#endif

#if __VERSION__ > 120 || defined(OGRE_GLSLANG)
#define texture1D texture
#define texture2D texture
#define texture3D texture
#define texture2DArray texture
#define textureCube texture
#define shadow2D texture
#define shadow2DProj textureProj
#define texture2DProj textureProj
#define texture2DLod textureLod
#define textureCubeLod textureLod

#if defined(OGRE_GLSLANG) || (__VERSION__ > 150 && defined(OGRE_VERTEX_SHADER)) || __VERSION__ >= 410
#define IN(decl, loc) layout(location = loc) in decl;
#else
#define IN(decl, loc) in decl;
#endif

#if defined(OGRE_GLSLANG) || (__VERSION__ > 150 && defined(OGRE_FRAGMENT_SHADER)) || __VERSION__ >= 410
#define OUT(decl, loc) layout(location = loc) out decl;
#else
#define OUT(decl, loc) out decl;
#endif

#else

#ifdef OGRE_VERTEX_SHADER
#define IN(decl, loc) attribute decl;
#define OUT(decl, loc) varying decl;
#else
#define IN(decl, loc) varying decl;
#define OUT(decl, loc) out decl;
#endif

#endif

#if defined(OGRE_FRAGMENT_SHADER) && (defined(OGRE_GLSLANG) || (__VERSION__ > 130))
#define gl_FragColor FragColor
OUT(vec4 FragColor, 0)
#endif

#ifdef VULKAN

#ifdef OGRE_VERTEX_SHADER
#define OGRE_UNIFORMS_BEGIN layout(binding = 0, row_major) uniform OgreUniforms {
#else
#define OGRE_UNIFORMS_BEGIN layout(binding = 1, row_major) uniform OgreUniforms {
#endif

#define OGRE_UNIFORMS_END };

#else

#define OGRE_UNIFORMS_BEGIN
#define OGRE_UNIFORMS_END

#endif