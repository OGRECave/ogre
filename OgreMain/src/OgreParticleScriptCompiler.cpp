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
#include "OgreParticleScriptCompiler.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"

namespace Ogre{

	// ParticleScriptCompilerListener
	ParticleScriptCompilerListener::ParticleScriptCompilerListener()
	{
	}

	bool ParticleScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, Ogre::ParticleScriptCompiler *)
	{
		return false;
	}

	ParticleSystem *ParticleScriptCompilerListener::getParticleSystem(const Ogre::String &name, const Ogre::String &group)
	{
		// By default create a new template
		return ParticleSystemManager::getSingleton().createTemplate(name, group);
	}

	// ParticleScriptCompiler
	ParticleScriptCompiler::ParticleScriptCompiler()
		:mListener(0), mSystem(0)
	{
		mAllowNontypedObjects = true;
	}

	void ParticleScriptCompiler::setListener(ParticleScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	ParticleSystem *ParticleScriptCompiler::getParticleSystem() const
	{
		return mSystem;
	}

	bool ParticleScriptCompiler::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			// Delegate some processing to the listener
			if(!processNode(i, nodes->end()))
			{
				// The first just the name of the particle system, but ignore "abstract"
				if((*i)->token != "abstract")
				{
					compileParticleSystem(*i);
				}
				++i;
			}
		}
		return mErrors.empty();
	}

	bool ParticleScriptCompiler::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		if(mListener)
			return mListener->processNode(i, end, this);
		return false;
	}

	ScriptNodeListPtr ParticleScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		return nodes;
	}

	void ParticleScriptCompiler::preParse()
	{
		if(mListener)
			mListener->preParse(mWordIDs);
	}

	bool ParticleScriptCompiler::errorRaised(const ScriptCompilerErrorPtr &error)
	{
		return mListener ? mListener->errorRaised(error) : true;
	}

	void ParticleScriptCompiler::compileParticleSystem(const ScriptNodePtr &node)
	{
		// Use the listener to get the particle system object
		if(mListener)
			mSystem = mListener->getParticleSystem(node->token, mGroup);
		else
			mSystem = ParticleSystemManager::getSingleton().createTemplate(node->token, mGroup);
		if(!mSystem)
		{
			addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, node->column);
			return;
		}

		// The particle system does not support extra option after its name, so skip ahead to the '{'
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				// Each property in the particle system has only 1 value associated with it
				String name = (*j)->token, value;

				if(name == "emitter")
				{
					compileEmitter(*j);
				}
				else if(name == "affector")
				{
					compileAffector(*j);
				}
				else
				{
					// Construct the parameter values from the children of the property
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
					if(!mSystem->setParameter(name, value))
					{
						if(mSystem->getRenderer())
						{
							if(!mSystem->getRenderer()->setParameter(name, value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
					}
				}
				++j;
			}
		}

		// Reset the pointer to the system
		mSystem = 0;
	}

	void ParticleScriptCompiler::compileEmitter(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleEmitter *emitter = 0;
		String type = node->children.front()->token;
		try{
			emitter = mSystem->addEmitter(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!emitter->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	void ParticleScriptCompiler::compileAffector(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleAffector *affector = 0;
		String type = node->children.front()->token;
		try{
			affector = mSystem->addAffector(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!affector->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	String ParticleScriptCompiler::getParameterValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		String retval;
		if(i != end)
		{
			if((*i)->type == SNT_WORD || (*i)->type == SNT_QUOTE)
				retval = (*i)->token;
			else if((*i)->type == SNT_NUMBER)
				retval = StringConverter::toString((*i)->data);
		}

		++i;
		while(i != end)
		{
			if((*i)->type == SNT_WORD || (*i)->type == SNT_QUOTE)
				retval = retval + " " + (*i)->token;
			else if((*i)->type == SNT_NUMBER)
				retval = retval + " " + StringConverter::toString((*i)->data);
			++i;
		}

		return retval;
	}
}