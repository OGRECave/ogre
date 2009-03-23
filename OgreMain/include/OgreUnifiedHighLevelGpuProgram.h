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
#ifndef __UnifiedHighLevelGpuProgram_H__
#define __UnifiedHighLevelGpuProgram_H__

#include "OgrePrerequisites.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Specialisation of HighLevelGpuProgram which just delegates its implementation
		to one other high level program, allowing a single program definition
		to represent one supported program from a number of options
	@remarks
		Whilst you can use Technique to implement several ways to render an object
		depending on hardware support, if the only reason to need multiple paths is
		because of the high-level shader language supported, this can be 
		cumbersome. For example you might want to implement the same shader 
		in HLSL and	GLSL for portability but apart from the implementation detail,
		the shaders do the same thing and take the same parameters. If the materials
		in question are complex, duplicating the techniques just to switch language
		is not optimal, so instead you can define high-level programs with a 
		syntax of 'unified', and list the actual implementations in order of
		preference via repeated use of the 'delegate' parameter, which just points
		at another program name. The first one which has a supported syntax 
		will be used.
	*/
	class _OgreExport UnifiedHighLevelGpuProgram : public HighLevelGpuProgram
	{
	public:
		/// Command object for setting delegate (can set more than once)
		class CmdDelegate : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};

	protected:
		static CmdDelegate msCmdDelegate;

		/// Ordered list of potential delegates
		StringVector mDelegateNames;
		/// The chosen delegate
		mutable HighLevelGpuProgramPtr mChosenDelegate;

		/// Choose the delegate to use
		void chooseDelegate() const;

		void createLowLevelImpl(void);
		void unloadHighLevelImpl(void);
		void buildConstantDefinitions() const;
		void loadFromSource(void);

	public:
		/** Constructor, should be used only by factory classes. */
		UnifiedHighLevelGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
			const String& group, bool isManual = false, ManualResourceLoader* loader = 0);
		~UnifiedHighLevelGpuProgram();


		/** Adds a new delegate program to the list.
		@remarks
			Delegates are tested in order so earlier ones are preferred.
		*/
		void addDelegateProgram(const String& name);

		/// Remove all delegate programs
		void clearDelegatePrograms();

		/// Get the chosen delegate
		const HighLevelGpuProgramPtr& _getDelegate() const;

		/** @copydoc GpuProgram::getLanguage */
        const String& getLanguage(void) const;

		/** Creates a new parameters object compatible with this program definition. 
		@remarks
		Unlike low-level assembly programs, parameters objects are specific to the
		program and therefore must be created from it rather than by the 
		HighLevelGpuProgramManager. This method creates a new instance of a parameters
		object containing the definition of the parameters this program understands.
		*/
		GpuProgramParametersSharedPtr createParameters(void);
		/** @copydoc GpuProgram::getBindingDelegate */
		GpuProgram* _getBindingDelegate(void);

		// All the following methods must delegate to the implementation

		/** @copydoc GpuProgram::isSupported */
		bool isSupported(void) const;
		
		/** @copydoc GpuProgram::isSkeletalAnimationIncluded */
		bool isSkeletalAnimationIncluded(void) const;

		bool isMorphAnimationIncluded(void) const;

		bool isPoseAnimationIncluded(void) const;

		bool isVertexTextureFetchRequired(void) const;
		GpuProgramParametersSharedPtr getDefaultParameters(void);
		bool hasDefaultParameters(void) const;
		bool getPassSurfaceAndLightStates(void) const;
		bool getPassFogStates(void) const;
		bool getPassTransformStates(void) const;
		bool hasCompileError(void) const;
		void resetCompileError(void);

		void load(bool backgroundThread = false);
		void reload(void);
		bool isReloadable(void) const;
		bool isLoaded(void) const;
		bool isLoading() const;
		LoadingState getLoadingState() const;
		void unload(void);
		size_t getSize(void) const;
		void touch(void);
		bool isBackgroundLoaded(void) const;
		void setBackgroundLoaded(bool bl);
		void escalateLoading();
		void addListener(Listener* lis);
		void removeListener(Listener* lis);

	};

	/** Factory class for Unified programs. */
	class UnifiedHighLevelGpuProgramFactory : public HighLevelGpuProgramFactory
	{
	public:
		UnifiedHighLevelGpuProgramFactory();
		~UnifiedHighLevelGpuProgramFactory();
		/// Get the name of the language this factory creates programs for
		const String& getLanguage(void) const;
		HighLevelGpuProgram* create(ResourceManager* creator, 
			const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader);
		void destroy(HighLevelGpuProgram* prog);

	};

	/** @} */
	/** @} */

}
#endif
