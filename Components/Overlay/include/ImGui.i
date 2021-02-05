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
%ignore ImGui::BulletTextV;
%ignore ImGui::TreeNodeV;
%ignore ImGui::TreeNodeExV;
%ignore ImGui::SetTooltipV;
%ignore ImGuiTextBuffer::appendfv;

%apply bool* INOUT { bool* p_open };
%apply float* INOUT { float* v };
%apply int* INOUT { int* v };
%include "imgui.h"