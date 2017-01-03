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
#include "OgreQuake3ShaderManager.h"
#include "OgreQuake3Shader.h"
#include "OgreStringVector.h"
#include "OgreException.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> Quake3ShaderManager *Singleton<Quake3ShaderManager>::msSingleton = 0;
    Quake3ShaderManager* Quake3ShaderManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    Quake3ShaderManager& Quake3ShaderManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    Quake3ShaderManager::Quake3ShaderManager()
    {
        mScriptPatterns.push_back("*.shader");
        ResourceGroupManager::getSingleton()._registerScriptLoader(this);
    }
    //-----------------------------------------------------------------------
    Quake3ShaderManager::~Quake3ShaderManager()
    {
        // delete all shaders
        clear();
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
    //-----------------------------------------------------------------------
    const StringVector& Quake3ShaderManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //-----------------------------------------------------------------------
    Real Quake3ShaderManager::getLoadingOrder(void) const
    {
        return 110.0f;
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::clear(void)
    {
        for (Quake3ShaderMap::iterator i = mShaderMap.begin();
            i != mShaderMap.end(); ++i)
        {
            OGRE_DELETE i->second;
        }
        mShaderMap.clear();
    }
    //-----------------------------------------------------------------------
    Quake3Shader* Quake3ShaderManager::getByName(const String& name)
    {
        Quake3ShaderMap::iterator i = mShaderMap.find(name);
        if (i == mShaderMap.end())
        {
            return 0;
        }
        return i->second;
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseScript(DataStreamPtr& stream, const String& group)
    {
        String line;
        Quake3Shader* pShader;
        char tempBuf[512];

        pShader = 0;
        bool dummy = false;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (pShader == 0)
                {
                    // No current shader
                    if (getByName(line) == 0)
                    {
                        dummy = false;
                    }
                    else
                    {
                        // Defined before, parse but ignore
                        // Q3A has duplicates in shaders, doh
                        dummy = true;
                    }

                    // So first valid data should be a shader name
                    pShader = create(line);
                    // Skip to and over next {
                    stream->readLine(tempBuf, 511, "{");
                }
                else
                {
                    // Already in a shader
                    if (line == "}")
                    {
                        // Finished shader
                        if (dummy && pShader)
                        {
                            OGRE_DELETE pShader;
                        }
                        pShader = 0;
                    }
                    else if (line == "{")
                    {
                        // new pass
                        parseNewShaderPass(stream, pShader);

                    }
                    else
                    {
                        // Attribute
                        StringUtil::toLowerCase(line);
                        parseShaderAttrib(line, pShader);
                    }

                }

            }


        }

    }
    //-----------------------------------------------------------------------
    Quake3Shader* Quake3ShaderManager::create( const String& name)
    {
        // Gah, Q3A shader scripts include some duplicates - grr
        Quake3Shader* s = OGRE_NEW Quake3Shader(name);
        if (mShaderMap.find(name) == mShaderMap.end())
        {
            mShaderMap[name] = s;
        }
        else
        {
            // deliberately ignore, will get parsed again but will not be used
        }
        return s;
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseNewShaderPass(DataStreamPtr& stream, Quake3Shader* pShader)
    {
        String line;
        int passIdx;

        passIdx = pShader->numPasses;
        pShader->numPasses++;
        pShader->pass.resize(pShader->numPasses);

        // Default pass details
        pShader->pass[passIdx].animNumFrames = 0;
        pShader->pass[passIdx].blend = LBO_REPLACE;
        pShader->pass[passIdx].blendDest = SBF_ZERO;
        pShader->pass[passIdx].blendSrc = SBF_ONE;
        pShader->pass[passIdx].depthFunc = CMPF_LESS_EQUAL;
        pShader->pass[passIdx].flags = 0;
        pShader->pass[passIdx].rgbGenFunc = SHADER_GEN_IDENTITY;
        pShader->pass[passIdx].tcModRotate = 0;
        pShader->pass[passIdx].tcModScale[0] = pShader->pass[passIdx].tcModScale[1] = 1.0;
        pShader->pass[passIdx].tcModScroll[0] = pShader->pass[passIdx].tcModScroll[1] = 0;
        pShader->pass[passIdx].tcModStretchWave = SHADER_FUNC_NONE;
        pShader->pass[passIdx].tcModTransform[0] = pShader->pass[passIdx].tcModTransform[1] = 0;
        pShader->pass[passIdx].tcModTurbOn = false;
        pShader->pass[passIdx].tcModTurb[0] = pShader->pass[passIdx].tcModTurb[1] =
            pShader->pass[passIdx].tcModTurb[2] = pShader->pass[passIdx].tcModTurb[3] = 0;
        pShader->pass[passIdx].texGen = TEXGEN_BASE;
        pShader->pass[passIdx].addressMode = TextureUnitState::TAM_WRAP;
        pShader->pass[passIdx].customBlend = false;
        pShader->pass[passIdx].alphaVal = 0;
        pShader->pass[passIdx].alphaFunc = CMPF_ALWAYS_PASS;

        while (!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (line.length() != 0 && line.substr(0,2) != "//")
            {
                if (line == "}")
                {
                    // end of shader
                    return;
                }
                else
                {
                    parseShaderPassAttrib(line, pShader, &pShader->pass[passIdx]);
                }
            }


        }
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseShaderAttrib( const String& line, Quake3Shader* pShader)
    {
        StringVector vecparams;

        vecparams = StringUtil::split(line, " \t");
        StringVector::iterator params = vecparams.begin();

        if (params[0] == "skyparms")
        {
            if (params[1] != "-")
            {
                pShader->farbox = true;
                pShader->farboxName = params[1];
            }
            if (params[2] != "-")
            {
                pShader->skyDome = true;
                pShader->cloudHeight = atof(params[2].c_str());
            }
            // nearbox not supported
        }
        else if (params[0] == "cull")
        {
            if (params[1] == "disable" || params[1] == "none")
            {
                pShader->cullMode = MANUAL_CULL_NONE;
            }
            else if (params[1] == "front")
            {
                pShader->cullMode = MANUAL_CULL_FRONT;
            }
            else if (params[1] == "back")
            {
                pShader->cullMode = MANUAL_CULL_BACK;
            }
        }
        else if (params[0] == "deformvertexes")
        {
            // TODO
        }
        else if (params[0] == "fogparms")
        {
            Real r,g,b;
            r = atof(params[1].c_str());
            g = atof(params[2].c_str());
            b = atof(params[3].c_str());
            pShader->fog = true;
            pShader->fogColour = ColourValue(r,g,b);
            pShader->fogDistance = atof(params[4].c_str());

        }
    }
    //-----------------------------------------------------------------------
    void Quake3ShaderManager::parseShaderPassAttrib( const String& line, Quake3Shader* pShader, Quake3Shader::Pass* pPass)
    {
        StringVector vecparams;

        vecparams = StringUtil::split(line, " \t");
        StringVector::iterator params = vecparams.begin();

        StringUtil::toLowerCase(params[0]);
        if (params[0] != "map" && params[0] != "clampmap" && params[0] != "animmap")
        {
            // lower case all except textures
            for (size_t i = 1; i < vecparams.size(); ++i)
                StringUtil::toLowerCase(params[i]);
        }


        // MAP
        if (params[0] == "map")
        {
            pPass->textureName = params[1];
            StringUtil::toLowerCase(params[1]);
            if (params[1] == "$lightmap")
                pPass->texGen = TEXGEN_LIGHTMAP;
        }
        // CLAMPMAP
        if (params[0] == "clampmap")
        {
            pPass->textureName = params[1];
            StringUtil::toLowerCase(params[1]);
            if (params[1] == "$lightmap")
                pPass->texGen = TEXGEN_LIGHTMAP;
            pPass->addressMode = TextureUnitState::TAM_CLAMP;
        }
        // ANIMMAP
        else if (params[0] == "animmap")
        {
            pPass->animFps = atof(params[1].c_str());
            pPass->animNumFrames = static_cast<unsigned int>( vecparams.size() - 2 );
            for (unsigned int frame = 0; frame < pPass->animNumFrames; ++frame)
            {
                pPass->frames[frame] = params[frame+2];
            }
        }
        // BLENDFUNC
        else if (params[0] == "blendfunc")
        {
            if (params[1] == "add" || params[1] == "gl_add")
            {
                pPass->blend = LBO_ADD;
                pPass->blendDest = SBF_ONE;
                pPass->blendSrc = SBF_ONE;
            }
            else if (params[1] == "filter" || params[1] == "gl_filter")
            {
                pPass->blend = LBO_MODULATE;
                pPass->blendDest = SBF_ZERO;
                pPass->blendSrc = SBF_DEST_COLOUR;
            }
            else if (params[1] == "blend" || params[1] == "gl_blend")
            {
                pPass->blend = LBO_ALPHA_BLEND;
                pPass->blendDest = SBF_ONE_MINUS_SOURCE_ALPHA;
                pPass->blendSrc = SBF_SOURCE_ALPHA;
            }
            else
            {
                // Manual blend
                pPass->blendSrc = convertBlendFunc(params[1]);
                pPass->blendDest = convertBlendFunc(params[2]);
                // Detect common blends
                if (pPass->blendSrc == SBF_ONE && pPass->blendDest == SBF_ZERO)
                    pPass->blend = LBO_REPLACE;
                else if (pPass->blendSrc == SBF_ONE && pPass->blendDest == SBF_ONE)
                    pPass->blend = LBO_ADD;
                else if ((pPass->blendSrc == SBF_ZERO && pPass->blendDest == SBF_SOURCE_COLOUR) ||
                    (pPass->blendSrc == SBF_DEST_COLOUR && pPass->blendDest == SBF_ZERO))
                    pPass->blend = LBO_MODULATE;
                else if (pPass->blendSrc == SBF_SOURCE_ALPHA && pPass->blendDest == SBF_ONE_MINUS_SOURCE_ALPHA)
                    pPass->blend = LBO_ALPHA_BLEND;
                else
                    pPass->customBlend = true;


                // NB other custom blends might not work due to OGRE trying to use multitexture over multipass
            }
        }
        // RGBGEN
        else if (params[0] == "rgbgen")
        {
            // TODO
        }
        // ALPHAGEN
        else if (params[0] == "alphagen")
        {
            // TODO
        }
        // TCGEN
        else if (params[0] == "tcgen")
        {
            if (params[1] == "base")
            {
                pPass->texGen = TEXGEN_BASE;
            }
            else if (params[1] == "lightmap")
            {
                pPass->texGen = TEXGEN_LIGHTMAP;
            }
            else if (params[1] == "environment")
            {
                pPass->texGen = TEXGEN_ENVIRONMENT;
            }
        }
        // TCMOD
        else if (params[0] == "tcmod")
        {
            if (params[1] == "rotate")
            {
                pPass->tcModRotate = -atof(params[2].c_str()) / 360; // +ve is clockwise degrees in Q3 shader, anticlockwise complete rotations in Ogre
            }
            else if (params[1] == "scroll")
            {
                pPass->tcModScroll[0] = atof(params[2].c_str());
                pPass->tcModScroll[1] = atof(params[3].c_str());
            }
            else if (params[1] == "scale")
            {
                pPass->tcModScale[0] = atof(params[2].c_str());
                pPass->tcModScale[1] = atof(params[3].c_str());
            }
            else if (params[1] == "stretch")
            {
                if (params[2] == "sin")
                    pPass->tcModStretchWave = SHADER_FUNC_SIN;
                else if (params[2] == "triangle")
                    pPass->tcModStretchWave = SHADER_FUNC_TRIANGLE;
                else if (params[2] == "square")
                    pPass->tcModStretchWave = SHADER_FUNC_SQUARE;
                else if (params[2] == "sawtooth")
                    pPass->tcModStretchWave = SHADER_FUNC_SAWTOOTH;
                else if (params[2] == "inversesawtooth")
                    pPass->tcModStretchWave = SHADER_FUNC_INVERSESAWTOOTH;

                pPass->tcModStretchParams[0] = atof(params[3].c_str());
                pPass->tcModStretchParams[1] = atof(params[4].c_str());
                pPass->tcModStretchParams[2] = atof(params[5].c_str());
                pPass->tcModStretchParams[3] = atof(params[6].c_str());

            }
        }
        // TURB
        else if (params[0] == "turb")
        {
            pPass->tcModTurbOn = true;
            pPass->tcModTurb[0] = atof(params[2].c_str());
            pPass->tcModTurb[1] = atof(params[3].c_str());
            pPass->tcModTurb[2] = atof(params[4].c_str());
            pPass->tcModTurb[3] = atof(params[5].c_str());
        }
        // DEPTHFUNC
        else if (params[0] == "depthfunc")
        {
            // TODO
        }
        // DEPTHWRITE
        else if (params[0] == "depthwrite")
        {
            // TODO
        }
        // ALPHAFUNC
        else if (params[0] == "alphafunc")
        {
            if (params[1] == "gt0")
            {
                pPass->alphaVal = 0;
                pPass->alphaFunc = CMPF_GREATER;
            }
            else if (params[1] == "ge128")
            {
                pPass->alphaVal = 128;
                pPass->alphaFunc = CMPF_GREATER_EQUAL;
            }
            else if (params[1] == "lt128")
            {
                pPass->alphaVal = 128;
                pPass->alphaFunc = CMPF_LESS;
            }
        }



    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Quake3ShaderManager::convertBlendFunc( const String& q3func)
    {
        if (q3func == "gl_one")
        {
            return SBF_ONE;
        }
        else if (q3func == "gl_zero")
        {
            return SBF_ZERO;
        }
        else if (q3func == "gl_dst_color")
        {
            return SBF_DEST_COLOUR;
        }
        else if (q3func == "gl_src_color")
        {
            return SBF_SOURCE_COLOUR;
        }
        else if (q3func == "gl_one_minus_dst_color")
        {
            return SBF_ONE_MINUS_DEST_COLOUR;
        }
        else if (q3func == "gl_src_alpha")
        {
            return SBF_SOURCE_ALPHA;
        }
        else if (q3func == "gl_one_minus_src_alpha")
        {
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        }

        // Default if unrecognised
        return SBF_ONE;

    }

}
