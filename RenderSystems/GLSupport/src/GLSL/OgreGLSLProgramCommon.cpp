/*
 * OgreGLSLProgramCommon.cpp
 *
 *  Created on: 15.01.2017
 *      Author: pavel
 */

#include "OgreGLSLProgramCommon.h"
#include "OgreStringConverter.h"

namespace Ogre
{
VertexElementSemantic GLSLProgramCommon::getAttributeSemanticEnum(String type)
{
    VertexElementSemantic semantic = mSemanticTypeMap[type];
    if (semantic > 0)
    {
        return semantic;
    }
    else
    {
        OgreAssertDbg(false, "Missing attribute!");
        return (VertexElementSemantic)0;
    }
}

GLSLProgramCommon::GLSLProgramCommon(GLSLShaderCommon* vertexShader)
    : mVertexShader(vertexShader),
      mUniformRefsBuilt(false),
      mGLProgramHandle(0),
      mLinked(false),
      mTriedToLinkAndFailed(false),
      mSkeletalAnimation(false)
{
    // init mCustomAttributesIndexes
    for (size_t i = 0; i < VES_COUNT; i++)
    {
        for (size_t j = 0; j < OGRE_MAX_TEXTURE_COORD_SETS; j++)
        {
            mCustomAttributesIndexes[i][j] = NULL_CUSTOM_ATTRIBUTES_INDEX;
        }
    }

    // Initialize the attribute to semantic map
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("vertex", VES_POSITION));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("blendWeights", VES_BLEND_WEIGHTS));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("normal", VES_NORMAL));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("colour", VES_DIFFUSE));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("secondary_colour", VES_SPECULAR));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("blendIndices", VES_BLEND_INDICES));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("tangent", VES_TANGENT));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("binormal", VES_BINORMAL));
    mSemanticTypeMap.insert(SemanticToStringMap::value_type("uv", VES_TEXTURE_COORDINATES));
}

const char* GLSLProgramCommon::getAttributeSemanticString(VertexElementSemantic semantic)
{
    for (SemanticToStringMap::iterator i = mSemanticTypeMap.begin(); i != mSemanticTypeMap.end();
         ++i)
    {
        if ((*i).second == semantic)
            return (*i).first.c_str();
    }

    OgreAssertDbg(false, "Missing attribute!");
    return 0;
}

void GLSLProgramCommon::extractLayoutQualifiers(void)
{
    // Format is:
    //      layout(location = 0) attribute vec4 vertex;

    if (!mVertexShader)
    {
        return;
    }

    String shaderSource = mVertexShader->getSource();
    String::size_type currPos = shaderSource.find("layout");
    while (currPos != String::npos)
    {
        VertexElementSemantic semantic;
        int index = 0;

        String::size_type endPos = shaderSource.find(";", currPos);
        if (endPos == String::npos)
        {
            // Problem, missing semicolon, abort.
            break;
        }

        String line = shaderSource.substr(currPos, endPos - currPos);

        // Skip over 'layout'.
        currPos += 6;

        // Skip until '='.
        String::size_type eqPos = line.find("=");
        String::size_type parenPos = line.find(")");

        // Skip past '=' up to a ')' which contains an integer(the position).
        // TODO This could be a definition, does the preprocessor do replacement?
        String attrLocation = line.substr(eqPos + 1, parenPos - eqPos - 1);
        StringUtil::trim(attrLocation);
        int attrib = StringConverter::parseInt(attrLocation);

        // The rest of the line is a standard attribute definition.
        // Erase up to it then split the remainder by spaces.
        line.erase(0, parenPos + 1);
        StringUtil::trim(line);
        StringVector parts = StringUtil::split(line, " ");

        if (parts.size() < 3)
        {
            // This is a malformed attribute.
            // It should contain 3 parts, i.e. "attribute vec4 vertex".
            break;
        }
        if (parts[0] == "out")
        {
            // This is an output attribute, skip it
            continue;
        }

        String attrName = parts[2];
        String::size_type uvPos = attrName.find("uv");

        // Special case for attribute named position.
        if (attrName == "position")
            semantic = getAttributeSemanticEnum("vertex");
        else if(uvPos == 0)
            semantic = getAttributeSemanticEnum("uv"); // treat "uvXY" as "uv"
        else
            semantic = getAttributeSemanticEnum(attrName);

        // Find the texture unit index.
        if (uvPos == 0)
        {
            String uvIndex = attrName.substr(uvPos + 2, attrName.length() - 2);
            index = StringConverter::parseInt(uvIndex);
        }

        mCustomAttributesIndexes[semantic - 1][index] = attrib;

        currPos = shaderSource.find("layout", currPos);
    }
}
} /* namespace Ogre */
