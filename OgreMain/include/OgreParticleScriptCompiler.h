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

#ifndef __PARTICLESCRIPTCOMPILER_H_
#define __PARTICLESCRIPTCOMPILER_H_

#include "OgreScriptCompiler.h"

namespace Ogre{

	class ParticleScriptCompiler;
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

}

#endif
