#ifdef SWIGPYTHON
%module(package="Ogre", directors="1") Overlay
#else
%module OgreOverlay
#endif
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreUnifiedHighLevelGpuProgram.h"

#include "OgreOverlayPrerequisites.h"
#include "OgreFont.h"
#include "OgreFontManager.h"
#include "OgreOverlay.h"
#include "OgreOverlaySystem.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayElementFactory.h"
#include "OgreOverlayManager.h"
#include "OgrePanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"

%}
#ifdef HAVE_IMGUI
%{
#include "imgui.h"
#include "OgreImGuiOverlay.h"
%}
#endif

%include std_string.i
%include exception.i 
%import "Ogre.i"

#define _OgreOverlayExport

#ifdef SWIGCSHARP
%csmethodmodifiers Ogre::OverlaySystem::eventOccurred "public";
#endif

%include "OgreOverlayPrerequisites.h"
SHARED_PTR(Font);
%include "OgreFont.h"
%include "OgreFontManager.h"
%ignore Ogre::Overlay::get2DElementsIterator;
%include "OgreOverlay.h"
SHARED_PTR(OverlayElement);
%include "OgreOverlayElement.h"
%include "OgreOverlayElementFactory.h"
SHARED_PTR(OverlayContainer);
%include "OgreOverlayContainer.h"
%include "OgreOverlayManager.h"
SHARED_PTR(OverlaySystem);
%include "OgreOverlaySystem.h"
SHARED_PTR(PanelOverlayElement);
%include "OgrePanelOverlayElement.h"
%ignore Ogre::TextAreaOverlayElement::getFontName;
SHARED_PTR(TextAreaOverlayElement);
%include "OgreTextAreaOverlayElement.h"

#ifdef HAVE_IMGUI
%include stdint.i
%include typemaps.i

%include "OgreImGuiOverlay.h"

/// Imgui
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
#endif