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

#ifndef __PARTICLESCRIPTCOMPILER_H_
#define __PARTICLESCRIPTCOMPILER_H_

#include "OgreScriptCompiler.h"

namespace Ogre{

	class ParticleScriptCompiler;
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** This is the specific listener for the particle script compiler.
		It allows overriding behavior for specific functionality of this compiler.
	*/
	class _OgreExport ParticleScriptCompilerListener : public ScriptCompilerListener
	{
	public:
		ParticleScriptCompilerListener();
		/// Override this to do custom processing of the script nodes
		virtual bool processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, ParticleScriptCompiler*);
		/// This provides the compiler with the particle system it wishes to compile into. Override it for custom system allocations.
		virtual ParticleSystem *getParticleSystem(const String &name, const String &group);
	};

	class _OgreExport ParticleScriptCompiler : public ScriptCompiler
	{
	public:
		ParticleScriptCompiler();

		/// Sets the listener for this compiler
		void setListener(ParticleScriptCompilerListener *listener);
		/// Returns the particle system currently being compiled
		ParticleSystem *getParticleSystem() const;
	protected:
		/// This begins the compilation of the particle system from the final transformed AST
		bool compileImpl(ScriptNodeListPtr nodes);
		/// Delegates to the listener if it can, otherwise returns false. If it returns true, then some input was consumed.
		bool processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		/// This is the override for loading imports
		ScriptNodeListPtr loadImportPath(const String &name);
		/// Allows a listener to override the word id map before parsing
		void preParse();
		/// Allows a listener to override error handling in the compiler
		bool errorRaised(const ScriptCompilerErrorPtr &error);
	private: // Handlers for compiling script elements
		void compileParticleSystem(const ScriptNodePtr &node);
		void compileEmitter(const ScriptNodePtr &node);
		void compileAffector(const ScriptNodePtr &node);
		String getParameterValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
	private: // Listener and context data
		ParticleScriptCompilerListener *mListener;

		// The system being compiled
		ParticleSystem *mSystem;
	};
	/** @} */
	/** @} */

}

#endif
