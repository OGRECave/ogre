%module OgreOverlay
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

%include std_string.i
%include exception.i 
%import "Ogre.i"

#define _OgreOverlayExport

%include "OgreOverlayPrerequisites.h"
%include "OgreFont.h"
%include "OgreFontManager.h"
%ignore Ogre::Overlay::get2DElementsIterator;
%include "OgreOverlay.h"
%include "OgreOverlayElement.h"
%include "OgreOverlayElementFactory.h"
%include "OgreOverlayContainer.h"
%include "OgreOverlayManager.h"
%include "OgreOverlaySystem.h"
%include "OgrePanelOverlayElement.h"
%ignore Ogre::TextAreaOverlayElement::getFontName;
%include "OgreTextAreaOverlayElement.h"