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

#ifndef __CompositorScriptScompiler2_H__
#define __CompositorScriptScompiler2_H__

#include "OgreScriptCompiler.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
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
	/** @} */
	/** @} */

}

#endif
