%module OgreRTShader
%{
/* Includes the header in the wrapper code */
#include "Ogre.h"
#include "OgreRTShaderSystem.h"
%}

%include std_string.i
%include exception.i 
%import "Ogre.i"

#define _OgreRTSSExport

%include "OgreShaderPrerequisites.h"
%include "OgreShaderScriptTranslator.h"
%include "OgreShaderSubRenderState.h"
%include "OgreShaderProgramWriter.h"
%include "OgreShaderGenerator.h"
%include "OgreShaderRenderState.h"
%include "OgreShaderFFPTransform.h"
%include "OgreShaderFFPColour.h"
%include "OgreShaderFFPLighting.h"
%include "OgreShaderFFPTexturing.h"
%include "OgreShaderFFPFog.h"
%include "OgreShaderExPerPixelLighting.h"
%include "OgreShaderExNormalMapLighting.h"
%include "OgreShaderExIntegratedPSSM3.h"
%include "OgreShaderExLayeredBlending.h"
%include "OgreShaderExHardwareSkinning.h"
%include "OgreShaderMaterialSerializerListener.h"
