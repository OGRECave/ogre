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
    @remarks
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

    /** Acquire CPU/GPU programs set associated with the given render state and bind them to the pass.
    @param pass The pass to bind the programs to.
    @param renderState The render state that describes the program that need to be generated.
    */
    void acquirePrograms(Pass* pass, TargetRenderState* renderState);

    /** Release CPU/GPU programs set associated with the given render state and pass.
    @param pass The pass to release the programs from.
    @param renderState The render state holds the programs.
    */
    void releasePrograms(Pass* pass, TargetRenderState* renderState);

    /** Flush the local GPU programs cache.
    */
    void flushGpuProgramsCache();

protected:

    //-----------------------------------------------------------------------------
    typedef std::map<String, GpuProgramPtr>            GpuProgramsMap;
    typedef GpuProgramsMap::iterator                    GpuProgramsMapIterator;
    typedef GpuProgramsMap::const_iterator              GpuProgramsMapConstIterator;

    //-----------------------------------------------------------------------------
    typedef std::set<Program*>                         ProgramList;
    typedef ProgramList::iterator                       ProgramListIterator;
    typedef std::map<String, ProgramWriter*>           ProgramWriterMap;
    typedef ProgramWriterMap::iterator                  ProgramWriterIterator;
    typedef std::vector<ProgramWriterFactory*>         ProgramWriterFactoryList;
    
    //-----------------------------------------------------------------------------
    typedef std::map<String, ProgramProcessor*>        ProgramProcessorMap;
    typedef ProgramProcessorMap::iterator               ProgramProcessorIterator;
    typedef ProgramProcessorMap::const_iterator         ProgramProcessorConstIterator;
    typedef std::vector<ProgramProcessor*>             ProgramProcessorList;

    
protected:
    /** Create default program processors. */
    void createDefaultProgramProcessors();
    
    /** Destroy default program processors. */
    void destroyDefaultProgramProcessors();

    /** Create default program processors. */
    void createDefaultProgramWriterFactories();

    /** Destroy default program processors. */
    void destroyDefaultProgramWriterFactories();

    /** Destroy all program writers. */
    void destroyProgramWriters();

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
    bool createGpuPrograms(ProgramSet* programSet);
        
    /** 
    Generates a unique hash from a string
    @param programString string to generate a hash value for
    @return A string representing a 128 bit hash value of the original string
    */
    static String generateHash(const String& programString);

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
        const StringVector& profilesList,
        const String& cachePath);

    /** 
    Add program processor instance to this manager.
    @param processor The instance to add.
    */
    void addProgramProcessor(ProgramProcessor* processor);

    /** 
    Remove program processor instance from this manager. 
    @param processor The instance to remove.
    */
    void removeProgramProcessor(ProgramProcessor* processor);

    /** Destroy a GPU program by name.
    @param gpuProgram The program to destroy.
    */
    void destroyGpuProgram(GpuProgramPtr& gpuProgram);

    /** Flush the local GPU programs cache.
    @param gpuProgramsMap The GPU programs cache.
    */
    void flushGpuProgramsCache(GpuProgramsMap& gpuProgramsMap);
    
    /** Return the number of created shaders. */
    size_t getShaderCount(GpuProgramType type) const;

    /** Fix the input of the pixel shader to be the same as the output of the vertex shader */
    void synchronizePixelnToBeVertexOut(ProgramSet* programSet);

    /** Bind the uniform parameters of a given CPU and GPU program set. */
    void bindUniformParameters(Program* pCpuProgram, const GpuProgramParametersSharedPtr& passParams);


protected:
    

protected:
    // CPU programs list.                   
    ProgramList mCpuProgramsList;
    // Map between target language and shader program writer.                   
    ProgramWriterMap mProgramWritersMap;
    // Map between target language and shader program processor.    
    ProgramProcessorMap mProgramProcessorsMap;
    // Holds standard shader writer factories
    ProgramWriterFactoryList mProgramWriterFactories;
    // The generated vertex shaders.
    GpuProgramsMap mVertexShaderMap;
    // The generated fragment shaders.
    GpuProgramsMap mFragmentShaderMap;
    // The default program processors.
    ProgramProcessorList mDefaultProgramProcessors;

private:
    friend class ProgramSet;
    friend class TargetRenderState;
    friend class ShaderGenerator;
};

/** @} */
/** @} */

}
}

#endif

