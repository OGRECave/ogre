/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreStableHeaders.h"
#include "OgreScriptCompiler.h"
#include "OgreScriptLexer.h"
#include "OgreScriptParser.h"
#include "OgreScriptTranslator.h"

namespace Ogre
{
	// AbstractNode
	AbstractNode::AbstractNode(AbstractNode *ptr)
		:parent(ptr), type(ANT_UNKNOWN), line(0)
	{}

	// AtomAbstractNode
	AtomAbstractNode::AtomAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), id(0)
	{
		type = ANT_ATOM;
	}

	AbstractNode *AtomAbstractNode::clone() const
	{
		AtomAbstractNode *node = OGRE_NEW AtomAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->id = id;
		node->type = type;
		node->value = value;
		return node;
	}

	String AtomAbstractNode::getValue() const
	{
		return value;
	}

	// ObjectAbstractNode
	ObjectAbstractNode::ObjectAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), abstract(false), id(0)
	{
		type = ANT_OBJECT;
	}

	AbstractNode *ObjectAbstractNode::clone() const
	{
		ObjectAbstractNode *node = OGRE_NEW ObjectAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		node->cls = cls;
		node->id = id;
		node->abstract = abstract;
		for(AbstractNodeList::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->children.push_back(newNode);
		}
		for(AbstractNodeList::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->values.push_back(newNode);
		}
		node->mEnv = mEnv;
		return node;
	}

	String ObjectAbstractNode::getValue() const
	{
		return cls;
	}

	void ObjectAbstractNode::addVariable(const Ogre::String &name)
	{
		mEnv.insert(std::make_pair(name, ""));
	}

	void ObjectAbstractNode::setVariable(const Ogre::String &name, const Ogre::String &value)
	{
		mEnv[name] = value;
	}

	std::pair<bool,String> ObjectAbstractNode::getVariable(const String &name) const
	{
		map<String,String>::type::const_iterator i = mEnv.find(name);
		if(i != mEnv.end())
			return std::make_pair(true, i->second);

		ObjectAbstractNode *parent = (ObjectAbstractNode*)this->parent;
		while(parent)
		{
			i = parent->mEnv.find(name);
			if(i != parent->mEnv.end())
				return std::make_pair(true, i->second);
			parent = (ObjectAbstractNode*)parent->parent;
		}
		return std::make_pair(false, "");
	}

	const map<String,String>::type &ObjectAbstractNode::getVariables() const
	{
		return mEnv;
	}

	// PropertyAbstractNode
	PropertyAbstractNode::PropertyAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), id(0)
	{
		type = ANT_PROPERTY;
	}

	AbstractNode *PropertyAbstractNode::clone() const
	{
		PropertyAbstractNode *node = OGRE_NEW PropertyAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		node->id = id;
		for(AbstractNodeList::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->values.push_back(newNode);
		}
		return node;
	}

	String PropertyAbstractNode::getValue() const
	{
		return name;
	}

	// ImportAbstractNode
	ImportAbstractNode::ImportAbstractNode()
		:AbstractNode(0)
	{
		type = ANT_IMPORT;
	}

	AbstractNode *ImportAbstractNode::clone() const
	{
		ImportAbstractNode *node = OGRE_NEW ImportAbstractNode();
		node->file = file;
		node->line = line;
		node->type = type;
		node->target = target;
		node->source = source;
		return node;
	}

	String ImportAbstractNode::getValue() const
	{
		return target;
	}

	// VariableAccessAbstractNode
	VariableAccessAbstractNode::VariableAccessAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr)
	{
		type = ANT_VARIABLE_ACCESS;
	}

	AbstractNode *VariableAccessAbstractNode::clone() const
	{
		VariableAccessAbstractNode *node = OGRE_NEW VariableAccessAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		return node;
	}

	String VariableAccessAbstractNode::getValue() const
	{
		return name;
	}

	// ScriptCompilerListener
	ScriptCompilerListener::ScriptCompilerListener()
	{
	}

	ConcreteNodeListPtr ScriptCompilerListener::importFile(ScriptCompiler *compiler, const String &name)
	{
		return ConcreteNodeListPtr();
	}

	void ScriptCompilerListener::preConversion(ScriptCompiler *compiler, ConcreteNodeListPtr nodes)
	{
		
	}

	bool ScriptCompilerListener::postConversion(ScriptCompiler *compiler, const AbstractNodeListPtr &nodes)
	{
		return true;
	}

	void ScriptCompilerListener::handleError(ScriptCompiler *compiler, uint32 code, const String &file, int line, const String &msg)
	{
		Ogre::String str = "Compiler error: ";
		str = str + ScriptCompiler::formatErrorCode(code) + " in " + file + "(" +
			Ogre::StringConverter::toString(line) + ")";
		if(!msg.empty())
			str = str + ": " + msg;
		Ogre::LogManager::getSingleton().logMessage(str);
	}

	bool ScriptCompilerListener::handleEvent(ScriptCompiler *compiler, const String &name, const vector<Ogre::Any>::type &args, Ogre::Any *retval)
	{
		return false;
	}

	Ogre::Any ScriptCompilerListener::createObject(ScriptCompiler *compiler, const String &type, const vector<Ogre::Any>::type &args)
	{
		return Ogre::Any();
	}

	// ScriptCompiler
	String ScriptCompiler::formatErrorCode(uint32 code)
	{
		switch(code)
		{
		case ScriptCompiler::CE_STRINGEXPECTED:
			return "string expected";
		case ScriptCompiler::CE_NUMBEREXPECTED:
			return "number expected";
		case ScriptCompiler::CE_FEWERPARAMETERSEXPECTED:
			return "fewer parameters expected";
		case ScriptCompiler::CE_VARIABLEEXPECTED:
			return "variable expected";
		case ScriptCompiler::CE_UNDEFINEDVARIABLE:
			return "undefined variable";
		case ScriptCompiler::CE_OBJECTNAMEEXPECTED:
			return "object name expected";
		case ScriptCompiler::CE_OBJECTALLOCATIONERROR:
			return "object allocation error";
		case ScriptCompiler::CE_INVALIDPARAMETERS:
			return "invalid parameters";
		case ScriptCompiler::CE_DUPLICATEOVERRIDE:
			return "duplicate object override";
		case ScriptCompiler::CE_UNSUPPORTEDBYRENDERSYSTEM:
			return "object unsupported by render system";
		case ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT:
			return "reference to a non existing object";
		default:
			return "unknown error";
		}
	}

	ScriptCompiler::ScriptCompiler()
		:mListener(0)
	{
		initWordMap();
	}

	bool ScriptCompiler::compile(const String &str, const String &source, const String &group)
	{
		ScriptLexer lexer;
		ScriptParser parser;
		ConcreteNodeListPtr nodes = parser.parse(lexer.tokenize(str, source));
		return compile(nodes, group);
	}

	static void logAST(int tabs, const AbstractNodePtr &node)
	{
		String msg = "";
		for(int i = 0; i < tabs; ++i)
			msg += "\t";

		switch(node->type)
		{
		case ANT_ATOM:
			{
				AtomAbstractNode *atom = reinterpret_cast<AtomAbstractNode*>(node.get());
				msg = msg + atom->value;
			}
			break;
		case ANT_PROPERTY:
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>(node.get());
				msg = msg + prop->name + " =";
				for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
				{
					if((*i)->type == ANT_ATOM)
						msg = msg + " " + reinterpret_cast<AtomAbstractNode*>((*i).get())->value;
				}
			}
			break;
		case ANT_OBJECT:
			{
				ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
				msg = msg + node->file + " - " + StringConverter::toString(node->line) + " - " + obj->cls + " \"" + obj->name + "\" =";
				for(AbstractNodeList::iterator i = obj->values.begin(); i != obj->values.end(); ++i)
				{
					if((*i)->type == ANT_ATOM)
						msg = msg + " " + reinterpret_cast<AtomAbstractNode*>((*i).get())->value;
				}
			}
			break;
		default:
			msg = msg + "Unacceptable node type: " + StringConverter::toString(node->type);
		}

		LogManager::getSingleton().logMessage(msg);

		if(node->type == ANT_OBJECT)
		{
			ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
			for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
			{
				logAST(tabs + 1, *i);
			}
		}
	}

	bool ScriptCompiler::compile(const ConcreteNodeListPtr &nodes, const String &group)
	{
		// Set up the compilation context
		mGroup = group;

		// Clear the past errors
		mErrors.clear();

		// Clear the environment
		mEnv.clear();

		if(mListener)
			mListener->preConversion(this, nodes);

		// Convert our nodes to an AST
		AbstractNodeListPtr ast = convertToAST(nodes);
		// Processes the imports for this script
		processImports(ast);
		// Process object inheritance
		processObjects(ast.get(), ast);
		// Process variable expansion
		processVariables(ast.get());

		// Allows early bail-out through the listener
		if(mListener && !mListener->postConversion(this, ast))
			return mErrors.empty();

		// Translate the nodes
		for(AbstractNodeList::iterator i = ast->begin(); i != ast->end(); ++i)
		{
			//logAST(0, *i);
			if((*i)->type == ANT_OBJECT && reinterpret_cast<ObjectAbstractNode*>((*i).get())->abstract)
				continue;
			ScriptTranslator *translator = ScriptCompilerManager::getSingleton().getTranslator(*i);
			if(translator)
				translator->translate(this, *i);
		}

		mImports.clear();
		mImportRequests.clear();
		mImportTable.clear();

		return mErrors.empty();
	}

	AbstractNodeListPtr ScriptCompiler::_generateAST(const String &str, const String &source, bool doImports, bool doObjects, bool doVariables)
	{
		// Clear the past errors
		mErrors.clear();

		ScriptLexer lexer;
		ScriptParser parser;
		ConcreteNodeListPtr cst = parser.parse(lexer.tokenize(str, source));

		// Call the listener to intercept CST
		if(mListener)
			mListener->preConversion(this, cst);

		// Convert our nodes to an AST
		AbstractNodeListPtr ast = convertToAST(cst);

		if(!ast.isNull() && doImports)
			processImports(ast);
		if(!ast.isNull() && doObjects)
			processObjects(ast.get(), ast);
		if(!ast.isNull() && doVariables)
			processVariables(ast.get());

		return ast;
	}

	bool ScriptCompiler::_compile(AbstractNodeListPtr nodes, const String &group, bool doImports, bool doObjects, bool doVariables)
	{
		// Set up the compilation context
		mGroup = group;

		// Clear the past errors
		mErrors.clear();

		// Clear the environment
		mEnv.clear();

		// Processes the imports for this script
		if(doImports)
			processImports(nodes);
		// Process object inheritance
		if(doObjects)
			processObjects(nodes.get(), nodes);
		// Process variable expansion
		if(doVariables)
			processVariables(nodes.get());

		// Translate the nodes
		for(AbstractNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			//logAST(0, *i);
			if((*i)->type == ANT_OBJECT && reinterpret_cast<ObjectAbstractNode*>((*i).get())->abstract)
				continue;
			ScriptTranslator *translator = ScriptCompilerManager::getSingleton().getTranslator(*i);
			if(translator)
				translator->translate(this, *i);
		}

		return mErrors.empty();
	}

	void ScriptCompiler::addError(uint32 code, const Ogre::String &file, int line, const String &msg)
	{
		ErrorPtr err(OGRE_NEW Error());
		err->code = code;
		err->file = file;
		err->line = line;
		err->message = msg;

		if(mListener)
		{
			mListener->handleError(this, code, file, line, msg);
		}
		else
		{
			Ogre::String str = "Compiler error: ";
			str = str + formatErrorCode(code) + " in " + file + "(" +
				Ogre::StringConverter::toString(line) + ")";
			if(!msg.empty())
				str = str + ": " + msg;
			Ogre::LogManager::getSingleton().logMessage(str);
		}

		mErrors.push_back(err);
	}

	void ScriptCompiler::setListener(ScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	ScriptCompilerListener *ScriptCompiler::getListener()
	{
		return mListener;
	}

	const String &ScriptCompiler::getResourceGroup() const
	{
		return mGroup;
	}

	bool ScriptCompiler::_fireEvent(const Ogre::String &name, const vector<Any>::type &args, Ogre::Any *retval)
	{
		if(mListener)
			return mListener->handleEvent(this, name, args, retval);
		return false;
	}

	Any ScriptCompiler::_fireCreateObject(const Ogre::String &type, const vector<Any>::type &args)
	{
		if(mListener)
			return mListener->createObject(this, type, args);
		return Any();
	}

	AbstractNodeListPtr ScriptCompiler::convertToAST(const Ogre::ConcreteNodeListPtr &nodes)
	{
		AbstractTreeBuilder builder(this);
		AbstractTreeBuilder::visit(&builder, *nodes.get());
		return builder.getResult();
	}

	void ScriptCompiler::processImports(Ogre::AbstractNodeListPtr &nodes)
	{
		// We only need to iterate over the top-level of nodes
		AbstractNodeList::iterator i = nodes->begin(), last = nodes->end();
		while(i != nodes->end())
		{
			// We move to the next node here and save the current one.
			// If any replacement happens, then we are still assured that
			// i points to the node *after* the replaced nodes, no matter
			// how many insertions and deletions may happen
			AbstractNodeList::iterator cur = i++;
			if((*cur)->type == ANT_IMPORT)
			{
				ImportAbstractNode *import = (ImportAbstractNode*)(*cur).get();
				// Only process if the file's contents haven't been loaded
				if(mImports.find(import->source) == mImports.end())
				{
					// Load the script
					AbstractNodeListPtr importedNodes = loadImportPath(import->source);
					if(!importedNodes.isNull() && !importedNodes->empty())
					{
						processImports(importedNodes);
						processObjects(importedNodes.get(), importedNodes);
					}
					if(!importedNodes.isNull() && !importedNodes->empty())
						mImports.insert(std::make_pair(import->source, importedNodes));
				}

				// Handle the target request now
				// If it is a '*' import we remove all previous requests and just use the '*'
				// Otherwise, ensure '*' isn't already registered and register our request
				if(import->target == "*")
				{
					mImportRequests.erase(mImportRequests.lower_bound(import->source),
						mImportRequests.upper_bound(import->source));
					mImportRequests.insert(std::make_pair(import->source, "*"));
				}
				else
				{
					ImportRequestMap::iterator iter = mImportRequests.lower_bound(import->source),
						end = mImportRequests.upper_bound(import->source);
					if(iter == end || iter->second != "*")
					{
						mImportRequests.insert(std::make_pair(import->source, import->target));
					}
				}

				nodes->erase(cur);
			}
		}

		// All import nodes are removed
		// We have cached the code blocks from all the imported scripts
		// We can process all import requests now
		for(ImportCacheMap::iterator i = mImports.begin(); i != mImports.end(); ++i)
		{
			ImportRequestMap::iterator j = mImportRequests.lower_bound(i->first),
				end = mImportRequests.upper_bound(i->first);
			if(j != end)
			{
				if(j->second == "*")
				{
					// Insert the entire AST into the import table
					mImportTable.insert(mImportTable.begin(), i->second->begin(), i->second->end());
					continue; // Skip ahead to the next file
				}
				else
				{
					for(; j != end; ++j)
					{
						// Locate this target and insert it into the import table
						AbstractNodeListPtr newNodes = locateTarget(i->second.get(), j->second);
						if(!newNodes.isNull() && !newNodes->empty())
							mImportTable.insert(mImportTable.begin(), newNodes->begin(), newNodes->end());
					}
				}
			}
		}
	}

	AbstractNodeListPtr ScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		AbstractNodeListPtr retval;
		ConcreteNodeListPtr nodes;

		if(mListener)
			nodes = mListener->importFile(this, name);

		if(nodes.isNull() && ResourceGroupManager::getSingletonPtr())
		{
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(name, mGroup);
			if(!stream.isNull())
			{
				ScriptLexer lexer;
				ScriptTokenListPtr tokens = lexer.tokenize(stream->getAsString(), name);
				ScriptParser parser;
				nodes = parser.parse(tokens);
			}
		}

		if(!nodes.isNull())
			retval = convertToAST(nodes);

		return retval;
	}

	AbstractNodeListPtr ScriptCompiler::locateTarget(AbstractNodeList *nodes, const Ogre::String &target)
	{
		AbstractNodeList::iterator iter = nodes->end();
	
		// Search for a top-level object node
		for(AbstractNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *impl = (ObjectAbstractNode*)(*i).get();
				if(impl->name == target)
					iter = i;
			}
		}

		// MEMCATEGORY_GENERAL is the only category supported for SharedPtr
		AbstractNodeListPtr newNodes(OGRE_NEW_T(AbstractNodeList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
		if(iter != nodes->end())
		{
			newNodes->push_back(*iter);
		}
		return newNodes;
	}

	void ScriptCompiler::processObjects(Ogre::AbstractNodeList *nodes, const Ogre::AbstractNodeListPtr &top)
	{
		for(AbstractNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();

				// Overlay base classes in order.
                for (std::vector<String>::const_iterator baseIt = obj->bases.begin(), end_it = obj->bases.end(); baseIt != end_it; ++baseIt)
				{
                    const String& base = *baseIt;
					// Check the top level first, then check the import table
					AbstractNodeListPtr newNodes = locateTarget(top.get(), base);
					if(newNodes->empty())
						newNodes = locateTarget(&mImportTable, base);

					if (!newNodes->empty()) {
						for(AbstractNodeList::iterator j = newNodes->begin(); j != newNodes->end(); ++j) {
							overlayObject(*j, obj);
                        }
					} else {
						addError(CE_OBJECTBASENOTFOUND, obj->file, obj->line,
							"base object named \"" + base + "\" not found in script definition");
					}
				}

				// Recurse into children
				processObjects(&obj->children, top);

				// Overrides now exist in obj's overrides list. These are non-object nodes which must now
				// Be placed in the children section of the object node such that overriding from parents
				// into children works properly.
				obj->children.insert(obj->children.begin(), obj->overrides.begin(), obj->overrides.end());
			}
		}
	}

	void ScriptCompiler::overlayObject(const AbstractNodePtr &source, ObjectAbstractNode *dest)
	{
		if(source->type == ANT_OBJECT)
		{
			ObjectAbstractNode *src = reinterpret_cast<ObjectAbstractNode*>(source.get());

			// Overlay the environment of one on top the other first
			for(map<String,String>::type::const_iterator i = src->getVariables().begin(); i != src->getVariables().end(); ++i)
			{
				std::pair<bool,String> var = dest->getVariable(i->first);
				if(!var.first)
					dest->setVariable(i->first, i->second);
			}
			
			// Create a vector storing each pairing of override between source and destination
			vector<std::pair<AbstractNodePtr,AbstractNodeList::iterator> >::type overrides; 
			// A list of indices for each destination node tracks the minimum
			// source node they can index-match against
			map<ObjectAbstractNode*,size_t>::type indices;
			// A map storing which nodes have overridden from the destination node
			map<ObjectAbstractNode*,bool>::type overridden;

			// Fill the vector with objects from the source node (base)
			// And insert non-objects into the overrides list of the destination
			AbstractNodeList::iterator insertPos = dest->children.begin();
			for(AbstractNodeList::const_iterator i = src->children.begin(); i != src->children.end(); ++i)
			{
				if((*i)->type == ANT_OBJECT)
				{
					overrides.push_back(std::make_pair(*i, dest->children.end()));
				}
				else
				{
					AbstractNodePtr newNode((*i)->clone());
					newNode->parent = dest;
					dest->overrides.push_back(newNode);
				}
			}

			// Track the running maximum override index in the name-matching phase
			size_t maxOverrideIndex = 0;

			// Loop through destination children searching for name-matching overrides
			for(AbstractNodeList::iterator i = dest->children.begin(); i != dest->children.end(); )
			{
				if((*i)->type == ANT_OBJECT)
				{
					// Start tracking the override index position for this object
					size_t overrideIndex = 0;

					ObjectAbstractNode *node = reinterpret_cast<ObjectAbstractNode*>((*i).get());
					indices[node] = maxOverrideIndex;
					overridden[node] = false;

					// special treatment for materials with * in their name
					bool nodeHasWildcard=node->name.find('*') != String::npos;

					// Find the matching name node
					for(size_t j = 0; j < overrides.size(); ++j)
					{
						ObjectAbstractNode *temp = reinterpret_cast<ObjectAbstractNode*>(overrides[j].first.get());
						// Consider a match a node that has a wildcard and matches an input name
						bool wildcardMatch = nodeHasWildcard && 
							(StringUtil::match(temp->name,node->name,true) || 
								(node->name.size() == 1 && temp->name.empty()));
						if(temp->cls == node->cls && !node->name.empty() && (temp->name == node->name || wildcardMatch))
						{
							// Pair these two together unless it's already paired
							if(overrides[j].second == dest->children.end())
							{
								AbstractNodeList::iterator currentIterator = i;
								ObjectAbstractNode *currentNode = node;
								if (wildcardMatch)
								{
									//If wildcard is matched, make a copy of current material and put it before the iterator, matching its name to the parent. Use same reinterpret cast as above when node is set
									AbstractNodePtr newNode((*i)->clone());
									currentIterator = dest->children.insert(currentIterator, newNode);
									currentNode = reinterpret_cast<ObjectAbstractNode*>((*currentIterator).get());
									currentNode->name = temp->name;//make the regex match its matcher
								}
								overrides[j] = std::make_pair(overrides[j].first, currentIterator);
								// Store the max override index for this matched pair
								overrideIndex = j;
								overrideIndex = maxOverrideIndex = std::max(overrideIndex, maxOverrideIndex);
								indices[currentNode] = overrideIndex;
								overridden[currentNode] = true;
							}
							else
							{
								addError(CE_DUPLICATEOVERRIDE, node->file, node->line);
							}

							if(!wildcardMatch)
								break;
						}
					}

					if (nodeHasWildcard)
					{
						//if the node has a wildcard it will be deleted since it was duplicated for every match
						AbstractNodeList::iterator deletable=i++;
						dest->children.erase(deletable);
					}
					else
					{
						++i; //Behavior in absence of regex, just increment iterator
					}
				}
				else 
				{
					++i; //Behavior in absence of replaceable object, just increment iterator to find another
				}
			}

			// Now make matches based on index
			// Loop through destination children searching for name-matching overrides
			for(AbstractNodeList::iterator i = dest->children.begin(); i != dest->children.end(); ++i)
			{
				if((*i)->type == ANT_OBJECT)
				{
					ObjectAbstractNode *node = reinterpret_cast<ObjectAbstractNode*>((*i).get());
					if(!overridden[node])
					{
						// Retrieve the minimum override index from the map
						size_t overrideIndex = indices[node];

						if(overrideIndex < overrides.size())
						{
							// Search for minimum matching override
							for(size_t j = overrideIndex; j < overrides.size(); ++j)
							{
								ObjectAbstractNode *temp = reinterpret_cast<ObjectAbstractNode*>(overrides[j].first.get());
								if(temp->name.empty() && temp->cls == node->cls && overrides[j].second == dest->children.end())
								{
									overrides[j] = std::make_pair(overrides[j].first, i);
									break;
								}
							}
						}
					}
				}
			}

			// Loop through overrides, either inserting source nodes or overriding
			insertPos = dest->children.begin();
			for(size_t i = 0; i < overrides.size(); ++i)
			{
				if(overrides[i].second != dest->children.end())
				{
					// Override the destination with the source (base) object
					overlayObject(overrides[i].first, 
						reinterpret_cast<ObjectAbstractNode*>((*overrides[i].second).get()));
					insertPos = overrides[i].second;
					insertPos++;
				}
				else
				{
					// No override was possible, so insert this node at the insert position
					// into the destination (child) object
					AbstractNodePtr newNode(overrides[i].first->clone());
					newNode->parent = dest;
					if(insertPos != dest->children.end())
					{
						dest->children.insert(insertPos, newNode);
					}
					else
					{
						dest->children.push_back(newNode);
					}
				}
			}
		}
	}

	bool ScriptCompiler::isNameExcluded(const String &cls, AbstractNode *parent)
	{
		// Run past the listener
		Any retval;
		vector<Any>::type args;
		args.push_back(Any(cls));
		args.push_back(Any(parent));
		_fireEvent("processNameExclusion", args, &retval);

		if(retval.isEmpty())
		{
			// Process the built-in name exclusions
			if(cls == "emitter" || cls == "affector")
			{
				// emitters or affectors inside a particle_system are excluded
				while(parent && parent->type == ANT_OBJECT)
				{
					ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(parent);
					if(obj->cls == "particle_system")
						return true;
					parent = obj->parent;
				}
				return false;
			}
			else if(cls == "pass")
			{
				// passes inside compositors are excluded
				while(parent && parent->type == ANT_OBJECT)
				{
					ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(parent);
					if(obj->cls == "compositor")
						return true;
					parent = obj->parent;
				}
				return false;
			}
			else if(cls == "texture_source")
			{
				// Parent must be texture_unit
				while(parent && parent->type == ANT_OBJECT)
				{
					ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(parent);
					if(obj->cls == "texture_unit")
						return true;
					parent = obj->parent;
				}
				return false;
			}
		}
		else
		{
			return any_cast<bool>(retval);
		}
		return false;
	}

	void ScriptCompiler::processVariables(Ogre::AbstractNodeList *nodes)
	{
		AbstractNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			AbstractNodeList::iterator cur = i;
			++i;

			if((*cur)->type == ANT_OBJECT)
			{
				// Only process if this object is not abstract
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*cur).get();
				if(!obj->abstract)
				{
					processVariables(&obj->children);
					processVariables(&obj->values);
				}
			}
			else if((*cur)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*cur).get();
				processVariables(&prop->values);
			}
			else if((*cur)->type == ANT_VARIABLE_ACCESS)
			{
				VariableAccessAbstractNode *var = (VariableAccessAbstractNode*)(*cur).get();

				// Look up the enclosing scope
				ObjectAbstractNode *scope = 0;
				AbstractNode *temp = var->parent;
				while(temp)
				{
					if(temp->type == ANT_OBJECT)
					{
						scope = (ObjectAbstractNode*)temp;
						break;
					}
					temp = temp->parent;
				}

				// Look up the variable in the environment
				std::pair<bool,String> varAccess;
				if(scope)
					varAccess = scope->getVariable(var->name);
				if(!scope || !varAccess.first)
				{
					map<String,String>::type::iterator k = mEnv.find(var->name);
					varAccess.first = k != mEnv.end();
					if(varAccess.first)
						varAccess.second = k->second;
				}

				if(varAccess.first)
				{
					// Found the variable, so process it and insert it into the tree
					ScriptLexer lexer;
					ScriptTokenListPtr tokens = lexer.tokenize(varAccess.second, var->file);
					ScriptParser parser;
					ConcreteNodeListPtr cst = parser.parseChunk(tokens);
					AbstractNodeListPtr ast = convertToAST(cst);

					// Set up ownership for these nodes
					for(AbstractNodeList::iterator j = ast->begin(); j != ast->end(); ++j)
						(*j)->parent = var->parent;

					// Recursively handle variable accesses within the variable expansion
					processVariables(ast.get());

					// Insert the nodes in place of the variable
					nodes->insert(cur, ast->begin(), ast->end());
				}
				else
				{
					// Error
					addError(CE_UNDEFINEDVARIABLE, var->file, var->line);
				}

				// Remove the variable node
				nodes->erase(cur);
			}
		}
	}

	void ScriptCompiler::initWordMap()
	{
		mIds["on"] = ID_ON;
		mIds["off"] = ID_OFF;
		mIds["true"] = ID_TRUE;
		mIds["false"] = ID_FALSE;
		mIds["yes"] = ID_YES;
		mIds["no"] = ID_NO;

		// Material ids
		mIds["material"] = ID_MATERIAL;
		mIds["vertex_program"] = ID_VERTEX_PROGRAM;
		mIds["geometry_program"] = ID_GEOMETRY_PROGRAM;
		mIds["fragment_program"] = ID_FRAGMENT_PROGRAM;
		mIds["technique"] = ID_TECHNIQUE;
		mIds["pass"] = ID_PASS;
		mIds["texture_unit"] = ID_TEXTURE_UNIT;
		mIds["vertex_program_ref"] = ID_VERTEX_PROGRAM_REF;
		mIds["geometry_program_ref"] = ID_GEOMETRY_PROGRAM_REF;
		mIds["fragment_program_ref"] = ID_FRAGMENT_PROGRAM_REF;
		mIds["shadow_caster_vertex_program_ref"] = ID_SHADOW_CASTER_VERTEX_PROGRAM_REF;
		mIds["shadow_receiver_vertex_program_ref"] = ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF;
		mIds["shadow_receiver_fragment_program_ref"] = ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF;

        mIds["lod_values"] = ID_LOD_VALUES;
        mIds["lod_strategy"] = ID_LOD_STRATEGY;
		mIds["lod_distances"] = ID_LOD_DISTANCES;
		mIds["receive_shadows"] = ID_RECEIVE_SHADOWS;
		mIds["transparency_casts_shadows"] = ID_TRANSPARENCY_CASTS_SHADOWS;
		mIds["set_texture_alias"] = ID_SET_TEXTURE_ALIAS;

		mIds["source"] = ID_SOURCE;
		mIds["syntax"] = ID_SYNTAX;
		mIds["default_params"] = ID_DEFAULT_PARAMS;
		mIds["param_indexed"] = ID_PARAM_INDEXED;
		mIds["param_named"] = ID_PARAM_NAMED;
		mIds["param_indexed_auto"] = ID_PARAM_INDEXED_AUTO;
		mIds["param_named_auto"] = ID_PARAM_NAMED_AUTO;

		mIds["scheme"] = ID_SCHEME;
		mIds["lod_index"] = ID_LOD_INDEX;
		mIds["shadow_caster_material"] = ID_SHADOW_CASTER_MATERIAL;
		mIds["shadow_receiver_material"] = ID_SHADOW_RECEIVER_MATERIAL;
		mIds["gpu_vendor_rule"] = ID_GPU_VENDOR_RULE;
		mIds["gpu_device_rule"] = ID_GPU_DEVICE_RULE;
		mIds["include"] = ID_INCLUDE;
		mIds["exclude"] = ID_EXCLUDE;



		mIds["ambient"] = ID_AMBIENT;
		mIds["diffuse"] = ID_DIFFUSE;
		mIds["specular"] = ID_SPECULAR;
		mIds["emissive"] = ID_EMISSIVE;
			mIds["vertexcolour"] = ID_VERTEXCOLOUR;
		mIds["scene_blend"] = ID_SCENE_BLEND;
		mIds["colour_blend"] = ID_COLOUR_BLEND;
			mIds["one"] = ID_ONE;
			mIds["zero"] = ID_ZERO;
			mIds["dest_colour"] = ID_DEST_COLOUR;
			mIds["src_colour"] = ID_SRC_COLOUR;
			mIds["one_minus_src_colour"] = ID_ONE_MINUS_SRC_COLOUR;
			mIds["one_minus_dest_colour"] = ID_ONE_MINUS_DEST_COLOUR;
			mIds["dest_alpha"] = ID_DEST_ALPHA;
			mIds["src_alpha"] = ID_SRC_ALPHA;
			mIds["one_minus_dest_alpha"] = ID_ONE_MINUS_DEST_ALPHA;
			mIds["one_minus_src_alpha"] = ID_ONE_MINUS_SRC_ALPHA;
		mIds["separate_scene_blend"] = ID_SEPARATE_SCENE_BLEND;
		mIds["scene_blend_op"] = ID_SCENE_BLEND_OP;
			mIds["reverse_subtract"] = ID_REVERSE_SUBTRACT;
			mIds["min"] = ID_MIN;
			mIds["max"] = ID_MAX;
		mIds["separate_scene_blend_op"] = ID_SEPARATE_SCENE_BLEND_OP;
		mIds["depth_check"] = ID_DEPTH_CHECK;
		mIds["depth_write"] = ID_DEPTH_WRITE;
		mIds["depth_func"] = ID_DEPTH_FUNC;
		mIds["depth_bias"] = ID_DEPTH_BIAS;
		mIds["iteration_depth_bias"] = ID_ITERATION_DEPTH_BIAS;
			mIds["always_fail"] = ID_ALWAYS_FAIL;
			mIds["always_pass"] = ID_ALWAYS_PASS;
			mIds["less_equal"] = ID_LESS_EQUAL;
			mIds["less"] = ID_LESS;
			mIds["equal"] = ID_EQUAL;
			mIds["not_equal"] = ID_NOT_EQUAL;
			mIds["greater_equal"] = ID_GREATER_EQUAL;
			mIds["greater"] = ID_GREATER;
		mIds["alpha_rejection"] = ID_ALPHA_REJECTION;
		mIds["alpha_to_coverage"] = ID_ALPHA_TO_COVERAGE;
		mIds["light_scissor"] = ID_LIGHT_SCISSOR;
		mIds["light_clip_planes"] = ID_LIGHT_CLIP_PLANES;
		mIds["transparent_sorting"] = ID_TRANSPARENT_SORTING;
		mIds["illumination_stage"] = ID_ILLUMINATION_STAGE;
			mIds["decal"] = ID_DECAL;
		mIds["cull_hardware"] = ID_CULL_HARDWARE;
			mIds["clockwise"] = ID_CLOCKWISE;
			mIds["anticlockwise"] = ID_ANTICLOCKWISE;
		mIds["cull_software"] = ID_CULL_SOFTWARE;
			mIds["back"] = ID_BACK;
			mIds["front"] = ID_FRONT;
		mIds["normalise_normals"] = ID_NORMALISE_NORMALS;
		mIds["lighting"] = ID_LIGHTING;
		mIds["shading"] = ID_SHADING;
			mIds["flat"] = ID_FLAT;
			mIds["gouraud"] = ID_GOURAUD;
			mIds["phong"] = ID_PHONG;
		mIds["polygon_mode"] = ID_POLYGON_MODE;
			mIds["solid"] = ID_SOLID;
			mIds["wireframe"] = ID_WIREFRAME;
			mIds["points"] = ID_POINTS;
		mIds["polygon_mode_overrideable"] = ID_POLYGON_MODE_OVERRIDEABLE;
		mIds["fog_override"] = ID_FOG_OVERRIDE;
			mIds["none"] = ID_NONE;
			mIds["linear"] = ID_LINEAR;
			mIds["exp"] = ID_EXP;
			mIds["exp2"] = ID_EXP2;
		mIds["colour_write"] = ID_COLOUR_WRITE;
		mIds["max_lights"] = ID_MAX_LIGHTS;
		mIds["start_light"] = ID_START_LIGHT;
		mIds["iteration"] = ID_ITERATION;
			mIds["once"] = ID_ONCE;
			mIds["once_per_light"] = ID_ONCE_PER_LIGHT;
			mIds["per_n_lights"] = ID_PER_N_LIGHTS;
			mIds["per_light"] = ID_PER_LIGHT;
			mIds["point"] = ID_POINT;
			mIds["spot"] = ID_SPOT;
			mIds["directional"] = ID_DIRECTIONAL;
		mIds["point_size"] = ID_POINT_SIZE;
		mIds["point_sprites"] = ID_POINT_SPRITES;
		mIds["point_size_min"] = ID_POINT_SIZE_MIN;
		mIds["point_size_max"] = ID_POINT_SIZE_MAX;
		mIds["point_size_attenuation"] = ID_POINT_SIZE_ATTENUATION;

		mIds["texture_alias"] = ID_TEXTURE_ALIAS;
		mIds["texture"] = ID_TEXTURE;
			mIds["1d"] = ID_1D;
			mIds["2d"] = ID_2D;
			mIds["3d"] = ID_3D;
			mIds["cubic"] = ID_CUBIC;
			mIds["unlimited"] = ID_UNLIMITED;
			mIds["alpha"] = ID_ALPHA;
			mIds["gamma"] = ID_GAMMA;
		mIds["anim_texture"] = ID_ANIM_TEXTURE;
		mIds["cubic_texture"] = ID_CUBIC_TEXTURE;
			mIds["separateUV"] = ID_SEPARATE_UV;
			mIds["combinedUVW"] = ID_COMBINED_UVW;
		mIds["tex_coord_set"] = ID_TEX_COORD_SET;
		mIds["tex_address_mode"] = ID_TEX_ADDRESS_MODE;
			mIds["wrap"] = ID_WRAP;
			mIds["clamp"] = ID_CLAMP;
			mIds["mirror"] = ID_MIRROR;
			mIds["border"] = ID_BORDER;
		mIds["tex_border_colour"] = ID_TEX_BORDER_COLOUR;
		mIds["filtering"] = ID_FILTERING;
			mIds["bilinear"] = ID_BILINEAR;
			mIds["trilinear"] = ID_TRILINEAR;
			mIds["anisotropic"] = ID_ANISOTROPIC;
		mIds["max_anisotropy"] = ID_MAX_ANISOTROPY;
		mIds["mipmap_bias"] = ID_MIPMAP_BIAS;
		mIds["colour_op"] = ID_COLOUR_OP;
			mIds["replace"] = ID_REPLACE;
			mIds["add"] = ID_ADD;
			mIds["modulate"] = ID_MODULATE;
			mIds["alpha_blend"] = ID_ALPHA_BLEND;
		mIds["colour_op_ex"] = ID_COLOUR_OP_EX;
			mIds["source1"] = ID_SOURCE1;
			mIds["source2"] = ID_SOURCE2;
			mIds["modulate"] = ID_MODULATE;
			mIds["modulate_x2"] = ID_MODULATE_X2;
			mIds["modulate_x4"] = ID_MODULATE_X4;
			mIds["add"] = ID_ADD;
			mIds["add_signed"] = ID_ADD_SIGNED;
			mIds["add_smooth"] = ID_ADD_SMOOTH;
			mIds["subtract"] = ID_SUBTRACT;
			mIds["blend_diffuse_alpha"] = ID_BLEND_DIFFUSE_ALPHA;
			mIds["blend_texture_alpha"] = ID_BLEND_TEXTURE_ALPHA;
			mIds["blend_current_alpha"] = ID_BLEND_CURRENT_ALPHA;
			mIds["blend_manual"] = ID_BLEND_MANUAL;
			mIds["dotproduct"] = ID_DOT_PRODUCT;
			mIds["blend_diffuse_colour"] = ID_BLEND_DIFFUSE_COLOUR;
			mIds["src_current"] = ID_SRC_CURRENT;
			mIds["src_texture"] = ID_SRC_TEXTURE;
			mIds["src_diffuse"] = ID_SRC_DIFFUSE;
			mIds["src_specular"] = ID_SRC_SPECULAR;
			mIds["src_manual"] = ID_SRC_MANUAL;
		mIds["colour_op_multipass_fallback"] = ID_COLOUR_OP_MULTIPASS_FALLBACK;
		mIds["alpha_op_ex"] = ID_ALPHA_OP_EX;
		mIds["env_map"] = ID_ENV_MAP;
			mIds["spherical"] = ID_SPHERICAL;
			mIds["planar"] = ID_PLANAR;
			mIds["cubic_reflection"] = ID_CUBIC_REFLECTION;
			mIds["cubic_normal"] = ID_CUBIC_NORMAL;
		mIds["scroll"] = ID_SCROLL;
		mIds["scroll_anim"] = ID_SCROLL_ANIM;
		mIds["rotate"] = ID_ROTATE;
		mIds["rotate_anim"] = ID_ROTATE_ANIM;
		mIds["scale"] = ID_SCALE;
		mIds["wave_xform"] = ID_WAVE_XFORM;
			mIds["scroll_x"] = ID_SCROLL_X;
			mIds["scroll_y"] = ID_SCROLL_Y;
			mIds["scale_x"] = ID_SCALE_X;
			mIds["scale_y"] = ID_SCALE_Y;
			mIds["sine"] = ID_SINE;
			mIds["triangle"] = ID_TRIANGLE;
			mIds["sawtooth"] = ID_SAWTOOTH;
			mIds["square"] = ID_SQUARE;
			mIds["inverse_sawtooth"] = ID_INVERSE_SAWTOOTH;
		mIds["transform"] = ID_TRANSFORM;
		mIds["binding_type"] = ID_BINDING_TYPE;
			mIds["vertex"] = ID_VERTEX;
			mIds["fragment"] = ID_FRAGMENT;
		mIds["content_type"] = ID_CONTENT_TYPE;
			mIds["named"] = ID_NAMED;
			mIds["shadow"] = ID_SHADOW;
		mIds["texture_source"] = ID_TEXTURE_SOURCE;
		mIds["shared_params"] = ID_SHARED_PARAMS;
		mIds["shared_param_named"] = ID_SHARED_PARAM_NAMED;
		mIds["shared_params_ref"] = ID_SHARED_PARAMS_REF;


		// Particle system
		mIds["particle_system"] = ID_PARTICLE_SYSTEM;
		mIds["emitter"] = ID_EMITTER;
		mIds["affector"] = ID_AFFECTOR;

		// Compositor
		mIds["compositor"] = ID_COMPOSITOR;
		mIds["target"] = ID_TARGET;
		mIds["target_output"] = ID_TARGET_OUTPUT;

		mIds["input"] = ID_INPUT;
			mIds["none"] = ID_NONE;
			mIds["previous"] = ID_PREVIOUS;
			mIds["target_width"] = ID_TARGET_WIDTH;
			mIds["target_height"] = ID_TARGET_HEIGHT;
			mIds["target_width_scaled"] = ID_TARGET_WIDTH_SCALED;
			mIds["target_height_scaled"] = ID_TARGET_HEIGHT_SCALED;
			mIds["shared"] = ID_SHARED;
			//mIds["gamma"] = ID_GAMMA; - already registered
			mIds["no_fsaa"] = ID_NO_FSAA;
		mIds["only_initial"] = ID_ONLY_INITIAL;
		mIds["visibility_mask"] = ID_VISIBILITY_MASK;
		mIds["lod_bias"] = ID_LOD_BIAS;
		mIds["material_scheme"] = ID_MATERIAL_SCHEME;
		mIds["shadows"] = ID_SHADOWS_ENABLED;

		mIds["clear"] = ID_CLEAR;
		mIds["stencil"] = ID_STENCIL;
		mIds["render_scene"] = ID_RENDER_SCENE;
		mIds["render_quad"] = ID_RENDER_QUAD;
		mIds["identifier"] = ID_IDENTIFIER;
		mIds["first_render_queue"] = ID_FIRST_RENDER_QUEUE;
		mIds["last_render_queue"] = ID_LAST_RENDER_QUEUE;
		mIds["quad_normals"] = ID_QUAD_NORMALS;
			mIds["camera_far_corners_view_space"] = ID_CAMERA_FAR_CORNERS_VIEW_SPACE;
			mIds["camera_far_corners_world_space"] = ID_CAMERA_FAR_CORNERS_WORLD_SPACE;

		mIds["buffers"] = ID_BUFFERS;
			mIds["colour"] = ID_COLOUR;
			mIds["depth"] = ID_DEPTH;
		mIds["colour_value"] = ID_COLOUR_VALUE;
		mIds["depth_value"] = ID_DEPTH_VALUE;
		mIds["stencil_value"] = ID_STENCIL_VALUE;

		mIds["check"] = ID_CHECK;
		mIds["comp_func"] = ID_COMP_FUNC;
		mIds["ref_value"] = ID_REF_VALUE;
		mIds["mask"] = ID_MASK;
		mIds["fail_op"] = ID_FAIL_OP;
			mIds["keep"] = ID_KEEP;
			mIds["increment"] = ID_INCREMENT;
			mIds["decrement"] = ID_DECREMENT;
			mIds["increment_wrap"] = ID_INCREMENT_WRAP;
			mIds["decrement_wrap"] = ID_DECREMENT_WRAP;
			mIds["invert"] = ID_INVERT;
		mIds["depth_fail_op"] = ID_DEPTH_FAIL_OP;
		mIds["pass_op"] = ID_PASS_OP;
		mIds["two_sided"] = ID_TWO_SIDED;
	}

	// AbstractTreeeBuilder
	ScriptCompiler::AbstractTreeBuilder::AbstractTreeBuilder(ScriptCompiler *compiler)
		:mCurrent(0), mNodes(OGRE_NEW_T(AbstractNodeList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T), mCompiler(compiler)
	{
	}

	const AbstractNodeListPtr &ScriptCompiler::AbstractTreeBuilder::getResult() const
	{
		return mNodes;
	}

	void ScriptCompiler::AbstractTreeBuilder::visit(ConcreteNode *node)
	{
		AbstractNodePtr asn;

		// Import = "import" >> 2 children, mCurrent == null
		if(node->type == CNT_IMPORT && mCurrent == 0)
		{
			if(node->children.size() > 2)
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				return;
			}
			if(node->children.size() < 2)
			{
				mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
				return;
			}

			ImportAbstractNode *impl = OGRE_NEW ImportAbstractNode();
			impl->line = node->line;
			impl->file = node->file;
			
			ConcreteNodeList::iterator iter = node->children.begin();
			impl->target = (*iter)->token;

			iter++;
			impl->source = (*iter)->token;

			asn = AbstractNodePtr(impl);
		}
		// variable set = "set" >> 2 children, children[0] == variable
		else if(node->type == CNT_VARIABLE_ASSIGN)
		{
			if(node->children.size() > 2)
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				return;
			}
			if(node->children.size() < 2)
			{
				mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
				return;
			}
			if(node->children.front()->type != CNT_VARIABLE)
			{
				mCompiler->addError(CE_VARIABLEEXPECTED, node->children.front()->file, node->children.front()->line);
				return;
			}

			ConcreteNodeList::iterator i = node->children.begin();
			String name = (*i)->token;

			++i;
			String value = (*i)->token;

			if(mCurrent && mCurrent->type == ANT_OBJECT)
			{
				ObjectAbstractNode *ptr = (ObjectAbstractNode*)mCurrent;
				ptr->setVariable(name, value);
			}
			else
			{
				mCompiler->mEnv.insert(std::make_pair(name, value));
			}
		}
		// variable = $*, no children
		else if(node->type == CNT_VARIABLE)
		{
			if(!node->children.empty())
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				return;
			}

			VariableAccessAbstractNode *impl = OGRE_NEW VariableAccessAbstractNode(mCurrent);
			impl->line = node->line;
			impl->file = node->file;
			impl->name = node->token;

			asn = AbstractNodePtr(impl);
		}
		// Handle properties and objects here
		else if(!node->children.empty())
		{
			// Grab the last two nodes
			ConcreteNodePtr temp1, temp2;
			ConcreteNodeList::reverse_iterator riter = node->children.rbegin();
			if(riter != node->children.rend())
			{
				temp1 = *riter;
				riter++;
			}
			if(riter != node->children.rend())
				temp2 = *riter;

			// object = last 2 children == { and }
			if(!temp1.isNull() && !temp2.isNull() &&
				temp1->type == CNT_RBRACE && temp2->type == CNT_LBRACE)
			{
				if(node->children.size() < 2)
				{
					mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
					return;
				}

				ObjectAbstractNode *impl = OGRE_NEW ObjectAbstractNode(mCurrent);
				impl->line = node->line;
				impl->file = node->file;
				impl->abstract = false;

				// Create a temporary detail list
				list<ConcreteNode*>::type temp;
				if(node->token == "abstract")
				{
					impl->abstract = true;
					for(ConcreteNodeList::const_iterator i = node->children.begin(); i != node->children.end(); ++i)
						temp.push_back((*i).get());
				}
				else
				{
					temp.push_back(node);
					for(ConcreteNodeList::const_iterator i = node->children.begin(); i != node->children.end(); ++i)
						temp.push_back((*i).get());
				}

				// Get the type of object
				list<ConcreteNode*>::type::const_iterator iter = temp.begin();
				impl->cls = (*iter)->token;
				++iter;

				// Get the name
				// Unless the type is in the exclusion list
				if(iter != temp.end() && ((*iter)->type == CNT_WORD || (*iter)->type == CNT_QUOTE) &&
					!mCompiler->isNameExcluded(impl->cls, mCurrent))
				{
					impl->name = (*iter)->token;
					++iter;
				}

				// Everything up until the colon is a "value" of this object
				while(iter != temp.end() && (*iter)->type != CNT_COLON && (*iter)->type != CNT_LBRACE)
				{
					if((*iter)->type == CNT_VARIABLE)
					{
						VariableAccessAbstractNode *var = OGRE_NEW VariableAccessAbstractNode(impl);
						var->file = (*iter)->file;
						var->line = (*iter)->line;
						var->type = ANT_VARIABLE_ACCESS;
						var->name = (*iter)->token;
						impl->values.push_back(AbstractNodePtr(var));
					}
					else
					{
						AtomAbstractNode *atom = OGRE_NEW AtomAbstractNode(impl);
						atom->file = (*iter)->file;
						atom->line = (*iter)->line;
						atom->type = ANT_ATOM;
						atom->value = (*iter)->token;
						impl->values.push_back(AbstractNodePtr(atom));
					}
					++iter;
				}

				// Find the bases
				if(iter != temp.end() && (*iter)->type == CNT_COLON)
				{
					// Children of the ':' are bases
					for(ConcreteNodeList::iterator j = (*iter)->children.begin(); j != (*iter)->children.end(); ++j)
						impl->bases.push_back((*j)->token);
                    ++iter;
				}

				// Finally try to map the cls to an id
				ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->cls);
				if(iter2 != mCompiler->mIds.end())
					impl->id = iter2->second;

				asn = AbstractNodePtr(impl);
				mCurrent = impl;

				// Visit the children of the {
				AbstractTreeBuilder::visit(this, temp2->children);

				// Go back up the stack
				mCurrent = impl->parent;
			}
			// Otherwise, it is a property
			else
			{
				PropertyAbstractNode *impl = OGRE_NEW PropertyAbstractNode(mCurrent);
				impl->line = node->line;
				impl->file = node->file;
				impl->name = node->token;

				ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->name);
				if(iter2 != mCompiler->mIds.end())
					impl->id = iter2->second;

				asn = AbstractNodePtr(impl);
				mCurrent = impl;

				// Visit the children of the {
				AbstractTreeBuilder::visit(this, node->children);

				// Go back up the stack
				mCurrent = impl->parent;
			}
		}
		// Otherwise, it is a standard atom
		else
		{
			AtomAbstractNode *impl = OGRE_NEW AtomAbstractNode(mCurrent);
			impl->line = node->line;
			impl->file = node->file;
			impl->value = node->token;

			ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->value);
			if(iter2 != mCompiler->mIds.end())
				impl->id = iter2->second;

			asn = AbstractNodePtr(impl);
		}

		// Here, we must insert the node into the tree
		if(!asn.isNull())
		{
			if(mCurrent)
			{
				if(mCurrent->type == ANT_PROPERTY)
				{
					PropertyAbstractNode *impl = reinterpret_cast<PropertyAbstractNode*>(mCurrent);
					impl->values.push_back(asn);
				}
				else
				{
					ObjectAbstractNode *impl = reinterpret_cast<ObjectAbstractNode*>(mCurrent);
					impl->children.push_back(asn);
				}
			}
			else
			{
				mNodes->push_back(asn);
			}
		}
	}

	void ScriptCompiler::AbstractTreeBuilder::visit(AbstractTreeBuilder *visitor, const ConcreteNodeList &nodes)
	{
		for(ConcreteNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			visitor->visit((*i).get());
	}
	

	// ScriptCompilerManager
	template<> ScriptCompilerManager *Singleton<ScriptCompilerManager>::ms_Singleton = 0;
	
	ScriptCompilerManager* ScriptCompilerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
	//-----------------------------------------------------------------------
    ScriptCompilerManager& ScriptCompilerManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	//-----------------------------------------------------------------------
	ScriptCompilerManager::ScriptCompilerManager()
		:mListener(0)
	{
		OGRE_LOCK_AUTO_MUTEX
#if OGRE_USE_NEW_COMPILERS == 1
		mScriptPatterns.push_back("*.program");
		mScriptPatterns.push_back("*.material");
		mScriptPatterns.push_back("*.particle");
		mScriptPatterns.push_back("*.compositor");
#endif
        mScriptPatterns.push_back("*.os");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

		OGRE_THREAD_POINTER_SET(mScriptCompiler, OGRE_NEW ScriptCompiler());

		mBuiltinTranslatorManager = OGRE_NEW BuiltinScriptTranslatorManager();
		mManagers.push_back(mBuiltinTranslatorManager);
	}
	//-----------------------------------------------------------------------
	ScriptCompilerManager::~ScriptCompilerManager()
	{
		OGRE_THREAD_POINTER_DELETE(mScriptCompiler);
		OGRE_DELETE mBuiltinTranslatorManager;
	}
	//-----------------------------------------------------------------------
	void ScriptCompilerManager::setListener(ScriptCompilerListener *listener)
	{
		OGRE_LOCK_AUTO_MUTEX
		mListener = listener;
	}
	//-----------------------------------------------------------------------
	ScriptCompilerListener *ScriptCompilerManager::getListener()
	{
		return mListener;
	}
	//-----------------------------------------------------------------------
	void ScriptCompilerManager::addTranslatorManager(Ogre::ScriptTranslatorManager *man)
	{
		OGRE_LOCK_AUTO_MUTEX
		mManagers.push_back(man);
	}
	//-----------------------------------------------------------------------
	void ScriptCompilerManager::removeTranslatorManager(Ogre::ScriptTranslatorManager *man)
	{
		OGRE_LOCK_AUTO_MUTEX
		
		for(vector<ScriptTranslatorManager*>::type::iterator i = mManagers.begin(); i != mManagers.end(); ++i)
		{
			if(*i == man)
			{
				mManagers.erase(i);
				break;
			}
		}
	}
	//-------------------------------------------------------------------------
	void ScriptCompilerManager::clearTranslatorManagers()
	{
		mManagers.clear();
	}
	//-----------------------------------------------------------------------
	ScriptTranslator *ScriptCompilerManager::getTranslator(const AbstractNodePtr &node)
	{
		ScriptTranslator *translator = 0;
		{
			OGRE_LOCK_AUTO_MUTEX
			
			// Start looking from the back
			for(vector<ScriptTranslatorManager*>::type::reverse_iterator i = mManagers.rbegin(); i != mManagers.rend(); ++i)
			{
				translator = (*i)->getTranslator(node);
				if(translator != 0)
					break;
			}
		}
		return translator;
	}
	//-----------------------------------------------------------------------
    const StringVector& ScriptCompilerManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //-----------------------------------------------------------------------
    Real ScriptCompilerManager::getLoadingOrder(void) const
    {
        /// Load relatively early, before most script loaders run
        return 90.0f;
    }
    //-----------------------------------------------------------------------
    void ScriptCompilerManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
#if OGRE_THREAD_SUPPORT
		// check we have an instance for this thread (should always have one for main thread)
		if (!mScriptCompiler.get())
		{
			// create a new instance for this thread - will get deleted when
			// the thread dies
			mScriptCompiler.reset(OGRE_NEW ScriptCompiler());
		}
#endif
		// Set the listener on the compiler before we continue
		{
			OGRE_LOCK_AUTO_MUTEX
			mScriptCompiler->setListener(mListener);
		}
        mScriptCompiler->compile(stream->getAsString(), stream->getName(), groupName);
    }
}


