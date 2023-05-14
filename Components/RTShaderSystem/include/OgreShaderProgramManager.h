/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd
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
#ifndef _ShaderProgramManager_
#define _ShaderProgramManager_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreGpuProgram.h"
#include "OgreStringVector.h"

namespace Ogre {
namespace RTShader {

    class ProgramWriter;
    class ProgramProcessor;

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A singleton manager class that manages shader based programs.
*/
class _OgreRTSSExport ProgramManager : public Singleton<ProgramManager>, public RTShaderSystemAlloc
{
// Interface.
public:

    /** Class default constructor */
    ProgramManager();

    /** Class destructor */
    ~ProgramManager();


    /** Override standard Singleton retrieval.

    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ProgramManager& getSingleton();  

    /// @copydoc Singleton::getSingleton()
    static ProgramManager* getSingletonPtr();

    /** Release CPU/GPU programs set associated with the given ProgramSet
    @param programSet The ProgramSet holds the programs.
    */
    void releasePrograms(const ProgramSet* programSet);

    /** Flush the local GPU programs cache.
    */
    void flushGpuProgramsCache();

private:

    //-----------------------------------------------------------------------------
    typedef std::map<String, GpuProgramPtr>            GpuProgramsMap;
    typedef GpuProgramsMap::iterator                    GpuProgramsMapIterator;
    typedef GpuProgramsMap::const_iterator              GpuProgramsMapConstIterator;

    //-----------------------------------------------------------------------------
    typedef std::set<Program*>                         ProgramList;
    typedef ProgramList::iterator                       ProgramListIterator;
    typedef std::map<String, ProgramWriter*>           ProgramWriterMap;
    typedef ProgramWriterMap::iterator                  ProgramWriterIterator;
    
    //-----------------------------------------------------------------------------
    typedef std::map<String, ProgramProcessor*>        ProgramProcessorMap;
    typedef ProgramProcessorMap::iterator               ProgramProcessorIterator;
    typedef ProgramProcessorMap::const_iterator         ProgramProcessorConstIterator;
    typedef std::vector<ProgramProcessor*>             ProgramProcessorList;


    /** Create default program processors. */
    void createDefaultProgramProcessors();
    
    /** Destroy default program processors. */
    void destroyDefaultProgramProcessors();

    /** Create CPU program .    
    @param type The type of the program to create.
    */
    Program* createCpuProgram(GpuProgramType type);

    /** Destroy a CPU program by name.
    @param shaderProgram The CPU program instance to destroy.
    */
    void destroyCpuProgram(Program* shaderProgram);

    /** Create GPU programs for the given program set based on the CPU programs it contains.
    @param programSet The program set container.
    */
    void createGpuPrograms(ProgramSet* programSet);
        
    /** 
    Generates a unique hash from a string
    @param programString source code to generate a hash value for
    @param defines defines for the final source code
    @return A string representing a 128 bit hash value of the original string
    */
    static String generateHash(const String& programString, const String& defines);

    /** Create GPU program based on the give CPU program.
    @param shaderProgram The CPU program instance.
    @param programWriter The program writer instance.
    @param language The target shader language.
    @param profiles The profiles string for program compilation.
    @param profilesList The profiles string for program compilation as string list.
    @param cachePath The output path to write the program into.
    */
    GpuProgramPtr createGpuProgram(Program* shaderProgram, 
        ProgramWriter* programWriter,
        const String& language,
        const String& profiles,
        const String& cachePath);

    /** 
    Add program processor instance to this manager.
    @param processor The instance to add.
    */
    void addProgramProcessor(const String& lang, ProgramProcessor* processor);

    /** 
    Remove program processor instance from this manager. 
    @param processor The instance to remove.
    */
    void removeProgramProcessor(const String& lang);
    
    /** Return the number of created shaders. */
    size_t getShaderCount(GpuProgramType type) const;

    /** Fix the input of the pixel shader to be the same as the output of the vertex shader */
    void matchVStoPSInterface(ProgramSet* programSet);

    // Map between target language and shader program processor.    
    ProgramProcessorMap mProgramProcessorsMap;
    // The generated shaders.
    std::vector<GpuProgramPtr> mShaderList;
    // The default program processors.
    ProgramProcessorList mDefaultProgramProcessors;

    friend class ProgramSet;
    friend class TargetRenderState;
    friend class ShaderGenerator;
};

/** @} */
/** @} */

}
}

#endif

