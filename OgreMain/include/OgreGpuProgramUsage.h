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
#ifndef __GpuProgramUsage_H__
#define __GpuProgramUsage_H__

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"


namespace Ogre 
{
    /** This class makes the usage of a vertex and fragment programs (low-level or high-level), 
        with a given set of parameters, explicit.
    @remarks
        Using a vertex or fragment program can get fairly complex; besides the fairly rudimentary
        process of binding a program to the GPU for rendering, managing usage has few
        complications, such as:
        <ul>
        <li>Programs can be high level (e.g. Cg, RenderMonkey) or low level (assembler). Using
        either should be relatively seamless, although high-level programs give you the advantage
        of being able to use named parameters, instead of just indexed registers</li>
        <li>Programs and parameters can be shared between multiple usages, in order to save
        memory</li>
        <li>When you define a user of a program, such as a material, you often want to be able to
        set up the definition but not load / compile / assemble the program at that stage, because
        it is not needed just yet. The program should be loaded when it is first needed, or
        earlier if specifically requested. The program may not be defined at this time, you
        may want to have scripts that can set up the definitions independent of the order in which
        those scripts are loaded.</li>
        </ul>
        This class packages up those details so you don't have to worry about them. For example,
        this class lets you define a high-level program and set up the parameters for it, without
        having loaded the program (which you normally could not do). When the program is loaded and
        compiled, this class will then validate the parameters you supplied earlier and turn them
        into runtime parameters.
    @par
        Just incase it wasn't clear from the above, this class provides linkage to both 
        GpuProgram and HighLevelGpuProgram, despite its name.
    */
	class _OgreExport GpuProgramUsage : public PassAlloc
    {
    protected:
        GpuProgramType mType;
        // The program link
        GpuProgramPtr mProgram;

        /// program parameters
        GpuProgramParametersSharedPtr mParameters;


    public:
        /** Default constructor.
        @param gptype The type of program to link to
        */
        GpuProgramUsage(GpuProgramType gptype);

		/** Copy constructor */
		GpuProgramUsage(const GpuProgramUsage& rhs);

        /** Gets the type of program we're trying to link to. */
        GpuProgramType getType(void) const { return mType; }

		/** Sets the name of the program to use. 
        @param name The name of the program to use
        @param resetParams
            If true, this will create a fresh set of parameters from the
            new program being linked, so if you had previously set parameters
            you will have to set them again. If you set this to false, you must
            be absolutely sure that the parameters match perfectly, and in the
            case of named parameters refers to the indexes underlying them, 
            not just the names.
        */
		void setProgramName(const String& name, bool resetParams = true);
		/** Sets the program to use.
        @remarks
            Note that this will create a fresh set of parameters from the
            new program being linked, so if you had previously set parameters
            you will have to set them again.
        */
        void setProgram(GpuProgramPtr& prog);
		/** Gets the program being used. */
        const GpuProgramPtr& getProgram() const { return mProgram; }
		/** Gets the program being used. */
        const String& getProgramName(void) const { return mProgram->getName(); }

        /** Sets the program parameters that should be used; because parameters can be
            shared between multiple usages for efficiency, this method is here for you
            to register externally created parameter objects. Otherwise, the parameters
            will be created for you when a program is linked.
        */
        void setParameters(GpuProgramParametersSharedPtr params);
        /** Gets the parameters being used here. 
        */
        GpuProgramParametersSharedPtr getParameters(void);

        /// Load this usage (and ensure program is loaded)
        void _load(void);
        /// Unload this usage 
        void _unload(void);

    };
}
#endif
