 %module(package="Ogre", directors="1") Ogre
 %{
 /* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreArchiveFactory.h"
#include "OgreRectangle2D.h"
#include "OgreWireBoundingBox.h"
#include "OgreVertexBoneAssignment.h"
#include "OgreCodec.h"
#include "OgreZip.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreScriptCompiler.h"
#include "OgreConfigDialog.h"
#include "OgreFileSystemLayer.h"
#include "OgrePredefinedControllers.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreDefaultDebugDrawer.h"
%}

%include stdint.i
%include std_shared_ptr.i
%include std_string.i
%include std_pair.i
%include std_map.i
#ifdef SWIGPYTHON
%include std_multimap.i
%include std_list.i
#endif
%include std_vector.i
%include exception.i
%include typemaps.i
 
// so swig correctly resolves "using std::*" declarations
%inline %{
using namespace std;
%}
 
/* Parse the header file to generate wrappers */
#define _OgreExport
#define _OgrePrivate
#define OGRE_DEPRECATED
#define OGRE_NORETURN
#define OGRE_NODISCARD
#define OGRE_STATIC_MUTEX(arg)
#define OGRE_MUTEX(arg)

%feature("autodoc", "1");
%feature("director") *::Listener;
#ifdef SWIGPYTHON
// should be turned on globally if all renames are in place
%feature("flatnested") Ogre::MaterialManager::Listener;
%feature("flatnested") Ogre::SceneManager::Listener;
#endif

%ignore *::operator=;  // needs rename to wrap
%ignore *::setUserAny; // deprecated
%ignore *::getUserAny; // deprecated
%ignore *::getSingletonPtr; // only expose the non ptr variant
%ignore Ogre::Exception::getNumber; // deprecated
%ignore Ogre::ExceptionFactory::throwExceptionEx; // deprecated
%rename(OgreException) Ogre::Exception; // confilcts with Python Exception

#ifdef __ANDROID__
%{
#include <android/native_window_jni.h>
static JavaVM *cached_jvm = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  cached_jvm = jvm;
  return JNI_VERSION_1_2;
}

JNIEnv* OgreJNIGetEnv() {
  JNIEnv *env;
  jint rc = cached_jvm->GetEnv((void **)&env, JNI_VERSION_1_2);
  if (rc == JNI_EDETACHED)
    throw std::runtime_error("current thread not attached");
  if (rc == JNI_EVERSION)
    throw std::runtime_error("jni version not supported");
  return env;
}
%}
#endif

#ifdef SWIG_DIRECTORS
%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}
#endif

#ifdef SWIGCSHARP
  %apply void *VOID_INT_PTR {void *}

  %typemap(csvarout) void * %{
    get {
	  global::System.IntPtr cPtr = $imcall;
	  if (OgrePINVOKE.SWIGPendingException.Pending) throw OgrePINVOKE.SWIGPendingException.Retrieve();
	  return cPtr;
	}
  %}
#endif

// convert c++ exceptions to language native exceptions
%exception {
    try {
        $action
    }
#ifdef SWIGPYTHON
    catch (Swig::DirectorException &e) { 
        SWIG_fail;
    }
#endif
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// connect operator<< to tp_repr
%ignore ::operator<<;
%feature("python:slot", "tp_repr", functype="reprfunc") *::__repr__;

%ignore *::operator+;
%rename(__add__) *::operator+;

%ignore *::operator-;
%rename(__sub__) *::operator-;

%ignore *::operator*;
%rename(__mul__) *::operator*;

%ignore *::operator/;
%rename(__div__) *::operator/;

#ifdef SWIGJAVA
#define REPRFUNC toString
#elif defined(SWIGCSHARP)
#define REPRFUNC ToString
#else
#define REPRFUNC __repr__
#endif

%define ADD_REPR(classname)
%extend Ogre::classname {
    const std::string REPRFUNC() {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}
%enddef

%define SHARED_PTR(classname)
// %shared_ptr(type);
%template(classname ## Ptr) Ogre::SharedPtr<Ogre::classname >;
%enddef

#ifdef SWIGJAVA
// use proper (1.5+) enums
%include "enums.swg"
// conflicts with SWIG method
%ignore *::getType;
#endif

#ifdef SWIGCSHARP
%ignore Ogre::HardwareBuffer::UsageEnum;
%ignore Ogre::TextureUsage;
%ignore Ogre::GpuConstantType;
%ignore Ogre::GpuProgramParameters::ElementType;
%ignore Ogre::Capabilities;
%typemap(csbase) Ogre::SceneManager::QueryTypeMask "uint";
%csmethodmodifiers *::ToString "public override";
// wrong "override" because of multiple inheritance
%csmethodmodifiers *::getMaterial "public";
%csmethodmodifiers *::getSquaredViewDepth "public";
%csmethodmodifiers *::getWorldTransforms "public";
%csmethodmodifiers *::getRenderOperation "public";
%csmethodmodifiers *::getLights "public";
%csmethodmodifiers *::queryResult "public";
%csmethodmodifiers *::createInstance "public";
%csmethodmodifiers *::loadingComplete "public";
%csmethodmodifiers *::getBoundingBox "public";
%csmethodmodifiers *::getBoundingRadius "public";
%csmethodmodifiers *::getMovableType "public";
%csmethodmodifiers *::visitRenderables "public";
%csmethodmodifiers *::loadResource "public";
%csmethodmodifiers *::createAnimation "public";
%csmethodmodifiers *::getAnimation "public";
%csmethodmodifiers *::hasAnimation "public";
%csmethodmodifiers *::removeAnimation "public";
%csmethodmodifiers *::getNumAnimations "public";
%csmethodmodifiers *::getAnimation "public";
%csmethodmodifiers *::_notifyCurrentCamera "public";
%csmethodmodifiers *::_updateRenderQueue "public";
%csmethodmodifiers *::_notifyAttached "public";
%csmethodmodifiers *::setRenderQueueGroup "public";
%csmethodmodifiers *::setRenderQueueGroupAndPriority "public";
%csmethodmodifiers *::getTypeFlags "public";
%csmethodmodifiers *::viewportDestroyed "public";
%csmethodmodifiers *::viewportDimensionsChanged "public";
%csmethodmodifiers *::viewportCameraChanged "public";
%csmethodmodifiers Ogre::BillboardChain::preRender "public";
#endif

// connect operator[] to __getitem__
%feature("python:slot", "sq_item", functype="ssizeargfunc") *::operator[];
%rename(__getitem__) *::operator[];
%ignore Ogre::Matrix3::operator[];
%ignore Ogre::Matrix4::operator[];
%ignore Ogre::ColourValue::operator[];

// connect __setitem__
%feature("python:slot", "sq_ass_item", functype="ssizeobjargproc") *::__setitem__;

// stringinterface internal
%rename("$ignore", regextarget=1) "^Cmd+";

/* these are ordered by dependancy */
%include "OgreBuildSettings.h"

#ifdef SWIGPYTHON
    #define XSTR(x) #x
    #define STR(x) XSTR(x)
    #define __version__ STR(OGRE_VERSION_MAJOR) "." STR(OGRE_VERSION_MINOR) "." STR(OGRE_VERSION_PATCH)
    #undef STR
    #undef XSTR
#endif

%include "OgrePrerequisites.h"
%include "OgrePlatform.h"
%include "OgreConfig.h"
%ignore Ogre::AllocPolicy;
%import "OgreMemoryAllocatorConfig.h"
%include "OgreCommon.h"
%template(NameValueMap) std::map<std::string, std::string>;
ADD_REPR(TRect)
%template(Rect) Ogre::TRect<long>;
%template(FloatRect) Ogre::TRect<float>;
%ignore Ogre::findCommandLineOpts; // not needed in python

// Basic Data Types
%include "OgreException.h"
%ignore Ogre::SharedPtr::useCount;
%ignore Ogre::SharedPtr::bind;
%ignore Ogre::SharedPtr::getPointer;
%ignore Ogre::SharedPtr::setNull;
%ignore Ogre::SharedPtr::isNull;
%ignore Ogre::SharedPtr::setUseCount;
%include "OgreSharedPtr.h"
%ignore Ogre::Any::getType; // deprecated
%ignore Ogre::Any::destroy; // deprecated
%ignore Ogre::Any::isEmpty; // deprecated
%include "OgreAny.h"
%include "OgreMath.h"
ADD_REPR(Degree)
ADD_REPR(Radian)
%template(RayTestResult) std::pair<bool, float>;
%include "OgreStringVector.h"
%template(StringList) std::vector<Ogre::String>;  // actual vector<T>
%template(StringListPtr) Ogre::SharedPtr<std::vector<Ogre::String> >;
%include "OgreFileSystemLayer.h"
// Linear Algebra
%ignore Ogre::Vector<2, Ogre::Real>::Vector(float, float, float);
%ignore Ogre::Vector<2, Ogre::Real>::Vector(float, float, float, float);
%ignore Ogre::Vector<2, Ogre::Real>::xy;
%ignore Ogre::Vector<2, Ogre::Real>::xyz;
%ignore Ogre::Vector<3, Ogre::Real>::Vector(float, float);
%ignore Ogre::Vector<3, Ogre::Real>::Vector(float, float, float, float);
%ignore Ogre::Vector<3, Ogre::Real>::xyz;
%ignore Ogre::Vector<3, int>::Vector(int, int, int, int);
%ignore Ogre::Vector<3, int>::xyz;
%ignore Ogre::Vector<4, Ogre::Real>::Vector(float, float);
%ignore Ogre::Vector<4, Ogre::Real>::Vector(float, float, float);
%include "OgreVector.h"
ADD_REPR(Vector)

%define TPL_VECTOR(N)
%ignore Ogre::VectorBase<N, Ogre::Real>::VectorBase;
%ignore Ogre::VectorBase<N, Ogre::Real>::ZERO;
%ignore Ogre::VectorBase<N, Ogre::Real>::UNIT_SCALE;
%ignore Ogre::VectorBase<N, Ogre::Real>::UNIT_X;
%ignore Ogre::VectorBase<N, Ogre::Real>::UNIT_Y;
%ignore Ogre::VectorBase<N, Ogre::Real>::UNIT_Z;
%ignore Ogre::VectorBase<N, Ogre::Real>::NEGATIVE_UNIT_X;
%ignore Ogre::VectorBase<N, Ogre::Real>::NEGATIVE_UNIT_Y;
%ignore Ogre::VectorBase<N, Ogre::Real>::NEGATIVE_UNIT_Z;
%template(VectorBase ## N) Ogre::VectorBase<N, Ogre::Real>;
%template(Vector ## N) Ogre::Vector<N, Ogre::Real>;

%extend Ogre::Vector<N, Ogre::Real> {
    void __setitem__(uint i, float v) { (*$self)[i] = v; }
}
%enddef

%ignore Ogre::VectorBase<3, int>::VectorBase;
%template(VectorBase3i) Ogre::VectorBase<3, int>;
%template(Vector3i) Ogre::Vector<3, int>;

TPL_VECTOR(2)
TPL_VECTOR(3)
TPL_VECTOR(4)

#ifdef SWIGCSHARP
%define CS_VECTOR_OPS(N)
%extend Ogre::Vector<N, Ogre::Real> {
    %proxycode %{
    public static Vector ## N operator+(Vector ## N lhs, Vector ## N rhs) { return lhs.__add__(rhs); }
    public static Vector ## N operator-(Vector ## N lhs, Vector ## N rhs) { return lhs.__sub__(rhs); }
    public static Vector ## N operator*(Vector ## N lhs, Vector ## N rhs) { return lhs.__mul__(rhs); }
    public static Vector ## N operator/(Vector ## N lhs, Vector ## N rhs) { return lhs.__div__(rhs); }
    public float this[uint i] { get { return __getitem__(i); } set { __setitem__(i, value); } }
    %}
}
%enddef
CS_VECTOR_OPS(2);
CS_VECTOR_OPS(3);
CS_VECTOR_OPS(4);
#endif

#ifdef SWIGPYTHON
// enable implicit conversion from float to Radian
%typemap(in) const Ogre::Radian& (float tmp) {
    void *argp = 0;
    int res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)(argp);
    }
    else {
        res = SWIG_AsVal_float($input, &tmp);
        $1 = (Ogre::Radian*)&tmp;

        if (!SWIG_IsOK(res))
            SWIG_exception_fail(SWIG_TypeError, "Expected float or Ogre::Radian");
    }
}
// punch through overload resolution
%typecheck(SWIG_TYPECHECK_POINTER) const Ogre::Radian& {
    $1 = true; // actual check in the typemap
}

%typemap(in) void*, Ogre::uchar* {
    void* argp;
    // always allow uchar* as thats how pixel data is usually passed around
    int res = SWIG_ConvertPtr($input, &argp, $descriptor(Ogre::uchar*), $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)(argp);
    } else {
        Py_buffer view;
        res = PyObject_GetBuffer($input, &view, PyBUF_CONTIG);
        if (res < 0) {
            SWIG_fail;
        }
        PyBuffer_Release(&view);
        $1 = ($ltype)view.buf;
    }
}
%typecheck(SWIG_TYPECHECK_POINTER) void*, Ogre::uchar* {
    $1 = true; // actual check in the typemap
}

%define TYPEMAP_SEQUENCE_FOR(TYPE, LEN_CHECK)
%typemap(in) const TYPE& (TYPE temp) {
    void *argp = 0;
    int res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)(argp);
    } else {
        if (!PySequence_Check($input)) {
            SWIG_exception_fail(SWIG_TypeError, "Expected TYPE or sequence") ;
        }
        int len = PySequence_Length($input);
        if (!(LEN_CHECK)) {
            SWIG_exception_fail(SWIG_ValueError, "Size mismatch. Expected LEN_CHECK");
        }
        for (int i = 0; i < len; i++) {
            PyObject *o = PySequence_GetItem($input, i);
            if (!PyNumber_Check(o)) {
                Py_XDECREF(o);
                SWIG_exception_fail(SWIG_TypeError, "Sequence elements must be numbers");
            }
            temp.ptr()[i] = (float)PyFloat_AsDouble(o);
            Py_DECREF(o);
        }
        $1 = &temp;
    }
}
%typecheck(SWIG_TYPECHECK_POINTER) const TYPE& {
    // actual check in the typemap, just skip strings here
    $1 = PyUnicode_Check($input) == 0;
}
%enddef
TYPEMAP_SEQUENCE_FOR(Ogre::Vector2, len == 2)
TYPEMAP_SEQUENCE_FOR(Ogre::Vector3, len == 3)
TYPEMAP_SEQUENCE_FOR(Ogre::Vector4, len == 4)
TYPEMAP_SEQUENCE_FOR(Ogre::ColourValue, len >= 3 && len <= 4)
#endif

%include "OgreMatrix3.h"
ADD_REPR(Matrix3)
%extend Ogre::Matrix3
{
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
}
%ignore Ogre::TransformBaseReal::extract3x3Matrix; // deprecated
%ignore Ogre::TransformBaseReal::extractQuaternion; // deprecated
%ignore Ogre::Matrix4::concatenate; // deprecated
%include "OgreMatrix4.h"
ADD_REPR(Matrix4)
%template(TransformBaseMatrix4) Ogre::TransformBase<4, Ogre::Real>;
%extend Ogre::Matrix4
{
    Ogre::Vector4 operator*(const Ogre::Vector4& v) { return *$self * v; }
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
    Ogre::Matrix4 operator*(const Ogre::Matrix4& m) { return *$self * m; }
    Ogre::Matrix4 operator+(const Ogre::Matrix4& m) { return *$self + m; }
    Ogre::Matrix4 operator-(const Ogre::Matrix4& m) { return *$self - m; }
    Ogre::Real __getitem__(int row, int column) { return (*$self)[row][column]; }
}
ADD_REPR(Affine3)
%extend Ogre::Affine3
{
    Ogre::Vector4 operator*(const Ogre::Vector4& v) { return *$self * v; }
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
    Ogre::Affine3 operator*(const Ogre::Affine3& m) { return *$self * m; }
}
%include "OgreQuaternion.h"
ADD_REPR(Quaternion)
%include "OgreSimpleSpline.h"
%include "OgreRotationalSpline.h"
// Geometric Primitives
%include "OgreAxisAlignedBox.h"
ADD_REPR(AxisAlignedBox)
%include "OgreSphere.h"
%include "OgrePlane.h"
ADD_REPR(Plane)
%include "OgrePlaneBoundedVolume.h"
// I/O
%include "OgreConfigOptionMap.h"
%template(ConfigOptionMap) std::map<std::string, Ogre::ConfigOption>;
%ignore Ogre::ConfigFile::load; // conflicting overloads
%ignore Ogre::ConfigFile::getSettingsIterator; // deprecated
%ignore Ogre::ConfigFile::getSectionIterator;
%template(SettingsBySectionMap) std::map<std::string, std::multimap< std::string, std::string> >;
#ifdef SWIGPYTHON
%template(SettingsMultiMap) std::multimap<std::string, std::string>;
#endif
%include "OgreConfigFile.h"
%ignore Ogre::Log::Stream; // not useful in bindings
%ignore Ogre::Log::stream;
%feature("director") Ogre::LogListener;
%ignore Ogre::Log::setLogDetail;
%include "OgreLog.h"
%ignore Ogre::LogManager::stream; // not useful in bindings
%ignore Ogre::LogManager::setLogDetail;
%include "OgreLogManager.h"
#ifdef SWIGJAVA
// conflicts with SWIG interal func
%ignore Ogre::MemoryDataStream::MemoryDataStream(size_t, bool);
%ignore Ogre::AtomAbstractNode::getValue;
#endif
SHARED_PTR(DataStream);
%ignore Ogre::MemoryDataStream::MemoryDataStream(DataStream&, bool = true, bool = false);
%ignore Ogre::MemoryDataStream::MemoryDataStream(const String&, DataStream&, bool = true, bool = false);
SHARED_PTR(MemoryDataStream);
SHARED_PTR(FileStreamDataStream);
SHARED_PTR(FileHandleDataStream);
%include "OgreDataStream.h"
%include "OgreArchive.h"
%include "OgreFactoryObj.h"
// %template(FactoryObjArchive) Ogre::FactoryObj<Ogre::Archive>;
%include "OgreArchiveFactory.h"
%ignore Ogre::ZipArchiveFactory; // private
%include "OgreZip.h"
%ignore Ogre::ArchiveManager::getArchiveIterator;
%include "OgreArchiveManager.h"
%include "OgreCodec.h"
%include "OgreSerializer.h"
%include "OgreScriptLoader.h"
// Listeners
%feature("director") Ogre::FrameListener;
%include "OgreFrameListener.h"
%feature("director") Ogre::LodListener;
%include "OgreLodListener.h"
%feature("director") Ogre::RenderObjectListener;
%include "OgreRenderObjectListener.h"
%feature("director") Ogre::RenderQueueListener;
%include "OgreRenderQueueListener.h"
%feature("director") Ogre::RenderTargetListener;
%include "OgreRenderTargetListener.h"
%feature("director") Ogre::MeshSerializerListener;
%feature("director") Ogre::ResourceLoadingListener;
// More Data Types
%include "OgreColourValue.h"
ADD_REPR(ColourValue)
%include "OgrePixelFormat.h"
#ifdef SWIGCSHARP
%extend Ogre::PixelBox
{
    void* getData() { return $self->data; }
}
#endif
%include "OgreBlendMode.h"
%include "OgreRay.h"
%include "OgreSceneQuery.h"
%template(RaySceneQueryResult) std::vector<Ogre::RaySceneQueryResultEntry>;
#ifdef SWIGPYTHON
%template(SceneQueryResultMovableList) std::list<Ogre::MovableObject*>;
#endif

%include "OgreNameGenerator.h"
%include "OgreController.h"
%ignore Ogre::RenderSystemCapabilities::setVertexTextureUnitsShared;
%ignore Ogre::RenderSystemCapabilities::getVertexTextureUnitsShared;
%rename("$ignore", regextarget=1) ".*ProgramConstantIntCount.*";
%rename("$ignore", regextarget=1) ".*ProgramConstantBoolCount.*";
%include "OgreRenderSystemCapabilities.h"
%ignore Ogre::GpuProgramParameters::hasPassIterationNumber; // deprecated
%ignore Ogre::GpuProgramParameters::getPassIterationNumberIndex; // deprecated
%ignore Ogre::GpuProgramParameters::setConstantFromTime;
%ignore Ogre::GpuProgramParameters::getConstantDefinitionIterator;
%ignore Ogre::GpuSharedParameters::getConstantDefinitionIterator;
SHARED_PTR(GpuProgramParameters);
%include "OgreGpuProgramParams.h"
%include "OgreImage.h"
%include "OgreBillboard.h"
%include "OgreParticle.h"
%include "OgreHardwareOcclusionQuery.h"
SHARED_PTR(HardwareBuffer);
%include "OgreHardwareBuffer.h"

#ifdef SWIGPYTHON
%ignore std::vector<Ogre::ParameterDef>::resize; // non default constructible
%ignore std::vector<Ogre::ParameterDef>::vector;
%template(ParameterList) std::vector<Ogre::ParameterDef>;
#endif
SHARED_PTR(StringInterface);
%include "OgreStringInterface.h"
    SHARED_PTR(ParticleAffector);
    %include "OgreParticleAffector.h"
        %include "OgreParticleAffectorFactory.h"
//    SHARED_PTR(ParticleEmitter);
    %include "OgreParticleEmitter.h"
        %include "OgreParticleEmitterFactory.h"
    SHARED_PTR(Resource);
    %include "OgreResource.h"
        SHARED_PTR(Texture);
        %ignore Ogre::Texture::setTreatLuminanceAsAlpha;
        %include "OgreTexture.h"
        SHARED_PTR(GpuProgram);
        %ignore Ogre::GpuProgram::setAdjacencyInfoRequired;
        %include "OgreGpuProgram.h"
            SHARED_PTR(HighLevelGpuProgram);
            %include "OgreHighLevelGpuProgram.h"
%ignore Ogre::PreApplyTextureAliasesScriptCompilerEvent;
%include "OgreScriptCompiler.h"
%ignore Ogre::TextureUnitState::setCubicTexture;
%ignore Ogre::TextureUnitState::setCubicTextureName;
%ignore Ogre::TextureUnitState::isCubic;
%ignore Ogre::TextureUnitState::is3D;
%ignore Ogre::TextureUnitState::setBindingType;
%ignore Ogre::TextureUnitState::getBindingType;
%ignore Ogre::TextureUnitState::setIsAlpha;
%ignore Ogre::TextureUnitState::setTextureNameAlias;
%ignore Ogre::TextureUnitState::getTextureNameAlias;
%include "OgreTextureUnitState.h"
%template(ControllerReal) Ogre::Controller<Ogre::Real>;
%template(ControllerValueRealPtr) Ogre::SharedPtr<Ogre::ControllerValue<Ogre::Real> >;
%template(ControllerFunctionPtr) Ogre::SharedPtr<Ogre::ControllerFunction<Ogre::Real> >;
%include "OgreControllerManager.h"
%include "OgrePredefinedControllers.h"
SHARED_PTR(Compositor);
%ignore Ogre::Compositor::getTechniqueIterator;
%ignore Ogre::Compositor::getSupportedTechniqueIterator;
%include "OgreCompositor.h"
%ignore Ogre::CompositionTechnique::getNumTextureDefinitions;
%ignore Ogre::CompositionTechnique::getTextureDefinitionIterator;
%ignore Ogre::CompositionTechnique::getTargetPass;
%ignore Ogre::CompositionTechnique::getNumTargetPasses;
%ignore Ogre::CompositionTechnique::getTargetPassIterator;
%include "OgreCompositionTechnique.h"
%ignore Ogre::CompositionTargetPass::getPass;
%ignore Ogre::CompositionTargetPass::getNumPasses;
%ignore Ogre::CompositionTargetPass::getPassIterator;
%include "OgreCompositionTargetPass.h"
%include "OgreResourceBackgroundQueue.h"
SHARED_PTR(HardwareBuffer);
SHARED_PTR(HardwareVertexBuffer);
#ifdef SWIGPYTHON
%template(VertexElementList) std::list<Ogre::VertexElement>;
#endif
%include "OgreHardwareVertexBuffer.h"
SHARED_PTR(HardwareIndexBuffer);
%include "OgreHardwareIndexBuffer.h"
SHARED_PTR(HardwarePixelBuffer);
#ifdef SWIGJAVA
%ignore Ogre::HardwarePixelBuffer::lock;  // duplicate definition
#endif
%include "OgreHardwarePixelBuffer.h"
%ignore Ogre::HardwareBufferManagerBase::_forceReleaseBufferCopies(HardwareVertexBuffer* sourceBuffer);
%include "OgreHardwareBufferManager.h"
%include "OgreVertexIndexData.h"
// managers
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool, Resource*) const;
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool) const;
%ignore Ogre::ResourceGroupManager::getResourceManagerIterator;
%include "OgreResourceGroupManager.h"
#if SWIG_VERSION < 0x030012 || defined(SWIGJAVA)
// conflicts with overloaded versions (GpuProgram, Texture)
%ignore Ogre::ResourceManager::getResourceByName;
%ignore Ogre::ResourceManager::createOrRetrieve;
%ignore Ogre::ResourceManager::prepare;
%ignore Ogre::ResourceManager::load;
#endif
%ignore Ogre::ResourceManager::getResourceIterator;
%include "OgreResourceManager.h"
    // ambiguity as enums are just ints
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat,int);
    %ignore Ogre::TextureManager::createManual(const String&, const String&,TextureType,uint,uint,int,PixelFormat);
    %include "OgreTextureManager.h"
    %include "OgreGpuProgramManager.h"
    %include "OgreHighLevelGpuProgramManager.h"
SHARED_PTR(UnifiedHighLevelGpuProgram);
%include "OgreUnifiedHighLevelGpuProgram.h"
// animations
%ignore Ogre::VertexPoseKeyFrame::getPoseReferenceIterator;
%include "OgreKeyFrame.h"
%ignore Ogre::Pose::getNormalsIterator;
%ignore Ogre::Pose::getVertexOffsetIterator;
%include "OgrePose.h"
%include "OgreAnimationTrack.h"
%ignore Ogre::AnimationStateSet::getAnimationStateIterator;
%ignore Ogre::AnimationStateSet::getEnabledAnimationStateIterator;
%include "OgreAnimationState.h"

#ifdef SWIGPYTHON
%{
    // this is a workaround for the following map with swig 3.0.12
    namespace swig {
    template<> struct traits<Ogre::AnimationState> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::AnimationState"; }
    };
    template<> struct traits<Ogre::Camera> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::Camera"; }
    };
    }
%}
#endif
%template(AnimationStateMap) std::map<std::string, Ogre::AnimationState*>;
%ignore Ogre::Animation::getVertexTrackIterator;
%ignore Ogre::Animation::getNodeTrackIterator;
%ignore Ogre::Animation::getNumericTrackIterator;
%include "OgreAnimation.h"
SHARED_PTR(Skeleton);
// deprecated
%ignore Ogre::Skeleton::getRootBone;
%ignore Ogre::Skeleton::getRootBoneIterator;
%ignore Ogre::Skeleton::getBoneIterator;
%ignore Ogre::Skeleton::getLinkedSkeletonAnimationSourceIterator;
%template(BoneList) std::vector<Ogre::Bone*>;
%include "OgreSkeleton.h"
    %ignore Ogre::SkeletonInstance::getLinkedSkeletonAnimationSourceIterator;
    SHARED_PTR(SkeletonInstance);
    %include "OgreSkeletonInstance.h"
%include "OgreSkeletonManager.h"
%include "OgreRenderQueue.h"
%include "OgreRenderOperation.h"
%ignore Ogre::Material::getLodValueIterator;
%ignore Ogre::Material::getUserLodValueIterator;
%ignore Ogre::Material::getSupportedTechniqueIterator;
%ignore Ogre::Material::getTechniqueIterator;
%ignore Ogre::Material::getSupportedTechnique;
%ignore Ogre::Material::getNumSupportedTechniques;
%ignore Ogre::Material::clone(const String&, bool) const;
%ignore Ogre::Material::clone(const String&, bool, const String&) const;
SHARED_PTR(Material);
%template(TechniqueList) std::vector<Ogre::Technique*>;
%include "OgreMaterial.h"
%ignore Ogre::RenderSystem::_setTextureUnitFiltering;
%ignore Ogre::RenderSystem::_setTextureAddressingMode;
%ignore Ogre::RenderSystem::_setSceneBlending;
%ignore Ogre::RenderSystem::_setSeparateSceneBlending;
%ignore Ogre::RenderSystem::_setDepthBufferCheckEnabled;
%ignore Ogre::RenderSystem::_setDepthBufferWriteEnabled;
%ignore Ogre::RenderSystem::_setDepthBufferFunction;
%ignore Ogre::RenderSystem::_setColourBufferWriteEnabled;
%ignore Ogre::RenderSystem::_setFog;
%ignore Ogre::RenderSystem::_setWorldMatrix;
%ignore Ogre::RenderSystem::_setViewMatrix;
%ignore Ogre::RenderSystem::_setVertexTexture;
%ignore Ogre::RenderSystem::_setProjectionMatrix;
%ignore Ogre::RenderSystem::getRenderTargetIterator;
%ignore Ogre::RenderSystem::convertColourValue;
%ignore Ogre::RenderSystem::getColourVertexElementType;
%ignore Ogre::RenderSystem::setStencilCheckEnabled;
%ignore Ogre::RenderSystem::setStencilBufferParams;
%ignore Ogre::RenderSystem::getDisplayMonitorCount;
%include "OgreRenderSystem.h"
%include "OgreCompositorManager.h"
#ifdef SWIGJAVA
%ignore Ogre::CompositorInstance::Listener; // issue with converting shared_ptr<Material>
#endif
%include "OgreCompositorInstance.h"
%include "OgreCompositionPass.h"
%include "OgreMaterialSerializer.h"
%include "OgreUserObjectBindings.h"
%rename(MaterialManager_Listener) Ogre::MaterialManager::Listener;
%include "OgreMaterialManager.h"
%include "OgreRenderable.h"
%ignore Ogre::ShadowCaster::getShadowVolumeRenderableIterator;
%include "OgreShadowCaster.h"
%extend Ogre::MovableObject {
  Entity* castEntity()
  {
    return dynamic_cast<Ogre::Entity*>($self);
  }
}
%extend Ogre::Node {
  SceneNode* castSceneNode()
  {
    return dynamic_cast<Ogre::SceneNode*>($self);
  }
}
%include "OgreMovableObject.h"
    %include "OgreBillboardChain.h"
        %ignore Ogre::RibbonTrail::getNodeIterator;
        %include "OgreRibbonTrail.h"
    %ignore Ogre::BillboardSet::setTextureCoords(const FloatRect*, uint16);
    %ignore Ogre::BillboardSet::getTextureCoords(uint16*);
    %include "OgreBillboardSet.h"
    %include "OgreMovablePlane.h"
    %ignore Ogre::Light::setPosition;
    %ignore Ogre::Light::getPosition;
    %ignore Ogre::Light::getDirection;
    %include "OgreLight.h"
    %ignore Ogre::Node::getChildIterator;
    %template(NodeList) std::vector<Ogre::Node*>;
    %include "OgreNode.h"
        %include "OgreBone.h"
        %ignore Ogre::SceneNode::getAttachedObjectIterator;
        %template(MovableObjectList) std::vector<Ogre::MovableObject*>;
        %include "OgreSceneNode.h"
    SHARED_PTR(ShadowCameraSetup);
    SHARED_PTR(DefaultShadowCameraSetup);
    %include "OgreShadowCameraSetup.h"
        SHARED_PTR(FocusedShadowCameraSetup);
        %include "OgreShadowCameraSetupFocused.h"
        SHARED_PTR(LiSPSMShadowCameraSetup);
        %include "OgreShadowCameraSetupLiSPSM.h"
        SHARED_PTR(PlaneOptimalShadowCameraSetup);
        %include "OgreShadowCameraSetupPlaneOptimal.h"
        SHARED_PTR(PSSMShadowCameraSetup);
        %include "OgreShadowCameraSetupPSSM.h"
            %template(SplitPointList) std::vector<Ogre::Real>;
    %ignore Ogre::Frustum::getFrustumExtents(Real&, Real& ,Real& ,Real&) const;
    %ignore Ogre::Frustum::getProjectionMatrixRS;
    %include "OgreFrustum.h"
        %ignore Ogre::Camera::setPosition;
        %ignore Ogre::Camera::getPosition;
        %ignore Ogre::Camera::setDirection;
        %ignore Ogre::Camera::getDirection;
        %ignore Ogre::Camera::setOrientation;
        %ignore Ogre::Camera::getOrientation;
        %ignore Ogre::Camera::rotate;
        %ignore Ogre::Camera::getUp;
        %ignore Ogre::Camera::getRight;
        %ignore Ogre::Camera::lookAt;
        %ignore Ogre::Camera::roll;
        %ignore Ogre::Camera::yaw;
        %ignore Ogre::Camera::pitch;
        %ignore Ogre::Camera::setFixedYawAxis;
        %ignore Ogre::Camera::setAutoTracking;
        %ignore Ogre::Camera::move;
        %ignore Ogre::Camera::moveRelative;
        %ignore Ogre::Camera::_renderScene(Viewport*, bool);
        %include "OgreCamera.h"
        ADD_REPR(Camera)
    %include "OgreManualObject.h"
    %template(SubEntityList) std::vector<Ogre::SubEntity*>;
    %ignore Ogre::Entity::getAttachedObjectIterator;
    %include "OgreEntity.h"
    %include "OgreSubEntity.h"
    %include "OgreParticleSystemRenderer.h"
    SHARED_PTR(ParticleSystem);
    %ignore Ogre::ParticleSystem::_getIterator;
    %include "OgreParticleSystem.h"
    %include "OgreInstancedEntity.h"
    %include "OgreInstanceBatch.h"
    %ignore Ogre::SimpleRenderable::setMaterial(const String&);
    %include "OgreSimpleRenderable.h"
        %include "OgreRectangle2D.h"
        %include "OgreWireBoundingBox.h"
%ignore Ogre::ParticleSystemManager::getTemplateIterator;
%ignore Ogre::ParticleSystemManager::getEmitterFactoryIterator;
%ignore Ogre::ParticleSystemManager::getAffectorFactoryIterator;
%ignore Ogre::ParticleSystemManager::getRendererFactoryIterator;
%include "OgreParticleSystemManager.h"
%ignore Ogre::InstanceManager::getInstanceBatchIterator;
%ignore Ogre::InstanceManager::getInstanceBatchMapIterator;
%include "OgreInstanceManager.h" 
%include "OgreVertexBoneAssignment.h"
// deprecated
%ignore Ogre::Mesh::getSubMeshIterator;
%ignore Ogre::Mesh::getPoseCount;
%ignore Ogre::Mesh::getPose;
%ignore Ogre::Mesh::getPoseIterator;
%ignore Ogre::Mesh::getBoneAssignmentIterator;
%template(PoseList) std::vector<Ogre::Pose*>;
%template(SubMeshList) std::vector<Ogre::SubMesh*>;
%apply unsigned short& OUTPUT { unsigned short& outSourceCoordSet, unsigned short& outIndex };
SHARED_PTR(Mesh);
%include "OgreMesh.h"
%ignore Ogre::SubMesh::getBoneAssignmentIterator;
%ignore Ogre::SubMesh::getAliasTextureIterator;
%ignore Ogre::SubMesh::removeAllTextureAliases;
%include "OgreSubMesh.h"
%ignore Ogre::StaticGeometry::getRegionIterator;
%ignore Ogre::StaticGeometry::Region::getLODIterator;
%ignore Ogre::StaticGeometry::MaterialBucket::getGeometryIterator;
%ignore Ogre::StaticGeometry::LODBucket::getMaterialIterator;
%include "OgreStaticGeometry.h"
%include "OgrePatchSurface.h"
    SHARED_PTR(PatchMesh);
    %include "OgrePatchMesh.h"
%include "OgreMeshSerializer.h"
%include "OgreMeshManager.h"
%include "OgreLodStrategy.h"
%include "OgrePixelCountLodStrategy.h"
%ignore Ogre::Pass::getTextureUnitStateIterator; // deprecated
%template(TextureUnitStateList) std::vector<Ogre::TextureUnitState*>;
%include "OgrePass.h"
    %ignore Ogre::Technique::getGPUVendorRuleIterator;
    %ignore Ogre::Technique::getGPUDeviceNameRuleIterator;
    %ignore Ogre::Technique::getIlluminationPassIterator;
    %ignore Ogre::Technique::getPassIterator();
    %template(PassList) std::vector<Ogre::Pass*>;
    %template(IlluminationPassList) std::vector<Ogre::IlluminationPass*>;
    %include "OgreTechnique.h"
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&);
%ignore Ogre::RenderTarget::copyContentsToMemory(const PixelBox&, FrameBuffer); // deprecated
%feature("flatnested") Ogre::RenderTarget::FrameStats;
%include "OgreRenderTarget.h"
#ifdef __ANDROID__
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*);
    %ignore Ogre::RenderWindow::_notifySurfaceCreated(void*, void*);
    %extend Ogre::RenderWindow {
        void _notifySurfaceCreated(jobject surface) {
            ANativeWindow* nativeWnd = ANativeWindow_fromSurface(OgreJNIGetEnv(), surface);
            $self->_notifySurfaceCreated(nativeWnd, NULL);
        }
    }
#endif
    %include "OgreRenderWindow.h"
    %include "OgreRenderTexture.h"
%ignore Ogre::Viewport::getActualDimensions(int&, int& ,int& ,int&) const;
%include "OgreViewport.h"
%ignore Ogre::CompositorChain::getNumCompositors;
%ignore Ogre::CompositorChain::getCompositor;
%ignore Ogre::CompositorChain::getCompositors;
%include "OgreCompositorChain.h"
%ignore Ogre::RenderQueueGroup::getIterator;
%include "OgreRenderQueueSortingGrouping.h"
%ignore Ogre::SceneManager::getCameraIterator; // deprecated
%ignore Ogre::SceneManager::getAnimationIterator;
%ignore Ogre::SceneManager::getAnimationStateIterator;
%ignore Ogre::SceneManager::getMovableObjectIterator;
%ignore Ogre::SceneManager::getShadowTextureCount;
%ignore Ogre::SceneManager::getShadowTextureConfigIterator;
%newobject Ogre::SceneManager::createRayQuery(const Ray&, uint32 mask);
%newobject Ogre::SceneManager::createRayQuery(const Ray&);
%rename(SceneManager_Listener) Ogre::SceneManager::Listener;
%template(MovableObjectMap) std::map<std::string, Ogre::MovableObject*>;
%template(CameraMap) std::map<std::string, Ogre::Camera*>;
%include "OgreSceneManager.h"
%ignore Ogre::SceneManagerEnumerator::createSceneManager(uint16);
%ignore Ogre::SceneManagerEnumerator::createSceneManager(uint16, const String&);
%ignore Ogre::SceneManagerEnumerator::getSceneManagerIterator;
%ignore Ogre::SceneManagerEnumerator::getMetaDataIterator;
%include "OgreDefaultDebugDrawer.h"
%include "OgreSceneManagerEnumerator.h"
%include "OgreConfigDialog.h"
%template(RenderSystemList) std::vector<Ogre::RenderSystem*>;
%ignore Ogre::Root::getSceneManagerMetaDataIterator;
%ignore Ogre::Root::getSceneManagerIterator;
%ignore Ogre::Root::createSceneManager(uint16);
%ignore Ogre::Root::createSceneManager(uint16, const String&);
%ignore Ogre::Root::getMovableObjectFactoryIterator;
%ignore Ogre::Root::convertColourValue;
%ignore Ogre::Root::getDisplayMonitorCount;
%include "OgreRoot.h"
// dont wrap: platform specific
// %include "OgreWindowEventUtilities.h"
// %include "OgreTimer.h"
// dont wrap: not useful in high level languages
// %include "OgreRadixSort.h"
// %include "OgreString.h"
// %include "OgreStringConverter.h"
// %include "OgreProfiler.h"
