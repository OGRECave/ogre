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
#ifdef SWIGPYTHON
%include factory.i
#endif
%import "Ogre.i"

#define _OgreOverlayExport

#ifdef SWIGCSHARP
%csmethodmodifiers Ogre::OverlaySystem::eventOccurred "public";
#endif

// DisplayString is implicitly constructable from String
// this breaks when using get*, will fix this by switching to String with 1.13
#define DisplayString String

%include "OgreOverlayPrerequisites.h"
SHARED_PTR(Font);
%include "OgreFont.h"
%include "OgreFontManager.h"
%ignore Ogre::Overlay::get2DElementsIterator;
%include "OgreOverlay.h"
SHARED_PTR(OverlayElement);
%extend Ogre::OverlayElement {
  OverlayContainer* castOverlayContainer()
  {
    return dynamic_cast<Ogre::OverlayContainer*>($self);
  }
  Ogre::PanelOverlayElement* castPanelOverlayElement()
  {
    return dynamic_cast<Ogre::PanelOverlayElement*>($self);
  }
  Ogre::TextAreaOverlayElement* castTextAreaOverlayElement()
  {
    return dynamic_cast<Ogre::TextAreaOverlayElement*>($self);
  }
}
%include "OgreOverlayElement.h"
%include "OgreOverlayElementFactory.h"
SHARED_PTR(OverlayContainer);
%ignore Ogre::OverlayContainer::getChildIterator;
%ignore Ogre::OverlayContainer::getChildContainerIterator;
%include "OgreOverlayContainer.h"
#ifdef SWIGPYTHON
%factory(Ogre::OverlayElement* Ogre::OverlayManager::createOverlayElement, Ogre::OverlayContainer);
#endif
%ignore Ogre::OverlayManager::getTemplateIterator;
%ignore Ogre::OverlayManager::getOverlayIterator;
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