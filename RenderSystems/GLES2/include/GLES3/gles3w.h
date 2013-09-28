#ifndef __gles3w_h_
#define __gles3w_h_

#if defined(__APPLE__) || defined(__APPLE_CC__)
#   include <OpenGLES/ES3/gl.h>
    // Prevent Apple's non-standard extension header from being included
#   define __gl_es30ext_h_
#else
#   include <GLES3/gl3.h>
#endif

#include <KHR/khrplatform.h>
#include <GLES3/gl3platform.h>
#include <GLES3/gl3ext.h>

#ifndef __gl3_h_
#define __gl3_h_
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* glesw api */
int gleswInit(void);
int gleswIsSupported(int major, int minor);
void *gleswGetProcAddress(const char *proc);

/* OpenGL functions */
typedef void           (GL_APIENTRY* PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void           (GL_APIENTRY* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void           (GL_APIENTRY* PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void           (GL_APIENTRY* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void           (GL_APIENTRY* PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void           (GL_APIENTRY* PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void           (GL_APIENTRY* PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void           (GL_APIENTRY* PFNGLBLENDCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (GL_APIENTRY* PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void           (GL_APIENTRY* PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void           (GL_APIENTRY* PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void           (GL_APIENTRY* PFNGLBLENDFUNCSEPARATEPROC) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void           (GL_APIENTRY* PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void           (GL_APIENTRY* PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef GLenum         (GL_APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void           (GL_APIENTRY* PFNGLCLEARPROC) (GLbitfield mask);
typedef void           (GL_APIENTRY* PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void           (GL_APIENTRY* PFNGLCLEARDEPTHFPROC) (GLfloat depth);
typedef void           (GL_APIENTRY* PFNGLCLEARSTENCILPROC) (GLint s);
typedef void           (GL_APIENTRY* PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void           (GL_APIENTRY* PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void           (GL_APIENTRY* PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY* PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void           (GL_APIENTRY* PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint         (GL_APIENTRY* PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint         (GL_APIENTRY* PFNGLCREATESHADERPROC) (GLenum type);
typedef void           (GL_APIENTRY* PFNGLCULLFACEPROC) (GLenum mode);
typedef void           (GL_APIENTRY* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void           (GL_APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
typedef void           (GL_APIENTRY* PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void           (GL_APIENTRY* PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void           (GL_APIENTRY* PFNGLDELETESHADERPROC) (GLuint shader);
typedef void           (GL_APIENTRY* PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint* textures);
typedef void           (GL_APIENTRY* PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void           (GL_APIENTRY* PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void           (GL_APIENTRY* PFNGLDEPTHRANGEFPROC) (GLfloat n, GLfloat f);
typedef void           (GL_APIENTRY* PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void           (GL_APIENTRY* PFNGLDISABLEPROC) (GLenum cap);
typedef void           (GL_APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void           (GL_APIENTRY* PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void           (GL_APIENTRY* PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
typedef void           (GL_APIENTRY* PFNGLENABLEPROC) (GLenum cap);
typedef void           (GL_APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void           (GL_APIENTRY* PFNGLFINISHPROC) (void);
typedef void           (GL_APIENTRY* PFNGLFLUSHPROC) (void);
typedef void           (GL_APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void           (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void           (GL_APIENTRY* PFNGLFRONTFACEPROC) (GLenum mode);
typedef void           (GL_APIENTRY* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void           (GL_APIENTRY* PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void           (GL_APIENTRY* PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
typedef void           (GL_APIENTRY* PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
typedef void           (GL_APIENTRY* PFNGLGENTEXTURESPROC) (GLsizei n, GLuint* textures);
typedef void           (GL_APIENTRY* PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY* PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY* PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
typedef GLint          (GL_APIENTRY* PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void           (GL_APIENTRY* PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean* params);
typedef void           (GL_APIENTRY* PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLenum         (GL_APIENTRY* PFNGLGETERRORPROC) (void);
typedef void           (GL_APIENTRY* PFNGLGETFLOATVPROC) (GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETINTEGERVPROC) (GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void           (GL_APIENTRY* PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void           (GL_APIENTRY* PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
typedef void           (GL_APIENTRY* PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
typedef const GLubyte* (GL_APIENTRY* PFNGLGETSTRINGPROC) (GLenum name);
typedef void           (GL_APIENTRY* PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint* params);
typedef GLint          (GL_APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void           (GL_APIENTRY* PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid** pointer);
typedef void           (GL_APIENTRY* PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef GLboolean      (GL_APIENTRY* PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean      (GL_APIENTRY* PFNGLISENABLEDPROC) (GLenum cap);
typedef GLboolean      (GL_APIENTRY* PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef GLboolean      (GL_APIENTRY* PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean      (GL_APIENTRY* PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef GLboolean      (GL_APIENTRY* PFNGLISSHADERPROC) (GLuint shader);
typedef GLboolean      (GL_APIENTRY* PFNGLISTEXTUREPROC) (GLuint texture);
typedef void           (GL_APIENTRY* PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void           (GL_APIENTRY* PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void           (GL_APIENTRY* PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void           (GL_APIENTRY* PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void           (GL_APIENTRY* PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef void           (GL_APIENTRY* PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void           (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLSAMPLECOVERAGEPROC) (GLfloat value, GLboolean invert);
typedef void           (GL_APIENTRY* PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLSHADERBINARYPROC) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
typedef void           (GL_APIENTRY* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void           (GL_APIENTRY* PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void           (GL_APIENTRY* PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void           (GL_APIENTRY* PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void           (GL_APIENTRY* PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void           (GL_APIENTRY* PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void           (GL_APIENTRY* PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef void           (GL_APIENTRY* PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY* PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void           (GL_APIENTRY* PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void           (GL_APIENTRY* PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint* params);
typedef void           (GL_APIENTRY* PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1FPROC) (GLint location, GLfloat x);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1IPROC) (GLint location, GLint x);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2FPROC) (GLint location, GLfloat x, GLfloat y);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2IPROC) (GLint location, GLint x, GLint y);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3FPROC) (GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3IPROC) (GLint location, GLint x, GLint y, GLint z);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4FPROC) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4IPROC) (GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void           (GL_APIENTRY* PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB1FPROC) (GLuint indx, GLfloat x);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB1FVPROC) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB2FPROC) (GLuint indx, GLfloat x, GLfloat y);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB2FVPROC) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB3FPROC) (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB3FVPROC) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB4FPROC) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIB4FVPROC) (GLuint indx, const GLfloat* values);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
typedef void           (GL_APIENTRY* PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLREADBUFFERPROC) (GLenum mode);
typedef void           (GL_APIENTRY* PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
typedef void           (GL_APIENTRY* PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY* PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void           (GL_APIENTRY* PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void           (GL_APIENTRY* PFNGLGENQUERIESPROC) (GLsizei n, GLuint* ids);
typedef void           (GL_APIENTRY* PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint* ids);
typedef GLboolean      (GL_APIENTRY* PFNGLISQUERYPROC) (GLuint id);
typedef void           (GL_APIENTRY* PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void           (GL_APIENTRY* PFNGLENDQUERYPROC) (GLenum target);
typedef void           (GL_APIENTRY* PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint* params);
typedef GLboolean      (GL_APIENTRY* PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void           (GL_APIENTRY* PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid** params);
typedef void           (GL_APIENTRY* PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void           (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURELAYERPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef GLvoid*        (GL_APIENTRY* PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void           (GL_APIENTRY* PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void           (GL_APIENTRY* PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void           (GL_APIENTRY* PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint* arrays);
typedef void           (GL_APIENTRY* PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
typedef GLboolean      (GL_APIENTRY* PFNGLISVERTEXARRAYPROC) (GLuint array);
typedef void           (GL_APIENTRY* PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index, GLint* data);
typedef void           (GL_APIENTRY* PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
typedef void           (GL_APIENTRY* PFNGLENDTRANSFORMFEEDBACKPROC) (void);
typedef void           (GL_APIENTRY* PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void           (GL_APIENTRY* PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void           (GL_APIENTRY* PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
typedef void           (GL_APIENTRY* PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void           (GL_APIENTRY* PFNGLGETVERTEXATTRIBIIVPROC) (GLuint index, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint index, GLenum pname, GLuint* params);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBI4IPROC) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBI4UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBI4IVPROC) (GLuint index, const GLint* v);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBI4UIVPROC) (GLuint index, const GLuint* v);
typedef void           (GL_APIENTRY* PFNGLGETUNIFORMUIVPROC) (GLuint program, GLint location, GLuint* params);
typedef GLint          (GL_APIENTRY* PFNGLGETFRAGDATALOCATIONPROC) (GLuint program, const GLchar *name);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2UIPROC) (GLint location, GLuint v0, GLuint v1);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void           (GL_APIENTRY* PFNGLUNIFORM1UIVPROC) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORM2UIVPROC) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORM3UIVPROC) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY* PFNGLUNIFORM4UIVPROC) (GLint location, GLsizei count, const GLuint* value);
typedef void           (GL_APIENTRY* PFNGLCLEARBUFFERIVPROC) (GLenum buffer, GLint drawbuffer, const GLint* value);
typedef void           (GL_APIENTRY* PFNGLCLEARBUFFERUIVPROC) (GLenum buffer, GLint drawbuffer, const GLuint* value);
typedef void           (GL_APIENTRY* PFNGLCLEARBUFFERFVPROC) (GLenum buffer, GLint drawbuffer, const GLfloat* value);
typedef void           (GL_APIENTRY* PFNGLCLEARBUFFERFIPROC) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte* (GL_APIENTRY* PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
typedef void           (GL_APIENTRY* PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void           (GL_APIENTRY* PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
typedef void           (GL_APIENTRY* PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
typedef GLuint         (GL_APIENTRY* PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar* uniformBlockName);
typedef void           (GL_APIENTRY* PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
typedef void           (GL_APIENTRY* PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void           (GL_APIENTRY* PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
typedef void           (GL_APIENTRY* PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
typedef GLsync         (GL_APIENTRY* PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean      (GL_APIENTRY* PFNGLISSYNCPROC) (GLsync sync);
typedef void           (GL_APIENTRY* PFNGLDELETESYNCPROC) (GLsync sync);
typedef GLenum         (GL_APIENTRY* PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void           (GL_APIENTRY* PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void           (GL_APIENTRY* PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64* params);
typedef void           (GL_APIENTRY* PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
typedef void           (GL_APIENTRY* PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index, GLint64* data);
typedef void           (GL_APIENTRY* PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target, GLenum pname, GLint64* params);
typedef void           (GL_APIENTRY* PFNGLGENSAMPLERSPROC) (GLsizei count, GLuint* samplers);
typedef void           (GL_APIENTRY* PFNGLDELETESAMPLERSPROC) (GLsizei count, const GLuint* samplers);
typedef GLboolean      (GL_APIENTRY* PFNGLISSAMPLERPROC) (GLuint sampler);
typedef void           (GL_APIENTRY* PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
typedef void           (GL_APIENTRY* PFNGLSAMPLERPARAMETERIPROC) (GLuint sampler, GLenum pname, GLint param);
typedef void           (GL_APIENTRY* PFNGLSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, const GLint* param);
typedef void           (GL_APIENTRY* PFNGLSAMPLERPARAMETERFPROC) (GLuint sampler, GLenum pname, GLfloat param);
typedef void           (GL_APIENTRY* PFNGLSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, const GLfloat* param);
typedef void           (GL_APIENTRY* PFNGLGETSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, GLint* params);
typedef void           (GL_APIENTRY* PFNGLGETSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, GLfloat* params);
typedef void           (GL_APIENTRY* PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
typedef void           (GL_APIENTRY* PFNGLBINDTRANSFORMFEEDBACKPROC) (GLenum target, GLuint id);
typedef void           (GL_APIENTRY* PFNGLDELETETRANSFORMFEEDBACKSPROC) (GLsizei n, const GLuint* ids);
typedef void           (GL_APIENTRY* PFNGLGENTRANSFORMFEEDBACKSPROC) (GLsizei n, GLuint* ids);
typedef GLboolean      (GL_APIENTRY* PFNGLISTRANSFORMFEEDBACKPROC) (GLuint id);
typedef void           (GL_APIENTRY* PFNGLPAUSETRANSFORMFEEDBACKPROC) (void);
typedef void           (GL_APIENTRY* PFNGLRESUMETRANSFORMFEEDBACKPROC) (void);
typedef void           (GL_APIENTRY* PFNGLGETPROGRAMBINARYPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
typedef void           (GL_APIENTRY* PFNGLPROGRAMBINARYPROC) (GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
typedef void           (GL_APIENTRY* PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
typedef void           (GL_APIENTRY* PFNGLINVALIDATEFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
typedef void           (GL_APIENTRY* PFNGLINVALIDATESUBFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLTEXSTORAGE2DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void           (GL_APIENTRY* PFNGLTEXSTORAGE3DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void           (GL_APIENTRY* PFNGLGETINTERNALFORMATIVPROC) (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);
typedef void (GL_APIENTRY* PFNGLLABELOBJECTEXTPROC) (GLenum type, GLuint object, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRY* PFNGLGETOBJECTLABELEXTPROC) (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRY* PFNGLINSERTEVENTMARKEREXTPROC) (GLsizei length, const GLchar *marker);
typedef void (GL_APIENTRY* PFNGLPUSHGROUPMARKEREXTPROC) (GLsizei length, const GLchar *marker);
typedef void (GL_APIENTRY* PFNGLPOPGROUPMARKEREXTPROC) (void);
typedef void (GL_APIENTRY* PFNGLUSEPROGRAMSTAGESEXTPROC) (GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (GL_APIENTRY* PFNGLACTIVESHADERPROGRAMEXTPROC) (GLuint pipeline, GLuint program);
typedef GLuint (GL_APIENTRY* PFNGLCREATESHADERPROGRAMVEXTPROC) (GLenum type, GLsizei count, const GLchar **strings);
typedef void (GL_APIENTRY* PFNGLBINDPROGRAMPIPELINEEXTPROC) (GLuint pipeline);
typedef void (GL_APIENTRY* PFNGLDELETEPROGRAMPIPELINESEXTPROC) (GLsizei n, const GLuint *pipelines);
typedef void (GL_APIENTRY* PFNGLGENPROGRAMPIPELINESEXTPROC) (GLsizei n, GLuint *pipelines);
typedef GLboolean (GL_APIENTRY* PFNGLISPROGRAMPIPELINEEXTPROC) (GLuint pipeline);
typedef void (GL_APIENTRY* PFNGLPROGRAMPARAMETERIEXTPROC) (GLuint program, GLenum pname, GLint value);
typedef void (GL_APIENTRY* PFNGLGETPROGRAMPIPELINEIVEXTPROC) (GLuint pipeline, GLenum pname, GLint *params);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM1IEXTPROC) (GLuint program, GLint location, GLint x);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM2IEXTPROC) (GLuint program, GLint location, GLint x, GLint y);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM3IEXTPROC) (GLuint program, GLint location, GLint x, GLint y, GLint z);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM4IEXTPROC) (GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM1FEXTPROC) (GLuint program, GLint location, GLfloat x);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM2FEXTPROC) (GLuint program, GLint location, GLfloat x, GLfloat y);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM3FEXTPROC) (GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM4FEXTPROC) (GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM1IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM2IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM3IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM4IVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM1FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM2FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM3FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORM4FVEXTPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GL_APIENTRY* PFNGLVALIDATEPROGRAMPIPELINEEXTPROC) (GLuint pipeline);
typedef void (GL_APIENTRY* PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

extern PFNGLACTIVETEXTUREPROC gleswActiveTexture;
extern PFNGLATTACHSHADERPROC gleswAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC gleswBindAttribLocation;
extern PFNGLBINDBUFFERPROC gleswBindBuffer;
extern PFNGLBINDFRAMEBUFFERPROC gleswBindFramebuffer;
extern PFNGLBINDRENDERBUFFERPROC gleswBindRenderbuffer;
extern PFNGLBINDTEXTUREPROC gleswBindTexture;
extern PFNGLBLENDCOLORPROC gleswBlendColor;
extern PFNGLBLENDEQUATIONPROC gleswBlendEquation;
extern PFNGLBLENDEQUATIONSEPARATEPROC gleswBlendEquationSeparate;
extern PFNGLBLENDFUNCPROC gleswBlendFunc;
extern PFNGLBLENDFUNCSEPARATEPROC gleswBlendFuncSeparate;
extern PFNGLBUFFERDATAPROC gleswBufferData;
extern PFNGLBUFFERSUBDATAPROC gleswBufferSubData;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC gleswCheckFramebufferStatus;
extern PFNGLCLEARPROC gleswClear;
extern PFNGLCLEARCOLORPROC gleswClearColor;
extern PFNGLCLEARDEPTHFPROC gleswClearDepthf;
extern PFNGLCLEARSTENCILPROC gleswClearStencil;
extern PFNGLCOLORMASKPROC gleswColorMask;
extern PFNGLCOMPILESHADERPROC gleswCompileShader;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC gleswCompressedTexImage2D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC gleswCompressedTexSubImage2D;
extern PFNGLCOPYTEXIMAGE2DPROC gleswCopyTexImage2D;
extern PFNGLCOPYTEXSUBIMAGE2DPROC gleswCopyTexSubImage2D;
extern PFNGLCREATEPROGRAMPROC gleswCreateProgram;
extern PFNGLCREATESHADERPROC gleswCreateShader;
extern PFNGLCULLFACEPROC gleswCullFace;
extern PFNGLDELETEBUFFERSPROC gleswDeleteBuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC gleswDeleteFramebuffers;
extern PFNGLDELETEPROGRAMPROC gleswDeleteProgram;
extern PFNGLDELETERENDERBUFFERSPROC gleswDeleteRenderbuffers;
extern PFNGLDELETESHADERPROC gleswDeleteShader;
extern PFNGLDELETETEXTURESPROC gleswDeleteTextures;
extern PFNGLDEPTHFUNCPROC gleswDepthFunc;
extern PFNGLDEPTHMASKPROC gleswDepthMask;
extern PFNGLDEPTHRANGEFPROC gleswDepthRangef;
extern PFNGLDETACHSHADERPROC gleswDetachShader;
extern PFNGLDISABLEPROC gleswDisable;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC gleswDisableVertexAttribArray;
extern PFNGLDRAWARRAYSPROC gleswDrawArrays;
extern PFNGLDRAWELEMENTSPROC gleswDrawElements;
extern PFNGLENABLEPROC gleswEnable;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC gleswEnableVertexAttribArray;
extern PFNGLFINISHPROC gleswFinish;
extern PFNGLFLUSHPROC gleswFlush;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC gleswFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC gleswFramebufferTexture2D;
extern PFNGLFRONTFACEPROC gleswFrontFace;
extern PFNGLGENBUFFERSPROC gleswGenBuffers;
extern PFNGLGENERATEMIPMAPPROC gleswGenerateMipmap;
extern PFNGLGENFRAMEBUFFERSPROC gleswGenFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC gleswGenRenderbuffers;
extern PFNGLGENTEXTURESPROC gleswGenTextures;
extern PFNGLGETACTIVEATTRIBPROC gleswGetActiveAttrib;
extern PFNGLGETACTIVEUNIFORMPROC gleswGetActiveUniform;
extern PFNGLGETATTACHEDSHADERSPROC gleswGetAttachedShaders;
extern PFNGLGETATTRIBLOCATIONPROC gleswGetAttribLocation;
extern PFNGLGETBOOLEANVPROC gleswGetBooleanv;
extern PFNGLGETBUFFERPARAMETERIVPROC gleswGetBufferParameteriv;
extern PFNGLGETERRORPROC gleswGetError;
extern PFNGLGETFLOATVPROC gleswGetFloatv;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC gleswGetFramebufferAttachmentParameteriv;
extern PFNGLGETINTEGERVPROC gleswGetIntegerv;
extern PFNGLGETPROGRAMIVPROC gleswGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC gleswGetProgramInfoLog;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC gleswGetRenderbufferParameteriv;
extern PFNGLGETSHADERIVPROC gleswGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC gleswGetShaderInfoLog;
extern PFNGLGETSHADERPRECISIONFORMATPROC gleswGetShaderPrecisionFormat;
extern PFNGLGETSHADERSOURCEPROC gleswGetShaderSource;
extern PFNGLGETSTRINGPROC gleswGetString;
extern PFNGLGETTEXPARAMETERFVPROC gleswGetTexParameterfv;
extern PFNGLGETTEXPARAMETERIVPROC gleswGetTexParameteriv;
extern PFNGLGETUNIFORMFVPROC gleswGetUniformfv;
extern PFNGLGETUNIFORMIVPROC gleswGetUniformiv;
extern PFNGLGETUNIFORMLOCATIONPROC gleswGetUniformLocation;
extern PFNGLGETVERTEXATTRIBFVPROC gleswGetVertexAttribfv;
extern PFNGLGETVERTEXATTRIBIVPROC gleswGetVertexAttribiv;
extern PFNGLGETVERTEXATTRIBPOINTERVPROC gleswGetVertexAttribPointerv;
extern PFNGLHINTPROC gleswHint;
extern PFNGLISBUFFERPROC gleswIsBuffer;
extern PFNGLISENABLEDPROC gleswIsEnabled;
extern PFNGLISFRAMEBUFFERPROC gleswIsFramebuffer;
extern PFNGLISPROGRAMPROC gleswIsProgram;
extern PFNGLISRENDERBUFFERPROC gleswIsRenderbuffer;
extern PFNGLISSHADERPROC gleswIsShader;
extern PFNGLISTEXTUREPROC gleswIsTexture;
extern PFNGLLINEWIDTHPROC gleswLineWidth;
extern PFNGLLINKPROGRAMPROC gleswLinkProgram;
extern PFNGLPIXELSTOREIPROC gleswPixelStorei;
extern PFNGLPOLYGONOFFSETPROC gleswPolygonOffset;
extern PFNGLREADPIXELSPROC gleswReadPixels;
extern PFNGLRELEASESHADERCOMPILERPROC gleswReleaseShaderCompiler;
extern PFNGLRENDERBUFFERSTORAGEPROC gleswRenderbufferStorage;
extern PFNGLSAMPLECOVERAGEPROC gleswSampleCoverage;
extern PFNGLSCISSORPROC gleswScissor;
extern PFNGLSHADERBINARYPROC gleswShaderBinary;
extern PFNGLSHADERSOURCEPROC gleswShaderSource;
extern PFNGLSTENCILFUNCPROC gleswStencilFunc;
extern PFNGLSTENCILFUNCSEPARATEPROC gleswStencilFuncSeparate;
extern PFNGLSTENCILMASKPROC gleswStencilMask;
extern PFNGLSTENCILMASKSEPARATEPROC gleswStencilMaskSeparate;
extern PFNGLSTENCILOPPROC gleswStencilOp;
extern PFNGLSTENCILOPSEPARATEPROC gleswStencilOpSeparate;
extern PFNGLTEXIMAGE2DPROC gleswTexImage2D;
extern PFNGLTEXPARAMETERFPROC gleswTexParameterf;
extern PFNGLTEXPARAMETERFVPROC gleswTexParameterfv;
extern PFNGLTEXPARAMETERIPROC gleswTexParameteri;
extern PFNGLTEXPARAMETERIVPROC gleswTexParameteriv;
extern PFNGLTEXSUBIMAGE2DPROC gleswTexSubImage2D;
extern PFNGLUNIFORM1FPROC gleswUniform1f;
extern PFNGLUNIFORM1FVPROC gleswUniform1fv;
extern PFNGLUNIFORM1IPROC gleswUniform1i;
extern PFNGLUNIFORM1IVPROC gleswUniform1iv;
extern PFNGLUNIFORM2FPROC gleswUniform2f;
extern PFNGLUNIFORM2FVPROC gleswUniform2fv;
extern PFNGLUNIFORM2IPROC gleswUniform2i;
extern PFNGLUNIFORM2IVPROC gleswUniform2iv;
extern PFNGLUNIFORM3FPROC gleswUniform3f;
extern PFNGLUNIFORM3FVPROC gleswUniform3fv;
extern PFNGLUNIFORM3IPROC gleswUniform3i;
extern PFNGLUNIFORM3IVPROC gleswUniform3iv;
extern PFNGLUNIFORM4FPROC gleswUniform4f;
extern PFNGLUNIFORM4FVPROC gleswUniform4fv;
extern PFNGLUNIFORM4IPROC gleswUniform4i;
extern PFNGLUNIFORM4IVPROC gleswUniform4iv;
extern PFNGLUNIFORMMATRIX2FVPROC gleswUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC gleswUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC gleswUniformMatrix4fv;
extern PFNGLUSEPROGRAMPROC gleswUseProgram;
extern PFNGLVALIDATEPROGRAMPROC gleswValidateProgram;
extern PFNGLVERTEXATTRIB1FPROC gleswVertexAttrib1f;
extern PFNGLVERTEXATTRIB1FVPROC gleswVertexAttrib1fv;
extern PFNGLVERTEXATTRIB2FPROC gleswVertexAttrib2f;
extern PFNGLVERTEXATTRIB2FVPROC gleswVertexAttrib2fv;
extern PFNGLVERTEXATTRIB3FPROC gleswVertexAttrib3f;
extern PFNGLVERTEXATTRIB3FVPROC gleswVertexAttrib3fv;
extern PFNGLVERTEXATTRIB4FPROC gleswVertexAttrib4f;
extern PFNGLVERTEXATTRIB4FVPROC gleswVertexAttrib4fv;
extern PFNGLVERTEXATTRIBPOINTERPROC gleswVertexAttribPointer;
extern PFNGLVIEWPORTPROC gleswViewport;
extern PFNGLREADBUFFERPROC gleswReadBuffer;
extern PFNGLDRAWRANGEELEMENTSPROC gleswDrawRangeElements;
extern PFNGLTEXIMAGE3DPROC gleswTexImage3D;
extern PFNGLTEXSUBIMAGE3DPROC gleswTexSubImage3D;
extern PFNGLCOPYTEXSUBIMAGE3DPROC gleswCopyTexSubImage3D;
extern PFNGLCOMPRESSEDTEXIMAGE3DPROC gleswCompressedTexImage3D;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC gleswCompressedTexSubImage3D;
extern PFNGLGENQUERIESPROC gleswGenQueries;
extern PFNGLDELETEQUERIESPROC gleswDeleteQueries;
extern PFNGLISQUERYPROC gleswIsQuery;
extern PFNGLBEGINQUERYPROC gleswBeginQuery;
extern PFNGLENDQUERYPROC gleswEndQuery;
extern PFNGLGETQUERYIVPROC gleswGetQueryiv;
extern PFNGLGETQUERYOBJECTUIVPROC gleswGetQueryObjectuiv;
extern PFNGLUNMAPBUFFERPROC gleswUnmapBuffer;
extern PFNGLGETBUFFERPOINTERVPROC gleswGetBufferPointerv;
extern PFNGLDRAWBUFFERSPROC gleswDrawBuffers;
extern PFNGLUNIFORMMATRIX2X3FVPROC gleswUniformMatrix2x3fv;
extern PFNGLUNIFORMMATRIX3X2FVPROC gleswUniformMatrix3x2fv;
extern PFNGLUNIFORMMATRIX2X4FVPROC gleswUniformMatrix2x4fv;
extern PFNGLUNIFORMMATRIX4X2FVPROC gleswUniformMatrix4x2fv;
extern PFNGLUNIFORMMATRIX3X4FVPROC gleswUniformMatrix3x4fv;
extern PFNGLUNIFORMMATRIX4X3FVPROC gleswUniformMatrix4x3fv;
extern PFNGLBLITFRAMEBUFFERPROC gleswBlitFramebuffer;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC gleswRenderbufferStorageMultisample;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC gleswFramebufferTextureLayer;
extern PFNGLMAPBUFFERRANGEPROC gleswMapBufferRange;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC gleswFlushMappedBufferRange;
extern PFNGLBINDVERTEXARRAYPROC gleswBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC gleswDeleteVertexArrays;
extern PFNGLGENVERTEXARRAYSPROC gleswGenVertexArrays;
extern PFNGLISVERTEXARRAYPROC gleswIsVertexArray;
extern PFNGLGETINTEGERI_VPROC gleswGetIntegeri_v;
extern PFNGLBEGINTRANSFORMFEEDBACKPROC gleswBeginTransformFeedback;
extern PFNGLENDTRANSFORMFEEDBACKPROC gleswEndTransformFeedback;
extern PFNGLBINDBUFFERRANGEPROC gleswBindBufferRange;
extern PFNGLBINDBUFFERBASEPROC gleswBindBufferBase;
extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC gleswTransformFeedbackVaryings;
extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC gleswGetTransformFeedbackVarying;
extern PFNGLVERTEXATTRIBIPOINTERPROC gleswVertexAttribIPointer;
extern PFNGLGETVERTEXATTRIBIIVPROC gleswGetVertexAttribIiv;
extern PFNGLGETVERTEXATTRIBIUIVPROC gleswGetVertexAttribIuiv;
extern PFNGLVERTEXATTRIBI4IPROC gleswVertexAttribI4i;
extern PFNGLVERTEXATTRIBI4UIPROC gleswVertexAttribI4ui;
extern PFNGLVERTEXATTRIBI4IVPROC gleswVertexAttribI4iv;
extern PFNGLVERTEXATTRIBI4UIVPROC gleswVertexAttribI4uiv;
extern PFNGLGETUNIFORMUIVPROC gleswGetUniformuiv;
extern PFNGLGETFRAGDATALOCATIONPROC gleswGetFragDataLocation;
extern PFNGLUNIFORM1UIPROC gleswUniform1ui;
extern PFNGLUNIFORM2UIPROC gleswUniform2ui;
extern PFNGLUNIFORM3UIPROC gleswUniform3ui;
extern PFNGLUNIFORM4UIPROC gleswUniform4ui;
extern PFNGLUNIFORM1UIVPROC gleswUniform1uiv;
extern PFNGLUNIFORM2UIVPROC gleswUniform2uiv;
extern PFNGLUNIFORM3UIVPROC gleswUniform3uiv;
extern PFNGLUNIFORM4UIVPROC gleswUniform4uiv;
extern PFNGLCLEARBUFFERIVPROC gleswClearBufferiv;
extern PFNGLCLEARBUFFERUIVPROC gleswClearBufferuiv;
extern PFNGLCLEARBUFFERFVPROC gleswClearBufferfv;
extern PFNGLCLEARBUFFERFIPROC gleswClearBufferfi;
extern PFNGLGETSTRINGIPROC gleswGetStringi;
extern PFNGLCOPYBUFFERSUBDATAPROC gleswCopyBufferSubData;
extern PFNGLGETUNIFORMINDICESPROC gleswGetUniformIndices;
extern PFNGLGETACTIVEUNIFORMSIVPROC gleswGetActiveUniformsiv;
extern PFNGLGETUNIFORMBLOCKINDEXPROC gleswGetUniformBlockIndex;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC gleswGetActiveUniformBlockiv;
extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC gleswGetActiveUniformBlockName;
extern PFNGLUNIFORMBLOCKBINDINGPROC gleswUniformBlockBinding;
extern PFNGLDRAWARRAYSINSTANCEDPROC gleswDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC gleswDrawElementsInstanced;
extern PFNGLFENCESYNCPROC gleswFenceSync;
extern PFNGLISSYNCPROC gleswIsSync;
extern PFNGLDELETESYNCPROC gleswDeleteSync;
extern PFNGLCLIENTWAITSYNCPROC gleswClientWaitSync;
extern PFNGLWAITSYNCPROC gleswWaitSync;
extern PFNGLGETINTEGER64VPROC gleswGetInteger64v;
extern PFNGLGETSYNCIVPROC gleswGetSynciv;
extern PFNGLGETINTEGER64I_VPROC gleswGetInteger64i_v;
extern PFNGLGETBUFFERPARAMETERI64VPROC gleswGetBufferParameteri64v;
extern PFNGLGENSAMPLERSPROC gleswGenSamplers;
extern PFNGLDELETESAMPLERSPROC gleswDeleteSamplers;
extern PFNGLISSAMPLERPROC gleswIsSampler;
extern PFNGLBINDSAMPLERPROC gleswBindSampler;
extern PFNGLSAMPLERPARAMETERIPROC gleswSamplerParameteri;
extern PFNGLSAMPLERPARAMETERIVPROC gleswSamplerParameteriv;
extern PFNGLSAMPLERPARAMETERFPROC gleswSamplerParameterf;
extern PFNGLSAMPLERPARAMETERFVPROC gleswSamplerParameterfv;
extern PFNGLGETSAMPLERPARAMETERIVPROC gleswGetSamplerParameteriv;
extern PFNGLGETSAMPLERPARAMETERFVPROC gleswGetSamplerParameterfv;
extern PFNGLVERTEXATTRIBDIVISORPROC gleswVertexAttribDivisor;
extern PFNGLBINDTRANSFORMFEEDBACKPROC gleswBindTransformFeedback;
extern PFNGLDELETETRANSFORMFEEDBACKSPROC gleswDeleteTransformFeedbacks;
extern PFNGLGENTRANSFORMFEEDBACKSPROC gleswGenTransformFeedbacks;
extern PFNGLISTRANSFORMFEEDBACKPROC gleswIsTransformFeedback;
extern PFNGLPAUSETRANSFORMFEEDBACKPROC gleswPauseTransformFeedback;
extern PFNGLRESUMETRANSFORMFEEDBACKPROC gleswResumeTransformFeedback;
extern PFNGLGETPROGRAMBINARYPROC gleswGetProgramBinary;
extern PFNGLPROGRAMBINARYPROC gleswProgramBinary;
extern PFNGLPROGRAMPARAMETERIPROC gleswProgramParameteri;
extern PFNGLINVALIDATEFRAMEBUFFERPROC gleswInvalidateFramebuffer;
extern PFNGLINVALIDATESUBFRAMEBUFFERPROC gleswInvalidateSubFramebuffer;
extern PFNGLTEXSTORAGE2DPROC gleswTexStorage2D;
extern PFNGLTEXSTORAGE3DPROC gleswTexStorage3D;
extern PFNGLGETINTERNALFORMATIVPROC gleswGetInternalformativ;
extern PFNGLLABELOBJECTEXTPROC gleswLabelObjectEXT;
extern PFNGLGETOBJECTLABELEXTPROC gleswGetObjectLabelEXT;
extern PFNGLINSERTEVENTMARKEREXTPROC gleswInsertEventMarkerEXT;
extern PFNGLPUSHGROUPMARKEREXTPROC gleswPushGroupMarkerEXT;
extern PFNGLPOPGROUPMARKEREXTPROC gleswPopGroupMarkerEXT;
extern PFNGLUSEPROGRAMSTAGESEXTPROC gleswUseProgramStagesEXT;
extern PFNGLACTIVESHADERPROGRAMEXTPROC gleswActiveShaderProgramEXT;
extern PFNGLCREATESHADERPROGRAMVEXTPROC gleswCreateShaderProgramvEXT;
extern PFNGLBINDPROGRAMPIPELINEEXTPROC gleswBindProgramPipelineEXT;
extern PFNGLDELETEPROGRAMPIPELINESEXTPROC gleswDeleteProgramPipelinesEXT;
extern PFNGLGENPROGRAMPIPELINESEXTPROC gleswGenProgramPipelinesEXT;
extern PFNGLISPROGRAMPIPELINEEXTPROC gleswIsProgramPipelineEXT;
extern PFNGLPROGRAMPARAMETERIEXTPROC gleswProgramParameteriEXT;
extern PFNGLGETPROGRAMPIPELINEIVEXTPROC gleswGetProgramPipelineivEXT;
extern PFNGLPROGRAMUNIFORM1IEXTPROC gleswProgramUniform1iEXT;
extern PFNGLPROGRAMUNIFORM2IEXTPROC gleswProgramUniform2iEXT;
extern PFNGLPROGRAMUNIFORM3IEXTPROC gleswProgramUniform3iEXT;
extern PFNGLPROGRAMUNIFORM4IEXTPROC gleswProgramUniform4iEXT;
extern PFNGLPROGRAMUNIFORM1FEXTPROC gleswProgramUniform1fEXT;
extern PFNGLPROGRAMUNIFORM2FEXTPROC gleswProgramUniform2fEXT;
extern PFNGLPROGRAMUNIFORM3FEXTPROC gleswProgramUniform3fEXT;
extern PFNGLPROGRAMUNIFORM4FEXTPROC gleswProgramUniform4fEXT;
extern PFNGLPROGRAMUNIFORM1IVEXTPROC gleswProgramUniform1ivEXT;
extern PFNGLPROGRAMUNIFORM2IVEXTPROC gleswProgramUniform2ivEXT;
extern PFNGLPROGRAMUNIFORM3IVEXTPROC gleswProgramUniform3ivEXT;
extern PFNGLPROGRAMUNIFORM4IVEXTPROC gleswProgramUniform4ivEXT;
extern PFNGLPROGRAMUNIFORM1FVEXTPROC gleswProgramUniform1fvEXT;
extern PFNGLPROGRAMUNIFORM2FVEXTPROC gleswProgramUniform2fvEXT;
extern PFNGLPROGRAMUNIFORM3FVEXTPROC gleswProgramUniform3fvEXT;
extern PFNGLPROGRAMUNIFORM4FVEXTPROC gleswProgramUniform4fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC gleswProgramUniformMatrix2fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC gleswProgramUniformMatrix3fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC gleswProgramUniformMatrix4fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC gleswProgramUniformMatrix2x3fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC gleswProgramUniformMatrix3x2fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC gleswProgramUniformMatrix2x4fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC gleswProgramUniformMatrix4x2fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC gleswProgramUniformMatrix3x4fvEXT;
extern PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC gleswProgramUniformMatrix4x3fvEXT;
extern PFNGLVALIDATEPROGRAMPIPELINEEXTPROC gleswValidateProgramPipelineEXT;
extern PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC gleswGetProgramPipelineInfoLogEXT;

#define glActiveTexture		gleswActiveTexture
#define glAttachShader		gleswAttachShader
#define glBindAttribLocation		gleswBindAttribLocation
#define glBindBuffer		gleswBindBuffer
#define glBindFramebuffer		gleswBindFramebuffer
#define glBindRenderbuffer		gleswBindRenderbuffer
#define glBindTexture		gleswBindTexture
#define glBlendColor		gleswBlendColor
#define glBlendEquation		gleswBlendEquation
#define glBlendEquationSeparate		gleswBlendEquationSeparate
#define glBlendFunc		gleswBlendFunc
#define glBlendFuncSeparate		gleswBlendFuncSeparate
#define glBufferData		gleswBufferData
#define glBufferSubData		gleswBufferSubData
#define glCheckFramebufferStatus		gleswCheckFramebufferStatus
#define glClear		gleswClear
#define glClearColor		gleswClearColor
#define glClearDepthf		gleswClearDepthf
#define glClearStencil		gleswClearStencil
#define glColorMask		gleswColorMask
#define glCompileShader		gleswCompileShader
#define glCompressedTexImage2D		gleswCompressedTexImage2D
#define glCompressedTexSubImage2D		gleswCompressedTexSubImage2D
#define glCopyTexImage2D		gleswCopyTexImage2D
#define glCopyTexSubImage2D		gleswCopyTexSubImage2D
#define glCreateProgram		gleswCreateProgram
#define glCreateShader		gleswCreateShader
#define glCullFace		gleswCullFace
#define glDeleteBuffers		gleswDeleteBuffers
#define glDeleteFramebuffers		gleswDeleteFramebuffers
#define glDeleteProgram		gleswDeleteProgram
#define glDeleteRenderbuffers		gleswDeleteRenderbuffers
#define glDeleteShader		gleswDeleteShader
#define glDeleteTextures		gleswDeleteTextures
#define glDepthFunc		gleswDepthFunc
#define glDepthMask		gleswDepthMask
#define glDepthRangef		gleswDepthRangef
#define glDetachShader		gleswDetachShader
#define glDisable		gleswDisable
#define glDisableVertexAttribArray		gleswDisableVertexAttribArray
#define glDrawArrays		gleswDrawArrays
#define glDrawElements		gleswDrawElements
#define glEnable		gleswEnable
#define glEnableVertexAttribArray		gleswEnableVertexAttribArray
#define glFinish		gleswFinish
#define glFlush		gleswFlush
#define glFramebufferRenderbuffer		gleswFramebufferRenderbuffer
#define glFramebufferTexture2D		gleswFramebufferTexture2D
#define glFrontFace		gleswFrontFace
#define glGenBuffers		gleswGenBuffers
#define glGenerateMipmap		gleswGenerateMipmap
#define glGenFramebuffers		gleswGenFramebuffers
#define glGenRenderbuffers		gleswGenRenderbuffers
#define glGenTextures		gleswGenTextures
#define glGetActiveAttrib		gleswGetActiveAttrib
#define glGetActiveUniform		gleswGetActiveUniform
#define glGetAttachedShaders		gleswGetAttachedShaders
#define glGetAttribLocation		gleswGetAttribLocation
#define glGetBooleanv		gleswGetBooleanv
#define glGetBufferParameteriv		gleswGetBufferParameteriv
#define glGetError		gleswGetError
#define glGetFloatv		gleswGetFloatv
#define glGetFramebufferAttachmentParameteriv		gleswGetFramebufferAttachmentParameteriv
#define glGetIntegerv		gleswGetIntegerv
#define glGetProgramiv		gleswGetProgramiv
#define glGetProgramInfoLog		gleswGetProgramInfoLog
#define glGetRenderbufferParameteriv		gleswGetRenderbufferParameteriv
#define glGetShaderiv		gleswGetShaderiv
#define glGetShaderInfoLog		gleswGetShaderInfoLog
#define glGetShaderPrecisionFormat		gleswGetShaderPrecisionFormat
#define glGetShaderSource		gleswGetShaderSource
#define glGetString		gleswGetString
#define glGetTexParameterfv		gleswGetTexParameterfv
#define glGetTexParameteriv		gleswGetTexParameteriv
#define glGetUniformfv		gleswGetUniformfv
#define glGetUniformiv		gleswGetUniformiv
#define glGetUniformLocation		gleswGetUniformLocation
#define glGetVertexAttribfv		gleswGetVertexAttribfv
#define glGetVertexAttribiv		gleswGetVertexAttribiv
#define glGetVertexAttribPointerv		gleswGetVertexAttribPointerv
#define glHint		gleswHint
#define glIsBuffer		gleswIsBuffer
#define glIsEnabled		gleswIsEnabled
#define glIsFramebuffer		gleswIsFramebuffer
#define glIsProgram		gleswIsProgram
#define glIsRenderbuffer		gleswIsRenderbuffer
#define glIsShader		gleswIsShader
#define glIsTexture		gleswIsTexture
#define glLineWidth		gleswLineWidth
#define glLinkProgram		gleswLinkProgram
#define glPixelStorei		gleswPixelStorei
#define glPolygonOffset		gleswPolygonOffset
#define glReadPixels		gleswReadPixels
#define glReleaseShaderCompiler		gleswReleaseShaderCompiler
#define glRenderbufferStorage		gleswRenderbufferStorage
#define glSampleCoverage		gleswSampleCoverage
#define glScissor		gleswScissor
#define glShaderBinary		gleswShaderBinary
#define glShaderSource		gleswShaderSource
#define glStencilFunc		gleswStencilFunc
#define glStencilFuncSeparate		gleswStencilFuncSeparate
#define glStencilMask		gleswStencilMask
#define glStencilMaskSeparate		gleswStencilMaskSeparate
#define glStencilOp		gleswStencilOp
#define glStencilOpSeparate		gleswStencilOpSeparate
#define glTexImage2D		gleswTexImage2D
#define glTexParameterf		gleswTexParameterf
#define glTexParameterfv		gleswTexParameterfv
#define glTexParameteri		gleswTexParameteri
#define glTexParameteriv		gleswTexParameteriv
#define glTexSubImage2D		gleswTexSubImage2D
#define glUniform1f		gleswUniform1f
#define glUniform1fv		gleswUniform1fv
#define glUniform1i		gleswUniform1i
#define glUniform1iv		gleswUniform1iv
#define glUniform2f		gleswUniform2f
#define glUniform2fv		gleswUniform2fv
#define glUniform2i		gleswUniform2i
#define glUniform2iv		gleswUniform2iv
#define glUniform3f		gleswUniform3f
#define glUniform3fv		gleswUniform3fv
#define glUniform3i		gleswUniform3i
#define glUniform3iv		gleswUniform3iv
#define glUniform4f		gleswUniform4f
#define glUniform4fv		gleswUniform4fv
#define glUniform4i		gleswUniform4i
#define glUniform4iv		gleswUniform4iv
#define glUniformMatrix2fv		gleswUniformMatrix2fv
#define glUniformMatrix3fv		gleswUniformMatrix3fv
#define glUniformMatrix4fv		gleswUniformMatrix4fv
#define glUseProgram		gleswUseProgram
#define glValidateProgram		gleswValidateProgram
#define glVertexAttrib1f		gleswVertexAttrib1f
#define glVertexAttrib1fv		gleswVertexAttrib1fv
#define glVertexAttrib2f		gleswVertexAttrib2f
#define glVertexAttrib2fv		gleswVertexAttrib2fv
#define glVertexAttrib3f		gleswVertexAttrib3f
#define glVertexAttrib3fv		gleswVertexAttrib3fv
#define glVertexAttrib4f		gleswVertexAttrib4f
#define glVertexAttrib4fv		gleswVertexAttrib4fv
#define glVertexAttribPointer		gleswVertexAttribPointer
#define glViewport		gleswViewport
#define glReadBuffer		gleswReadBuffer
#define glDrawRangeElements		gleswDrawRangeElements
#define glTexImage3D		gleswTexImage3D
#define glTexSubImage3D		gleswTexSubImage3D
#define glCopyTexSubImage3D		gleswCopyTexSubImage3D
#define glCompressedTexImage3D		gleswCompressedTexImage3D
#define glCompressedTexSubImage3D		gleswCompressedTexSubImage3D
#define glGenQueries		gleswGenQueries
#define glDeleteQueries		gleswDeleteQueries
#define glIsQuery		gleswIsQuery
#define glBeginQuery		gleswBeginQuery
#define glEndQuery		gleswEndQuery
#define glGetQueryiv		gleswGetQueryiv
#define glGetQueryObjectuiv		gleswGetQueryObjectuiv
#define glUnmapBuffer		gleswUnmapBuffer
#define glGetBufferPointerv		gleswGetBufferPointerv
#define glDrawBuffers		gleswDrawBuffers
#define glUniformMatrix2x3fv		gleswUniformMatrix2x3fv
#define glUniformMatrix3x2fv		gleswUniformMatrix3x2fv
#define glUniformMatrix2x4fv		gleswUniformMatrix2x4fv
#define glUniformMatrix4x2fv		gleswUniformMatrix4x2fv
#define glUniformMatrix3x4fv		gleswUniformMatrix3x4fv
#define glUniformMatrix4x3fv		gleswUniformMatrix4x3fv
#define glBlitFramebuffer		gleswBlitFramebuffer
#define glRenderbufferStorageMultisample		gleswRenderbufferStorageMultisample
#define glFramebufferTextureLayer		gleswFramebufferTextureLayer
#define glMapBufferRange		gleswMapBufferRange
#define glFlushMappedBufferRange		gleswFlushMappedBufferRange
#define glBindVertexArray		gleswBindVertexArray
#define glDeleteVertexArrays		gleswDeleteVertexArrays
#define glGenVertexArrays		gleswGenVertexArrays
#define glIsVertexArray		gleswIsVertexArray
#define glGetIntegeri_v		gleswGetIntegeri_v
#define glBeginTransformFeedback		gleswBeginTransformFeedback
#define glEndTransformFeedback		gleswEndTransformFeedback
#define glBindBufferRange		gleswBindBufferRange
#define glBindBufferBase		gleswBindBufferBase
#define glTransformFeedbackVaryings		gleswTransformFeedbackVaryings
#define glGetTransformFeedbackVarying		gleswGetTransformFeedbackVarying
#define glVertexAttribIPointer		gleswVertexAttribIPointer
#define glGetVertexAttribIiv		gleswGetVertexAttribIiv
#define glGetVertexAttribIuiv		gleswGetVertexAttribIuiv
#define glVertexAttribI4i		gleswVertexAttribI4i
#define glVertexAttribI4ui		gleswVertexAttribI4ui
#define glVertexAttribI4iv		gleswVertexAttribI4iv
#define glVertexAttribI4uiv		gleswVertexAttribI4uiv
#define glGetUniformuiv		gleswGetUniformuiv
#define glGetFragDataLocation		gleswGetFragDataLocation
#define glUniform1ui		gleswUniform1ui
#define glUniform2ui		gleswUniform2ui
#define glUniform3ui		gleswUniform3ui
#define glUniform4ui		gleswUniform4ui
#define glUniform1uiv		gleswUniform1uiv
#define glUniform2uiv		gleswUniform2uiv
#define glUniform3uiv		gleswUniform3uiv
#define glUniform4uiv		gleswUniform4uiv
#define glClearBufferiv		gleswClearBufferiv
#define glClearBufferuiv		gleswClearBufferuiv
#define glClearBufferfv		gleswClearBufferfv
#define glClearBufferfi		gleswClearBufferfi
#define glGetStringi		gleswGetStringi
#define glCopyBufferSubData		gleswCopyBufferSubData
#define glGetUniformIndices		gleswGetUniformIndices
#define glGetActiveUniformsiv		gleswGetActiveUniformsiv
#define glGetUniformBlockIndex		gleswGetUniformBlockIndex
#define glGetActiveUniformBlockiv		gleswGetActiveUniformBlockiv
#define glGetActiveUniformBlockName		gleswGetActiveUniformBlockName
#define glUniformBlockBinding		gleswUniformBlockBinding
#define glDrawArraysInstanced		gleswDrawArraysInstanced
#define glDrawElementsInstanced		gleswDrawElementsInstanced
#define glFenceSync		gleswFenceSync
#define glIsSync		gleswIsSync
#define glDeleteSync		gleswDeleteSync
#define glClientWaitSync		gleswClientWaitSync
#define glWaitSync		gleswWaitSync
#define glGetInteger64v		gleswGetInteger64v
#define glGetSynciv		gleswGetSynciv
#define glGetInteger64i_v		gleswGetInteger64i_v
#define glGetBufferParameteri64v		gleswGetBufferParameteri64v
#define glGenSamplers		gleswGenSamplers
#define glDeleteSamplers		gleswDeleteSamplers
#define glIsSampler		gleswIsSampler
#define glBindSampler		gleswBindSampler
#define glSamplerParameteri		gleswSamplerParameteri
#define glSamplerParameteriv		gleswSamplerParameteriv
#define glSamplerParameterf		gleswSamplerParameterf
#define glSamplerParameterfv		gleswSamplerParameterfv
#define glGetSamplerParameteriv		gleswGetSamplerParameteriv
#define glGetSamplerParameterfv		gleswGetSamplerParameterfv
#define glVertexAttribDivisor		gleswVertexAttribDivisor
#define glBindTransformFeedback		gleswBindTransformFeedback
#define glDeleteTransformFeedbacks		gleswDeleteTransformFeedbacks
#define glGenTransformFeedbacks		gleswGenTransformFeedbacks
#define glIsTransformFeedback		gleswIsTransformFeedback
#define glPauseTransformFeedback		gleswPauseTransformFeedback
#define glResumeTransformFeedback		gleswResumeTransformFeedback
#define glGetProgramBinary		gleswGetProgramBinary
#define glProgramBinary		gleswProgramBinary
#define glProgramParameteri		gleswProgramParameteri
#define glInvalidateFramebuffer		gleswInvalidateFramebuffer
#define glInvalidateSubFramebuffer		gleswInvalidateSubFramebuffer
#define glTexStorage2D		gleswTexStorage2D
#define glTexStorage3D		gleswTexStorage3D
#define glGetInternalformativ		gleswGetInternalformativ
#define glLabelObjectEXT		gleswLabelObjectEXT
#define glGetObjectLabelEXT		gleswGetObjectLabelEXT
#define glInsertEventMarkerEXT		gleswInsertEventMarkerEXT
#define glPushGroupMarkerEXT		gleswPushGroupMarkerEXT
#define glPopGroupMarkerEXT		gleswPopGroupMarkerEXT
#define glUseProgramStagesEXT		gleswUseProgramStagesEXT
#define glActiveShaderProgramEXT		gleswActiveShaderProgramEXT
#define glCreateShaderProgramvEXT		gleswCreateShaderProgramvEXT
#define glBindProgramPipelineEXT		gleswBindProgramPipelineEXT
#define glDeleteProgramPipelinesEXT		gleswDeleteProgramPipelinesEXT
#define glGenProgramPipelinesEXT		gleswGenProgramPipelinesEXT
#define glIsProgramPipelineEXT		gleswIsProgramPipelineEXT
#define glProgramParameteriEXT		gleswProgramParameteriEXT
#define glGetProgramPipelineivEXT		gleswGetProgramPipelineivEXT
#define glProgramUniform1iEXT		gleswProgramUniform1iEXT
#define glProgramUniform2iEXT		gleswProgramUniform2iEXT
#define glProgramUniform3iEXT		gleswProgramUniform3iEXT
#define glProgramUniform4iEXT		gleswProgramUniform4iEXT
#define glProgramUniform1fEXT		gleswProgramUniform1fEXT
#define glProgramUniform2fEXT		gleswProgramUniform2fEXT
#define glProgramUniform3fEXT		gleswProgramUniform3fEXT
#define glProgramUniform4fEXT		gleswProgramUniform4fEXT
#define glProgramUniform1ivEXT		gleswProgramUniform1ivEXT
#define glProgramUniform2ivEXT		gleswProgramUniform2ivEXT
#define glProgramUniform3ivEXT		gleswProgramUniform3ivEXT
#define glProgramUniform4ivEXT		gleswProgramUniform4ivEXT
#define glProgramUniform1fvEXT		gleswProgramUniform1fvEXT
#define glProgramUniform2fvEXT		gleswProgramUniform2fvEXT
#define glProgramUniform3fvEXT		gleswProgramUniform3fvEXT
#define glProgramUniform4fvEXT		gleswProgramUniform4fvEXT
#define glProgramUniformMatrix2fvEXT		gleswProgramUniformMatrix2fvEXT
#define glProgramUniformMatrix3fvEXT		gleswProgramUniformMatrix3fvEXT
#define glProgramUniformMatrix4fvEXT		gleswProgramUniformMatrix4fvEXT
#define glProgramUniformMatrix2x3fvEXT		gleswProgramUniformMatrix2x3fvEXT
#define glProgramUniformMatrix3x2fvEXT		gleswProgramUniformMatrix3x2fvEXT
#define glProgramUniformMatrix2x4fvEXT		gleswProgramUniformMatrix2x4fvEXT
#define glProgramUniformMatrix4x2fvEXT		gleswProgramUniformMatrix4x2fvEXT
#define glProgramUniformMatrix3x4fvEXT		gleswProgramUniformMatrix3x4fvEXT
#define glProgramUniformMatrix4x3fvEXT		gleswProgramUniformMatrix4x3fvEXT
#define glValidateProgramPipelineEXT		gleswValidateProgramPipelineEXT
#define glGetProgramPipelineInfoLogEXT		gleswGetProgramPipelineInfoLogEXT

#ifdef __cplusplus
}
#endif

#endif
