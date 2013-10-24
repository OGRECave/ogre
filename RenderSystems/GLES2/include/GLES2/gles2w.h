#ifndef __gles2w_h_
#define __gles2w_h_

#if defined(__APPLE__) || defined(__APPLE_CC__)
#   include <OpenGLES/ES2/gl.h>
    // Prevent Apple's non-standard extension header from being included
#   define __gl_es20ext_h_
#else
#   include <GLES2/gl2.h>
#endif

#include <KHR/khrplatform.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>

typedef khronos_int64_t  GLint64EXT;
typedef khronos_uint64_t GLuint64EXT;

#ifndef __gl2_h_
#define __gl2_h_
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* glesw api */
int gleswInit(void);
int gleswIsSupported(int major, int minor);
void *gleswGetProcAddress(const char *proc);

/* OpenGL functions */
typedef void         (GL_APIENTRY* PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void         (GL_APIENTRY* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void         (GL_APIENTRY* PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void         (GL_APIENTRY* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void         (GL_APIENTRY* PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void         (GL_APIENTRY* PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void         (GL_APIENTRY* PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void         (GL_APIENTRY* PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void         (GL_APIENTRY* PFNGLBLENDEQUATIONPROC) ( GLenum mode );
typedef void         (GL_APIENTRY* PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void         (GL_APIENTRY* PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void         (GL_APIENTRY* PFNGLBLENDFUNCSEPARATEPROC) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void         (GL_APIENTRY* PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void         (GL_APIENTRY* PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef GLenum       (GL_APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void         (GL_APIENTRY* PFNGLCLEARPROC) (GLbitfield mask);
typedef void         (GL_APIENTRY* PFNGLCLEARCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void         (GL_APIENTRY* PFNGLCLEARDEPTHFPROC) (GLclampf depth);
typedef void         (GL_APIENTRY* PFNGLCLEARSTENCILPROC) (GLint s);
typedef void         (GL_APIENTRY* PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void         (GL_APIENTRY* PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void         (GL_APIENTRY* PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void         (GL_APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void         (GL_APIENTRY* PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void         (GL_APIENTRY* PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef GLuint       (GL_APIENTRY* PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint       (GL_APIENTRY* PFNGLCREATESHADERPROC) (GLenum type);
typedef void         (GL_APIENTRY* PFNGLCULLFACEPROC) (GLenum mode);
typedef void         (GL_APIENTRY* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void         (GL_APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
typedef void         (GL_APIENTRY* PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void         (GL_APIENTRY* PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void         (GL_APIENTRY* PFNGLDELETESHADERPROC) (GLuint shader);
typedef void         (GL_APIENTRY* PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint* textures);
typedef void         (GL_APIENTRY* PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void         (GL_APIENTRY* PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void         (GL_APIENTRY* PFNGLDEPTHRANGEFPROC) (GLclampf zNear, GLclampf zFar);
typedef void         (GL_APIENTRY* PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void         (GL_APIENTRY* PFNGLDISABLEPROC) (GLenum cap);
typedef void         (GL_APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void         (GL_APIENTRY* PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void         (GL_APIENTRY* PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
typedef void         (GL_APIENTRY* PFNGLENABLEPROC) (GLenum cap);
typedef void         (GL_APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void         (GL_APIENTRY* PFNGLFINISHPROC) (void);
typedef void         (GL_APIENTRY* PFNGLFLUSHPROC) (void);
typedef void         (GL_APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void         (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void         (GL_APIENTRY* PFNGLFRONTFACEPROC) (GLenum mode);
typedef void         (GL_APIENTRY* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void         (GL_APIENTRY* PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void         (GL_APIENTRY* PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
typedef void         (GL_APIENTRY* PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
typedef void         (GL_APIENTRY* PFNGLGENTEXTURESPROC) (GLsizei n, GLuint* textures);
typedef void         (GL_APIENTRY* PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void         (GL_APIENTRY* PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void         (GL_APIENTRY* PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
typedef GLint        (GL_APIENTRY* PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void         (GL_APIENTRY* PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean* params);
typedef void         (GL_APIENTRY* PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLenum       (GL_APIENTRY* PFNGLGETERRORPROC) (void);
typedef void         (GL_APIENTRY* PFNGLGETFLOATVPROC) (GLenum pname, GLfloat* params);
typedef void         (GL_APIENTRY* PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETINTEGERVPROC) (GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void         (GL_APIENTRY* PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
typedef void         (GL_APIENTRY* PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
typedef void         (GL_APIENTRY* PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
typedef const GLubyte* (GL_APIENTRY* PFNGLGETSTRINGPROC) (GLenum name);
typedef void         (GL_APIENTRY* PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat* params);
typedef void         (GL_APIENTRY* PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat* params);
typedef void         (GL_APIENTRY* PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint* params);
typedef GLint        (GL_APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void         (GL_APIENTRY* PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat* params);
typedef void         (GL_APIENTRY* PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint* params);
typedef void         (GL_APIENTRY* PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid** pointer);
typedef void         (GL_APIENTRY* PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef GLboolean    (GL_APIENTRY* PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean    (GL_APIENTRY* PFNGLISENABLEDPROC) (GLenum cap);
typedef GLboolean    (GL_APIENTRY* PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef GLboolean    (GL_APIENTRY* PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean    (GL_APIENTRY* PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef GLboolean    (GL_APIENTRY* PFNGLISSHADERPROC) (GLuint shader);
typedef GLboolean    (GL_APIENTRY* PFNGLISTEXTUREPROC) (GLuint texture);
typedef void         (GL_APIENTRY* PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void         (GL_APIENTRY* PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void         (GL_APIENTRY* PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void         (GL_APIENTRY* PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void         (GL_APIENTRY* PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef void         (GL_APIENTRY* PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void         (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void         (GL_APIENTRY* PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
typedef void         (GL_APIENTRY* PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void         (GL_APIENTRY* PFNGLSHADERBINARYPROC) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
typedef void         (GL_APIENTRY* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void         (GL_APIENTRY* PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void         (GL_APIENTRY* PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void         (GL_APIENTRY* PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void         (GL_APIENTRY* PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void         (GL_APIENTRY* PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void         (GL_APIENTRY* PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef void         (GL_APIENTRY* PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void         (GL_APIENTRY* PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void         (GL_APIENTRY* PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat* params);
typedef void         (GL_APIENTRY* PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void         (GL_APIENTRY* PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint* params);
typedef void         (GL_APIENTRY* PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void         (GL_APIENTRY* PFNGLUNIFORM1FPROC) (GLint location, GLfloat x);
typedef void         (GL_APIENTRY* PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM1IPROC) (GLint location, GLint x);
typedef void         (GL_APIENTRY* PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM2FPROC) (GLint location, GLfloat x, GLfloat y);
typedef void         (GL_APIENTRY* PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM2IPROC) (GLint location, GLint x, GLint y);
typedef void         (GL_APIENTRY* PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM3FPROC) (GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void         (GL_APIENTRY* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM3IPROC) (GLint location, GLint x, GLint y, GLint z);
typedef void         (GL_APIENTRY* PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM4FPROC) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void         (GL_APIENTRY* PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORM4IPROC) (GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void         (GL_APIENTRY* PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* v);
typedef void         (GL_APIENTRY* PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void         (GL_APIENTRY* PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void         (GL_APIENTRY* PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void         (GL_APIENTRY* PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void         (GL_APIENTRY* PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB1FPROC) (GLuint indx, GLfloat x);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB1FVPROC) (GLuint indx, const GLfloat* values);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB2FPROC) (GLuint indx, GLfloat x, GLfloat y);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB2FVPROC) (GLuint indx, const GLfloat* values);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB3FPROC) (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB3FVPROC) (GLuint indx, const GLfloat* values);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB4FPROC) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIB4FVPROC) (GLuint indx, const GLfloat* values);
typedef void         (GL_APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
typedef void         (GL_APIENTRY* PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRY* PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) (GLenum target, GLeglImageOES image);
typedef void (GL_APIENTRY* PFNGLGETPROGRAMBINARYOESPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
typedef void (GL_APIENTRY* PFNGLPROGRAMBINARYOESPROC) (GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length);
typedef void* (GL_APIENTRY* PFNGLMAPBUFFEROESPROC) (GLenum target, GLenum access);
typedef GLboolean (GL_APIENTRY* PFNGLUNMAPBUFFEROESPROC) (GLenum target);
typedef void (GL_APIENTRY* PFNGLGETBUFFERPOINTERVOESPROC) (GLenum target, GLenum pname, GLvoid** params);
typedef void (GL_APIENTRY* PFNGLTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRY* PFNGLTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRY* PFNGLCOPYTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
typedef void (GL_APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE3DOESPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (GL_APIENTRY* PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
typedef void (GL_APIENTRY* PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint *arrays);
typedef void (GL_APIENTRY* PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (GL_APIENTRY* PFNGLISVERTEXARRAYOESPROC) (GLuint array);
typedef void (GL_APIENTRY* PFNGLDEBUGMESSAGECONTROLKHRPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (GL_APIENTRY* PFNGLDEBUGMESSAGEINSERTKHRPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (GL_APIENTRY* PFNGLDEBUGMESSAGECALLBACKKHRPROC) (GLDEBUGPROCKHR callback, const void *userParam);
typedef GLuint (GL_APIENTRY* PFNGLGETDEBUGMESSAGELOGKHRPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
typedef void (GL_APIENTRY* PFNGLPUSHDEBUGGROUPKHRPROC) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
typedef void (GL_APIENTRY* PFNGLPOPDEBUGGROUPKHRPROC) (void);
typedef void (GL_APIENTRY* PFNGLOBJECTLABELKHRPROC) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRY* PFNGLGETOBJECTLABELKHRPROC) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRY* PFNGLOBJECTPTRLABELKHRPROC) (const void *ptr, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRY* PFNGLGETOBJECTPTRLABELKHRPROC) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRY* PFNGLGETPOINTERVKHRPROC) (GLenum pname, void **params);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORGROUPSAMDPROC) (GLint *numGroups, GLsizei groupsSize, GLuint *groups);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORCOUNTERSAMDPROC) (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) (GLuint group, GLuint counter, GLenum pname, GLvoid *data);
typedef void (GL_APIENTRY* PFNGLGENPERFMONITORSAMDPROC) (GLsizei n, GLuint *monitors);
typedef void (GL_APIENTRY* PFNGLDELETEPERFMONITORSAMDPROC) (GLsizei n, GLuint *monitors);
typedef void (GL_APIENTRY* PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *countersList);
typedef void (GL_APIENTRY* PFNGLBEGINPERFMONITORAMDPROC) (GLuint monitor);
typedef void (GL_APIENTRY* PFNGLENDPERFMONITORAMDPROC) (GLuint monitor);
typedef void (GL_APIENTRY* PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten);
typedef void (GL_APIENTRY* PFNGLBLITFRAMEBUFFERANGLEPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLDRAWARRAYSINSTANCEDANGLEPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLDRAWELEMENTSINSTANCEDANGLEPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLVERTEXATTRIBDIVISORANGLEPROC) (GLuint index, GLuint divisor);
typedef void (GL_APIENTRY* PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC) (GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *source);
typedef void (GL_APIENTRY* PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
typedef void (GL_APIENTRY* PFNGLCOPYTEXTURELEVELSAPPLEPROC) (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount);
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
typedef void (GL_APIENTRY* PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC) (void);
typedef GLsync (GL_APIENTRY* PFNGLFENCESYNCAPPLEPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean (GL_APIENTRY* PFNGLISSYNCAPPLEPROC) (GLsync sync);
typedef void (GL_APIENTRY* PFNGLDELETESYNCAPPLEPROC) (GLsync sync);
typedef GLenum (GL_APIENTRY* PFNGLCLIENTWAITSYNCAPPLEPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (GL_APIENTRY* PFNGLWAITSYNCAPPLEPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (GL_APIENTRY* PFNGLGETINTEGER64VAPPLEPROC) (GLenum pname, GLint64 *params);
typedef void (GL_APIENTRY* PFNGLGETSYNCIVAPPLEPROC) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
typedef void (GL_APIENTRY* PFNGLLABELOBJECTEXTPROC) (GLenum type, GLuint object, GLsizei length, const GLchar *label);
typedef void (GL_APIENTRY* PFNGLGETOBJECTLABELEXTPROC) (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (GL_APIENTRY* PFNGLINSERTEVENTMARKEREXTPROC) (GLsizei length, const GLchar *marker);
typedef void (GL_APIENTRY* PFNGLPUSHGROUPMARKEREXTPROC) (GLsizei length, const GLchar *marker);
typedef void (GL_APIENTRY* PFNGLPOPGROUPMARKEREXTPROC) (void);
typedef void (GL_APIENTRY* PFNGLDISCARDFRAMEBUFFEREXTPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
typedef void (GL_APIENTRY* PFNGLGENQUERIESEXTPROC) (GLsizei n, GLuint *ids);
typedef void (GL_APIENTRY* PFNGLDELETEQUERIESEXTPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (GL_APIENTRY* PFNGLISQUERYEXTPROC) (GLuint id);
typedef void (GL_APIENTRY* PFNGLBEGINQUERYEXTPROC) (GLenum target, GLuint id);
typedef void (GL_APIENTRY* PFNGLENDQUERYEXTPROC) (GLenum target);
typedef void (GL_APIENTRY* PFNGLQUERYCOUNTEREXTPROC) (GLuint id, GLenum target);
typedef void (GL_APIENTRY* PFNGLGETQUERYIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GL_APIENTRY* PFNGLGETQUERYOBJECTIVEXTPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (GL_APIENTRY* PFNGLGETQUERYOBJECTUIVEXTPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (GL_APIENTRY* PFNGLGETQUERYOBJECTI64VEXTPROC) (GLuint id, GLenum pname, GLint64EXT *params);
typedef void (GL_APIENTRY* PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname, GLuint64EXT *params);
typedef void* (GL_APIENTRY* PFNGLMAPBUFFERRANGEEXTPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (GL_APIENTRY* PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
typedef void (GL_APIENTRY* PFNGLREADBUFFERINDEXEDEXTPROC) (GLenum src, GLint index);
typedef void (GL_APIENTRY* PFNGLDRAWBUFFERSINDEXEDEXTPROC) (GLint n, const GLenum *location, const GLint *indices);
typedef void (GL_APIENTRY* PFNGLGETINTEGERI_VEXTPROC) (GLenum target, GLuint index, GLint *data);
typedef void (GL_APIENTRY* PFNGLMULTIDRAWARRAYSEXTPROC) (GLenum, const GLint *, const GLsizei *, GLsizei);
typedef void (GL_APIENTRY* PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
typedef GLenum (GL_APIENTRY* PFNGLGETGRAPHICSRESETSTATUSEXTPROC) (void);
typedef void (GL_APIENTRY* PFNGLREADNPIXELSEXTPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
typedef void (GL_APIENTRY* PFNGLGETNUNIFORMFVEXTPROC) (GLuint program, GLint location, GLsizei bufSize, float *params);
typedef void (GL_APIENTRY* PFNGLGETNUNIFORMIVEXTPROC) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
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
typedef void (GL_APIENTRY* PFNGLVALIDATEPROGRAMPIPELINEEXTPROC) (GLuint pipeline);
typedef void (GL_APIENTRY* PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GL_APIENTRY* PFNGLTEXSTORAGE1DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (GL_APIENTRY* PFNGLTEXSTORAGE2DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLTEXSTORAGE3DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (GL_APIENTRY* PFNGLTEXTURESTORAGE1DEXTPROC) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (GL_APIENTRY* PFNGLTEXTURESTORAGE2DEXTPROC) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLTEXTURESTORAGE3DEXTPROC) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);
typedef void (GL_APIENTRY* PFNGLCOVERAGEMASKNVPROC) (GLboolean mask);
typedef void (GL_APIENTRY* PFNGLCOVERAGEOPERATIONNVPROC) (GLenum operation);
typedef void (GL_APIENTRY* PFNGLDRAWBUFFERSNVPROC) (GLsizei n, const GLenum *bufs);
typedef void (GL_APIENTRY* PFNGLDRAWARRAYSINSTANCEDNVPROC) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLDRAWELEMENTSINSTANCEDNVPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
typedef void (GL_APIENTRY* PFNGLDELETEFENCESNVPROC) (GLsizei, const GLuint *);
typedef void (GL_APIENTRY* PFNGLGENFENCESNVPROC) (GLsizei, GLuint *);
typedef GLboolean (GL_APIENTRY* PFNGLISFENCENVPROC) (GLuint);
typedef GLboolean (GL_APIENTRY* PFNGLTESTFENCENVPROC) (GLuint);
typedef void (GL_APIENTRY* PFNGLGETFENCEIVNVPROC) (GLuint, GLenum, GLint *);
typedef void (GL_APIENTRY* PFNGLFINISHFENCENVPROC) (GLuint);
typedef void (GL_APIENTRY* PFNGLSETFENCENVPROC) (GLuint, GLenum);
typedef void (GL_APIENTRY* PFNGLBLITFRAMEBUFFERNVPROC) (int srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC) ( GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLVERTEXATTRIBDIVISORNVPROC) (GLuint index, GLuint divisor);
typedef void (GL_APIENTRY* PFNGLREADBUFFERNVPROC) (GLenum mode);
typedef void (GL_APIENTRY* PFNGLALPHAFUNCQCOMPROC) (GLenum func, GLclampf ref);
typedef void (GL_APIENTRY* PFNGLGETDRIVERCONTROLSQCOMPROC) (GLint *num, GLsizei size, GLuint *driverControls);
typedef void (GL_APIENTRY* PFNGLGETDRIVERCONTROLSTRINGQCOMPROC) (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString);
typedef void (GL_APIENTRY* PFNGLENABLEDRIVERCONTROLQCOMPROC) (GLuint driverControl);
typedef void (GL_APIENTRY* PFNGLDISABLEDRIVERCONTROLQCOMPROC) (GLuint driverControl);
typedef void (GL_APIENTRY* PFNGLEXTGETTEXTURESQCOMPROC) (GLuint *textures, GLint maxTextures, GLint *numTextures);
typedef void (GL_APIENTRY* PFNGLEXTGETBUFFERSQCOMPROC) (GLuint *buffers, GLint maxBuffers, GLint *numBuffers);
typedef void (GL_APIENTRY* PFNGLEXTGETRENDERBUFFERSQCOMPROC) (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers);
typedef void (GL_APIENTRY* PFNGLEXTGETFRAMEBUFFERSQCOMPROC) (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers);
typedef void (GL_APIENTRY* PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC) (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params);
typedef void (GL_APIENTRY* PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC) (GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRY* PFNGLEXTGETTEXSUBIMAGEQCOMPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid *texels);
typedef void (GL_APIENTRY* PFNGLEXTGETBUFFERPOINTERVQCOMPROC) (GLenum target, GLvoid **params);
typedef void (GL_APIENTRY* PFNGLEXTGETSHADERSQCOMPROC) (GLuint *shaders, GLint maxShaders, GLint *numShaders);
typedef void (GL_APIENTRY* PFNGLEXTGETPROGRAMSQCOMPROC) (GLuint *programs, GLint maxPrograms, GLint *numPrograms);
typedef GLboolean (GL_APIENTRY* PFNGLEXTISPROGRAMBINARYQCOMPROC) (GLuint program);
typedef void (GL_APIENTRY* PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC) (GLuint program, GLenum shadertype, GLchar *source, GLint *length);
typedef void (GL_APIENTRY* PFNGLSTARTTILINGQCOMPROC) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
typedef void (GL_APIENTRY* PFNGLENDTILINGQCOMPROC) (GLbitfield preserveMask);

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
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gleswEGLImageTargetTexture2DOES;
extern PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC gleswEGLImageTargetRenderbufferStorageOES;
extern PFNGLGETPROGRAMBINARYOESPROC gleswGetProgramBinaryOES;
extern PFNGLPROGRAMBINARYOESPROC gleswProgramBinaryOES;
extern PFNGLMAPBUFFEROESPROC gleswMapBufferOES;
extern PFNGLUNMAPBUFFEROESPROC gleswUnmapBufferOES;
extern PFNGLGETBUFFERPOINTERVOESPROC gleswGetBufferPointervOES;
extern PFNGLTEXIMAGE3DOESPROC gleswTexImage3DOES;
extern PFNGLTEXSUBIMAGE3DOESPROC gleswTexSubImage3DOES;
extern PFNGLCOPYTEXSUBIMAGE3DOESPROC gleswCopyTexSubImage3DOES;
extern PFNGLCOMPRESSEDTEXIMAGE3DOESPROC gleswCompressedTexImage3DOES;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC gleswCompressedTexSubImage3DOES;
extern PFNGLFRAMEBUFFERTEXTURE3DOESPROC gleswFramebufferTexture3DOES;
extern PFNGLBINDVERTEXARRAYOESPROC gleswBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC gleswDeleteVertexArraysOES;
extern PFNGLGENVERTEXARRAYSOESPROC gleswGenVertexArraysOES;
extern PFNGLISVERTEXARRAYOESPROC gleswIsVertexArrayOES;
extern PFNGLDEBUGMESSAGECONTROLKHRPROC gleswDebugMessageControlKHR;
extern PFNGLDEBUGMESSAGEINSERTKHRPROC gleswDebugMessageInsertKHR;
extern PFNGLDEBUGMESSAGECALLBACKKHRPROC gleswDebugMessageCallbackKHR;
extern PFNGLGETDEBUGMESSAGELOGKHRPROC gleswGetDebugMessageLogKHR;
extern PFNGLPUSHDEBUGGROUPKHRPROC gleswPushDebugGroupKHR;
extern PFNGLPOPDEBUGGROUPKHRPROC gleswPopDebugGroupKHR;
extern PFNGLOBJECTLABELKHRPROC gleswObjectLabelKHR;
extern PFNGLGETOBJECTLABELKHRPROC gleswGetObjectLabelKHR;
extern PFNGLOBJECTPTRLABELKHRPROC gleswObjectPtrLabelKHR;
extern PFNGLGETOBJECTPTRLABELKHRPROC gleswGetObjectPtrLabelKHR;
extern PFNGLGETPOINTERVKHRPROC gleswGetPointervKHR;
extern PFNGLGETPERFMONITORGROUPSAMDPROC gleswGetPerfMonitorGroupsAMD;
extern PFNGLGETPERFMONITORCOUNTERSAMDPROC gleswGetPerfMonitorCountersAMD;
extern PFNGLGETPERFMONITORGROUPSTRINGAMDPROC gleswGetPerfMonitorGroupStringAMD;
extern PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC gleswGetPerfMonitorCounterStringAMD;
extern PFNGLGETPERFMONITORCOUNTERINFOAMDPROC gleswGetPerfMonitorCounterInfoAMD;
extern PFNGLGENPERFMONITORSAMDPROC gleswGenPerfMonitorsAMD;
extern PFNGLDELETEPERFMONITORSAMDPROC gleswDeletePerfMonitorsAMD;
extern PFNGLSELECTPERFMONITORCOUNTERSAMDPROC gleswSelectPerfMonitorCountersAMD;
extern PFNGLBEGINPERFMONITORAMDPROC gleswBeginPerfMonitorAMD;
extern PFNGLENDPERFMONITORAMDPROC gleswEndPerfMonitorAMD;
extern PFNGLGETPERFMONITORCOUNTERDATAAMDPROC gleswGetPerfMonitorCounterDataAMD;
extern PFNGLBLITFRAMEBUFFERANGLEPROC gleswBlitFramebufferANGLE;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC gleswRenderbufferStorageMultisampleANGLE;
extern PFNGLDRAWARRAYSINSTANCEDANGLEPROC gleswDrawArraysInstancedANGLE;
extern PFNGLDRAWELEMENTSINSTANCEDANGLEPROC gleswDrawElementsInstancedANGLE;
extern PFNGLVERTEXATTRIBDIVISORANGLEPROC gleswVertexAttribDivisorANGLE;
extern PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC gleswGetTranslatedShaderSourceANGLE;
extern PFNGLDRAWARRAYSINSTANCEDEXTPROC gleswDrawArraysInstancedEXT;
extern PFNGLDRAWELEMENTSINSTANCEDEXTPROC gleswDrawElementsInstancedEXT;
extern PFNGLVERTEXATTRIBDIVISOREXTPROC gleswVertexAttribDivisorEXT;
extern PFNGLCOPYTEXTURELEVELSAPPLEPROC gleswCopyTextureLevelsAPPLE;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC gleswRenderbufferStorageMultisampleAPPLE;
extern PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC gleswResolveMultisampleFramebufferAPPLE;
extern PFNGLFENCESYNCAPPLEPROC gleswFenceSyncAPPLE;
extern PFNGLISSYNCAPPLEPROC gleswIsSyncAPPLE;
extern PFNGLDELETESYNCAPPLEPROC gleswDeleteSyncAPPLE;
extern PFNGLCLIENTWAITSYNCAPPLEPROC gleswClientWaitSyncAPPLE;
extern PFNGLWAITSYNCAPPLEPROC gleswWaitSyncAPPLE;
extern PFNGLGETINTEGER64VAPPLEPROC gleswGetInteger64vAPPLE;
extern PFNGLGETSYNCIVAPPLEPROC gleswGetSyncivAPPLE;
extern PFNGLLABELOBJECTEXTPROC gleswLabelObjectEXT;
extern PFNGLGETOBJECTLABELEXTPROC gleswGetObjectLabelEXT;
extern PFNGLINSERTEVENTMARKEREXTPROC gleswInsertEventMarkerEXT;
extern PFNGLPUSHGROUPMARKEREXTPROC gleswPushGroupMarkerEXT;
extern PFNGLPOPGROUPMARKEREXTPROC gleswPopGroupMarkerEXT;
extern PFNGLDISCARDFRAMEBUFFEREXTPROC gleswDiscardFramebufferEXT;
extern PFNGLGENQUERIESEXTPROC gleswGenQueriesEXT;
extern PFNGLDELETEQUERIESEXTPROC gleswDeleteQueriesEXT;
extern PFNGLISQUERYEXTPROC gleswIsQueryEXT;
extern PFNGLBEGINQUERYEXTPROC gleswBeginQueryEXT;
extern PFNGLENDQUERYEXTPROC gleswEndQueryEXT;
extern PFNGLQUERYCOUNTEREXTPROC gleswQueryCounterEXT;
extern PFNGLGETQUERYIVEXTPROC gleswGetQueryivEXT;
extern PFNGLGETQUERYOBJECTIVEXTPROC gleswGetQueryObjectivEXT;
extern PFNGLGETQUERYOBJECTUIVEXTPROC gleswGetQueryObjectuivEXT;
extern PFNGLGETQUERYOBJECTI64VEXTPROC gleswGetQueryObjecti64vEXT;
extern PFNGLGETQUERYOBJECTUI64VEXTPROC gleswGetQueryObjectui64vEXT;
extern PFNGLMAPBUFFERRANGEEXTPROC gleswMapBufferRangeEXT;
extern PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC gleswFlushMappedBufferRangeEXT;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC gleswRenderbufferStorageMultisampleEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC gleswFramebufferTexture2DMultisampleEXT;
extern PFNGLREADBUFFERINDEXEDEXTPROC gleswReadBufferIndexedEXT;
extern PFNGLDRAWBUFFERSINDEXEDEXTPROC gleswDrawBuffersIndexedEXT;
extern PFNGLGETINTEGERI_VEXTPROC gleswGetIntegeri_vEXT;
extern PFNGLMULTIDRAWARRAYSEXTPROC gleswMultiDrawArraysEXT;
extern PFNGLMULTIDRAWELEMENTSEXTPROC gleswMultiDrawElementsEXT;
extern PFNGLGETGRAPHICSRESETSTATUSEXTPROC gleswGetGraphicsResetStatusEXT;
extern PFNGLREADNPIXELSEXTPROC gleswReadnPixelsEXT;
extern PFNGLGETNUNIFORMFVEXTPROC gleswGetnUniformfvEXT;
extern PFNGLGETNUNIFORMIVEXTPROC gleswGetnUniformivEXT;
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
extern PFNGLVALIDATEPROGRAMPIPELINEEXTPROC gleswValidateProgramPipelineEXT;
extern PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC gleswGetProgramPipelineInfoLogEXT;
extern PFNGLTEXSTORAGE1DEXTPROC gleswTexStorage1DEXT;
extern PFNGLTEXSTORAGE2DEXTPROC gleswTexStorage2DEXT;
extern PFNGLTEXSTORAGE3DEXTPROC gleswTexStorage3DEXT;
extern PFNGLTEXTURESTORAGE1DEXTPROC gleswTextureStorage1DEXT;
extern PFNGLTEXTURESTORAGE2DEXTPROC gleswTextureStorage2DEXT;
extern PFNGLTEXTURESTORAGE3DEXTPROC gleswTextureStorage3DEXT;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC gleswRenderbufferStorageMultisampleIMG;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC gleswFramebufferTexture2DMultisampleIMG;
extern PFNGLCOVERAGEMASKNVPROC gleswCoverageMaskNV;
extern PFNGLCOVERAGEOPERATIONNVPROC gleswCoverageOperationNV;
extern PFNGLDRAWBUFFERSNVPROC gleswDrawBuffersNV;
extern PFNGLDRAWARRAYSINSTANCEDNVPROC gleswDrawArraysInstancedNV;
extern PFNGLDRAWELEMENTSINSTANCEDNVPROC gleswDrawElementsInstancedNV;
extern PFNGLDELETEFENCESNVPROC gleswDeleteFencesNV;
extern PFNGLGENFENCESNVPROC gleswGenFencesNV;
extern PFNGLISFENCENVPROC gleswIsFenceNV;
extern PFNGLTESTFENCENVPROC gleswTestFenceNV;
extern PFNGLGETFENCEIVNVPROC gleswGetFenceivNV;
extern PFNGLFINISHFENCENVPROC gleswFinishFenceNV;
extern PFNGLSETFENCENVPROC gleswSetFenceNV;
extern PFNGLBLITFRAMEBUFFERNVPROC gleswBlitFramebufferNV;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC gleswRenderbufferStorageMultisampleNV;
extern PFNGLVERTEXATTRIBDIVISORNVPROC gleswVertexAttribDivisorNV;
extern PFNGLREADBUFFERNVPROC gleswReadBufferNV;
extern PFNGLALPHAFUNCQCOMPROC gleswAlphaFuncQCOM;
extern PFNGLGETDRIVERCONTROLSQCOMPROC gleswGetDriverControlsQCOM;
extern PFNGLGETDRIVERCONTROLSTRINGQCOMPROC gleswGetDriverControlStringQCOM;
extern PFNGLENABLEDRIVERCONTROLQCOMPROC gleswEnableDriverControlQCOM;
extern PFNGLDISABLEDRIVERCONTROLQCOMPROC gleswDisableDriverControlQCOM;
extern PFNGLEXTGETTEXTURESQCOMPROC gleswExtGetTexturesQCOM;
extern PFNGLEXTGETBUFFERSQCOMPROC gleswExtGetBuffersQCOM;
extern PFNGLEXTGETRENDERBUFFERSQCOMPROC gleswExtGetRenderbuffersQCOM;
extern PFNGLEXTGETFRAMEBUFFERSQCOMPROC gleswExtGetFramebuffersQCOM;
extern PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC gleswExtGetTexLevelParameterivQCOM;
extern PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC gleswExtTexObjectStateOverrideiQCOM;
extern PFNGLEXTGETTEXSUBIMAGEQCOMPROC gleswExtGetTexSubImageQCOM;
extern PFNGLEXTGETBUFFERPOINTERVQCOMPROC gleswExtGetBufferPointervQCOM;
extern PFNGLEXTGETSHADERSQCOMPROC gleswExtGetShadersQCOM;
extern PFNGLEXTGETPROGRAMSQCOMPROC gleswExtGetProgramsQCOM;
extern PFNGLEXTISPROGRAMBINARYQCOMPROC gleswExtIsProgramBinaryQCOM;
extern PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC gleswExtGetProgramBinarySourceQCOM;
extern PFNGLSTARTTILINGQCOMPROC gleswStartTilingQCOM;
extern PFNGLENDTILINGQCOMPROC gleswEndTilingQCOM;

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
#define glEGLImageTargetTexture2DOES		gleswEGLImageTargetTexture2DOES
#define glEGLImageTargetRenderbufferStorageOES		gleswEGLImageTargetRenderbufferStorageOES
#define glGetProgramBinaryOES		gleswGetProgramBinaryOES
#define glProgramBinaryOES		gleswProgramBinaryOES
#define glMapBufferOES		gleswMapBufferOES
#define glUnmapBufferOES		gleswUnmapBufferOES
#define glGetBufferPointervOES		gleswGetBufferPointervOES
#define glTexImage3DOES		gleswTexImage3DOES
#define glTexSubImage3DOES		gleswTexSubImage3DOES
#define glCopyTexSubImage3DOES		gleswCopyTexSubImage3DOES
#define glCompressedTexImage3DOES		gleswCompressedTexImage3DOES
#define glCompressedTexSubImage3DOES		gleswCompressedTexSubImage3DOES
#define glFramebufferTexture3DOES		gleswFramebufferTexture3DOES
#define glBindVertexArrayOES		gleswBindVertexArrayOES
#define glDeleteVertexArraysOES		gleswDeleteVertexArraysOES
#define glGenVertexArraysOES		gleswGenVertexArraysOES
#define glIsVertexArrayOES		gleswIsVertexArrayOES
#define glDebugMessageControlKHR		gleswDebugMessageControlKHR
#define glDebugMessageInsertKHR		gleswDebugMessageInsertKHR
#define glDebugMessageCallbackKHR		gleswDebugMessageCallbackKHR
#define glGetDebugMessageLogKHR		gleswGetDebugMessageLogKHR
#define glPushDebugGroupKHR		gleswPushDebugGroupKHR
#define glPopDebugGroupKHR		gleswPopDebugGroupKHR
#define glObjectLabelKHR		gleswObjectLabelKHR
#define glGetObjectLabelKHR		gleswGetObjectLabelKHR
#define glObjectPtrLabelKHR		gleswObjectPtrLabelKHR
#define glGetObjectPtrLabelKHR		gleswGetObjectPtrLabelKHR
#define glGetPointervKHR		gleswGetPointervKHR
#define glGetPerfMonitorGroupsAMD		gleswGetPerfMonitorGroupsAMD
#define glGetPerfMonitorCountersAMD		gleswGetPerfMonitorCountersAMD
#define glGetPerfMonitorGroupStringAMD		gleswGetPerfMonitorGroupStringAMD
#define glGetPerfMonitorCounterStringAMD		gleswGetPerfMonitorCounterStringAMD
#define glGetPerfMonitorCounterInfoAMD		gleswGetPerfMonitorCounterInfoAMD
#define glGenPerfMonitorsAMD		gleswGenPerfMonitorsAMD
#define glDeletePerfMonitorsAMD		gleswDeletePerfMonitorsAMD
#define glSelectPerfMonitorCountersAMD		gleswSelectPerfMonitorCountersAMD
#define glBeginPerfMonitorAMD		gleswBeginPerfMonitorAMD
#define glEndPerfMonitorAMD		gleswEndPerfMonitorAMD
#define glGetPerfMonitorCounterDataAMD		gleswGetPerfMonitorCounterDataAMD
#define glBlitFramebufferANGLE		gleswBlitFramebufferANGLE
#define glRenderbufferStorageMultisampleANGLE		gleswRenderbufferStorageMultisampleANGLE
#define glDrawArraysInstancedANGLE		gleswDrawArraysInstancedANGLE
#define glDrawElementsInstancedANGLE		gleswDrawElementsInstancedANGLE
#define glVertexAttribDivisorANGLE		gleswVertexAttribDivisorANGLE
#define glGetTranslatedShaderSourceANGLE		gleswGetTranslatedShaderSourceANGLE
#define glDrawArraysInstancedEXT		gleswDrawArraysInstancedEXT
#define glDrawElementsInstancedEXT		gleswDrawElementsInstancedEXT
#define glVertexAttribDivisorEXT		gleswVertexAttribDivisorEXT
#define glCopyTextureLevelsAPPLE		gleswCopyTextureLevelsAPPLE
#define glRenderbufferStorageMultisampleAPPLE		gleswRenderbufferStorageMultisampleAPPLE
#define glResolveMultisampleFramebufferAPPLE		gleswResolveMultisampleFramebufferAPPLE
#define glFenceSyncAPPLE		gleswFenceSyncAPPLE
#define glIsSyncAPPLE		gleswIsSyncAPPLE
#define glDeleteSyncAPPLE		gleswDeleteSyncAPPLE
#define glClientWaitSyncAPPLE		gleswClientWaitSyncAPPLE
#define glWaitSyncAPPLE		gleswWaitSyncAPPLE
#define glGetInteger64vAPPLE		gleswGetInteger64vAPPLE
#define glGetSyncivAPPLE		gleswGetSyncivAPPLE
#define glLabelObjectEXT		gleswLabelObjectEXT
#define glGetObjectLabelEXT		gleswGetObjectLabelEXT
#define glInsertEventMarkerEXT		gleswInsertEventMarkerEXT
#define glPushGroupMarkerEXT		gleswPushGroupMarkerEXT
#define glPopGroupMarkerEXT		gleswPopGroupMarkerEXT
#define glDiscardFramebufferEXT		gleswDiscardFramebufferEXT
#define glGenQueriesEXT		gleswGenQueriesEXT
#define glDeleteQueriesEXT		gleswDeleteQueriesEXT
#define glIsQueryEXT		gleswIsQueryEXT
#define glBeginQueryEXT		gleswBeginQueryEXT
#define glEndQueryEXT		gleswEndQueryEXT
#define glQueryCounterEXT		gleswQueryCounterEXT
#define glGetQueryivEXT		gleswGetQueryivEXT
#define glGetQueryObjectivEXT		gleswGetQueryObjectivEXT
#define glGetQueryObjectuivEXT		gleswGetQueryObjectuivEXT
#define glGetQueryObjecti64vEXT		gleswGetQueryObjecti64vEXT
#define glGetQueryObjectui64vEXT		gleswGetQueryObjectui64vEXT
#define glMapBufferRangeEXT		gleswMapBufferRangeEXT
#define glFlushMappedBufferRangeEXT		gleswFlushMappedBufferRangeEXT
#define glRenderbufferStorageMultisampleEXT		gleswRenderbufferStorageMultisampleEXT
#define glFramebufferTexture2DMultisampleEXT		gleswFramebufferTexture2DMultisampleEXT
#define glReadBufferIndexedEXT		gleswReadBufferIndexedEXT
#define glDrawBuffersIndexedEXT		gleswDrawBuffersIndexedEXT
#define glGetIntegeri_vEXT		gleswGetIntegeri_vEXT
#define glMultiDrawArraysEXT		gleswMultiDrawArraysEXT
#define glMultiDrawElementsEXT		gleswMultiDrawElementsEXT
#define glGetGraphicsResetStatusEXT		gleswGetGraphicsResetStatusEXT
#define glReadnPixelsEXT		gleswReadnPixelsEXT
#define glGetnUniformfvEXT		gleswGetnUniformfvEXT
#define glGetnUniformivEXT		gleswGetnUniformivEXT
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
#define glValidateProgramPipelineEXT		gleswValidateProgramPipelineEXT
#define glGetProgramPipelineInfoLogEXT		gleswGetProgramPipelineInfoLogEXT
#define glTexStorage1DEXT		gleswTexStorage1DEXT
#define glTexStorage2DEXT		gleswTexStorage2DEXT
#define glTexStorage3DEXT		gleswTexStorage3DEXT
#define glTextureStorage1DEXT		gleswTextureStorage1DEXT
#define glTextureStorage2DEXT		gleswTextureStorage2DEXT
#define glTextureStorage3DEXT		gleswTextureStorage3DEXT
#define glRenderbufferStorageMultisampleIMG		gleswRenderbufferStorageMultisampleIMG
#define glFramebufferTexture2DMultisampleIMG		gleswFramebufferTexture2DMultisampleIMG
#define glCoverageMaskNV		gleswCoverageMaskNV
#define glCoverageOperationNV		gleswCoverageOperationNV
#define glDrawBuffersNV		gleswDrawBuffersNV
#define glDrawArraysInstancedNV		gleswDrawArraysInstancedNV
#define glDrawElementsInstancedNV		gleswDrawElementsInstancedNV
#define glDeleteFencesNV		gleswDeleteFencesNV
#define glGenFencesNV		gleswGenFencesNV
#define glIsFenceNV		gleswIsFenceNV
#define glTestFenceNV		gleswTestFenceNV
#define glGetFenceivNV		gleswGetFenceivNV
#define glFinishFenceNV		gleswFinishFenceNV
#define glSetFenceNV		gleswSetFenceNV
#define glBlitFramebufferNV		gleswBlitFramebufferNV
#define glRenderbufferStorageMultisampleNV		gleswRenderbufferStorageMultisampleNV
#define glVertexAttribDivisorNV		gleswVertexAttribDivisorNV
#define glReadBufferNV		gleswReadBufferNV
#define glAlphaFuncQCOM		gleswAlphaFuncQCOM
#define glGetDriverControlsQCOM		gleswGetDriverControlsQCOM
#define glGetDriverControlStringQCOM		gleswGetDriverControlStringQCOM
#define glEnableDriverControlQCOM		gleswEnableDriverControlQCOM
#define glDisableDriverControlQCOM		gleswDisableDriverControlQCOM
#define glExtGetTexturesQCOM		gleswExtGetTexturesQCOM
#define glExtGetBuffersQCOM		gleswExtGetBuffersQCOM
#define glExtGetRenderbuffersQCOM		gleswExtGetRenderbuffersQCOM
#define glExtGetFramebuffersQCOM		gleswExtGetFramebuffersQCOM
#define glExtGetTexLevelParameterivQCOM		gleswExtGetTexLevelParameterivQCOM
#define glExtTexObjectStateOverrideiQCOM		gleswExtTexObjectStateOverrideiQCOM
#define glExtGetTexSubImageQCOM		gleswExtGetTexSubImageQCOM
#define glExtGetBufferPointervQCOM		gleswExtGetBufferPointervQCOM
#define glExtGetShadersQCOM		gleswExtGetShadersQCOM
#define glExtGetProgramsQCOM		gleswExtGetProgramsQCOM
#define glExtIsProgramBinaryQCOM		gleswExtIsProgramBinaryQCOM
#define glExtGetProgramBinarySourceQCOM		gleswExtGetProgramBinarySourceQCOM
#define glStartTilingQCOM		gleswStartTilingQCOM
#define glEndTilingQCOM		gleswEndTilingQCOM

#ifdef __cplusplus
}
#endif

#endif
