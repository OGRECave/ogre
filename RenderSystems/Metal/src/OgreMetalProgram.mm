/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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
#import "OgreHighLevelGpuProgramManager.h"
#import "OgreHardwareBufferManager.h"
#import "OgreLogManager.h"
#import "OgrePass.h"
#import "OgreTechnique.h"
#import "OgreRoot.h"
#import "OgreStringConverter.h"
#import "OgreMetalRenderSystem.h"
#import "OgreMetalProgram.h"
#import "OgreRenderOperation.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    MetalProgram::CmdPreprocessorDefines MetalProgram::msCmdPreprocessorDefines;
    MetalProgram::CmdEntryPoint MetalProgram::msCmdEntryPoint;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MetalProgram::MetalProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader) 
        , mLibrary(nil), mFunction(nil), mCompiled(0)
    {
        if (createParamDictionary("MetalProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("entry_point",
                                            "The entry point for the Metal program.",
                                            PT_STRING),&msCmdEntryPoint);
            dict->addParameter(ParameterDef("preprocessor_defines",
                                            "Preprocessor defines use to compile the program.",
                                            PT_STRING),&msCmdPreprocessorDefines);
        }
        mTargetBufferName = "";

        // Manually assign language now since we use it immediately
        mSyntaxCode = "metal";
        mEntryPoint = "main_hlms";
    }
    //---------------------------------------------------------------------------
    MetalProgram::~MetalProgram()
    {
        mLibrary = nil;
        mFunction = nil;

        // Have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //-----------------------------------------------------------------------
    void MetalProgram::loadFromSource(void)
    {
    }
    //-----------------------------------------------------------------------
    bool MetalProgram::compile(const bool checkErrors)
    {
        return (mCompiled == 1);
    }
    //-----------------------------------------------------------------------
    void MetalProgram::createLowLevelImpl(void)
    {
        mAssemblerProgram = GpuProgramPtr(this);
        if(!mCompiled)
            compile(true);
    }
    //---------------------------------------------------------------------------
    void MetalProgram::unloadImpl()
    {   
        // We didn't create mAssemblerProgram through a manager, so override this
        // implementation so that we don't try to remove it from one. Since getCreator()
        // is used, it might target a different matching handle!
        mAssemblerProgram.setNull();

        unloadHighLevel();
    }
    //-----------------------------------------------------------------------
    void MetalProgram::unloadHighLevelImpl(void)
    {
        if (isSupported())
        {
            // Release everything
            if(mLibrary)
            {
                mLibrary = nil;
            }

            if(mFunction)
            {
                mFunction = nil;
            }
            mCompiled = 0;
        }
    }
    //-----------------------------------------------------------------------------
    void MetalProgram::bindProgram(void)
    {
        LogManager::getSingleton().logMessage("Deleting shader program " + mName);
    }
    //-----------------------------------------------------------------------------
    void MetalProgram::unbindProgram(void)
    {
    }
    //-----------------------------------------------------------------------------
    void MetalProgram::bindProgramParameters(id <MTLRenderCommandEncoder> &encoder, GpuProgramParametersSharedPtr params, uint16 mask)
    {

    }
    //-----------------------------------------------------------------------
    void MetalProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        getConstantDefinitions();
        params->_setNamedConstants(mConstantDefs);
    }
    //-----------------------------------------------------------------------
    void MetalProgram::buildConstantDefinitions() const
    {

    }
    //---------------------------------------------------------------------
    void MetalProgram::bindUniformBuffers(id <MTLRenderCommandEncoder> &encoder, const Renderable *rend, const Pass *pass, const GpuProgramParametersSharedPtr params, MTLRenderPipelineDescriptor *pipeline)
    {

    }
    //-----------------------------------------------------------------------
    void MetalProgram::updateAttributeBuffer(id <MTLRenderCommandEncoder> &encoder, const size_t index, const size_t vertexStart, v1::HardwareVertexBufferSharedPtr hwBuffer)
    {

    }
    //---------------------------------------------------------------------
    inline bool MetalProgram::getPassSurfaceAndLightStates(void) const
    {
        // Scenemanager should pass on light & material state to the rendersystem
        return true;
    }
    //---------------------------------------------------------------------
    inline bool MetalProgram::getPassTransformStates(void) const
    {
        // Scenemanager should pass on transform state to the rendersystem
        return true;
    }
    //---------------------------------------------------------------------
    inline bool MetalProgram::getPassFogStates(void) const
    {
        // Scenemanager should pass on fog state to the rendersystem
        return true;
    }
    //-----------------------------------------------------------------------
    String MetalProgram::CmdEntryPoint::doGet(const void *target) const
    {
        return static_cast<const MetalProgram*>(target)->getEntryPoint();
    }
    void MetalProgram::CmdEntryPoint::doSet(void *target, const String& val)
    {
        static_cast<MetalProgram*>(target)->setEntryPoint(val);
    }
    //-----------------------------------------------------------------------
    String MetalProgram::CmdPreprocessorDefines::doGet(const void *target) const
    {
        return static_cast<const MetalProgram*>(target)->getPreprocessorDefines();
    }
    //-----------------------------------------------------------------------
    void MetalProgram::CmdPreprocessorDefines::doSet(void *target, const String& val)
    {
        static_cast<MetalProgram*>(target)->setPreprocessorDefines(val);
    }
    //-----------------------------------------------------------------------
    const String& MetalProgram::getLanguage(void) const
    {
        static const String language = "metal";

        return language;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr MetalProgram::createParameters( void )
    {
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();
        params->setTransposeMatrices(true);
        return params;
    }
    //-----------------------------------------------------------------------
    NSUInteger MetalProgram::getFunctionParamCount(void)
    {
        return 0;
    }
    //-----------------------------------------------------------------------
    size_t MetalProgram::getSharedParamCount(void)
    {
        return 0;
    }
    //-----------------------------------------------------------------------
}
