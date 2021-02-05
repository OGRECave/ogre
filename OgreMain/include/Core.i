%module(package="Ogre", directors="1") Core
%{
#include "Ogre.h"
#include "OgreFileSystemLayer.h"
#include "OgreDataStream.h"
#include "OgreCodec.h"
#include "OgreZip.h"
%}

%include exception.i
%include typemaps.i

%include std_shared_ptr.i
%include std_string.i
%include std_vector.i
%include std_pair.i
%include std_map.i

// so swig correctly resolves "using std::*" declarations
%inline %{
using namespace std;
%}

#define _OgreExport
#define _OgrePrivate

#define OGRE_DEPRECATED
#define OGRE_NORETURN
#define OGRE_NODISCARD
#define OGRE_STATIC_MUTEX(arg)
#define OGRE_MUTEX(arg)

%feature("autodoc", "1");

%ignore *::operator=;  // needs rename to wrap
%ignore *::setUserAny; // deprecated
%ignore *::getUserAny; // deprecated

%ignore *::getSingletonPtr; // only expose the non ptr variant
%ignore Ogre::Exception::getNumber; // deprecated
%ignore Ogre::ExceptionFactory::throwExceptionEx; // deprecated
%rename(OgreException) Ogre::Exception; // confilcts with Python Exception

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

// connect operator[] to __getitem__
%feature("python:slot", "sq_item", functype="ssizeargfunc") *::operator[];
%rename(__getitem__) *::operator[];
%ignore Ogre::Matrix3::operator[];
%ignore Ogre::Matrix4::operator[];
%ignore Ogre::ColourValue::operator[];

// + - * /
%ignore *::operator+;
%rename(__add__) *::operator+;

%ignore *::operator-;
%rename(__sub__) *::operator-;

%ignore *::operator*;
%rename(__mul__) *::operator*;

%ignore *::operator/;
%rename(__div__) *::operator/;

// connect __setitem__
%feature("python:slot", "sq_ass_item", functype="ssizeobjargproc") *::__setitem__;

// connect operator<< to tp_repr
%ignore ::operator<<;
%feature("python:slot", "tp_repr", functype="reprfunc") *::__repr__;


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

/* these are ordered by dependency */
%include "OgreBuildSettings.h"
%include "OgrePrerequisites.h"
%include "OgrePlatform.h"
%include "OgreConfig.h"
%ignore Ogre::AllocPolicy;
%import "OgreMemoryAllocatorConfig.h"
%include "OgreCommon.h"
%template(NameValuePairList) std::map<Ogre::String, Ogre::String>;
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
%template(StringVector) std::vector<Ogre::String>;  // actual vector<T>
%template(StringVectorPtr) Ogre::SharedPtr<std::vector<Ogre::String> >;

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
    void __setitem__(int i, float v) { (*$self)[i] = v; }
}
%enddef

%ignore Ogre::VectorBase<3, int>::VectorBase;
%template(VectorBase3i) Ogre::VectorBase<3, int>;
%template(Vector3i) Ogre::Vector<3, int>;

TPL_VECTOR(2)
TPL_VECTOR(3)
TPL_VECTOR(4)

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
    $1 = true; // actual check in the typemap
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
%include "OgreColourValue.h"
ADD_REPR(ColourValue)
%include "OgreRay.h"
// image
%ignore Ogre::PixelUtil::getBNFExpressionOfPixelFormats;
%include "OgrePixelFormat.h"
#ifdef SWIGCSHARP
%extend Ogre::PixelBox
{
    void* getData() { return $self->data; }
}
#endif
%include "OgreImage.h"

// I/O
%include "OgreFileSystemLayer.h"

%feature("valuewrapper") Ogre::Log::Stream;
%feature("director") Ogre::LogListener;
%include "OgreLog.h"
%include "OgreLogManager.h"

// data streams
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

%ignore Ogre::Codec::getCodecIterator;
%include "OgreCodec.h"

%include "OgreArchive.h"
%include "OgreFactoryObj.h"
// %template(FactoryObjArchive) Ogre::FactoryObj<Ogre::Archive>;
%include "OgreArchiveFactory.h"
%ignore Ogre::ZipArchiveFactory; // private
%ignore Ogre::ZipDataStream; // private
%include "OgreZip.h"
%ignore Ogre::ArchiveManager::getArchiveIterator;
%include "OgreArchiveManager.h"

// string interface
#ifdef SWIGPYTHON
%ignore std::vector<Ogre::ParameterDef>::resize; // non default constructible
%ignore std::vector<Ogre::ParameterDef>::vector;
%template(ParameterList) std::vector<Ogre::ParameterDef>;
#endif
SHARED_PTR(StringInterface);
%include "OgreStringInterface.h"

%include "OgreBlendMode.h"
%include "OgreRenderSystemCapabilities.h"