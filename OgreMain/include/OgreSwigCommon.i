// declarations shared by all parts of the Ogre SWIG module: wrapper includes,
// std typemaps, features, renames and helper macros
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
#include "OgreScriptCompiler.h"
#include "OgreConfigDialog.h"
#include "OgreFileSystemLayer.h"
#include "OgrePredefinedControllers.h"
#include "OgrePixelCountLodStrategy.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreLodStrategyManager.h"
#include "OgreDefaultDebugDrawer.h"
#include "OgreCompositorLogic.h"

using namespace Ogre;
%}

%include stdint.i
%include std_shared_ptr.i
%include std_string.i
%include std_pair.i
%include std_map.i
%include std_unordered_map.i
%include std_list.i
%include std_set.i
%include std_array.i
#ifdef SWIGPYTHON
%include std_multimap.i
#endif
%include std_vector.i
%include exception.i
%include typemaps.i

#ifdef OGRE_SWIG_PART
// each part is a separate module, emit swig::traits before the PTR_TRAITS blocks use it
%fragment("StdTraits");
#endif

#ifdef SWIGPYTHON
// std::map<K, T*> and std::set<T*> need these for T, unless a vector/list of T* is instantiated first
%define PTR_TRAITS(T)
%{
    namespace swig {
    template<> struct traits<T> {
        typedef pointer_category category;
        static const char* type_name() { return #T; }
    };
    }
%}
%enddef
#else
%define PTR_TRAITS(T)
%enddef
#endif
 
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

typedef long int time_t;

%feature("autodoc", "1");
%feature("director") *::Listener;
#ifdef SWIGPYTHON
%feature("compactdefaultargs");
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
    catch (std::length_error &e) {
        SWIG_exception(SWIG_IndexError, e.what());
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

// None and an empty SharedPtr are the same thing in Python
%define SHARED_PTR_NONE(TYPE...)
#ifdef SWIGPYTHON
%typemap(in) const Ogre::SharedPtr<TYPE >& (Ogre::SharedPtr<TYPE > empty) {
    if ($input == Py_None) {
        $1 = &empty;
    } else {
        void* argp = 0;
        int res = SWIG_ConvertPtr($input, &argp, $descriptor, SWIG_POINTER_NO_NULL);
        if (!SWIG_IsOK(res))
            %argument_fail(res, "$type", $symname, $argnum);
        $1 = reinterpret_cast<$ltype>(argp);
    }
}
%typemap(in) Ogre::SharedPtr<TYPE > {
    if ($input != Py_None) {
        void* argp = 0;
        int res = SWIG_ConvertPtr($input, &argp, $&descriptor, SWIG_POINTER_NO_NULL);
        if (!SWIG_IsOK(res))
            %argument_fail(res, "$type", $symname, $argnum);
        $1 = *reinterpret_cast<$&ltype>(argp);
    }
}
%typecheck(SWIG_TYPECHECK_POINTER) const Ogre::SharedPtr<TYPE >& {
    $1 = $input == Py_None || SWIG_CheckState(SWIG_ConvertPtr($input, 0, $descriptor, SWIG_POINTER_NO_NULL));
}
%typecheck(SWIG_TYPECHECK_POINTER) Ogre::SharedPtr<TYPE > {
    $1 = $input == Py_None || SWIG_CheckState(SWIG_ConvertPtr($input, 0, $&descriptor, SWIG_POINTER_NO_NULL));
}
%typemap(out) Ogre::SharedPtr<TYPE > {
    $result = static_cast<bool>($1) ? SWIG_NewPointerObj(new $1_ltype($1), $&1_descriptor, SWIG_POINTER_OWN)
                                    : SWIG_Py_Void();
}
%typemap(out) const Ogre::SharedPtr<TYPE >&, Ogre::SharedPtr<TYPE >& {
    $result = static_cast<bool>(*$1) ? SWIG_NewPointerObj(SWIG_as_voidptr($1), $1_descriptor, 0)
                                     : SWIG_Py_Void();
}
#endif
%enddef

%define SHARED_PTR(classname)
// %shared_ptr(type);
%ignore Ogre::SharedPtr<Ogre::classname >::operator const shared_ptr<Ogre::classname >&;
SHARED_PTR_NONE(Ogre::classname)
%template(classname ## Ptr) Ogre::SharedPtr<Ogre::classname >;
%enddef

%ignore *::operator[];
%define SEQUENCE_METHODS
{
    void __setitem__(uint i, float v) {
        if(i >= sizeof(*$self)/sizeof(float))
            throw std::length_error("out of bounds");
        (*$self)[i] = v;
    }
    size_t __len__() { return sizeof(*$self)/sizeof(float); }
    float __getitem__(uint i) {
        if(i >= sizeof(*$self)/sizeof(float))
            throw std::length_error("out of bounds");
        return (*$self)[i];
    }
}
%enddef

#ifdef SWIGJAVA
// use proper (1.5+) enums
%include "enums.swg"
// conflicts with SWIG method
%ignore *::getType;
#endif

#ifdef SWIGCSHARP
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

%ignore Ogre::HardwareBuffer::UsageEnum;
%ignore Ogre::TextureUsage;
%ignore Ogre::GpuConstantType;
%ignore Ogre::GpuProgramParameters::ElementType;
%ignore Ogre::Capabilities;
%ignore Ogre::TextureUnitState::EnvMapType;
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
%csmethodmodifiers *::unloadingComplete "public";
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
%csmethodmodifiers *::createAnimableValue "public";
%csmethodmodifiers Ogre::BillboardChain::preRender "public";
#endif

// connect __getitem__
%feature("python:slot", "sq_item", functype="ssizeargfunc") *::__getitem__;

// connect __setitem__
%feature("python:slot", "sq_ass_item", functype="ssizeobjargproc") *::__setitem__;

// connect __len__
%feature("python:slot", "sq_length", functype="lenfunc") *::__len__;

// stringinterface internal
%rename("$ignore", regextarget=1) "^Cmd+";
