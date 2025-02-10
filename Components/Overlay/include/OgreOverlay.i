#ifdef SWIGPYTHON
%module(package="Ogre", directors="1") Overlay
#else
%module OgreOverlay
#endif
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"

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

#if SWIG_VERSION == 0x040200
// https://github.com/swig/swig/issues/2744
%fragment("SwigPyIterator_T");
#endif

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
%feature("director") Ogre::OverlayElementFactory;
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
#ifdef SWIGPYTHON
%{
    // this is a workaround for the following map
    namespace swig {
    template<> struct traits<Ogre::OverlayElement> {
        typedef pointer_category category;
        static const char* type_name() { return "Ogre::OverlayElement"; }
    };
    }
%}
#endif
%template(OverlayElementMap) std::map<std::string, Ogre::OverlayElement*>;
%include "OgreOverlayManager.h"
SHARED_PTR(OverlaySystem);
%include "OgreOverlaySystem.h"
SHARED_PTR(PanelOverlayElement);
%include "OgrePanelOverlayElement.h"
%ignore Ogre::TextAreaOverlayElement::getFontName;
SHARED_PTR(TextAreaOverlayElement);
%include "OgreTextAreaOverlayElement.h"

#ifdef HAVE_IMGUI
%apply Ogre::String* INOUT { Ogre::String& renderSystemName };
%include "OgreImGuiOverlay.h"
#endif