/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreCompositorScriptCompiler.h"
#include "OgreCommon.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

namespace Ogre {

	//-----------------------------------------------------------------------
    // Static definitions
    //-----------------------------------------------------------------------
    CompositorScriptCompiler::TokenActionMap CompositorScriptCompiler::mTokenActionMap;

    const String& CompositorScriptCompiler::getClientBNFGrammer(void) const
    {
		// simplified Backus - Naur Form (BNF) grammer for compositor scripts
	    static const String compositorScript_BNF =
		// Top level rule
		"<Script> ::= {<Compositor>} \n"
		"<Compositor> ::= 'compositor' <Flex_Label> '{' {<Technique>} '}' \n"
		// Technique
		"<Technique> ::= 'technique' '{' {<Logic>} {<Texture>} {<TextureReference>} {<Target>} <TargetOutput> '}' \n"
		"<Logic> ::= 'compositor_logic' <Label> \n"
		"<Texture> ::= 'texture' <Label> <WidthOption> <HeightOption> <PixelFormat> {<PixelFormat>} [<Shared>] [<Scope>] \n"
		"<TextureReference> ::= 'texture_ref' <Label> <RefCompositorName> <RefTextureName> \n"
		"<WidthOption> ::= <TargetWidthScaled> | 'target_width' | <#width> \n"
		"<HeightOption> ::= <TargetHeightScaled> | 'target_height' | <#height> \n"
		"<TargetWidthScaled> ::= 'target_width_scaled' <#scaling> \n"
		"<TargetHeightScaled> ::= 'target_height_scaled' <#scaling> \n"
		"<PixelFormat> ::= 'PF_A8R8G8B8' | 'PF_R8G8B8A8' | 'PF_R8G8B8' | 'PF_FLOAT16_RGBA' | \n"
        "   'PF_FLOAT16_RGB' | 'PF_FLOAT16_R' | 'PF_FLOAT32_RGBA' | 'PF_FLOAT32_RGB' | 'PF_FLOAT32_R' | \n"
		"   'PF_FLOAT16_GR' | 'PF_FLOAT32_GR' \n"
		//TODO GSOC : Change this to pooled later on
		"<Shared> ::= 'shared' \n"
		"<Scope> ::= 'local_scope' | 'chain_scope' | 'global_scope' \n"
		// Target
		"<Target> ::= 'target ' <Label> '{' {<TargetOptions>} {<Pass>} '}' \n"
	    "<TargetOptions> ::=	<TargetInput> | <OnlyInitial> | <VisibilityMask> | \n"
	    "   <LodBias> | <MaterialScheme> | <Shadows> \n"
		"<TargetInput> ::= 'input' <TargetInputOptions> \n"
		"<TargetInputOptions> ::= 'none' | 'previous' \n"
		"<OnlyInitial> ::= 'only_initial' <On_Off> \n"
		"<VisibilityMask> ::= 'visibility_mask' <#mask> \n"
		"<LodBias> ::= 'lod_bias' <#lodbias> \n"
		"<MaterialScheme> ::= 'material_scheme' <Label> \n"
		"<Shadows> ::= 'shadows' <On_Off> \n"
		"<TargetOutput> ::= 'target_output' '{' [<TargetOptions>] {<Pass>} '}' \n"
		// Pass
		"<Pass> ::= 'pass' <PassTypes> '{' {<PassOptions>} '}' \n"
		"<PassTypes> ::= 'render_quad' | 'clear' | 'stencil' | 'render_scene' \n"
		"<PassOptions> ::= <PassFirstRenderQueue> | <PassLastRenderQueue> | \n"
		"    <PassIdentifier> | <PassMaterial> | <PassInput> | <ClearSection> | <StencilSection> \n"
		"<PassMaterial> ::= 'material' <Label> \n"
		"<PassInput> ::= 'input' <#id> <Label> [<#mrtIndex>] \n"
		"<PassFirstRenderQueue> ::= 'first_render_queue' <#queue> \n"
		"<PassLastRenderQueue> ::= 'last_render_queue' <#queue> \n"
		"<PassIdentifier> ::= 'identifier' <#id> \n"
		// clear
		"<ClearSection> ::= -'clear' -'{' {<ClearOptions>} -'}' \n"
		"<ClearOptions> ::= <Buffers> | <ColourValue> | <DepthValue> | <StencilValue> \n"
		"<Buffers> ::= 'buffers' {<BufferTypes>} \n"
		"<BufferTypes> ::= <Colour> | <Depth> | <Stencil> \n"
		"<Colour> ::= 'colour' (?!<ValueChk>) \n"
		"<Depth> ::= 'depth' (?!<ValueChk>) \n"
		"<Stencil> ::= 'stencil' (?!<ValueChk>) \n"
		"<ValueChk> ::= '_value' \n"
		"<ColourValue> ::= 'colour_value' <#red> <#green> <#blue> <#alpha> \n"
		"<DepthValue> ::= 'depth_value' <#depth> \n"
		"<StencilValue> ::= 'stencil_value' <#val> \n"
		// stencil
		"<StencilSection> ::= -'stencil' -'{' {<StencilOptions>} -'}' \n"
		"<StencilOptions> ::=  <Check> | <CompareFunction> | <RefVal> | <Mask> | <FailOp> | <DepthFailOp> | \n"
		"   <PassOp> | <TwoSided> \n"
		"<Check> ::= 'check' <On_Off> \n"
		"<CompareFunction> ::= 'comp_func' <CompFunc> \n"
		"<CompFunc> ::= 'always_fail' | 'always_pass' | 'less_equal' | 'less' | 'equal' | \n"
		"   'not_equal' | 'equal' | 'greater_equal' | 'greater' \n"
        "<RefVal> ::= 'ref_value' <#val> \n"
        "<Mask> ::= 'mask' <#mask> \n"
        "<FailOp> ::= 'fail_op' <StencilOperation> \n"
        "<DepthFailOp> ::= 'depth_fail_op' <StencilOperation> \n"
        "<PassOp> ::= 'pass_op' <StencilOperation> \n"
        "<TwoSided> ::= 'two_sided' <On_Off> \n"
		"<StencilOperation> ::= 'keep' | 'zero' | 'replace' | 'increment_wrap' | 'increment' | \n"
		"   'decrement_wrap' | 'decrement' | 'invert' \n"

		// common rules
		"<On_Off> ::= 'on' | 'off' \n"
		"<Label> ::= <Quoted_Label> | <Unquoted_Label> \n"
		"<Flex_Label> ::= <Quoted_Label> | <Spaced_Label> \n"
		"<Quoted_Label> ::= -'\"' <Spaced_Label> -'\"' \n"
		"<Spaced_Label> ::= <Spaced_Label_Illegals> {<Spaced_Label_Illegals>} \n"
        "<Unquoted_Label> ::= <Unquoted_Label_Illegals> {<Unquoted_Label_Illegals>} \n"
		"<Spaced_Label_Illegals> ::= (!,\n\r\t{}\") \n"
		"<Unquoted_Label_Illegals> ::= (! \n\r\t{}\") \n"

		;

        return compositorScript_BNF;
    }
	//-----------------------------------------------------------------------
    const String& CompositorScriptCompiler::getClientGrammerName(void) const
    {
        static const String grammerName = "Compositor Script";
        return grammerName;
    }
	//-----------------------------------------------------------------------
	CompositorScriptCompiler::CompositorScriptCompiler(void)
	{
        // set default group resource name
        mScriptContext.groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
	}
	//-----------------------------------------------------------------------
	CompositorScriptCompiler::~CompositorScriptCompiler(void)
	{

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::setupTokenDefinitions(void)
	{
		addLexemeAction("{", &CompositorScriptCompiler::parseOpenBrace);
		addLexemeAction("}", &CompositorScriptCompiler::parseCloseBrace);
		addLexemeAction("compositor", &CompositorScriptCompiler::parseCompositor);

		// Technique section
		addLexemeAction("technique", &CompositorScriptCompiler::parseTechnique);
		addLexemeAction("texture", &CompositorScriptCompiler::parseTexture);
		addLexemeAction("texture_ref", &CompositorScriptCompiler::parseTextureRef);
		addLexemeAction("compositor_logic", &CompositorScriptCompiler::parseCompositorLogic);
		addLexemeAction("scheme", &CompositorScriptCompiler::parseScheme);
		addLexemeToken("target_width_scaled", ID_TARGET_WIDTH_SCALED);
		addLexemeToken("target_height_scaled", ID_TARGET_HEIGHT_SCALED);
		addLexemeToken("target_width", ID_TARGET_WIDTH);
		addLexemeToken("target_height", ID_TARGET_HEIGHT);
		addLexemeToken("PF_A8R8G8B8", ID_PF_A8R8G8B8);
		addLexemeToken("PF_R8G8B8A8", ID_PF_R8G8B8A8);
		addLexemeToken("PF_R8G8B8", ID_PF_R8G8B8);
		addLexemeToken("PF_FLOAT16_R", ID_PF_FLOAT16_R);
		addLexemeToken("PF_FLOAT16_GR", ID_PF_FLOAT16_GR);
		addLexemeToken("PF_FLOAT16_RGB", ID_PF_FLOAT16_RGB);
		addLexemeToken("PF_FLOAT16_RGBA", ID_PF_FLOAT16_RGBA);
		addLexemeToken("PF_FLOAT32_R", ID_PF_FLOAT32_R);
		addLexemeToken("PF_FLOAT32_GR", ID_PF_FLOAT32_GR);
		addLexemeToken("PF_FLOAT32_RGB", ID_PF_FLOAT32_RGB);
		addLexemeToken("PF_FLOAT32_RGBA", ID_PF_FLOAT32_RGBA);
		addLexemeToken("shared", ID_POOLED); //TODO GSOC : change name string later too
		addLexemeToken("local_scope", ID_SCOPE_LOCAL); 
		addLexemeToken("chain_scope", ID_SCOPE_CHAIN); 
		addLexemeToken("global_scope", ID_SCOPE_GLOBAL);
		addLexemeToken("gamma", ID_GAMMA);
		addLexemeToken("no_fsaa", ID_NO_FSAA);

		// Target section
		addLexemeAction("target ", &CompositorScriptCompiler::parseTarget);
		addLexemeAction("input", &CompositorScriptCompiler::parseInput);
		addLexemeToken("none", ID_NONE);
		addLexemeToken("previous", ID_PREVIOUS);
		addLexemeAction("target_output", &CompositorScriptCompiler::parseTargetOutput);
		addLexemeAction("only_initial", &CompositorScriptCompiler::parseOnlyInitial);
		addLexemeAction("visibility_mask", &CompositorScriptCompiler::parseVisibilityMask);
		addLexemeAction("lod_bias", &CompositorScriptCompiler::parseLodBias);
		addLexemeAction("material_scheme", &CompositorScriptCompiler::parseMaterialScheme);
		addLexemeAction("shadows", &CompositorScriptCompiler::parseShadowsEnabled);

		// pass section
		addLexemeAction("pass", &CompositorScriptCompiler::parsePass);
		// input defined above
		addLexemeToken("render_quad", ID_RENDER_QUAD);
		addLexemeToken("clear", ID_CLEAR);
		addLexemeToken("stencil", ID_STENCIL);
		addLexemeToken("render_scene", ID_RENDER_SCENE);
		// pass attributes
		addLexemeAction("material", &CompositorScriptCompiler::parseMaterial);
		addLexemeAction("first_render_queue", &CompositorScriptCompiler::parseFirstRenderQueue);
		addLexemeAction("last_render_queue", &CompositorScriptCompiler::parseLastRenderQueue);
		addLexemeAction("identifier", &CompositorScriptCompiler::parseIdentifier);
		// clear
		addLexemeAction("buffers", &CompositorScriptCompiler::parseClearBuffers);
		addLexemeToken("colour", ID_CLR_COLOUR);
		addLexemeToken("depth", ID_CLR_DEPTH);
		addLexemeAction("colour_value", &CompositorScriptCompiler::parseClearColourValue);
		addLexemeAction("depth_value", &CompositorScriptCompiler::parseClearDepthValue);
		addLexemeAction("stencil_value", &CompositorScriptCompiler::parseClearStencilValue);
		// stencil
		addLexemeAction("check", &CompositorScriptCompiler::parseStencilCheck);
		addLexemeAction("comp_func", &CompositorScriptCompiler::parseStencilFunc);
		addLexemeAction("ref_value", &CompositorScriptCompiler::parseStencilRefVal);
		addLexemeAction("mask", &CompositorScriptCompiler::parseStencilMask);
		addLexemeAction("fail_op", &CompositorScriptCompiler::parseStencilFailOp);
		addLexemeAction("depth_fail_op", &CompositorScriptCompiler::parseStencilDepthFailOp);
		addLexemeAction("pass_op", &CompositorScriptCompiler::parseStencilPassOp);
		addLexemeAction("two_sided", &CompositorScriptCompiler::parseStencilTwoSided);
		// compare functions
		addLexemeToken("always_fail", ID_ST_ALWAYS_FAIL);
		addLexemeToken("always_pass", ID_ST_ALWAYS_PASS);
		addLexemeToken("less", ID_ST_LESS);
		addLexemeToken("less_equal", ID_ST_LESS_EQUAL);
		addLexemeToken("equal", ID_ST_EQUAL);
		addLexemeToken("not_equal", ID_ST_NOT_EQUAL);
		addLexemeToken("greater_equal", ID_ST_GREATER_EQUAL);
		addLexemeToken("greater", ID_ST_GREATER);
		// stencil operations
		addLexemeToken("keep", ID_ST_KEEP);
		addLexemeToken("zero", ID_ST_ZERO);
		addLexemeToken("replace", ID_ST_REPLACE);
		addLexemeToken("increment", ID_ST_INCREMENT);
		addLexemeToken("decrement", ID_ST_DECREMENT);
		addLexemeToken("increment_wrap", ID_ST_INCREMENT_WRAP);
		addLexemeToken("decrement_wrap", ID_ST_DECREMENT_WRAP);
		addLexemeToken("invert", ID_ST_INVERT);

		// common section
		addLexemeToken("on", ID_ON);
		addLexemeToken("off", ID_OFF);

	}

	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::addLexemeTokenAction(const String& lexeme,
		const size_t token, const CSC_Action action)
	{
		size_t newtokenID = addLexemeToken(lexeme, token, action != 0);
        // only add actions to the map if they exist
        if (action)
		    mTokenActionMap[newtokenID] = action;
	}

	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::executeTokenAction(const size_t tokenID)
	{
		TokenActionIterator action = mTokenActionMap.find(tokenID);

		if (action == mTokenActionMap.end())
		{
			// BAD command. BAD!
			logParseError("Unrecognised compositor script command action");
			return;
		}
		else
		{
			try
			{
				(this->*action->second)();
			}
			catch (Exception& ogreException)
			{
				// an unknown token found or BNF Grammer rule was not successful
				// in finding a valid terminal token to complete the rule expression.
				logParseError(ogreException.getDescription());
			}
		}
	}

	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::logParseError(const String& error)
	{
		// log material name only if filename not specified
		if (mSourceName.empty() && !mScriptContext.compositor.isNull())
		{
			LogManager::getSingleton().logMessage(
				"Error in compositor " + mScriptContext.compositor->getName() +
				" : " + error);
		}
		else
		{
			if (!mScriptContext.compositor.isNull())
			{
				LogManager::getSingleton().logMessage(
					"Error in compositor " + mScriptContext.compositor->getName() +
					" at line " + StringConverter::toString(mCurrentLine) +
					" of " + mSourceName + ": " + error);
			}
			else
			{
				LogManager::getSingleton().logMessage(
					"Error at line " + StringConverter::toString(mCurrentLine) +
					" of " + mSourceName + ": " + error);
			}
		}
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseOpenBrace(void)
	{

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseCloseBrace(void)
	{
		switch(mScriptContext.section)
		{
		case CSS_NONE:
			logParseError("Unexpected terminating brace.");
			break;
		case CSS_COMPOSITOR:
			// End of compositor
			mScriptContext.section = CSS_NONE;
			mScriptContext.compositor.setNull();
			break;
		case CSS_TECHNIQUE:
			// End of technique
			mScriptContext.section = CSS_COMPOSITOR;
			mScriptContext.technique = NULL;
			break;
		case CSS_TARGET:
			// End of target
			mScriptContext.section = CSS_TECHNIQUE;
			mScriptContext.target = NULL;
			break;
		case CSS_PASS:
			// End of pass
			mScriptContext.section = CSS_TARGET;
			mScriptContext.pass = NULL;
			break;
		};
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseCompositor(void)
	{
		const String compositorName = getNextTokenLabel();
		mScriptContext.compositor = CompositorManager::getSingleton().create(
            compositorName, mScriptContext.groupName
			);
		mScriptContext.section = CSS_COMPOSITOR;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTechnique(void)
	{
		mScriptContext.technique = mScriptContext.compositor->createTechnique();
		mScriptContext.section = CSS_TECHNIQUE;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTexture(void)
	{
	    assert(mScriptContext.technique);
		const String textureName = getNextTokenLabel();
        CompositionTechnique::TextureDefinition* textureDef = mScriptContext.technique->createTextureDefinition(textureName);
        // if peek next token is target_width then get token and use 0 for width
        // determine width parameter
		if (testNextTokenID(ID_TARGET_WIDTH_SCALED))
		{
			getNextToken();
			// a value of zero causes texture to be size of render target
			textureDef->width = 0;
			// get factor from next token
			textureDef->widthFactor = static_cast<float>(getNextTokenValue());

		}
        else if (testNextTokenID(ID_TARGET_WIDTH))
        {
            getNextToken();
            // a value of zero causes texture to be size of render target
            textureDef->width = 0;
			textureDef->widthFactor = 1.0f;
        }
        else
        {
            textureDef->width = static_cast<size_t>(getNextTokenValue());
        }
        // determine height parameter
		if (testNextTokenID(ID_TARGET_HEIGHT_SCALED))
		{
			getNextToken();
			// a value of zero causes texture to be dependent on render target
			textureDef->height = 0;
			// get factor from next token
			textureDef->heightFactor = static_cast<float>(getNextTokenValue());

		}
        else if (testNextTokenID(ID_TARGET_HEIGHT))
        {
            getNextToken();
            // a value of zero causes texture to be size of render target
            textureDef->height = 0;
			textureDef->heightFactor = 1.0f;
        }
        else
        {
            textureDef->height = static_cast<size_t>(getNextTokenValue());
        }
        // get pixel factor & shared option
		while (getRemainingTokensForAction() > 0)
		{
			switch (getNextTokenID())
			{
			case ID_PF_A8R8G8B8:
				textureDef->formatList.push_back(PF_A8R8G8B8);
				break;

			case ID_PF_R8G8B8A8:
				textureDef->formatList.push_back(PF_R8G8B8A8);
				break;
			case ID_PF_R8G8B8:
				textureDef->formatList.push_back(PF_R8G8B8);
				break;
			case ID_PF_FLOAT16_R:
				textureDef->formatList.push_back(PF_FLOAT16_R);
				break;
			case ID_PF_FLOAT16_GR:
				textureDef->formatList.push_back(PF_FLOAT16_GR);
				break;
			case ID_PF_FLOAT16_RGB:
				textureDef->formatList.push_back(PF_FLOAT16_RGB);
				break;
			case ID_PF_FLOAT16_RGBA:
				textureDef->formatList.push_back(PF_FLOAT16_RGBA);
				break;
			case ID_PF_FLOAT32_R:
				textureDef->formatList.push_back(PF_FLOAT32_R);
				break;
			case ID_PF_FLOAT32_GR:
				textureDef->formatList.push_back(PF_FLOAT32_GR);
				break;
			case ID_PF_FLOAT32_RGB:
				textureDef->formatList.push_back(PF_FLOAT32_RGB);
				break;
			case ID_PF_FLOAT32_RGBA:
				textureDef->formatList.push_back(PF_FLOAT32_RGBA);
				break;
			case ID_POOLED:
				textureDef->pooled = true;
				break;
			case ID_GAMMA:
				textureDef->hwGammaWrite = true;
				break;
			case ID_NO_FSAA:
				textureDef->fsaa = false;
				break;
			case ID_SCOPE_LOCAL:
				textureDef->scope = CompositionTechnique::TS_LOCAL;
				break;
			case ID_SCOPE_CHAIN:
				textureDef->scope = CompositionTechnique::TS_CHAIN;
				break;
			case ID_SCOPE_GLOBAL:
				textureDef->scope = CompositionTechnique::TS_GLOBAL;
				break;
			default:
				// should never get here?
				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTextureRef(void)
	{
	    assert(mScriptContext.technique);
		const String textureName = getNextTokenLabel();
		const String compositorName = getNextTokenLabel();
		const String compositorTextureName = getNextTokenLabel();

        CompositionTechnique::TextureDefinition* textureDef = mScriptContext.technique->createTextureDefinition(textureName);
		textureDef->refCompName = compositorName;
		textureDef->refTexName = compositorTextureName;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseCompositorLogic(void)
	{
	    assert(mScriptContext.technique);
		const String compositorLogicName = getNextTokenLabel();

		mScriptContext.technique->setCompositorLogicName(compositorLogicName);
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseScheme(void)
	{
		assert(mScriptContext.technique);
		const String schemeName = getNextTokenLabel();

		mScriptContext.technique->setSchemeName(schemeName);
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTarget(void)
	{
        assert(mScriptContext.technique);

		mScriptContext.section = CSS_TARGET;
        mScriptContext.target = mScriptContext.technique->createTargetPass();
        mScriptContext.target->setOutputName(getNextTokenLabel());

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseInput(void)
	{
		// input parameters depends on context either target or pass
		if (mScriptContext.section == CSS_TARGET)
		{
		    // for input in target, there is only one parameter
		    assert(mScriptContext.target);
		    if (testNextTokenID(ID_PREVIOUS))
                mScriptContext.target->setInputMode(CompositionTargetPass::IM_PREVIOUS);
            else
                mScriptContext.target->setInputMode(CompositionTargetPass::IM_NONE);
		}
		else // assume for pass section context
		{
		    // for input in pass, there are two parameters
		    assert(mScriptContext.pass);
		    uint32 id = static_cast<uint32>(getNextTokenValue());
		    const String& textureName = getNextTokenLabel();
			// MRT index?
			size_t mrtIndex = 0;
			if (getRemainingTokensForAction() > 0)
			{
				mrtIndex = static_cast<size_t>(getNextTokenValue()); 
			}
		    mScriptContext.pass->setInput(id, textureName, mrtIndex);
		}

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTargetOutput(void)
	{
		assert(mScriptContext.technique);
		mScriptContext.target = mScriptContext.technique->getOutputTargetPass();
		mScriptContext.section = CSS_TARGET;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseOnlyInitial(void)
	{
        assert(mScriptContext.target);
        mScriptContext.target->setOnlyInitial(testNextTokenID(ID_ON));
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseVisibilityMask(void)
	{
        assert(mScriptContext.target);
        mScriptContext.target->setVisibilityMask(static_cast<uint32>(getNextTokenValue()));
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseLodBias(void)
	{
        assert(mScriptContext.target);
        mScriptContext.target->setLodBias(getNextTokenValue());
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseMaterialScheme(void)
	{
		assert(mScriptContext.target);
		mScriptContext.target->setMaterialScheme(getNextTokenLabel());
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseShadowsEnabled(void)
	{
		assert(mScriptContext.target);
		mScriptContext.target->setShadowsEnabled(testNextTokenID(ID_ON));
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parsePass(void)
	{
		assert(mScriptContext.target);
        mScriptContext.pass = mScriptContext.target->createPass();
        CompositionPass::PassType passType = CompositionPass::PT_RENDERQUAD;
        switch (getNextTokenID())
        {
        case ID_RENDER_QUAD:
            passType = CompositionPass::PT_RENDERQUAD;
            break;

        case ID_CLEAR:
            passType = CompositionPass::PT_CLEAR;
            break;

        case ID_STENCIL:
            passType = CompositionPass::PT_STENCIL;
            break;

        case ID_RENDER_SCENE:
            passType = CompositionPass::PT_RENDERSCENE;
            break;

		case ID_RENDER_CUSTOM:
			passType = CompositionPass::PT_RENDERCUSTOM;
			mScriptContext.pass->setCustomType(getNextTokenLabel());
			break;

        default:
            break;
        }

        mScriptContext.pass->setType(passType);

		mScriptContext.section = CSS_PASS;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseMaterial(void)
	{
		assert(mScriptContext.pass);
        mScriptContext.pass->setMaterialName(getNextTokenLabel());
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseFirstRenderQueue(void)
	{
		assert(mScriptContext.pass);
		mScriptContext.pass->setFirstRenderQueue(static_cast<uint8>(getNextTokenValue()));
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseLastRenderQueue(void)
	{
		assert(mScriptContext.pass);
		mScriptContext.pass->setLastRenderQueue(static_cast<uint8>(getNextTokenValue()));
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseIdentifier(void)
	{
		assert(mScriptContext.pass);
		mScriptContext.pass->setIdentifier(static_cast<uint32>(getNextTokenValue()));
	}
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseClearBuffers(void)
    {
		assert(mScriptContext.pass);
		// while there are tokens for the action, get next token and set buffer flag
		uint32 bufferFlags = 0;

		while (getRemainingTokensForAction() > 0)
		{
		    switch (getNextTokenID())
		    {
            case ID_CLR_COLOUR:
                bufferFlags |= FBT_COLOUR;
                break;

            case ID_CLR_DEPTH:
                bufferFlags |= FBT_DEPTH;
                break;

            case ID_STENCIL:
                bufferFlags |= FBT_STENCIL;
                break;

            default:
                break;
		    }
		}
		mScriptContext.pass->setClearBuffers(bufferFlags);
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseClearColourValue(void)
    {
		assert(mScriptContext.pass);
		Real red = getNextTokenValue();
		Real green = getNextTokenValue();
		Real blue = getNextTokenValue();
		Real alpha = getNextTokenValue();
		mScriptContext.pass->setClearColour(ColourValue(red, green, blue, alpha));
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseClearDepthValue(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setClearDepth(getNextTokenValue());
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseClearStencilValue(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setClearStencil(static_cast<uint32>(getNextTokenValue()));
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilCheck(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilCheck(testNextTokenID(ID_ON));
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilFunc(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilFunc(extractCompareFunc());
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilRefVal(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilRefValue(static_cast<uint32>(getNextTokenValue()));
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilMask(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilMask(static_cast<uint32>(getNextTokenValue()));
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilFailOp(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilFailOp(extractStencilOp());
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilDepthFailOp(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilDepthFailOp(extractStencilOp());
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilPassOp(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilPassOp(extractStencilOp());
    }
	//-----------------------------------------------------------------------
    void CompositorScriptCompiler::parseStencilTwoSided(void)
    {
		assert(mScriptContext.pass);
		mScriptContext.pass->setStencilTwoSidedOperation(testNextTokenID(ID_ON));
    }
	//-----------------------------------------------------------------------
	StencilOperation CompositorScriptCompiler::extractStencilOp(void)
	{
	    StencilOperation sop = SOP_KEEP;

        switch (getNextTokenID())
        {
        case ID_ST_KEEP:
            sop = SOP_KEEP;
            break;

        case ID_ST_ZERO:
            sop = SOP_ZERO;
            break;

        case ID_ST_REPLACE:
            sop = SOP_REPLACE;
            break;

        case ID_ST_INCREMENT:
            sop = SOP_INCREMENT;
            break;

        case ID_ST_DECREMENT:
            sop = SOP_DECREMENT;
            break;

        case ID_ST_INCREMENT_WRAP:
            sop = SOP_INCREMENT_WRAP;
            break;

        case ID_ST_DECREMENT_WRAP:
            sop = SOP_DECREMENT_WRAP;
            break;

        case ID_ST_INVERT:
            sop = SOP_INVERT;
            break;

        default:
            break;
        }

        return sop;
	}
    CompareFunction CompositorScriptCompiler::extractCompareFunc(void)
	{
	    CompareFunction compFunc = CMPF_ALWAYS_PASS;

        switch (getNextTokenID())
        {
        case ID_ST_ALWAYS_FAIL:
            compFunc = CMPF_ALWAYS_FAIL;
            break;

        case ID_ST_ALWAYS_PASS:
            compFunc = CMPF_ALWAYS_PASS;
            break;

        case ID_ST_LESS:
            compFunc = CMPF_LESS;
            break;

        case ID_ST_LESS_EQUAL:
            compFunc = CMPF_LESS_EQUAL;
            break;

        case ID_ST_EQUAL:
            compFunc = CMPF_EQUAL;
            break;

        case ID_ST_NOT_EQUAL:
            compFunc = CMPF_NOT_EQUAL;
            break;

        case ID_ST_GREATER_EQUAL:
            compFunc = CMPF_GREATER_EQUAL;
            break;

        case ID_ST_GREATER:
            compFunc = CMPF_GREATER;
            break;

        default:
            break;
        }

        return compFunc;
	}

}
