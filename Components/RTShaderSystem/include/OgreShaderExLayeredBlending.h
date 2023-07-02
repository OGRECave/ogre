/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _OgreShaderExLayeredBlending_
#define _OgreShaderExLayeredBlending_

#include "OgreShaderPrerequisites.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderSubRenderState.h"

namespace Ogre {
namespace RTShader {

enum BlendMode : int;
enum SourceModifier : int;

class _OgreRTSSExport LayeredBlending : public FFPTexturing
{
public:
    /** Class default constructor */
    LayeredBlending();

    /** 
    @see SubRenderState::getType.
    */
    const Ogre::String& getType                 () const override;


    /** 
    Set the blend mode of the given texture unit layer with the previous layer.
    @param index The texture unit texture. Textures units (index-1) and (index) will be blended.
    @param mode The blend mode to apply.
    */
    bool setBlendMode(uint16 index, const String& mode);

    /** 
    Return the blend mode of the given texture unit index.
    */
    BlendMode getBlendMode(unsigned short index) const;

    

    /** 
    Set the source modifier parameters for a given texture unit
    @param index Texture blend index
    @param modType The source modification type to use
    @param customNum The custom parameter number used to control the modification
    */
    bool setSourceModifier(unsigned short index, const String& modType, int customNum);

    /** 
    Returns the source modifier parameters for a given texture unit
    @return True if a valid modifier exist for the given texture unit
    @param index Texture blend index
    @param modType The source modification type to use
    @param customNum The custom parameter number used to control the modification
    */
    bool getSourceModifier(unsigned short index, SourceModifier& modType, int& customNum) const;

    /** 
    @see SubRenderState::copyFrom.
    */
    void copyFrom(const SubRenderState& rhs) override;

    static String Type;

private:
    
    /** 
    @see SubRenderState::resolveParameters.
    */
    bool resolveParameters(ProgramSet* programSet) override;

    /** 
    @see SubRenderState::resolveDependencies.
    */
    bool resolveDependencies(Ogre::RTShader::ProgramSet* programSet) override;


    void addPSBlendInvocations(Function* psMain,
                                       ParameterPtr arg1,
                                       ParameterPtr arg2,
                                       ParameterPtr texel,
                                       int samplerIndex,
                                       const LayerBlendModeEx& blendMode,
                                       const int groupOrder, 
                                       Operand::OpMask targetChannels) override;
    /** 
    Adds the function invocation to the pixel shader which will modify
    the blend sources according to the source modification parameters.
    */
    void addPSModifierInvocation(Function* psMain, 
                                 int samplerIndex, 
                                 ParameterPtr arg1,
                                 ParameterPtr arg2,
                                 const int groupOrder, 
                                 Operand::OpMask targetChannels);

    struct TextureBlend
    {
        TextureBlend();
        //The blend mode to use
        BlendMode blendMode;
        //The source modification to use
        SourceModifier sourceModifier;
        // The number of the custom param controlling the source modification
        int customNum;
        //The parameter controlling the source modification
        ParameterPtr modControlParam;
    };
    std::vector<TextureBlend> mTextureBlends;

};

} // namespace RTShader
} // namespace Ogre

#endif // RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#endif // _OgreShaderExLayeredBlending_
