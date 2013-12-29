/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "OgreMaterialSerializer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"
#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"

namespace Ogre
{

    //-----------------------------------------------------------------------
    // Internal parser methods
    //-----------------------------------------------------------------------
    void logParseError(const String& error, const MaterialScriptContext& context)
    {
        // log material name only if filename not specified
        if (context.filename.empty() && !context.material.isNull())
        {
            LogManager::getSingleton().logMessage(
                "Error in material " + context.material->getName() +
                " : " + error, LML_CRITICAL);
        }
        else
        {
            if (!context.material.isNull())
            {
                LogManager::getSingleton().logMessage(
                    "Error in material " + context.material->getName() +
                    " at line " + StringConverter::toString(context.lineNo) +
                    " of " + context.filename + ": " + error, LML_CRITICAL);
            }
            else
            {
                LogManager::getSingleton().logMessage(
                    "Error at line " + StringConverter::toString(context.lineNo) +
                    " of " + context.filename + ": " + error, LML_CRITICAL);
            }
        }
    }
    //-----------------------------------------------------------------------
    ColourValue _parseColourValue(StringVector& vecparams)
    {
        return ColourValue(
            StringConverter::parseReal(vecparams[0]) ,
            StringConverter::parseReal(vecparams[1]) ,
            StringConverter::parseReal(vecparams[2]) ,
            (vecparams.size()==4) ? StringConverter::parseReal(vecparams[3]) : 1.0f ) ;
    }
    //-----------------------------------------------------------------------
    FilterOptions convertFiltering(const String& s)
    {
        if (s == "none")
        {
            return FO_NONE;
        }
        else if (s == "point")
        {
            return FO_POINT;
        }
        else if (s == "linear")
        {
            return FO_LINEAR;
        }
        else if (s == "anisotropic")
        {
            return FO_ANISOTROPIC;
        }

        return FO_POINT;
    }
    //-----------------------------------------------------------------------
    bool parseAmbient(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_AMBIENT);
            } else {
                logParseError(
                    "Bad ambient attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setAmbient( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_AMBIENT);
        }
        else
        {
            logParseError(
                "Bad ambient attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }
        return false;
    }
   //-----------------------------------------------------------------------
    bool parseDiffuse(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_DIFFUSE);
            } else {
                logParseError(
                    "Bad diffuse attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setDiffuse( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_DIFFUSE);
        }
        else
        {
            logParseError(
                "Bad diffuse attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }        return false;
    }
    //-----------------------------------------------------------------------
    bool parseSpecular(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 2, 4 or 5 parameters
        if(vecparams.size() == 2)
        {
            if(vecparams[0] == "vertexcolour") {
                context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_SPECULAR);
                context.pass->setShininess(StringConverter::parseReal(vecparams[1]) );
            }
            else
            {
                logParseError(
                    "Bad specular attribute, double parameter statement must be 'vertexcolour <shininess>'",
                    context);
            }
        }
        else if(vecparams.size() == 4 || vecparams.size() == 5)
        {
            context.pass->setSpecular(
                StringConverter::parseReal(vecparams[0]),
                StringConverter::parseReal(vecparams[1]),
                StringConverter::parseReal(vecparams[2]),
                vecparams.size() == 5?
                    StringConverter::parseReal(vecparams[3]) : 1.0f);
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_SPECULAR);
            context.pass->setShininess(
                StringConverter::parseReal(vecparams[vecparams.size() - 1]) );
        }
        else
        {
            logParseError(
                "Bad specular attribute, wrong number of parameters (expected 2, 4 or 5)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEmissive(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_EMISSIVE);
            } else {
                logParseError(
                    "Bad emissive attribute, single parameter flag must be 'vertexcolour'",
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setSelfIllumination( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_EMISSIVE);
        }
        else
        {
            logParseError(
                "Bad emissive attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor convertBlendFactor(const String& param)
    {
        if (param == "one")
            return SBF_ONE;
        else if (param == "zero")
            return SBF_ZERO;
        else if (param == "dest_colour")
            return SBF_DEST_COLOUR;
        else if (param == "src_colour")
            return SBF_SOURCE_COLOUR;
        else if (param == "one_minus_dest_colour")
            return SBF_ONE_MINUS_DEST_COLOUR;
        else if (param == "one_minus_src_colour")
            return SBF_ONE_MINUS_SOURCE_COLOUR;
        else if (param == "dest_alpha")
            return SBF_DEST_ALPHA;
        else if (param == "src_alpha")
            return SBF_SOURCE_ALPHA;
        else if (param == "one_minus_dest_alpha")
            return SBF_ONE_MINUS_DEST_ALPHA;
        else if (param == "one_minus_src_alpha")
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend factor.", "convertBlendFactor");
        }


    }
    //-----------------------------------------------------------------------
    bool parseSceneBlend(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Should be 1 or 2 params
        if (vecparams.size() == 1)
        {
            //simple
            SceneBlendType stype;
            if (vecparams[0] == "add")
                stype = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stype = SBT_MODULATE;
			else if (vecparams[0] == "colour_blend")
				stype = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stype = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad scene_blend attribute, unrecognised parameter '" + vecparams[0] + "'",
                    context);
                return false;
            }
            context.pass->setSceneBlending(stype);

        }
        else if (vecparams.size() == 2)
        {
            //src/dest
            SceneBlendFactor src, dest;

            try {
                src = convertBlendFactor(vecparams[0]);
                dest = convertBlendFactor(vecparams[1]);
                context.pass->setSceneBlending(src,dest);
            }
            catch (Exception& e)
            {
                logParseError("Bad scene_blend attribute, " + e.getDescription(), context);
            }

        }
        else
        {
            logParseError(
                "Bad scene_blend attribute, wrong number of parameters (expected 1 or 2)",
                context);
        }

        return false;

    }
    //-----------------------------------------------------------------------
    bool parseSeparateSceneBlend(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Should be 2 or 4 params
        if (vecparams.size() == 2)
        {
            //simple
            SceneBlendType stype;
            if (vecparams[0] == "add")
                stype = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stype = SBT_MODULATE;
			else if (vecparams[0] == "colour_blend")
				stype = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stype = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad separate_scene_blend attribute, unrecognised parameter '" + vecparams[0] + "'",
                    context);
                return false;
            }

            SceneBlendType stypea;
            if (vecparams[0] == "add")
                stypea = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stypea = SBT_MODULATE;
			else if (vecparams[0] == "colour_blend")
				stypea = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stypea = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad separate_scene_blend attribute, unrecognised parameter '" + vecparams[1] + "'",
                    context);
                return false;
            }
			
			context.pass->setSeparateSceneBlending(stype, stypea);
        }
        else if (vecparams.size() == 4)
        {
            //src/dest
            SceneBlendFactor src, dest;
			SceneBlendFactor srca, desta;

            try {
                src = convertBlendFactor(vecparams[0]);
                dest = convertBlendFactor(vecparams[1]);
                srca = convertBlendFactor(vecparams[2]);
                desta = convertBlendFactor(vecparams[3]);
				context.pass->setSeparateSceneBlending(src,dest,srca,desta);
            }
            catch (Exception& e)
            {
                logParseError("Bad separate_scene_blend attribute, " + e.getDescription(), context);
            }

        }
        else
        {
            logParseError(
                "Bad separate_scene_blend attribute, wrong number of parameters (expected 2 or 4)",
                context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    CompareFunction convertCompareFunction(const String& param)
    {
        if (param == "always_fail")
            return CMPF_ALWAYS_FAIL;
        else if (param == "always_pass")
            return CMPF_ALWAYS_PASS;
        else if (param == "less")
            return CMPF_LESS;
        else if (param == "less_equal")
            return CMPF_LESS_EQUAL;
        else if (param == "equal")
            return CMPF_EQUAL;
        else if (param == "not_equal")
            return CMPF_NOT_EQUAL;
        else if (param == "greater_equal")
            return CMPF_GREATER_EQUAL;
        else if (param == "greater")
            return CMPF_GREATER;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare function", "convertCompareFunction");

    }
    //-----------------------------------------------------------------------
    bool parseDepthCheck(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthCheckEnabled(true);
        else if (params == "off")
            context.pass->setDepthCheckEnabled(false);
        else
            logParseError(
            "Bad depth_check attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseDepthWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthWriteEnabled(true);
        else if (params == "off")
            context.pass->setDepthWriteEnabled(false);
        else
            logParseError(
                "Bad depth_write attribute, valid parameters are 'on' or 'off'.",
                context);
        return false;
    }
	//-----------------------------------------------------------------------
	bool parseLightScissor(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		if (params == "on")
			context.pass->setLightScissoringEnabled(true);
		else if (params == "off")
			context.pass->setLightScissoringEnabled(false);
		else
			logParseError(
			"Bad light_scissor attribute, valid parameters are 'on' or 'off'.",
			context);
		return false;
	}
	//-----------------------------------------------------------------------
	bool parseLightClip(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		if (params == "on")
			context.pass->setLightClipPlanesEnabled(true);
		else if (params == "off")
			context.pass->setLightClipPlanesEnabled(false);
		else
			logParseError(
			"Bad light_clip_planes attribute, valid parameters are 'on' or 'off'.",
			context);
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseDepthFunc(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try {
            CompareFunction func = convertCompareFunction(params);
            context.pass->setDepthFunction(func);
        }
        catch (...)
        {
            logParseError("Bad depth_func attribute, invalid function parameter.", context);
        }

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseNormaliseNormals(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		if (params == "on")
			context.pass->setNormaliseNormals(true);
		else if (params == "off")
			context.pass->setNormaliseNormals(false);
		else
			logParseError(
			"Bad normalise_normals attribute, valid parameters are 'on' or 'off'.",
			context);
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseColourWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setColourWriteEnabled(true);
        else if (params == "off")
            context.pass->setColourWriteEnabled(false);
        else
            logParseError(
                "Bad colour_write attribute, valid parameters are 'on' or 'off'.",
                context);
        return false;
    }

    //-----------------------------------------------------------------------
    bool parseCullHardware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setCullingMode(CULL_NONE);
        else if (params=="anticlockwise")
            context.pass->setCullingMode(CULL_ANTICLOCKWISE);
        else if (params=="clockwise")
            context.pass->setCullingMode(CULL_CLOCKWISE);
        else
            logParseError(
                "Bad cull_hardware attribute, valid parameters are "
                "'none', 'clockwise' or 'anticlockwise'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseCullSoftware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setManualCullingMode(MANUAL_CULL_NONE);
        else if (params=="back")
            context.pass->setManualCullingMode(MANUAL_CULL_BACK);
        else if (params=="front")
            context.pass->setManualCullingMode(MANUAL_CULL_FRONT);
        else
            logParseError(
                "Bad cull_software attribute, valid parameters are 'none', "
                "'front' or 'back'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLighting(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="on")
            context.pass->setLightingEnabled(true);
        else if (params=="off")
            context.pass->setLightingEnabled(false);
        else
            logParseError(
                "Bad lighting attribute, valid parameters are 'on' or 'off'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMaxLights(String& params, MaterialScriptContext& context)
    {
		context.pass->setMaxSimultaneousLights((ushort)StringConverter::parseInt(params));
        return false;
    }
	//-----------------------------------------------------------------------
	bool parseStartLight(String& params, MaterialScriptContext& context)
	{
		context.pass->setStartLight((ushort)StringConverter::parseInt(params));
		return false;
	}
    //-----------------------------------------------------------------------
    void parseIterationLightTypes(String& params, MaterialScriptContext& context)
    {
        // Parse light type
        if (params == "directional")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
        }
        else if (params == "point")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_POINT);
        }
        else if (params == "spot")
        {
            context.pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
        }
        else
        {
            logParseError("Bad iteration attribute, valid values for light type parameter "
                "are 'point' or 'directional' or 'spot'.", context);
        }

    }
    //-----------------------------------------------------------------------
    bool parseIteration(String& params, MaterialScriptContext& context)
    {
        // we could have more than one parameter
        /** combinations could be:
            iteration once
            iteration once_per_light [light type]
            iteration <number>
            iteration <number> [per_light] [light type]
			iteration <number> [per_n_lights] <num_lights> [light type]
        */
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 1 || vecparams.size() > 4)
        {
            logParseError("Bad iteration attribute, expected 1 to 3 parameters.", context);
            return false;
        }

        if (vecparams[0]=="once")
            context.pass->setIteratePerLight(false, false);
        else if (vecparams[0]=="once_per_light")
        {
            if (vecparams.size() == 2)
            {
                parseIterationLightTypes(vecparams[1], context);
            }
            else
            {
                context.pass->setIteratePerLight(true, false);
            }

        }
        else // could be using form: <number> [per_light] [light type]
        {
            int passIterationCount = StringConverter::parseInt(vecparams[0]);
            if (passIterationCount > 0)
            {
                context.pass->setPassIterationCount(passIterationCount);
                if (vecparams.size() > 1)
                {
                    if (vecparams[1] == "per_light")
                    {
                        if (vecparams.size() == 3)
                        {
                            parseIterationLightTypes(vecparams[2], context);
                        }
                        else
                        {
                            context.pass->setIteratePerLight(true, false);
                        }
                    }
					else if (vecparams[1] == "per_n_lights")
					{
						if (vecparams.size() < 3)
						{
							logParseError(
								"Bad iteration attribute, expected number of lights.", 
								context);
						}
						else
						{
							// Parse num lights
							context.pass->setLightCountPerIteration(
								(ushort)StringConverter::parseInt(vecparams[2]));
							// Light type
							if (vecparams.size() == 4)
							{
								parseIterationLightTypes(vecparams[3], context);
							}
							else
							{
								context.pass->setIteratePerLight(true, false);
							}
						}
					}
                    else
                        logParseError(
                            "Bad iteration attribute, valid parameters are <number> [per_light|per_n_lights <num_lights>] [light type].", context);
                }
            }
            else
                logParseError(
                    "Bad iteration attribute, valid parameters are 'once' or 'once_per_light' or <number> [per_light|per_n_lights <num_lights>] [light type].", context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSize(String& params, MaterialScriptContext& context)
    {
        context.pass->setPointSize(StringConverter::parseReal(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parsePointSprites(String& params, MaterialScriptContext& context)
    {
        if (params=="on")
	        context.pass->setPointSpritesEnabled(true);
		else if (params=="off")
	        context.pass->setPointSpritesEnabled(false);
		else
            logParseError(
                "Bad point_sprites attribute, valid parameters are 'on' or 'off'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
	bool parsePointAttenuation(String& params, MaterialScriptContext& context)
	{
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 1 && vecparams.size() != 4)
		{
			logParseError("Bad point_size_attenuation attribute, 1 or 4 parameters expected", context);
			return false;
		}
		if (vecparams[0] == "off")
		{
			context.pass->setPointAttenuation(false);
		}
		else if (vecparams[0] == "on")
		{
			if (vecparams.size() == 4)
			{
				context.pass->setPointAttenuation(true,
					StringConverter::parseReal(vecparams[1]),
					StringConverter::parseReal(vecparams[2]),
					StringConverter::parseReal(vecparams[3]));
			}
			else
			{
				context.pass->setPointAttenuation(true);
			}
		}

		return false;
	}
    //-----------------------------------------------------------------------
	bool parsePointSizeMin(String& params, MaterialScriptContext& context)
	{
		context.pass->setPointMinSize(
			StringConverter::parseReal(params));
		return false;
	}
    //-----------------------------------------------------------------------
	bool parsePointSizeMax(String& params, MaterialScriptContext& context)
	{
		context.pass->setPointMaxSize(
			StringConverter::parseReal(params));
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseFogging(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams[0]=="true")
        {
            // if true, we need to see if they supplied all arguments, or just the 1... if just the one,
            // Assume they want to disable the default fog from effecting this material.
            if( vecparams.size() == 8 )
            {
                FogMode mFogtype;
                if( vecparams[1] == "none" )
                    mFogtype = FOG_NONE;
                else if( vecparams[1] == "linear" )
                    mFogtype = FOG_LINEAR;
                else if( vecparams[1] == "exp" )
                    mFogtype = FOG_EXP;
                else if( vecparams[1] == "exp2" )
                    mFogtype = FOG_EXP2;
                else
                {
                    logParseError(
                        "Bad fogging attribute, valid parameters are "
                        "'none', 'linear', 'exp', or 'exp2'.", context);
                    return false;
                }

                context.pass->setFog(
                    true,
                    mFogtype,
                    ColourValue(
                    StringConverter::parseReal(vecparams[2]),
                    StringConverter::parseReal(vecparams[3]),
                    StringConverter::parseReal(vecparams[4])),
                    StringConverter::parseReal(vecparams[5]),
                    StringConverter::parseReal(vecparams[6]),
                    StringConverter::parseReal(vecparams[7])
                    );
            }
            else
            {
                context.pass->setFog(true);
            }
        }
        else if (vecparams[0]=="false")
            context.pass->setFog(false);
        else
            logParseError(
                "Bad fog_override attribute, valid parameters are 'true' or 'false'.",
                context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseShading(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="flat")
            context.pass->setShadingMode(SO_FLAT);
        else if (params=="gouraud")
            context.pass->setShadingMode(SO_GOURAUD);
        else if (params=="phong")
            context.pass->setShadingMode(SO_PHONG);
        else
            logParseError("Bad shading attribute, valid parameters are 'flat', "
                "'gouraud' or 'phong'.", context);

        return false;
    }
	//-----------------------------------------------------------------------
	bool parsePolygonMode(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		if (params=="solid")
			context.pass->setPolygonMode(PM_SOLID);
		else if (params=="wireframe")
			context.pass->setPolygonMode(PM_WIREFRAME);
		else if (params=="points")
			context.pass->setPolygonMode(PM_POINTS);
		else
			logParseError("Bad polygon_mode attribute, valid parameters are 'solid', "
			"'wireframe' or 'points'.", context);

		return false;
	}
	//-----------------------------------------------------------------------
	bool parsePolygonModeOverrideable(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);

		context.pass->setPolygonModeOverrideable(
			StringConverter::parseBool(params));
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseFiltering(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1 or 3 parameters
        if (vecparams.size() == 1)
        {
            // Simple format
            if (vecparams[0]=="none")
                context.textureUnit->setTextureFiltering(TFO_NONE);
            else if (vecparams[0]=="bilinear")
                context.textureUnit->setTextureFiltering(TFO_BILINEAR);
            else if (vecparams[0]=="trilinear")
                context.textureUnit->setTextureFiltering(TFO_TRILINEAR);
            else if (vecparams[0]=="anisotropic")
                context.textureUnit->setTextureFiltering(TFO_ANISOTROPIC);
            else
            {
                logParseError("Bad filtering attribute, valid parameters for simple format are "
                    "'none', 'bilinear', 'trilinear' or 'anisotropic'.", context);
                return false;
            }
        }
        else if (vecparams.size() == 3)
        {
            // Complex format
            context.textureUnit->setTextureFiltering(
                convertFiltering(vecparams[0]),
                convertFiltering(vecparams[1]),
                convertFiltering(vecparams[2]));


        }
        else
        {
            logParseError(
                "Bad filtering attribute, wrong number of parameters (expected 1, 3 or 4)",
                context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
	bool parseCompareTest(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try 
		{
			if(params == "on")
			{
				context.textureUnit->setTextureCompareEnabled(true);
			}
			else if(params == "off")
			{
				context.textureUnit->setTextureCompareEnabled(false);
			}
			else
			{
				  OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare setting", "parseCompareEnabled");
			}
        }
        catch (...)
        {
            logParseError("Bad compare_test attribute, invalid function parameter.", context);
        }

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseCompareFunction(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		try {
			CompareFunction func = convertCompareFunction(params);
			context.textureUnit->setTextureCompareFunction(func);
		}
		catch (...)
		{
			logParseError("Bad compare_func attribute, invalid function parameter.", context);
		}

		return false;
	}
	//-----------------------------------------------------------------------
    // Texture layer attributes
    bool parseTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        const size_t numParams = vecparams.size();
        if (numParams > 5)
        {
            logParseError("Invalid texture attribute - expected only up to 5 parameters.",
                context);
        }
        TextureType tt = TEX_TYPE_2D;
		int mipmaps = MIP_DEFAULT; // When passed to TextureManager::load, this means default to default number of mipmaps
        bool isAlpha = false;
		bool hwGamma = false;
        PixelFormat desiredFormat = PF_UNKNOWN;
		for (size_t p = 1; p < numParams; ++p)
		{
            StringUtil::toLowerCase(vecparams[p]);
            if (vecparams[p] == "1d")
            {
                tt = TEX_TYPE_1D;
            }
            else if (vecparams[p] == "2d")
            {
                tt = TEX_TYPE_2D;
            }
            else if (vecparams[p] == "3d")
            {
                tt = TEX_TYPE_3D;
            }
            else if (vecparams[p] == "cubic")
            {
                tt = TEX_TYPE_CUBE_MAP;
            }
			else if (vecparams[p] == "unlimited")
			{
				mipmaps = MIP_UNLIMITED;
			}
			else if (StringConverter::isNumber(vecparams[p]))
			{
				mipmaps = StringConverter::parseInt(vecparams[p]);
			}
			else if (vecparams[p] == "alpha")
			{
				isAlpha = true;
			}
			else if (vecparams[p] == "gamma")
			{
				hwGamma = true;
			}
            else if ((desiredFormat = PixelUtil::getFormatFromName(vecparams[p], true)) != PF_UNKNOWN)
            {
                // nothing to do here
            }
			else
			{
				logParseError("Invalid texture option - "+vecparams[p]+".",
                context);
			}
        }

		context.textureUnit->setTextureName(vecparams[0], tt);
        context.textureUnit->setNumMipmaps(mipmaps);
        context.textureUnit->setIsAlpha(isAlpha);
        context.textureUnit->setDesiredFormat(desiredFormat);
		context.textureUnit->setHardwareGammaEnabled(hwGamma);
        return false;
    }
	//---------------------------------------------------------------------
	bool parseBindingType(String& params, MaterialScriptContext& context)
	{
		if (params == "fragment")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_FRAGMENT);
		}
		else if (params == "vertex")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_VERTEX);
		}
		else if (params == "geometry")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_GEOMETRY);
		}
		else if (params == "tessellation_hull")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_TESSELLATION_HULL);
		}
		else if (params == "tessellation_domain")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_TESSELLATION_DOMAIN);
		}
		else if (params == "compute")
		{
			context.textureUnit->setBindingType(TextureUnitState::BT_COMPUTE);
		}
		else
		{
			logParseError("Invalid binding_type option - "+params+".",
				context);
		}
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseAnimTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();
        // Determine which form it is
        // Must have at least 3 params though
        if (numParams < 3)
        {
            logParseError("Bad anim_texture attribute, wrong number of parameters "
                "(expected at least 3)", context);
            return false;
        }
        if (numParams == 3 && StringConverter::parseInt(vecparams[1]) != 0 )
        {
            // First form using base name & number of frames
            context.textureUnit->setAnimatedTextureName(
                vecparams[0],
                StringConverter::parseInt(vecparams[1]),
                StringConverter::parseReal(vecparams[2]));
        }
        else
        {
            // Second form using individual names
            context.textureUnit->setAnimatedTextureName(
                (String*)&vecparams[0],
                static_cast<unsigned int>(numParams-1),
                StringConverter::parseReal(vecparams[numParams-1]));
        }
        return false;

    }
    //-----------------------------------------------------------------------
    bool parseCubicTexture(String& params, MaterialScriptContext& context)
    {

        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        // Get final param
        bool useUVW;
        String& uvOpt = vecparams[numParams-1];
		StringUtil::toLowerCase(uvOpt);
        if (uvOpt == "combineduvw")
            useUVW = true;
        else if (uvOpt == "separateuv")
            useUVW = false;
        else
        {
            logParseError("Bad cubic_texture attribute, final parameter must be "
                "'combinedUVW' or 'separateUV'.", context);
            return false;
        }
        // Determine which form it is
        if (numParams == 2)
        {
            // First form using base name
            context.textureUnit->setCubicTextureName(vecparams[0], useUVW);
        }
        else if (numParams == 7)
        {
            // Second form using individual names
            // Can use vecparams[0] as array start point
            context.textureUnit->setCubicTextureName((String*)&vecparams[0], useUVW);
        }
        else
        {
            logParseError(
                "Bad cubic_texture attribute, wrong number of parameters (expected 2 or 7)",
                context);
            return false;
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexCoord(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureCoordSet(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
	TextureUnitState::TextureAddressingMode convTexAddressMode(const String& params, MaterialScriptContext& context)
	{
		if (params=="wrap")
			return TextureUnitState::TAM_WRAP;
		else if (params=="mirror")
			return TextureUnitState::TAM_MIRROR;
		else if (params=="clamp")
			return TextureUnitState::TAM_CLAMP;
		else if (params=="border")
			return TextureUnitState::TAM_BORDER;
		else
			logParseError("Bad tex_address_mode attribute, valid parameters are "
				"'wrap', 'mirror', 'clamp' or 'border'.", context);
		// default
		return TextureUnitState::TAM_WRAP;
	}
    //-----------------------------------------------------------------------
    bool parseTexAddressMode(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);

        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

		if (numParams > 3 || numParams < 1)
		{
			logParseError("Invalid number of parameters to tex_address_mode"
					" - must be between 1 and 3", context);
		}
		if (numParams == 1)
		{
			// Single-parameter option
			context.textureUnit->setTextureAddressingMode(
				convTexAddressMode(vecparams[0], context));
		}
		else
		{
			// 2-3 parameter option
			TextureUnitState::UVWAddressingMode uvw;
			uvw.u = convTexAddressMode(vecparams[0], context);
			uvw.v = convTexAddressMode(vecparams[1], context);
			if (numParams == 3)
			{
				// w
				uvw.w = convTexAddressMode(vecparams[2], context);
			}
			else
			{
				uvw.w = TextureUnitState::TAM_WRAP;
			}
			context.textureUnit->setTextureAddressingMode(uvw);
		}
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexBorderColour(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 3 or 4 parameters
        if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.textureUnit->setTextureBorderColour( _parseColourValue(vecparams) );
        }
        else
        {
            logParseError(
                "Bad tex_border_colour attribute, wrong number of parameters (expected 3 or 4)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourOp(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="replace")
            context.textureUnit->setColourOperation(LBO_REPLACE);
        else if (params=="add")
            context.textureUnit->setColourOperation(LBO_ADD);
        else if (params=="modulate")
            context.textureUnit->setColourOperation(LBO_MODULATE);
        else if (params=="alpha_blend")
            context.textureUnit->setColourOperation(LBO_ALPHA_BLEND);
        else
            logParseError("Bad colour_op attribute, valid parameters are "
                "'replace', 'add', 'modulate' or 'alpha_blend'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAlphaRejection(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError(
                "Bad alpha_rejection attribute, wrong number of parameters (expected 2)",
                context);
            return false;
        }

        CompareFunction cmp;
        try {
            cmp = convertCompareFunction(vecparams[0]);
        }
        catch (...)
        {
            logParseError("Bad alpha_rejection attribute, invalid compare function.", context);
            return false;
        }

        context.pass->setAlphaRejectSettings(cmp, (unsigned char)StringConverter::parseInt(vecparams[1]));

        return false;
    }
	//---------------------------------------------------------------------
	bool parseAlphaToCoverage(String& params, MaterialScriptContext& context)
	{
		StringUtil::toLowerCase(params);
		if (params == "on")
			context.pass->setAlphaToCoverageEnabled(true);
		else if (params == "off")
			context.pass->setAlphaToCoverageEnabled(false);
		else
			logParseError(
			"Bad alpha_to_coverage attribute, valid parameters are 'on' or 'off'.",
			context);

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseTransparentSorting(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setTransparentSortingEnabled(true);
        else if (params == "off")
            context.pass->setTransparentSortingEnabled(false);
        else if (params == "force")
            context.pass->setTransparentSortingForced(true);
        else
            logParseError(
            "Bad transparent_sorting attribute, valid parameters are 'on', 'off' or 'force'.",
            context);

        return false;
    }
    //-----------------------------------------------------------------------
    LayerBlendOperationEx convertBlendOpEx(const String& param)
    {
        if (param == "source1")
            return LBX_SOURCE1;
        else if (param == "source2")
            return LBX_SOURCE2;
        else if (param == "modulate")
            return LBX_MODULATE;
        else if (param == "modulate_x2")
            return LBX_MODULATE_X2;
        else if (param == "modulate_x4")
            return LBX_MODULATE_X4;
        else if (param == "add")
            return LBX_ADD;
        else if (param == "add_signed")
            return LBX_ADD_SIGNED;
        else if (param == "add_smooth")
            return LBX_ADD_SMOOTH;
        else if (param == "subtract")
            return LBX_SUBTRACT;
        else if (param == "blend_diffuse_colour")
            return LBX_BLEND_DIFFUSE_COLOUR;
        else if (param == "blend_diffuse_alpha")
            return LBX_BLEND_DIFFUSE_ALPHA;
        else if (param == "blend_texture_alpha")
            return LBX_BLEND_TEXTURE_ALPHA;
        else if (param == "blend_current_alpha")
            return LBX_BLEND_CURRENT_ALPHA;
        else if (param == "blend_manual")
            return LBX_BLEND_MANUAL;
        else if (param == "dotproduct")
            return LBX_DOTPRODUCT;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend function", "convertBlendOpEx");
    }
    //-----------------------------------------------------------------------
    LayerBlendSource convertBlendSource(const String& param)
    {
        if (param == "src_current")
            return LBS_CURRENT;
        else if (param == "src_texture")
            return LBS_TEXTURE;
        else if (param == "src_diffuse")
            return LBS_DIFFUSE;
        else if (param == "src_specular")
            return LBS_SPECULAR;
        else if (param == "src_manual")
            return LBS_MANUAL;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend source", "convertBlendSource");
    }
    //-----------------------------------------------------------------------
    bool parseColourOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        if (numParams < 3 || numParams > 10)
        {
            logParseError(
                "Bad colour_op_ex attribute, wrong number of parameters (expected 3 to 10)",
                context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        ColourValue colSrc1 = ColourValue::White;
        ColourValue colSrc2 = ColourValue::White;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);

            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams < 4)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }

            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc1.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc1.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc1.a = 1.0f;
                }
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex += 3;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc2.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc2.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc2.a = 1.0f;
                }
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_ex attribute, " + e.getDescription(), context);
            return false;
        }

        context.textureUnit->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourOpFallback(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, wrong number "
                "of parameters (expected 2)", context);
            return false;
        }

        //src/dest
        SceneBlendFactor src, dest;

        try {
            src = convertBlendFactor(vecparams[0]);
            dest = convertBlendFactor(vecparams[1]);
            context.textureUnit->setColourOpMultipassFallback(src,dest);
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, "
                + e.getDescription(), context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAlphaOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();
        if (numParams < 3 || numParams > 6)
        {
            logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                "(expected 3 to 6)", context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        Real arg1 = 1.0, arg2 = 1.0;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);
            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams != 4)
                {
                    logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }
            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters (expected " +
                        StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg1 = StringConverter::parseReal(vecparams[parIndex]);
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg2 = StringConverter::parseReal(vecparams[parIndex]);
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad alpha_op_ex attribute, " + e.getDescription(), context);
            return false;
        }

        context.textureUnit->setAlphaOperation(op, src1, src2, arg1, arg2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEnvMap(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="off")
            context.textureUnit->setEnvironmentMap(false);
        else if (params=="spherical")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
        else if (params=="planar")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
        else if (params=="cubic_reflection")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
        else if (params=="cubic_normal")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
        else
            logParseError("Bad env_map attribute, valid parameters are 'off', "
                "'spherical', 'planar', 'cubic_reflection' and 'cubic_normal'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScroll(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScroll(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));


        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScrollAnim(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll_anim attribute, wrong number of "
                "parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setScrollAnimation(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotate(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureRotate(
            StringConverter::parseAngle(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotateAnim(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setRotateAnimation(
            StringConverter::parseReal(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScale(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scale attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScale(
            StringConverter::parseReal(vecparams[0]),
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseWaveXform(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");

        if (vecparams.size() != 6)
        {
            logParseError("Bad wave_xform attribute, wrong number of parameters "
                "(expected 6)", context);
            return false;
        }
        TextureUnitState::TextureTransformType ttype;
        WaveformType waveType;
        // Check transform type
        if (vecparams[0]=="scroll_x")
            ttype = TextureUnitState::TT_TRANSLATE_U;
        else if (vecparams[0]=="scroll_y")
            ttype = TextureUnitState::TT_TRANSLATE_V;
        else if (vecparams[0]=="rotate")
            ttype = TextureUnitState::TT_ROTATE;
        else if (vecparams[0]=="scale_x")
            ttype = TextureUnitState::TT_SCALE_U;
        else if (vecparams[0]=="scale_y")
            ttype = TextureUnitState::TT_SCALE_V;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 1 must be 'scroll_x', "
                "'scroll_y', 'rotate', 'scale_x' or 'scale_y'", context);
            return false;
        }
        // Check wave type
        if (vecparams[1]=="sine")
            waveType = WFT_SINE;
        else if (vecparams[1]=="triangle")
            waveType = WFT_TRIANGLE;
        else if (vecparams[1]=="square")
            waveType = WFT_SQUARE;
        else if (vecparams[1]=="sawtooth")
            waveType = WFT_SAWTOOTH;
        else if (vecparams[1]=="inverse_sawtooth")
            waveType = WFT_INVERSE_SAWTOOTH;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 2 must be 'sine', "
                "'triangle', 'square', 'sawtooth' or 'inverse_sawtooth'", context);
            return false;
        }

        context.textureUnit->setTransformAnimation(
            ttype,
            waveType,
            StringConverter::parseReal(vecparams[2]),
            StringConverter::parseReal(vecparams[3]),
            StringConverter::parseReal(vecparams[4]),
            StringConverter::parseReal(vecparams[5]) );

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseTransform(String& params, MaterialScriptContext& context)
	{
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 16)
		{
			logParseError("Bad transform attribute, wrong number of parameters (expected 16)", context);
			return false;
		}
		Matrix4 xform(
			StringConverter::parseReal(vecparams[0]),
			StringConverter::parseReal(vecparams[1]),
			StringConverter::parseReal(vecparams[2]),
			StringConverter::parseReal(vecparams[3]),
			StringConverter::parseReal(vecparams[4]),
			StringConverter::parseReal(vecparams[5]),
			StringConverter::parseReal(vecparams[6]),
			StringConverter::parseReal(vecparams[7]),
			StringConverter::parseReal(vecparams[8]),
			StringConverter::parseReal(vecparams[9]),
			StringConverter::parseReal(vecparams[10]),
			StringConverter::parseReal(vecparams[11]),
			StringConverter::parseReal(vecparams[12]),
			StringConverter::parseReal(vecparams[13]),
			StringConverter::parseReal(vecparams[14]),
			StringConverter::parseReal(vecparams[15]) );
		context.textureUnit->setTextureTransform(xform);


		return false;
	}
    //-----------------------------------------------------------------------
    bool parseDepthBias(String& params, MaterialScriptContext& context)
    {
		StringVector vecparams = StringUtil::split(params, " \t");

		float constantBias = static_cast<float>(StringConverter::parseReal(vecparams[0]));
		float slopeScaleBias = 0.0f;
		if (vecparams.size() > 1)
		{
			slopeScaleBias = static_cast<float>(StringConverter::parseReal(vecparams[1]));
		}
        context.pass->setDepthBias(constantBias, slopeScaleBias);

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseIterationDepthBias(String& params, MaterialScriptContext& context)
	{
		StringVector vecparams = StringUtil::split(params, " \t");

		float bias = static_cast<float>(StringConverter::parseReal(vecparams[0]));
		context.pass->setIterationDepthBias(bias);

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseAnisotropy(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureAnisotropy(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTextureAlias(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureNameAlias(params);

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseMipmapBias(String& params, MaterialScriptContext& context)
	{
		context.textureUnit->setTextureMipmapBias(
			(float)StringConverter::parseReal(params));

		return false;
	}
	//-----------------------------------------------------------------------
	bool parseContentType(String& params, MaterialScriptContext& context)
	{
		StringVector vecparams = StringUtil::tokenise(params, " \t");
		if (vecparams.empty())
		{
			logParseError("No content_type specified", context);
			return false;
		}
		String& paramType = vecparams[0];
		if (paramType == "named")
		{
			context.textureUnit->setContentType(TextureUnitState::CONTENT_NAMED);
		}
		else if (paramType == "shadow")
		{
			context.textureUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
		}
		else if (paramType == "compositor")
		{
			context.textureUnit->setContentType(TextureUnitState::CONTENT_COMPOSITOR);
			if (vecparams.size() == 3)
			{
				context.textureUnit->setCompositorReference(vecparams[1], vecparams[2]);
			}
			else if (vecparams.size() == 4)
			{
				context.textureUnit->setCompositorReference(vecparams[1], vecparams[2], 
					StringConverter::parseUnsignedInt(vecparams[3]));
			}
			else
			{
				logParseError("compositor content_type requires 2 or 3 extra params", context);
			}
		}
		else
		{
			logParseError("Invalid content_type specified : " + paramType, context);
		}
		return false;
	}
	//-----------------------------------------------------------------------
	bool parseIlluminationStage(String& params, MaterialScriptContext& context)
	{
		if (params == "ambient")
		{
			context.pass->setIlluminationStage(IS_AMBIENT);
		}
		else if (params == "per_light")
		{
			context.pass->setIlluminationStage(IS_PER_LIGHT);
		}
		else if (params == "decal")
		{
			context.pass->setIlluminationStage(IS_DECAL);
		}
		else
		{
			logParseError("Invalid illumination_stage specified.", context);
		}
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseLodValues(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");

        // iterate over the parameters and parse values out of them
        Material::LodValueList lodList;
        StringVector::iterator i, iend;
        iend = vecparams.end();
        for (i = vecparams.begin(); i != iend; ++i)
        {
            lodList.push_back(StringConverter::parseReal(*i));
        }

        context.material->setLodLevels(lodList);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodIndex(String& params, MaterialScriptContext& context)
    {
        context.technique->setLodIndex((ushort)StringConverter::parseInt(params));
        return false;
    }
	//-----------------------------------------------------------------------
	bool parseScheme(String& params, MaterialScriptContext& context)
	{
		context.technique->setSchemeName(params);
		return false;
	}
	//-----------------------------------------------------------------------
	bool parseGPUVendorRule(String& params, MaterialScriptContext& context)
	{
		Technique::GPUVendorRule rule;
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
			logParseError("Wrong number of parameters for gpu_vendor_rule, expected 2", context);
			return false;
		}
		if (vecparams[0] == "include")
		{
			rule.includeOrExclude = Technique::INCLUDE;
		}
		else if (vecparams[0] == "exclude")
		{
			rule.includeOrExclude = Technique::EXCLUDE;
		}
		else
		{
			logParseError("Wrong parameter to gpu_vendor_rule, expected 'include' or 'exclude'", context);
			return false;
		}

		rule.vendor = RenderSystemCapabilities::vendorFromString(vecparams[1]);
		if (rule.vendor == GPU_UNKNOWN)
		{
			logParseError("Unknown vendor '" + vecparams[1] + "' ignored in gpu_vendor_rule", context);
			return false;
		}
		context.technique->addGPUVendorRule(rule);
		return false;
	}
	//-----------------------------------------------------------------------
	bool parseGPUDeviceRule(String& params, MaterialScriptContext& context)
	{
		Technique::GPUDeviceNameRule rule;
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2 && vecparams.size() != 3)
		{
			logParseError("Wrong number of parameters for gpu_vendor_rule, expected 2 or 3", context);
			return false;
		}
		if (vecparams[0] == "include")
		{
			rule.includeOrExclude = Technique::INCLUDE;
		}
		else if (vecparams[0] == "exclude")
		{
			rule.includeOrExclude = Technique::EXCLUDE;
		}
		else
		{
			logParseError("Wrong parameter to gpu_device_rule, expected 'include' or 'exclude'", context);
			return false;
		}

		rule.devicePattern = vecparams[1];
		if (vecparams.size() == 3)
			rule.caseSensitive = StringConverter::parseBool(vecparams[2]);

		context.technique->addGPUDeviceNameRule(rule);
		return false;
	}
    //-----------------------------------------------------------------------
	bool parseShadowCasterMaterial(String& params, MaterialScriptContext& context)
	{
		context.technique->setShadowCasterMaterial(params);
		return false;
	}
	//-----------------------------------------------------------------------
	bool parseShadowReceiverMaterial(String& params, MaterialScriptContext& context)
	{
		context.technique->setShadowReceiverMaterial(params);
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseSetTextureAlias(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Wrong number of parameters for texture_alias, expected 2", context);
            return false;
        }
        // first parameter is alias name and second parameter is texture name
        context.textureAliases[vecparams[0]] = vecparams[1];

        return false;
    }
    //-----------------------------------------------------------------------
    inline size_t parseParamDimensions(String& dimensions, size_t start)
    {
        // Assume 1 unless otherwise specified
        size_t dims = 1;

        if (start != String::npos)
        {
            size_t end = dimensions.find_first_of("[", start);

            // int1, int2, etc.
            if (end != start)
            {
                dims *= StringConverter::parseInt(
                    dimensions.substr(start, end - start));
                start = end;
            }

            // C-style array
            while (start != String::npos)
            {
                end = dimensions.find_first_of("]", start);
                dims *= StringConverter::parseInt(
                    dimensions.substr(start + 1, end - start - 1));
                start = dimensions.find_first_of("[", start);
            }
        }
        
        return dims;
    }
    //-----------------------------------------------------------------------
	void processManualProgramParam(bool isNamed, const String &commandname,
                                   StringVector& vecparams, MaterialScriptContext& context,
                                   size_t index = 0, const String& paramName = StringUtil::BLANK)
    {
        // NB we assume that the first element of vecparams is taken up with either
        // the index or the parameter name, which we ignore

        // Determine type
        size_t dims, roundedDims, i;
        bool isReal;
        bool isMatrix4x4 = false;
        bool isInt = false;
        bool isUnsignedInt = false;

        StringUtil::toLowerCase(vecparams[1]);

        if (vecparams[1] == "matrix4x4")
        {
            dims = 16;
            isReal = true;
            isMatrix4x4 = true;
        }
        else if (vecparams[1].find("float") != String::npos)
        {
            dims = parseParamDimensions(vecparams[1], 
                                        vecparams[1].find_first_not_of("float"));
            isReal = true;
        }
        else if (vecparams[1].find("double") != String::npos)
        {
            dims = parseParamDimensions(vecparams[1], 
                                        vecparams[1].find_first_not_of("double"));
            isReal = true;
        }
        else if (vecparams[1].find("int") != String::npos)
        {
            dims = parseParamDimensions(vecparams[1], 
                                        vecparams[1].find_first_not_of("int"));
            isReal = false;
            isInt = true;
        }
        else if (vecparams[1].find("uint") != String::npos)
        {
            dims = parseParamDimensions(vecparams[1],
                                        vecparams[1].find_first_not_of("uint"));
            isReal = false;
            isUnsignedInt = true;
        }
        else if (vecparams[1].find("bool") != String::npos)
        {
            dims = parseParamDimensions(vecparams[1], 
                                        vecparams[1].find_first_not_of("bool"));
            isReal = false;
        }
        else
        {
            logParseError("Invalid " + commandname + " attribute - unrecognised "
                          "parameter type " + vecparams[1], context);
            return;
        }

        if (vecparams.size() != 2 + dims)
        {
            logParseError("Invalid " + commandname + " attribute - you need " +
                          StringConverter::toString(2 + dims) + " parameters for a parameter of "
                          "type " + vecparams[1], context);
        }

        // Clear any auto parameter bound to this constant, it would
        // override this setting can cause problems overriding
        // materials or changing default params
        if (isNamed)
            context.programParams->clearNamedAutoConstant(paramName);
        else
            context.programParams->clearAutoConstant(index);

        // Round dims to multiple of 4
        if (dims %4 != 0)
        {
            roundedDims = dims + 4 - (dims % 4);
        }
        else
        {
            roundedDims = dims;
        }

        // Now parse all the values
        if (isReal)
        {
            Real* realBuffer = OGRE_ALLOC_T(Real, roundedDims, MEMCATEGORY_SCRIPTING);
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                realBuffer[i] = StringConverter::parseReal(vecparams[i+2]);
            }
            // Fill up to multiple of 4 with zero
            for (; i < roundedDims; ++i)
            {
                realBuffer[i] = 0.0f;

            }

            if (isMatrix4x4)
            {
                // It's a Matrix4x4 so pass as a Matrix4. Use
                // specialized setConstant that takes a matrix so
                // matrix is transposed if required.
                Matrix4 m4x4(
                    realBuffer[0],  realBuffer[1],  realBuffer[2],  realBuffer[3],
                    realBuffer[4],  realBuffer[5],  realBuffer[6],  realBuffer[7],
                    realBuffer[8],  realBuffer[9],  realBuffer[10], realBuffer[11],
                    realBuffer[12], realBuffer[13], realBuffer[14], realBuffer[15]
                );
                if (isNamed)
                    context.programParams->setNamedConstant(paramName, m4x4);
                else
                    context.programParams->setConstant(index, m4x4);
            }
            else
            {
                // Set
                if (isNamed)
                {
                    // For named, only set up to the precise number of elements
                    // (no rounding to 4 elements)
                    // GLSL can support sub-float4 elements and we support that
                    // in the buffer now. Note how we set the 'multiple' param to 1
                    context.programParams->setNamedConstant(paramName, realBuffer,
                                                            dims, 1);
                }
                else
                {
                    context.programParams->setConstant(index, realBuffer,
                                                       static_cast<size_t>(roundedDims * 0.25));
                }

            }

            OGRE_FREE(realBuffer, MEMCATEGORY_SCRIPTING);
        }
        else if (isInt)
        {
            int* buffer = OGRE_ALLOC_T(int, roundedDims, MEMCATEGORY_SCRIPTING);
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                buffer[i] = StringConverter::parseInt(vecparams[i + 2]);
            }
            // Fill to multiple of 4 with 0
            for (; i < roundedDims; ++i)
            {
                buffer[i] = 0;
            }
            // Set
            if (isNamed)
            {
                // For named, only set up to the precise number of elements
                // (no rounding to 4 elements)
                // GLSL can support sub-float4 elements and we support that
                // in the buffer now. Note how we set the 'multiple' param to 1
                context.programParams->setNamedConstant(paramName, buffer,
                                                        dims, 1);
            }
            else
            {
                context.programParams->setConstant(index, buffer,
                                                   static_cast<size_t>(roundedDims * 0.25));
            }
            OGRE_FREE(buffer, MEMCATEGORY_SCRIPTING);
        }
        else if (isUnsignedInt)
        {
            uint* buffer = OGRE_ALLOC_T(uint, roundedDims, MEMCATEGORY_SCRIPTING);
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                buffer[i] = StringConverter::parseUnsignedInt(vecparams[i + 2]);
            }
            // Fill to multiple of 4 with 0
            for (; i < roundedDims; ++i)
            {
                buffer[i] = 0;
            }
            // Set
            if (isNamed)
            {
                // For named, only set up to the precise number of elements
                // (no rounding to 4 elements)
                // GLSL can support sub-float4 elements and we support that
                // in the buffer now. Note how we set the 'multiple' param to 1
                context.programParams->setNamedConstant(paramName, buffer,
                                                        dims, 1);
            }
            else
            {
                context.programParams->setConstant(index, buffer,
                                                   static_cast<size_t>(roundedDims * 0.25));
            }
            OGRE_FREE(buffer, MEMCATEGORY_SCRIPTING);
        }
        else // bool
        {
            uint* buffer = OGRE_ALLOC_T(uint, roundedDims, MEMCATEGORY_SCRIPTING);
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                buffer[i] = StringConverter::parseBool(vecparams[i + 2]) != 0;
            }
            // Fill to multiple of 4 with 0
            for (; i < roundedDims; ++i)
            {
                buffer[i] = false;
            }
            // Set
            if (isNamed)
            {
                // For named, only set up to the precise number of elements
                // (no rounding to 4 elements)
                // GLSL can support sub-float4 elements and we support that
                // in the buffer now. Note how we set the 'multiple' param to 1
                context.programParams->setNamedConstant(paramName, buffer,
                                                        dims, 1);
            }
            else
            {
                context.programParams->setConstant(index, buffer,
                                                   static_cast<size_t>(roundedDims * 0.25));
            }
            OGRE_FREE(buffer, MEMCATEGORY_SCRIPTING);
        }
    }
    //-----------------------------------------------------------------------
    void processAutoProgramParam(bool isNamed, const String& commandname,
        StringVector& vecparams, MaterialScriptContext& context,
		size_t index = 0, const String& paramName = StringUtil::BLANK)
    {
        // NB we assume that the first element of vecparams is taken up with either
        // the index or the parameter name, which we ignore

        // make sure param is in lower case
        StringUtil::toLowerCase(vecparams[1]);

        // lookup the param to see if its a valid auto constant
        const GpuProgramParameters::AutoConstantDefinition* autoConstantDef =
            context.programParams->getAutoConstantDefinition(vecparams[1]);

        // exit with error msg if the auto constant definition wasn't found
        if (!autoConstantDef)
		{
			logParseError("Invalid " + commandname + " attribute - "
				+ vecparams[1], context);
			return;
		}

        // add AutoConstant based on the type of data it uses
        switch (autoConstantDef->dataType)
        {
        case GpuProgramParameters::ACDT_NONE:
			if (isNamed)
				context.programParams->setNamedAutoConstant(paramName, autoConstantDef->acType, 0);
			else
	            context.programParams->setAutoConstant(index, autoConstantDef->acType, 0);
            break;

        case GpuProgramParameters::ACDT_INT:
            {
				// Special case animation_parametric, we need to keep track of number of times used
				if (autoConstantDef->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
				{
					if (isNamed)
						context.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, context.numAnimationParametrics++);
					else
						context.programParams->setAutoConstant(
							index, autoConstantDef->acType, context.numAnimationParametrics++);
				}
				// Special case texture projector - assume 0 if data not specified
				else if ((autoConstantDef->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX ||
						autoConstantDef->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
						autoConstantDef->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
						autoConstantDef->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX)
					&& vecparams.size() == 2)
				{
					if (isNamed)
						context.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, 0);
					else
						context.programParams->setAutoConstant(
							index, autoConstantDef->acType, 0);

				}
				else
				{

					if (vecparams.size() != 3)
					{
						logParseError("Invalid " + commandname + " attribute - "
							"expected 3 parameters.", context);
						return;
					}

					size_t extraParam = StringConverter::parseInt(vecparams[2]);
					if (isNamed)
						context.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, extraParam);
					else
						context.programParams->setAutoConstant(
							index, autoConstantDef->acType, extraParam);
				}
            }
            break;

        case GpuProgramParameters::ACDT_REAL:
            {
                // special handling for time
                if (autoConstantDef->acType == GpuProgramParameters::ACT_TIME ||
                    autoConstantDef->acType == GpuProgramParameters::ACT_FRAME_TIME)
                {
                    Real factor = 1.0f;
                    if (vecparams.size() == 3)
                    {
                        factor = StringConverter::parseReal(vecparams[2]);
                    }

					if (isNamed)
						context.programParams->setNamedAutoConstantReal(paramName, 
							autoConstantDef->acType, factor);
					else
	                    context.programParams->setAutoConstantReal(index, 
							autoConstantDef->acType, factor);
                }
                else // normal processing for auto constants that take an extra real value
                {
                    if (vecparams.size() != 3)
                    {
                        logParseError("Invalid " + commandname + " attribute - "
                            "expected 3 parameters.", context);
                        return;
                    }

			        Real rData = StringConverter::parseReal(vecparams[2]);
					if (isNamed)
						context.programParams->setNamedAutoConstantReal(paramName, 
							autoConstantDef->acType, rData);
					else
						context.programParams->setAutoConstantReal(index, 
							autoConstantDef->acType, rData);
                }
            }
            break;

        } // end switch


    }

    //-----------------------------------------------------------------------
    bool parseParamIndexed(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 3)
        {
            logParseError("Invalid param_indexed attribute - expected at least 3 parameters.",
                context);
            return false;
        }

        // Get start index
        size_t index = StringConverter::parseInt(vecparams[0]);

        processManualProgramParam(false, "param_indexed", vecparams, context, index);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamIndexedAuto(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2 && vecparams.size() != 3)
        {
            logParseError("Invalid param_indexed_auto attribute - expected 2 or 3 parameters.",
                context);
            return false;
        }

        // Get start index
        size_t index = StringConverter::parseInt(vecparams[0]);

        processAutoProgramParam(false, "param_indexed_auto", vecparams, context, index);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamNamed(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 3)
        {
            logParseError("Invalid param_named attribute - expected at least 3 parameters.",
                          context);
            return false;
        }

        try {
            const GpuConstantDefinition& def = 
                context.programParams->getConstantDefinition(vecparams[0]);
            (void)def; // Silence warning
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named attribute - " + e.getDescription(), context);
            return false;
        }

        processManualProgramParam(true, "param_named", vecparams, context, 0, vecparams[0]);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamNamedAuto(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2 && vecparams.size() != 3)
        {
            logParseError("Invalid param_indexed_auto attribute - expected 2 or 3 parameters.",
                context);
            return false;
        }

        // Get start index from name
        try {
			const GpuConstantDefinition& def = 
				context.programParams->getConstantDefinition(vecparams[0]);
            (void)def; // Silence warning
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named_auto attribute - " + e.getDescription(), context);
            return false;
        }

        processAutoProgramParam(true, "param_named_auto", vecparams, context, 0, vecparams[0]);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMaterial(String& params, MaterialScriptContext& context)
    {
        // nfz:
        // check params for reference to parent material to copy from
        // syntax: material name : parentMaterialName
        // check params for a colon after the first name and extract the parent name
        StringVector vecparams = StringUtil::split(params, ":", 1);
        MaterialPtr basematerial;

        // Create a brand new material
        if (vecparams.size() >= 2)
        {
            // if a second parameter exists then assume its the name of the base material
            // that this new material should clone from
            StringUtil::trim(vecparams[1]);
            // make sure base material exists
            basematerial = MaterialManager::getSingleton().getByName(vecparams[1]);
            // if it doesn't exist then report error in log and just create a new material
            if (basematerial.isNull())
            {
                logParseError("parent material: " + vecparams[1] + " not found for new material:"
                    + vecparams[0], context);
            }
        }

        // get rid of leading and trailing white space from material name
        StringUtil::trim(vecparams[0]);

        context.material =
            MaterialManager::getSingleton().create(vecparams[0], context.groupName);

        if (!basematerial.isNull())
        {
            // copy parent material details to new material
            basematerial->copyDetailsTo(context.material);
        }
        else
        {
            // Remove pre-created technique from defaults
            context.material->removeAllTechniques();
        }

		context.material->_notifyOrigin(context.filename);

        // update section
        context.section = MSS_MATERIAL;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseTechnique(String& params, MaterialScriptContext& context)
    {

        // if params is not empty then see if the technique name already exists
        if (!params.empty() && (context.material->getNumTechniques() > 0))
        {
            // find the technique with name = params
            Technique * foundTechnique = context.material->getTechnique(params);
            if (foundTechnique)
            {
                // figure out technique index by iterating through technique container
                // would be nice if each technique remembered its index
                int count = 0;
                Material::TechniqueIterator i = context.material->getTechniqueIterator();
                while(i.hasMoreElements())
                {
                    if (foundTechnique == i.peekNext())
                        break;
                    i.moveNext();
                    ++count;
                }

                context.techLev = count;
            }
            else
            {
                // name was not found so a new technique is needed
                // position technique level to the end index
                // a new technique will be created later on
                context.techLev = context.material->getNumTechniques();
            }

        }
        else
        {
            // no name was given in the script so a new technique will be created
		    // Increase technique level depth
		    ++context.techLev;
        }

        // Create a new technique if it doesn't already exist
        if (context.material->getNumTechniques() > context.techLev)
        {
            context.technique = context.material->getTechnique((ushort)context.techLev);
        }
        else
        {
            context.technique = context.material->createTechnique();
            if (!params.empty())
                context.technique->setName(params);
        }

        // update section
        context.section = MSS_TECHNIQUE;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parsePass(String& params, MaterialScriptContext& context)
    {
        // if params is not empty then see if the pass name already exists
        if (!params.empty() && (context.technique->getNumPasses() > 0))
        {
            // find the pass with name = params
            Pass * foundPass = context.technique->getPass(params);
            if (foundPass)
            {
                context.passLev = foundPass->getIndex();
            }
            else
            {
                // name was not found so a new pass is needed
                // position pass level to the end index
                // a new pass will be created later on
                context.passLev = context.technique->getNumPasses();
            }

        }
        else
        {
		    //Increase pass level depth
		    ++context.passLev;
        }

        if (context.technique->getNumPasses() > context.passLev)
        {
            context.pass = context.technique->getPass((ushort)context.passLev);
        }
        else
        {
            // Create a new pass
            context.pass = context.technique->createPass();
            if (!params.empty())
                context.pass->setName(params);
        }

        // update section
        context.section = MSS_PASS;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseTextureUnit(String& params, MaterialScriptContext& context)
    {
        // if params is a name then see if that texture unit exists
        // if not then log the warning and just move on to the next TU from current
        if (!params.empty() && (context.pass->getNumTextureUnitStates() > 0))
        {
            // specifying a TUS name in the script for a TU means that a specific TU is being requested
            // try to get the specific TU
            // if the index requested is not valid, just creat a new TU
            // find the TUS with name = params
            TextureUnitState * foundTUS = context.pass->getTextureUnitState(params);
            if (foundTUS)
            {
                context.stateLev = context.pass->getTextureUnitStateIndex(foundTUS);
            }
            else
            {
                // name was not found so a new TUS is needed
                // position TUS level to the end index
                // a new TUS will be created later on
                context.stateLev = static_cast<int>(context.pass->getNumTextureUnitStates());
            }
        }
        else
        {
		    //Increase Texture Unit State level depth
		    ++context.stateLev;
        }

        if (context.pass->getNumTextureUnitStates() > static_cast<size_t>(context.stateLev))
        {
            context.textureUnit = context.pass->getTextureUnitState((ushort)context.stateLev);
        }
        else
        {
            // Create a new texture unit
            context.textureUnit = context.pass->createTextureUnitState();
            if (!params.empty())
                context.textureUnit->setName(params);
        }
        // update section
        context.section = MSS_TEXTUREUNIT;

        // Return TRUE because this must be followed by a {
        return true;
    }

    //-----------------------------------------------------------------------
    bool parseVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        // check if pass has a vertex program already
        if (context.pass->hasVertexProgram())
        {
            // if existing pass vertex program has same name as params
            // or params is empty then use current vertex program
            if (params.empty() || (context.pass->getVertexProgramName() == params))
            {
                context.program = context.pass->getVertexProgram();
            }
        }

        // if context.program was not set then try to get the vertex program using the name
        // passed in params
        if (context.program.isNull())
        {
            context.program = GpuProgramManager::getSingleton().getByName(params);
            if (context.program.isNull())
            {
                // Unknown program
                logParseError("Invalid vertex_program_ref entry - vertex program "
                    + params + " has not been defined.", context);
                return true;
            }

            // Set the vertex program for this pass
            context.pass->setVertexProgram(params);
        }

        context.isVertexProgramShadowCaster = false;
        context.isFragmentProgramShadowCaster = false;
        context.isVertexProgramShadowReceiver = false;
        context.isFragmentProgramShadowReceiver = false;

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getVertexProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
	//-----------------------------------------------------------------------
    bool parseGeometryProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        // check if pass has a vertex program already
        if (context.pass->hasGeometryProgram())
        {
            // if existing pass vertex program has same name as params
            // or params is empty then use current vertex program
            if (params.empty() || (context.pass->getGeometryProgramName() == params))
            {
                context.program = context.pass->getGeometryProgram();
            }
        }

        // if context.program was not set then try to get the geometry program using the name
        // passed in params
        if (context.program.isNull())
        {
            context.program = GpuProgramManager::getSingleton().getByName(params);
            if (context.program.isNull())
            {
                // Unknown program
                logParseError("Invalid geometry_program_ref entry - vertex program "
                    + params + " has not been defined.", context);
                return true;
            }

            // Set the vertex program for this pass
            context.pass->setGeometryProgram(params);
        }

        context.isVertexProgramShadowCaster = false;
        context.isFragmentProgramShadowCaster = false;
        context.isVertexProgramShadowReceiver = false;
        context.isFragmentProgramShadowReceiver = false;

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getGeometryProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseShadowCasterVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid shadow_caster_vertex_program_ref entry - vertex program "
                + params + " has not been defined.", context);
            return true;
        }

        context.isVertexProgramShadowCaster = true;
        context.isFragmentProgramShadowCaster = false;
        context.isVertexProgramShadowReceiver = false;
		context.isFragmentProgramShadowReceiver = false;

        // Set the vertex program for this pass
        context.pass->setShadowCasterVertexProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getShadowCasterVertexProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseShadowCasterFragmentProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid shadow_caster_fragment_program_ref entry - fragment program "
                + params + " has not been defined.", context);
            return true;
        }

        context.isVertexProgramShadowCaster = false;
        context.isFragmentProgramShadowCaster = true;
        context.isVertexProgramShadowReceiver = false;
		context.isFragmentProgramShadowReceiver = false;

        // Set the vertex program for this pass
        context.pass->setShadowCasterFragmentProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getShadowCasterFragmentProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseShadowReceiverVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid shadow_receiver_vertex_program_ref entry - vertex program "
                + params + " has not been defined.", context);
            return true;
        }


        context.isVertexProgramShadowCaster = false;
        context.isFragmentProgramShadowCaster = false;
        context.isVertexProgramShadowReceiver = true;
		context.isFragmentProgramShadowReceiver = false;

        // Set the vertex program for this pass
        context.pass->setShadowReceiverVertexProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getShadowReceiverVertexProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
	//-----------------------------------------------------------------------
	bool parseShadowReceiverFragmentProgramRef(String& params, MaterialScriptContext& context)
	{
		// update section
		context.section = MSS_PROGRAM_REF;

		context.program = GpuProgramManager::getSingleton().getByName(params);
		if (context.program.isNull())
		{
			// Unknown program
			logParseError("Invalid shadow_receiver_fragment_program_ref entry - fragment program "
				+ params + " has not been defined.", context);
			return true;
		}


        context.isVertexProgramShadowCaster = false;
        context.isFragmentProgramShadowCaster = false;
		context.isVertexProgramShadowReceiver = false;
		context.isFragmentProgramShadowReceiver = true;

		// Set the vertex program for this pass
		context.pass->setShadowReceiverFragmentProgram(params);

		// Create params? Skip this if program is not supported
		if (context.program->isSupported())
		{
			context.programParams = context.pass->getShadowReceiverFragmentProgramParameters();
			context.numAnimationParametrics = 0;
		}

		// Return TRUE because this must be followed by a {
		return true;
	}
    //-----------------------------------------------------------------------
    bool parseFragmentProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        // check if pass has a fragment program already
        if (context.pass->hasFragmentProgram())
        {
            // if existing pass fragment program has same name as params
            // or params is empty then use current fragment program
            if (params.empty() || (context.pass->getFragmentProgramName() == params))
            {
                context.program = context.pass->getFragmentProgram();
            }
        }

        // if context.program was not set then try to get the fragment program using the name
        // passed in params
        if (context.program.isNull())
        {
            context.program = GpuProgramManager::getSingleton().getByName(params);
            if (context.program.isNull())
            {
                // Unknown program
                logParseError("Invalid fragment_program_ref entry - fragment program "
                    + params + " has not been defined.", context);
                return true;
            }

            // Set the vertex program for this pass
            context.pass->setFragmentProgram(params);
        }

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getFragmentProgramParameters();
			context.numAnimationParametrics = 0;
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseVertexProgram(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		context.programDef = OGRE_NEW_T(MaterialScriptProgramDefinition, MEMCATEGORY_SCRIPTING)();
		context.programDef->progType = GPT_VERTEX_PROGRAM;
        context.programDef->supportsSkeletalAnimation = false;
		context.programDef->supportsMorphAnimation = false;
		context.programDef->supportsPoseAnimation = 0;
		context.programDef->usesVertexTextureFetch = false;

		// Get name and language code
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
            logParseError("Invalid vertex_program entry - expected "
				"2 parameters.", context);
            return true;
		}
		// Name, preserve case
		context.programDef->name = vecparams[0];
		// language code, make lower case
		context.programDef->language = vecparams[1];
		StringUtil::toLowerCase(context.programDef->language);

        // Return TRUE because this must be followed by a {
        return true;
	}
    //-----------------------------------------------------------------------
    bool parseGeometryProgram(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		context.programDef = OGRE_NEW_T(MaterialScriptProgramDefinition, MEMCATEGORY_SCRIPTING)();
		context.programDef->progType = GPT_GEOMETRY_PROGRAM;
        context.programDef->supportsSkeletalAnimation = false;
		context.programDef->supportsMorphAnimation = false;
		context.programDef->supportsPoseAnimation = 0;
		context.programDef->usesVertexTextureFetch = false;

		// Get name and language code
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
            logParseError("Invalid geometry_program entry - expected "
				"2 parameters.", context);
            return true;
		}
		// Name, preserve case
		context.programDef->name = vecparams[0];
		// language code, make lower case
		context.programDef->language = vecparams[1];
		StringUtil::toLowerCase(context.programDef->language);

        // Return TRUE because this must be followed by a {
        return true;
	}
    //-----------------------------------------------------------------------
    bool parseFragmentProgram(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		context.programDef = OGRE_NEW_T(MaterialScriptProgramDefinition, MEMCATEGORY_SCRIPTING)();
		context.programDef->progType = GPT_FRAGMENT_PROGRAM;
		context.programDef->supportsSkeletalAnimation = false;
		context.programDef->supportsMorphAnimation = false;
		context.programDef->supportsPoseAnimation = 0;
		context.programDef->usesVertexTextureFetch = false;

		// Get name and language code
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
            logParseError("Invalid fragment_program entry - expected "
				"2 parameters.", context);
            return true;
		}
		// Name, preserve case
		context.programDef->name = vecparams[0];
		// language code, make lower case
		context.programDef->language = vecparams[1];
		StringUtil::toLowerCase(context.programDef->language);

		// Return TRUE because this must be followed by a {
        return true;

	}
    //-----------------------------------------------------------------------
    bool parseProgramSource(String& params, MaterialScriptContext& context)
    {
		// Source filename, preserve case
		context.programDef->source = params;

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseProgramSkeletalAnimation(String& params, MaterialScriptContext& context)
    {
        // Source filename, preserve case
        context.programDef->supportsSkeletalAnimation
            = StringConverter::parseBool(params);

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseProgramMorphAnimation(String& params, MaterialScriptContext& context)
	{
		// Source filename, preserve case
		context.programDef->supportsMorphAnimation
			= StringConverter::parseBool(params);

		return false;
	}
	//-----------------------------------------------------------------------
	bool parseProgramPoseAnimation(String& params, MaterialScriptContext& context)
	{
		// Source filename, preserve case
		context.programDef->supportsPoseAnimation
			= (ushort)StringConverter::parseInt(params);

		return false;
	}
	//-----------------------------------------------------------------------
	bool parseProgramVertexTextureFetch(String& params, MaterialScriptContext& context)
	{
		// Source filename, preserve case
		context.programDef->usesVertexTextureFetch
			= StringConverter::parseBool(params);

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseProgramSyntax(String& params, MaterialScriptContext& context)
    {
		// Syntax code, make lower case
        StringUtil::toLowerCase(params);
		context.programDef->syntax = params;

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseProgramCustomParameter(String& params, MaterialScriptContext& context)
    {
		// This params object does not have the command stripped
		// Lower case the command, but not the value incase it's relevant
		// Split only up to first delimiter, program deals with the rest
		StringVector vecparams = StringUtil::split(params, " \t", 1);
		if (vecparams.size() != 2)
		{
            logParseError("Invalid custom program parameter entry; "
				"there must be a parameter name and at least one value.",
				context);
            return false;
		}

		context.programDef->customParameters.push_back(
			std::pair<String, String>(vecparams[0], vecparams[1]));

		return false;
	}

	//-----------------------------------------------------------------------
    bool parseTextureSource(String& params, MaterialScriptContext& context)
    {
		StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 1)
			logParseError("Invalid texture source attribute - expected 1 parameter.",                 context);
        //The only param should identify which ExternalTextureSource is needed
		ExternalTextureSourceManager::getSingleton().setCurrentPlugIn( vecparams[0] );

		if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0 )
		{
			String tps;
			tps = StringConverter::toString( context.techLev ) + " "
				+ StringConverter::toString( context.passLev ) + " "
				+ StringConverter::toString( context.stateLev);

			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( "set_T_P_S", tps );
		}

        // update section
        context.section = MSS_TEXTURESOURCE;
        // Return TRUE because this must be followed by a {
        return true;
    }

    //-----------------------------------------------------------------------
    bool parseTextureCustomParameter(String& params, MaterialScriptContext& context)
    {
		// This params object does not have the command stripped
		// Split only up to first delimiter, program deals with the rest
		StringVector vecparams = StringUtil::split(params, " \t", 1);
		if (vecparams.size() != 2)
		{
            logParseError("Invalid texture parameter entry; "
				"there must be a parameter name and at least one value.",
				context);
            return false;
		}

		if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0 )
			////First is command, next could be a string with one or more values
			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( vecparams[0], vecparams[1] );

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseReceiveShadows(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.material->setReceiveShadows(true);
        else if (params == "off")
            context.material->setReceiveShadows(false);
        else
            logParseError(
            "Bad receive_shadows attribute, valid parameters are 'on' or 'off'.",
            context);

        return false;

    }
    //-----------------------------------------------------------------------
    bool parseDefaultParams(String& params, MaterialScriptContext& context)
    {
        context.section = MSS_DEFAULT_PARAMETERS;
        // Should be a brace next
        return true;
    }

	//-----------------------------------------------------------------------
	bool parseTransparencyCastsShadows(String& params, MaterialScriptContext& context)
	{
        StringUtil::toLowerCase(params);
		if (params == "on")
			context.material->setTransparencyCastsShadows(true);
		else if (params == "off")
			context.material->setTransparencyCastsShadows(false);
		else
			logParseError(
			"Bad transparency_casts_shadows attribute, valid parameters are 'on' or 'off'.",
			context);

		return false;

	}
    //-----------------------------------------------------------------------
    bool parseLodStrategy(String& params, MaterialScriptContext& context)
    {
        LodStrategy *strategy = LodStrategyManager::getSingleton().getStrategy(params);
        
        if (strategy == 0)
            logParseError(
            "Bad lod_strategy attribute, available LOD strategy name expected.",
            context);

        context.material->setLodStrategy(strategy);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodDistances(String& params, MaterialScriptContext& context)
    {
        // Set to distance strategy
        context.material->setLodStrategy(DistanceLodSphereStrategy::getSingletonPtr());

        StringVector vecparams = StringUtil::split(params, " \t");

        // iterate over the parameters and parse values out of them
        Material::LodValueList lodList;
        StringVector::iterator i, iend;
        iend = vecparams.end();
        for (i = vecparams.begin(); i != iend; ++i)
        {
            lodList.push_back(StringConverter::parseReal(*i));
        }

        context.material->setLodLevels(lodList);

        return false;
    }
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MaterialSerializer::MaterialSerializer()
    {
        // Set up root attribute parsers
        mRootAttribParsers.insert(AttribParserList::value_type("material", (ATTRIBUTE_PARSER)parseMaterial));
        mRootAttribParsers.insert(AttribParserList::value_type("vertex_program", (ATTRIBUTE_PARSER)parseVertexProgram));
		mRootAttribParsers.insert(AttribParserList::value_type("geometry_program", (ATTRIBUTE_PARSER)parseGeometryProgram));
        mRootAttribParsers.insert(AttribParserList::value_type("fragment_program", (ATTRIBUTE_PARSER)parseFragmentProgram));

        // Set up material attribute parsers
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_values", (ATTRIBUTE_PARSER)parseLodValues));
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_strategy", (ATTRIBUTE_PARSER)parseLodStrategy));
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_distances", (ATTRIBUTE_PARSER)parseLodDistances));
        mMaterialAttribParsers.insert(AttribParserList::value_type("receive_shadows", (ATTRIBUTE_PARSER)parseReceiveShadows));
		mMaterialAttribParsers.insert(AttribParserList::value_type("transparency_casts_shadows", (ATTRIBUTE_PARSER)parseTransparencyCastsShadows));
        mMaterialAttribParsers.insert(AttribParserList::value_type("technique", (ATTRIBUTE_PARSER)parseTechnique));
        mMaterialAttribParsers.insert(AttribParserList::value_type("set_texture_alias", (ATTRIBUTE_PARSER)parseSetTextureAlias));

        // Set up technique attribute parsers
        mTechniqueAttribParsers.insert(AttribParserList::value_type("lod_index", (ATTRIBUTE_PARSER)parseLodIndex));
		mTechniqueAttribParsers.insert(AttribParserList::value_type("shadow_caster_material", (ATTRIBUTE_PARSER)parseShadowCasterMaterial));
		mTechniqueAttribParsers.insert(AttribParserList::value_type("shadow_receiver_material", (ATTRIBUTE_PARSER)parseShadowReceiverMaterial));
		mTechniqueAttribParsers.insert(AttribParserList::value_type("scheme", (ATTRIBUTE_PARSER)parseScheme));
		mTechniqueAttribParsers.insert(AttribParserList::value_type("gpu_vendor_rule", (ATTRIBUTE_PARSER)parseGPUVendorRule));
		mTechniqueAttribParsers.insert(AttribParserList::value_type("gpu_device_rule", (ATTRIBUTE_PARSER)parseGPUDeviceRule));
        mTechniqueAttribParsers.insert(AttribParserList::value_type("pass", (ATTRIBUTE_PARSER)parsePass));

        // Set up pass attribute parsers
        mPassAttribParsers.insert(AttribParserList::value_type("ambient", (ATTRIBUTE_PARSER)parseAmbient));
        mPassAttribParsers.insert(AttribParserList::value_type("diffuse", (ATTRIBUTE_PARSER)parseDiffuse));
        mPassAttribParsers.insert(AttribParserList::value_type("specular", (ATTRIBUTE_PARSER)parseSpecular));
        mPassAttribParsers.insert(AttribParserList::value_type("emissive", (ATTRIBUTE_PARSER)parseEmissive));
        mPassAttribParsers.insert(AttribParserList::value_type("scene_blend", (ATTRIBUTE_PARSER)parseSceneBlend));
        mPassAttribParsers.insert(AttribParserList::value_type("separate_scene_blend", (ATTRIBUTE_PARSER)parseSeparateSceneBlend));
		mPassAttribParsers.insert(AttribParserList::value_type("depth_check", (ATTRIBUTE_PARSER)parseDepthCheck));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_write", (ATTRIBUTE_PARSER)parseDepthWrite));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_func", (ATTRIBUTE_PARSER)parseDepthFunc));
		mPassAttribParsers.insert(AttribParserList::value_type("normalise_normals", (ATTRIBUTE_PARSER)parseNormaliseNormals));
		mPassAttribParsers.insert(AttribParserList::value_type("alpha_rejection", (ATTRIBUTE_PARSER)parseAlphaRejection));
		mPassAttribParsers.insert(AttribParserList::value_type("alpha_to_coverage", (ATTRIBUTE_PARSER)parseAlphaToCoverage));
		mPassAttribParsers.insert(AttribParserList::value_type("transparent_sorting", (ATTRIBUTE_PARSER)parseTransparentSorting));
        mPassAttribParsers.insert(AttribParserList::value_type("colour_write", (ATTRIBUTE_PARSER)parseColourWrite));
		mPassAttribParsers.insert(AttribParserList::value_type("light_scissor", (ATTRIBUTE_PARSER)parseLightScissor));
		mPassAttribParsers.insert(AttribParserList::value_type("light_clip_planes", (ATTRIBUTE_PARSER)parseLightClip));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_hardware", (ATTRIBUTE_PARSER)parseCullHardware));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_software", (ATTRIBUTE_PARSER)parseCullSoftware));
        mPassAttribParsers.insert(AttribParserList::value_type("lighting", (ATTRIBUTE_PARSER)parseLighting));
        mPassAttribParsers.insert(AttribParserList::value_type("fog_override", (ATTRIBUTE_PARSER)parseFogging));
        mPassAttribParsers.insert(AttribParserList::value_type("shading", (ATTRIBUTE_PARSER)parseShading));
		mPassAttribParsers.insert(AttribParserList::value_type("polygon_mode", (ATTRIBUTE_PARSER)parsePolygonMode));
		mPassAttribParsers.insert(AttribParserList::value_type("polygon_mode_overrideable", (ATTRIBUTE_PARSER)parsePolygonModeOverrideable));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_bias", (ATTRIBUTE_PARSER)parseDepthBias));
		mPassAttribParsers.insert(AttribParserList::value_type("iteration_depth_bias", (ATTRIBUTE_PARSER)parseIterationDepthBias));
        mPassAttribParsers.insert(AttribParserList::value_type("texture_unit", (ATTRIBUTE_PARSER)parseTextureUnit));
        mPassAttribParsers.insert(AttribParserList::value_type("vertex_program_ref", (ATTRIBUTE_PARSER)parseVertexProgramRef));
		mPassAttribParsers.insert(AttribParserList::value_type("geometry_program_ref", (ATTRIBUTE_PARSER)parseGeometryProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_caster_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowCasterVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_caster_fragment_program_ref", (ATTRIBUTE_PARSER)parseShadowCasterFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_receiver_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowReceiverVertexProgramRef));
		mPassAttribParsers.insert(AttribParserList::value_type("shadow_receiver_fragment_program_ref", (ATTRIBUTE_PARSER)parseShadowReceiverFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("fragment_program_ref", (ATTRIBUTE_PARSER)parseFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("max_lights", (ATTRIBUTE_PARSER)parseMaxLights));
		mPassAttribParsers.insert(AttribParserList::value_type("start_light", (ATTRIBUTE_PARSER)parseStartLight));
        mPassAttribParsers.insert(AttribParserList::value_type("iteration", (ATTRIBUTE_PARSER)parseIteration));
		mPassAttribParsers.insert(AttribParserList::value_type("point_size", (ATTRIBUTE_PARSER)parsePointSize));
		mPassAttribParsers.insert(AttribParserList::value_type("point_sprites", (ATTRIBUTE_PARSER)parsePointSprites));
		mPassAttribParsers.insert(AttribParserList::value_type("point_size_attenuation", (ATTRIBUTE_PARSER)parsePointAttenuation));
		mPassAttribParsers.insert(AttribParserList::value_type("point_size_min", (ATTRIBUTE_PARSER)parsePointSizeMin));
		mPassAttribParsers.insert(AttribParserList::value_type("point_size_max", (ATTRIBUTE_PARSER)parsePointSizeMax));
		mPassAttribParsers.insert(AttribParserList::value_type("illumination_stage", (ATTRIBUTE_PARSER)parseIlluminationStage));

        // Set up texture unit attribute parsers
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture_source", (ATTRIBUTE_PARSER)parseTextureSource));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture", (ATTRIBUTE_PARSER)parseTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("anim_texture", (ATTRIBUTE_PARSER)parseAnimTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("cubic_texture", (ATTRIBUTE_PARSER)parseCubicTexture));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("binding_type", (ATTRIBUTE_PARSER)parseBindingType));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_coord_set", (ATTRIBUTE_PARSER)parseTexCoord));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_address_mode", (ATTRIBUTE_PARSER)parseTexAddressMode));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_border_colour", (ATTRIBUTE_PARSER)parseTexBorderColour));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op", (ATTRIBUTE_PARSER)parseColourOp));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_ex", (ATTRIBUTE_PARSER)parseColourOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_multipass_fallback", (ATTRIBUTE_PARSER)parseColourOpFallback));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("alpha_op_ex", (ATTRIBUTE_PARSER)parseAlphaOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("env_map", (ATTRIBUTE_PARSER)parseEnvMap));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll", (ATTRIBUTE_PARSER)parseScroll));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll_anim", (ATTRIBUTE_PARSER)parseScrollAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate", (ATTRIBUTE_PARSER)parseRotate));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate_anim", (ATTRIBUTE_PARSER)parseRotateAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scale", (ATTRIBUTE_PARSER)parseScale));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("wave_xform", (ATTRIBUTE_PARSER)parseWaveXform));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("transform", (ATTRIBUTE_PARSER)parseTransform));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("filtering", (ATTRIBUTE_PARSER)parseFiltering));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("compare_test", (ATTRIBUTE_PARSER)parseCompareTest));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("compare_func", (ATTRIBUTE_PARSER)parseCompareFunction));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("max_anisotropy", (ATTRIBUTE_PARSER)parseAnisotropy));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture_alias", (ATTRIBUTE_PARSER)parseTextureAlias));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("mipmap_bias", (ATTRIBUTE_PARSER)parseMipmapBias));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("content_type", (ATTRIBUTE_PARSER)parseContentType));

        // Set up program reference attribute parsers
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_indexed", (ATTRIBUTE_PARSER)parseParamIndexed));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_indexed_auto", (ATTRIBUTE_PARSER)parseParamIndexedAuto));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_named", (ATTRIBUTE_PARSER)parseParamNamed));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_named_auto", (ATTRIBUTE_PARSER)parseParamNamedAuto));

        // Set up program definition attribute parsers
        mProgramAttribParsers.insert(AttribParserList::value_type("source", (ATTRIBUTE_PARSER)parseProgramSource));
        mProgramAttribParsers.insert(AttribParserList::value_type("syntax", (ATTRIBUTE_PARSER)parseProgramSyntax));
        mProgramAttribParsers.insert(AttribParserList::value_type("includes_skeletal_animation", (ATTRIBUTE_PARSER)parseProgramSkeletalAnimation));
        mProgramAttribParsers.insert(AttribParserList::value_type("includes_morph_animation", (ATTRIBUTE_PARSER)parseProgramMorphAnimation));
        mProgramAttribParsers.insert(AttribParserList::value_type("includes_pose_animation", (ATTRIBUTE_PARSER)parseProgramPoseAnimation));
        mProgramAttribParsers.insert(AttribParserList::value_type("uses_vertex_texture_fetch", (ATTRIBUTE_PARSER)parseProgramVertexTextureFetch));
        mProgramAttribParsers.insert(AttribParserList::value_type("default_params", (ATTRIBUTE_PARSER)parseDefaultParams));

        // Set up program default param attribute parsers
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_indexed", (ATTRIBUTE_PARSER)parseParamIndexed));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_indexed_auto", (ATTRIBUTE_PARSER)parseParamIndexedAuto));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_named", (ATTRIBUTE_PARSER)parseParamNamed));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_named_auto", (ATTRIBUTE_PARSER)parseParamNamedAuto));

        mScriptContext.section = MSS_NONE;
        mScriptContext.material.setNull();
        mScriptContext.technique = 0;
        mScriptContext.pass = 0;
        mScriptContext.textureUnit = 0;
        mScriptContext.program.setNull();
        mScriptContext.lineNo = 0;
        mScriptContext.filename.clear();
		mScriptContext.techLev = -1;
		mScriptContext.passLev = -1;
		mScriptContext.stateLev = -1;
        mDefaults = false;

        mBuffer.clear();
    }

    //-----------------------------------------------------------------------
    void MaterialSerializer::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        String line;
        bool nextIsOpenBrace = false;

        mScriptContext.section = MSS_NONE;
        mScriptContext.material.setNull();
        mScriptContext.technique = 0;
        mScriptContext.pass = 0;
        mScriptContext.textureUnit = 0;
        mScriptContext.program.setNull();
        mScriptContext.lineNo = 0;
		mScriptContext.techLev = -1;
		mScriptContext.passLev = -1;
		mScriptContext.stateLev = -1;
        mScriptContext.filename = stream->getName();
		mScriptContext.groupName = groupName;
        while(!stream->eof())
        {
            line = stream->getLine();
            mScriptContext.lineNo++;

            // DEBUG LINE
            // LogManager::getSingleton().logMessage("About to attempt line(#" +
            //    StringConverter::toString(mScriptContext.lineNo) + "): " + line);

            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (nextIsOpenBrace)
                {
                    // NB, parser will have changed context already
                    if (line != "{")
                    {
                        logParseError("Expecting '{' but got " +
                            line + " instead.", mScriptContext);
                    }
                    nextIsOpenBrace = false;
                }
                else
                {
                    nextIsOpenBrace = parseScriptLine(line);
                }

            }
        }

        // Check all braces were closed
        if (mScriptContext.section != MSS_NONE)
        {
            logParseError("Unexpected end of file.", mScriptContext);
        }

		// Make sure we invalidate our context shared pointer (don't want to hold on)
		mScriptContext.material.setNull();

    }
    //-----------------------------------------------------------------------
    bool MaterialSerializer::parseScriptLine(String& line)
    {
        switch(mScriptContext.section)
        {
        case MSS_NONE:
            if (line == "}")
            {
                logParseError("Unexpected terminating brace.", mScriptContext);
                return false;
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mRootAttribParsers);
            }
            break;
        case MSS_MATERIAL:
            if (line == "}")
            {
                // End of material
                // if texture aliases were found, pass them to the material
                // to update texture names used in Texture unit states
                if (!mScriptContext.textureAliases.empty())
                {
                    // request material to update all texture names in TUS's
                    // that use texture aliases in the list
                    mScriptContext.material->applyTextureAliases(mScriptContext.textureAliases);
                }

                mScriptContext.section = MSS_NONE;
                mScriptContext.material.setNull();
				//Reset all levels for next material
				mScriptContext.passLev = -1;
				mScriptContext.stateLev= -1;
				mScriptContext.techLev = -1;
                mScriptContext.textureAliases.clear();
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mMaterialAttribParsers);
            }
            break;
        case MSS_TECHNIQUE:
            if (line == "}")
            {
                // End of technique
                mScriptContext.section = MSS_MATERIAL;
                mScriptContext.technique = NULL;
				mScriptContext.passLev = -1;	//Reset pass level (yes, the pass level)
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mTechniqueAttribParsers);
            }
            break;
        case MSS_PASS:
            if (line == "}")
            {
                // End of pass
                mScriptContext.section = MSS_TECHNIQUE;
                mScriptContext.pass = NULL;
				mScriptContext.stateLev = -1;	//Reset state level (yes, the state level)
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mPassAttribParsers);
            }
            break;
        case MSS_TEXTUREUNIT:
            if (line == "}")
            {
                // End of texture unit
                mScriptContext.section = MSS_PASS;
                mScriptContext.textureUnit = NULL;
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mTextureUnitAttribParsers);
            }
            break;
		case MSS_TEXTURESOURCE:
			if( line == "}" )
			{
				//End texture source section
				//Finish creating texture here
				String sMaterialName = mScriptContext.material->getName();
				if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0)
					ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->
					createDefinedTexture( sMaterialName, mScriptContext.groupName );
				//Revert back to texture unit
				mScriptContext.section = MSS_TEXTUREUNIT;
			}
			else
			{
				// custom texture parameter, use original line
				parseTextureCustomParameter(line, mScriptContext);
			}
			break;
        case MSS_PROGRAM_REF:
            if (line == "}")
            {
                // End of program
                mScriptContext.section = MSS_PASS;
                mScriptContext.program.setNull();
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mProgramRefAttribParsers);
            }
            break;
        case MSS_PROGRAM:
			// Program definitions are slightly different, they are deferred
			// until all the information required is known
            if (line == "}")
            {
                // End of program
				finishProgramDefinition();
                mScriptContext.section = MSS_NONE;
                OGRE_DELETE_T(mScriptContext.programDef, MaterialScriptProgramDefinition, MEMCATEGORY_SCRIPTING);
                mScriptContext.defaultParamLines.clear();
                mScriptContext.programDef = NULL;
            }
            else
            {
                // find & invoke a parser
				// do this manually because we want to call a custom
				// routine when the parser is not found
				// First, split line on first divisor only
				StringVector splitCmd = StringUtil::split(line, " \t", 1);
				// Find attribute parser
				AttribParserList::iterator iparser = mProgramAttribParsers.find(splitCmd[0]);
				if (iparser == mProgramAttribParsers.end())
				{
					// custom parameter, use original line
					parseProgramCustomParameter(line, mScriptContext);
				}
				else
				{
                    String cmd = splitCmd.size() >= 2? splitCmd[1]:StringUtil::BLANK;
					// Use parser with remainder
                    return iparser->second(cmd, mScriptContext );
				}

            }
            break;
        case MSS_DEFAULT_PARAMETERS:
            if (line == "}")
            {
                // End of default parameters
                mScriptContext.section = MSS_PROGRAM;
            }
            else
            {
                // Save default parameter lines up until we finalise the program
                mScriptContext.defaultParamLines.push_back(line);
            }


            break;
        };

        return false;
    }
    //-----------------------------------------------------------------------
	void MaterialSerializer::finishProgramDefinition(void)
	{
		// Now it is time to create the program and propagate the parameters
		MaterialScriptProgramDefinition* def = mScriptContext.programDef;
        GpuProgramPtr gp;
		if (def->language == "asm")
		{
			// Native assembler
			// Validate
			if (def->source.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a source file.", mScriptContext);
			}
			if (def->syntax.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a syntax code.", mScriptContext);
			}
			// Create
			gp = GpuProgramManager::getSingleton().
				createProgram(def->name, mScriptContext.groupName, def->source,
                    def->progType, def->syntax);

		}
		else
		{
			// High-level program
			// Validate
			if (def->source.empty() && def->language != "unified")
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a source file.", mScriptContext);
			}
			// Create
            try
            {
			    HighLevelGpuProgramPtr hgp = HighLevelGpuProgramManager::getSingleton().
				    createProgram(def->name, mScriptContext.groupName,
                        def->language, def->progType);
                // Assign to generalised version
                gp = hgp;
                // Set source file
                hgp->setSourceFile(def->source);

			    // Set custom parameters
				vector<std::pair<String, String> >::type::const_iterator i, iend;
			    iend = def->customParameters.end();
			    for (i = def->customParameters.begin(); i != iend; ++i)
			    {
				    if (!hgp->setParameter(i->first, i->second))
				    {
					    logParseError("Error in program " + def->name +
						    " parameter " + i->first + " is not valid.", mScriptContext);
				    }
			    }
            }
            catch (Exception& e)
            {
                logParseError("Could not create GPU program '"
                    + def->name + "', error reported was: " + e.getDescription(), mScriptContext);
				mScriptContext.program.setNull();
            	mScriptContext.programParams.setNull();
				return;
            }
        }
        // Set skeletal animation option
        gp->setSkeletalAnimationIncluded(def->supportsSkeletalAnimation);
		// Set morph animation option
		gp->setMorphAnimationIncluded(def->supportsMorphAnimation);
		// Set pose animation option
		gp->setPoseAnimationIncluded(def->supportsPoseAnimation);
		// Set vertex texture usage
		gp->setVertexTextureFetchRequired(def->usesVertexTextureFetch);
		// set origin
		gp->_notifyOrigin(mScriptContext.filename);

        // Set up to receive default parameters
        if (gp->isSupported()
            && !mScriptContext.defaultParamLines.empty())
        {
            mScriptContext.programParams = gp->getDefaultParameters();
			mScriptContext.numAnimationParametrics = 0;
            mScriptContext.program = gp;
            StringVector::iterator i, iend;
            iend = mScriptContext.defaultParamLines.end();
            for (i = mScriptContext.defaultParamLines.begin();
                i != iend; ++i)
            {
                // find & invoke a parser
                // do this manually because we want to call a custom
                // routine when the parser is not found
                // First, split line on first divisor only
                StringVector splitCmd = StringUtil::split(*i, " \t", 1);
                // Find attribute parser
                AttribParserList::iterator iparser
                    = mProgramDefaultParamAttribParsers.find(splitCmd[0]);
                if (iparser != mProgramDefaultParamAttribParsers.end())
                {
                    String cmd = splitCmd.size() >= 2? splitCmd[1]:StringUtil::BLANK;
                    // Use parser with remainder
                    iparser->second(cmd, mScriptContext );
                }

            }
            // Reset
            mScriptContext.program.setNull();
            mScriptContext.programParams.setNull();
        }

	}
    //-----------------------------------------------------------------------
	bool MaterialSerializer::invokeParser(String& line, AttribParserList& parsers)
    {
        // First, split line on first divisor only
        StringVector splitCmd(StringUtil::split(line, " \t", 1));

        // Find attribute parser
        AttribParserList::iterator iparser = parsers.find(splitCmd[0]);
        if (iparser == parsers.end())
        {
            // BAD command. BAD!
            logParseError("Unrecognised command: " + splitCmd[0], mScriptContext);
            return false;
        }
        else
        {
            String cmd;
            if(splitCmd.size() >= 2)
                cmd = splitCmd[1];
            // Use parser, make sure we have 2 params before using splitCmd[1]
            return iparser->second( cmd, mScriptContext );
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportMaterial(const MaterialPtr& pMat, const String &fileName, bool exportDefaults,
        const bool includeProgDef, const String& programFilename, const String& materialName)
    {
        clearQueue();
        mDefaults = exportDefaults;
        writeMaterial(pMat, materialName);
        exportQueued(fileName, includeProgDef, programFilename);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportQueued(const String &fileName, const bool includeProgDef, const String& programFilename)
    {
        // write out gpu program definitions to the buffer
        writeGpuPrograms();

        if (mBuffer.empty())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Queue is empty !", "MaterialSerializer::exportQueued");

        LogManager::getSingleton().logMessage("MaterialSerializer : writing material(s) to material script : " + fileName, LML_NORMAL);
        FILE *fp;
        fp = fopen(fileName.c_str(), "w");
        if (!fp)
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create material file.",
            "MaterialSerializer::export");

        // output gpu program definitions to material script file if includeProgDef is true
        if (includeProgDef && !mGpuProgramBuffer.empty())
        {
            fputs(mGpuProgramBuffer.c_str(), fp);
        }

        // output main buffer holding material script
        fputs(mBuffer.c_str(), fp);
        fclose(fp);

        // write program script if program filename and program definitions
        // were not included in material script
        if (!includeProgDef && !mGpuProgramBuffer.empty() && !programFilename.empty())
        {
            FILE *locFp;
            locFp = fopen(programFilename.c_str(), "w");
            if (!locFp)
                OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create program material file.",
                "MaterialSerializer::export");
            fputs(mGpuProgramBuffer.c_str(), locFp);
            fclose(locFp);
        }

        LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_NORMAL);
        clearQueue();
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::queueForExport(const MaterialPtr& pMat,
		bool clearQueued, bool exportDefaults, const String& materialName)
    {
        if (clearQueued)
            clearQueue();

        mDefaults = exportDefaults;
        writeMaterial(pMat, materialName);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::clearQueue()
    {
        mBuffer.clear();
        mGpuProgramBuffer.clear();
        mGpuProgramDefinitionContainer.clear();
    }
    //-----------------------------------------------------------------------
    const String &MaterialSerializer::getQueuedAsString() const
    {
        return mBuffer;
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeMaterial(const MaterialPtr& pMat, const String& materialName)
    {
		String outMaterialName;

		if (materialName.length() > 0)
		{
			outMaterialName = materialName;
		}
		else
		{
			outMaterialName = pMat->getName();
		}

        LogManager::getSingleton().logMessage("MaterialSerializer : writing material " + outMaterialName + " to queue.", LML_NORMAL);

		bool skipWriting = false;

		// Fire pre-write event.
		fireMaterialEvent(MSE_PRE_WRITE, skipWriting, pMat.get());
		if (skipWriting)		
			return;		

        // Material name
		writeAttribute(0, "material");
		writeValue(quoteWord(outMaterialName));
		
        beginSection(0);
        {
			// Fire write begin event.
			fireMaterialEvent(MSE_WRITE_BEGIN, skipWriting, pMat.get());

            // Write LOD information
            Material::LodValueIterator valueIt = pMat->getUserLodValueIterator();
            // Skip zero value
            if (valueIt.hasMoreElements())
                valueIt.getNext();
            String attributeVal;
            while (valueIt.hasMoreElements())
            {
                attributeVal.append(StringConverter::toString(valueIt.getNext()));
                if (valueIt.hasMoreElements())
                    attributeVal.append(" ");
            }
            if (!attributeVal.empty())
            {
                writeAttribute(1, "lod_values");
                writeValue(attributeVal);
            }


            // Shadow receive
            if (mDefaults ||
                pMat->getReceiveShadows() != true)
            {
                writeAttribute(1, "receive_shadows");
                writeValue(pMat->getReceiveShadows() ? "on" : "off");
            }

			// When rendering shadows, treat transparent things as opaque?
			if (mDefaults ||
				pMat->getTransparencyCastsShadows() == true)
			{
				writeAttribute(1, "transparency_casts_shadows");
				writeValue(pMat->getTransparencyCastsShadows() ? "on" : "off");
			}

            // Iterate over techniques
            Material::TechniqueIterator it = pMat->getTechniqueIterator();
            while (it.hasMoreElements())
            {
                writeTechnique(it.getNext());
                mBuffer += "\n";
            }

			// Fire write end event.
			fireMaterialEvent(MSE_WRITE_END, skipWriting, pMat.get());
        }
        endSection(0);
        mBuffer += "\n";

		// Fire post section write event.
		fireMaterialEvent(MSE_POST_WRITE, skipWriting, pMat.get());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTechnique(const Technique* pTech)
    {
		bool skipWriting = false;

		// Fire pre-write event.
		fireTechniqueEvent(MSE_PRE_WRITE, skipWriting, pTech);
		if (skipWriting)		
			return;	
		
        // Technique header
        writeAttribute(1, "technique");
        // only output technique name if it exists.
        if (!pTech->getName().empty())
            writeValue(quoteWord(pTech->getName()));

        beginSection(1);
        {
			// Fire write begin event.
			fireTechniqueEvent(MSE_WRITE_BEGIN, skipWriting, pTech);

			// LOD index
			if (mDefaults ||
				pTech->getLodIndex() != 0)
			{
				writeAttribute(2, "lod_index");
				writeValue(StringConverter::toString(pTech->getLodIndex()));
			}

			// Scheme name
			if (mDefaults ||
				pTech->getSchemeName() != MaterialManager::DEFAULT_SCHEME_NAME)
			{
				writeAttribute(2, "scheme");
				writeValue(quoteWord(pTech->getSchemeName()));
			}

			// ShadowCasterMaterial name
			if (!pTech->getShadowCasterMaterial().isNull())
			{
				writeAttribute(2, "shadow_caster_material");
				writeValue(quoteWord(pTech->getShadowCasterMaterial()->getName()));
			}
			// ShadowReceiverMaterial name
			if (!pTech->getShadowReceiverMaterial().isNull())
			{
				writeAttribute(2, "shadow_receiver_material");
				writeValue(quoteWord(pTech->getShadowReceiverMaterial()->getName()));
			}
			// GPU vendor rules
			Technique::GPUVendorRuleIterator vrit = pTech->getGPUVendorRuleIterator();
			while (vrit.hasMoreElements())
			{
				const Technique::GPUVendorRule& rule = vrit.getNext();
				writeAttribute(2, "gpu_vendor_rule");
				if (rule.includeOrExclude == Technique::INCLUDE)
					writeValue("include");
				else
					writeValue("exclude");
				writeValue(quoteWord(RenderSystemCapabilities::vendorToString(rule.vendor)));
			}
			// GPU device rules
			Technique::GPUDeviceNameRuleIterator dnit = pTech->getGPUDeviceNameRuleIterator();
			while (dnit.hasMoreElements())
			{
				const Technique::GPUDeviceNameRule& rule = dnit.getNext();
				writeAttribute(2, "gpu_device_rule");
				if (rule.includeOrExclude == Technique::INCLUDE)
					writeValue("include");
				else
					writeValue("exclude");
				writeValue(quoteWord(rule.devicePattern));
				writeValue(StringConverter::toString(rule.caseSensitive));
			}
            // Iterate over passes
            Technique::PassIterator it = const_cast<Technique*>(pTech)->getPassIterator();
            while (it.hasMoreElements())
            {
                writePass(it.getNext());
                mBuffer += "\n";
            }

			// Fire write end event.
			fireTechniqueEvent(MSE_WRITE_END, skipWriting, pTech);
        }
        endSection(1);

		// Fire post section write event.
		fireTechniqueEvent(MSE_POST_WRITE, skipWriting, pTech);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writePass(const Pass* pPass)
    {
		bool skipWriting = false;

		// Fire pre-write event.
		firePassEvent(MSE_PRE_WRITE, skipWriting, pPass);
		if (skipWriting)		
			return;
		
        writeAttribute(2, "pass");
        // only output pass name if its not the default name
        if (pPass->getName() != StringConverter::toString(pPass->getIndex()))
            writeValue(quoteWord(pPass->getName()));

        beginSection(2);
        {
			// Fire write begin event.
			firePassEvent(MSE_WRITE_BEGIN, skipWriting, pPass);

            //lighting
            if (mDefaults ||
                pPass->getLightingEnabled() != true)
            {
                writeAttribute(3, "lighting");
                writeValue(pPass->getLightingEnabled() ? "on" : "off");
            }
			// max_lights
            if (mDefaults ||
                pPass->getMaxSimultaneousLights() != OGRE_MAX_SIMULTANEOUS_LIGHTS)
            {
                writeAttribute(3, "max_lights");
                writeValue(StringConverter::toString(pPass->getMaxSimultaneousLights()));
            }
			// start_light
			if (mDefaults ||
				pPass->getStartLight() != 0)
			{
				writeAttribute(3, "start_light");
				writeValue(StringConverter::toString(pPass->getStartLight()));
			}
			// iteration
            if (mDefaults ||
                pPass->getIteratePerLight() || (pPass->getPassIterationCount() > 1))
            {
                writeAttribute(3, "iteration");
                // pass iteration count
                if (pPass->getPassIterationCount() > 1 || pPass->getLightCountPerIteration() > 1)
                {
                    writeValue(StringConverter::toString(pPass->getPassIterationCount()));
                    if (pPass->getIteratePerLight())
					{
						if (pPass->getLightCountPerIteration() > 1)
						{
							writeValue("per_n_lights");
							writeValue(StringConverter::toString(
								pPass->getLightCountPerIteration()));
						}
						else
						{
							writeValue("per_light");
						}
					}
                }
                else
                {
                    writeValue(pPass->getIteratePerLight() ? "once_per_light" : "once");
                }

                if (pPass->getIteratePerLight() && pPass->getRunOnlyForOneLightType())
                {
                    switch (pPass->getOnlyLightType())
                    {
                    case Light::LT_DIRECTIONAL:
                        writeValue("directional");
                        break;
                    case Light::LT_POINT:
                        writeValue("point");
                        break;
                    case Light::LT_SPOTLIGHT:
                        writeValue("spot");
                        break;
                    };
                }
            }

			if(mDefaults || pPass->getLightMask() != 0xFFFFFFFF)
			{
				writeAttribute(3, "light_mask");
				writeValue(StringConverter::toString(pPass->getLightMask()));
			}

            if (pPass->getLightingEnabled())
            {
                // Ambient
                if (mDefaults ||
                    pPass->getAmbient().r != 1 ||
                    pPass->getAmbient().g != 1 ||
                    pPass->getAmbient().b != 1 ||
                    pPass->getAmbient().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_AMBIENT))
                {
                    writeAttribute(3, "ambient");
                    if (pPass->getVertexColourTracking() & TVC_AMBIENT)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getAmbient(), true);
                }

                // Diffuse
                if (mDefaults ||
                    pPass->getDiffuse().r != 1 ||
                    pPass->getDiffuse().g != 1 ||
                    pPass->getDiffuse().b != 1 ||
                    pPass->getDiffuse().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_DIFFUSE))
                {
                    writeAttribute(3, "diffuse");
                    if (pPass->getVertexColourTracking() & TVC_DIFFUSE)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getDiffuse(), true);
                }

                // Specular
                if (mDefaults ||
                    pPass->getSpecular().r != 0 ||
                    pPass->getSpecular().g != 0 ||
                    pPass->getSpecular().b != 0 ||
                    pPass->getSpecular().a != 1 ||
                    pPass->getShininess() != 0 ||
                    (pPass->getVertexColourTracking() & TVC_SPECULAR))
                {
                    writeAttribute(3, "specular");
                    if (pPass->getVertexColourTracking() & TVC_SPECULAR)
                    {
                        writeValue("vertexcolour");
                    }
                    else
                    {
                        writeColourValue(pPass->getSpecular(), true);
                    }
                    writeValue(StringConverter::toString(pPass->getShininess()));

                }

                // Emissive
                if (mDefaults ||
                    pPass->getSelfIllumination().r != 0 ||
                    pPass->getSelfIllumination().g != 0 ||
                    pPass->getSelfIllumination().b != 0 ||
                    pPass->getSelfIllumination().a != 1 ||
                    (pPass->getVertexColourTracking() & TVC_EMISSIVE))
                {
                    writeAttribute(3, "emissive");
                    if (pPass->getVertexColourTracking() & TVC_EMISSIVE)
                        writeValue("vertexcolour");
                    else
                        writeColourValue(pPass->getSelfIllumination(), true);
                }
            }

            // Point size
            if (mDefaults ||
                pPass->getPointSize() != 1.0)
            {
                writeAttribute(3, "point_size");
                writeValue(StringConverter::toString(pPass->getPointSize()));
            }

            // Point sprites
            if (mDefaults ||
                pPass->getPointSpritesEnabled())
            {
                writeAttribute(3, "point_sprites");
                writeValue(pPass->getPointSpritesEnabled() ? "on" : "off");
            }

            // Point attenuation
            if (mDefaults ||
                pPass->isPointAttenuationEnabled())
            {
                writeAttribute(3, "point_size_attenuation");
                writeValue(pPass->isPointAttenuationEnabled() ? "on" : "off");
                if (pPass->isPointAttenuationEnabled() &&
                    (pPass->getPointAttenuationConstant() != 0.0 ||
                     pPass->getPointAttenuationLinear() != 1.0 ||
                     pPass->getPointAttenuationQuadratic() != 0.0))
                {
                    writeValue(StringConverter::toString(pPass->getPointAttenuationConstant()));
                    writeValue(StringConverter::toString(pPass->getPointAttenuationLinear()));
                    writeValue(StringConverter::toString(pPass->getPointAttenuationQuadratic()));
                }
            }

            // Point min size
            if (mDefaults ||
                pPass->getPointMinSize() != 0.0)
            {
                writeAttribute(3, "point_size_min");
                writeValue(StringConverter::toString(pPass->getPointMinSize()));
            }

            // Point max size
            if (mDefaults ||
                pPass->getPointMaxSize() != 0.0)
            {
                writeAttribute(3, "point_size_max");
                writeValue(StringConverter::toString(pPass->getPointMaxSize()));
            }

            // scene blend factor
			if (pPass->hasSeparateSceneBlending())
			{
				if (mDefaults ||
					pPass->getSourceBlendFactor() != SBF_ONE ||
					pPass->getDestBlendFactor() != SBF_ZERO ||
					pPass->getSourceBlendFactorAlpha() != SBF_ONE ||
					pPass->getDestBlendFactorAlpha() != SBF_ZERO)
				{
					writeAttribute(3, "separate_scene_blend");
					writeSceneBlendFactor(pPass->getSourceBlendFactor(), pPass->getDestBlendFactor(), 
						pPass->getSourceBlendFactorAlpha(), pPass->getDestBlendFactorAlpha());
				}
			}
			else
			{
				if (mDefaults ||
					pPass->getSourceBlendFactor() != SBF_ONE ||
					pPass->getDestBlendFactor() != SBF_ZERO)
				{
					writeAttribute(3, "scene_blend");
					writeSceneBlendFactor(pPass->getSourceBlendFactor(), pPass->getDestBlendFactor());
				}
			}


            //depth check
            if (mDefaults ||
                pPass->getDepthCheckEnabled() != true)
            {
                writeAttribute(3, "depth_check");
                writeValue(pPass->getDepthCheckEnabled() ? "on" : "off");
            }
			// alpha_rejection
			if (mDefaults ||
				pPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS ||
				pPass->getAlphaRejectValue() != 0)
			{
				writeAttribute(3, "alpha_rejection");
				writeCompareFunction(pPass->getAlphaRejectFunction());
				writeValue(StringConverter::toString(pPass->getAlphaRejectValue()));
			}
			// alpha_to_coverage
			if (mDefaults ||
				pPass->isAlphaToCoverageEnabled())
			{
				writeAttribute(3, "alpha_to_coverage");
				writeValue(pPass->isAlphaToCoverageEnabled() ? "on" : "off");
			}
			// transparent_sorting
			if (mDefaults ||
				pPass->getTransparentSortingForced() == true ||
				pPass->getTransparentSortingEnabled() != true)
			{
				writeAttribute(3, "transparent_sorting");
				writeValue(pPass->getTransparentSortingForced() ? "force" :
					(pPass->getTransparentSortingEnabled() ? "on" : "off"));
			}


            //depth write
            if (mDefaults ||
                pPass->getDepthWriteEnabled() != true)
            {
                writeAttribute(3, "depth_write");
                writeValue(pPass->getDepthWriteEnabled() ? "on" : "off");
            }

            //depth function
            if (mDefaults ||
                pPass->getDepthFunction() != CMPF_LESS_EQUAL)
            {
                writeAttribute(3, "depth_func");
                writeCompareFunction(pPass->getDepthFunction());
            }

            //depth bias
            if (mDefaults ||
                pPass->getDepthBiasConstant() != 0 ||
				pPass->getDepthBiasSlopeScale() != 0)
            {
                writeAttribute(3, "depth_bias");
                writeValue(StringConverter::toString(pPass->getDepthBiasConstant()));
				writeValue(StringConverter::toString(pPass->getDepthBiasSlopeScale()));
            }
			//iteration depth bias
			if (mDefaults ||
				pPass->getIterationDepthBias() != 0)
			{
				writeAttribute(3, "iteration_depth_bias");
				writeValue(StringConverter::toString(pPass->getIterationDepthBias()));
			}

			//light scissor
			if (mDefaults ||
				pPass->getLightScissoringEnabled() != false)
			{
				writeAttribute(3, "light_scissor");
				writeValue(pPass->getLightScissoringEnabled() ? "on" : "off");
			}

			//light clip planes
			if (mDefaults ||
				pPass->getLightClipPlanesEnabled() != false)
			{
				writeAttribute(3, "light_clip_planes");
				writeValue(pPass->getLightClipPlanesEnabled() ? "on" : "off");
			}

			// illumination stage
			if (pPass->getIlluminationStage() != IS_UNKNOWN)
			{
				writeAttribute(3, "illumination_stage");
				switch(pPass->getIlluminationStage())
				{
				case IS_AMBIENT:
					writeValue("ambient");
					break;
				case IS_PER_LIGHT:
					writeValue("per_light");
					break;
				case IS_DECAL:
					writeValue("decal");
					break;
                case IS_UNKNOWN:
                    break;
				};
			}

			// hardware culling mode
            if (mDefaults ||
                pPass->getCullingMode() != CULL_CLOCKWISE)
            {
                CullingMode hcm = pPass->getCullingMode();
                writeAttribute(3, "cull_hardware");
                switch (hcm)
                {
                case CULL_NONE :
                    writeValue("none");
                    break;
                case CULL_CLOCKWISE :
                    writeValue("clockwise");
                    break;
                case CULL_ANTICLOCKWISE :
                    writeValue("anticlockwise");
                    break;
                }
            }

            // software culling mode
            if (mDefaults ||
                pPass->getManualCullingMode() != MANUAL_CULL_BACK)
            {
                ManualCullingMode scm = pPass->getManualCullingMode();
                writeAttribute(3, "cull_software");
                switch (scm)
                {
                case MANUAL_CULL_NONE :
                    writeValue("none");
                    break;
                case MANUAL_CULL_BACK :
                    writeValue("back");
                    break;
                case MANUAL_CULL_FRONT :
                    writeValue("front");
                    break;
                }
            }

            //shading
            if (mDefaults ||
                pPass->getShadingMode() != SO_GOURAUD)
            {
                writeAttribute(3, "shading");
                switch (pPass->getShadingMode())
                {
                case SO_FLAT:
                    writeValue("flat");
                    break;
                case SO_GOURAUD:
                    writeValue("gouraud");
                    break;
                case SO_PHONG:
                    writeValue("phong");
                    break;
                }
            }


			if (mDefaults ||
				pPass->getPolygonMode() != PM_SOLID)
			{
				writeAttribute(3, "polygon_mode");
				switch (pPass->getPolygonMode())
				{
				case PM_POINTS:
					writeValue("points");
					break;
				case PM_WIREFRAME:
					writeValue("wireframe");
					break;
				case PM_SOLID:
					writeValue("solid");
					break;
				}
			}

			// polygon mode overrideable
			if (mDefaults ||
				!pPass->getPolygonModeOverrideable())
			{
				writeAttribute(3, "polygon_mode_overrideable");
				writeValue(pPass->getPolygonModeOverrideable() ? "on" : "off");
			}

			// normalise normals
			if (mDefaults ||
				pPass->getNormaliseNormals() != false)
			{
				writeAttribute(3, "normalise_normals");
				writeValue(pPass->getNormaliseNormals() ? "on" : "off");
			}

            //fog override
            if (mDefaults ||
                pPass->getFogOverride() != false)
            {
                writeAttribute(3, "fog_override");
                writeValue(pPass->getFogOverride() ? "true" : "false");
                if (pPass->getFogOverride())
                {
                    switch (pPass->getFogMode())
                    {
                    case FOG_NONE:
                        writeValue("none");
                        break;
                    case FOG_LINEAR:
                        writeValue("linear");
                        break;
                    case FOG_EXP2:
                        writeValue("exp2");
                        break;
                    case FOG_EXP:
                        writeValue("exp");
                        break;
                    }

                    if (pPass->getFogMode() != FOG_NONE)
                    {
                        writeColourValue(pPass->getFogColour());
                        writeValue(StringConverter::toString(pPass->getFogDensity()));
                        writeValue(StringConverter::toString(pPass->getFogStart()));
                        writeValue(StringConverter::toString(pPass->getFogEnd()));
                    }
                }
            }

            // nfz

            //  GPU Vertex and Fragment program references and parameters
            if (pPass->hasVertexProgram())
            {
                writeVertexProgramRef(pPass);
            }

            if (pPass->hasFragmentProgram())
            {
                writeFragmentProgramRef(pPass);
            }

            if (pPass->hasShadowCasterVertexProgram())
            {
                writeShadowCasterVertexProgramRef(pPass);
            }

            if (pPass->hasShadowReceiverVertexProgram())
            {
                writeShadowReceiverVertexProgramRef(pPass);
            }

            if (pPass->hasShadowReceiverFragmentProgram())
            {
                writeShadowReceiverFragmentProgramRef(pPass);
            }

            // Nested texture layers
            Pass::TextureUnitStateIterator it = const_cast<Pass*>(pPass)->getTextureUnitStateIterator();
            while(it.hasMoreElements())
            {
                writeTextureUnit(it.getNext());
            }

			// Fire write end event.
			firePassEvent(MSE_WRITE_END, skipWriting, pPass);
        }
        endSection(2);
		
		// Fire post section write event.
		firePassEvent(MSE_POST_WRITE, skipWriting, pPass);
        
		LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_NORMAL);
    }
    //-----------------------------------------------------------------------
    String MaterialSerializer::convertFiltering(FilterOptions fo)
    {
        switch (fo)
        {
        case FO_NONE:
            return "none";
        case FO_POINT:
            return "point";
        case FO_LINEAR:
            return "linear";
        case FO_ANISOTROPIC:
            return "anisotropic";
        }

        return "point";
    }
    //-----------------------------------------------------------------------
    String convertTexAddressMode(TextureUnitState::TextureAddressingMode tam)
	{
        switch (tam)
        {
        case TextureUnitState::TAM_BORDER:
            return "border";
        case TextureUnitState::TAM_CLAMP:
            return "clamp";
        case TextureUnitState::TAM_MIRROR:
            return "mirror";
        case TextureUnitState::TAM_WRAP:
        case TextureUnitState::TAM_UNKNOWN:
            return "wrap";
        }

        return "wrap";
	}
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTextureUnit(const TextureUnitState *pTex)
    {
		bool skipWriting = false;

		// Fire pre-write event.
		fireTextureUnitStateEvent(MSE_PRE_WRITE, skipWriting, pTex);
		if (skipWriting)		
			return;
	
        LogManager::getSingleton().logMessage("MaterialSerializer : parsing texture layer.", LML_NORMAL);
        mBuffer += "\n";
        writeAttribute(3, "texture_unit");
        // only write out name if its not equal to the default name
        if (pTex->getName() != StringConverter::toString(pTex->getParent()->getTextureUnitStateIndex(pTex)))
            writeValue(quoteWord(pTex->getName()));

        beginSection(3);
        {
			// Fire write begin event.
			fireTextureUnitStateEvent(MSE_WRITE_BEGIN, skipWriting, pTex);

            // texture_alias
            if (!pTex->getTextureNameAlias().empty())
            {
                writeAttribute(4, "texture_alias");
                writeValue(quoteWord(pTex->getTextureNameAlias()));
            }

            //texture name
            if (pTex->getNumFrames() == 1 && !pTex->getTextureName().empty() && !pTex->isCubic())
            {
                writeAttribute(4, "texture");
                writeValue(quoteWord(pTex->getTextureName()));

                switch (pTex->getTextureType())
                {
                case TEX_TYPE_1D:
                    writeValue("1d");
                    break;
                case TEX_TYPE_2D:
                    // nothing, this is the default
                    break;
                case TEX_TYPE_3D:
                    writeValue("3d");
                    break;
                case TEX_TYPE_CUBE_MAP:
                    // nothing, deal with this as cubic_texture since it copes with all variants
                    break;
                default:
                    break;
                };

                if (pTex->getNumMipmaps() != MIP_DEFAULT)
                {
                    writeValue(StringConverter::toString(pTex->getNumMipmaps()));
                }

                if (pTex->getIsAlpha())
                {
                    writeValue("alpha");
                }

                if (pTex->getDesiredFormat() != PF_UNKNOWN)
                {
                    writeValue(PixelUtil::getFormatName(pTex->getDesiredFormat()));
                }
            }

            //anim. texture
            if (pTex->getNumFrames() > 1 && !pTex->isCubic())
            {
                writeAttribute(4, "anim_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(quoteWord(pTex->getFrameTextureName(n)));
                writeValue(StringConverter::toString(pTex->getAnimationDuration()));
            }

            //cubic texture
            if (pTex->isCubic())
            {
                writeAttribute(4, "cubic_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(quoteWord(pTex->getFrameTextureName(n)));

                //combinedUVW/separateUW
                if (pTex->getTextureType() == TEX_TYPE_CUBE_MAP)
                    writeValue("combinedUVW");
                else
                    writeValue("separateUV");
            }

            //anisotropy level
            if (mDefaults ||
                pTex->getTextureAnisotropy() != 1)
            {
                writeAttribute(4, "max_anisotropy");
                writeValue(StringConverter::toString(pTex->getTextureAnisotropy()));
            }

            //texture coordinate set
            if (mDefaults ||
                pTex->getTextureCoordSet() != 0)
            {
                writeAttribute(4, "tex_coord_set");
                writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
            }

            //addressing mode
			const TextureUnitState::UVWAddressingMode& uvw =
				pTex->getTextureAddressingMode();
            if (mDefaults ||
                uvw.u != Ogre::TextureUnitState::TAM_WRAP ||
				uvw.v != Ogre::TextureUnitState::TAM_WRAP ||
				uvw.w != Ogre::TextureUnitState::TAM_WRAP )
            {
                writeAttribute(4, "tex_address_mode");
                if (uvw.u == uvw.v && uvw.u == uvw.w)
                {
                    writeValue(convertTexAddressMode(uvw.u));
                }
                else
                {
                    writeValue(convertTexAddressMode(uvw.u));
                    writeValue(convertTexAddressMode(uvw.v));
                    if (uvw.w != TextureUnitState::TAM_WRAP)
                    {
                        writeValue(convertTexAddressMode(uvw.w));
                    }
                }
            }

            //border colour
            const ColourValue& borderColour =
                pTex->getTextureBorderColour();
            if (mDefaults ||
                borderColour != ColourValue::Black)
            {
                writeAttribute(4, "tex_border_colour");
                writeColourValue(borderColour, true);
            }

            //filtering
            if (mDefaults ||
                pTex->getTextureFiltering(FT_MIN) != FO_LINEAR ||
                pTex->getTextureFiltering(FT_MAG) != FO_LINEAR ||
                pTex->getTextureFiltering(FT_MIP) != FO_POINT)
            {
                writeAttribute(4, "filtering");
                writeValue(
                    convertFiltering(pTex->getTextureFiltering(FT_MIN))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MAG))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MIP)));
            }

			// Mip biasing
			if (mDefaults ||
				pTex->getTextureMipmapBias() != 0.0f)
			{
				writeAttribute(4, "mipmap_bias");
				writeValue(
					StringConverter::toString(pTex->getTextureMipmapBias()));
			}

            // colour_op_ex
            if (mDefaults ||
                pTex->getColourBlendMode().operation != LBX_MODULATE ||
                pTex->getColourBlendMode().source1 != LBS_TEXTURE ||
                pTex->getColourBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "colour_op_ex");
                writeLayerBlendOperationEx(pTex->getColourBlendMode().operation);
                writeLayerBlendSource(pTex->getColourBlendMode().source1);
                writeLayerBlendSource(pTex->getColourBlendMode().source2);
                if (pTex->getColourBlendMode().operation == LBX_BLEND_MANUAL)
                    writeValue(StringConverter::toString(pTex->getColourBlendMode().factor));
                if (pTex->getColourBlendMode().source1 == LBS_MANUAL)
                    writeColourValue(pTex->getColourBlendMode().colourArg1, false);
                if (pTex->getColourBlendMode().source2 == LBS_MANUAL)
                    writeColourValue(pTex->getColourBlendMode().colourArg2, false);

                //colour_op_multipass_fallback
                writeAttribute(4, "colour_op_multipass_fallback");
                writeSceneBlendFactor(pTex->getColourBlendFallbackSrc());
                writeSceneBlendFactor(pTex->getColourBlendFallbackDest());
            }

            // alpha_op_ex
            if (mDefaults ||
                pTex->getAlphaBlendMode().operation != LBX_MODULATE ||
                pTex->getAlphaBlendMode().source1 != LBS_TEXTURE ||
                pTex->getAlphaBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "alpha_op_ex");
                writeLayerBlendOperationEx(pTex->getAlphaBlendMode().operation);
                writeLayerBlendSource(pTex->getAlphaBlendMode().source1);
                writeLayerBlendSource(pTex->getAlphaBlendMode().source2);
                if (pTex->getAlphaBlendMode().operation == LBX_BLEND_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().factor));
                else if (pTex->getAlphaBlendMode().source1 == LBS_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg1));
                else if (pTex->getAlphaBlendMode().source2 == LBS_MANUAL)
                    writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg2));
            }

			bool individualTransformElems = false;
            // rotate
            if (mDefaults ||
                pTex->getTextureRotate() != Radian(0))
            {
                writeAttribute(4, "rotate");
                writeValue(StringConverter::toString(pTex->getTextureRotate().valueDegrees()));
				individualTransformElems = true;
            }

            // scroll
            if (mDefaults ||
                pTex->getTextureUScroll() != 0 ||
                pTex->getTextureVScroll() != 0 )
            {
                writeAttribute(4, "scroll");
                writeValue(StringConverter::toString(pTex->getTextureUScroll()));
                writeValue(StringConverter::toString(pTex->getTextureVScroll()));
				individualTransformElems = true;
            }
            // scale
            if (mDefaults ||
                pTex->getTextureUScale() != 1.0 ||
                pTex->getTextureVScale() != 1.0 )
            {
                writeAttribute(4, "scale");
                writeValue(StringConverter::toString(pTex->getTextureUScale()));
                writeValue(StringConverter::toString(pTex->getTextureVScale()));
				individualTransformElems = true;
            }

			// free transform
			if (!individualTransformElems &&
				(mDefaults ||
				pTex->getTextureTransform() != Matrix4::IDENTITY))
			{
				writeAttribute(4, "transform");
				const Matrix4& xform = pTex->getTextureTransform();
				for (int row = 0; row < 4; ++row)
				{
					for (int col = 0; col < 4; ++col)
					{
						writeValue(StringConverter::toString(xform[row][col]));
					}
				}
			}

			// Used to store the u and v speeds of scroll animation effects
			float scrollAnimU = 0;
			float scrollAnimV = 0;

            EffectMap effMap = pTex->getEffects();
            if (!effMap.empty())
            {
                EffectMap::const_iterator it;
                for (it = effMap.begin(); it != effMap.end(); ++it)
                {
                    const TextureUnitState::TextureEffect& ef = it->second;
                    switch (ef.type)
                    {
                    case TextureUnitState::ET_ENVIRONMENT_MAP :
                        writeEnvironmentMapEffect(ef, pTex);
                        break;
                    case TextureUnitState::ET_ROTATE :
                        writeRotationEffect(ef, pTex);
                        break;
					case TextureUnitState::ET_UVSCROLL :
						scrollAnimU = scrollAnimV = ef.arg1;
						break;
                    case TextureUnitState::ET_USCROLL :
						scrollAnimU = ef.arg1;
						break;
					case TextureUnitState::ET_VSCROLL :
						scrollAnimV = ef.arg1;
                        break;
                    case TextureUnitState::ET_TRANSFORM :
                        writeTransformEffect(ef, pTex);
                        break;
                    default:
                        break;
                    }
                }
            }

			// u and v scroll animation speeds merged, if present serialize scroll_anim
			if(scrollAnimU || scrollAnimV) {
				TextureUnitState::TextureEffect texEffect;
				texEffect.arg1 = scrollAnimU;
				texEffect.arg2 = scrollAnimV;
				writeScrollEffect(texEffect, pTex);
			}

			// Binding type
			TextureUnitState::BindingType bt = pTex->getBindingType();
			if (mDefaults ||
				bt != TextureUnitState::BT_FRAGMENT)
			{
				writeAttribute(4, "binding_type");
				switch(bt)
				{
				case TextureUnitState::BT_FRAGMENT:
					writeValue("fragment");
					break;
				case TextureUnitState::BT_VERTEX:
					writeValue("vertex");
					break;
                case TextureUnitState::BT_GEOMETRY:
                case TextureUnitState::BT_TESSELLATION_DOMAIN:
                case TextureUnitState::BT_TESSELLATION_HULL:
                case TextureUnitState::BT_COMPUTE:
                    break;
				};
		
			}
			// Content type
			if (mDefaults ||
				pTex->getContentType() != TextureUnitState::CONTENT_NAMED)
			{
				writeAttribute(4, "content_type");
				switch(pTex->getContentType())
				{
				case TextureUnitState::CONTENT_NAMED:
					writeValue("named");
					break;
				case TextureUnitState::CONTENT_SHADOW:
					writeValue("shadow");
					break;
				case TextureUnitState::CONTENT_COMPOSITOR:
					writeValue("compositor");
					writeValue(quoteWord(pTex->getReferencedCompositorName()));
					writeValue(quoteWord(pTex->getReferencedTextureName()));
					writeValue(StringConverter::toString(pTex->getReferencedMRTIndex()));
					break;
				};
			}

			// Fire write end event.
			fireTextureUnitStateEvent(MSE_WRITE_END, skipWriting, pTex);
        }
        endSection(3);

		// Fire post section write event.
		fireTextureUnitStateEvent(MSE_POST_WRITE, skipWriting, pTex);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeEnvironmentMapEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "env_map");
        switch (effect.subtype)
        {
        case TextureUnitState::ENV_PLANAR:
            writeValue("planar");
            break;
        case TextureUnitState::ENV_CURVED:
            writeValue("spherical");
            break;
        case TextureUnitState::ENV_NORMAL:
            writeValue("cubic_normal");
            break;
        case TextureUnitState::ENV_REFLECTION:
            writeValue("cubic_reflection");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeRotationEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1)
        {
            writeAttribute(4, "rotate_anim");
            writeValue(StringConverter::toString(effect.arg1));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTransformEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "wave_xform");

        switch (effect.subtype)
        {
        case TextureUnitState::TT_ROTATE:
            writeValue("rotate");
            break;
        case TextureUnitState::TT_SCALE_U:
            writeValue("scale_x");
            break;
        case TextureUnitState::TT_SCALE_V:
            writeValue("scale_y");
            break;
        case TextureUnitState::TT_TRANSLATE_U:
            writeValue("scroll_x");
            break;
        case TextureUnitState::TT_TRANSLATE_V:
            writeValue("scroll_y");
            break;
        }

        switch (effect.waveType)
        {
        case WFT_INVERSE_SAWTOOTH:
            writeValue("inverse_sawtooth");
            break;
        case WFT_SAWTOOTH:
            writeValue("sawtooth");
            break;
        case WFT_SINE:
            writeValue("sine");
            break;
        case WFT_SQUARE:
            writeValue("square");
            break;
        case WFT_TRIANGLE:
            writeValue("triangle");
            break;
        case WFT_PWM:
            writeValue("pwm");
            break;
        }

        writeValue(StringConverter::toString(effect.base));
        writeValue(StringConverter::toString(effect.frequency));
        writeValue(StringConverter::toString(effect.phase));
        writeValue(StringConverter::toString(effect.amplitude));
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeScrollEffect(
		const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1 || effect.arg2)
        {
            writeAttribute(4, "scroll_anim");
            writeValue(StringConverter::toString(effect.arg1));
            writeValue(StringConverter::toString(effect.arg2));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf)
    {
        switch (sbf)
        {
        case SBF_DEST_ALPHA:
            writeValue("dest_alpha");
            break;
        case SBF_DEST_COLOUR:
            writeValue("dest_colour");
            break;
        case SBF_ONE:
            writeValue("one");
            break;
        case SBF_ONE_MINUS_DEST_ALPHA:
            writeValue("one_minus_dest_alpha");
            break;
        case SBF_ONE_MINUS_DEST_COLOUR:
            writeValue("one_minus_dest_colour");
            break;
        case SBF_ONE_MINUS_SOURCE_ALPHA:
            writeValue("one_minus_src_alpha");
            break;
        case SBF_ONE_MINUS_SOURCE_COLOUR:
            writeValue("one_minus_src_colour");
            break;
        case SBF_SOURCE_ALPHA:
            writeValue("src_alpha");
            break;
        case SBF_SOURCE_COLOUR:
            writeValue("src_colour");
            break;
        case SBF_ZERO:
            writeValue("zero");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dst)
    {
        if (sbf_src == SBF_ONE && sbf_dst == SBF_ONE )
            writeValue("add");
        else if (sbf_src == SBF_DEST_COLOUR && sbf_dst == SBF_ZERO)
            writeValue("modulate");
        else if (sbf_src == SBF_SOURCE_COLOUR && sbf_dst == SBF_ONE_MINUS_SOURCE_COLOUR)
            writeValue("colour_blend");
        else if (sbf_src == SBF_SOURCE_ALPHA && sbf_dst == SBF_ONE_MINUS_SOURCE_ALPHA)
            writeValue("alpha_blend");
        else
        {
            writeSceneBlendFactor(sbf_src);
            writeSceneBlendFactor(sbf_dst);
        }
    }
	//-----------------------------------------------------------------------
	void MaterialSerializer::writeSceneBlendFactor(
		const SceneBlendFactor c_src, const SceneBlendFactor c_dest, 
		const SceneBlendFactor a_src, const SceneBlendFactor a_dest)
	{
		writeSceneBlendFactor(c_src, c_dest);
		writeSceneBlendFactor(a_src, a_dest);
	}
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeCompareFunction(const CompareFunction cf)
    {
        switch (cf)
        {
        case CMPF_ALWAYS_FAIL:
            writeValue("always_fail");
            break;
        case CMPF_ALWAYS_PASS:
            writeValue("always_pass");
            break;
        case CMPF_EQUAL:
            writeValue("equal");
            break;
        case CMPF_GREATER:
            writeValue("greater");
            break;
        case CMPF_GREATER_EQUAL:
            writeValue("greater_equal");
            break;
        case CMPF_LESS:
            writeValue("less");
            break;
        case CMPF_LESS_EQUAL:
            writeValue("less_equal");
            break;
        case CMPF_NOT_EQUAL:
            writeValue("not_equal");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeColourValue(const ColourValue &colour, bool writeAlpha)
    {
        writeValue(StringConverter::toString(colour.r));
        writeValue(StringConverter::toString(colour.g));
        writeValue(StringConverter::toString(colour.b));
        if (writeAlpha)
            writeValue(StringConverter::toString(colour.a));
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLayerBlendOperationEx(const LayerBlendOperationEx op)
    {
        switch (op)
        {
        case LBX_ADD:
            writeValue("add");
            break;
        case LBX_ADD_SIGNED:
            writeValue("add_signed");
            break;
        case LBX_ADD_SMOOTH:
            writeValue("add_smooth");
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            writeValue("blend_current_alpha");
            break;
        case LBX_BLEND_DIFFUSE_COLOUR:
            writeValue("blend_diffuse_colour");
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            writeValue("blend_diffuse_alpha");
            break;
        case LBX_BLEND_MANUAL:
            writeValue("blend_manual");
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
            writeValue("blend_texture_alpha");
            break;
        case LBX_MODULATE:
            writeValue("modulate");
            break;
        case LBX_MODULATE_X2:
            writeValue("modulate_x2");
            break;
        case LBX_MODULATE_X4:
            writeValue("modulate_x4");
            break;
        case LBX_SOURCE1:
            writeValue("source1");
            break;
        case LBX_SOURCE2:
            writeValue("source2");
            break;
        case LBX_SUBTRACT:
            writeValue("subtract");
            break;
        case LBX_DOTPRODUCT:
            writeValue("dotproduct");
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLayerBlendSource(const LayerBlendSource lbs)
    {
        switch (lbs)
        {
        case LBS_CURRENT:
            writeValue("src_current");
            break;
        case LBS_DIFFUSE:
            writeValue("src_diffuse");
            break;
        case LBS_MANUAL:
            writeValue("src_manual");
            break;
        case LBS_SPECULAR:
            writeValue("src_specular");
            break;
        case LBS_TEXTURE:
            writeValue("src_texture");
            break;
        }
    }

    // nfz
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeVertexProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("vertex_program_ref",
            pPass->getVertexProgram(), pPass->getVertexProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowCasterVertexProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_caster_vertex_program_ref",
            pPass->getShadowCasterVertexProgram(), pPass->getShadowCasterVertexProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowCasterFragmentProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_caster_fragment_program_ref",
            pPass->getShadowCasterFragmentProgram(), pPass->getShadowCasterFragmentProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowReceiverVertexProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_receiver_vertex_program_ref",
            pPass->getShadowReceiverVertexProgram(), pPass->getShadowReceiverVertexProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeShadowReceiverFragmentProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("shadow_receiver_fragment_program_ref",
            pPass->getShadowReceiverFragmentProgram(), pPass->getShadowReceiverFragmentProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeFragmentProgramRef(const Pass* pPass)
    {
        writeGpuProgramRef("fragment_program_ref",
            pPass->getFragmentProgram(), pPass->getFragmentProgramParameters());
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuProgramRef(const String& attrib,
                                                const GpuProgramPtr& program, const GpuProgramParametersSharedPtr& params)
    {		
        bool skipWriting = false;

        // Fire pre-write event.
        fireGpuProgramRefEvent(MSE_PRE_WRITE, skipWriting, attrib, program, params, NULL);
        if (skipWriting)		
            return;

        mBuffer += "\n";
        writeAttribute(3, attrib);
        writeValue(quoteWord(program->getName()));
        beginSection(3);
        {
            // write out parameters
            GpuProgramParameters* defaultParams = 0;
            // does the GPU program have default parameters?
            if (program->hasDefaultParameters())
                defaultParams = program->getDefaultParameters().getPointer();

            // Fire write begin event.
            fireGpuProgramRefEvent(MSE_WRITE_BEGIN, skipWriting, attrib, program, params, defaultParams);

            writeGPUProgramParameters(params, defaultParams);

            // Fire write end event.
            fireGpuProgramRefEvent(MSE_WRITE_END, skipWriting, attrib, program, params, defaultParams);
        }
        endSection(3);

        // add to GpuProgram contatiner
        mGpuProgramDefinitionContainer.insert(program->getName());

        // Fire post section write event.
        fireGpuProgramRefEvent(MSE_POST_WRITE, skipWriting, attrib, program, params, NULL);		
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGPUProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        // iterate through the constant definitions
        if (params->hasNamedParameters())
        {
            writeNamedGpuProgramParameters(params, defaultParams, level, useMainBuffer);
        }
        else
        {
            writeLowLevelGpuProgramParameters(params, defaultParams, level, useMainBuffer);
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeNamedGpuProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        GpuConstantDefinitionIterator constIt = params->getConstantDefinitionIterator();
        while(constIt.hasMoreElements())
        {
            // get the constant definition
            const String& paramName = constIt.peekNextKey();
            const GpuConstantDefinition& def =
                constIt.getNext();

            // get any auto-link
            const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                params->findAutoConstantEntry(paramName);
            const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
            if (defaultParams)
            {
                defaultAutoEntry = 
                    defaultParams->findAutoConstantEntry(paramName);
            }

            writeGpuProgramParameter("param_named", 
                                     paramName, autoEntry, defaultAutoEntry, 
                                     def.isFloat(), def.isDouble(), def.isInt(), def.isUnsignedInt(),
                                     def.physicalIndex, def.elementSize * def.arraySize,
                                     params, defaultParams, level, useMainBuffer);
        }

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeLowLevelGpuProgramParameters(
        const GpuProgramParametersSharedPtr& params,
        GpuProgramParameters* defaultParams, unsigned short level,
        const bool useMainBuffer)
    {
        // Iterate over the logical->physical mappings
        // This will represent the values which have been set

        // float params
        GpuLogicalBufferStructPtr floatLogical = params->getFloatLogicalBufferStruct();
        if( !floatLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(floatLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin();
                i != floatLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findFloatAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findFloatAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, true, false, false, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }
        }

        // double params
        GpuLogicalBufferStructPtr doubleLogical = params->getDoubleLogicalBufferStruct();
        if( !doubleLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(floatLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = doubleLogical->map.begin();
                i != doubleLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry =
                    params->findDoubleAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findDoubleAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed",
                                         StringConverter::toString(logicalIndex), autoEntry,
                                         defaultAutoEntry, false, true, false, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }
        }

        // int params
        GpuLogicalBufferStructPtr intLogical = params->getIntLogicalBufferStruct();
        if( !intLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(intLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = intLogical->map.begin();
                i != intLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findIntAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findIntAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, false, false, true, false,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }

        }

        // uint params
        GpuLogicalBufferStructPtr uintLogical = params->getUnsignedIntLogicalBufferStruct();
        if( !uintLogical.isNull() )
        {
            OGRE_LOCK_MUTEX(uintLogical->mutex);

            for(GpuLogicalIndexUseMap::const_iterator i = uintLogical->map.begin();
                i != uintLogical->map.end(); ++i)
            {
                size_t logicalIndex = i->first;
                const GpuLogicalIndexUse& logicalUse = i->second;

                const GpuProgramParameters::AutoConstantEntry* autoEntry = 
                    params->findUnsignedIntAutoConstantEntry(logicalIndex);
                const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
                if (defaultParams)
                {
                    defaultAutoEntry = defaultParams->findUnsignedIntAutoConstantEntry(logicalIndex);
                }

                writeGpuProgramParameter("param_indexed", 
                                         StringConverter::toString(logicalIndex), autoEntry, 
                                         defaultAutoEntry, false, false, false, true,
                                         logicalUse.physicalIndex, logicalUse.currentSize,
                                         params, defaultParams, level, useMainBuffer);
            }

        }

        // // bool params
        // GpuLogicalBufferStructPtr boolLogical = params->getBoolLogicalBufferStruct();
        // if( !boolLogical.isNull() )
        // {
        //     OGRE_LOCK_MUTEX(boolLogical->mutex);

        //     for(GpuLogicalIndexUseMap::const_iterator i = boolLogical->map.begin();
        //         i != boolLogical->map.end(); ++i)
        //     {
        //         size_t logicalIndex = i->first;
        //         const GpuLogicalIndexUse& logicalUse = i->second;

        //         const GpuProgramParameters::AutoConstantEntry* autoEntry = 
        //             params->findBoolAutoConstantEntry(logicalIndex);
        //         const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry = 0;
        //         if (defaultParams)
        //         {
        //             defaultAutoEntry = defaultParams->findBoolAutoConstantEntry(logicalIndex);
        //         }

        //         writeGpuProgramParameter("param_indexed", 
        //                                  StringConverter::toString(logicalIndex), autoEntry, 
        //                                  defaultAutoEntry, false, false, false, false,
        //                                  logicalUse.physicalIndex, logicalUse.currentSize,
        //                                  params, defaultParams, level, useMainBuffer);
        //     }

        // }

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuProgramParameter(
        const String& commandName, const String& identifier, 
        const GpuProgramParameters::AutoConstantEntry* autoEntry, 
        const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry, 
        bool isFloat, bool isDouble, bool isInt, bool isUnsignedInt,
        size_t physicalIndex, size_t physicalSize,
        const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
        const ushort level, const bool useMainBuffer)
    {
        // Skip any params with array qualifiers
        // These are only for convenience of setters, the full array will be
        // written using the base, non-array identifier
        if (identifier.find("[") != String::npos)
        {
            return;
        }

        // get any auto-link
        // don't duplicate constants that are defined as a default parameter
        bool different = false;
        if (defaultParams)
        {
            // if default is auto but we're not or vice versa
            if ((autoEntry == 0) != (defaultAutoEntry == 0))
            {
                different = true;
            }
            else if (autoEntry)
            {
                // both must be auto
                // compare the auto values
                different = (autoEntry->paramType != defaultAutoEntry->paramType
                             || autoEntry->data != defaultAutoEntry->data);
            }
            else
            {
                // compare the non-auto (raw buffer) values
                // param buffers are always initialised with all zeros
                // so unset == unset
                if (isFloat)
                {
                    different = memcmp(
                        params->getFloatPointer(physicalIndex), 
                        defaultParams->getFloatPointer(physicalIndex),
                        sizeof(float) * physicalSize) != 0;
                }
                else if (isDouble)
                {
                    different = memcmp(
                        params->getDoublePointer(physicalIndex),
                        defaultParams->getDoublePointer(physicalIndex),
                        sizeof(double) * physicalSize) != 0;
                }
                else if (isInt)
                {
                    different = memcmp(
                        params->getIntPointer(physicalIndex), 
                        defaultParams->getIntPointer(physicalIndex),
                        sizeof(int) * physicalSize) != 0;
                }
                else if (isUnsignedInt)
                {
                    different = memcmp(
                        params->getUnsignedIntPointer(physicalIndex),
                        defaultParams->getUnsignedIntPointer(physicalIndex),
                        sizeof(uint) * physicalSize) != 0;
                }
                else //if (isBool)
                {
                    // different = memcmp(
                    //     params->getBoolPointer(physicalIndex), 
                    //     defaultParams->getBoolPointer(physicalIndex),
                    //     sizeof(bool) * physicalSize) != 0;
                    different = memcmp(
                        params->getUnsignedIntPointer(physicalIndex), 
                        defaultParams->getUnsignedIntPointer(physicalIndex),
                        sizeof(uint) * physicalSize) != 0;
                }
            }
        }

        if (!defaultParams || different)
        {
            String label = commandName;

            // is it auto
            if (autoEntry)
                label += "_auto";

            writeAttribute(level, label, useMainBuffer);
            // output param index / name
            writeValue(quoteWord(identifier), useMainBuffer);

            // if auto output auto type name and data if needed
            if (autoEntry)
            {
                const GpuProgramParameters::AutoConstantDefinition* autoConstDef =
                    GpuProgramParameters::getAutoConstantDefinition(autoEntry->paramType);

                assert(autoConstDef && "Bad auto constant Definition Table");
                // output auto constant name
                writeValue(quoteWord(autoConstDef->name), useMainBuffer);
                // output data if it uses it
                switch(autoConstDef->dataType)
                {
                case GpuProgramParameters::ACDT_REAL:
                    writeValue(StringConverter::toString(autoEntry->fData), useMainBuffer);
                    break;

                case GpuProgramParameters::ACDT_INT:
                    writeValue(StringConverter::toString(autoEntry->data), useMainBuffer);
                    break;

                default:
                    break;
                }
            }
            else // not auto so output all the values used
            {
                String countLabel;

                // only write a number if > 1
                if (physicalSize > 1)
                    countLabel = StringConverter::toString(physicalSize);

                if (isFloat)
                {
                    // Get pointer to start of values
                    const float* pFloat = params->getFloatPointer(physicalIndex);

                    writeValue("float" + countLabel, useMainBuffer);
                    // iterate through real constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pFloat++), useMainBuffer);
                    }
                }
                else if (isDouble)
                {
                    // Get pointer to start of values
                    const double* pDouble = params->getDoublePointer(physicalIndex);

                    writeValue("double" + countLabel, useMainBuffer);
                    // iterate through dobule constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pDouble++), useMainBuffer);
                    }
                }
                else if (isInt)
                {
                    // Get pointer to start of values
                    const int* pInt = params->getIntPointer(physicalIndex);

                    writeValue("int" + countLabel, useMainBuffer);
                    // iterate through int constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pInt++), useMainBuffer);
                    }
                }
                else if (isUnsignedInt) 
                {
                    // Get pointer to start of values
                    const uint* pUInt = params->getUnsignedIntPointer(physicalIndex);

                    writeValue("uint" + countLabel, useMainBuffer);
                    // iterate through uint constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pUInt++), useMainBuffer);
                    }
                }
                else //if (isBool)
                {
                    // Get pointer to start of values
                    // const bool* pBool = params->getBoolPointer(physicalIndex);
                    const uint* pBool = params->getUnsignedIntPointer(physicalIndex);

                    writeValue("bool" + countLabel, useMainBuffer);
                    // iterate through bool constants
                    for (size_t f = 0; f < physicalSize; ++f)
                    {
                        writeValue(StringConverter::toString(*pBool++), useMainBuffer);
                    }
                }//// end if (float/int)

            }

        }

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeGpuPrograms(void)
    {
        // iterate through gpu program names in container
        GpuProgramDefIterator currentDef = mGpuProgramDefinitionContainer.begin();
        GpuProgramDefIterator endDef = mGpuProgramDefinitionContainer.end();

        while (currentDef != endDef)
        {
            // get gpu program from gpu program manager
            GpuProgramPtr program = GpuProgramManager::getSingleton().getByName((*currentDef));
            // write gpu program definition type to buffer
            // check program type for vertex program
            // write program type
            mGpuProgramBuffer += "\n";
            writeAttribute(0, program->getParameter("type"), false);

            // write program name
            writeValue( quoteWord(program->getName()), false);
            // write program language
            const String language = program->getLanguage();
            writeValue( language, false );
            // write opening braces
            beginSection(0, false);
            {
                // write program source + filename
                writeAttribute(1, "source", false);
                writeValue(quoteWord(program->getSourceFile()), false);
                // write special parameters based on language
                const ParameterList& params = program->getParameters();
                ParameterList::const_iterator currentParam = params.begin();
                ParameterList::const_iterator endParam = params.end();

                while (currentParam != endParam)
                {
                    if (currentParam->name != "type" &&
                        currentParam->name != "assemble_code" &&
                        currentParam->name != "micro_code" &&
                        currentParam->name != "external_micro_code")
                    {
                        String paramstr = program->getParameter(currentParam->name);
                        if ((currentParam->name == "includes_skeletal_animation")
                            && (paramstr == "false"))
                            paramstr.clear();
                        if ((currentParam->name == "includes_morph_animation")
                            && (paramstr == "false"))
                            paramstr.clear();
                        if ((currentParam->name == "includes_pose_animation")
                            && (paramstr == "0"))
                            paramstr.clear();
                        if ((currentParam->name == "uses_vertex_texture_fetch")
                            && (paramstr == "false"))
                            paramstr.clear();

                        if ((language != "asm") && (currentParam->name == "syntax"))
                            paramstr.clear();

                        if (!paramstr.empty())
                        {
                            writeAttribute(1, currentParam->name, false);
                            writeValue(paramstr, false);
                        }
                    }
                    ++currentParam;
                }

                // write default parameters
                if (program->hasDefaultParameters())
                {
                    mGpuProgramBuffer += "\n";
                    GpuProgramParametersSharedPtr gpuDefaultParams = program->getDefaultParameters();
                    writeAttribute(1, "default_params", false);
                    beginSection(1, false);
                    writeGPUProgramParameters(gpuDefaultParams, 0, 2, false);
                    endSection(1, false);
                }
            }
            // write closing braces
            endSection(0, false);

            ++currentDef;

        }

        mGpuProgramBuffer += "\n";
    }

	//---------------------------------------------------------------------
	void MaterialSerializer::addListener(Listener* listener)
	{
		mListeners.push_back(listener);
	}

	//---------------------------------------------------------------------
	void MaterialSerializer::removeListener(Listener* listener)
	{
		ListenerListIterator i, iend;
		iend = mListeners.end();
		for (i = mListeners.begin(); i != iend; ++i)
		{
			if (*i == listener)
			{
				mListeners.erase(i);
				break;
			}
		}
	}

	//---------------------------------------------------------------------
	void MaterialSerializer::fireMaterialEvent(SerializeEvent event, bool& skip, const Material* mat)
	{
		ListenerListIterator it	   = mListeners.begin();
		ListenerListIterator itEnd = mListeners.end();

		while (it != itEnd)
		{
			(*it)->materialEventRaised(this, event, skip, mat);			
			if (skip)
				break;
			++it;
		}		
	}

	//---------------------------------------------------------------------
	void MaterialSerializer::fireTechniqueEvent(SerializeEvent event, bool& skip, const Technique* tech)
	{
		ListenerListIterator it	   = mListeners.begin();
		ListenerListIterator itEnd = mListeners.end();

		while (it != itEnd)
		{
			(*it)->techniqueEventRaised(this, event, skip, tech);
			if (skip)
				break;
			++it;
		}
	}

	//---------------------------------------------------------------------
	void MaterialSerializer::firePassEvent(SerializeEvent event, bool& skip, const Pass* pass)
	{
		ListenerListIterator it	   = mListeners.begin();
		ListenerListIterator itEnd = mListeners.end();

		while (it != itEnd)
		{
			(*it)->passEventRaised(this, event, skip, pass);
			if (skip)
				break;
			++it;
		}
	}

	//---------------------------------------------------------------------
	void MaterialSerializer::fireGpuProgramRefEvent(SerializeEvent event, bool& skip,
		const String& attrib, 
		const GpuProgramPtr& program, 
		const GpuProgramParametersSharedPtr& params,
		GpuProgramParameters* defaultParams)
	{
		ListenerListIterator it	   = mListeners.begin();
		ListenerListIterator itEnd = mListeners.end();

		while (it != itEnd)
		{
			(*it)->gpuProgramRefEventRaised(this, event, skip, attrib, program, params, defaultParams);
			if (skip)
				break;
			++it;
		}
	}	

	//---------------------------------------------------------------------
	void MaterialSerializer::fireTextureUnitStateEvent(SerializeEvent event, bool& skip,
		const TextureUnitState* textureUnit)
	{
		ListenerListIterator it	   = mListeners.begin();
		ListenerListIterator itEnd = mListeners.end();

		while (it != itEnd)
		{
			(*it)->textureUnitStateEventRaised(this, event, skip, textureUnit);
			if (skip)
				break;
			++it;
		}
	}	
}
