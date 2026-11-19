// Ogre SWIG bindings: prerequisites, math, primitives, config, logging, data streams, archives, scene queries, frame listeners, controllers.
// Part of Ogre.i.

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
%template(LightList) std::vector<Ogre::Light*>;
%template(UnaryOptionList) std::map<std::string, bool>;
%template(IndexMap) std::vector<unsigned short>;
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
%ignore Ogre::Math::calculateTangentSpaceVector; // deprecated
%template(PlaneList) std::vector<Ogre::Plane>;
%include "OgreMath.h"
ADD_REPR(Degree)
ADD_REPR(Radian)
%template(RayTestResult) std::pair<bool, float>;
%include "OgreStringVector.h"
%template(StringList) std::vector<Ogre::String>;  // actual vector<T>
SHARED_PTR_NONE(std::vector<Ogre::String>)
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

%extend Ogre::Vector<N, Ogre::Real> SEQUENCE_METHODS
%enddef

%ignore Ogre::VectorBase<3, int>::VectorBase;
%template(VectorBase3i) Ogre::VectorBase<3, int>;
%template(Vector3i) Ogre::Vector<3, int>;
%extend Ogre::Vector<3, int> SEQUENCE_METHODS

TPL_VECTOR(2)
TPL_VECTOR(3)
TPL_VECTOR(4)

%extend Ogre::ColourValue SEQUENCE_METHODS
%extend Ogre::Quaternion SEQUENCE_METHODS

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
%typemap(in) const Ogre::Radian& (Ogre::Radian tmp) {
    void *argp = 0;
    int res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)(argp);
    }
    else if(SWIG_IsOK(SWIG_ConvertPtr($input, &argp, $descriptor(Ogre::Degree*), $disown)))
    {
        tmp = *((Ogre::Degree*)argp);
        $1 = &tmp;
    }
    else
    {
        res = SWIG_AsVal_float($input, (float*)&tmp);
        $1 = &tmp;

        if (!SWIG_IsOK(res))
            %argument_fail(SWIG_TypeError, "float (Ogre::Radian) or Ogre::Degree", $symname, $argnum);
    }
}
// accept what the typemap accepts, but let a Real overload take plain floats first
%typecheck(SWIG_TYPECHECK_DOUBLE) const Ogre::Radian& {
    $1 = SWIG_CheckState(SWIG_ConvertPtr($input, 0, $descriptor, SWIG_POINTER_NO_NULL)) ||
         SWIG_CheckState(SWIG_ConvertPtr($input, 0, $descriptor(Ogre::Degree*), SWIG_POINTER_NO_NULL)) ||
         SWIG_CheckState(SWIG_AsVal_float($input, NULL));
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

// specific typemap for loadDynamicImage, to allow passing a numpy array directly
%define PYBUFFER_TO_IMAGE_DATA(buffer_var, width_var, height_var, depth_var, format_var, data_var)
    int res = PyObject_GetBuffer($input, &buffer_var, PyBUF_CONTIG | PyBUF_FORMAT);
    if (res < 0) {
        SWIG_fail;
    }

    data_var = (Ogre::uchar*)buffer_var.buf;
    int channels = 1;
    depth_var = 1;

    if (buffer_var.ndim == 2)
    {
        height_var = (Ogre::uint32)buffer_var.shape[0];
        width_var = (Ogre::uint32)buffer_var.shape[1];
    }
    else if (buffer_var.ndim == 3)
    {
        height_var = (Ogre::uint32)buffer_var.shape[0];
        width_var = (Ogre::uint32)buffer_var.shape[1];
        channels = buffer_var.shape[2];
    }
    else if(buffer_var.ndim == 4)
    {
        depth_var = (Ogre::uint32)buffer_var.shape[0];
        height_var = (Ogre::uint32)buffer_var.shape[1];
        width_var = (Ogre::uint32)buffer_var.shape[2];
        channels = buffer_var.shape[3];
    }
    else
    {
        PyBuffer_Release(&buffer_var);
        PyErr_Format(PyExc_ValueError, "ndim must be 2, 3 or 4, got: %d", buffer_var.ndim);
        SWIG_fail;
    }

    char type_char = buffer_var.format ? buffer_var.format[0] : 'B';

    switch(channels)
    {
    case 4:
        format_var = Ogre::PF_BYTE_RGBA;
        break;
    case 3:
        format_var = Ogre::PF_BYTE_RGB;
        break;
    case 1:
        {
            switch(type_char)
            {
            case 'B':
            case 'b':
                format_var = Ogre::PF_L8;
                break;
            case 'H':
                format_var = Ogre::PF_L16;
                break;
            case 'f':
                format_var = Ogre::PF_FLOAT32_R;
                break;
            default:
                PyBuffer_Release(&buffer_var);
                PyErr_Format(PyExc_ValueError, "Unsupported data type: %c", type_char);
                SWIG_fail;
            }
        }
        break;
    default:
        PyBuffer_Release(&buffer_var);
        PyErr_Format(PyExc_ValueError, "Unsupported channel count: %d", channels);
        SWIG_fail;
    }
%enddef

%typecheck(SWIG_TYPECHECK_POINTER) (Ogre::uchar* data, Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat format) {
    $1 = PyObject_CheckBuffer($input);
}

%typemap(in) (Ogre::uchar* data, Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat format) {
    Py_buffer view;
    PYBUFFER_TO_IMAGE_DATA(view, $2, $3, $4, $5, $1)
    PyBuffer_Release(&view);
}
// the Image only borrows the data, so keep the buffer alive as long as the Image references it
%typemap(argout) (Ogre::uchar* data, Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat format) {
    if (PyObject_SetAttrString(self, "_buffer", $input) < 0) SWIG_fail;
}

%typecheck(SWIG_TYPECHECK_POINTER) (Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat pixelFormat, void *pixelData) {
    $1 = PyObject_CheckBuffer($input);
}

%typemap(in) (Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat pixelFormat, void *pixelData) {
    Py_buffer view;
    PYBUFFER_TO_IMAGE_DATA(view, $1, $2, $3, $4, $5)
    PyBuffer_Release(&view);
}
// the PixelBox only borrows the data, so keep the buffer alive as long as the PixelBox references it
%typemap(argout) (Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 depth, Ogre::PixelFormat pixelFormat, void *pixelData) {
    if (PyObject_SetAttrString(self, "_buffer", $input) < 0) SWIG_fail;
}
// Python owns the buffer, so the Image must only borrow it
%typemap(check) (Ogre::uchar* buffer, bool autoDelete) {
    if ($1) $2 = false;
}
%typemap(argout) (Ogre::uchar* buffer, bool autoDelete) {
    if ($1 && PyObject_SetAttrString(self, "_buffer", $input) < 0) SWIG_fail;
}
// the returned PixelBox points into the data of self, so keep self alive as long as the PixelBox references it
%typemap(ret) Ogre::PixelBox getPixelBox, Ogre::PixelBox getSubVolume {
    if (PyObject_SetAttrString($result, "_owner", self) < 0) SWIG_fail;
}


%define TYPEMAP_SEQUENCE_FOR(TYPE, LEN_CHECK, DESCRIPTION)
%typemap(in) const TYPE& (TYPE temp) {
    void *argp = 0;
    int res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)(argp);
    } else {
        // overload resolution checks the length, but plain functions do not
        Py_ssize_t len = PySequence_Check($input) ? PySequence_Length($input) : -1;
        if (len < 0 || !(LEN_CHECK)) {
            %argument_fail(SWIG_TypeError, DESCRIPTION, $symname, $argnum);
        }
        for (Py_ssize_t i = 0; i < len; i++) {
            PyObject *o = PySequence_GetItem($input, i);
            double v = o ? PyFloat_AsDouble(o) : -1;
            Py_XDECREF(o);
            if (v == -1 && PyErr_Occurred()) {
                %argument_fail(SWIG_TypeError, DESCRIPTION, $symname, $argnum);
            }
            temp.ptr()[i] = (float)v;
        }
        $1 = &temp;
    }
}
%typecheck(SWIG_TYPECHECK_STRING) const TYPE& {
    if (SWIG_CheckState(SWIG_ConvertPtr($input, 0, $descriptor, SWIG_POINTER_NO_NULL))) {
        $1 = true;
    }
    // do not treat other wrapped types as sequences, e.g. a ColourValue is not a Quaternion
    else if (SWIG_Python_GetSwigThis($input)) {
        $1 = false;
    }
    else if (PySequence_Check($input))
    {
        int len = PySequence_Length($input);
        $1 = (LEN_CHECK);
    }
    else {
        $1 = false;
    }
}
%enddef
TYPEMAP_SEQUENCE_FOR(Ogre::Vector2, len == 2, "Sequence[float] of length 2")
TYPEMAP_SEQUENCE_FOR(Ogre::Vector3, len == 3, "Sequence[float] of length 3")
TYPEMAP_SEQUENCE_FOR(Ogre::Vector4, len == 4, "Sequence[float] of length 4")
TYPEMAP_SEQUENCE_FOR(Ogre::Quaternion, len == 4, "Sequence[float] of length 4")
TYPEMAP_SEQUENCE_FOR(Ogre::ColourValue, len >= 3 && len <= 4, "Sequence[float] of length 3 or 4")
#endif

%ignore Ogre::Matrix3::Orthonormalize;
%include "OgreMatrix3.h"
ADD_REPR(Matrix3)
%extend Ogre::Matrix3
{
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
}
%ignore Ogre::TransformBaseReal::extract3x3Matrix; // deprecated
%ignore Ogre::TransformBaseReal::extractQuaternion; // deprecated
%ignore Ogre::Matrix4::concatenate; // deprecated
#ifdef SWIGPYTHON
// m[row, column]
%feature("python:slot", "mp_subscript", functype="binaryfunc") Ogre::Matrix4::__getitem__;
%typemap(in) (int row, int column) {
    if (!PyTuple_Check($input) || !PyArg_ParseTuple($input, "ii", &$1, &$2)) {
        %argument_fail(SWIG_TypeError, "tuple (row, column)", $symname, $argnum);
    }
}
#endif
%include "OgreMatrix4.h"
ADD_REPR(Matrix4)
%template(TransformBaseMatrix4) Ogre::TransformBase<4, Ogre::Real>;
%extend Ogre::Matrix4
{
    Ogre::Vector4 operator*(const Ogre::Vector4& v) { return *$self * v; }
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
    Ogre::Matrix4 operator*(const Ogre::Matrix4& m) { return *$self * m; }
    Ogre::Matrix4 operator*(const Ogre::Affine3& m) { return *$self * m; }
    Ogre::Matrix4 operator+(const Ogre::Matrix4& m) { return *$self + m; }
    Ogre::Matrix4 operator-(const Ogre::Matrix4& m) { return *$self - m; }
    Ogre::Real __getitem__(int row, int column) {
        if(row < 0 || row > 3 || column < 0 || column > 3)
            throw std::length_error("out of bounds");
        return (*$self)[row][column];
    }
}
ADD_REPR(Affine3)
%extend Ogre::Affine3
{
    Ogre::Vector4 operator*(const Ogre::Vector4& v) { return *$self * v; }
    Ogre::Vector3 operator*(const Ogre::Vector3& v) { return *$self * v; }
    Ogre::Affine3 operator*(const Ogre::Affine3& m) { return *$self * m; }
    Ogre::Matrix4 operator*(const Ogre::Matrix4& m) { return *$self * m; }
}
%include "OgreQuaternion.h"
ADD_REPR(Quaternion)
%include "OgreSimpleSpline.h"
%include "OgreRotationalSpline.h"
// Geometric Primitives
%template(Corners) std::array<Ogre::Vector3, 8>;
%template(Vector3List) std::vector<Ogre::Vector3>;
%template(Vector4List) std::vector<Ogre::Vector4>;
%include "OgreAxisAlignedBox.h"
ADD_REPR(AxisAlignedBox)
%include "OgreSphere.h"
%include "OgrePlane.h"
ADD_REPR(Plane)
%include "OgrePlaneBoundedVolume.h"
// I/O
%template(PlaneBoundedVolumeList) std::vector<Ogre::PlaneBoundedVolume>;
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
%feature("director") Ogre::MeshSerializerListener;
%feature("director") Ogre::ResourceLoadingListener;
// More Data Types
%ignore Ogre::ColourValue::getHSB; // deprecated
%include "OgreColourValue.h"
%include "OgreRay.h"
%template(SceneQueryMovableObjectPair) std::pair<Ogre::MovableObject*, Ogre::MovableObject*>;
#ifndef SWIGJAVA // older std_list.i clashes with JDK 21 List.removeFirst
%template(SceneQueryMovableIntersectionList) std::list<std::pair<Ogre::MovableObject*, Ogre::MovableObject*> >;
#endif
%include "OgreSceneQuery.h"
%template(RaySceneQueryResult) std::vector<Ogre::RaySceneQueryResultEntry>;
#ifdef SWIGPYTHON
%template(SceneQueryResultMovableList) std::list<Ogre::MovableObject*>;
#endif

%ignore *::convertColourValue; // deprecated

%include "OgreNameGenerator.h"
%include "OgreController.h"

#ifdef SWIGPYTHON
%ignore std::vector<Ogre::ParameterDef>::resize; // non default constructible
%ignore std::vector<Ogre::ParameterDef>::vector;
%template(ParameterList) std::vector<Ogre::ParameterDef>;
#endif
SHARED_PTR(StringInterface);
%include "OgreStringInterface.h"
%template(ControllerFloat) Ogre::Controller<float>;
// the GPU parameter controllers take a GpuProgramParametersPtr, wrapped later
SHARED_PTR_NONE(Ogre::GpuProgramParameters)
SHARED_PTR_NONE(Ogre::ControllerValue<float>)
SHARED_PTR_NONE(Ogre::ControllerFunction<float>)
%template(ControllerValueFloatPtr) Ogre::SharedPtr<Ogre::ControllerValue<float> >;
%template(ControllerFunctionPtr) Ogre::SharedPtr<Ogre::ControllerFunction<float> >;
%include "OgreControllerManager.h"
%include "OgrePredefinedControllers.h"
