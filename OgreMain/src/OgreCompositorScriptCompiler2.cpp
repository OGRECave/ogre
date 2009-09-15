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
#include "OgreCompositorManager.h"
#include "OgreCompositorScriptCompiler2.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"


namespace Ogre
{

	CompositorScriptCompilerListener::CompositorScriptCompilerListener()
	{
	}

	Compositor *CompositorScriptCompilerListener::getCompositor(const String &name, const String &group)
	{
		return (Compositor*)CompositorManager::getSingleton().create(name, group).get();
	}

	bool CompositorScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, CompositorScriptCompiler2*)
	{
		return false;
	}

	CompositorScriptCompiler2::CompositorScriptCompiler2()
		:mListener(0), mCompositor(0)
	{
		mWordIDs["compositor"] = ID_COMPOSITOR;
		mWordIDs["technique"] = ID_TECHNIQUE;
		mWordIDs["target"] = ID_TARGET;
		mWordIDs["target_output"] = ID_TARGET_OUTPUT;
		mWordIDs["pass"] = ID_PASS;

		mWordIDs["texture"] = ID_TEXTURE;
		mWordIDs["input"] = ID_INPUT;
			mWordIDs["none"] = ID_NONE;
			mWordIDs["previous"] = ID_PREVIOUS;
			mWordIDs["target_width"] = ID_TARGET_WIDTH;
			mWordIDs["target_height"] = ID_TARGET_HEIGHT;
		mWordIDs["only_initial"] = ID_ONLY_INITIAL;
		mWordIDs["visibility_mask"] = ID_VISIBILITY_MASK;
		mWordIDs["lod_bias"] = ID_LOD_BIAS;
		mWordIDs["material_scheme"] = ID_MATERIAL_SCHEME;

		mWordIDs["clear"] = ID_CLEAR;
		mWordIDs["stencil"] = ID_STENCIL;
		mWordIDs["render_scene"] = ID_RENDER_SCENE;
		mWordIDs["render_quad"] = ID_RENDER_QUAD;
		mWordIDs["material"] = ID_MATERIAL;
		mWordIDs["identifier"] = ID_IDENTIFIER;
		mWordIDs["first_render_queue"] = ID_FIRST_RENDER_QUEUE;
		mWordIDs["last_render_queue"] = ID_LAST_RENDER_QUEUE;

		mWordIDs["buffers"] = ID_BUFFERS;
			mWordIDs["colour"] = ID_COLOUR;
			mWordIDs["depth"] = ID_DEPTH;
		mWordIDs["colour_value"] = ID_COLOUR_VALUE;
		mWordIDs["depth_value"] = ID_DEPTH_VALUE;
		mWordIDs["stencil_value"] = ID_STENCIL_VALUE;

		mWordIDs["check"] = ID_CHECK;
		mWordIDs["comp_func"] = ID_COMP_FUNC;
			mWordIDs["always_fail"] = ID_ALWAYS_FAIL;
			mWordIDs["always_pass"] = ID_ALWAYS_PASS;
			mWordIDs["less_equal"] = ID_LESS_EQUAL;
			mWordIDs["less"] = ID_LESS;
			mWordIDs["equal"] = ID_EQUAL;
			mWordIDs["not_equal"] = ID_NOT_EQUAL;
			mWordIDs["greater_equal"] = ID_GREATER_EQUAL;
			mWordIDs["greater"] = ID_GREATER;
		mWordIDs["ref_value"] = ID_REF_VALUE;
		mWordIDs["mask"] = ID_MASK;
		mWordIDs["fail_op"] = ID_FAIL_OP;
			mWordIDs["keep"] = ID_KEEP;
			mWordIDs["zero"] = ID_ZERO;
			mWordIDs["replace"] = ID_REPLACE;
			mWordIDs["increment"] = ID_INCREMENT;
			mWordIDs["decrement"] = ID_DECREMENT;
			mWordIDs["increment_wrap"] = ID_INCREMENT_WRAP;
			mWordIDs["decrement_wrap"] = ID_DECREMENT_WRAP;
			mWordIDs["invert"] = ID_INVERT;
		mWordIDs["depth_fail_op"] = ID_DEPTH_FAIL_OP;
		mWordIDs["pass_op"] = ID_PASS_OP;
		mWordIDs["two_sided"] = ID_TWO_SIDED;
	}

	void CompositorScriptCompiler2::setListener(CompositorScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	bool CompositorScriptCompiler2::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			if(!processNode(i, nodes->end()))
			{
				if((*i)->wordID == ID_COMPOSITOR)
				{
					compileCompositor(*i);
				}
				++i;
			}
		}
		return mErrors.empty();
	}

	bool CompositorScriptCompiler2::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		return mListener ? mListener->processNode(i, end, this) : false;
	}

	ScriptNodeListPtr CompositorScriptCompiler2::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		// If we got any AST loaded, do the necessary pre-processing steps
		if(!nodes.isNull())
		{
			// Expand all imports
			processImports(nodes);
		}

		return nodes;
	}

	void CompositorScriptCompiler2::preParse()
	{
		if(mListener)
			mListener->preParse(mWordIDs);
	}

	bool CompositorScriptCompiler2::errorRaised(const ScriptCompilerErrorPtr &error)
	{
		return mListener ? mListener->errorRaised(error) : true;
	}

	void CompositorScriptCompiler2::compileCompositor(const ScriptNodePtr &node)
	{
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// Anything up until the '{' is put together to form the compositor name
		ScriptNodeList::iterator i = node->children.begin();
		String name = (*i)->token;
		++i;
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			name = name + " " + (*i)->token;
			++i;
		}

		// Verify we reached the '{'
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		if(mListener)
			mCompositor = mListener->getCompositor(name, mGroup);
		else
			mCompositor = (Compositor*)CompositorManager::getSingleton().create(name, mGroup).get();

		if(!mCompositor)
		{
			addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				// Only techniques are supported here
				if((*j)->wordID == ID_TECHNIQUE)
				{
					compileTechnique(*j);
				}
				++j;
			}
		}

		mCompositor = 0;
	}

	void CompositorScriptCompiler2::compileTechnique(const Ogre::ScriptNodePtr &node)
	{
		// The technique immediately starts with a '{'
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		CompositionTechnique *technique = mCompositor->createTechnique();

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_TEXTURE)
				{
					if((*j)->children.size() >= 4)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
							node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);

						if(node2->wordID != ID_TARGET_WIDTH && node2->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							goto fail;
						}
						if(node3->wordID != ID_TARGET_HEIGHT && node3->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							goto fail;
						}

						PixelFormat format = PixelUtil::getFormatFromName(node4->token, true);
						CompositionTechnique::TextureDefinition *def = 
							technique->createTextureDefinition(node1->token);
						def->width = node2->wordID == ID_TARGET_WIDTH ? 0 : node2->data;
						def->height = node3->wordID == ID_TARGET_HEIGHT ? 0 : node3->data;
						def->formatList.push_back(format);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TARGET)
				{
					compileTarget(*j, technique);
				}
				else if((*j)->wordID == ID_TARGET_OUTPUT)
				{
					compileTargetOutput(*j, technique);
				}
fail:
				++j;
			}
		}
	}

	void CompositorScriptCompiler2::compileTarget(const Ogre::ScriptNodePtr &node, Ogre::CompositionTechnique *technique)
	{
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// Anything up until the '{' is put together to form the compositor name
		ScriptNodeList::iterator i = node->children.begin();
		String name = (*i)->token;
		++i;
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			name = name + " " + (*i)->token;
			++i;
		}

		// Verify we reached the '{'
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Create the target
		CompositionTargetPass *target = technique->createTargetPass();
		target->setOutputName(name);

		compileTargetOptions((*i)->children.begin(), (*i)->children.end(), target);		
	}

	void CompositorScriptCompiler2::compileTargetOutput(const Ogre::ScriptNodePtr &node, Ogre::CompositionTechnique *technique)
	{
		// The target_output immediately starts with a '{'
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		compileTargetOptions((*i)->children.begin(), (*i)->children.end(), technique->getOutputTargetPass());
	}

	void CompositorScriptCompiler2::compilePass(const Ogre::ScriptNodePtr &node, Ogre::CompositionTargetPass *target)
	{
		// The pass starts out with a type
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		CompositionPass::PassType type;
		switch((*i)->wordID)
		{
		case ID_CLEAR:
			type = CompositionPass::PT_CLEAR;
			break;
		case ID_STENCIL:
			type = CompositionPass::PT_STENCIL;
			break;
		case ID_RENDER_SCENE:
			type = CompositionPass::PT_RENDERSCENE;
			break;
		case ID_RENDER_QUAD:
			type = CompositionPass::PT_RENDERQUAD;
			break;
		default:
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Search for the '{'
		i = findNode(i, node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		CompositionPass *pass = target->createPass();
		pass->setType(type);

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_MATERIAL)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					pass->setMaterialName((*j)->children.front()->token);
				}
				else if((*j)->wordID == ID_INPUT)
				{
					if((*j)->children.size() >= 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
						if(node1->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							goto fail;
						}
						if(!node3.isNull() && node3->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							goto fail;
						}

						pass->setInput(node1->data, node2->token, node3.isNull() ? 0 : node3->data);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_IDENTIFIER)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setIdentifier(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_FIRST_RENDER_QUEUE)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setFirstRenderQueue(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_LAST_RENDER_QUEUE)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setLastRenderQueue(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_CLEAR)
				{
					ScriptNodeList::iterator k = findNode((*j)->children.begin(), (*j)->children.end(), SNT_LBRACE);
					if(k == (*j)->children.end())
					{
						addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					while(k != (*j)->children.end())
					{
						if(!processNode(k, (*j)->children.end()))
						{
							if((*k)->wordID == ID_BUFFERS)
							{
								if((*k)->children.empty())
								{
									addError(CE_STRINGEXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								uint32 buffers = 0;
								for(ScriptNodeList::iterator m = (*j)->children.begin(); m != (*j)->children.end(); ++m)
								{
									switch((*m)->wordID)
									{
									case ID_COLOUR:
										buffers |= FBT_COLOUR;
										break;
									case ID_DEPTH:
										buffers |= FBT_DEPTH;
										break;
									case ID_STENCIL:
										buffers |= FBT_STENCIL;
										break;
									default:
										addError(CE_INVALIDPROPERTYVALUE, (*m)->file, (*m)->line, (*m)->column);
									}
								}

								pass->setClearBuffers(buffers);
							}
							else if((*k)->wordID == ID_COLOUR_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_STRINGEXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ColourValue c;
								if(getColourValue((*k)->children.begin(), (*k)->children.end(), c))
									pass->setClearColour(c);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
							}
							else if((*k)->wordID == ID_DEPTH_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setClearDepth(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_STENCIL_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setClearStencil(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
clear_fail:
							++k;
						}
					}
				}
				else if((*j)->wordID == ID_STENCIL)
				{
					ScriptNodeList::iterator k = findNode((*j)->children.begin(), (*j)->children.end(), SNT_LBRACE);
					if(k == (*j)->children.end())
					{
						addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					while(k != (*j)->children.end())
					{
						if(!processNode(k, (*j)->children.end()))
						{
							if((*k)->wordID == ID_CHECK)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								bool val = true;
								ScriptNodePtr node = (*k)->children.front();
								if(getTruthValue(node, val))
									pass->setStencilCheck(val);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_COMP_FUNC)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								CompareFunction func;
								ScriptNodePtr node = (*k)->children.front();
								if(getCompareFunction(node, func))
									pass->setStencilFunc(func);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_REF_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setStencilRefValue(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_MASK)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setStencilMask(StringConverter::parseUnsignedInt(node->token));
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_FAIL_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilFailOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_DEPTH_FAIL_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilDepthFailOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_PASS_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilPassOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							if((*k)->wordID == ID_TWO_SIDED)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								bool val = true;
								ScriptNodePtr node = (*k)->children.front();
								if(getTruthValue(node, val))
									pass->setStencilTwoSidedOperation(val);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
stencil_fail:
							++k;
						}
					}
				}
fail:
				++j;
			}
		}
	}

	void CompositorScriptCompiler2::compileTargetOptions(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::CompositionTargetPass *target)
	{
		while(i != end)
		{
			if(!processNode(i, end))
			{
				if((*i)->wordID == ID_INPUT)
				{
					if((*i)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}
					ScriptNodePtr node = (*i)->children.front();
					switch(node->wordID)
					{
					case ID_NONE:
						target->setInputMode(CompositionTargetPass::IM_NONE);
						break;
					case ID_PREVIOUS:
						target->setInputMode(CompositionTargetPass::IM_PREVIOUS);
						break;
					default:
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						goto fail;
					}
				}
				else if((*i)->wordID == ID_ONLY_INITIAL)
				{
					if((*i)->children.empty())
					{
						addError(CE_TRUTHVALUEEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					bool val = false;
					ScriptNodePtr node = (*i)->children.front();
					if(getTruthValue(node, val))
						target->setOnlyInitial(val);
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_VISIBILITY_MASK)
				{
					if((*i)->children.empty())
					{
						addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					// Notice we don't use data here!
					// Just in case we do a conversion straight to an integer because we could lose
					// large numbers!
					ScriptNodePtr node = (*i)->children.front();
					if(node->type == SNT_NUMBER)
						target->setVisibilityMask(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_LOD_BIAS)
				{
					if((*i)->children.empty())
					{
						addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*i)->children.front();
					if(node->type == SNT_NUMBER)
						target->setLodBias(node->data);
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_MATERIAL_SCHEME)
				{
					if((*i)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*i)->children.front();
					target->setMaterialScheme(node->token);
				}
				else if((*i)->wordID == ID_PASS)
				{
					compilePass(*i, target);
				}
fail:
				++i;
			}
		}
	}

	bool CompositorScriptCompiler2::getCompareFunction(const ScriptNodePtr &node, CompareFunction &func)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_ALWAYS_FAIL:
			func = Ogre::CMPF_ALWAYS_FAIL;
			break;
		case ID_ALWAYS_PASS:
			func = CMPF_ALWAYS_PASS;
			break;
		case ID_LESS:
			func = CMPF_LESS;
			break;
		case ID_LESS_EQUAL:
			func = CMPF_LESS_EQUAL;
			break;
		case ID_EQUAL:
			func = CMPF_EQUAL;
			break;
		case ID_NOT_EQUAL:
			func = CMPF_NOT_EQUAL;
			break;
		case ID_GREATER_EQUAL:
			func = CMPF_GREATER_EQUAL;
			break;
		case ID_GREATER:
			func = CMPF_GREATER;
			break;
		default:
			success = false;
		}
		return success;
	}

	bool CompositorScriptCompiler2::getStencilOp(const ScriptNodePtr &node, StencilOperation &op)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_KEEP:
			op = SOP_KEEP;
			break;
		case ID_ZERO:
			op = SOP_ZERO;
			break;
		case ID_REPLACE:
			op = SOP_REPLACE;
			break;
		case ID_INCREMENT:
			op = SOP_INCREMENT;
			break;
		case ID_DECREMENT:
			op = SOP_DECREMENT;
			break;
		case ID_INCREMENT_WRAP:
			op = SOP_INCREMENT_WRAP;
			break;
		case ID_DECREMENT_WRAP:
			op = SOP_DECREMENT_WRAP;
			break;
		case ID_INVERT:
			op = SOP_INVERT;
			break;
		default:
			success = false;
		}
		return success;
	}

	bool CompositorScriptCompiler2::getColourValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c)
	{
		if(i == end)
			return false;

		if((*i)->type == SNT_NUMBER)
		{
			c.r = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.g = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.b = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.a = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		return true;
	}

}

