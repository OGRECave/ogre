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

#ifndef __CompositorScriptScompiler2_H__
#define __CompositorScriptScompiler2_H__

#include "OgreScriptCompiler.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

namespace Ogre
{
	class CompositorScriptCompiler2;
	
	/** This is the listener specific to the compositor compiler. It allows for
		notifications and overriding of compiler functions.
	*/
	class _OgreExport CompositorScriptCompilerListener : public ScriptCompilerListener
	{
	public:
		CompositorScriptCompilerListener();

		/// Override this to customly allocate compositors
		Compositor *getCompositor(const String &name, const String &group);
		/// Override this to do custom processing
		virtual bool processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, CompositorScriptCompiler2*);
	};

	/** This class uses the new ScriptCompiler to compile compositor scripts.
		It uses a word id map, and allows a listener to override resource acquisitions
		and allocations.
	*/
	class _OgreExport CompositorScriptCompiler2 : public ScriptCompiler
	{
	public:
		enum
		{
			ID_COMPOSITOR,
			ID_TECHNIQUE,
			ID_TARGET,
			ID_TARGET_OUTPUT,
			ID_PASS,

			ID_TEXTURE,
			ID_INPUT,
				ID_NONE,
				ID_PREVIOUS,
				ID_TARGET_WIDTH,
				ID_TARGET_HEIGHT,
			ID_ONLY_INITIAL,
			ID_VISIBILITY_MASK,
			ID_LOD_BIAS,
			ID_MATERIAL_SCHEME,

			ID_CLEAR,
			ID_STENCIL,
			ID_RENDER_SCENE,
			ID_RENDER_QUAD,
			ID_MATERIAL,
			ID_IDENTIFIER,
			ID_FIRST_RENDER_QUEUE,
			ID_LAST_RENDER_QUEUE,

			ID_BUFFERS,
				ID_COLOUR,
				ID_DEPTH,
			ID_COLOUR_VALUE,
			ID_DEPTH_VALUE,
			ID_STENCIL_VALUE,

			ID_CHECK,
			ID_COMP_FUNC,
				ID_ALWAYS_FAIL,
				ID_ALWAYS_PASS,
				ID_LESS_EQUAL,
				ID_LESS,
				ID_EQUAL,
				ID_NOT_EQUAL,
				ID_GREATER_EQUAL,
				ID_GREATER,
			ID_REF_VALUE,
			ID_MASK,
			ID_FAIL_OP,
				ID_KEEP,
				ID_ZERO,
				ID_REPLACE,
				ID_INCREMENT,
				ID_DECREMENT,
				ID_INCREMENT_WRAP,
				ID_DECREMENT_WRAP,
				ID_INVERT,
			ID_DEPTH_FAIL_OP,
			ID_PASS_OP,
			ID_TWO_SIDED
		};
	public:
		CompositorScriptCompiler2();

		/// Sets the listener used for overriding compiler functions
		void setListener(CompositorScriptCompilerListener *listener);
	protected:
		/// This is the implementation for compiling the compositor scripts
		bool compileImpl(ScriptNodeListPtr nodes);
		/// This allows for overriding processing of script nodes
		bool processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		/// This is the override for loading imports
		ScriptNodeListPtr loadImportPath(const String &name);
		/// Allows a listener to override the word id map before parsing
		void preParse();
		/// Allows a listener to override error handling in the compiler
		bool errorRaised(const ScriptCompilerErrorPtr &error);
	private: // Custom node handling
		void compileCompositor(const ScriptNodePtr &node);
		void compileTechnique(const ScriptNodePtr &node);
		void compileTarget(const ScriptNodePtr &node, CompositionTechnique *technique);
		void compileTargetOutput(const ScriptNodePtr &node, CompositionTechnique *technique);
		void compilePass(const ScriptNodePtr &node, CompositionTargetPass *target);
		void compileTargetOptions(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, CompositionTargetPass *target);
		bool getCompareFunction(const ScriptNodePtr &node, CompareFunction &func);
		bool getStencilOp(const ScriptNodePtr &node, StencilOperation &op);
		bool getColourValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c);
	private:
		// The listener
		CompositorScriptCompilerListener *mListener;
		// This is the compositor being compiled
		Compositor *mCompositor;
	};

}

#endif
