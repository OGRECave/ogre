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
    size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
    for (size_t i = 0; i < numAttribs; ++i)
    {
        if (msCustomAttributes[i].name == type)
            return msCustomAttributes[i].semantic;
    }

    OgreAssertDbg(false, "Missing attribute!");
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
}

const char* GLSLProgramCommon::getAttributeSemanticString(VertexElementSemantic semantic)
{
    if(semantic == VES_TEXTURE_COORDINATES)
        return "uv";

    size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
    for (size_t i = 0; i < numAttribs; ++i)
    {
        if (msCustomAttributes[i].semantic == semantic)
            return msCustomAttributes[i].name;
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
            semantic = getAttributeSemanticEnum("uv0"); // treat "uvXY" as "uv0"
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

// Some drivers (e.g. OS X on nvidia) incorrectly determine the attribute binding automatically
// and end up aliasing existing built-ins. So avoid! Fixed builtins are:

//  a  builtin              custom attrib name
// ----------------------------------------------
//  0  gl_Vertex            vertex
//  1  n/a                  blendWeights
//  2  gl_Normal            normal
//  3  gl_Color             colour
//  4  gl_SecondaryColor    secondary_colour
//  5  gl_FogCoord          fog_coord
//  7  n/a                  blendIndices
//  8  gl_MultiTexCoord0    uv0
//  9  gl_MultiTexCoord1    uv1
//  10 gl_MultiTexCoord2    uv2
//  11 gl_MultiTexCoord3    uv3
//  12 gl_MultiTexCoord4    uv4
//  13 gl_MultiTexCoord5    uv5
//  14 gl_MultiTexCoord6    uv6, tangent
//  15 gl_MultiTexCoord7    uv7, binormal
GLSLProgramCommon::CustomAttribute GLSLProgramCommon::msCustomAttributes[16] = {
    {"vertex", getFixedAttributeIndex(VES_POSITION, 0), VES_POSITION},
    {"blendWeights", getFixedAttributeIndex(VES_BLEND_WEIGHTS, 0), VES_BLEND_WEIGHTS},
    {"normal", getFixedAttributeIndex(VES_NORMAL, 0), VES_NORMAL},
    {"colour", getFixedAttributeIndex(VES_DIFFUSE, 0), VES_DIFFUSE},
    {"secondary_colour", getFixedAttributeIndex(VES_SPECULAR, 0), VES_SPECULAR},
    {"blendIndices", getFixedAttributeIndex(VES_BLEND_INDICES, 0), VES_BLEND_INDICES},
    {"uv0", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 0), VES_TEXTURE_COORDINATES},
    {"uv1", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 1), VES_TEXTURE_COORDINATES},
    {"uv2", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 2), VES_TEXTURE_COORDINATES},
    {"uv3", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 3), VES_TEXTURE_COORDINATES},
    {"uv4", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 4), VES_TEXTURE_COORDINATES},
    {"uv5", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 5), VES_TEXTURE_COORDINATES},
    {"uv6", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 6), VES_TEXTURE_COORDINATES},
    {"uv7", getFixedAttributeIndex(VES_TEXTURE_COORDINATES, 7), VES_TEXTURE_COORDINATES},
    {"tangent", getFixedAttributeIndex(VES_TANGENT, 0), VES_TANGENT},
    {"binormal", getFixedAttributeIndex(VES_BINORMAL, 0), VES_BINORMAL},
};

uint32 GLSLProgramCommon::getFixedAttributeIndex(VertexElementSemantic semantic, uint index)
{
    switch(semantic)
    {
    case VES_POSITION:
        return 0;
    case VES_BLEND_WEIGHTS:
        return 1;
    case VES_NORMAL:
        return 2;
    case VES_DIFFUSE:
        return 3;
    case VES_SPECULAR:
        return 4;
    case VES_BLEND_INDICES:
        return 7;
    case VES_TEXTURE_COORDINATES:
        return 8 + index;
    case VES_TANGENT:
        return 14;
    case VES_BINORMAL:
        return 15;
    default:
        OgreAssertDbg(false, "Missing attribute!");
        return 0;
    };
}
} /* namespace Ogre */
