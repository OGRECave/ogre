/*
	ATI fragment shader Extension program file.
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

#include "OgreGLATIFSInit.h"
#include "OgreGLPrerequisites.h"

// ATI_fragment_program functions
PFNGLGENFRAGMENTSHADERSATIPROC        glGenFragmentShadersATI_ptr=NULL;
PFNGLBINDFRAGMENTSHADERATIPROC        glBindFragmentShaderATI_ptr=NULL;
PFNGLDELETEFRAGMENTSHADERATIPROC      glDeleteFragmentShaderATI_ptr=NULL;
PFNGLBEGINFRAGMENTSHADERATIPROC       glBeginFragmentShaderATI_ptr=NULL;
PFNGLENDFRAGMENTSHADERATIPROC         glEndFragmentShaderATI_ptr=NULL;
PFNGLPASSTEXCOORDATIPROC              glPassTexCoordATI_ptr=NULL;
PFNGLSAMPLEMAPATIPROC                 glSampleMapATI_ptr=NULL;
PFNGLCOLORFRAGMENTOP1ATIPROC          glColorFragmentOp1ATI_ptr=NULL;
PFNGLCOLORFRAGMENTOP2ATIPROC          glColorFragmentOp2ATI_ptr=NULL;
PFNGLCOLORFRAGMENTOP3ATIPROC          glColorFragmentOp3ATI_ptr=NULL;
PFNGLALPHAFRAGMENTOP1ATIPROC          glAlphaFragmentOp1ATI_ptr=NULL;
PFNGLALPHAFRAGMENTOP2ATIPROC          glAlphaFragmentOp2ATI_ptr=NULL;
PFNGLALPHAFRAGMENTOP3ATIPROC          glAlphaFragmentOp3ATI_ptr=NULL;
PFNGLSETFRAGMENTSHADERCONSTANTATIPROC glSetFragmentShaderConstantATI_ptr=NULL;

bool InitATIFragmentShaderExtensions(Ogre::GLSupport& glSupport)
{
    static bool init = false;
    //char *extList;

    if(init) return init;

	
	/* confirm that the version of OpenGL supports ATI fragment shader */
	/* done in GLRenderSystem
    extList = (char *)glGetString(GL_EXTENSIONS);

    if (strstr(extList, "GL_ATI_fragment_shader") == NULL)  {
		//MessageBox(NULL, "GL_ATI_fragment_shader extension not supported", "GL Extension error", MB_OK);
		// ** should raise exception
		init = false;
		return false;
    }

	*/


	glGenFragmentShadersATI_ptr    = (PFNGLGENFRAGMENTSHADERSATIPROC) glSupport.getProcAddress("glGenFragmentShadersATI");
	glBindFragmentShaderATI_ptr    = (PFNGLBINDFRAGMENTSHADERATIPROC) glSupport.getProcAddress("glBindFragmentShaderATI");
	glDeleteFragmentShaderATI_ptr  = (PFNGLDELETEFRAGMENTSHADERATIPROC) glSupport.getProcAddress("glDeleteFragmentShaderATI");
	glBeginFragmentShaderATI_ptr   = (PFNGLBEGINFRAGMENTSHADERATIPROC) glSupport.getProcAddress("glBeginFragmentShaderATI");
	glEndFragmentShaderATI_ptr     = (PFNGLENDFRAGMENTSHADERATIPROC) glSupport.getProcAddress("glEndFragmentShaderATI");
	glPassTexCoordATI_ptr          = (PFNGLPASSTEXCOORDATIPROC) glSupport.getProcAddress("glPassTexCoordATI");
	glSampleMapATI_ptr             = (PFNGLSAMPLEMAPATIPROC) glSupport.getProcAddress("glSampleMapATI");
	glColorFragmentOp1ATI_ptr      = (PFNGLCOLORFRAGMENTOP1ATIPROC) glSupport.getProcAddress("glColorFragmentOp1ATI");
	glColorFragmentOp2ATI_ptr      = (PFNGLCOLORFRAGMENTOP2ATIPROC) glSupport.getProcAddress("glColorFragmentOp2ATI");
	glColorFragmentOp3ATI_ptr      = (PFNGLCOLORFRAGMENTOP3ATIPROC) glSupport.getProcAddress("glColorFragmentOp3ATI");
	glAlphaFragmentOp1ATI_ptr      = (PFNGLALPHAFRAGMENTOP1ATIPROC) glSupport.getProcAddress("glAlphaFragmentOp1ATI");
	glAlphaFragmentOp2ATI_ptr      = (PFNGLALPHAFRAGMENTOP2ATIPROC) glSupport.getProcAddress("glAlphaFragmentOp2ATI");
	glAlphaFragmentOp3ATI_ptr      = (PFNGLALPHAFRAGMENTOP3ATIPROC) glSupport.getProcAddress("glAlphaFragmentOp3ATI");
	glSetFragmentShaderConstantATI_ptr = (PFNGLSETFRAGMENTSHADERCONSTANTATIPROC) glSupport.getProcAddress("glSetFragmentShaderConstantATI");

	if (glGenFragmentShadersATI_ptr == NULL) return false;

	if (glBindFragmentShaderATI_ptr == NULL) return false;

	if (glDeleteFragmentShaderATI_ptr == NULL) return false;

	if (glBeginFragmentShaderATI_ptr == NULL) return false;

	if (glEndFragmentShaderATI_ptr == NULL) return false;

	if (glPassTexCoordATI_ptr == NULL) return false;

	if (glColorFragmentOp1ATI_ptr == NULL) return false;

	if (glColorFragmentOp2ATI_ptr == NULL) return false;

	if (glColorFragmentOp3ATI_ptr == NULL) return false;

	if (glAlphaFragmentOp1ATI_ptr == NULL) return false;

	if (glAlphaFragmentOp2ATI_ptr == NULL) return false;

	if (glAlphaFragmentOp2ATI_ptr == NULL) return false;

	if (glAlphaFragmentOp3ATI_ptr == NULL) return false;

	if (glSetFragmentShaderConstantATI_ptr == NULL) return false;

    init = true;

	return true;
}

