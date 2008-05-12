/*
	ATI fragment shader Extension header file.
	setup by NFZ
	extracted from ATI 8500 SDK

** GL_ATI_fragment_shader
**
** Support:
**   Rage 128 * based  : Not Supported
**   Radeon   * based  : Not Supported
**   R200     * based  : Supported
**   R200 : 8500, 9000, 9100, 9200
**   also works on R300 but pointless since ARBFP1.0 supported
*/


#ifndef _GL_ATI_FRAGMENT_SHADER_H_
#define _GL_ATI_FRAGMENT_SHADER_H_

#include "OgreGLSupport.h"

// ATI_fragment_program functions
extern PFNGLGENFRAGMENTSHADERSATIPROC        glGenFragmentShadersATI_ptr;
extern PFNGLBINDFRAGMENTSHADERATIPROC        glBindFragmentShaderATI_ptr;
extern PFNGLDELETEFRAGMENTSHADERATIPROC      glDeleteFragmentShaderATI_ptr;
extern PFNGLBEGINFRAGMENTSHADERATIPROC       glBeginFragmentShaderATI_ptr;
extern PFNGLENDFRAGMENTSHADERATIPROC         glEndFragmentShaderATI_ptr;
extern PFNGLPASSTEXCOORDATIPROC              glPassTexCoordATI_ptr;
extern PFNGLSAMPLEMAPATIPROC                 glSampleMapATI_ptr;
extern PFNGLCOLORFRAGMENTOP1ATIPROC          glColorFragmentOp1ATI_ptr;
extern PFNGLCOLORFRAGMENTOP2ATIPROC          glColorFragmentOp2ATI_ptr;
extern PFNGLCOLORFRAGMENTOP3ATIPROC          glColorFragmentOp3ATI_ptr;
extern PFNGLALPHAFRAGMENTOP1ATIPROC          glAlphaFragmentOp1ATI_ptr;
extern PFNGLALPHAFRAGMENTOP2ATIPROC          glAlphaFragmentOp2ATI_ptr;
extern PFNGLALPHAFRAGMENTOP3ATIPROC          glAlphaFragmentOp3ATI_ptr;
extern PFNGLSETFRAGMENTSHADERCONSTANTATIPROC glSetFragmentShaderConstantATI_ptr;

bool InitATIFragmentShaderExtensions(Ogre::GLSupport& glSupport);

#endif	//_GL_ATI_FRAGMENT_SHADER_H_

