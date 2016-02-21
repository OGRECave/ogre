#include <GLES2/gles2w.h>

#if defined(_WIN32) && !defined(ANDROID)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <EGL/egl.h>

static HMODULE libgl;

static void open_libgl(void)
{
    libgl = LoadLibraryA("libGLESv2.dll");
}

static void close_libgl(void)
{
    FreeLibrary(libgl);
}

static void *get_proc(const char *proc)
{
    void *res;

    res = eglGetProcAddress(proc);
    if (!res)
        res = GetProcAddress(libgl, proc);
    return res;
}
#elif defined(__APPLE__) || defined(__APPLE_CC__)
#import <CoreFoundation/CoreFoundation.h>
#import <UIKit/UIDevice.h>
#import <string>
#import <iostream>
#import <stdio.h>

// Routine to run a system command and retrieve the output.
// From http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c
std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

CFBundleRef bundle;
CFURLRef bundleURL;

static void open_libgl(void)
{
    CFStringRef frameworkPath = CFSTR("/System/Library/Frameworks/OpenGLES.framework");
    NSString *sysVersion = [UIDevice currentDevice].systemVersion;
    NSArray *sysVersionComponents = [sysVersion componentsSeparatedByString:@"."];

    BOOL isSimulator = ([[UIDevice currentDevice].model rangeOfString:@"Simulator"].location != NSNotFound);
    if(isSimulator)
    {
        // Ask where Xcode is installed
        std::string xcodePath = "/Applications/Xcode.app/Contents/Developer\n";

        // The result contains an end line character. Remove it.
        size_t pos = xcodePath.find("\n");
        xcodePath.erase(pos);

        char tempPath[PATH_MAX];
        sprintf(tempPath,
                "%s/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator%s.%s.sdk/System/Library/Frameworks/OpenGLES.framework",
                xcodePath.c_str(),
                [[sysVersionComponents objectAtIndex:0] cStringUsingEncoding:NSUTF8StringEncoding],
                [[sysVersionComponents objectAtIndex:1] cStringUsingEncoding:NSUTF8StringEncoding]);
        frameworkPath = CFStringCreateWithCString(kCFAllocatorDefault, tempPath, kCFStringEncodingUTF8);
    }

    bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                              frameworkPath,
                                              kCFURLPOSIXPathStyle, true);

    CFRelease(frameworkPath);

    bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);

    assert(bundle != NULL);
}

static void close_libgl(void)
{
    CFRelease(bundle);
    CFRelease(bundleURL);
}

static void *get_proc(const char *proc)
{
    void *res;

    CFStringRef procname = CFStringCreateWithCString(kCFAllocatorDefault, proc,
                                                     kCFStringEncodingASCII);
    res = CFBundleGetFunctionPointerForName(bundle, procname);
    CFRelease(procname);
    return res;
}
#elif defined(__EMSCRIPTEN__)
#include <EGL/egl.h>
static void open_libgl() {}
static void close_libgl() {}
static void *get_proc(const char *proc)
{
    return (void*)eglGetProcAddress(proc);
}
#else
#include <dlfcn.h>
#include <EGL/egl.h>

static void *libgl;

static void open_libgl(void)
{
    libgl = dlopen("libGLESv2.so", RTLD_LAZY | RTLD_GLOBAL);
}

static void close_libgl(void)
{
    dlclose(libgl);
}

static void *get_proc(const char *proc)
{
    void *res;
    res = dlsym(libgl, proc);
    return res;
}
#endif

static struct {
    int major, minor;
} version;

static int parse_version(void)
{
    version.major = 2;
    version.minor = 0;

    return 0;
}

static void load_procs(void);

int gleswInit(void)
{
    open_libgl();
    load_procs();
    close_libgl();
    return parse_version();
}

int gleswIsSupported(int major, int minor)
{
    if (major < 2)
        return 0;
    if (version.major == major)
        return version.minor >= minor;
    return version.major >= major;
}

void *gleswGetProcAddress(const char *proc)
{
    return get_proc(proc);
}

PFNGLACTIVETEXTUREPROC gleswActiveTexture;
PFNGLATTACHSHADERPROC gleswAttachShader;
PFNGLBINDATTRIBLOCATIONPROC gleswBindAttribLocation;
PFNGLBINDBUFFERPROC gleswBindBuffer;
PFNGLBINDFRAMEBUFFERPROC gleswBindFramebuffer;
PFNGLBINDRENDERBUFFERPROC gleswBindRenderbuffer;
PFNGLBINDTEXTUREPROC gleswBindTexture;
PFNGLBLENDCOLORPROC gleswBlendColor;
PFNGLBLENDEQUATIONPROC gleswBlendEquation;
PFNGLBLENDEQUATIONSEPARATEPROC gleswBlendEquationSeparate;
PFNGLBLENDFUNCPROC gleswBlendFunc;
PFNGLBLENDFUNCSEPARATEPROC gleswBlendFuncSeparate;
PFNGLBUFFERDATAPROC gleswBufferData;
PFNGLBUFFERSUBDATAPROC gleswBufferSubData;
PFNGLCHECKFRAMEBUFFERSTATUSPROC gleswCheckFramebufferStatus;
PFNGLCLEARPROC gleswClear;
PFNGLCLEARCOLORPROC gleswClearColor;
PFNGLCLEARDEPTHFPROC gleswClearDepthf;
PFNGLCLEARSTENCILPROC gleswClearStencil;
PFNGLCOLORMASKPROC gleswColorMask;
PFNGLCOMPILESHADERPROC gleswCompileShader;
PFNGLCOMPRESSEDTEXIMAGE2DPROC gleswCompressedTexImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC gleswCompressedTexSubImage2D;
PFNGLCOPYTEXIMAGE2DPROC gleswCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE2DPROC gleswCopyTexSubImage2D;
PFNGLCREATEPROGRAMPROC gleswCreateProgram;
PFNGLCREATESHADERPROC gleswCreateShader;
PFNGLCULLFACEPROC gleswCullFace;
PFNGLDELETEBUFFERSPROC gleswDeleteBuffers;
PFNGLDELETEFRAMEBUFFERSPROC gleswDeleteFramebuffers;
PFNGLDELETEPROGRAMPROC gleswDeleteProgram;
PFNGLDELETERENDERBUFFERSPROC gleswDeleteRenderbuffers;
PFNGLDELETESHADERPROC gleswDeleteShader;
PFNGLDELETETEXTURESPROC gleswDeleteTextures;
PFNGLDEPTHFUNCPROC gleswDepthFunc;
PFNGLDEPTHMASKPROC gleswDepthMask;
PFNGLDEPTHRANGEFPROC gleswDepthRangef;
PFNGLDETACHSHADERPROC gleswDetachShader;
PFNGLDISABLEPROC gleswDisable;
PFNGLDISABLEVERTEXATTRIBARRAYPROC gleswDisableVertexAttribArray;
PFNGLDRAWARRAYSPROC gleswDrawArrays;
PFNGLDRAWELEMENTSPROC gleswDrawElements;
PFNGLENABLEPROC gleswEnable;
PFNGLENABLEVERTEXATTRIBARRAYPROC gleswEnableVertexAttribArray;
PFNGLFINISHPROC gleswFinish;
PFNGLFLUSHPROC gleswFlush;
PFNGLFRAMEBUFFERRENDERBUFFERPROC gleswFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC gleswFramebufferTexture2D;
PFNGLFRONTFACEPROC gleswFrontFace;
PFNGLGENBUFFERSPROC gleswGenBuffers;
PFNGLGENERATEMIPMAPPROC gleswGenerateMipmap;
PFNGLGENFRAMEBUFFERSPROC gleswGenFramebuffers;
PFNGLGENRENDERBUFFERSPROC gleswGenRenderbuffers;
PFNGLGENTEXTURESPROC gleswGenTextures;
PFNGLGETACTIVEATTRIBPROC gleswGetActiveAttrib;
PFNGLGETACTIVEUNIFORMPROC gleswGetActiveUniform;
PFNGLGETATTACHEDSHADERSPROC gleswGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC gleswGetAttribLocation;
PFNGLGETBOOLEANVPROC gleswGetBooleanv;
PFNGLGETBUFFERPARAMETERIVPROC gleswGetBufferParameteriv;
PFNGLGETERRORPROC gleswGetError;
PFNGLGETFLOATVPROC gleswGetFloatv;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC gleswGetFramebufferAttachmentParameteriv;
PFNGLGETINTEGERVPROC gleswGetIntegerv;
PFNGLGETPROGRAMIVPROC gleswGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC gleswGetProgramInfoLog;
PFNGLGETRENDERBUFFERPARAMETERIVPROC gleswGetRenderbufferParameteriv;
PFNGLGETSHADERIVPROC gleswGetShaderiv;
PFNGLGETSHADERINFOLOGPROC gleswGetShaderInfoLog;
PFNGLGETSHADERPRECISIONFORMATPROC gleswGetShaderPrecisionFormat;
PFNGLGETSHADERSOURCEPROC gleswGetShaderSource;
PFNGLGETSTRINGPROC gleswGetString;
PFNGLGETTEXPARAMETERFVPROC gleswGetTexParameterfv;
PFNGLGETTEXPARAMETERIVPROC gleswGetTexParameteriv;
PFNGLGETUNIFORMFVPROC gleswGetUniformfv;
PFNGLGETUNIFORMIVPROC gleswGetUniformiv;
PFNGLGETUNIFORMLOCATIONPROC gleswGetUniformLocation;
PFNGLGETVERTEXATTRIBFVPROC gleswGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC gleswGetVertexAttribiv;
PFNGLGETVERTEXATTRIBPOINTERVPROC gleswGetVertexAttribPointerv;
PFNGLHINTPROC gleswHint;
PFNGLISBUFFERPROC gleswIsBuffer;
PFNGLISENABLEDPROC gleswIsEnabled;
PFNGLISFRAMEBUFFERPROC gleswIsFramebuffer;
PFNGLISPROGRAMPROC gleswIsProgram;
PFNGLISRENDERBUFFERPROC gleswIsRenderbuffer;
PFNGLISSHADERPROC gleswIsShader;
PFNGLISTEXTUREPROC gleswIsTexture;
PFNGLLINEWIDTHPROC gleswLineWidth;
PFNGLLINKPROGRAMPROC gleswLinkProgram;
PFNGLPIXELSTOREIPROC gleswPixelStorei;
PFNGLPOLYGONOFFSETPROC gleswPolygonOffset;
PFNGLREADPIXELSPROC gleswReadPixels;
PFNGLRELEASESHADERCOMPILERPROC gleswReleaseShaderCompiler;
PFNGLRENDERBUFFERSTORAGEPROC gleswRenderbufferStorage;
PFNGLSAMPLECOVERAGEPROC gleswSampleCoverage;
PFNGLSCISSORPROC gleswScissor;
PFNGLSHADERBINARYPROC gleswShaderBinary;
PFNGLSHADERSOURCEPROC gleswShaderSource;
PFNGLSTENCILFUNCPROC gleswStencilFunc;
PFNGLSTENCILFUNCSEPARATEPROC gleswStencilFuncSeparate;
PFNGLSTENCILMASKPROC gleswStencilMask;
PFNGLSTENCILMASKSEPARATEPROC gleswStencilMaskSeparate;
PFNGLSTENCILOPPROC gleswStencilOp;
PFNGLSTENCILOPSEPARATEPROC gleswStencilOpSeparate;
PFNGLTEXIMAGE2DPROC gleswTexImage2D;
PFNGLTEXPARAMETERFPROC gleswTexParameterf;
PFNGLTEXPARAMETERFVPROC gleswTexParameterfv;
PFNGLTEXPARAMETERIPROC gleswTexParameteri;
PFNGLTEXPARAMETERIVPROC gleswTexParameteriv;
PFNGLTEXSUBIMAGE2DPROC gleswTexSubImage2D;
PFNGLUNIFORM1FPROC gleswUniform1f;
PFNGLUNIFORM1FVPROC gleswUniform1fv;
PFNGLUNIFORM1IPROC gleswUniform1i;
PFNGLUNIFORM1IVPROC gleswUniform1iv;
PFNGLUNIFORM2FPROC gleswUniform2f;
PFNGLUNIFORM2FVPROC gleswUniform2fv;
PFNGLUNIFORM2IPROC gleswUniform2i;
PFNGLUNIFORM2IVPROC gleswUniform2iv;
PFNGLUNIFORM3FPROC gleswUniform3f;
PFNGLUNIFORM3FVPROC gleswUniform3fv;
PFNGLUNIFORM3IPROC gleswUniform3i;
PFNGLUNIFORM3IVPROC gleswUniform3iv;
PFNGLUNIFORM4FPROC gleswUniform4f;
PFNGLUNIFORM4FVPROC gleswUniform4fv;
PFNGLUNIFORM4IPROC gleswUniform4i;
PFNGLUNIFORM4IVPROC gleswUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC gleswUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC gleswUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC gleswUniformMatrix4fv;
PFNGLUSEPROGRAMPROC gleswUseProgram;
PFNGLVALIDATEPROGRAMPROC gleswValidateProgram;
PFNGLVERTEXATTRIB1FPROC gleswVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC gleswVertexAttrib1fv;
PFNGLVERTEXATTRIB2FPROC gleswVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC gleswVertexAttrib2fv;
PFNGLVERTEXATTRIB3FPROC gleswVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC gleswVertexAttrib3fv;
PFNGLVERTEXATTRIB4FPROC gleswVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC gleswVertexAttrib4fv;
PFNGLVERTEXATTRIBPOINTERPROC gleswVertexAttribPointer;
PFNGLVIEWPORTPROC gleswViewport;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gleswEGLImageTargetTexture2DOES;
PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC gleswEGLImageTargetRenderbufferStorageOES;
PFNGLGETPROGRAMBINARYOESPROC gleswGetProgramBinaryOES;
PFNGLPROGRAMBINARYOESPROC gleswProgramBinaryOES;
PFNGLMAPBUFFEROESPROC gleswMapBufferOES;
PFNGLUNMAPBUFFEROESPROC gleswUnmapBufferOES;
PFNGLGETBUFFERPOINTERVOESPROC gleswGetBufferPointervOES;
PFNGLTEXIMAGE3DOESPROC gleswTexImage3DOES;
PFNGLTEXSUBIMAGE3DOESPROC gleswTexSubImage3DOES;
PFNGLCOPYTEXSUBIMAGE3DOESPROC gleswCopyTexSubImage3DOES;
PFNGLCOMPRESSEDTEXIMAGE3DOESPROC gleswCompressedTexImage3DOES;
PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC gleswCompressedTexSubImage3DOES;
PFNGLFRAMEBUFFERTEXTURE3DOESPROC gleswFramebufferTexture3DOES;
PFNGLBINDVERTEXARRAYOESPROC gleswBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC gleswDeleteVertexArraysOES;
PFNGLGENVERTEXARRAYSOESPROC gleswGenVertexArraysOES;
PFNGLISVERTEXARRAYOESPROC gleswIsVertexArrayOES;
PFNGLDEBUGMESSAGECONTROLKHRPROC gleswDebugMessageControlKHR;
PFNGLDEBUGMESSAGEINSERTKHRPROC gleswDebugMessageInsertKHR;
PFNGLDEBUGMESSAGECALLBACKKHRPROC gleswDebugMessageCallbackKHR;
PFNGLGETDEBUGMESSAGELOGKHRPROC gleswGetDebugMessageLogKHR;
PFNGLPUSHDEBUGGROUPKHRPROC gleswPushDebugGroupKHR;
PFNGLPOPDEBUGGROUPKHRPROC gleswPopDebugGroupKHR;
PFNGLOBJECTLABELKHRPROC gleswObjectLabelKHR;
PFNGLGETOBJECTLABELKHRPROC gleswGetObjectLabelKHR;
PFNGLOBJECTPTRLABELKHRPROC gleswObjectPtrLabelKHR;
PFNGLGETOBJECTPTRLABELKHRPROC gleswGetObjectPtrLabelKHR;
PFNGLGETPOINTERVKHRPROC gleswGetPointervKHR;
PFNGLGETPERFMONITORGROUPSAMDPROC gleswGetPerfMonitorGroupsAMD;
PFNGLGETPERFMONITORCOUNTERSAMDPROC gleswGetPerfMonitorCountersAMD;
PFNGLGETPERFMONITORGROUPSTRINGAMDPROC gleswGetPerfMonitorGroupStringAMD;
PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC gleswGetPerfMonitorCounterStringAMD;
PFNGLGETPERFMONITORCOUNTERINFOAMDPROC gleswGetPerfMonitorCounterInfoAMD;
PFNGLGENPERFMONITORSAMDPROC gleswGenPerfMonitorsAMD;
PFNGLDELETEPERFMONITORSAMDPROC gleswDeletePerfMonitorsAMD;
PFNGLSELECTPERFMONITORCOUNTERSAMDPROC gleswSelectPerfMonitorCountersAMD;
PFNGLBEGINPERFMONITORAMDPROC gleswBeginPerfMonitorAMD;
PFNGLENDPERFMONITORAMDPROC gleswEndPerfMonitorAMD;
PFNGLGETPERFMONITORCOUNTERDATAAMDPROC gleswGetPerfMonitorCounterDataAMD;
PFNGLBLITFRAMEBUFFERANGLEPROC gleswBlitFramebufferANGLE;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC gleswRenderbufferStorageMultisampleANGLE;
PFNGLDRAWARRAYSINSTANCEDANGLEPROC gleswDrawArraysInstancedANGLE;
PFNGLDRAWELEMENTSINSTANCEDANGLEPROC gleswDrawElementsInstancedANGLE;
PFNGLVERTEXATTRIBDIVISORANGLEPROC gleswVertexAttribDivisorANGLE;
PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC gleswGetTranslatedShaderSourceANGLE;
PFNGLDRAWARRAYSINSTANCEDEXTPROC gleswDrawArraysInstancedEXT;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC gleswDrawElementsInstancedEXT;
PFNGLVERTEXATTRIBDIVISOREXTPROC gleswVertexAttribDivisorEXT;
PFNGLCOPYTEXTURELEVELSAPPLEPROC gleswCopyTextureLevelsAPPLE;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC gleswRenderbufferStorageMultisampleAPPLE;
PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC gleswResolveMultisampleFramebufferAPPLE;
PFNGLFENCESYNCAPPLEPROC gleswFenceSyncAPPLE;
PFNGLISSYNCAPPLEPROC gleswIsSyncAPPLE;
PFNGLDELETESYNCAPPLEPROC gleswDeleteSyncAPPLE;
PFNGLCLIENTWAITSYNCAPPLEPROC gleswClientWaitSyncAPPLE;
PFNGLWAITSYNCAPPLEPROC gleswWaitSyncAPPLE;
PFNGLGETINTEGER64VAPPLEPROC gleswGetInteger64vAPPLE;
PFNGLGETSYNCIVAPPLEPROC gleswGetSyncivAPPLE;
PFNGLLABELOBJECTEXTPROC gleswLabelObjectEXT;
PFNGLGETOBJECTLABELEXTPROC gleswGetObjectLabelEXT;
PFNGLINSERTEVENTMARKEREXTPROC gleswInsertEventMarkerEXT;
PFNGLPUSHGROUPMARKEREXTPROC gleswPushGroupMarkerEXT;
PFNGLPOPGROUPMARKEREXTPROC gleswPopGroupMarkerEXT;
PFNGLDISCARDFRAMEBUFFEREXTPROC gleswDiscardFramebufferEXT;
PFNGLGENQUERIESEXTPROC gleswGenQueriesEXT;
PFNGLDELETEQUERIESEXTPROC gleswDeleteQueriesEXT;
PFNGLISQUERYEXTPROC gleswIsQueryEXT;
PFNGLBEGINQUERYEXTPROC gleswBeginQueryEXT;
PFNGLENDQUERYEXTPROC gleswEndQueryEXT;
PFNGLQUERYCOUNTEREXTPROC gleswQueryCounterEXT;
PFNGLGETQUERYIVEXTPROC gleswGetQueryivEXT;
PFNGLGETQUERYOBJECTIVEXTPROC gleswGetQueryObjectivEXT;
PFNGLGETQUERYOBJECTUIVEXTPROC gleswGetQueryObjectuivEXT;
PFNGLGETQUERYOBJECTI64VEXTPROC gleswGetQueryObjecti64vEXT;
PFNGLGETQUERYOBJECTUI64VEXTPROC gleswGetQueryObjectui64vEXT;
PFNGLMAPBUFFERRANGEEXTPROC gleswMapBufferRangeEXT;
PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC gleswFlushMappedBufferRangeEXT;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC gleswRenderbufferStorageMultisampleEXT;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC gleswFramebufferTexture2DMultisampleEXT;
PFNGLREADBUFFERINDEXEDEXTPROC gleswReadBufferIndexedEXT;
PFNGLDRAWBUFFERSINDEXEDEXTPROC gleswDrawBuffersIndexedEXT;
PFNGLGETINTEGERI_VEXTPROC gleswGetIntegeri_vEXT;
PFNGLMULTIDRAWARRAYSEXTPROC gleswMultiDrawArraysEXT;
PFNGLMULTIDRAWELEMENTSEXTPROC gleswMultiDrawElementsEXT;
PFNGLGETGRAPHICSRESETSTATUSEXTPROC gleswGetGraphicsResetStatusEXT;
PFNGLREADNPIXELSEXTPROC gleswReadnPixelsEXT;
PFNGLGETNUNIFORMFVEXTPROC gleswGetnUniformfvEXT;
PFNGLGETNUNIFORMIVEXTPROC gleswGetnUniformivEXT;
PFNGLUSEPROGRAMSTAGESEXTPROC gleswUseProgramStagesEXT;
PFNGLACTIVESHADERPROGRAMEXTPROC gleswActiveShaderProgramEXT;
PFNGLCREATESHADERPROGRAMVEXTPROC gleswCreateShaderProgramvEXT;
PFNGLBINDPROGRAMPIPELINEEXTPROC gleswBindProgramPipelineEXT;
PFNGLDELETEPROGRAMPIPELINESEXTPROC gleswDeleteProgramPipelinesEXT;
PFNGLGENPROGRAMPIPELINESEXTPROC gleswGenProgramPipelinesEXT;
PFNGLISPROGRAMPIPELINEEXTPROC gleswIsProgramPipelineEXT;
PFNGLPROGRAMPARAMETERIEXTPROC gleswProgramParameteriEXT;
PFNGLGETPROGRAMPIPELINEIVEXTPROC gleswGetProgramPipelineivEXT;
PFNGLPROGRAMUNIFORM1IEXTPROC gleswProgramUniform1iEXT;
PFNGLPROGRAMUNIFORM2IEXTPROC gleswProgramUniform2iEXT;
PFNGLPROGRAMUNIFORM3IEXTPROC gleswProgramUniform3iEXT;
PFNGLPROGRAMUNIFORM4IEXTPROC gleswProgramUniform4iEXT;
PFNGLPROGRAMUNIFORM1FEXTPROC gleswProgramUniform1fEXT;
PFNGLPROGRAMUNIFORM2FEXTPROC gleswProgramUniform2fEXT;
PFNGLPROGRAMUNIFORM3FEXTPROC gleswProgramUniform3fEXT;
PFNGLPROGRAMUNIFORM4FEXTPROC gleswProgramUniform4fEXT;
PFNGLPROGRAMUNIFORM1IVEXTPROC gleswProgramUniform1ivEXT;
PFNGLPROGRAMUNIFORM2IVEXTPROC gleswProgramUniform2ivEXT;
PFNGLPROGRAMUNIFORM3IVEXTPROC gleswProgramUniform3ivEXT;
PFNGLPROGRAMUNIFORM4IVEXTPROC gleswProgramUniform4ivEXT;
PFNGLPROGRAMUNIFORM1FVEXTPROC gleswProgramUniform1fvEXT;
PFNGLPROGRAMUNIFORM2FVEXTPROC gleswProgramUniform2fvEXT;
PFNGLPROGRAMUNIFORM3FVEXTPROC gleswProgramUniform3fvEXT;
PFNGLPROGRAMUNIFORM4FVEXTPROC gleswProgramUniform4fvEXT;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC gleswProgramUniformMatrix2fvEXT;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC gleswProgramUniformMatrix3fvEXT;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC gleswProgramUniformMatrix4fvEXT;
PFNGLVALIDATEPROGRAMPIPELINEEXTPROC gleswValidateProgramPipelineEXT;
PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC gleswGetProgramPipelineInfoLogEXT;
PFNGLTEXSTORAGE1DEXTPROC gleswTexStorage1DEXT;
PFNGLTEXSTORAGE2DEXTPROC gleswTexStorage2DEXT;
PFNGLTEXSTORAGE3DEXTPROC gleswTexStorage3DEXT;
PFNGLTEXTURESTORAGE1DEXTPROC gleswTextureStorage1DEXT;
PFNGLTEXTURESTORAGE2DEXTPROC gleswTextureStorage2DEXT;
PFNGLTEXTURESTORAGE3DEXTPROC gleswTextureStorage3DEXT;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC gleswRenderbufferStorageMultisampleIMG;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC gleswFramebufferTexture2DMultisampleIMG;
PFNGLCOVERAGEMASKNVPROC gleswCoverageMaskNV;
PFNGLCOVERAGEOPERATIONNVPROC gleswCoverageOperationNV;
PFNGLDRAWBUFFERSNVPROC gleswDrawBuffersNV;
PFNGLDRAWARRAYSINSTANCEDNVPROC gleswDrawArraysInstancedNV;
PFNGLDRAWELEMENTSINSTANCEDNVPROC gleswDrawElementsInstancedNV;
PFNGLDELETEFENCESNVPROC gleswDeleteFencesNV;
PFNGLGENFENCESNVPROC gleswGenFencesNV;
PFNGLISFENCENVPROC gleswIsFenceNV;
PFNGLTESTFENCENVPROC gleswTestFenceNV;
PFNGLGETFENCEIVNVPROC gleswGetFenceivNV;
PFNGLFINISHFENCENVPROC gleswFinishFenceNV;
PFNGLSETFENCENVPROC gleswSetFenceNV;
PFNGLBLITFRAMEBUFFERNVPROC gleswBlitFramebufferNV;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC gleswRenderbufferStorageMultisampleNV;
PFNGLVERTEXATTRIBDIVISORNVPROC gleswVertexAttribDivisorNV;
PFNGLREADBUFFERNVPROC gleswReadBufferNV;
PFNGLALPHAFUNCQCOMPROC gleswAlphaFuncQCOM;
PFNGLGETDRIVERCONTROLSQCOMPROC gleswGetDriverControlsQCOM;
PFNGLGETDRIVERCONTROLSTRINGQCOMPROC gleswGetDriverControlStringQCOM;
PFNGLENABLEDRIVERCONTROLQCOMPROC gleswEnableDriverControlQCOM;
PFNGLDISABLEDRIVERCONTROLQCOMPROC gleswDisableDriverControlQCOM;
PFNGLEXTGETTEXTURESQCOMPROC gleswExtGetTexturesQCOM;
PFNGLEXTGETBUFFERSQCOMPROC gleswExtGetBuffersQCOM;
PFNGLEXTGETRENDERBUFFERSQCOMPROC gleswExtGetRenderbuffersQCOM;
PFNGLEXTGETFRAMEBUFFERSQCOMPROC gleswExtGetFramebuffersQCOM;
PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC gleswExtGetTexLevelParameterivQCOM;
PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC gleswExtTexObjectStateOverrideiQCOM;
PFNGLEXTGETTEXSUBIMAGEQCOMPROC gleswExtGetTexSubImageQCOM;
PFNGLEXTGETBUFFERPOINTERVQCOMPROC gleswExtGetBufferPointervQCOM;
PFNGLEXTGETSHADERSQCOMPROC gleswExtGetShadersQCOM;
PFNGLEXTGETPROGRAMSQCOMPROC gleswExtGetProgramsQCOM;
PFNGLEXTISPROGRAMBINARYQCOMPROC gleswExtIsProgramBinaryQCOM;
PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC gleswExtGetProgramBinarySourceQCOM;
PFNGLSTARTTILINGQCOMPROC gleswStartTilingQCOM;
PFNGLENDTILINGQCOMPROC gleswEndTilingQCOM;

static void load_procs(void)
{
    gleswActiveTexture = (PFNGLACTIVETEXTUREPROC) get_proc("glActiveTexture");
    gleswAttachShader = (PFNGLATTACHSHADERPROC) get_proc("glAttachShader");
    gleswBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) get_proc("glBindAttribLocation");
    gleswBindBuffer = (PFNGLBINDBUFFERPROC) get_proc("glBindBuffer");
    gleswBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) get_proc("glBindFramebuffer");
    gleswBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) get_proc("glBindRenderbuffer");
    gleswBindTexture = (PFNGLBINDTEXTUREPROC) get_proc("glBindTexture");
    gleswBlendColor = (PFNGLBLENDCOLORPROC) get_proc("glBlendColor");
    gleswBlendEquation = (PFNGLBLENDEQUATIONPROC) get_proc("glBlendEquation");
    gleswBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) get_proc("glBlendEquationSeparate");
    gleswBlendFunc = (PFNGLBLENDFUNCPROC) get_proc("glBlendFunc");
    gleswBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) get_proc("glBlendFuncSeparate");
    gleswBufferData = (PFNGLBUFFERDATAPROC) get_proc("glBufferData");
    gleswBufferSubData = (PFNGLBUFFERSUBDATAPROC) get_proc("glBufferSubData");
    gleswCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) get_proc("glCheckFramebufferStatus");
    gleswClear = (PFNGLCLEARPROC) get_proc("glClear");
    gleswClearColor = (PFNGLCLEARCOLORPROC) get_proc("glClearColor");
    gleswClearDepthf = (PFNGLCLEARDEPTHFPROC) get_proc("glClearDepthf");
    gleswClearStencil = (PFNGLCLEARSTENCILPROC) get_proc("glClearStencil");
    gleswColorMask = (PFNGLCOLORMASKPROC) get_proc("glColorMask");
    gleswCompileShader = (PFNGLCOMPILESHADERPROC) get_proc("glCompileShader");
    gleswCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) get_proc("glCompressedTexImage2D");
    gleswCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) get_proc("glCompressedTexSubImage2D");
    gleswCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) get_proc("glCopyTexImage2D");
    gleswCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) get_proc("glCopyTexSubImage2D");
    gleswCreateProgram = (PFNGLCREATEPROGRAMPROC) get_proc("glCreateProgram");
    gleswCreateShader = (PFNGLCREATESHADERPROC) get_proc("glCreateShader");
    gleswCullFace = (PFNGLCULLFACEPROC) get_proc("glCullFace");
    gleswDeleteBuffers = (PFNGLDELETEBUFFERSPROC) get_proc("glDeleteBuffers");
    gleswDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) get_proc("glDeleteFramebuffers");
    gleswDeleteProgram = (PFNGLDELETEPROGRAMPROC) get_proc("glDeleteProgram");
    gleswDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) get_proc("glDeleteRenderbuffers");
    gleswDeleteShader = (PFNGLDELETESHADERPROC) get_proc("glDeleteShader");
    gleswDeleteTextures = (PFNGLDELETETEXTURESPROC) get_proc("glDeleteTextures");
    gleswDepthFunc = (PFNGLDEPTHFUNCPROC) get_proc("glDepthFunc");
    gleswDepthMask = (PFNGLDEPTHMASKPROC) get_proc("glDepthMask");
    gleswDepthRangef = (PFNGLDEPTHRANGEFPROC) get_proc("glDepthRangef");
    gleswDetachShader = (PFNGLDETACHSHADERPROC) get_proc("glDetachShader");
    gleswDisable = (PFNGLDISABLEPROC) get_proc("glDisable");
    gleswDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) get_proc("glDisableVertexAttribArray");
    gleswDrawArrays = (PFNGLDRAWARRAYSPROC) get_proc("glDrawArrays");
    gleswDrawElements = (PFNGLDRAWELEMENTSPROC) get_proc("glDrawElements");
    gleswEnable = (PFNGLENABLEPROC) get_proc("glEnable");
    gleswEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) get_proc("glEnableVertexAttribArray");
    gleswFinish = (PFNGLFINISHPROC) get_proc("glFinish");
    gleswFlush = (PFNGLFLUSHPROC) get_proc("glFlush");
    gleswFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) get_proc("glFramebufferRenderbuffer");
    gleswFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) get_proc("glFramebufferTexture2D");
    gleswFrontFace = (PFNGLFRONTFACEPROC) get_proc("glFrontFace");
    gleswGenBuffers = (PFNGLGENBUFFERSPROC) get_proc("glGenBuffers");
    gleswGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) get_proc("glGenerateMipmap");
    gleswGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) get_proc("glGenFramebuffers");
    gleswGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) get_proc("glGenRenderbuffers");
    gleswGenTextures = (PFNGLGENTEXTURESPROC) get_proc("glGenTextures");
    gleswGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) get_proc("glGetActiveAttrib");
    gleswGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) get_proc("glGetActiveUniform");
    gleswGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) get_proc("glGetAttachedShaders");
    gleswGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) get_proc("glGetAttribLocation");
    gleswGetBooleanv = (PFNGLGETBOOLEANVPROC) get_proc("glGetBooleanv");
    gleswGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) get_proc("glGetBufferParameteriv");
    gleswGetError = (PFNGLGETERRORPROC) get_proc("glGetError");
    gleswGetFloatv = (PFNGLGETFLOATVPROC) get_proc("glGetFloatv");
    gleswGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) get_proc("glGetFramebufferAttachmentParameteriv");
    gleswGetIntegerv = (PFNGLGETINTEGERVPROC) get_proc("glGetIntegerv");
    gleswGetProgramiv = (PFNGLGETPROGRAMIVPROC) get_proc("glGetProgramiv");
    gleswGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) get_proc("glGetProgramInfoLog");
    gleswGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) get_proc("glGetRenderbufferParameteriv");
    gleswGetShaderiv = (PFNGLGETSHADERIVPROC) get_proc("glGetShaderiv");
    gleswGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) get_proc("glGetShaderInfoLog");
    gleswGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC) get_proc("glGetShaderPrecisionFormat");
    gleswGetShaderSource = (PFNGLGETSHADERSOURCEPROC) get_proc("glGetShaderSource");
    gleswGetString = (PFNGLGETSTRINGPROC) get_proc("glGetString");
    gleswGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) get_proc("glGetTexParameterfv");
    gleswGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) get_proc("glGetTexParameteriv");
    gleswGetUniformfv = (PFNGLGETUNIFORMFVPROC) get_proc("glGetUniformfv");
    gleswGetUniformiv = (PFNGLGETUNIFORMIVPROC) get_proc("glGetUniformiv");
    gleswGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) get_proc("glGetUniformLocation");
    gleswGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) get_proc("glGetVertexAttribfv");
    gleswGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) get_proc("glGetVertexAttribiv");
    gleswGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) get_proc("glGetVertexAttribPointerv");
    gleswHint = (PFNGLHINTPROC) get_proc("glHint");
    gleswIsBuffer = (PFNGLISBUFFERPROC) get_proc("glIsBuffer");
    gleswIsEnabled = (PFNGLISENABLEDPROC) get_proc("glIsEnabled");
    gleswIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) get_proc("glIsFramebuffer");
    gleswIsProgram = (PFNGLISPROGRAMPROC) get_proc("glIsProgram");
    gleswIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) get_proc("glIsRenderbuffer");
    gleswIsShader = (PFNGLISSHADERPROC) get_proc("glIsShader");
    gleswIsTexture = (PFNGLISTEXTUREPROC) get_proc("glIsTexture");
    gleswLineWidth = (PFNGLLINEWIDTHPROC) get_proc("glLineWidth");
    gleswLinkProgram = (PFNGLLINKPROGRAMPROC) get_proc("glLinkProgram");
    gleswPixelStorei = (PFNGLPIXELSTOREIPROC) get_proc("glPixelStorei");
    gleswPolygonOffset = (PFNGLPOLYGONOFFSETPROC) get_proc("glPolygonOffset");
    gleswReadPixels = (PFNGLREADPIXELSPROC) get_proc("glReadPixels");
    gleswReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC) get_proc("glReleaseShaderCompiler");
    gleswRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) get_proc("glRenderbufferStorage");
    gleswSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) get_proc("glSampleCoverage");
    gleswScissor = (PFNGLSCISSORPROC) get_proc("glScissor");
    gleswShaderBinary = (PFNGLSHADERBINARYPROC) get_proc("glShaderBinary");
    gleswShaderSource = (PFNGLSHADERSOURCEPROC) get_proc("glShaderSource");
    gleswStencilFunc = (PFNGLSTENCILFUNCPROC) get_proc("glStencilFunc");
    gleswStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) get_proc("glStencilFuncSeparate");
    gleswStencilMask = (PFNGLSTENCILMASKPROC) get_proc("glStencilMask");
    gleswStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) get_proc("glStencilMaskSeparate");
    gleswStencilOp = (PFNGLSTENCILOPPROC) get_proc("glStencilOp");
    gleswStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) get_proc("glStencilOpSeparate");
    gleswTexImage2D = (PFNGLTEXIMAGE2DPROC) get_proc("glTexImage2D");
    gleswTexParameterf = (PFNGLTEXPARAMETERFPROC) get_proc("glTexParameterf");
    gleswTexParameterfv = (PFNGLTEXPARAMETERFVPROC) get_proc("glTexParameterfv");
    gleswTexParameteri = (PFNGLTEXPARAMETERIPROC) get_proc("glTexParameteri");
    gleswTexParameteriv = (PFNGLTEXPARAMETERIVPROC) get_proc("glTexParameteriv");
    gleswTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) get_proc("glTexSubImage2D");
    gleswUniform1f = (PFNGLUNIFORM1FPROC) get_proc("glUniform1f");
    gleswUniform1fv = (PFNGLUNIFORM1FVPROC) get_proc("glUniform1fv");
    gleswUniform1i = (PFNGLUNIFORM1IPROC) get_proc("glUniform1i");
    gleswUniform1iv = (PFNGLUNIFORM1IVPROC) get_proc("glUniform1iv");
    gleswUniform2f = (PFNGLUNIFORM2FPROC) get_proc("glUniform2f");
    gleswUniform2fv = (PFNGLUNIFORM2FVPROC) get_proc("glUniform2fv");
    gleswUniform2i = (PFNGLUNIFORM2IPROC) get_proc("glUniform2i");
    gleswUniform2iv = (PFNGLUNIFORM2IVPROC) get_proc("glUniform2iv");
    gleswUniform3f = (PFNGLUNIFORM3FPROC) get_proc("glUniform3f");
    gleswUniform3fv = (PFNGLUNIFORM3FVPROC) get_proc("glUniform3fv");
    gleswUniform3i = (PFNGLUNIFORM3IPROC) get_proc("glUniform3i");
    gleswUniform3iv = (PFNGLUNIFORM3IVPROC) get_proc("glUniform3iv");
    gleswUniform4f = (PFNGLUNIFORM4FPROC) get_proc("glUniform4f");
    gleswUniform4fv = (PFNGLUNIFORM4FVPROC) get_proc("glUniform4fv");
    gleswUniform4i = (PFNGLUNIFORM4IPROC) get_proc("glUniform4i");
    gleswUniform4iv = (PFNGLUNIFORM4IVPROC) get_proc("glUniform4iv");
    gleswUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) get_proc("glUniformMatrix2fv");
    gleswUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) get_proc("glUniformMatrix3fv");
    gleswUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) get_proc("glUniformMatrix4fv");
    gleswUseProgram = (PFNGLUSEPROGRAMPROC) get_proc("glUseProgram");
    gleswValidateProgram = (PFNGLVALIDATEPROGRAMPROC) get_proc("glValidateProgram");
    gleswVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) get_proc("glVertexAttrib1f");
    gleswVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC) get_proc("glVertexAttrib1fv");
    gleswVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) get_proc("glVertexAttrib2f");
    gleswVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC) get_proc("glVertexAttrib2fv");
    gleswVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) get_proc("glVertexAttrib3f");
    gleswVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC) get_proc("glVertexAttrib3fv");
    gleswVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) get_proc("glVertexAttrib4f");
    gleswVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC) get_proc("glVertexAttrib4fv");
    gleswVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) get_proc("glVertexAttribPointer");
    gleswViewport = (PFNGLVIEWPORTPROC) get_proc("glViewport");
    gleswEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) get_proc("glEGLImageTargetTexture2DOES");
    gleswEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) get_proc("glEGLImageTargetRenderbufferStorageOES");
    gleswGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) get_proc("glGetProgramBinaryOES");
    gleswProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) get_proc("glProgramBinaryOES");
    gleswMapBufferOES = (PFNGLMAPBUFFEROESPROC) get_proc("glMapBufferOES");
    gleswUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) get_proc("glUnmapBufferOES");
    gleswGetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOESPROC) get_proc("glGetBufferPointervOES");
    gleswTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC) get_proc("glTexImage3DOES");
    gleswTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC) get_proc("glTexSubImage3DOES");
    gleswCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC) get_proc("glCopyTexSubImage3DOES");
    gleswCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) get_proc("glCompressedTexImage3DOES");
    gleswCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) get_proc("glCompressedTexSubImage3DOES");
    gleswFramebufferTexture3DOES = (PFNGLFRAMEBUFFERTEXTURE3DOESPROC) get_proc("glFramebufferTexture3DOES");
    gleswBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) get_proc("glBindVertexArrayOES");
    gleswDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) get_proc("glDeleteVertexArraysOES");
    gleswGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC) get_proc("glGenVertexArraysOES");
    gleswIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC) get_proc("glIsVertexArrayOES");
    gleswDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC) get_proc("glDebugMessageControlKHR");
    gleswDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC) get_proc("glDebugMessageInsertKHR");
    gleswDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC) get_proc("glDebugMessageCallbackKHR");
    gleswGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC) get_proc("glGetDebugMessageLogKHR");
    gleswPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC) get_proc("glPushDebugGroupKHR");
    gleswPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC) get_proc("glPopDebugGroupKHR");
    gleswObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC) get_proc("glObjectLabelKHR");
    gleswGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC) get_proc("glGetObjectLabelKHR");
    gleswObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC) get_proc("glObjectPtrLabelKHR");
    gleswGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC) get_proc("glGetObjectPtrLabelKHR");
    gleswGetPointervKHR = (PFNGLGETPOINTERVKHRPROC) get_proc("glGetPointervKHR");
    gleswGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC) get_proc("glGetPerfMonitorGroupsAMD");
    gleswGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC) get_proc("glGetPerfMonitorCountersAMD");
    gleswGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC) get_proc("glGetPerfMonitorGroupStringAMD");
    gleswGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC) get_proc("glGetPerfMonitorCounterStringAMD");
    gleswGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC) get_proc("glGetPerfMonitorCounterInfoAMD");
    gleswGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC) get_proc("glGenPerfMonitorsAMD");
    gleswDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC) get_proc("glDeletePerfMonitorsAMD");
    gleswSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC) get_proc("glSelectPerfMonitorCountersAMD");
    gleswBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC) get_proc("glBeginPerfMonitorAMD");
    gleswEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC) get_proc("glEndPerfMonitorAMD");
    gleswGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC) get_proc("glGetPerfMonitorCounterDataAMD");
    gleswBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC) get_proc("glBlitFramebufferANGLE");
    gleswRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) get_proc("glRenderbufferStorageMultisampleANGLE");
    gleswDrawArraysInstancedANGLE = (PFNGLDRAWARRAYSINSTANCEDANGLEPROC) get_proc("glDrawArraysInstancedANGLE");
    gleswDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC) get_proc("glDrawElementsInstancedANGLE");
    gleswVertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC) get_proc("glVertexAttribDivisorANGLE");
    gleswGetTranslatedShaderSourceANGLE = (PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC) get_proc("glGetTranslatedShaderSourceANGLE");
    gleswDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC) get_proc("glDrawArraysInstancedEXT");
    gleswDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC) get_proc("glDrawElementsInstancedEXT");
    gleswVertexAttribDivisorEXT = (PFNGLVERTEXATTRIBDIVISOREXTPROC) get_proc("glVertexAttribDivisorEXT");
    gleswCopyTextureLevelsAPPLE = (PFNGLCOPYTEXTURELEVELSAPPLEPROC) get_proc("glCopyTextureLevelsAPPLE");
    gleswRenderbufferStorageMultisampleAPPLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC) get_proc("glRenderbufferStorageMultisampleAPPLE");
    gleswResolveMultisampleFramebufferAPPLE = (PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC) get_proc("glResolveMultisampleFramebufferAPPLE");
    gleswFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC) get_proc("glFenceSyncAPPLE");
    gleswIsSyncAPPLE = (PFNGLISSYNCAPPLEPROC) get_proc("glIsSyncAPPLE");
    gleswDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC) get_proc("glDeleteSyncAPPLE");
    gleswClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC) get_proc("glClientWaitSyncAPPLE");
    gleswWaitSyncAPPLE = (PFNGLWAITSYNCAPPLEPROC) get_proc("glWaitSyncAPPLE");
    gleswGetInteger64vAPPLE = (PFNGLGETINTEGER64VAPPLEPROC) get_proc("glGetInteger64vAPPLE");
    gleswGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC) get_proc("glGetSyncivAPPLE");
    gleswLabelObjectEXT = (PFNGLLABELOBJECTEXTPROC) get_proc("glLabelObjectEXT");
    gleswGetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC) get_proc("glGetObjectLabelEXT");
    gleswInsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC) get_proc("glInsertEventMarkerEXT");
    gleswPushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC) get_proc("glPushGroupMarkerEXT");
    gleswPopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC) get_proc("glPopGroupMarkerEXT");
    gleswDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC) get_proc("glDiscardFramebufferEXT");
    gleswGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) get_proc("glGenQueriesEXT");
    gleswDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) get_proc("glDeleteQueriesEXT");
    gleswIsQueryEXT = (PFNGLISQUERYEXTPROC) get_proc("glIsQueryEXT");
    gleswBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) get_proc("glBeginQueryEXT");
    gleswEndQueryEXT = (PFNGLENDQUERYEXTPROC) get_proc("glEndQueryEXT");
    gleswQueryCounterEXT = (PFNGLQUERYCOUNTEREXTPROC) get_proc("glQueryCounterEXT");
    gleswGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) get_proc("glGetQueryivEXT");
    gleswGetQueryObjectivEXT = (PFNGLGETQUERYOBJECTIVEXTPROC) get_proc("glGetQueryObjectivEXT");
    gleswGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) get_proc("glGetQueryObjectuivEXT");
    gleswGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC) get_proc("glGetQueryObjecti64vEXT");
    gleswGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC) get_proc("glGetQueryObjectui64vEXT");
    gleswMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC) get_proc("glMapBufferRangeEXT");
    gleswFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) get_proc("glFlushMappedBufferRangeEXT");
    gleswRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) get_proc("glRenderbufferStorageMultisampleEXT");
    gleswFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) get_proc("glFramebufferTexture2DMultisampleEXT");
    gleswReadBufferIndexedEXT = (PFNGLREADBUFFERINDEXEDEXTPROC) get_proc("glReadBufferIndexedEXT");
    gleswDrawBuffersIndexedEXT = (PFNGLDRAWBUFFERSINDEXEDEXTPROC) get_proc("glDrawBuffersIndexedEXT");
    gleswGetIntegeri_vEXT = (PFNGLGETINTEGERI_VEXTPROC) get_proc("glGetIntegeri_vEXT");
    gleswMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC) get_proc("glMultiDrawArraysEXT");
    gleswMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC) get_proc("glMultiDrawElementsEXT");
    gleswGetGraphicsResetStatusEXT = (PFNGLGETGRAPHICSRESETSTATUSEXTPROC) get_proc("glGetGraphicsResetStatusEXT");
    gleswReadnPixelsEXT = (PFNGLREADNPIXELSEXTPROC) get_proc("glReadnPixelsEXT");
    gleswGetnUniformfvEXT = (PFNGLGETNUNIFORMFVEXTPROC) get_proc("glGetnUniformfvEXT");
    gleswGetnUniformivEXT = (PFNGLGETNUNIFORMIVEXTPROC) get_proc("glGetnUniformivEXT");
    gleswUseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC) get_proc("glUseProgramStagesEXT");
    gleswActiveShaderProgramEXT = (PFNGLACTIVESHADERPROGRAMEXTPROC) get_proc("glActiveShaderProgramEXT");
    gleswCreateShaderProgramvEXT = (PFNGLCREATESHADERPROGRAMVEXTPROC) get_proc("glCreateShaderProgramvEXT");
    gleswBindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC) get_proc("glBindProgramPipelineEXT");
    gleswDeleteProgramPipelinesEXT = (PFNGLDELETEPROGRAMPIPELINESEXTPROC) get_proc("glDeleteProgramPipelinesEXT");
    gleswGenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC) get_proc("glGenProgramPipelinesEXT");
    gleswIsProgramPipelineEXT = (PFNGLISPROGRAMPIPELINEEXTPROC) get_proc("glIsProgramPipelineEXT");
    gleswProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC) get_proc("glProgramParameteriEXT");
    gleswGetProgramPipelineivEXT = (PFNGLGETPROGRAMPIPELINEIVEXTPROC) get_proc("glGetProgramPipelineivEXT");
    gleswProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC) get_proc("glProgramUniform1iEXT");
    gleswProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC) get_proc("glProgramUniform2iEXT");
    gleswProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC) get_proc("glProgramUniform3iEXT");
    gleswProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC) get_proc("glProgramUniform4iEXT");
    gleswProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC) get_proc("glProgramUniform1fEXT");
    gleswProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC) get_proc("glProgramUniform2fEXT");
    gleswProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC) get_proc("glProgramUniform3fEXT");
    gleswProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC) get_proc("glProgramUniform4fEXT");
    gleswProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC) get_proc("glProgramUniform1ivEXT");
    gleswProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC) get_proc("glProgramUniform2ivEXT");
    gleswProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC) get_proc("glProgramUniform3ivEXT");
    gleswProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC) get_proc("glProgramUniform4ivEXT");
    gleswProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC) get_proc("glProgramUniform1fvEXT");
    gleswProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC) get_proc("glProgramUniform2fvEXT");
    gleswProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC) get_proc("glProgramUniform3fvEXT");
    gleswProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC) get_proc("glProgramUniform4fvEXT");
    gleswProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) get_proc("glProgramUniformMatrix2fvEXT");
    gleswProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) get_proc("glProgramUniformMatrix3fvEXT");
    gleswProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) get_proc("glProgramUniformMatrix4fvEXT");
    gleswValidateProgramPipelineEXT = (PFNGLVALIDATEPROGRAMPIPELINEEXTPROC) get_proc("glValidateProgramPipelineEXT");
    gleswGetProgramPipelineInfoLogEXT = (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) get_proc("glGetProgramPipelineInfoLogEXT");
    gleswTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC) get_proc("glTexStorage1DEXT");
    gleswTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC) get_proc("glTexStorage2DEXT");
    gleswTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC) get_proc("glTexStorage3DEXT");
    gleswTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC) get_proc("glTextureStorage1DEXT");
    gleswTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC) get_proc("glTextureStorage2DEXT");
    gleswTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC) get_proc("glTextureStorage3DEXT");
    gleswRenderbufferStorageMultisampleIMG = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC) get_proc("glRenderbufferStorageMultisampleIMG");
    gleswFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) get_proc("glFramebufferTexture2DMultisampleIMG");
    gleswCoverageMaskNV = (PFNGLCOVERAGEMASKNVPROC) get_proc("glCoverageMaskNV");
    gleswCoverageOperationNV = (PFNGLCOVERAGEOPERATIONNVPROC) get_proc("glCoverageOperationNV");
    gleswDrawBuffersNV = (PFNGLDRAWBUFFERSNVPROC) get_proc("glDrawBuffersNV");
    gleswDrawArraysInstancedNV = (PFNGLDRAWARRAYSINSTANCEDNVPROC) get_proc("glDrawArraysInstancedNV");
    gleswDrawElementsInstancedNV = (PFNGLDRAWELEMENTSINSTANCEDNVPROC) get_proc("glDrawElementsInstancedNV");
    gleswDeleteFencesNV = (PFNGLDELETEFENCESNVPROC) get_proc("glDeleteFencesNV");
    gleswGenFencesNV = (PFNGLGENFENCESNVPROC) get_proc("glGenFencesNV");
    gleswIsFenceNV = (PFNGLISFENCENVPROC) get_proc("glIsFenceNV");
    gleswTestFenceNV = (PFNGLTESTFENCENVPROC) get_proc("glTestFenceNV");
    gleswGetFenceivNV = (PFNGLGETFENCEIVNVPROC) get_proc("glGetFenceivNV");
    gleswFinishFenceNV = (PFNGLFINISHFENCENVPROC) get_proc("glFinishFenceNV");
    gleswSetFenceNV = (PFNGLSETFENCENVPROC) get_proc("glSetFenceNV");
    gleswBlitFramebufferNV = (PFNGLBLITFRAMEBUFFERNVPROC) get_proc("glBlitFramebufferNV");
    gleswRenderbufferStorageMultisampleNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC) get_proc("glRenderbufferStorageMultisampleNV");
    gleswVertexAttribDivisorNV = (PFNGLVERTEXATTRIBDIVISORNVPROC) get_proc("glVertexAttribDivisorNV");
    gleswReadBufferNV = (PFNGLREADBUFFERNVPROC) get_proc("glReadBufferNV");
    gleswAlphaFuncQCOM = (PFNGLALPHAFUNCQCOMPROC) get_proc("glAlphaFuncQCOM");
    gleswGetDriverControlsQCOM = (PFNGLGETDRIVERCONTROLSQCOMPROC) get_proc("glGetDriverControlsQCOM");
    gleswGetDriverControlStringQCOM = (PFNGLGETDRIVERCONTROLSTRINGQCOMPROC) get_proc("glGetDriverControlStringQCOM");
    gleswEnableDriverControlQCOM = (PFNGLENABLEDRIVERCONTROLQCOMPROC) get_proc("glEnableDriverControlQCOM");
    gleswDisableDriverControlQCOM = (PFNGLDISABLEDRIVERCONTROLQCOMPROC) get_proc("glDisableDriverControlQCOM");
    gleswExtGetTexturesQCOM = (PFNGLEXTGETTEXTURESQCOMPROC) get_proc("glExtGetTexturesQCOM");
    gleswExtGetBuffersQCOM = (PFNGLEXTGETBUFFERSQCOMPROC) get_proc("glExtGetBuffersQCOM");
    gleswExtGetRenderbuffersQCOM = (PFNGLEXTGETRENDERBUFFERSQCOMPROC) get_proc("glExtGetRenderbuffersQCOM");
    gleswExtGetFramebuffersQCOM = (PFNGLEXTGETFRAMEBUFFERSQCOMPROC) get_proc("glExtGetFramebuffersQCOM");
    gleswExtGetTexLevelParameterivQCOM = (PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC) get_proc("glExtGetTexLevelParameterivQCOM");
    gleswExtTexObjectStateOverrideiQCOM = (PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC) get_proc("glExtTexObjectStateOverrideiQCOM");
    gleswExtGetTexSubImageQCOM = (PFNGLEXTGETTEXSUBIMAGEQCOMPROC) get_proc("glExtGetTexSubImageQCOM");
    gleswExtGetBufferPointervQCOM = (PFNGLEXTGETBUFFERPOINTERVQCOMPROC) get_proc("glExtGetBufferPointervQCOM");
    gleswExtGetShadersQCOM = (PFNGLEXTGETSHADERSQCOMPROC) get_proc("glExtGetShadersQCOM");
    gleswExtGetProgramsQCOM = (PFNGLEXTGETPROGRAMSQCOMPROC) get_proc("glExtGetProgramsQCOM");
    gleswExtIsProgramBinaryQCOM = (PFNGLEXTISPROGRAMBINARYQCOMPROC) get_proc("glExtIsProgramBinaryQCOM");
    gleswExtGetProgramBinarySourceQCOM = (PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC) get_proc("glExtGetProgramBinarySourceQCOM");
    gleswStartTilingQCOM = (PFNGLSTARTTILINGQCOMPROC) get_proc("glStartTilingQCOM");
    gleswEndTilingQCOM = (PFNGLENDTILINGQCOMPROC) get_proc("glEndTilingQCOM");
}
