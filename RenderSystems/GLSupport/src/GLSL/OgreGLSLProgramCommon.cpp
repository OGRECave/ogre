// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#include "OgreGLSLProgramCommon.h"
#include "OgreStringConverter.h"

namespace Ogre
{
VertexElementSemantic GLSLProgramCommon::getAttributeSemanticEnum(const String& type)
{
    size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
    for (size_t i = 0; i < numAttribs; ++i)
    {
        if (msCustomAttributes[i].name == type)
            return msCustomAttributes[i].semantic;
    }

    OgreAssertDbg(false, "Missing attribute!");
    return VertexElementSemantic(0);
}

GLSLProgramCommon::GLSLProgramCommon(const GLShaderList& shaders)
    : mShaders(shaders),
      mUniformRefsBuilt(false),
      mGLProgramHandle(0),
      mLinked(false)
{
    // compute shader presence means no other shaders are allowed
    if(shaders[GPT_COMPUTE_PROGRAM])
    {
        mShaders.fill(NULL);
        mShaders[GPT_COMPUTE_PROGRAM] = shaders[GPT_COMPUTE_PROGRAM];
    }

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

    if (!mShaders[GPT_VERTEX_PROGRAM])
    {
        return;
    }

    String shaderSource = mShaders[GPT_VERTEX_PROGRAM]->getSource();
    String::size_type currPos = shaderSource.find("layout");
    while (currPos != String::npos)
    {
        VertexElementSemantic semantic;
        int index = 0;

        String::size_type endPos = shaderSource.find(';', currPos);
        if (endPos == String::npos)
        {
            // Problem, missing semicolon, abort.
            break;
        }

        String line = shaderSource.substr(currPos, endPos - currPos);

        // Skip over 'layout'.
        currPos += 6;

        // Skip until '='.
        String::size_type eqPos = line.find('=');
        String::size_type parenPos = line.find(')');

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
        size_t attrStart = 0;
        if (parts.size() == 4)
        {
            if( parts[0] == "flat" || parts[0] == "smooth"|| parts[0] == "perspective")
            {
                // Skip the interpolation qualifier
                attrStart = 1;
            }
        }

        // Skip output attribute
        if (parts[attrStart] != "out")
        {
            String attrName = parts[attrStart + 2];
            String::size_type uvPos = attrName.find("uv");

            if(uvPos == 0)
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
        }

        currPos = shaderSource.find("layout", currPos);
    }
}

// Switching attribute bindings requires re-creating VAOs. So avoid!
// Fixed builtins (from ARB_vertex_program Table X.2) are:

//  a  builtin              custom attrib name
// ----------------------------------------------
//  0  gl_Vertex            vertex/ position
//  1  n/a                  blendWeights
//  2  gl_Normal            normal
//  3  gl_Color             colour
//  4  gl_SecondaryColor    secondary_colour
//  5  gl_FogCoord          n/a
//  6  n/a                  n/a
//  7  n/a                  blendIndices
//  8  gl_MultiTexCoord0    uv0
//  9  gl_MultiTexCoord1    uv1
//  10 gl_MultiTexCoord2    uv2
//  11 gl_MultiTexCoord3    uv3
//  12 gl_MultiTexCoord4    uv4
//  13 gl_MultiTexCoord5    uv5
//  14 gl_MultiTexCoord6    uv6, tangent
//  15 gl_MultiTexCoord7    uv7, binormal
GLSLProgramCommon::CustomAttribute GLSLProgramCommon::msCustomAttributes[17] = {
    {"vertex", getFixedAttributeIndex(VES_POSITION, 0), VES_POSITION},
    {"position", getFixedAttributeIndex(VES_POSITION, 0), VES_POSITION}, // allow alias for "vertex"
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

static int32 attributeIndex[VES_COUNT + 1] = {
        -1,// n/a
        0, // VES_POSITION
        1, // VES_BLEND_WEIGHTS
        7, // VES_BLEND_INDICES
        2, // VES_NORMAL
        3, // VES_DIFFUSE
        4, // VES_SPECULAR
        8, // VES_TEXTURE_COORDINATES
        15,// VES_BINORMAL
        14 // VES_TANGENT
};

void GLSLProgramCommon::useTightAttributeLayout() {
    //  a  builtin              custom attrib name
    // ----------------------------------------------
    //  0  gl_Vertex            vertex/ position
    //  1  gl_Normal            normal
    //  2  gl_Color             colour
    //  3  gl_MultiTexCoord0    uv0
    //  4  gl_MultiTexCoord1    uv1, blendWeights
    //  5  gl_MultiTexCoord2    uv2, blendIndices
    //  6  gl_MultiTexCoord3    uv3, tangent
    //  7  gl_MultiTexCoord4    uv4, binormal

    size_t numAttribs = sizeof(msCustomAttributes)/sizeof(CustomAttribute);
    for (size_t i = 0; i < numAttribs; ++i)
    {
        CustomAttribute& a = msCustomAttributes[i];
        a.attrib -= attributeIndex[a.semantic]; // only keep index (for uvXY)
    }

    attributeIndex[VES_NORMAL] = 1;
    attributeIndex[VES_DIFFUSE] = 2;
    attributeIndex[VES_TEXTURE_COORDINATES] = 3;
    attributeIndex[VES_BLEND_WEIGHTS] = 4;
    attributeIndex[VES_BLEND_INDICES] = 5;
    attributeIndex[VES_TANGENT] = 6;
    attributeIndex[VES_BINORMAL] = 7;

    for (size_t i = 0; i < numAttribs; ++i)
    {
        CustomAttribute& a = msCustomAttributes[i];
        a.attrib += attributeIndex[a.semantic];
    }
}

int32 GLSLProgramCommon::getFixedAttributeIndex(VertexElementSemantic semantic, uint index)
{
    OgreAssertDbg(semantic > 0 && semantic <= VES_COUNT, "Missing attribute!");

    if(semantic == VES_TEXTURE_COORDINATES)
        return attributeIndex[semantic] + index;

    return attributeIndex[semantic];
}

String GLSLProgramCommon::getCombinedName()
{
    StringStream ss;

    for(auto s : mShaders)
    {
        if (s)
        {
            ss << s->getName() << "\n";
        }
    }

    return ss.str();
}

uint32 GLSLProgramCommon::getCombinedHash()
{
    uint32 hash = 0;

    for (auto p : mShaders)
    {
        if(!p) continue;
        hash = p->_getHash(hash);
    }
    return hash;
}

} /* namespace Ogre */
