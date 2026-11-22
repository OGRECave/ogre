// Python fragments that take precedence over the SWIG defaults, see SWIG's own pyfragments.swg.
// Accept numbers like Python does: anything with __index__ for integers and anything with
// __float__ or __index__ for floats. This allows passing numpy scalars like np.float32 or np.int64.

%fragment(SWIG_AsVal_frag(double), "header") {
SWIGINTERN int SWIG_AsVal_dec(double)(PyObject *obj, double *val)
{
    double v = PyFloat_AsDouble(obj);
    if (v == -1.0 && PyErr_Occurred()) {
        PyErr_Clear();
        return SWIG_TypeError;
    }
    if (val) *val = v;
    return SWIG_OK;
}
}

%define AS_VAL_INDEX(Type, Convert)
%fragment(SWIG_AsVal_frag(Type), "header") {
SWIGINTERN int SWIG_AsVal_dec(Type)(PyObject *obj, Type *val)
{
    if (!PyIndex_Check(obj))
        return SWIG_TypeError;
    PyObject *index = PyNumber_Index(obj);
    if (!index) {
        PyErr_Clear();
        return SWIG_TypeError;
    }
    Type v = Convert(index);
    Py_DECREF(index);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        return SWIG_OverflowError;
    }
    if (val) *val = v;
    return SWIG_OK;
}
}
%enddef
AS_VAL_INDEX(long, PyLong_AsLong)
AS_VAL_INDEX(unsigned long, PyLong_AsUnsignedLong)
AS_VAL_INDEX(long long, PyLong_AsLongLong)
AS_VAL_INDEX(unsigned long long, PyLong_AsUnsignedLongLong)
