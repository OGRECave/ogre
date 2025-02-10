%module(package="Ogre", directors="1") ImGui
%{
#include "imgui.h"
%}

#define IMGUI_API

%include stdint.i
%include typemaps.i

// ignore va list methods
%ignore ImGui::TextV;
%ignore ImGui::TextColoredV;
%ignore ImGui::TextDisabledV;
%ignore ImGui::TextWrappedV;
%ignore ImGui::LabelTextV;
%ignore ImGui::LogTextV;
%ignore ImGui::BulletTextV;
%ignore ImGui::TreeNodeV;
%ignore ImGui::TreeNodeExV;
%ignore ImGui::SetTooltipV;
%ignore ImGui::SetItemTooltipV;
%ignore ImGui::DebugLogV;
%ignore ImGuiSelectionBasicStorage;

// not needed in high level languages
%ignore ImGuiTextBuffer;

#ifdef SWIGPYTHON
%typemap(in) ImTextureID {
    size_t argp;
    int res = SWIG_AsVal_size_t($input, &argp);
    if(SWIG_IsOK(res))
        $1 = ($ltype)(argp);
    else
        %argument_fail(SWIG_TypeError, "size_t", $symname, $argnum);
}
%typecheck(SWIG_TYPECHECK_POINTER) ImTextureID {
    $1 = true; // actual check in the typemap
}

%typemap(in) float[4], float[3], float[2] {
    void* argp;
    int res = SWIG_ConvertPtr($input, &argp, $descriptor(ImVec4*), $disown);
    if (SWIG_IsOK(res)) {
        $1 = ($ltype)argp;
    } else {
        %argument_fail(SWIG_TypeError, "ImVec4", $symname, $argnum);
    }
}

// match the signature of the by value variants
%typemap(argout) float[4], float[3], float[2] {
    $result = SWIG_AppendOutput($result, SWIG_NewPointerObj($1, $descriptor(ImVec4*), 0));
}

// for PlotHistogram, PlotLines
%typemap(in) (const float* values, int values_count)
{
    Py_buffer view;
    int res = PyObject_GetBuffer($input, &view, PyBUF_CONTIG_RO | PyBUF_FORMAT);
    if (res < 0) {
        SWIG_fail;
    }
    PyBuffer_Release(&view);

    if(view.ndim != 1 || strcmp(view.format, "f") != 0) {
        %argument_fail(SWIG_TypeError, "array(f)", $symname, $argnum);
    }

    $1 = ($ltype)view.buf;
    $2 = view.len / sizeof(float);
}

%typecheck(SWIG_TYPECHECK_STRING) (const float* values, int values_count) {
    $1 = true; // actual check in the typemap
}

%typecheck(SWIG_TYPECHECK_STRING) float[4], float[3], float[2] {
    $1 = true; // actual check in the typemap
}

%rename("__version__") "IMGUI_VERSION";

// strip duplicate namespace for ImGuiSomething_FlagName flags
%rename("%(strip:[ImGui])s", regextarget=1) "^ImGui.+_.+";
#endif

%apply bool* INOUT { bool* p_open };
%apply bool* INOUT { bool* v };
%apply float* INOUT { float* v };
%apply int* INOUT { int* v };
%include "imgui.h"

%extend ImGuiStyle
{
    const ImVec4& getColor(int i) const {
        return $self->Colors[i];
    }

    void setColor(int i, const ImVec4& v) {
        $self->Colors[i] = v;
    }
}