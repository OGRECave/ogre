/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __CompositorScriptScompiler_H__
#define __CompositorScriptScompiler_H__

#include "OgrePrerequisites.h"
#include "OgreCompiler2Pass.h"
#include "OgreCompositor.h"
#include "OgreRenderSystem.h"


namespace Ogre {

	/** Compiler for parsing & lexing .compositor scripts */
	class _OgreExport CompositorScriptCompiler : public Compiler2Pass
	{

	public:
		CompositorScriptCompiler(void);
		~CompositorScriptCompiler(void);

        /** gets BNF Grammar for Compositor script.
        */
        virtual const String& getClientBNFGrammer(void) const;

        /** get the name of the Compositor script BNF grammar.
        */
        virtual const String& getClientGrammerName(void) const;

        /** Compile a compositor script from a data stream using a specific resource group name.
        @param stream Weak reference to a data stream which is the source of the material script
        @param groupName The name of the resource group that resources which are
			parsed are to become a member of. If this group is loaded or unloaded,
			then the resources discovered in this script will be loaded / unloaded
			with it.
        */
        void parseScript(DataStreamPtr& stream, const String& groupName)
        {
            mScriptContext.groupName = groupName;
            Compiler2Pass::compile(stream->getAsString(),  stream->getName());
        }

	protected:
		// Token ID enumeration
		enum TokenID {
			// Terminal Tokens section
			ID_UNKOWN = 0,
			// Techniques
			ID_TARGET_WIDTH, ID_TARGET_HEIGHT,
			ID_TARGET_WIDTH_SCALED, ID_TARGET_HEIGHT_SCALED,
			ID_PF_A8R8G8B8, ID_PF_R8G8B8A8, ID_PF_R8G8B8, 
			ID_PF_FLOAT16_R, ID_PF_FLOAT16_RGB, ID_PF_FLOAT16_RGBA,
			ID_PF_FLOAT32_R, ID_PF_FLOAT32_RGB, ID_PF_FLOAT32_RGBA,
			ID_PF_FLOAT16_GR, ID_PF_FLOAT32_GR,
			ID_SHARED, ID_GAMMA, ID_NO_FSAA,
			// Targets
			ID_PREVIOUS, ID_NONE,
			// Passes
			ID_RENDER_QUAD, ID_CLEAR, ID_STENCIL, ID_RENDER_SCENE,
			// Clear section
			ID_CLR_COLOUR, ID_CLR_DEPTH,
			// Stencil section

			// compare functions
            ID_ST_ALWAYS_FAIL, ID_ST_ALWAYS_PASS, ID_ST_LESS,
            ID_ST_LESS_EQUAL, ID_ST_EQUAL, ID_ST_NOT_EQUAL,
            ID_ST_GREATER_EQUAL, ID_ST_GREATER,

            // stencil operations
            ID_ST_KEEP, ID_ST_ZERO, ID_ST_REPLACE, ID_ST_INCREMENT,
            ID_ST_DECREMENT, ID_ST_INCREMENT_WRAP, ID_ST_DECREMENT_WRAP,
            ID_ST_INVERT,

			// general
			ID_ON, ID_OFF, ID_TRUE, ID_FALSE,
            // where auto generated tokens start so do not remove
            ID_AUTOTOKENSTART
		};

		/** Enum to identify compositor sections. */
		enum CompositorScriptSection
		{
			CSS_NONE,
			CSS_COMPOSITOR,
			CSS_TECHNIQUE,
			CSS_TARGET,
			CSS_PASS
		};
		/** Struct for holding the script context while parsing. */
		struct CompositorScriptContext
		{
			CompositorScriptSection section;
		    String groupName;
			CompositorPtr compositor;
			CompositionTechnique* technique;
			CompositionTargetPass* target;
			CompositionPass* pass;
		};

		CompositorScriptContext mScriptContext;

		typedef void (CompositorScriptCompiler::* CSC_Action)(void);
		typedef std::map<size_t, CSC_Action> TokenActionMap;
		typedef TokenActionMap::iterator TokenActionIterator;
		/** Map of Token value as key to an Action.  An Action converts tokens into
		the final format.
            All instances use the same Token Action Map.
		*/
		static TokenActionMap mTokenActionMap;

		/** Execute an Action associated with a token.  Gets called when the compiler finishes tokenizing a
		section of the source that has been parsed.
		**/
		virtual void executeTokenAction(const size_t tokenID);
        /** Get the start position of auto generated token IDs.
        */
        virtual size_t getAutoTokenIDStart() const {return ID_AUTOTOKENSTART;}
		/** Associate all the lexemes used in a material script with their corresponding tokens and actions.
		**/
        virtual void setupTokenDefinitions(void);
		void addLexemeTokenAction(const String& lexeme, const size_t token, const CSC_Action action = 0);
        void addLexemeAction(const String& lexeme, const CSC_Action action) { addLexemeTokenAction(lexeme, 0, action); }

		void logParseError(const String& error);

		// Token Actions which get called when tokens are created during parsing.
		void parseOpenBrace(void);
		void parseCloseBrace(void);
		void parseCompositor(void);
		void parseTechnique(void);
		void parseTexture(void);
		void parseTarget(void);
		void parseInput(void);
		void parseTargetOutput(void);
		void parseOnlyInitial(void);
		void parseVisibilityMask(void);
		void parseLodBias(void);
		void parseMaterialScheme(void);
		void parsePass(void);
		void parseMaterial(void);
		void parseFirstRenderQueue(void);
		void parseLastRenderQueue(void);
		void parseIdentifier(void);
		void parseClearBuffers(void);
		void parseClearColourValue(void);
		void parseClearDepthValue(void);
		void parseClearStencilValue(void);
		void parseStencilCheck(void);
		void parseStencilFunc(void);
		void parseStencilRefVal(void);
		void parseStencilMask(void);
		void parseStencilFailOp(void);
		void parseStencilDepthFailOp(void);
		void parseStencilPassOp(void);
		void parseStencilTwoSided(void);
		StencilOperation extractStencilOp(void);
        CompareFunction extractCompareFunc(void);
	};
}

#endif
