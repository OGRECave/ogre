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
%ignore ImGuiTextBuffer::appendfv;

%typemap(in) ImTextureID {
    size_t argp;
    int res = SWIG_AsVal_size_t($input, &argp);
    if(SWIG_IsOK(res))
        $1 = ($ltype)(argp);
    else
        SWIG_exception_fail(SWIG_TypeError, "Expected size_t");
}
%typecheck(SWIG_TYPECHECK_POINTER) ImTextureID {
    $1 = true; // actual check in the typemap
}

%apply bool* INOUT { bool* p_open };
%apply float* INOUT { float* v };
%apply int* INOUT { int* v };
%include "imgui.h"