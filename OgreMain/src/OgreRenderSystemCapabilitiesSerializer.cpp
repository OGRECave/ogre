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
#include "OgreStableHeaders.h"

#include "OgreRenderSystemCapabilitiesSerializer.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreRenderSystemCapabilities.h"

namespace Ogre
{

    RenderSystemCapabilitiesSerializer::RenderSystemCapabilitiesSerializer() : mCurrentLineNumber(0), mCurrentLine(0),
        mCurrentCapabilities(0)
    {
        initialiaseDispatchTables();
    }
    
    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesSerializer::write(const RenderSystemCapabilities* caps, const String &name, std::ostream &file)
    {
        using namespace std;

        file << "render_system_capabilities \"" << name << "\"" << endl;
        file << "{" << endl;

        file << "\t" << "render_system_name " << caps->getRenderSystemName() << endl;
        file << endl;
        

        file << "\t" << "device_name " << caps->getDeviceName() << endl;
        const DriverVersion& driverVer = caps->getDriverVersion();
        file << "\t" << "driver_version " << driverVer.toString() << endl;
        file << "\t" << "vendor " << caps->vendorToString(caps->getVendor());

        file << endl;

        file << endl;

        for(auto & it : mCapabilitiesMap) {
            file << "\t" << it.first << " " << StringConverter::toString(caps->hasCapability(it.second)) << endl;
        }

        file << endl;

        RenderSystemCapabilities::ShaderProfiles profiles = caps->getSupportedShaderProfiles();
        // write every profile
        for(const auto & profile : profiles)
        {
            file << "\t" << "shader_profile " << profile << endl;
        }

        file << endl;
        file << "\t" << "max_point_size " << StringConverter::toString(caps->getMaxPointSize()) << endl;

        file << endl;
        file << "\t" << "non_pow2_textures_limited " << StringConverter::toString(caps->getNonPOW2TexturesLimited()) << endl;
        
        file << endl;
        file << "\t" << "num_texture_units " << StringConverter::toString(caps->getNumTextureUnits()) << endl;
        file << "\t" << "num_multi_render_targets " << StringConverter::toString(caps->getNumMultiRenderTargets()) << endl;
        file << "\t" << "vertex_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_VERTEX_PROGRAM)) << endl;
        file << "\t" << "fragment_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_FRAGMENT_PROGRAM)) << endl;
        file << "\t" << "geometry_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_GEOMETRY_PROGRAM)) << endl;
        file << "\t" << "tessellation_hull_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_HULL_PROGRAM)) << endl;
        file << "\t" << "tessellation_domain_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_DOMAIN_PROGRAM)) << endl;
        file << "\t" << "compute_program_constant_float_count " << StringConverter::toString(caps->getConstantFloatCount(GPT_COMPUTE_PROGRAM)) << endl;
        file << "\t" << "num_vertex_texture_units " << StringConverter::toString(caps->getNumVertexTextureUnits()) << endl;
        file << "\t" << "num_vertex_attributes " << StringConverter::toString(caps->getNumVertexAttributes()) << endl;

        file << endl;

        file << "}" << endl;
    }

    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesSerializer::writeScript(const RenderSystemCapabilities* caps, const String &name, const String& filename)
    {
        using namespace std;

        ofstream file(filename.c_str());

        write(caps, name, file);

        file.close();
    }
    
    //-----------------------------------------------------------------------
    String RenderSystemCapabilitiesSerializer::writeString(const RenderSystemCapabilities* caps, const String &name)
    {
        using namespace std;
        
        stringstream stream;
        
        write(caps, name, stream);
        
        return stream.str();
    }

    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesSerializer::parseScript(DataStreamPtr& stream)
    {
        // reset parsing data to NULL
        mCurrentLineNumber = 0;
        mCurrentLine = 0;
        mCurrentStream.reset();
        mCurrentCapabilities = 0;

        mCurrentStream = stream;

        // parser operating data
        String line;
        ParseAction parseAction = PARSE_HEADER;
        StringVector tokens;
        bool parsedAtLeastOneRSC = false;

        // collect capabilities lines (i.e. everything that is not header, "{", "}",
        // comment or empty line) for further processing
        CapabilitiesLinesList capabilitiesLines;

        // for reading data
        char tmpBuf[OGRE_STREAM_TEMP_SIZE]; 


        // TODO: build a smarter tokenizer so that "{" and "}"
        // don't need separate lines
        while (!stream->eof())
        {
            stream->readLine(tmpBuf, OGRE_STREAM_TEMP_SIZE-1);
            line = String(tmpBuf);
            StringUtil::trim(line);

            // keep track of parse position
            mCurrentLine = &line;
            mCurrentLineNumber++;

            tokens = StringUtil::split(line);

            // skip empty and comment lines
            // TODO: handle end of line comments
            if (tokens[0] == "" || tokens[0].substr(0,2) == "//")
                continue;

            switch (parseAction)
            {
                // header line must look like this:
                // render_system_capabilities "Vendor Card Name Version xx.xxx"

                case PARSE_HEADER:

                    if(tokens[0] != "render_system_capabilities")
                    {
                        logParseError("The first keyword must be render_system_capabilities. RenderSystemCapabilities NOT created!");
                        return;
                    }
                    else
                    {
                        // the rest of the tokens are irrevelant, beause everything between "..." is one name
                        String rscName = line.substr(tokens[0].size());
                        StringUtil::trim(rscName);

                        // the second argument must be a "" delimited string
                        if (!StringUtil::match(rscName, "\"*\""))
                        {
                            logParseError("The argument to render_system_capabilities must be a quote delimited (\"...\") string. RenderSystemCapabilities NOT created!");
                            return;
                        }
                        else
                        {
                            // we have a valid header

                            // remove quotes
                            rscName = rscName.substr(1);
                            rscName = rscName.substr(0, rscName.size() - 1);

                            // create RSC
                            mCurrentCapabilities = OGRE_NEW RenderSystemCapabilities();
                            // RSCManager is responsible for deleting mCurrentCapabilities
                            RenderSystemCapabilitiesManager::getSingleton()._addRenderSystemCapabilities(rscName, mCurrentCapabilities);

                            LogManager::getSingleton().logMessage("Created RenderSystemCapabilities" + rscName);

                            // do next action
                            parseAction = FIND_OPEN_BRACE;
                            parsedAtLeastOneRSC = true;
                        }
                    }

                break;

                case FIND_OPEN_BRACE:
                    if (tokens[0] != "{" || tokens.size() != 1)
                    {
                        logParseError("Expected '{' got: " + line + ". Continuing to next line.");
                    }
                    else
                    {
                        parseAction = COLLECT_LINES;
                    }

                break;

                case COLLECT_LINES:
                    if (tokens[0] == "}")
                    {
                        // this render_system_capabilities section is over
                        // let's process the data and look for the next one
                        parseCapabilitiesLines(capabilitiesLines);
                        capabilitiesLines.clear();
                        parseAction = PARSE_HEADER;

                    }
                    else
                        capabilitiesLines.push_back(CapabilitiesLinesList::value_type(line, mCurrentLineNumber));
                break;

            }
        }

        // Datastream is empty
        // if we are still looking for header, this means that we have either
        // finished reading one, or this is an empty file
        if(parseAction == PARSE_HEADER && parsedAtLeastOneRSC == false)
        {
            logParseError ("The file is empty");
        }
        if(parseAction == FIND_OPEN_BRACE)

        {
            logParseError ("Bad .rendercaps file. Were not able to find a '{'");
        }
        if(parseAction == COLLECT_LINES)
        {
            logParseError ("Bad .rendercaps file. Were not able to find a '}'");
        }

    }

    void RenderSystemCapabilitiesSerializer::initialiaseDispatchTables()
    {
        // set up driver version parsing
        addKeywordType("driver_version", SET_STRING_METHOD);
        // set up the setters for driver versions
        addSetStringMethod("driver_version", &RenderSystemCapabilities::parseDriverVersionFromString);
        
        // set up device name parsing
        addKeywordType("device_name", SET_STRING_METHOD);
        // set up the setters for device names
        addSetStringMethod("device_name", &RenderSystemCapabilities::setDeviceName);
        
        // set up render system name parsing
        addKeywordType("render_system_name", SET_STRING_METHOD);
        // set up the setters 
        addSetStringMethod("render_system_name", &RenderSystemCapabilities::setRenderSystemName);

        // set up vendor parsing
        addKeywordType("vendor", SET_STRING_METHOD);
        // set up the setters for driver versions
        addSetStringMethod("vendor", &RenderSystemCapabilities::parseVendorFromString);

        // initialize int types
        addKeywordType("num_world_matrices", SET_INT_METHOD);
        addKeywordType("num_texture_units", SET_INT_METHOD);
        addKeywordType("stencil_buffer_bit_depth", SET_INT_METHOD);
        addKeywordType("num_vertex_blend_matrices", SET_INT_METHOD);
        addKeywordType("num_multi_render_targets", SET_INT_METHOD);
        addKeywordType("vertex_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("vertex_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("vertex_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("fragment_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("fragment_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("fragment_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("geometry_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("geometry_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("geometry_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("tessellation_hull_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("tessellation_hull_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("tessellation_hull_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("tessellation_domain_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("tessellation_domain_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("tessellation_domain_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("compute_program_constant_float_count", SET_INT_METHOD);
        addKeywordType("compute_program_constant_int_count", SET_INT_METHOD);
        addKeywordType("compute_program_constant_bool_count", SET_INT_METHOD);
        addKeywordType("num_vertex_texture_units", SET_INT_METHOD);

        // initialize int setters
        addSetIntMethod("num_texture_units", &RenderSystemCapabilities::setNumTextureUnits);
        addSetIntMethod("stencil_buffer_bit_depth", &RenderSystemCapabilities::setStencilBufferBitDepth);
        addSetIntMethod("num_multi_render_targets", &RenderSystemCapabilities::setNumMultiRenderTargets);
        addSetIntMethod("vertex_program_constant_float_count", &RenderSystemCapabilities::setVertexProgramConstantFloatCount);
        addSetIntMethod("fragment_program_constant_float_count", &RenderSystemCapabilities::setFragmentProgramConstantFloatCount);
        addSetIntMethod("geometry_program_constant_float_count", &RenderSystemCapabilities::setGeometryProgramConstantFloatCount);
        addSetIntMethod("tessellation_hull_program_constant_float_count", &RenderSystemCapabilities::setTessellationHullProgramConstantFloatCount);
        addSetIntMethod("tessellation_domain_program_constant_float_count", &RenderSystemCapabilities::setTessellationDomainProgramConstantFloatCount);
        addSetIntMethod("compute_program_constant_float_count", &RenderSystemCapabilities::setComputeProgramConstantFloatCount);
        addSetIntMethod("num_vertex_texture_units", &RenderSystemCapabilities::setNumVertexTextureUnits);

        // initialize bool types
        addKeywordType("non_pow2_textures_limited", SET_BOOL_METHOD);

        // initialize bool setters
        addSetBoolMethod("non_pow2_textures_limited", &RenderSystemCapabilities::setNonPOW2TexturesLimited);

        // initialize Real types
        addKeywordType("max_point_size", SET_REAL_METHOD);

        // initialize Real setters
        addSetRealMethod("max_point_size", &RenderSystemCapabilities::setMaxPointSize);

        // there is no dispatch table for shader profiles, just the type
        addKeywordType("shader_profile", ADD_SHADER_PROFILE_STRING);

        addCapabilitiesMapping("fixed_function", RSC_FIXED_FUNCTION);
        addCapabilitiesMapping("anisotropy", RSC_ANISOTROPY);
        addCapabilitiesMapping("hwstencil", RSC_HWSTENCIL);
        addCapabilitiesMapping("32bit_index", RSC_32BIT_INDEX);
        addCapabilitiesMapping("vertex_program", RSC_VERTEX_PROGRAM);
        addCapabilitiesMapping("geometry_program", RSC_GEOMETRY_PROGRAM);
        addCapabilitiesMapping("tessellation_hull_program", RSC_TESSELLATION_HULL_PROGRAM);
        addCapabilitiesMapping("tessellation_domain_program", RSC_TESSELLATION_DOMAIN_PROGRAM);
        addCapabilitiesMapping("compute_program", RSC_COMPUTE_PROGRAM);
        addCapabilitiesMapping("two_sided_stencil", RSC_TWO_SIDED_STENCIL);
        addCapabilitiesMapping("stencil_wrap", RSC_STENCIL_WRAP);
        addCapabilitiesMapping("hwocclusion", RSC_HWOCCLUSION);
        addCapabilitiesMapping("user_clip_planes", RSC_USER_CLIP_PLANES);
        addCapabilitiesMapping("hwrender_to_texture", RSC_HWRENDER_TO_TEXTURE);
        addCapabilitiesMapping("texture_float", RSC_TEXTURE_FLOAT);
        addCapabilitiesMapping("non_power_of_2_textures", RSC_NON_POWER_OF_2_TEXTURES);
        addCapabilitiesMapping("texture_3d", RSC_TEXTURE_3D);
        addCapabilitiesMapping("texture_2d_array", RSC_TEXTURE_3D);
        addCapabilitiesMapping("texture_1d", RSC_TEXTURE_1D);
        addCapabilitiesMapping("point_sprites", RSC_POINT_SPRITES);
        addCapabilitiesMapping("wide_lines", RSC_WIDE_LINES);
        addCapabilitiesMapping("vertex_texture_fetch", RSC_VERTEX_TEXTURE_FETCH);
        addCapabilitiesMapping("mipmap_lod_bias", RSC_MIPMAP_LOD_BIAS);
        addCapabilitiesMapping("atomic_counters", RSC_READ_WRITE_BUFFERS);
        addCapabilitiesMapping("texture_compression", RSC_TEXTURE_COMPRESSION);
        addCapabilitiesMapping("texture_compression_dxt", RSC_TEXTURE_COMPRESSION_DXT);
        addCapabilitiesMapping("texture_compression_vtc", RSC_TEXTURE_COMPRESSION_VTC);
        addCapabilitiesMapping("texture_compression_pvrtc", RSC_TEXTURE_COMPRESSION_PVRTC);
        addCapabilitiesMapping("texture_compression_atc", RSC_TEXTURE_COMPRESSION_ATC);
        addCapabilitiesMapping("texture_compression_etc1", RSC_TEXTURE_COMPRESSION_ETC1);
        addCapabilitiesMapping("texture_compression_etc2", RSC_TEXTURE_COMPRESSION_ETC2);
        addCapabilitiesMapping("texture_compression_bc4_bc5", RSC_TEXTURE_COMPRESSION_BC4_BC5);
        addCapabilitiesMapping("texture_compression_bc6h_bc7", RSC_TEXTURE_COMPRESSION_BC6H_BC7);
        addCapabilitiesMapping("texture_compression_astc", RSC_TEXTURE_COMPRESSION_ASTC);
        addCapabilitiesMapping("hwrender_to_vertex_buffer", RSC_HWRENDER_TO_VERTEX_BUFFER);

        addCapabilitiesMapping("pbuffer", RSC_PBUFFER);
        addCapabilitiesMapping("perstageconstant", RSC_PERSTAGECONSTANT);
        addCapabilitiesMapping("vao", RSC_VAO);
        addCapabilitiesMapping("separate_shader_objects", RSC_SEPARATE_SHADER_OBJECTS);
        addCapabilitiesMapping("glsl_sso_redeclare", RSC_GLSL_SSO_REDECLARE);
        addCapabilitiesMapping("debug", RSC_DEBUG);
        addCapabilitiesMapping("mapbuffer", RSC_MAPBUFFER);
        addCapabilitiesMapping("automipmap_compressed", RSC_AUTOMIPMAP_COMPRESSED);
    }

    void RenderSystemCapabilitiesSerializer::parseCapabilitiesLines(CapabilitiesLinesList& lines)
    {
        StringVector tokens;

        for (auto & line : lines)
        {
            // restore the current line information for debugging
            mCurrentLine = &(line.first);
            mCurrentLineNumber = line.second;

            tokens = StringUtil::split(line.first);
            // check for incomplete lines
            if(tokens.size() < 2)
            {
                logParseError("No parameters given for the capability keyword");
                continue;
            }

            // the first token must the the keyword identifying the capability
            // the remaining tokens are the parameters
            String keyword = tokens[0];
            String everythingElse = "";
            for(unsigned int i = 1; i < tokens.size() - 1; i ++)
            {
                everythingElse = everythingElse + tokens[i] + " ";
            }
            everythingElse = everythingElse + tokens[tokens.size() - 1];

            CapabilityKeywordType keywordType = getKeywordType(keyword);

            switch(keywordType)
            {
                case UNDEFINED_CAPABILITY_TYPE:
                    logParseError("Unknown capability keyword: " + keyword);
                    break;
                case SET_STRING_METHOD:
                    callSetStringMethod(keyword, everythingElse);
                    break;
                case SET_INT_METHOD:
                {
                    ushort integer = (ushort)StringConverter::parseInt(tokens[1]);
                    callSetIntMethod(keyword, integer);
                    break;
                }
                case SET_BOOL_METHOD:
                {
                    bool b = StringConverter::parseBool(tokens[1]);
                    callSetBoolMethod(keyword, b);
                    break;
                }
                case SET_REAL_METHOD:
                {
                    Real real = StringConverter::parseReal(tokens[1]);
                    callSetRealMethod(keyword, real);
                    break;
                }
                case ADD_SHADER_PROFILE_STRING:
                {
                    addShaderProfile(tokens[1]);
                    break;
                }
                case SET_CAPABILITY_ENUM_BOOL:
                {
                    bool b = StringConverter::parseBool(tokens[1]);
                    setCapabilityEnumBool(tokens[0], b);
                    break;
                }
            }
        }
    }

    void RenderSystemCapabilitiesSerializer::logParseError(const String& error) const
    {
        // log the line with error in it if the current line is available
        if (mCurrentLine != 0 && mCurrentStream)
        {
            LogManager::getSingleton().logMessage(
                "Error in .rendercaps " + mCurrentStream->getName() + ":" + StringConverter::toString(mCurrentLineNumber) +
                " : " + error);
        }
        else if (mCurrentStream)
        {
            LogManager::getSingleton().logMessage(
                "Error in .rendercaps " + mCurrentStream->getName() +
                " : " + error);
        }
    }

}


