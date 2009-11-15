/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreScriptTranslator.h"
#include "OgreScriptCompiler.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"

namespace Ogre{
	
	GpuProgramType translateIDToGpuProgramType(uint32 id)
	{
		switch (id)
		{
			case ID_VERTEX_PROGRAM:
			default:
				return GPT_VERTEX_PROGRAM;
			case ID_GEOMETRY_PROGRAM:
				return GPT_GEOMETRY_PROGRAM;
			case ID_FRAGMENT_PROGRAM:
				return GPT_FRAGMENT_PROGRAM;
		}
	}
	
	void ScriptTranslator::processNode(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		if(node->type != ANT_OBJECT)
			return;

		// Abstract objects are completely skipped
		if((reinterpret_cast<ObjectAbstractNode*>(node.get()))->abstract)
			return;

		// Retrieve the translator to use
		ScriptTranslator *translator = 
			ScriptCompilerManager::getSingleton().getTranslator(node);
		
		if(translator)
			translator->translate(compiler, node);
		else
			compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, node->file, node->line,
				"token \"" + reinterpret_cast<ObjectAbstractNode*>(node.get())->cls + "\" is not recognized");
	}
	//-------------------------------------------------------------------------
	AbstractNodeList::const_iterator ScriptTranslator::getNodeAt(const AbstractNodeList &nodes, int index)
	{
		AbstractNodeList::const_iterator i = nodes.begin();
		int n = 0;
		while(i != nodes.end())
		{
			if(n == index)
				return i;
			++i;
			++n;
		}
		return nodes.end();
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getBoolean(const AbstractNodePtr &node, bool *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(atom->id != 1 && atom->id != 2)
			return false;
		
		*result = atom->id == 1 ? true : false;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getString(const AbstractNodePtr &node, String *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();		
		*result = atom->value;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getReal(const Ogre::AbstractNodePtr &node, Ogre::Real *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(!Ogre::StringConverter::isNumber(atom->value))
			return false;
		StringStream stream;
		stream << atom->value;
		stream >> *result;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getFloat(const Ogre::AbstractNodePtr &node, float *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(!Ogre::StringConverter::isNumber(atom->value))
			return false;
		StringStream stream;
		stream << atom->value;
		stream >> *result;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getInt(const Ogre::AbstractNodePtr &node, int *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(!Ogre::StringConverter::isNumber(atom->value))
			return false;
		StringStream stream;
		stream << atom->value;
		stream >> *result;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getUInt(const Ogre::AbstractNodePtr &node, uint32 *result)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(!Ogre::StringConverter::isNumber(atom->value))
			return false;
		StringStream stream;
		stream << atom->value;
		stream >> *result;
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result, int maxEntries)
	{
		int n = 0;
		while(i != end && n < maxEntries)
		{
			float v = 0;
			if(getFloat(*i, &v))
			{
				switch(n)
				{
				case 0:
					result->r = v;
					break;
				case 1:
					result->g = v;
					break;
				case 2:
					result->b = v;
					break;
				case 3:
					result->a = v;
					break;
				}
			}
			else
			{
				return false;
			}
			++n;
			++i;
		}
		// return error if we found less than rgb before end, unless constrained
		return (n >= 3 || n == maxEntries);
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getSceneBlendFactor(const Ogre::AbstractNodePtr &node, Ogre::SceneBlendFactor *sbf)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_ONE:
			*sbf = SBF_ONE;
			break;
		case ID_ZERO:
			*sbf = SBF_ZERO;
			break;
		case ID_DEST_COLOUR:
			*sbf = SBF_DEST_COLOUR;
			break;
		case ID_DEST_ALPHA:
			*sbf = SBF_DEST_ALPHA;
			break;
		case ID_SRC_ALPHA:
			*sbf = SBF_SOURCE_ALPHA;
			break;
		case ID_SRC_COLOUR:
			*sbf = SBF_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_COLOUR:
			*sbf = SBF_ONE_MINUS_DEST_COLOUR;
			break;
		case ID_ONE_MINUS_SRC_COLOUR:
			*sbf = SBF_ONE_MINUS_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_ALPHA:
			*sbf = SBF_ONE_MINUS_DEST_ALPHA;
			break;
		case ID_ONE_MINUS_SRC_ALPHA:
			*sbf = SBF_ONE_MINUS_SOURCE_ALPHA;
			break;
		default:
			return false;
		}
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getCompareFunction(const AbstractNodePtr &node, CompareFunction *func)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_ALWAYS_FAIL:
			*func = CMPF_ALWAYS_FAIL;
			break;
		case ID_ALWAYS_PASS:
			*func = CMPF_ALWAYS_PASS;
			break;
		case ID_LESS:
			*func = CMPF_LESS;
			break;
		case ID_LESS_EQUAL:
			*func = CMPF_LESS_EQUAL;
			break;
		case ID_EQUAL:
			*func = CMPF_EQUAL;
			break;
		case ID_NOT_EQUAL:
			*func = CMPF_NOT_EQUAL;
			break;
		case ID_GREATER_EQUAL:
			*func = CMPF_GREATER_EQUAL;
			break;
		case ID_GREATER:
			*func = CMPF_GREATER;
			break;
		default:
			return false;
		}
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m)
	{
		int n = 0;
		while(i != end && n < 16)
		{
			if(i != end)
			{
				Real r = 0;
				if(getReal(*i, &r))
					(*m)[n/4][n%4] = r;
				else
					return false;
			}
			else
			{
				return false;
			}
			++i;
			++n;
		}
		return true;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				int v = 0;
				if(getInt(*i, &v))
					vals[n] = v;
				else
					break;
				++i;
			}
			else
				vals[n] = 0;
			++n;
		}

		if(n < count)
			success = false;

		return success;
	}
	//----------------------------------------------------------------------------
	bool ScriptTranslator::getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				float v = 0;
				if(getFloat(*i, &v))
					vals[n] = v;
				else
					break;
				++i;
			}
			else
				vals[n] = 0;
			++n;
		}

		if(n < count)
			success = false;

		return success;
	}
	//-------------------------------------------------------------------------
	bool ScriptTranslator::getStencilOp(const Ogre::AbstractNodePtr &node, Ogre::StencilOperation *op)
	{
		if(node->type != ANT_ATOM)
			return false;
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_KEEP:
			*op = SOP_KEEP;
			break;
		case ID_ZERO:
			*op = SOP_ZERO;
			break;
		case ID_REPLACE:
			*op = SOP_REPLACE;
			break;
		case ID_INCREMENT:
			*op = SOP_INCREMENT;
			break;
		case ID_DECREMENT:
			*op = SOP_DECREMENT;
			break;
		case ID_INCREMENT_WRAP:
			*op = SOP_INCREMENT_WRAP;
			break;
		case ID_DECREMENT_WRAP:
			*op = SOP_DECREMENT_WRAP;
			break;
		case ID_INVERT:
			*op = SOP_INVERT;
			break;
		default:
			return false;
		}
		return true;
	}
	//---------------------------------------------------------------------
	bool ScriptTranslator::getConstantType(AbstractNodeList::const_iterator i, GpuConstantType *op)
	{

		String val;
		getString(*i, &val);
		if(val.find("float") != String::npos)
		{
			int count = 1;
			if (val.size() == 6)
				count = StringConverter::parseInt(val.substr(5));
			else if (val.size() > 6)
				return false;

			if (count > 4 || count == 0)
				return false;

			*op = (GpuConstantType)(GCT_FLOAT1 + count - 1);
		}
		else if(val.find("int") != String::npos)
		{
			int count = 1;
			if (val.size() == 4)
				count = StringConverter::parseInt(val.substr(3));
			else if (val.size() > 4)
				return false;

			if (count > 4 || count == 0)
				return false;

			*op = (GpuConstantType)(GCT_INT1 + count - 1);
		}
		else if(val.find("matrix") != String::npos)
		{
			int count1, count2;

			if (val.size() == 9)
			{
				count1 = StringConverter::parseInt(val.substr(6, 1));
				count2 = StringConverter::parseInt(val.substr(8, 1));
			}
			else 
				return false;

			if (count1 > 4 || count1 < 2 || count2 > 4 || count2 < 2)
				return false;

			switch(count1)
			{
			case 2:
				*op = (GpuConstantType)(GCT_MATRIX_2X2 + count2 - 1);
				break;
			case 3:
				*op = (GpuConstantType)(GCT_MATRIX_3X2 + count2 - 1);
				break;
			case 4:
				*op = (GpuConstantType)(GCT_MATRIX_4X2 + count2 - 1);
				break;
			}

		}

		return true;
	}

	/**************************************************************************
	 * MaterialTranslator
	 *************************************************************************/
	MaterialTranslator::MaterialTranslator()
		:mMaterial(0)
	{
	}
	//-------------------------------------------------------------------------
	void MaterialTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
		if(obj->name.empty())
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);

		// Create a material with the given name
		CreateMaterialScriptCompilerEvent evt(node->file, obj->name, compiler->getResourceGroup());
		bool processed = compiler->_fireEvent(&evt, (void*)&mMaterial);

		if(!processed)
		{
			mMaterial = reinterpret_cast<Ogre::Material*>(MaterialManager::getSingleton().create(obj->name, compiler->getResourceGroup()).get());
		}
		else
		{
			if(!mMaterial)
				compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, 
					"failed to find or create material \"" + obj->name + "\"");
		}

		mMaterial->removeAllTechniques();
		obj->context = Any(mMaterial);
		mMaterial->_notifyOrigin(obj->file);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_LOD_VALUES:
					{
						Material::LodValueList lods;
						for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
						{
							Real v = 0;
							if(getReal(*j, &v))
								lods.push_back(v);
							else
								compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
									"lod_values expects only numbers as arguments");
						}
						mMaterial->setLodLevels(lods);
					}
					break;
                case ID_LOD_DISTANCES:
                    {
                        // Set strategy to distance strategy
                        LodStrategy *strategy = DistanceLodStrategy::getSingletonPtr();
                        mMaterial->setLodStrategy(strategy);

                        // Read in lod distances
                        Material::LodValueList lods;
                        for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
                        {
                            Real v = 0;
                            if(getReal(*j, &v))
                                lods.push_back(v);
                            else
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                    "lod_values expects only numbers as arguments");
                        }
                        mMaterial->setLodLevels(lods);
                    }
                    break;
                case ID_LOD_STRATEGY:
                    if (prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if (prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "lod_strategy only supports 1 argument");
                    }
                    else
                    {
                        String strategyName;
                        bool result = getString(prop->values.front(), &strategyName);
                        if (result)
                        {
                            LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(strategyName);

                            result = (strategy != 0);

                            if (result)
                                mMaterial->setLodStrategy(strategy);
                        }
                        
                        if (!result)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "lod_strategy argument must be a valid lod strategy");
                        }
                    }
                    break;
				case ID_RECEIVE_SHADOWS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"receive_shadows only supports 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val))
							mMaterial->setReceiveShadows(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"receive_shadows argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
					}
					break;
				case ID_TRANSPARENCY_CASTS_SHADOWS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"transparency_casts_shadows only supports 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val))
							mMaterial->setTransparencyCastsShadows(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"transparency_casts_shadows argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
					}
					break;
				case ID_SET_TEXTURE_ALIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 3)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"set_texture_alias only supports 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						String name, value;
						if(getString(*i0, &name) && getString(*i1, &value))
							mTextureAliases.insert(std::make_pair(name, value));
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"set_texture_alias must have 2 string argument");
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
		}

		// Apply the texture aliases
		if(compiler->getListener())
		{
			PreApplyTextureAliasesScriptCompilerEvent evt(mMaterial, &mTextureAliases);
			compiler->_fireEvent(&evt, 0);
		}
		mMaterial->applyTextureAliases(mTextureAliases);
		mTextureAliases.clear();
	}
	
	/**************************************************************************
	 * TechniqueTranslator
	 *************************************************************************/
	TechniqueTranslator::TechniqueTranslator()
		:mTechnique(0)
	{
	}
	//-------------------------------------------------------------------------
	void TechniqueTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
		
		// Create the technique from the material
		Ogre::Material *material = Ogre::any_cast<Ogre::Material*>(obj->parent->context);
		mTechnique = material->createTechnique();
		obj->context = Any(mTechnique);

		// Get the name of the technique
		if(!obj->name.empty())
			mTechnique->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_SCHEME:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scheme only supports 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						String scheme;
						if(getString(*i0, &scheme))
							mTechnique->setSchemeName(scheme);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"scheme must have 1 string argument");
					}
					break;
				case ID_LOD_INDEX:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"lod_index only supports 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						uint32 v = 0;
						if(getUInt(*i0, &v))
							mTechnique->setLodIndex(static_cast<unsigned short>(v));
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"lod_index cannot accept argument \"" + (*i0)->getValue() + "\"");
					}
					break;
				case ID_SHADOW_CASTER_MATERIAL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"shadow_caster_material only accepts 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						String matName;
						if(getString(*i0, &matName))
						{
							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, matName);
							compiler->_fireEvent(&evt, 0);
							mTechnique->setShadowCasterMaterial(evt.mName); // Use the processed name
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"shadow_caster_material cannot accept argument \"" + (*i0)->getValue() + "\"");
					}
					break;
				case ID_SHADOW_RECEIVER_MATERIAL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"shadow_receiver_material only accepts 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						String matName;
						if(getString(*i0, &matName))
						{
							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, matName);
							compiler->_fireEvent(&evt, 0);
							mTechnique->setShadowReceiverMaterial(evt.mName);
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"shadow_receiver_material_name cannot accept argument \"" + (*i0)->getValue() + "\"");
					}
					break;
				case ID_GPU_VENDOR_RULE:
					if(prop->values.size() < 2)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"gpu_vendor_rule must have 2 arguments");
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"gpu_vendor_rule must have 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);

						Technique::GPUVendorRule rule;
						if ((*i0)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();
							if (atom0->id == ID_INCLUDE)
							{
								rule.includeOrExclude = Technique::INCLUDE;
							}
							else if (atom0->id == ID_EXCLUDE)
							{
								rule.includeOrExclude = Technique::EXCLUDE;
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"gpu_vendor_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
							}

							String vendor;
							if(!getString(*i1, &vendor))
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"gpu_vendor_rule cannot accept \"" + (*i1)->getValue() + "\" as second argument");

							rule.vendor = RenderSystemCapabilities::vendorFromString(vendor);

							if (rule.vendor != GPU_UNKNOWN)
							{
								mTechnique->addGPUVendorRule(rule);
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"gpu_vendor_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
						}

					}
					break;
				case ID_GPU_DEVICE_RULE:
					if(prop->values.size() < 2)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"gpu_device_rule must have at least 2 arguments");
					}
					else if(prop->values.size() > 3)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"gpu_device_rule must have at most 3 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);

						Technique::GPUDeviceNameRule rule;
						if ((*i0)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();
							if (atom0->id == ID_INCLUDE)
							{
								rule.includeOrExclude = Technique::INCLUDE;
							}
							else if (atom0->id == ID_EXCLUDE)
							{
								rule.includeOrExclude = Technique::EXCLUDE;
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"gpu_device_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
							}

							if(!getString(*i1, &rule.devicePattern))
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"gpu_device_rule cannot accept \"" + (*i1)->getValue() + "\" as second argument");

							if (prop->values.size() == 3)
							{
								AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
								if (!getBoolean(*i2, &rule.caseSensitive))
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										"gpu_device_rule third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
							}

							mTechnique->addGPUDeviceNameRule(rule);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"gpu_device_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
						}

					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
		}
	}

	/**************************************************************************
	 * PassTranslator
	 *************************************************************************/
	PassTranslator::PassTranslator()
		:mPass(0)
	{
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		Technique *technique = any_cast<Technique*>(obj->parent->context);
		mPass = technique->createPass();
		obj->context = Any(mPass);

		// Get the name of the technique
		if(!obj->name.empty())
			mPass->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_AMBIENT:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 4)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"ambient must have at most 4 parameters");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM && 
							((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
						{
							mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_AMBIENT);
						}
						else
						{
							ColourValue val = ColourValue::White;
							if(getColour(prop->values.begin(), prop->values.end(), &val))
								mPass->setAmbient(val);
							else
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"ambient requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
						}
					}
					break;
				case ID_DIFFUSE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 4)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"diffuse must have at most 4 arguments");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM && 
							((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
						{
							mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_DIFFUSE);
						}
						else
						{
							ColourValue val = ColourValue::White;
							if(getColour(prop->values.begin(), prop->values.end(), &val))
								mPass->setDiffuse(val);
							else
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"diffuse requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
						}
					}
					break;
				case ID_SPECULAR:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 5)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"specular must have at most 5 arguments");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM && 
							((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
						{
							mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_SPECULAR);

							if(prop->values.size() >= 2)
							{
								Real val = 0;
								if(getReal(prop->values.back(), &val))
									mPass->setShininess(val);
								else
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										"specular does not support \"" + prop->values.back()->getValue() + "\" as its second argument");
							}
						}
						else
						{
							AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
								i1 = getNodeAt(prop->values, 1),
								i2 = getNodeAt(prop->values, 2);
							ColourValue val(0.0f, 0.0f, 0.0f, 1.0f);
							if(getFloat(*i0, &val.r) && getFloat(*i1, &val.g) && getFloat(*i2, &val.b))
							{
								if(prop->values.size() == 4)
								{
									mPass->setSpecular(val);

									AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
									Real shininess = 0.0f;
									if(getReal(*i3, &shininess))
										mPass->setShininess(shininess);
									else
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"specular fourth argument must be a valid number for shininess attribute");
								}
								else if(prop->values.size() > 4)
								{
									AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
									if(!getFloat(*i3, &val.a))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"specular fourth argument must be a valid color component value");
									else
										mPass->setSpecular(val);
									
									AbstractNodeList::const_iterator i4 = getNodeAt(prop->values, 4);
									Real shininess = 0.0f;
									if(getReal(*i4, &shininess))
										mPass->setShininess(shininess);
									else
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"specular fourth argument must be a valid number for shininess attribute"); 
								}
								else
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"specular expects at least 4 arguments"); 

							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"specular must have first 3 arguments be a valid colour");
							}
						}
					}
					break;
				case ID_EMISSIVE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 4)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"emissive must have at most 4 arguments");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM && 
							((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
						{
							mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_EMISSIVE);
						}
						else
						{
							ColourValue val(0.0f, 0.0f, 0.0f, 1.0f);
							if(getColour(prop->values.begin(), prop->values.end(), &val))
								mPass->setSelfIllumination(val);
							else
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"emissive requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
						}
					}
					break;
				case ID_SCENE_BLEND:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scene_blend supports at most 2 arguments");
					}
					else if(prop->values.size() == 1)
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_ADD:
								mPass->setSceneBlending(SBT_ADD);
								break;
							case ID_MODULATE:
								mPass->setSceneBlending(SBT_MODULATE);
								break;
							case ID_COLOUR_BLEND:
								mPass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
								break;
							case ID_ALPHA_BLEND:
								mPass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"scene_blend does not support \"" + prop->values.front()->getValue() + "\" for argument 1");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"scene_blend does not support \"" + prop->values.front()->getValue() + "\" for argument 1");
						}
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						SceneBlendFactor sbf0, sbf1;
						if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
						{
							mPass->setSceneBlending(sbf0, sbf1);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"scene_blend does not support \"" + (*i0)->getValue() + "\" and \"" + (*i1)->getValue() + "\" as arguments");
						}				
					}
					break;
				case ID_SEPARATE_SCENE_BLEND:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() == 3)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"separate_scene_blend must have 2 or 4 arguments");
					}
					else if(prop->values.size() > 4)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"separate_scene_blend must have 2 or 4 arguments");
					}
					else if(prop->values.size() == 2)
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
							SceneBlendType sbt0, sbt1;
							switch(atom0->id)
							{
							case ID_ADD:
								sbt0 = SBT_ADD;
								break;
							case ID_MODULATE:
								sbt0 = SBT_MODULATE;
								break;
							case ID_COLOUR_BLEND:
								sbt0 = SBT_TRANSPARENT_COLOUR;
								break;
							case ID_ALPHA_BLEND:
								sbt0 = SBT_TRANSPARENT_ALPHA;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"separate_scene_blend does not support \"" + atom0->value + "\" as argument 1");
								return;
							}
							
							switch(atom1->id)
							{
							case ID_ADD:
								sbt1 = SBT_ADD;
								break;
							case ID_MODULATE:
								sbt1 = SBT_MODULATE;
								break;
							case ID_COLOUR_BLEND:
								sbt1 = SBT_TRANSPARENT_COLOUR;
								break;
							case ID_ALPHA_BLEND:
								sbt1 = SBT_TRANSPARENT_ALPHA;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"separate_scene_blend does not support \"" + atom1->value + "\" as argument 2");
								return;
							}

							mPass->setSeparateSceneBlending(sbt0, sbt1);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"separate_scene_blend does not support \"" + (*i0)->getValue() + "\" as argument 1");
						}
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM)
						{
							SceneBlendFactor sbf0, sbf1, sbf2, sbf3;
							if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1) && getSceneBlendFactor(*i2, &sbf2) && 
								getSceneBlendFactor(*i3, &sbf3))
							{
								mPass->setSeparateSceneBlending(sbf0, sbf1, sbf2, sbf3);
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"one of the arguments to separate_scene_blend is not a valid scene blend factor directive");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"one of the arguments to separate_scene_blend is not a valid scene blend factor directive");
						}
					}
					break;
				case ID_SCENE_BLEND_OP:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scene_blend_op must have 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = reinterpret_cast<AtomAbstractNode*>(prop->values.front().get());
							switch(atom->id)
							{
							case ID_ADD:
								mPass->setSceneBlendingOperation(SBO_ADD);
								break;
							case ID_SUBTRACT:
								mPass->setSceneBlendingOperation(SBO_SUBTRACT);
								break;
							case ID_REVERSE_SUBTRACT:
								mPass->setSceneBlendingOperation(SBO_REVERSE_SUBTRACT);
								break;
							case ID_MIN:
								mPass->setSceneBlendingOperation(SBO_MIN);
								break;
							case ID_MAX:
								mPass->setSceneBlendingOperation(SBO_MAX);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom->value + ": unrecognized argument");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + ": unrecognized argument");
						}
					}
					break;
				case ID_SEPARATE_SCENE_BLEND_OP:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() != 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"separate_scene_blend_op must have 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = reinterpret_cast<AtomAbstractNode*>((*i0).get()),
								*atom1 = reinterpret_cast<AtomAbstractNode*>((*i1).get());
							SceneBlendOperation op = SBO_ADD, alphaOp = SBO_ADD;
							switch(atom0->id)
							{
							case ID_ADD:
								op = SBO_ADD;
								break;
							case ID_SUBTRACT:
								op = SBO_SUBTRACT;
								break;
							case ID_REVERSE_SUBTRACT:
								op = SBO_REVERSE_SUBTRACT;
								break;
							case ID_MIN:
								op = SBO_MIN;
								break;
							case ID_MAX:
								op = SBO_MAX;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom0->value + ": unrecognized first argument");
							}

							switch(atom1->id)
							{
							case ID_ADD:
								alphaOp = SBO_ADD;
								break;
							case ID_SUBTRACT:
								alphaOp = SBO_SUBTRACT;
								break;
							case ID_REVERSE_SUBTRACT:
								alphaOp = SBO_REVERSE_SUBTRACT;
								break;
							case ID_MIN:
								alphaOp = SBO_MIN;
								break;
							case ID_MAX:
								alphaOp = SBO_MAX;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom1->value + ": unrecognized second argument");
							}

							mPass->setSeparateSceneBlendingOperation(op, alphaOp);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + ": unrecognized argument");
						}
					}
					break;
				case ID_DEPTH_CHECK:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"depth_check must have 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val))
							mPass->setDepthCheckEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"depth_check third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
					}
					break;
				case ID_DEPTH_WRITE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"depth_write must have 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val))
							mPass->setDepthWriteEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"depth_write third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
					}
					break;
				case ID_DEPTH_BIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"depth_bias must have at most 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						float val0, val1 = 0.0f;
						if(getFloat(*i0, &val0))
						{
							if(i1 != prop->values.end())
								getFloat(*i1, &val1);
							mPass->setDepthBias(val0, val1);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"depth_bias does not support \"" + (*i0)->getValue() + "\" for argument 1");
						}
					}
					break;
				case ID_DEPTH_FUNC:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"depth_func must have 1 argument");
					}
					else
					{
						CompareFunction func;
						if(getCompareFunction(prop->values.front(), &func))
							mPass->setDepthFunction(func);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid CompareFunction");
					}
					break;
				case ID_ITERATION_DEPTH_BIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"iteration_depth_bias must have 1 argument");
					}
					else
					{
						float val = 0.0f;
						if(getFloat(prop->values.front(), &val))
							mPass->setIterationDepthBias(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							prop->values.front()->getValue() + " is not a valid float value");
					}
					break;
				case ID_ALPHA_REJECTION:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"alpha_rejection must have at most 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						CompareFunction func;
						if(getCompareFunction(*i0, &func))
						{
							if(i1 != prop->values.end())
							{
								uint32 val = 0;
								if(getUInt(*i1, &val))
									mPass->setAlphaRejectSettings(func, static_cast<unsigned char>(val));
								else
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i1)->getValue() + " is not a valid integer");
							}
							else
								mPass->setAlphaRejectFunction(func);
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								(*i0)->getValue() + " is not a valid CompareFunction");
					}
					break;
				case ID_ALPHA_TO_COVERAGE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"alpha_to_coverage must have 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val))
							mPass->setAlphaToCoverageEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"alpha_to_coverage argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
					}
					break;
				case ID_LIGHT_SCISSOR:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"light_scissor must have only 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setLightScissoringEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_LIGHT_CLIP_PLANES:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"light_clip_planes must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setLightClipPlanesEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_TRANSPARENT_SORTING:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"transparent_sorting must have at most 1 argument");
					}
					else
					{
						bool val = true;
						if(getBoolean(prop->values.front(), &val)) 
						{
							mPass->setTransparentSortingEnabled(val);
                            mPass->setTransparentSortingForced(false);
                        } 
						else 
						{
                            String val2;
                            if (getString(prop->values.front(), &val2) && val2=="force") 
							{
                                mPass->setTransparentSortingEnabled(true);
                                mPass->setTransparentSortingForced(true);
                            }
							else 
							{
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                    prop->values.front()->getValue() + " must be boolean or force");
                            }
                        }    
					}
					break;
				case ID_ILLUMINATION_STAGE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"illumination_stage must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_AMBIENT:
								mPass->setIlluminationStage(IS_AMBIENT);
								break;
							case ID_PER_LIGHT:
								mPass->setIlluminationStage(IS_PER_LIGHT);
								break;
							case ID_DECAL:
								mPass->setIlluminationStage(IS_DECAL);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid IlluminationStage");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid IlluminationStage");
						}
					}
					break;
				case ID_CULL_HARDWARE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"cull_hardware must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_CLOCKWISE:
								mPass->setCullingMode(CULL_CLOCKWISE);
								break;
							case ID_ANTICLOCKWISE:
								mPass->setCullingMode(CULL_ANTICLOCKWISE);
								break;
							case ID_NONE:
								mPass->setCullingMode(CULL_NONE);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid CullingMode");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid CullingMode");
						}
					}
					break;
				case ID_CULL_SOFTWARE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"cull_software must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_FRONT:
								mPass->setManualCullingMode(MANUAL_CULL_FRONT);
								break;
							case ID_BACK:
								mPass->setManualCullingMode(MANUAL_CULL_BACK);
								break;
							case ID_NONE:
								mPass->setManualCullingMode(MANUAL_CULL_NONE);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid ManualCullingMode");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid ManualCullingMode");
						}
					}
					break;
				case ID_NORMALISE_NORMALS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"normalise_normals must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setNormaliseNormals(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_LIGHTING:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"lighting must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setLightingEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_SHADING:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"shading must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_FLAT:
								mPass->setShadingMode(SO_FLAT);
								break;
							case ID_GOURAUD:
								mPass->setShadingMode(SO_GOURAUD);
								break;
							case ID_PHONG:
								mPass->setShadingMode(SO_PHONG);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid shading mode (flat, gouraud, or phong)");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid shading mode (flat, gouraud, or phong)");
						}
					}
					break;
				case ID_POLYGON_MODE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"polygon_mode must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_SOLID:
								mPass->setPolygonMode(PM_SOLID);
								break;
							case ID_POINTS:
								mPass->setPolygonMode(PM_POINTS);
								break;
							case ID_WIREFRAME:
								mPass->setPolygonMode(PM_WIREFRAME);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
						}
					}
					break;
				case ID_POLYGON_MODE_OVERRIDEABLE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"polygon_mode_overrideable must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setPolygonModeOverrideable(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_FOG_OVERRIDE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 8)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"fog_override must have at most 8 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
						{
							FogMode mode = FOG_NONE;
							ColourValue clr = ColourValue::White;
							Real dens = 0.001, start = 0.0f, end = 1.0f;

							if(i1 != prop->values.end())
							{
								if((*i1)->type == ANT_ATOM)
								{
									AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
									switch(atom->id)
									{
									case ID_NONE:
										mode = FOG_NONE;
										break;
									case ID_LINEAR:
										mode = FOG_LINEAR;
										break;
									case ID_EXP:
										mode = FOG_EXP;
										break;
									case ID_EXP2:
										mode = FOG_EXP2;
										break;
									default:
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*i1)->getValue() + " is not a valid FogMode");
										break;
									}
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i1)->getValue() + " is not a valid FogMode");
									break;
								}
							}

							if(i2 != prop->values.end())
							{
								  if(!getColour(i2, prop->values.end(), &clr, 3))
								  {
									 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									 (*i2)->getValue() + " is not a valid colour");
									 break;
								  }

							   i2 = getNodeAt(prop->values, 5);
							} 

							if(i2 != prop->values.end())
							{
								if(!getReal(*i2, &dens))
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i2)->getValue() + " is not a valid number");
									break;
								}
								++i2;
							}

							if(i2 != prop->values.end())
							{
								if(!getReal(*i2, &start))
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i2)->getValue() + " is not a valid number");
									return;
								}
								++i2;
							}

							if(i2 != prop->values.end())
							{
								if(!getReal(*i2, &end))
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i2)->getValue() + " is not a valid number");
									return;
								}
								++i2;
							}

							mPass->setFog(val, mode, clr, dens, start, end);
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_COLOUR_WRITE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"colour_write must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setColourWriteEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_MAX_LIGHTS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"max_lights must have at most 1 argument");
					}
					else
					{
						uint32 val = 0;
						if(getUInt(prop->values.front(), &val))
							mPass->setMaxSimultaneousLights(static_cast<unsigned short>(val));
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid integer");
					}
					break;
				case ID_START_LIGHT:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"start_light must have at most 1 argument");
					}
					else
					{
						uint32 val = 0;
						if(getUInt(prop->values.front(), &val))
							mPass->setStartLight(static_cast<unsigned short>(val));
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid integer");
					}
					break;
				case ID_ITERATION:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						if((*i0)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
							if(atom->id == ID_ONCE)
							{
								mPass->setIteratePerLight(false);
							}
							else if(atom->id == ID_ONCE_PER_LIGHT)
							{
								AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
								if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
								{
									atom = (AtomAbstractNode*)(*i1).get();
									switch(atom->id)
									{
									case ID_POINT:
										mPass->setIteratePerLight(true);
										break;
									case ID_DIRECTIONAL:
										mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
										break;
									case ID_SPOT:
										mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
										break;
									default:
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											prop->values.front()->getValue() + " is not a valid light type (point, directional, or spot)");
									}
								}
								else
								{
									mPass->setIteratePerLight(true, false);
								}

							}
							else if(StringConverter::isNumber(atom->value))
							{
								mPass->setPassIterationCount(Ogre::StringConverter::parseInt(atom->value));

								AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
								if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
								{
									atom = (AtomAbstractNode*)(*i1).get();
									if(atom->id == ID_PER_LIGHT)
									{
										AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
										if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
										{
											atom = (AtomAbstractNode*)(*i2).get();
											switch(atom->id)
											{
											case ID_POINT:
												mPass->setIteratePerLight(true);
												break;
											case ID_DIRECTIONAL:
												mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
												break;
											case ID_SPOT:
												mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
												break;
											default:
												compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
													(*i2)->getValue() + " is not a valid light type (point, directional, or spot)");
											}
										}
										else
										{
											mPass->setIteratePerLight(true, false);
										}
									}
									else if(ID_PER_N_LIGHTS)
									{
										AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
										if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
										{
											atom = (AtomAbstractNode*)(*i2).get();
											if(StringConverter::isNumber(atom->value))
											{
												mPass->setLightCountPerIteration(
													static_cast<unsigned short>(StringConverter::parseInt(atom->value)));
												
												AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
												if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
												{
													atom = (AtomAbstractNode*)(*i3).get();
													switch(atom->id)
													{
													case ID_POINT:
														mPass->setIteratePerLight(true);
														break;
													case ID_DIRECTIONAL:
														mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
														break;
													case ID_SPOT:
														mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
														break;
													default:
														compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
															(*i3)->getValue() + " is not a valid light type (point, directional, or spot)");
													}
												}
												else
												{
													mPass->setIteratePerLight(true, false);
												}
											}
											else
											{
												compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
													(*i2)->getValue() + " is not a valid number");
											}
										}
										else
										{
											compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
												prop->values.front()->getValue() + " is not a valid number");
										}
									}
								}
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_POINT_SIZE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"point_size must have at most 1 argument");
					}
					else
					{
						Real val = 0.0f;
						if(getReal(prop->values.front(), &val))
							mPass->setPointSize(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number");
					}
					break;
				case ID_POINT_SPRITES:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"point_sprites must have at most 1 argument");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
							mPass->setPointSpritesEnabled(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_POINT_SIZE_ATTENUATION:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 4)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"point_size_attenuation must have at most 4 arguments");
					}
					else
					{
						bool val = false;
						if(getBoolean(prop->values.front(), &val))
						{
							if(val)
							{
								AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2), 
									i3 = getNodeAt(prop->values, 3);

								if (prop->values.size() > 1)
								{

									Real constant = 0.0f, linear = 1.0f, quadratic = 0.0f;

									if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
									{
										AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
										if(StringConverter::isNumber(atom->value))
											constant = StringConverter::parseReal(atom->value);
										else
											compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
									else
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*i1)->getValue() + " is not a valid number");
									}

									if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
									{
										AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
										if(StringConverter::isNumber(atom->value))
											linear = StringConverter::parseReal(atom->value);
										else
											compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
									else
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*i2)->getValue() + " is not a valid number");
									}

									if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
									{
										AtomAbstractNode *atom = (AtomAbstractNode*)(*i3).get();
										if(StringConverter::isNumber(atom->value))
											quadratic = StringConverter::parseReal(atom->value);
										else
											compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
									else
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*i3)->getValue() + " is not a valid number");
									}

									mPass->setPointAttenuation(true, constant, linear, quadratic);
								}
								else
								{
									mPass->setPointAttenuation(true);
								}
							}
							else
							{
								mPass->setPointAttenuation(false);
							}
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid boolean");
					}
					break;
				case ID_POINT_SIZE_MIN:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"point_size_min must have at most 1 argument");
					}
					else
					{
						Real val = 0.0f;
						if(getReal(prop->values.front(), &val))
							mPass->setPointMinSize(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number");
					}
					break;
				case ID_POINT_SIZE_MAX:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"point_size_max must have at most 1 argument");
					}
					else
					{
						Real val = 0.0f;
						if(getReal(prop->values.front(), &val))
							mPass->setPointMaxSize(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number");
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *child = reinterpret_cast<ObjectAbstractNode*>((*i).get());
				switch(child->id)
				{
				case ID_FRAGMENT_PROGRAM_REF:
					translateFragmentProgramRef(compiler, child);
					break;
				case ID_VERTEX_PROGRAM_REF:
					translateVertexProgramRef(compiler, child);
					break;
				case ID_GEOMETRY_PROGRAM_REF:
					translateGeometryProgramRef(compiler, child);
					break;
				case ID_SHADOW_CASTER_VERTEX_PROGRAM_REF:
					translateShadowCasterVertexProgramRef(compiler, child);
					break;
				case ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF:
					translateShadowReceiverVertexProgramRef(compiler, child);
					break;
				case ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF:
					translateShadowReceiverFragmentProgramRef(compiler, child);
					break;
				default:
					processNode(compiler, *i);
				}
			}
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateFragmentProgramRef(Ogre::ScriptCompiler *compiler, Ogre::ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setFragmentProgram(evt.mName);
		if(pass->getFragmentProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setVertexProgram(evt.mName);
		if(pass->getVertexProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateGeometryProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setGeometryProgram(evt.mName);
		if(pass->getGeometryProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getGeometryProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateShadowCasterVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setShadowCasterVertexProgram(evt.mName);
		if(pass->getShadowCasterVertexProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getShadowCasterVertexProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateShadowReceiverVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setShadowReceiverVertexProgram(evt.mName);
		if(pass->getShadowReceiverVertexProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getShadowReceiverVertexProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}
	//-------------------------------------------------------------------------
	void PassTranslator::translateShadowReceiverFragmentProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
	{
		if(node->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
			return;
		}

		ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
		compiler->_fireEvent(&evt, 0);

		if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
		{
			compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
			return;
		}

		Pass *pass = any_cast<Pass*>(node->parent->context);
		pass->setShadowReceiverFragmentProgram(evt.mName);
		if(pass->getShadowReceiverFragmentProgram()->isSupported())
		{
			GpuProgramParametersSharedPtr params = pass->getShadowReceiverFragmentProgramParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, params, node);
		}
	}

	/**************************************************************************
	 * TextureUnitTranslator
	 *************************************************************************/
	TextureUnitTranslator::TextureUnitTranslator()
		:mUnit(0)
	{
	}
	//-------------------------------------------------------------------------
	void TextureUnitTranslator::translate(ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		Pass *pass = any_cast<Pass*>(obj->parent->context);
		mUnit = pass->createTextureUnitState();
		obj->context = Any(mUnit);

		// Get the name of the technique
		if(!obj->name.empty())
			mUnit->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_TEXTURE_ALIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"texture_alias must have at most 1 argument");
					}
					else
					{
						String val;
						if(getString(prop->values.front(), &val))
							mUnit->setTextureNameAlias(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid texture alias");
					}
					break;
				case ID_TEXTURE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 5)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"texture must have at most 5 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator j = prop->values.begin();
						String val;
						if(getString(*j, &val))
						{
							TextureType texType = TEX_TYPE_2D;
							bool isAlpha = false;
							bool sRGBRead = false;
							PixelFormat format = PF_UNKNOWN;
							int mipmaps = MIP_DEFAULT;

							++j;
							while(j != prop->values.end())
							{
								if((*j)->type == ANT_ATOM)
								{
									AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
									switch(atom->id)
									{
									case ID_1D:
										texType = TEX_TYPE_1D;
										break;
									case ID_2D:
										texType = TEX_TYPE_2D;
										break;
									case ID_3D:
										texType = TEX_TYPE_3D;
										break;
									case ID_CUBIC:
										texType = TEX_TYPE_CUBE_MAP;
										break;
									case ID_UNLIMITED:
										mipmaps = MIP_UNLIMITED;
										break;
									case ID_ALPHA:
										isAlpha = true;
										break;
									case ID_GAMMA:
										sRGBRead = true;
										break;
									default:
										if(StringConverter::isNumber(atom->value))
											mipmaps = StringConverter::parseInt(atom->value);
										else
											format = PixelUtil::getFormatFromName(atom->value, true);
									}
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*j)->getValue() + " is not a supported argument to the texture property");
								}
								++j;
							}

							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, val);
							compiler->_fireEvent(&evt, 0);

							mUnit->setTextureName(evt.mName, texType);
							mUnit->setDesiredFormat(format);
							mUnit->setIsAlpha(isAlpha);
							mUnit->setNumMipmaps(mipmaps);
							mUnit->setHardwareGammaEnabled(sRGBRead);
						}
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								(*j)->getValue() + " is not a valid texture name");
					}
					break;
				case ID_ANIM_TEXTURE:
					if(prop->values.size() < 3)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else
					{
						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
						if((*i1)->type == ANT_ATOM && StringConverter::isNumber(((AtomAbstractNode*)(*i1).get())->value))
						{
							// Short form
							AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i2 = getNodeAt(prop->values, 2);
							if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
							{
								String val0;
								uint32 val1;
								Real val2;
								if(getString(*i0, &val0) && getUInt(*i1, &val1) && getReal(*i2, &val2))
								{
									ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, val0);
									compiler->_fireEvent(&evt, 0);

									mUnit->setAnimatedTextureName(evt.mName, val1, val2);
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"anim_texture short form requires a texture name, number of frames, and animation duration");
								}
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"anim_texture short form requires a texture name, number of frames, and animation duration");
							}
						}
						else
						{
							// Long form has n number of frames
							Real duration = 0;
							AbstractNodeList::const_iterator in = getNodeAt(prop->values, static_cast<int>(prop->values.size()) - 1);
							if(getReal(*in, &duration))
							{
								String *names = OGRE_NEW_ARRAY_T(String, prop->values.size() - 1, MEMCATEGORY_SCRIPTING);
								int n = 0;

								AbstractNodeList::iterator j = prop->values.begin();
								while(j != in)
								{
									if((*j)->type == ANT_ATOM)
									{
										String name = ((AtomAbstractNode*)(*j).get())->value;
										// Run the name through the listener
										if(compiler->getListener())
										{
											ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, name);
											compiler->_fireEvent(&evt, 0);
											names[n++] = evt.mName;
										}
										else
										{
											names[n++] = name;
										}
									}
									else
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*j)->getValue() + " is not supported as a texture name");
									++j;
								}

								mUnit->setAnimatedTextureName(names, n, duration);

								OGRE_DELETE_ARRAY_T(names, String, prop->values.size() - 1, MEMCATEGORY_SCRIPTING);
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
									(*in)->getValue() + " is not supported for the duration argument");
							}
						}
					}
					break;
				case ID_CUBIC_TEXTURE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() == 2)
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
							i1 = getNodeAt(prop->values, 1);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
						{	
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();

							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, atom0->value);
							compiler->_fireEvent(&evt, 0);

							mUnit->setCubicTextureName(evt.mName, atom1->id == ID_COMBINED_UVW);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					else if(prop->values.size() == 7)
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
							i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2),
							i3 = getNodeAt(prop->values, 3),
							i4 = getNodeAt(prop->values, 4),
							i5 = getNodeAt(prop->values, 5),
							i6 = getNodeAt(prop->values, 6);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM &&
							(*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM && (*i6)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get(),
								*atom2 = (AtomAbstractNode*)(*i2).get(), *atom3 = (AtomAbstractNode*)(*i3).get(),
								*atom4 = (AtomAbstractNode*)(*i4).get(), *atom5 = (AtomAbstractNode*)(*i5).get(),
								*atom6 = (AtomAbstractNode*)(*i6).get();
							String names[6];
							names[0] = atom0->value;
							names[1] = atom1->value;
							names[2] = atom2->value;
							names[3] = atom3->value;
							names[4] = atom4->value;
							names[5] = atom5->value;

							if(compiler->getListener())
							{
								// Run each name through the listener
								for(int i = 0; i < 6; ++i)
								{
									ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, names[i]);
									compiler->_fireEvent(&evt, 0);
									names[i] = evt.mName;
								}
							}

							mUnit->setCubicTextureName(names, atom6->id == ID_COMBINED_UVW);
						}

					}
					else
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"cubic_texture must have at most 7 arguments");
					}
					break;
				case ID_TEX_COORD_SET:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"tex_coord_set must have at most 1 argument");
					}
					else
					{
						uint32 val = 0;
						if(getUInt(prop->values.front(), &val))
							mUnit->setTextureCoordSet(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not supported as an integer argument");
					}
					break;
				case ID_TEX_ADDRESS_MODE:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						}
						else
						{
							AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), 
								i1 = getNodeAt(prop->values, 1), 
								i2 = getNodeAt(prop->values, 2);
							TextureUnitState::UVWAddressingMode mode;
							mode.u = mode.v = mode.w = TextureUnitState::TAM_WRAP;
							
							if(i0 != prop->values.end() && (*i0)->type == ANT_ATOM)
							{
								AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
								switch(atom->id)
								{
								case ID_WRAP:
									mode.u = TextureUnitState::TAM_WRAP;
									break;
								case ID_CLAMP:
									mode.u = TextureUnitState::TAM_CLAMP;
									break;
								case ID_MIRROR:
									mode.u = TextureUnitState::TAM_MIRROR;
									break;
								case ID_BORDER:
									mode.u = TextureUnitState::TAM_BORDER;
									break;
								default:
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i0)->getValue() + " not supported as first argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
								}
							}
							mode.v = mode.u;
							mode.w = mode.u;
							
							if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
							{
								AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
								switch(atom->id)
								{
								case ID_WRAP:
									mode.v = TextureUnitState::TAM_WRAP;
									break;
								case ID_CLAMP:
									mode.v = TextureUnitState::TAM_CLAMP;
									break;
								case ID_MIRROR:
									mode.v = TextureUnitState::TAM_MIRROR;
									break;
								case ID_BORDER:
									mode.v = TextureUnitState::TAM_BORDER;
									break;
								default:
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i0)->getValue() + " not supported as second argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
								}
							}

							if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
							{
								AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
								switch(atom->id)
								{
								case ID_WRAP:
									mode.w = TextureUnitState::TAM_WRAP;
									break;
								case ID_CLAMP:
									mode.w = TextureUnitState::TAM_CLAMP;
									break;
								case ID_MIRROR:
									mode.w = TextureUnitState::TAM_MIRROR;
									break;
								case ID_BORDER:
									mode.w = TextureUnitState::TAM_BORDER;
									break;
								default:
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										(*i0)->getValue() + " not supported as third argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
								}
							}

							mUnit->setTextureAddressingMode(mode);
						}
					}
					break;
				case ID_TEX_BORDER_COLOUR:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else
					{
						ColourValue val;
						if(getColour(prop->values.begin(), prop->values.end(), &val))
							mUnit->setTextureBorderColour(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"tex_border_colour only accepts a colour argument");
					}
					break;
				case ID_FILTERING:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() == 1)
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_NONE:
								mUnit->setTextureFiltering(TFO_NONE);
								break;
							case ID_BILINEAR:
								mUnit->setTextureFiltering(TFO_BILINEAR);
								break;
							case ID_TRILINEAR:
								mUnit->setTextureFiltering(TFO_TRILINEAR);
								break;
							case ID_ANISOTROPIC:
								mUnit->setTextureFiltering(TFO_ANISOTROPIC);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " not supported as first argument (must be \"none\", \"bilinear\", \"trilinear\", or \"anisotropic\")");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " not supported as first argument (must be \"none\", \"bilinear\", \"trilinear\", or \"anisotropic\")");
						}
					}
					else if(prop->values.size() == 3)
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
							i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2);
						if((*i0)->type == ANT_ATOM &&
							(*i1)->type == ANT_ATOM &&
							(*i2)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
								*atom1 = (AtomAbstractNode*)(*i1).get(),
								*atom2 = (AtomAbstractNode*)(*i2).get();
							FilterOptions tmin = FO_NONE, tmax = FO_NONE, tmip = FO_NONE;
							switch(atom0->id)
							{
							case ID_NONE:
								tmin = FO_NONE;
								break;
							case ID_POINT:
								tmin = FO_POINT;
								break;
							case ID_LINEAR:
								tmin = FO_LINEAR;
								break;
							case ID_ANISOTROPIC:
								tmin = FO_ANISOTROPIC;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i0)->getValue() + " not supported as first argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
							}

							switch(atom1->id)
							{
							case ID_NONE:
								tmax = FO_NONE;
								break;
							case ID_POINT:
								tmax = FO_POINT;
								break;
							case ID_LINEAR:
								tmax = FO_LINEAR;
								break;
							case ID_ANISOTROPIC:
								tmax = FO_ANISOTROPIC;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i0)->getValue() + " not supported as second argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
							}

							switch(atom2->id)
							{
							case ID_NONE:
								tmip = FO_NONE;
								break;
							case ID_POINT:
								tmip = FO_POINT;
								break;
							case ID_LINEAR:
								tmip = FO_LINEAR;
								break;
							case ID_ANISOTROPIC:
								tmip = FO_ANISOTROPIC;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i0)->getValue() + " not supported as third argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
							}

							mUnit->setTextureFiltering(tmin, tmax, tmip);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"filtering must have either 1 or 3 arguments");
					}
					break;
				case ID_MAX_ANISOTROPY:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"max_anisotropy must have at most 1 argument");
					}
					else
					{
						uint32 val = 0;
						if(getUInt(prop->values.front(), &val))
							mUnit->setTextureAnisotropy(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid integer argument");
					}
					break;
				case ID_MIPMAP_BIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"mipmap_bias must have at most 1 argument");
					}
					else
					{
						Real val = 0.0f;
						if(getReal(prop->values.front(), &val))
							mUnit->setTextureMipmapBias(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number argument");
					}
					break;
				case ID_COLOUR_OP:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"colour_op must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_REPLACE:
								mUnit->setColourOperation(LBO_REPLACE);
								break;
							case ID_ADD:
								mUnit->setColourOperation(LBO_ADD);
								break;
							case ID_MODULATE:
								mUnit->setColourOperation(LBO_MODULATE);
								break;
							case ID_ALPHA_BLEND:
								mUnit->setColourOperation(LBO_ALPHA_BLEND);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid argument (must be \"replace\", \"add\", \"modulate\", or \"alpha_blend\")");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid argument (must be \"replace\", \"add\", \"modulate\", or \"alpha_blend\")");
						}
					}
					break;
				case ID_COLOUR_OP_EX:
					if(prop->values.size() < 3)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"colour_op_ex must have at least 3 arguments");
					}
					else if(prop->values.size() > 10)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"colour_op_ex must have at most 10 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
							i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
								*atom1 = (AtomAbstractNode*)(*i1).get(),
								*atom2 = (AtomAbstractNode*)(*i2).get();
							LayerBlendOperationEx op = LBX_ADD;
							LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
							ColourValue arg1 = ColourValue::White, arg2 = ColourValue::White;
							Real manualBlend = 0.0f;

							switch(atom0->id)
							{
							case ID_SOURCE1:
								op = LBX_SOURCE1;
								break;
							case ID_SOURCE2:
								op = LBX_SOURCE2;
								break;
							case ID_MODULATE:
								op = LBX_MODULATE;
								break;
							case ID_MODULATE_X2:
								op = LBX_MODULATE_X2;
								break;
							case ID_MODULATE_X4:
								op = LBX_MODULATE_X4;
								break;
							case ID_ADD:
								op = LBX_ADD;
								break;
							case ID_ADD_SIGNED:
								op = LBX_ADD_SIGNED;
								break;
							case ID_ADD_SMOOTH:
								op = LBX_ADD_SMOOTH;
								break;
							case ID_SUBTRACT:
								op = LBX_SUBTRACT;
								break;
							case ID_BLEND_DIFFUSE_ALPHA:
								op = LBX_BLEND_DIFFUSE_ALPHA;
								break;
							case ID_BLEND_TEXTURE_ALPHA:
								op = LBX_BLEND_TEXTURE_ALPHA;
								break;
							case ID_BLEND_CURRENT_ALPHA:
								op = LBX_BLEND_CURRENT_ALPHA;
								break;
							case ID_BLEND_MANUAL:
								op = LBX_BLEND_MANUAL;
								break;
							case ID_DOT_PRODUCT:
								op = LBX_DOTPRODUCT;
								break;
							case ID_BLEND_DIFFUSE_COLOUR:
								op = LBX_BLEND_DIFFUSE_COLOUR;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i0)->getValue() + " is not a valid first argument (must be \"source1\", \"source2\", \"modulate\", \"modulate_x2\", \"modulate_x4\", \"add\", \"add_signed\", \"add_smooth\", \"subtract\", \"blend_diffuse_alpha\", \"blend_texture_alpha\", \"blend_current_alpha\", \"blend_manual\", \"dot_product\", or \"blend_diffuse_colour\")");
							}

							switch(atom1->id)
							{
							case ID_SRC_CURRENT:
								source1 = LBS_CURRENT;
								break;
							case ID_SRC_TEXTURE:
								source1 = LBS_TEXTURE;
								break;
							case ID_SRC_DIFFUSE:
								source1 = LBS_DIFFUSE;
								break;
							case ID_SRC_SPECULAR:
								source1 = LBS_SPECULAR;
								break;
							case ID_SRC_MANUAL:
								source1 = LBS_MANUAL;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i1)->getValue() + " is not a valid second argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
							}

							switch(atom2->id)
							{
							case ID_SRC_CURRENT:
								source2 = LBS_CURRENT;
								break;
							case ID_SRC_TEXTURE:
								source2 = LBS_TEXTURE;
								break;
							case ID_SRC_DIFFUSE:
								source2 = LBS_DIFFUSE;
								break;
							case ID_SRC_SPECULAR:
								source2 = LBS_SPECULAR;
								break;
							case ID_SRC_MANUAL:
								source2 = LBS_MANUAL;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i2)->getValue() + " is not a valid third argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
							}

							if(op == LBX_BLEND_MANUAL)
							{
								AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
								if(i3 != prop->values.end())
								{
									if(!getReal(*i3, &manualBlend))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											(*i3)->getValue() + " is not a valid number argument");
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"fourth argument expected when blend_manual is used");
								}
							}

							AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
							if(op == LBX_BLEND_MANUAL)
								++j;
							if(source1 == LBS_MANUAL)
							{
								if(j != prop->values.end())
								{
									if(!getColour(j, prop->values.end(), &arg1, 3))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"valid colour expected when src_manual is used");
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"valid colour expected when src_manual is used");
								}
							}
							if(source2 == LBS_MANUAL)
							{
								if(j != prop->values.end())
								{
									if(!getColour(j, prop->values.end(), &arg2, 3))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"valid colour expected when src_manual is used");
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"valid colour expected when src_manual is used");
								}
							}

							mUnit->setColourOperationEx(op, source1, source2, arg1, arg2, manualBlend);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_COLOUR_OP_MULTIPASS_FALLBACK:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"colour_op_multiplass_fallback must have at most 2 arguments");
					}
					else if(prop->values.size() == 1)
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_ADD:
								mUnit->setColourOpMultipassFallback(SBF_ONE, SBF_ONE);
								break;
							case ID_MODULATE:
								mUnit->setColourOpMultipassFallback(SBF_DEST_COLOUR, SBF_ZERO);
								break;
							case ID_COLOUR_BLEND:
								mUnit->setColourOpMultipassFallback(SBF_SOURCE_COLOUR, SBF_ONE_MINUS_SOURCE_COLOUR);
								break;
							case ID_ALPHA_BLEND:
								mUnit->setColourOpMultipassFallback(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
								break;
							case ID_REPLACE:
								mUnit->setColourOpMultipassFallback(SBF_ONE, SBF_ZERO);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"argument must be a valid scene blend type (add, modulate, colour_blend, alpha_blend, or replace)");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"argument must be a valid scene blend type (add, modulate, colour_blend, alpha_blend, or replace)");
						}
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						SceneBlendFactor sbf0, sbf1;
						if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
							mUnit->setColourOpMultipassFallback(sbf0, sbf1);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"arguments must be valid scene blend factors");
					}
					break;
				case ID_ALPHA_OP_EX:
					if(prop->values.size() < 3)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"alpha_op_ex must have at least 3 arguments");
					}
					else if(prop->values.size() > 6)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"alpha_op_ex must have at most 6 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
							i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
								*atom1 = (AtomAbstractNode*)(*i1).get(),
								*atom2 = (AtomAbstractNode*)(*i2).get();
							LayerBlendOperationEx op = LBX_ADD;
							LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
							Real arg1 = 0.0f, arg2 = 0.0f;
							Real manualBlend = 0.0f;

							switch(atom0->id)
							{
							case ID_SOURCE1:
								op = LBX_SOURCE1;
								break;
							case ID_SOURCE2:
								op = LBX_SOURCE2;
								break;
							case ID_MODULATE:
								op = LBX_MODULATE;
								break;
							case ID_MODULATE_X2:
								op = LBX_MODULATE_X2;
								break;
							case ID_MODULATE_X4:
								op = LBX_MODULATE_X4;
								break;
							case ID_ADD:
								op = LBX_ADD;
								break;
							case ID_ADD_SIGNED:
								op = LBX_ADD_SIGNED;
								break;
							case ID_ADD_SMOOTH:
								op = LBX_ADD_SMOOTH;
								break;
							case ID_SUBTRACT:
								op = LBX_SUBTRACT;
								break;
							case ID_BLEND_DIFFUSE_ALPHA:
								op = LBX_BLEND_DIFFUSE_ALPHA;
								break;
							case ID_BLEND_TEXTURE_ALPHA:
								op = LBX_BLEND_TEXTURE_ALPHA;
								break;
							case ID_BLEND_CURRENT_ALPHA:
								op = LBX_BLEND_CURRENT_ALPHA;
								break;
							case ID_BLEND_MANUAL:
								op = LBX_BLEND_MANUAL;
								break;
							case ID_DOT_PRODUCT:
								op = LBX_DOTPRODUCT;
								break;
							case ID_BLEND_DIFFUSE_COLOUR:
								op = LBX_BLEND_DIFFUSE_COLOUR;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i0)->getValue() + " is not a valid first argument (must be \"source1\", \"source2\", \"modulate\", \"modulate_x2\", \"modulate_x4\", \"add\", \"add_signed\", \"add_smooth\", \"subtract\", \"blend_diffuse_alpha\", \"blend_texture_alpha\", \"blend_current_alpha\", \"blend_manual\", \"dot_product\", or \"blend_diffuse_colour\")");
							}

							switch(atom1->id)
							{
							case ID_SRC_CURRENT:
								source1 = LBS_CURRENT;
								break;
							case ID_SRC_TEXTURE:
								source1 = LBS_TEXTURE;
								break;
							case ID_SRC_DIFFUSE:
								source1 = LBS_DIFFUSE;
								break;
							case ID_SRC_SPECULAR:
								source1 = LBS_SPECULAR;
								break;
							case ID_SRC_MANUAL:
								source1 = LBS_MANUAL;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i1)->getValue() + " is not a valid second argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
							}

							switch(atom2->id)
							{
							case ID_SRC_CURRENT:
								source2 = LBS_CURRENT;
								break;
							case ID_SRC_TEXTURE:
								source2 = LBS_TEXTURE;
								break;
							case ID_SRC_DIFFUSE:
								source2 = LBS_DIFFUSE;
								break;
							case ID_SRC_SPECULAR:
								source2 = LBS_SPECULAR;
								break;
							case ID_SRC_MANUAL:
								source2 = LBS_MANUAL;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									(*i2)->getValue() + " is not a valid third argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
							}

							if(op == LBX_BLEND_MANUAL)
							{
								AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
								if(i3 != prop->values.end())
								{
									if(!getReal(*i3, &manualBlend))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"valid number expected when blend_manual is used");
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"valid number expected when blend_manual is used");
								}
							}

							AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
							if(op == LBX_BLEND_MANUAL)
								++j;
							if(source1 == LBS_MANUAL)
							{
								if(j != prop->values.end())
								{
									if(!getReal(*j, &arg1))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"valid colour expected when src_manual is used");
									else
										++j;
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"valid colour expected when src_manual is used");
								}
							}
							if(source2 == LBS_MANUAL)
							{
								if(j != prop->values.end())
								{
									if(!getReal(*j, &arg2))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"valid colour expected when src_manual is used");
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"valid colour expected when src_manual is used");
								}
							}

							mUnit->setAlphaOperation(op, source1, source2, arg1, arg2, manualBlend);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_ENV_MAP:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"env_map must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ScriptCompiler::ID_OFF:
								mUnit->setEnvironmentMap(false);
								break;
							case ID_SPHERICAL:
								mUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
								break;
							case ID_PLANAR:
								mUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
								break;
							case ID_CUBIC_REFLECTION:
								mUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
								break;
							case ID_CUBIC_NORMAL:
								mUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									prop->values.front()->getValue() + " is not a valid argument (must be \"off\", \"spherical\", \"planar\", \"cubic_reflection\", or \"cubic_normal\")");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid argument (must be \"off\", \"spherical\", \"planar\", \"cubic_reflection\", or \"cubic_normal\")");
						}
					}
					break;
				case ID_SCROLL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scroll must have at most 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						Real x, y;
						if(getReal(*i0, &x) && getReal(*i1, &y))
							mUnit->setTextureScroll(x, y);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								(*i0)->getValue() + " and/or " + (*i1)->getValue() + " is invalid; both must be numbers");
					}
					break;
				case ID_SCROLL_ANIM:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scroll_anim must have at most 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						Real x, y;
						if(getReal(*i0, &x) && getReal(*i1, &y))
							mUnit->setScrollAnimation(x, y);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								(*i0)->getValue() + " and/or " + (*i1)->getValue() + " is invalid; both must be numbers");
					}
					break;
				case ID_ROTATE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"rotate must have at most 1 argument");
					}
					else
					{
						Real angle;
						if(getReal(prop->values.front(), &angle))
							mUnit->setTextureRotate(Degree(angle));
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number value");
					}
					break;
				case ID_ROTATE_ANIM:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"rotate_anim must have at most 1 argument");
					}
					else
					{
						Real angle;
						if(getReal(prop->values.front(), &angle))
							mUnit->setRotateAnimation(angle);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid number value");
					}
					break;
				case ID_SCALE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 2)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scale must have at most 2 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
						Real x, y;
						if(getReal(*i0, &x) && getReal(*i1, &y))
							mUnit->setTextureScale(x, y);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"first and second arguments must both be valid number values (received " + (*i0)->getValue() + ", " + (*i1)->getValue() + ")");
					}
					break;
				case ID_WAVE_XFORM:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 6)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"wave_xform must have at most 6 arguments");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
							i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3),
							i4 = getNodeAt(prop->values, 4), i5 = getNodeAt(prop->values, 5);
						if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM &&
							(*i3)->type == ANT_ATOM && (*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
							TextureUnitState::TextureTransformType type = TextureUnitState::TT_ROTATE;
							WaveformType wave = WFT_SINE;
							Real base = 0.0f, freq = 0.0f, phase = 0.0f, amp = 0.0f;

							switch(atom0->id)
							{
							case ID_SCROLL_X:
								type = TextureUnitState::TT_TRANSLATE_U;
								break;
							case ID_SCROLL_Y:
								type = TextureUnitState::TT_TRANSLATE_V;
								break;
							case ID_SCALE_X:
								type = TextureUnitState::TT_SCALE_U;
								break;
							case ID_SCALE_Y:
								type = TextureUnitState::TT_SCALE_V;
								break;
							case ID_ROTATE:
								type = TextureUnitState::TT_ROTATE;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom0->value + " is not a valid transform type (must be \"scroll_x\", \"scroll_y\", \"scale_x\", \"scale_y\", or \"rotate\")");
							}

							switch(atom1->id)
							{
							case ID_SINE:
								wave = WFT_SINE;
								break;
							case ID_TRIANGLE:
								wave = WFT_TRIANGLE;
								break;
							case ID_SQUARE:
								wave = WFT_SQUARE;
								break;
							case ID_SAWTOOTH:
								wave = WFT_SAWTOOTH;
								break;
							case ID_INVERSE_SAWTOOTH:
								wave = WFT_INVERSE_SAWTOOTH;
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom1->value + " is not a valid waveform type (must be \"sine\", \"triangle\", \"square\", \"sawtooth\", or \"inverse_sawtooth\")");
							}

							if(!getReal(*i2, &base) || !getReal(*i3, &freq) || !getReal(*i4, &phase) || !getReal(*i5, &amp))
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"arguments 3, 4, 5, and 6 must be valid numbers; received " + (*i2)->getValue() + ", " + (*i3)->getValue() + ", " + (*i4)->getValue() + ", " + (*i5)->getValue());

							mUnit->setTransformAnimation(type, wave, base, freq, phase, amp);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_TRANSFORM:
					{
						Matrix4 m;
						if(getMatrix4(prop->values.begin(), prop->values.end(), &m))
							mUnit->setTextureTransform(m);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_BINDING_TYPE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"binding_type must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_VERTEX:
								mUnit->setBindingType(TextureUnitState::BT_VERTEX);
								break;
							case ID_FRAGMENT:
								mUnit->setBindingType(TextureUnitState::BT_FRAGMENT);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom->value + " is not a valid binding type (must be \"vertex\" or \"fragment\")");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid binding type");
						}
					}
					break;
				case ID_CONTENT_TYPE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"content_type must have at most 1 argument");
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_NAMED:
								mUnit->setContentType(TextureUnitState::CONTENT_NAMED);
								break;
							case ID_SHADOW:
								mUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									atom->value + " is not a valid content type (must be \"named\" or \"shadows\")");
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								prop->values.front()->getValue() + " is not a valid content type");
						}
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}				
			}
			else if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
		}
	}

	/**************************************************************************
	* TextureSourceTranslator
	**************************************************************************/
	TextureSourceTranslator::TextureSourceTranslator()
	{
	}
	//-------------------------------------------------------------------------
	void TextureSourceTranslator::translate(Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		// It has to have one value identifying the texture source name
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, node->file, node->line,
				"texture_source requires a type value");
			return;
		}

		// Set the value of the source
		ExternalTextureSourceManager::getSingleton().setCurrentPlugIn(obj->values.front()->getValue());

		// Set up the technique, pass, and texunit levels
		if(ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0)
		{
			TextureUnitState *texunit = any_cast<TextureUnitState*>(obj->parent->context);
			Pass *pass = texunit->getParent();
			Technique *technique = pass->getParent();
			Material *material = technique->getParent();

			unsigned short techniqueIndex = 0, passIndex = 0, texUnitIndex = 0;
			for(unsigned short i = 0; i < material->getNumTechniques(); i++)
			{
				if(material->getTechnique(i) == technique)
				{
					techniqueIndex = i;
					break;
				}
			}
			for(unsigned short i = 0; i < technique->getNumPasses(); i++)
			{
				if(technique->getPass(i) == pass)
				{
					passIndex = i;
					break;
				}
			}
			for(unsigned short i = 0; i < pass->getNumTextureUnitStates(); i++)
			{
				if(pass->getTextureUnitState(i) == texunit)
				{
					texUnitIndex = i;
					break;
				}
			}

			String tps;
			tps = StringConverter::toString(techniqueIndex) + " "
				+ StringConverter::toString(passIndex) + " "
				+ StringConverter::toString(texUnitIndex);

			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( "set_T_P_S", tps );

			for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
			{
				if((*i)->type == ANT_PROPERTY)
				{
					PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
					// Glob the property values all together
					String str = "";
					for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
					{
						if(j != prop->values.begin())
							str = str + " ";
						str = str + (*j)->getValue();
					}
					ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter(prop->name, str);
				}
				else if((*i)->type == ANT_OBJECT)
				{
					processNode(compiler, *i);
				}
			}

			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->createDefinedTexture(material->getName(), material->getGroup());
		}
	}
	
	/**************************************************************************
	 * GpuProgramTranslator
	 *************************************************************************/
	GpuProgramTranslator::GpuProgramTranslator()
	{
	}
	//-------------------------------------------------------------------------
	void GpuProgramTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		// Must have a name
		if(obj->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
				"gpu program object must have names");
			return;
		}

		// Must have a language type
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line,
				"gpu program object require language declarations");
			return;
		}

		// Get the language
		String language;
		if(!getString(obj->values.front(), &language))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
			return;
		}

		if(language == "asm")
			translateGpuProgram(compiler, obj);
		else if(language == "unified")
			translateUnifiedGpuProgram(compiler, obj);
		else
			translateHighLevelGpuProgram(compiler, obj);
	}
	//-------------------------------------------------------------------------
	void GpuProgramTranslator::translateGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
	{
		list<std::pair<String,String> >::type customParameters;
		String syntax, source;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->id == ID_SOURCE)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							source = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"source file expected");
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"source file expected");
					}
				}
				else if(prop->id == ID_SYNTAX)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							syntax = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"syntax string expected");
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"syntax string expected");
					}
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;
							value += ((AtomAbstractNode*)(*i).get())->value;
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
					params = *i;
				else
					processNode(compiler, *i);
			}
		}
		
		if (!GpuProgramManager::getSingleton().isSyntaxSupported(syntax))
		{
			compiler->addError(ScriptCompiler::CE_UNSUPPORTEDBYRENDERSYSTEM, obj->file, obj->line);
			//Register the unsupported program so that materials that use it know that
			//it exists but is unsupported
			GpuProgramPtr unsupportedProg = GpuProgramManager::getSingleton().create(obj->name, 
					compiler->getResourceGroup(), translateIDToGpuProgramType(obj->id), syntax);
			return;
		}

		// Allocate the program
		GpuProgram *prog = 0;
		CreateGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), source, syntax, translateIDToGpuProgramType(obj->id));
		bool processed = compiler->_fireEvent(&evt, (void*)&prog);
		if(!processed)
		{
			prog = reinterpret_cast<GpuProgram*>(GpuProgramManager::getSingleton().createProgram(obj->name, 
					compiler->getResourceGroup(), source, translateIDToGpuProgramType(obj->id), syntax).get());
		}

		// Check that allocation worked
		if(prog == 0)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
				"gpu program \"" + obj->name + "\" could not be created");
			return;
		}

		obj->context = Any(prog);

		prog->setMorphAnimationIncluded(false);
		prog->setPoseAnimationIncluded(0);
		prog->setSkeletalAnimationIncluded(false);
		prog->setVertexTextureFetchRequired(false);
		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, ptr, reinterpret_cast<ObjectAbstractNode*>(params.get()));
		}
	}
	//-------------------------------------------------------------------------
	void GpuProgramTranslator::translateUnifiedGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
	{
		list<std::pair<String,String> >::type customParameters;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->name == "delegate")
				{
					String value;
					if(!prop->values.empty() && prop->values.front()->type == ANT_ATOM)
						value = ((AtomAbstractNode*)prop->values.front().get())->value;
					
					ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, value);
					compiler->_fireEvent(&evt, 0);
					customParameters.push_back(std::make_pair("delegate", evt.mName));
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;
							value += ((AtomAbstractNode*)(*i).get())->value;
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
					params = *i;
				else
					processNode(compiler, *i);
			}
		}

		// Allocate the program
		HighLevelGpuProgram *prog = 0;
		CreateHighLevelGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), "", "unified", translateIDToGpuProgramType(obj->id));
		bool processed = compiler->_fireEvent(&evt, (void*)&prog);

		if(!processed)
		{
			prog = reinterpret_cast<HighLevelGpuProgram*>(
				HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, compiler->getResourceGroup(), 
				"unified", translateIDToGpuProgramType(obj->id)).get());
		}

		// Check that allocation worked
		if(prog == 0)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
				"gpu program \"" + obj->name + "\" could not be created");
			return;
		}

		obj->context = Any(prog);

		prog->setMorphAnimationIncluded(false);
		prog->setPoseAnimationIncluded(0);
		prog->setSkeletalAnimationIncluded(false);
		prog->setVertexTextureFetchRequired(false);
		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, ptr, reinterpret_cast<ObjectAbstractNode*>(params.get()));
		}

	}
	//-------------------------------------------------------------------------
	void GpuProgramTranslator::translateHighLevelGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
	{
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
			return;
		}
		String language;
		if(!getString(obj->values.front(), &language))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
			return;
		}

		list<std::pair<String,String> >::type customParameters;
		String source;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->id == ID_SOURCE)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							source = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"source file expected");
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
							"source file expected");
					}
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;

							if(prop->name == "attach")
							{
								ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, ((AtomAbstractNode*)(*i).get())->value);
								compiler->_fireEvent(&evt, 0);
								value += evt.mName;
							}
							else
							{
								value += ((AtomAbstractNode*)(*i).get())->value;
							}
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
					params = *i;
				else
					processNode(compiler, *i);
			}
		}

		// Allocate the program
		HighLevelGpuProgram *prog = 0;
		CreateHighLevelGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), source, language, 
			translateIDToGpuProgramType(obj->id));
		bool processed = compiler->_fireEvent(&evt, (void*)&prog);
		if(!processed)
		{
			prog = reinterpret_cast<HighLevelGpuProgram*>(
				HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, compiler->getResourceGroup(), 
				language, translateIDToGpuProgramType(obj->id)).get());
			prog->setSourceFile(source);
		}

		// Check that allocation worked
		if(prog == 0)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
				"gpu program \"" + obj->name + "\" could not be created");
			return;
		}

		obj->context = Any(prog);

		prog->setMorphAnimationIncluded(false);
		prog->setPoseAnimationIncluded(0);
		prog->setSkeletalAnimationIncluded(false);
		prog->setVertexTextureFetchRequired(false);
		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramTranslator::translateProgramParameters(compiler, ptr, reinterpret_cast<ObjectAbstractNode*>(params.get()));
		}	

	}
	//-------------------------------------------------------------------------
	void GpuProgramTranslator::translateProgramParameters(ScriptCompiler *compiler, GpuProgramParametersSharedPtr params, ObjectAbstractNode *obj)
	{
		size_t animParametricsCount = 0;

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_SHARED_PARAMS_REF:
					{
						if(prop->values.size() != 1)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"shared_params_ref requires a single parameter");
							continue;
						}

						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						if((*i0)->type != ANT_ATOM)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"shared parameter set name expected");
							continue;
						}
						AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();

						try 
						{
							params->addSharedParameters(atom0->value);
						}
						catch(Exception& e)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								e.getDescription());
						}
					}
					break;
				case ID_PARAM_INDEXED:
				case ID_PARAM_NAMED:
					{
						if(prop->values.size() >= 3)
						{
							bool named = (prop->id == ID_PARAM_NAMED);
							AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), 
								k = getNodeAt(prop->values, 2);

							if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"name or index and parameter type expected");
								return;
							}

							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
							if(!named && !StringConverter::isNumber(atom0->value))
							{
								compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
									"parameter index expected");
								return;
							}

							String name;
							size_t index = 0;
							// Assign the name/index
							if(named)
								name = atom0->value;
							else
								index = StringConverter::parseInt(atom0->value);
						
							// Determine the type
							if(atom1->value == "matrix4x4")
							{	
								Matrix4 m;
								if(getMatrix4(k, prop->values.end(), &m))
								{
									try
									{
										if(named)
											params->setNamedConstant(name, m);
										else
											params->setConstant(index, m);
									}
									catch(...)
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"setting matrix4x4 parameter failed");
									}
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"incorrect matrix4x4 declaration");
								}
							}
							else
							{
								// Find the number of parameters
								bool isValid = true;
								GpuProgramParameters::ElementType type = GpuProgramParameters::ET_REAL;
								int count = 0;
								if(atom1->value.find("float") != String::npos)
								{
									type = GpuProgramParameters::ET_REAL;
									if(atom1->value.size() >= 6)
										count = StringConverter::parseInt(atom1->value.substr(5));
									else
									{
										count = 1;
									}
								}
								else if(atom1->value.find("int") != String::npos)
								{
									type = GpuProgramParameters::ET_INT;
									if(atom1->value.size() >= 4)
										count = StringConverter::parseInt(atom1->value.substr(3));
									else
									{
										count = 1;
									}
								}
								else
								{
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
										"incorrect type specified; only variants of int and float allowed");
									isValid = false;
								}

								if(isValid)
								{
									// First, clear out any offending auto constants
									if(named)
										params->clearNamedAutoConstant(name);
									else
										params->clearAutoConstant(index);

									int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;
									if(type == GpuProgramParameters::ET_INT)
									{
										int *vals = OGRE_ALLOC_T(int, roundedCount, MEMCATEGORY_SCRIPTING);
										if(getInts(k, prop->values.end(), vals, roundedCount))
										{
											try
											{
												if(named)
													params->setNamedConstant(name, vals, count, 1);
												else
													params->setConstant(index, vals, roundedCount/4);
											}
											catch(...)
											{
												compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
													"setting of constant failed");
											}
										}
										else
										{
											compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
												"incorrect integer constant declaration");
										}
										OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
									}
									else
									{
										float *vals = OGRE_ALLOC_T(float, roundedCount, MEMCATEGORY_SCRIPTING);
										if(getFloats(k, prop->values.end(), vals, roundedCount))
										{
											try
											{
												if(named)
													params->setNamedConstant(name, vals, count, 1);
												else
													params->setConstant(index, vals, roundedCount/4);
											}
											catch(...)
											{
												compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
													"setting of constant failed");
											}
										}
										else
										{
											compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
												"incorrect float constant declaration");
										}
										OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
									}
								}
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"param_named and param_indexed properties requires at least 3 arguments");
						}
					}
					break;
				case ID_PARAM_INDEXED_AUTO:
				case ID_PARAM_NAMED_AUTO:
					{
						bool named = (prop->id == ID_PARAM_NAMED_AUTO);
						String name;
						size_t index = 0;

						if(prop->values.size() >= 2)
						{
							AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
								i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3);
							if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
									"name or index and auto constant type expected");
								return;
							}
							AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
							if(!named && !StringConverter::isNumber(atom0->value))
							{
								compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
									"parameter index expected");
								return;
							}

							if(named)
								name = atom0->value;
							else
								index = StringConverter::parseInt(atom0->value);

							// Look up the auto constant
							StringUtil::toLowerCase(atom1->value);
							const GpuProgramParameters::AutoConstantDefinition *def =
								GpuProgramParameters::getAutoConstantDefinition(atom1->value);
							if(def)
							{
								switch(def->dataType)
								{
								case GpuProgramParameters::ACDT_NONE:
									// Set the auto constant
									try
									{
										if(named)
											params->setNamedAutoConstant(name, def->acType);
										else
											params->setAutoConstant(index, def->acType);
									}
									catch(...)
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"setting of constant failed");
									}
									break;
								case GpuProgramParameters::ACDT_INT:
									if(def->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
									{
										try
										{
											if(named)
												params->setNamedAutoConstant(name, def->acType, animParametricsCount++);
											else
												params->setAutoConstant(index, def->acType, animParametricsCount++);
										}
										catch(...)
										{
											compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
												"setting of constant failed");
										}
									}
									else
									{
										// Only certain texture projection auto params will assume 0
										// Otherwise we will expect that 3rd parameter
										if(i2 == prop->values.end())
										{
											if(def->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX ||
												def->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
												def->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
												def->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX
												)
											{
												try
												{
													if(named)
														params->setNamedAutoConstant(name, def->acType, 0);
													else
														params->setAutoConstant(index, def->acType, 0);
												}
												catch(...)
												{
													compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
														"setting of constant failed");
												}
											}
											else
											{
												compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
													"extra parameters required by constant definition " + atom1->value);
											}
										}
										else
										{
											bool success = false;
											uint32 extraInfo = 0;
											if(i3 == prop->values.end())
											{ // Handle only one extra value
												if(getUInt(*i2, &extraInfo))
												{
													success = true;
												}
											}
											else
											{ // Handle two extra values
												uint32 extraInfo1 = 0, extraInfo2 = 0;
												if(getUInt(*i2, &extraInfo1) && getUInt(*i3, &extraInfo2))
												{
													extraInfo = extraInfo1 | (extraInfo2 << 16);
													success = true;
												}
											}

											if(success)
											{
												try
												{
													if(named)
														params->setNamedAutoConstant(name, def->acType, extraInfo);
													else
														params->setAutoConstant(index, def->acType, extraInfo);
												}
												catch(...)
												{
													compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
														"setting of constant failed");
												}
											}
											else
											{
												compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
													"invalid auto constant extra info parameter");
											}
										}
									}
									break;
								case GpuProgramParameters::ACDT_REAL:
									if(def->acType == GpuProgramParameters::ACT_TIME ||
										def->acType == GpuProgramParameters::ACT_FRAME_TIME)
									{
										Real f = 1.0f;
										if(i2 != prop->values.end())
											getReal(*i2, &f);
										
										try
										{
											if(named)
												params->setNamedAutoConstantReal(name, def->acType, f);
											else
												params->setAutoConstantReal(index, def->acType, f);
										}
										catch(...)
										{
											compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
												"setting of constant failed");
										}
									}
									else
									{
										if(i2 != prop->values.end())
										{
											Real extraInfo = 0.0f;
											if(getReal(*i2, &extraInfo))
											{
												try
												{
													if(named)
														params->setNamedAutoConstantReal(name, def->acType, extraInfo);
													else
														params->setAutoConstantReal(index, def->acType, extraInfo);
												}
												catch(...)
												{
													compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
														"setting of constant failed");
												}
											}
											else
											{
												compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
													"incorrect float argument definition in extra parameters");
											}
										}
										else
										{
											compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
												"extra parameters required by constant definition " + atom1->value);
										}
									}
									break;
								}
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}
	/**************************************************************************
	* SharedParamsTranslator
	*************************************************************************/
	SharedParamsTranslator::SharedParamsTranslator()
	{
	}
	//-------------------------------------------------------------------------
	void SharedParamsTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		// Must have a name
		if(obj->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
				"shared_params must be given a name");
			return;
		}

		GpuSharedParameters* sharedParams = 0;
		CreateGpuSharedParametersScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
		bool processed = compiler->_fireEvent(&evt, (void*)&sharedParams);

		if(!processed)
		{
			sharedParams = GpuProgramManager::getSingleton().createSharedParameters(obj->name).get();
		}
		
		if(!sharedParams)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}


		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_SHARED_PARAM_NAMED:
					{
						if(prop->values.size() < 2)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"shared_param_named - expected 2 or more arguments");
							continue;
						}

						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);

						if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"name and parameter type expected");
							continue;
						}


						AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();

						String pName = atom0->value;
						GpuConstantType constType;
						size_t arraySz = 1;
						if (!getConstantType(i1, &constType))
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								"invalid parameter type");
							continue;
						}

						bool isFloat = GpuConstantDefinition::isFloat(constType);

						FloatConstantList mFloats;
						IntConstantList mInts;

						AbstractNodeList::const_iterator otherValsi = prop->values.begin();
						std::advance(otherValsi, 2);

						for (; otherValsi != prop->values.end(); ++otherValsi)
						{
							if((*otherValsi)->type != ANT_ATOM)
								continue;

							AtomAbstractNode *atom = (AtomAbstractNode*)(*otherValsi).get();

							if (atom->value.at(0) == '[' && atom->value.at(atom->value.size() - 1) == ']')
							{
								String arrayStr = atom->value.substr(1, atom->value.size() - 2);
								if(!StringConverter::isNumber(arrayStr))
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										"invalid array size");
									continue;
								}
								arraySz = StringConverter::parseInt(arrayStr);						
							}
							else
							{
								if(!StringConverter::isNumber(atom->value))
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
										atom->value + " invalid - extra parameters to shared_param_named must be numbers");
									continue;
								}
								if (isFloat)
									mFloats.push_back((float)StringConverter::parseReal(atom->value));
								else
									mInts.push_back(StringConverter::parseInt(atom->value));
							}

						} // each extra param

						// define constant entry
						try 
						{
							sharedParams->addConstantDefinition(pName, constType, arraySz);
						}
						catch(Exception& e)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
								e.getDescription());
							continue;
						}


						// initial values
						size_t elemsExpected = GpuConstantDefinition::getElementSize(constType, false) * arraySz;
						size_t elemsFound = isFloat ? mFloats.size() : mInts.size();
						if (elemsFound)
						{
							if (elemsExpected != elemsFound)
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, 
									"Wrong number of values supplied for parameter type");
								continue;
							}

							if (isFloat)
								sharedParams->setNamedConstant(pName, &mFloats[0], elemsFound);
							else
								sharedParams->setNamedConstant(pName, &mInts[0], elemsFound);

						}

					}
				}
			}
		}



	}

	/**************************************************************************
	 * ParticleSystemTranslator
	 *************************************************************************/
	ParticleSystemTranslator::ParticleSystemTranslator()
		:mSystem(0)
	{
	}

	void ParticleSystemTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
		// Find the name
		if(obj->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		// Allocate the particle system
		CreateParticleSystemScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
		bool processed = compiler->_fireEvent(&evt, (void*)&mSystem);

		if(!processed)
		{
			mSystem = ParticleSystemManager::getSingleton().createTemplate(obj->name, compiler->getResourceGroup());
		}

		if(!mSystem)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}
		
		mSystem->_notifyOrigin(obj->file);

		mSystem->removeAllEmitters();
		mSystem->removeAllAffectors();

		obj->context = Any(mSystem);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_MATERIAL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							String name = ((AtomAbstractNode*)prop->values.front().get())->value;
							
							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, name);
							compiler->_fireEvent(&evt, 0);

							if(!mSystem->setParameter("material", evt.mName))
							{
								if(mSystem->getRenderer())
								{
									if(!mSystem->getRenderer()->setParameter("material", evt.mName))
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
											"material property could not be set with material \"" + evt.mName + "\"");
								}
							}
						}
					}
					break;
				default:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						String name = prop->name, value;

						// Glob the values together
						for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
						{
							if((*i)->type == ANT_ATOM)
							{
								if(value.empty())
									value = ((AtomAbstractNode*)(*i).get())->value;
								else
									value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
							}
							else
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
								return;
							}
						}

						if(!mSystem->setParameter(name, value))
						{
							if(mSystem->getRenderer())
							{
								if(!mSystem->getRenderer()->setParameter(name, value))
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
					}
				}
			}
			else
			{
				processNode(compiler, *i);
			}
		}
	}

	/**************************************************************************
	 * ParticleEmitterTranslator
	 *************************************************************************/
	ParticleEmitterTranslator::ParticleEmitterTranslator()
		:mEmitter(0)
	{
	}
	//-------------------------------------------------------------------------
	void ParticleEmitterTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		// Must have a type as the first value
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
			return;
		}

		String type;
		if(!getString(obj->values.front(), &type))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
			return;
		}

		ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);
		mEmitter = system->addEmitter(type);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				String value;

				// Glob the values together
				for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
				{
					if((*i)->type == ANT_ATOM)
					{
						if(value.empty())
							value = ((AtomAbstractNode*)(*i).get())->value;
						else
							value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						break;
					}
				}

				if(!mEmitter->setParameter(prop->name, value))
				{
					compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else
			{
				processNode(compiler, *i);
			}
		}
	}

	/**************************************************************************
	 * ParticleAffectorTranslator
	 *************************************************************************/
	ParticleAffectorTranslator::ParticleAffectorTranslator()
		:mAffector(0)
	{
	}
	//-------------------------------------------------------------------------
	void ParticleAffectorTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		// Must have a type as the first value
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
			return;
		}

		String type;
		if(!getString(obj->values.front(), &type))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
			return;
		}

		ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);
		mAffector = system->addAffector(type);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				String value;

				// Glob the values together
				for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
				{
					if((*i)->type == ANT_ATOM)
					{
						if(value.empty())
							value = ((AtomAbstractNode*)(*i).get())->value;
						else
							value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
					}
					else
					{
						compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						break;
					}
				}

				if(!mAffector->setParameter(prop->name, value))
				{
					compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else
			{
				processNode(compiler, *i);
			}
		}
	}

	/**************************************************************************
	 * CompositorTranslator
	 *************************************************************************/
	CompositorTranslator::CompositorTranslator()
		:mCompositor(0)
	{
	}

	void CompositorTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
		if(obj->name.empty())
		{
			compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		// Create the compositor
		CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
		bool processed = compiler->_fireEvent(&evt, (void*)&mCompositor);
		
		if(!processed)
		{
			mCompositor = reinterpret_cast<Compositor*>(CompositorManager::getSingleton().create(obj->name, 
				compiler->getResourceGroup()).get());
		}

		if(mCompositor == 0)
		{
			compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		// Prepare the compositor
		mCompositor->removeAllTechniques();
		mCompositor->_notifyOrigin(obj->file);
		obj->context = Any(mCompositor);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else
			{
				compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
					"token not recognized");
			}
		}
	}

	/**************************************************************************
	 * CompositionTechniqueTranslator
	 *************************************************************************/
	CompositionTechniqueTranslator::CompositionTechniqueTranslator()
		:mTechnique(0)
	{
	}
	//-------------------------------------------------------------------------
	void CompositionTechniqueTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		Compositor *compositor = any_cast<Compositor*>(obj->parent->context);
		mTechnique = compositor->createTechnique();
		obj->context = Any(mTechnique);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_TEXTURE:
					{
						size_t atomIndex = 1;

						AbstractNodeList::const_iterator i = getNodeAt(prop->values, 0);

						if((*i)->type != ANT_ATOM)
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
						// Save the first atom, should be name
						AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i).get();

						size_t width = 0, height = 0;
						float widthFactor = 1.0f, heightFactor = 1.0f;
						bool widthSet = false, heightSet = false, formatSet = false;
						bool pooled = false;
						bool hwGammaWrite = false;
						bool fsaa = true;
						CompositionTechnique::TextureScope scope = CompositionTechnique::TS_LOCAL;
						Ogre::PixelFormatList formats;

						while (atomIndex < prop->values.size())
						{
							i = getNodeAt(prop->values, static_cast<int>(atomIndex++));
							if((*i)->type != ANT_ATOM)
							{
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
								return;
							}
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i).get();

							switch(atom->id)
							{
							case ID_TARGET_WIDTH:
								width = 0;
								widthSet = true;
								break;
							case ID_TARGET_HEIGHT:
								height = 0;
								heightSet = true;
								break;
							case ID_TARGET_WIDTH_SCALED:
							case ID_TARGET_HEIGHT_SCALED:
								{
									bool *pSetFlag;
									size_t *pSize;
									float *pFactor;
									if (atom->id == ID_TARGET_WIDTH_SCALED)
									{
										pSetFlag = &widthSet;
										pSize = &width;
										pFactor = &widthFactor;
									}
									else
									{
										pSetFlag = &heightSet;
										pSize = &height;
										pFactor = &heightFactor;
									}
									// advance to next to get scaling
									i = getNodeAt(prop->values, static_cast<int>(atomIndex++));
									if(prop->values.end() == i || (*i)->type != ANT_ATOM)
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
										return;
									}
									atom = (AtomAbstractNode*)(*i).get();
									if (!StringConverter::isNumber(atom->value))
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
										return;
									}

									*pSize = 0;
									*pFactor = StringConverter::parseReal(atom->value);
									*pSetFlag = true;
								}
								break;
							case ID_POOLED:
								pooled = true;
								break;
							case ID_SCOPE_LOCAL:
								scope = CompositionTechnique::TS_LOCAL;
								break;
							case ID_SCOPE_CHAIN:
								scope = CompositionTechnique::TS_CHAIN;
								break;
							case ID_SCOPE_GLOBAL:
								scope = CompositionTechnique::TS_GLOBAL;
								break;
							case ID_GAMMA:
								hwGammaWrite = true;
								break;
							case ID_NO_FSAA:
								fsaa = false;
								break;
							default:
								if (StringConverter::isNumber(atom->value))
								{
									if (atomIndex == 2)
									{
										width = StringConverter::parseInt(atom->value);
										widthSet = true;
									}
									else if (atomIndex == 3)
									{
										height = StringConverter::parseInt(atom->value);
										heightSet = true;
									}
									else
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
										return;
									}
								}
								else
								{
									// pixel format?
									PixelFormat format = PixelUtil::getFormatFromName(atom->value, true);
									if (format == PF_UNKNOWN)
									{
										compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
										return;
									}
									formats.push_back(format);
									formatSet = true;
								}

							}
						}
						if (!widthSet || !heightSet || !formatSet)
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
				

						// No errors, create
						CompositionTechnique::TextureDefinition *def = mTechnique->createTextureDefinition(atom0->value);
						def->width = width;
						def->height = height;
						def->widthFactor = widthFactor;
						def->heightFactor = heightFactor;
						def->formatList = formats;
						def->hwGammaWrite = hwGammaWrite;
						def->fsaa = fsaa;
						def->pooled = pooled;
						def->scope = scope;
					}
					break;
				case ID_TEXTURE_REF:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() != 3)
					{
						compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"texture_ref only supports 3 argument");
					}
					else
					{
						String texName, refCompName, refTexName;

						AbstractNodeList::const_iterator i = getNodeAt(prop->values, 0);
						if(!getString(*i, &texName))
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"texture_ref must have 3 string arguments");

						i = getNodeAt(prop->values, 1);
						if(!getString(*i, &refCompName))
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"texture_ref must have 3 string arguments");

						i = getNodeAt(prop->values, 2);
						if(!getString(*i, &refTexName))
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"texture_ref must have 3 string arguments");

						CompositionTechnique::TextureDefinition* refTexDef = 
							mTechnique->createTextureDefinition(texName);

						refTexDef->refCompName = refCompName;
						refTexDef->refTexName = refTexName;
					}
					break;
				case ID_SCHEME:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"scheme only supports 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						String scheme;
						if(getString(*i0, &scheme))
							mTechnique->setSchemeName(scheme);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"scheme must have 1 string argument");
					}
					break;
				case ID_COMPOSITOR_LOGIC:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
					}
					else if(prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
							"compositor logic only supports 1 argument");
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
						String logicName;
						if(getString(*i0, &logicName))
							mTechnique->setCompositorLogicName(logicName);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
							"compositor logic must have 1 string argument");
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}

	/**************************************************************************
	 * CompositionTargetPass
	 *************************************************************************/
	CompositionTargetPassTranslator::CompositionTargetPassTranslator()
		:mTarget(0)
	{
	}
	//-------------------------------------------------------------------------
	void CompositionTargetPassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		CompositionTechnique *technique = any_cast<CompositionTechnique*>(obj->parent->context);
		if(obj->id == ID_TARGET)
		{
			mTarget = technique->createTargetPass();
			if(!obj->name.empty())
			{
				String name = obj->name;

				mTarget->setOutputName(name);
			}
		}
		else if(obj->id == ID_TARGET_OUTPUT)
		{
			mTarget = technique->getOutputTargetPass();
		}
		obj->context = Any(mTarget);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_INPUT:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
							switch(atom->id)
							{
							case ID_NONE:
								mTarget->setInputMode(CompositionTargetPass::IM_NONE);
								break;
							case ID_PREVIOUS:
								mTarget->setInputMode(CompositionTargetPass::IM_PREVIOUS);
								break;
							default:
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_ONLY_INITIAL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						bool val;
						if(getBoolean(prop->values.front(), &val))
						{
							mTarget->setOnlyInitial(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_VISIBILITY_MASK:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						uint32 val;
						if(getUInt(prop->values.front(), &val))
						{
							mTarget->setVisibilityMask(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_LOD_BIAS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						float val;
						if(getFloat(prop->values.front(), &val))
						{
							mTarget->setLodBias(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_MATERIAL_SCHEME:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						String val;
						if(getString(prop->values.front(), &val))
						{
							mTarget->setMaterialScheme(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_SHADOWS_ENABLED:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						bool val;
						if(getBoolean(prop->values.front(), &val))
						{
							mTarget->setShadowsEnabled(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}
		
	/**************************************************************************
	 * CompositionPassTranslator
	 *************************************************************************/
	CompositionPassTranslator::CompositionPassTranslator()
		:mPass(0)
	{
	}

	void CompositionPassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		CompositionTargetPass *target = any_cast<CompositionTargetPass*>(obj->parent->context);
		mPass = target->createPass();
		obj->context = Any(mPass);

		// The name is the type of the pass
		if(obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
			return;
		}
		String type;
		if(!getString(obj->values.front(), &type))
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
			return;
		}

		if(type == "clear")
			mPass->setType(CompositionPass::PT_CLEAR);
		else if(type == "stencil")
			mPass->setType(CompositionPass::PT_STENCIL);
		else if(type == "render_quad")
			mPass->setType(CompositionPass::PT_RENDERQUAD);
		else if(type == "render_scene")
			mPass->setType(CompositionPass::PT_RENDERSCENE);
		else if(type == "render_custom") {
			mPass->setType(CompositionPass::PT_RENDERCUSTOM);
			String customType;
			//This is the ugly one liner for safe access to the second parameter.
			if (obj->values.size() < 2 || !getString(*(++(obj->values.begin())), &customType))
			{
				compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
				return;
			}
			mPass->setCustomType(customType);
		}
		else
		{
			compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line,
				"pass types must be \"clear\", \"stencil\", \"render_quad\", \"render_scene\" or \"render_custom\".");
			return;
		}

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_MATERIAL:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						String val;
						if(getString(prop->values.front(), &val))
						{
							ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, val);
							compiler->_fireEvent(&evt, 0);
							mPass->setMaterialName(evt.mName);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_INPUT:
					if(prop->values.size() < 2)
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 3)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
						uint32 id;
						String name;
						if(getUInt(*i0, &id) && getString(*i1, &name))
						{
							uint32 index = 0;
							if(i2 != prop->values.end())
							{
								if(!getUInt(*i2, &index))
								{
									compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
									return;
								}
							}

							mPass->setInput(id, name, index);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_IDENTIFIER:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						uint32 val;
						if(getUInt(prop->values.front(), &val))
						{
							mPass->setIdentifier(val);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_FIRST_RENDER_QUEUE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						uint32 val;
						if(getUInt(prop->values.front(), &val))
						{
							mPass->setFirstRenderQueue(static_cast<uint8>(val));
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_LAST_RENDER_QUEUE:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						uint32 val;
						if(getUInt(prop->values.front(), &val))
						{
							mPass->setLastRenderQueue(static_cast<uint8>(val));
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				case ID_QUAD_NORMALS:
					if(prop->values.empty())
					{
						compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
						return;
					}
					else if (prop->values.size() > 1)
					{
						compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
						return;
					}
					else
					{
						if(prop->values.front()->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = reinterpret_cast<AtomAbstractNode*>(prop->values.front().get());
							if(atom->id == ID_CAMERA_FAR_CORNERS_VIEW_SPACE)
								mPass->setQuadFarCorners(true, true);
							else if(atom->id == ID_CAMERA_FAR_CORNERS_WORLD_SPACE)
								mPass->setQuadFarCorners(true, false);
							else
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}

	/**************************************************************************
	* CompositionPassClearTranslator
	*************************************************************************/
	CompositionPassClearTranslator::CompositionPassClearTranslator()
		:mPass(0)
	{
	}

	void CompositionPassClearTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		mPass = any_cast<CompositionPass*>(obj->parent->context);

		// Should be no parameters, just children
		if(!obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, obj->file, obj->line);
		}

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_BUFFERS:
					{
						uint32 buffers = 0;
						for(AbstractNodeList::iterator k = prop->values.begin(); k != prop->values.end(); ++k)
						{
							if((*k)->type == ANT_ATOM)
							{
								switch(((AtomAbstractNode*)(*k).get())->id)
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
									compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
								}
							}
							else
								compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						mPass->setClearBuffers(buffers);
					}
					break;
				case ID_COLOUR_VALUE:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
							return;
						}
						ColourValue val;
						if(getColour(prop->values.begin(), prop->values.end(), &val))
							mPass->setClearColour(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_DEPTH_VALUE:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
							return;
						}
						Real val;
						if(getReal(prop->values.front(), &val))
							mPass->setClearDepth(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_STENCIL_VALUE:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
							return;
						}
						uint32 val;
						if(getUInt(prop->values.front(), &val))
							mPass->setClearStencil(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}

	/**************************************************************************
	* CompositionPassStencilTranslator
	*************************************************************************/
	CompositionPassStencilTranslator::CompositionPassStencilTranslator()
		:mPass(0)
	{
	}

	void CompositionPassStencilTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
	{
		ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

		mPass = any_cast<CompositionPass*>(obj->parent->context);

		// Should be no parameters, just children
		if(!obj->values.empty())
		{
			compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, obj->file, obj->line);
		}

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				processNode(compiler, *i);
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				switch(prop->id)
				{
				case ID_CHECK:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						bool val;
						if(getBoolean(prop->values.front(), &val))
							mPass->setStencilCheck(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_COMP_FUNC:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						CompareFunction func;
						if(getCompareFunction(prop->values.front(), &func))
							mPass->setStencilFunc(func);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_REF_VALUE:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
							return;
						}
						uint32 val;
						if(getUInt(prop->values.front(), &val))
							mPass->setStencilRefValue(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_MASK:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
							return;
						}
						uint32 val;
						if(getUInt(prop->values.front(), &val))
							mPass->setStencilMask(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_FAIL_OP:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						StencilOperation val;
						if(getStencilOp(prop->values.front(), &val))
							mPass->setStencilFailOp(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_DEPTH_FAIL_OP:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						StencilOperation val;
						if(getStencilOp(prop->values.front(), &val))
							mPass->setStencilDepthFailOp(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_PASS_OP:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						StencilOperation val;
						if(getStencilOp(prop->values.front(), &val))
							mPass->setStencilPassOp(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				case ID_TWO_SIDED:
					{
						if(prop->values.empty())
						{
							compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
							return;
						}
						bool val;
						if(getBoolean(prop->values.front(), &val))
							mPass->setStencilTwoSidedOperation(val);
						else
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					break;
				default:
					compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
						"token \"" + prop->name + "\" is not recognized");
				}
			}
		}
	}

	/**************************************************************************
	 * BuiltinScriptTranslatorManager
	 *************************************************************************/
	BuiltinScriptTranslatorManager::BuiltinScriptTranslatorManager()
	{
	}
	//-------------------------------------------------------------------------
	size_t BuiltinScriptTranslatorManager::getNumTranslators() const
	{
		return 12;
	}
	//-------------------------------------------------------------------------
	ScriptTranslator *BuiltinScriptTranslatorManager::getTranslator(const AbstractNodePtr &node)
	{
		ScriptTranslator *translator = 0;

		if(node->type == ANT_OBJECT)
		{
			ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
			ObjectAbstractNode *parent = obj->parent ? reinterpret_cast<ObjectAbstractNode*>(obj->parent) : 0;
			if(obj->id == ID_MATERIAL)
				translator = &mMaterialTranslator;
			else if(obj->id == ID_TECHNIQUE && parent && parent->id == ID_MATERIAL)
				translator = &mTechniqueTranslator;
			else if(obj->id == ID_PASS && parent && parent->id == ID_TECHNIQUE)
				translator = &mPassTranslator;
			else if(obj->id == ID_TEXTURE_UNIT && parent && parent->id == ID_PASS)
				translator = &mTextureUnitTranslator;
			else if(obj->id == ID_TEXTURE_SOURCE && parent && parent->id == ID_TEXTURE_UNIT)
				translator = &mTextureSourceTranslator;
			else if(obj->id == ID_FRAGMENT_PROGRAM || obj->id == ID_VERTEX_PROGRAM || obj->id == ID_GEOMETRY_PROGRAM)
				translator = &mGpuProgramTranslator;
			else if(obj->id == ID_SHARED_PARAMS)
				translator = &mSharedParamsTranslator;
			else if(obj->id == ID_PARTICLE_SYSTEM)
				translator = &mParticleSystemTranslator;
			else if(obj->id == ID_EMITTER)
				translator = &mParticleEmitterTranslator;
			else if(obj->id == ID_AFFECTOR)
				translator = &mParticleAffectorTranslator;
			else if(obj->id == ID_COMPOSITOR)
				translator = &mCompositorTranslator;
			else if(obj->id == ID_TECHNIQUE && parent && parent->id == ID_COMPOSITOR)
				translator = &mCompositionTechniqueTranslator;
			else if((obj->id == ID_TARGET || obj->id == ID_TARGET_OUTPUT) && parent && parent->id == ID_TECHNIQUE)
				translator = &mCompositionTargetPassTranslator;
			else if(obj->id == ID_PASS && parent && (parent->id == ID_TARGET || parent->id == ID_TARGET_OUTPUT))
				translator = &mCompositionPassTranslator;
			else if(obj->id == ID_CLEAR && parent && parent->id == ID_PASS)
				translator = &mCompositionPassClearTranslator;
			else if(obj->id == ID_STENCIL && parent && parent->id == ID_PASS)
				translator = &mCompositionPassStencilTranslator;
		}

		return translator;
	}
}
