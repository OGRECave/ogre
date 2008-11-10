/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9HLSLProgram_H__
#define __D3D9HLSLProgram_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for D3D9 
        High-Level Shader Language (HLSL).
    @remarks
        Note that the syntax of D3D9 HLSL is identical to nVidia's Cg language, therefore
        unless you know you will only ever be deploying on Direct3D, or you have some specific
        reason for not wanting to use the Cg plugin, I suggest you use Cg instead since that
        can produce programs for OpenGL too.
    */
    class D3D9HLSLProgram : public HighLevelGpuProgram
    {
    public:
        /// Command object for setting entry point
        class CmdEntryPoint : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting target assembler
        class CmdTarget : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting macro defines
        class CmdPreprocessorDefines : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /// Command object for setting matrix packing in column-major order
        class CmdColumnMajorMatrices : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
		/// Command object for setting optimisation level
		class CmdOptimisation : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};

    protected:

        static CmdEntryPoint msCmdEntryPoint;
        static CmdTarget msCmdTarget;
        static CmdPreprocessorDefines msCmdPreprocessorDefines;
        static CmdColumnMajorMatrices msCmdColumnMajorMatrices;
		static CmdOptimisation msCmdOptimisation;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadHighLevelImpl(void);
        /// Populate the passed parameters with name->index map, must be overridden
        void buildConstantDefinitions() const;

        // Recursive utility method for buildParamNameMap
        void processParamElement(D3DXHANDLE parent, String prefix, unsigned int index) const;
		void populateDef(D3DXCONSTANT_DESC& d3dDesc, GpuConstantDefinition& def) const;

        String mTarget;
        String mEntryPoint;
        String mPreprocessorDefines;
        bool mColumnMajorMatrices;

        LPD3DXBUFFER mpMicroCode;
        LPD3DXCONSTANTTABLE mpConstTable;
	public:
		/// Shader optimisation level
		enum OptimisationLevel
		{
			/// default - no optimisation in debug mode, OPT_1 in release mode
			OPT_DEFAULT,
			/// No optimisation
			OPT_NONE,
			/// Optimisation level 0
			OPT_0, 
			/// Optimisation level 1
			OPT_1,
			/// Optimisation level 2
			OPT_2, 
			/// Optimisation level 3
			OPT_3
		};
	protected:
		OptimisationLevel mOptimisationLevel;

    public:
        D3D9HLSLProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~D3D9HLSLProgram();

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) const { return mEntryPoint; }
        /** Sets the shader target to compile down to, e.g. 'vs_1_1'. */
        void setTarget(const String& target);
        /** Gets the shader target to compile down to, e.g. 'vs_1_1'. */
        const String& getTarget(void) const { return mTarget; }
        /** Sets the preprocessor defines use to compile the program. */
        void setPreprocessorDefines(const String& defines) { mPreprocessorDefines = defines; }
        /** Sets the preprocessor defines use to compile the program. */
        const String& getPreprocessorDefines(void) const { return mPreprocessorDefines; }
        /** Sets whether matrix packing in column-major order. */ 
        void setColumnMajorMatrices(bool columnMajor) { mColumnMajorMatrices = columnMajor; }
        /** Gets whether matrix packed in column-major order. */
        bool getColumnMajorMatrices(void) const { return mColumnMajorMatrices; }
		/** Sets the optimisation level to use.
		@param opt Optimisation level
		*/
		void setOptimisationLevel(OptimisationLevel opt) { mOptimisationLevel = opt; }

		/** Gets the optimisation level to use. */
		OptimisationLevel getOptimisationLevel() const { return mOptimisationLevel; }

        /// Overridden from GpuProgram
        bool isSupported(void) const;
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);
        /// Overridden from GpuProgram
        const String& getLanguage(void) const;
    };
}

#endif
