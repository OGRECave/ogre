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

#include "OgreStableHeaders.h"
#include "OgreBuiltinScriptTranslators.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreDepthBuffer.h"
#include "OgreParticleSystem.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramUsage.h"

namespace Ogre{
    static void applyTextureAliases(ScriptCompiler *compiler, const Material* mat, const NameValuePairList& aliasList)
    {
        for (auto t : mat->getTechniques())
        {
            for (auto p : t->getPasses())
            {
                for (auto tus : p->getTextureUnitStates())
                {
                    auto aliasIt = aliasList.find(tus->getName());
                    if (aliasIt == aliasList.end())
                        continue;

                    ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, aliasIt->second);
                    compiler->_fireEvent(&evt, 0);

                    if (tus->getNumFrames() > 1)
                        tus->setAnimatedTextureName(evt.mName, tus->getNumFrames(),
                                                    tus->getAnimationDuration());
                    else
                        tus->setTextureName(evt.mName, tus->getTextureType());
                }
            }
        }
    }

    static GpuProgramType translateIDToGpuProgramType(uint32 id)
    {
        switch (id)
        {
        case ID_VERTEX_PROGRAM:
        default:
            return GPT_VERTEX_PROGRAM;
        case ID_GEOMETRY_PROGRAM:
            return GPT_GEOMETRY_PROGRAM;
        case ID_FRAGMENT_PROGRAM:
            return GPT_FRAGMENT_PROGRAM;
            case ID_TESSELLATION_HULL_PROGRAM:
            return GPT_HULL_PROGRAM;
            case ID_TESSELLATION_DOMAIN_PROGRAM:
            return GPT_DOMAIN_PROGRAM;
        case ID_COMPUTE_PROGRAM:
            return GPT_COMPUTE_PROGRAM;
        }
    }

    String getPropertyName(const ScriptCompiler *compiler, uint32 id)
    {
        for(auto& kv : compiler->mIds)
            if(kv.second == id)
                return kv.first;
        OgreAssertDbg(false,  "should not get here");
        return "unknown";
    }

    template <typename T>
    bool getValue(const AbstractNodePtr &node, T& result);
    template<> bool getValue(const AbstractNodePtr &node, float& result)
    {
        return StringConverter::parse(node->getString(), result);
    }
    template<> bool getValue(const AbstractNodePtr &node, double& result)
    {
        return StringConverter::parse(node->getString(), result);
    }
    template<> bool getValue(const AbstractNodePtr &node, bool& result)
    {
        return ScriptTranslator::getBoolean(node, &result);
    }
    template<> bool getValue(const AbstractNodePtr &node, uint32& result)
    {
        return StringConverter::parse(node->getString(), result);
    }
    template<> bool getValue(const AbstractNodePtr &node, int32& result)
    {
        return StringConverter::parse(node->getString(), result);
    }
    template<> bool getValue(const AbstractNodePtr &node, String& result)
    {
        return ScriptTranslator::getString(node, &result);
    }

    template<> bool getValue(const AbstractNodePtr& node, CompareFunction& result)
    {
        return ScriptTranslator::getCompareFunction(node, &result);
    }

    template<> bool getValue(const AbstractNodePtr& node, StencilOperation& result)
    {
        return ScriptTranslator::getStencilOp(node, &result);
    }

    template<> bool getValue(const AbstractNodePtr& node, IlluminationStage& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_AMBIENT:
            result = IS_AMBIENT;
            return true;
        case ID_PER_LIGHT:
            result = IS_PER_LIGHT;
            return true;
        case ID_DECAL:
            result = IS_DECAL;
            return true;
        default:
            return false;
        }
    }

    template<> bool getValue(const AbstractNodePtr& node, SceneBlendType& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_ADD:
            result = SBT_ADD;
            break;
        case ID_MODULATE:
            result = SBT_MODULATE;
            break;
        case ID_COLOUR_BLEND:
            result = SBT_TRANSPARENT_COLOUR;
            break;
        case ID_ALPHA_BLEND:
            result = SBT_TRANSPARENT_ALPHA;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, SceneBlendOperation& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_ADD:
            result = SBO_ADD;
            break;
        case ID_SUBTRACT:
            result = SBO_SUBTRACT;
            break;
        case ID_REVERSE_SUBTRACT:
            result = SBO_REVERSE_SUBTRACT;
            break;
        case ID_MIN:
            result = SBO_MIN;
            break;
        case ID_MAX:
            result = SBO_MAX;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, CullingMode& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_CLOCKWISE:
            result = CULL_CLOCKWISE;
            break;
        case ID_ANTICLOCKWISE:
            result = CULL_ANTICLOCKWISE;
            break;
        case ID_NONE:
            result = CULL_NONE;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, ManualCullingMode& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_FRONT:
            result = MANUAL_CULL_FRONT;
            break;
        case ID_BACK:
            result = MANUAL_CULL_BACK;
            break;
        case ID_NONE:
            result = MANUAL_CULL_NONE;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, ShadeOptions& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_FLAT:
            result = SO_FLAT;
            break;
        case ID_GOURAUD:
            result = SO_GOURAUD;
            break;
        case ID_PHONG:
            result = SO_PHONG;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, PolygonMode& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_SOLID:
            result = PM_SOLID;
            break;
        case ID_POINTS:
            result = PM_POINTS;
            break;
        case ID_WIREFRAME:
            result = PM_WIREFRAME;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, LayerBlendOperation& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_REPLACE:
            result = LBO_REPLACE;
            break;
        case ID_ADD:
            result = LBO_ADD;
            break;
        case ID_MODULATE:
            result = LBO_MODULATE;
            break;
        case ID_ALPHA_BLEND:
            result = LBO_ALPHA_BLEND;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, LayerBlendOperationEx& op)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_SOURCE1:
            op = LBX_SOURCE1;
            break;
        case ID_SOURCE2:
            op = LBX_SOURCE2;
            break;
        case ID_MODULATE:
            op = LBX_MODULATE;
            break;
        case ID_MODULATE_X2:
            op = LBX_MODULATE_X2;
            break;
        case ID_MODULATE_X4:
            op = LBX_MODULATE_X4;
            break;
        case ID_ADD:
            op = LBX_ADD;
            break;
        case ID_ADD_SIGNED:
            op = LBX_ADD_SIGNED;
            break;
        case ID_ADD_SMOOTH:
            op = LBX_ADD_SMOOTH;
            break;
        case ID_SUBTRACT:
            op = LBX_SUBTRACT;
            break;
        case ID_BLEND_DIFFUSE_ALPHA:
            op = LBX_BLEND_DIFFUSE_ALPHA;
            break;
        case ID_BLEND_TEXTURE_ALPHA:
            op = LBX_BLEND_TEXTURE_ALPHA;
            break;
        case ID_BLEND_CURRENT_ALPHA:
            op = LBX_BLEND_CURRENT_ALPHA;
            break;
        case ID_BLEND_MANUAL:
            op = LBX_BLEND_MANUAL;
            break;
        case ID_DOT_PRODUCT:
            op = LBX_DOTPRODUCT;
            break;
        case ID_BLEND_DIFFUSE_COLOUR:
            op = LBX_BLEND_DIFFUSE_COLOUR;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, LayerBlendSource& source1)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_SRC_CURRENT:
            source1 = LBS_CURRENT;
            break;
        case ID_SRC_TEXTURE:
            source1 = LBS_TEXTURE;
            break;
        case ID_SRC_DIFFUSE:
            source1 = LBS_DIFFUSE;
            break;
        case ID_SRC_SPECULAR:
            source1 = LBS_SPECULAR;
            break;
        case ID_SRC_MANUAL:
            source1 = LBS_MANUAL;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, CompositionTargetPass::InputMode& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_NONE:
            result = CompositionTargetPass::IM_NONE;
            break;
        case ID_PREVIOUS:
            result = CompositionTargetPass::IM_PREVIOUS;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, FilterOptions& tmip)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_NONE:
            tmip = FO_NONE;
            break;
        case ID_POINT:
            tmip = FO_POINT;
            break;
        case ID_LINEAR:
            tmip = FO_LINEAR;
            break;
        case ID_ANISOTROPIC:
            tmip = FO_ANISOTROPIC;
            break;
        default:
            return false;
        }
        return true;
    }

    template<> bool getValue(const AbstractNodePtr& node, TextureAddressingMode& result)
    {
        if(node->type != ANT_ATOM)
            return false;

        switch (static_cast<AtomAbstractNode*>(node.get())->id)
        {
        case ID_WRAP:
            result = TextureUnitState::TAM_WRAP;
            break;
        case ID_CLAMP:
            result = TextureUnitState::TAM_CLAMP;
            break;
        case ID_MIRROR:
            result = TextureUnitState::TAM_MIRROR;
            break;
        case ID_BORDER:
            result = TextureUnitState::TAM_BORDER;
            break;
        default:
            return false;
        }
        return true;
    }

    template <typename T>
    static bool getValue(PropertyAbstractNode* prop, ScriptCompiler *compiler, T& val)
    {
        if(prop->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
        }
        else if(prop->values.size() > 1)
        {
            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                               getPropertyName(compiler, prop->id) +
                                   " must have at most 1 argument");
        }
        else
        {
            if (getValue(prop->values.front(), val))
                return true;
            else
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   prop->values.front()->getValue() + " is not a valid value for " +
                                       getPropertyName(compiler, prop->id));
        }

        return false;
    }

    template <typename T>
    static bool _getVector(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end,
                          std::vector<T>& vals, size_t count)
    {
        vals.reserve(count);
        size_t n = 0;
        while (n < count)
        {
            if (i != end)
            {
                T v;
                if (!getValue(*i++, v))
                    return false;
                vals.push_back(v);
            }
            ++n;
        }

        vals.resize(count);
        return true;
    }

    static GpuProgramType getProgramType(int id)
    {
        switch(id)
        {
        default:
            assert(false);
            OGRE_FALLTHROUGH;
        case ID_VERTEX_PROGRAM:
        case ID_VERTEX_PROGRAM_REF:
            return GPT_VERTEX_PROGRAM;
        case ID_FRAGMENT_PROGRAM:
        case ID_FRAGMENT_PROGRAM_REF:
            return GPT_FRAGMENT_PROGRAM;
        case ID_GEOMETRY_PROGRAM:
        case ID_GEOMETRY_PROGRAM_REF:
            return GPT_GEOMETRY_PROGRAM;
        case ID_TESSELLATION_DOMAIN_PROGRAM:
        case ID_TESSELLATION_DOMAIN_PROGRAM_REF:
            return GPT_DOMAIN_PROGRAM;
        case ID_TESSELLATION_HULL_PROGRAM:
        case ID_TESSELLATION_HULL_PROGRAM_REF:
            return GPT_HULL_PROGRAM;
        case ID_COMPUTE_PROGRAM:
        case ID_COMPUTE_PROGRAM_REF:
            return GPT_COMPUTE_PROGRAM;
        }
    }

    void ScriptTranslator::processNode(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        if(node->type != ANT_OBJECT)
            return;

        // Abstract objects are completely skipped
        if((static_cast<ObjectAbstractNode*>(node.get()))->abstract)
            return;

        // Retrieve the translator to use
        ScriptTranslator *translator =
            ScriptCompilerManager::getSingleton().getTranslator(node);

        if(translator)
            translator->translate(compiler, node);
        else
            compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, node->file, node->line,
                               "token \"" + static_cast<ObjectAbstractNode*>(node.get())->cls + "\" is not recognized");
    }
    //-------------------------------------------------------------------------
    AbstractNodeList::const_iterator ScriptTranslator::getNodeAt(const AbstractNodeList &nodes, size_t index)
    {
        if(index >= nodes.size())
            return nodes.end();

        return std::next(nodes.begin(), index);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getBoolean(const AbstractNodePtr &node, bool *result)
    {
        if (node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		 if (atom->id == 1 || atom->id == 2)
		{
			*result = atom->id == 1 ? true : false;
			return true;
		}
        //     return false;
		return false;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getString(const AbstractNodePtr &node, String *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        *result = atom->value;
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getFloat(const Ogre::AbstractNodePtr &node, float *result)
    {
        return StringConverter::parse(node->getString(), *result);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getDouble(const Ogre::AbstractNodePtr &node, double *result)
    {
        return StringConverter::parse(node->getString(), *result);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getInt(const Ogre::AbstractNodePtr &node, int *result)
    {
        return StringConverter::parse(node->getString(), *result);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getUInt(const Ogre::AbstractNodePtr &node, uint *result)
    {
        return StringConverter::parse(node->getString(), *result);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result, int maxEntries)
    {
        int n = 0;
        while(i != end && n < maxEntries)
        {
            float v = 0;
            if(getFloat(*i, &v))
            {
                switch(n)
                {
                case 0:
                    result->r = v;
                    break;
                case 1:
                    result->g = v;
                    break;
                case 2:
                    result->b = v;
                    break;
                case 3:
                    result->a = v;
                    break;
                }
            }
            else
            {
                return false;
            }
            ++n;
            ++i;
        }
        // return error if we found less than rgb before end, unless constrained
        return (n >= 3 || n == maxEntries);
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getSceneBlendFactor(const Ogre::AbstractNodePtr &node, Ogre::SceneBlendFactor *sbf)
    {
        if(node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        switch(atom->id)
        {
        case ID_ONE:
            *sbf = SBF_ONE;
            break;
        case ID_ZERO:
            *sbf = SBF_ZERO;
            break;
        case ID_DEST_COLOUR:
            *sbf = SBF_DEST_COLOUR;
            break;
        case ID_DEST_ALPHA:
            *sbf = SBF_DEST_ALPHA;
            break;
        case ID_SRC_ALPHA:
            *sbf = SBF_SOURCE_ALPHA;
            break;
        case ID_SRC_COLOUR:
            *sbf = SBF_SOURCE_COLOUR;
            break;
        case ID_ONE_MINUS_DEST_COLOUR:
            *sbf = SBF_ONE_MINUS_DEST_COLOUR;
            break;
        case ID_ONE_MINUS_SRC_COLOUR:
            *sbf = SBF_ONE_MINUS_SOURCE_COLOUR;
            break;
        case ID_ONE_MINUS_DEST_ALPHA:
            *sbf = SBF_ONE_MINUS_DEST_ALPHA;
            break;
        case ID_ONE_MINUS_SRC_ALPHA:
            *sbf = SBF_ONE_MINUS_SOURCE_ALPHA;
            break;
        default:
            return false;
        }
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getCompareFunction(const AbstractNodePtr &node, CompareFunction *func)
    {
        if(node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        switch(atom->id)
        {
        case ID_ALWAYS_FAIL:
            *func = CMPF_ALWAYS_FAIL;
            break;
        case ID_ALWAYS_PASS:
            *func = CMPF_ALWAYS_PASS;
            break;
        case ID_LESS:
            *func = CMPF_LESS;
            break;
        case ID_LESS_EQUAL:
            *func = CMPF_LESS_EQUAL;
            break;
        case ID_EQUAL:
            *func = CMPF_EQUAL;
            break;
        case ID_NOT_EQUAL:
            *func = CMPF_NOT_EQUAL;
            break;
        case ID_GREATER_EQUAL:
            *func = CMPF_GREATER_EQUAL;
            break;
        case ID_GREATER:
            *func = CMPF_GREATER;
            break;
        default:
            return false;
        }
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m)
    {
        int n = 0;
        while (i != end && n < 16)
        {
            Real r = 0;
            if (!getReal(*i, &r))
                return false;

            (*m)[n/4][n%4] = r;
            ++i;
            ++n;
        }
        return n == 16;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count)
    {
        bool success = true;
        int n = 0;
        while (n < count)
        {
            if (i != end)
            {
                int v = 0;
                if (getInt(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if (n < count)
            success = false;

        return success;
    }
    bool ScriptTranslator::getVector(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, std::vector<int>& vals, size_t count)
    {
        return _getVector(i, end, vals, count);
    }
    //----------------------------------------------------------------------------
    bool ScriptTranslator::getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count)
    {
        bool success = true;
        int n = 0;
        while (n < count)
        {
            if (i != end)
            {
                float v = 0;
                if (getFloat(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if (n < count)
            success = false;

        return success;
    }
    bool ScriptTranslator::getVector(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, std::vector<float>& vals, size_t count)
    {
        return _getVector(i, end, vals, count);
    }
    //----------------------------------------------------------------------------
    bool ScriptTranslator::getDoubles(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, double *vals, int count)
    {
        bool success = true;
        int n = 0;
        while (n < count)
        {
            if (i != end)
            {
                double v = 0;
                if (getDouble(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if (n < count)
            success = false;

        return success;
    }
    //----------------------------------------------------------------------------
    bool ScriptTranslator::getUInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count)
    {
        bool success = true;
        int n = 0;
        while (n < count)
        {
            if (i != end)
            {
                uint v = 0;
                if (getUInt(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if (n < count)
            success = false;

        return success;
    }
    //----------------------------------------------------------------------------
    bool ScriptTranslator::getBooleans(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count)
    {
        bool success = true;
        int n = 0;
        while (n < count)
        {
            if (i != end)
            {
                bool v = false;
                if (getBoolean(*i, &v))
                    vals[n] = (v != 0);
                else
                    break;
                ++i;
            }
            else
                vals[n] = false;
            ++n;
        }

        if (n < count)
            success = false;

        return success;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getStencilOp(const Ogre::AbstractNodePtr &node, Ogre::StencilOperation *op)
    {
        if(node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        switch(atom->id)
        {
        case ID_KEEP:
            *op = SOP_KEEP;
            break;
        case ID_ZERO:
            *op = SOP_ZERO;
            break;
        case ID_REPLACE:
            *op = SOP_REPLACE;
            break;
        case ID_INCREMENT:
            *op = SOP_INCREMENT;
            break;
        case ID_DECREMENT:
            *op = SOP_DECREMENT;
            break;
        case ID_INCREMENT_WRAP:
            *op = SOP_INCREMENT_WRAP;
            break;
        case ID_DECREMENT_WRAP:
            *op = SOP_DECREMENT_WRAP;
            break;
        case ID_INVERT:
            *op = SOP_INVERT;
            break;
        default:
            return false;
        }
        return true;
    }
    //---------------------------------------------------------------------
    bool ScriptTranslator::getConstantType(AbstractNodeList::const_iterator i, GpuConstantType *op)
    {
        const String& val = (*i)->getString();
        if(val.empty())
            return false;

        if (val.find("float") != String::npos)
        {
            int count = 1;
            if (val.size() == 6)
                count = StringConverter::parseInt(val.substr(5));
            else if (val.size() > 6)
                return false;

            if (count > 4 || count == 0)
                return false;

            *op = (GpuConstantType)(GCT_FLOAT1 + count - 1);
        }
        else if (val.find("double") != String::npos)
        {
            int count = 1;
            if (val.size() == 7)
                count = StringConverter::parseInt(val.substr(6));
            else if (val.size() > 7)
                return false;

            if (count > 4 || count == 0)
                return false;

            *op = (GpuConstantType)(GCT_DOUBLE1 + count - 1);
        }
        else if (val.find("uint") != String::npos)
        {
            int count = 1;
            if (val.size() == 5)
                count = StringConverter::parseInt(val.substr(4));
            else if (val.size() > 5)
                return false;

            if (count > 4 || count == 0)
                return false;

            *op = (GpuConstantType)(GCT_UINT1 + count - 1);
        }
        else if (val.find("int") != String::npos)
        {
            int count = 1;
            if (val.size() == 4)
                count = StringConverter::parseInt(val.substr(3));
            else if (val.size() > 4)
                return false;

            if (count > 4 || count == 0)
                return false;

            *op = (GpuConstantType)(GCT_INT1 + count - 1);
        }
        else if (val.find("bool") != String::npos)
        {
            int count = 1;
            if (val.size() == 5)
                count = StringConverter::parseInt(val.substr(4));
            else if (val.size() > 5)
                return false;

            if (count > 4 || count == 0)
                return false;

            *op = (GpuConstantType)(GCT_BOOL1 + count - 1);
        }
        else if(val.find("matrix") != String::npos)
        {
            int count1, count2;

            if (val.size() == 9)
            {
                count1 = StringConverter::parseInt(val.substr(6, 1));
                count2 = StringConverter::parseInt(val.substr(8, 1));
            }
            else
                return false;

            if (count1 > 4 || count1 < 2 || count2 > 4 || count2 < 2)
                return false;

            switch(count1)
            {
            case 2:
                *op = (GpuConstantType)(GCT_MATRIX_2X2 + count2 - 2);
                break;
            case 3:
                *op = (GpuConstantType)(GCT_MATRIX_3X2 + count2 - 2);
                break;
            case 4:
                *op = (GpuConstantType)(GCT_MATRIX_4X2 + count2 - 2);
                break;
            }

        }
        else
        {
            return false;
        }

        return true;
    }

    /**************************************************************************
     * MaterialTranslator
     *************************************************************************/
    MaterialTranslator::MaterialTranslator()
        :mMaterial(0)
    {
    }
    //-------------------------------------------------------------------------
    void MaterialTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);

        // Create a material with the given name
        CreateMaterialScriptCompilerEvent evt(node->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mMaterial);

        if(!processed)
        {
            mMaterial = MaterialManager::getSingleton().create(obj->name, compiler->getResourceGroup()).get();
        }

        if(!mMaterial)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, obj->name);
            return;
        }

        mMaterial->removeAllTechniques();
        obj->context = mMaterial;
        mMaterial->_notifyOrigin(obj->file);

        bool bval;
        String sval;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_LOD_VALUES:
                    {
                        Material::LodValueList lods;
                        for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
                        {
                            Real v = 0;
                            if(getReal(*j, &v))
                                lods.push_back(v);
                            else
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   "lod_values expects only numbers as arguments");
                        }
                        mMaterial->setLodLevels(lods);
                    }
                    break;
                case ID_LOD_DISTANCES:
                    {
                        // Deprecated! Only for backwards compatibility.
                        // Set strategy hard-coded to 'distance' strategy, since that was the only one available back then,
                        // when using this material keyword was still current.
                        LodStrategy *strategy = DistanceLodSphereStrategy::getSingletonPtr();
                        mMaterial->setLodStrategy(strategy);

                        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file,
                                           prop->line,
                                           "lod_distances. Use lod_values.");
                        // Read in LOD distances
                        Material::LodValueList lods;
                        for(auto& j : prop->values)
                        {
                            Real v = 0;
                            if(getReal(j, &v))
                                lods.push_back(v);
                            else
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   "lod_values expects only numbers as arguments");
                        }
                        mMaterial->setLodLevels(lods);
                    }
                    break;
                case ID_LOD_STRATEGY:
                    if(getValue(prop, compiler, sval))
                    {
                            if (sval == "Distance" || sval == "PixelCount")
                                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                                   sval + ". use distance_box or pixel_count");

                            LodStrategy* strategy = LodStrategyManager::getSingleton().getStrategy(sval);
                            if (strategy)
                                mMaterial->setLodStrategy(strategy);
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, sval);
                    }
                    break;
                case ID_RECEIVE_SHADOWS:
                    if(getValue(prop, compiler, bval))
                        mMaterial->setReceiveShadows(bval);
                    break;
                case ID_TRANSPARENCY_CASTS_SHADOWS:
                    if(getValue(prop, compiler, bval))
                        mMaterial->setTransparencyCastsShadows(bval);
                    break;
                case ID_SET_TEXTURE_ALIAS:
                    if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                            "set_texture_alias must have 2 string arguments");
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                            "set_texture_alias. Use 'set $variable value'");

                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        String name, value;
                        if(getString(*i0, &name) && getString(*i1, &value))
                            mTextureAliases.insert(std::make_pair(name, value));
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
        }

        applyTextureAliases(compiler, mMaterial, mTextureAliases);
        mTextureAliases.clear();
    }

    /**************************************************************************
     * TechniqueTranslator
     *************************************************************************/
    TechniqueTranslator::TechniqueTranslator()
        :mTechnique(0)
    {
    }
    //-------------------------------------------------------------------------
    void TechniqueTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // Create the technique from the material
        Ogre::Material *material = Ogre::any_cast<Ogre::Material*>(obj->parent->context);
        mTechnique = material->createTechnique();
        obj->context = mTechnique;

        // Get the name of the technique
        if(!obj->name.empty())
            mTechnique->setName(obj->name);

        uint32 uival;
        String sval;

        // Set the properties for the material
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_SCHEME:
                    if(getValue(prop, compiler, sval))
                        mTechnique->setSchemeName(sval);
                    break;
                case ID_LOD_INDEX:
                    if(getValue(prop, compiler, uival))
                        mTechnique->setLodIndex(static_cast<uint16>(uival));
                    break;
                case ID_SHADOW_CASTER_MATERIAL:
                    if(getValue(prop, compiler, sval))
                    {
                        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, sval);
                        compiler->_fireEvent(&evt, 0);
                        mTechnique->setShadowCasterMaterial(evt.mName); // Use the processed name
                    }
                    break;
                case ID_SHADOW_RECEIVER_MATERIAL:
                    if(getValue(prop, compiler, sval))
                    {
                        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, sval);
                        compiler->_fireEvent(&evt, 0);
                        mTechnique->setShadowReceiverMaterial(evt.mName); // Use the processed name
                    }
                    break;
                case ID_GPU_VENDOR_RULE:
                    if(prop->values.size() < 2)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "gpu_vendor_rule must have 2 arguments");
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "gpu_vendor_rule must have 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);

                        Technique::GPUVendorRule rule;
                        if ((*i0)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();
                            if (atom0->id == ID_INCLUDE)
                            {
                                rule.includeOrExclude = Technique::INCLUDE;
                            }
                            else if (atom0->id == ID_EXCLUDE)
                            {
                                rule.includeOrExclude = Technique::EXCLUDE;
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "gpu_vendor_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
                            }

                            String vendor;
                            if(!getString(*i1, &vendor))
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "gpu_vendor_rule cannot accept \"" + (*i1)->getValue() + "\" as second argument");

                            rule.vendor = RenderSystemCapabilities::vendorFromString(vendor);

                            if (rule.vendor != GPU_UNKNOWN)
                            {
                                mTechnique->addGPUVendorRule(rule);
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "gpu_vendor_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
                        }

                    }
                    break;
                case ID_GPU_DEVICE_RULE:
                    if(prop->values.size() < 2)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "gpu_device_rule must have at least 2 arguments");
                    }
                    else if(prop->values.size() > 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "gpu_device_rule must have at most 3 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);

                        Technique::GPUDeviceNameRule rule;
                        if ((*i0)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();
                            if (atom0->id == ID_INCLUDE)
                            {
                                rule.includeOrExclude = Technique::INCLUDE;
                            }
                            else if (atom0->id == ID_EXCLUDE)
                            {
                                rule.includeOrExclude = Technique::EXCLUDE;
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "gpu_device_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
                            }

                            if(!getString(*i1, &rule.devicePattern))
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "gpu_device_rule cannot accept \"" + (*i1)->getValue() + "\" as second argument");

                            if (prop->values.size() == 3)
                            {
                                AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
                                if (!getBoolean(*i2, &rule.caseSensitive))
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "gpu_device_rule third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                            }

                            mTechnique->addGPUDeviceNameRule(rule);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "gpu_device_rule cannot accept \"" + (*i0)->getValue() + "\" as first argument");
                        }

                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
        }
    }

    /**************************************************************************
     * PassTranslator
     *************************************************************************/
    PassTranslator::PassTranslator()
        :mPass(0)
    {
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        Technique *technique = any_cast<Technique*>(obj->parent->context);
        mPass = technique->createPass();
        obj->context = mPass;

        // Get the name of the technique
        if(!obj->name.empty())
            mPass->setName(obj->name);

        Real fval;
        bool bval;
        uint32 uival;

        // Set the properties for the material
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_AMBIENT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "ambient must have at most 4 parameters");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM &&
                           ((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
                        {
                            mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_AMBIENT);
                        }
                        else
                        {
                            ColourValue val = ColourValue::White;
                            if(getColour(prop->values.begin(), prop->values.end(), &val))
                                mPass->setAmbient(val);
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "ambient requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
                        }
                    }
                    break;
                case ID_DIFFUSE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "diffuse must have at most 4 arguments");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM &&
                           ((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
                        {
                            mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_DIFFUSE);
                        }
                        else
                        {
                            ColourValue val = ColourValue::White;
                            if(getColour(prop->values.begin(), prop->values.end(), &val))
                                mPass->setDiffuse(val);
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "diffuse requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
                        }
                    }
                    break;
                case ID_SPECULAR:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 5)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "specular must have at most 5 arguments");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM &&
                           ((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
                        {
                            mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_SPECULAR);

                            if(prop->values.size() >= 2)
                            {
                                Real val = 0;
                                if(getReal(prop->values.back(), &val))
                                    mPass->setShininess(val);
                                else
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "specular does not support \"" + prop->values.back()->getValue() + "\" as its second argument");
                            }
                        }
                        else
                        {
                            if(prop->values.size() < 4)
                            {
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   "specular expects at least 4 arguments");
                            }
                            else
                            {
                                ColourValue val;
                                if(getColour(prop->values.begin(), prop->values.end(), &val))
                                {
                                    if(prop->values.size() == 4)
                                    {
                                        mPass->setShininess(val.a);
                                        val.a = 1.0f;
                                        mPass->setSpecular(val);
                                    }
                                    else
                                    {
                                        mPass->setSpecular(val);

                                        Real shininess;
                                        if(getReal(*getNodeAt(prop->values, 4), &shininess))
                                            mPass->setShininess(shininess);
                                        else
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "specular fourth argument must be a valid number for shininess attribute");
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "specular must have first 3 arguments be a valid colour");
                                }
                            }

                        }
                    }
                    break;
                case ID_EMISSIVE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "emissive must have at most 4 arguments");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM &&
                           ((AtomAbstractNode*)prop->values.front().get())->id == ID_VERTEXCOLOUR)
                        {
                            mPass->setVertexColourTracking(mPass->getVertexColourTracking() | TVC_EMISSIVE);
                        }
                        else
                        {
                            ColourValue val(0.0f, 0.0f, 0.0f, 1.0f);
                            if(getColour(prop->values.begin(), prop->values.end(), &val))
                                mPass->setSelfIllumination(val);
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "emissive requires 3 or 4 colour arguments, or a \"vertexcolour\" directive");
                        }
                    }
                    break;
                case ID_SCENE_BLEND:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scene_blend supports at most 2 arguments");
                    }
                    else if(prop->values.size() == 1)
                    {
                        SceneBlendType enval;
                        if(getValue(prop->values.front(), enval))
                        {
                            mPass->setSceneBlending(enval);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "scene_blend does not support \"" + prop->values.front()->getValue() + "\" for argument 1");
                        }
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        SceneBlendFactor sbf0, sbf1;
                        if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
                        {
                            mPass->setSceneBlending(sbf0, sbf1);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "scene_blend does not support \"" + (*i0)->getValue() + "\" and \"" + (*i1)->getValue() + "\" as arguments");
                        }
                    }
                    break;
                case ID_SEPARATE_SCENE_BLEND:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() == 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "separate_scene_blend must have 2 or 4 arguments");
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "separate_scene_blend must have 2 or 4 arguments");
                    }
                    else if(prop->values.size() == 2)
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        SceneBlendType sbt0, sbt1;
                        if (getValue(*i0, sbt0) && getValue(*i1, sbt1))
                        {
                            mPass->setSeparateSceneBlending(sbt0, sbt1);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "separate_scene_blend does not support \"" + (*i0)->getValue() + "\" as argument 1");
                        }
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
                            i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3);
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM)
                        {
                            SceneBlendFactor sbf0, sbf1, sbf2, sbf3;
                            if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1) && getSceneBlendFactor(*i2, &sbf2) &&
                               getSceneBlendFactor(*i3, &sbf3))
                            {
                                mPass->setSeparateSceneBlending(sbf0, sbf1, sbf2, sbf3);
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "one of the arguments to separate_scene_blend is not a valid scene blend factor directive");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "one of the arguments to separate_scene_blend is not a valid scene blend factor directive");
                        }
                    }
                    break;
                case ID_SCENE_BLEND_OP:
                    SceneBlendOperation sop;
                    if(getValue(prop, compiler, sop))
                        mPass->setSceneBlendingOperation(sop);
                    break;
                case ID_SEPARATE_SCENE_BLEND_OP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "separate_scene_blend_op must have 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        SceneBlendOperation op, alphaOp;
                        if(getValue(*i0, op) && getValue(*i1, alphaOp))
                        {
                            mPass->setSeparateSceneBlendingOperation(op, alphaOp);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + ": unrecognized argument");
                        }
                    }
                    break;
                case ID_DEPTH_CHECK:
                    if(getValue(prop, compiler, bval))
                        mPass->setDepthCheckEnabled(bval);
                    break;
                case ID_DEPTH_WRITE:
                    if(getValue(prop, compiler, bval))
                        mPass->setDepthWriteEnabled(bval);
                    break;
                case ID_DEPTH_BIAS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_bias must have at most 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        float val0, val1 = 0.0f;
                        if(getFloat(*i0, &val0))
                        {
                            if(i1 != prop->values.end())
                                getFloat(*i1, &val1);
                            mPass->setDepthBias(val0, val1);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_bias does not support \"" + (*i0)->getValue() + "\" for argument 1");
                        }
                    }
                    break;
                case ID_DEPTH_FUNC:
                    CompareFunction dfunc;
                    if(getValue(prop, compiler, dfunc))
                        mPass->setDepthFunction(dfunc);
                    break;
                case ID_ITERATION_DEPTH_BIAS:
                    if(getValue(prop, compiler, fval))
                        mPass->setIterationDepthBias(fval);
                    break;
                case ID_ALPHA_REJECTION:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "alpha_rejection must have at most 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        CompareFunction func;
                        if(getCompareFunction(*i0, &func))
                        {
                            if(i1 != prop->values.end())
                            {
                                uint32 val = 0;
                                if(getUInt(*i1, &val))
                                    mPass->setAlphaRejectSettings(func, static_cast<unsigned char>(val));
                                else
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i1)->getValue() + " is not a valid integer");
                            }
                            else
                                mPass->setAlphaRejectFunction(func);
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*i0)->getValue() + " is not a valid CompareFunction");
                    }
                    break;
                case ID_ALPHA_TO_COVERAGE:
                    if(getValue(prop, compiler, bval))
                        mPass->setAlphaToCoverageEnabled(bval);
                    break;
                case ID_LIGHT_SCISSOR:
                    if(getValue(prop, compiler, bval))
                        mPass->setLightScissoringEnabled(bval);
                    break;
                case ID_LIGHT_CLIP_PLANES:
                    if(getValue(prop, compiler, bval))
                        mPass->setLightClipPlanesEnabled(bval);
                    break;
                case ID_TRANSPARENT_SORTING:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "transparent_sorting must have at most 1 argument");
                    }
                    else
                    {
                        bool val = true;
                        if(getBoolean(prop->values.front(), &val))
                        {
                            mPass->setTransparentSortingEnabled(val);
                            mPass->setTransparentSortingForced(false);
                        }
                        else
                        {
                            String val2;
                            if (prop->values.front()->getString()=="force")
                            {
                                mPass->setTransparentSortingEnabled(true);
                                mPass->setTransparentSortingForced(true);
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " must be boolean or force");
                            }
                        }
                    }
                    break;
                case ID_ILLUMINATION_STAGE:
                    IlluminationStage is;
                    if(getValue(prop, compiler, is))
                        mPass->setIlluminationStage(is);
                    break;
                case ID_CULL_HARDWARE:
                    CullingMode cmode;
                    if(getValue(prop, compiler, cmode))
                        mPass->setCullingMode(cmode);
                    break;
                case ID_CULL_SOFTWARE:
                    ManualCullingMode mmode;
                    if(getValue(prop, compiler, mmode))
                        mPass->setManualCullingMode(mmode);
                    compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                       prop->name + ". Only used by the BSP scene manager.");
                    break;
                case ID_LIGHTING:
                    if(getValue(prop, compiler, bval))
                        mPass->setLightingEnabled(bval);
                    break;
                case ID_SHADING:
                    ShadeOptions smode;
                    if(getValue(prop, compiler, smode))
                        mPass->setShadingMode(smode);
                    break;
                case ID_POLYGON_MODE:
                    PolygonMode pmode;
                    if(getValue(prop, compiler, pmode))
                        mPass->setPolygonMode(pmode);
                    break;
                case ID_POLYGON_MODE_OVERRIDEABLE:
                    if(getValue(prop, compiler, bval))
                        mPass->setPolygonModeOverrideable(bval);
                    break;
                case ID_FOG_OVERRIDE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 8)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "fog_override must have at most 8 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                        {
                            FogMode mode = FOG_NONE;
                            ColourValue clr = ColourValue::White;
                            Real dens = 0.001, start = 0.0f, end = 1.0f;

                            if(i1 != prop->values.end())
                            {
                                if((*i1)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
                                    switch(atom->id)
                                    {
                                    case ID_NONE:
                                        mode = FOG_NONE;
                                        break;
                                    case ID_LINEAR:
                                        mode = FOG_LINEAR;
                                        break;
                                    case ID_EXP:
                                        mode = FOG_EXP;
                                        break;
                                    case ID_EXP2:
                                        mode = FOG_EXP2;
                                        break;
                                    default:
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*i1)->getValue() + " is not a valid FogMode");
                                        break;
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i1)->getValue() + " is not a valid FogMode");
                                    break;
                                }
                            }

                            if(i2 != prop->values.end())
                            {
                                if(!getColour(i2, prop->values.end(), &clr, 3))
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i2)->getValue() + " is not a valid colour");
                                    break;
                                }

                                i2 = getNodeAt(prop->values, 5);
                            }

                            if(i2 != prop->values.end())
                            {
                                if(!getReal(*i2, &dens))
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i2)->getValue() + " is not a valid number");
                                    break;
                                }
                                ++i2;
                            }

                            if(i2 != prop->values.end())
                            {
                                if(!getReal(*i2, &start))
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i2)->getValue() + " is not a valid number");
                                    return;
                                }
                                ++i2;
                            }

                            if(i2 != prop->values.end())
                            {
                                if(!getReal(*i2, &end))
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i2)->getValue() + " is not a valid number");
                                    return;
                                }
                                ++i2;
                            }

                            mPass->setFog(val, mode, clr, dens, start, end);
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
                    break;
                case ID_COLOUR_WRITE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() == 1)
                    {
                        if(getValue(prop, compiler, bval))
                            mPass->setColourWriteEnabled(bval);
                    }
                    else if(prop->values.size() != 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "colour_write must have exactly 1 or 4 arguments");
                    }
                    else
                    {
                        bool colourMask[] = {false ,false ,false, false};

                        uint8 channelIndex = 0;
                        for(const AbstractNodePtr& abstractNode : prop->values)
                        {
                            if(!getBoolean(abstractNode, &colourMask[channelIndex++]))
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   abstractNode->getValue() + " is not a valid boolean");
                                break;
                            }
                        }

                        mPass->setColourWriteEnabled(colourMask[0], colourMask[1], colourMask[2], colourMask[3]);
                    }
                    break;
                case ID_MAX_LIGHTS:
                    if(getValue(prop, compiler, uival))
                        mPass->setMaxSimultaneousLights(Math::uint16Cast(uival));
                    break;
                case ID_START_LIGHT:
                    if(getValue(prop, compiler, uival))
                        mPass->setStartLight(static_cast<uint16>(uival));
                    break;
                case ID_LIGHT_MASK:
                    if(getValue(prop, compiler, uival))
                        mPass->setLightMask(static_cast<uint16>(uival));
                    break;
                case ID_ITERATION:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        if((*i0)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
                            if(atom->id == ID_ONCE)
                            {
                                mPass->setIteratePerLight(false);
                            }
                            else if(atom->id == ID_ONCE_PER_LIGHT)
                            {
                                AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
                                if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
                                {
                                    atom = (AtomAbstractNode*)(*i1).get();
                                    switch(atom->id)
                                    {
                                    case ID_POINT:
                                        mPass->setIteratePerLight(true);
                                        break;
                                    case ID_DIRECTIONAL:
                                        mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
                                        break;
                                    case ID_SPOT:
                                        mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
                                        break;
                                    default:
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           prop->values.front()->getValue() + " is not a valid light type (point, directional, or spot)");
                                    }
                                }
                                else
                                {
                                    mPass->setIteratePerLight(true, false);
                                }

                            }
                            else if(StringConverter::isNumber(atom->value))
                            {
                                mPass->setPassIterationCount(Ogre::StringConverter::parseInt(atom->value));

                                AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
                                if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
                                {
                                    atom = (AtomAbstractNode*)(*i1).get();
                                    if(atom->id == ID_PER_LIGHT)
                                    {
                                        AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
                                        if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
                                        {
                                            atom = (AtomAbstractNode*)(*i2).get();
                                            switch(atom->id)
                                            {
                                            case ID_POINT:
                                                mPass->setIteratePerLight(true);
                                                break;
                                            case ID_DIRECTIONAL:
                                                mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
                                                break;
                                            case ID_SPOT:
                                                mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
                                                break;
                                            default:
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                   (*i2)->getValue() + " is not a valid light type (point, directional, or spot)");
                                            }
                                        }
                                        else
                                        {
                                            mPass->setIteratePerLight(true, false);
                                        }
                                    }
                                    else if(atom->id == ID_PER_N_LIGHTS)
                                    {
                                        AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
                                        if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
                                        {
                                            atom = (AtomAbstractNode*)(*i2).get();
                                            if(StringConverter::isNumber(atom->value))
                                            {
                                                mPass->setLightCountPerIteration(
                                                    static_cast<unsigned short>(StringConverter::parseInt(atom->value)));

                                                AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
                                                if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
                                                {
                                                    atom = (AtomAbstractNode*)(*i3).get();
                                                    switch(atom->id)
                                                    {
                                                    case ID_POINT:
                                                        mPass->setIteratePerLight(true);
                                                        break;
                                                    case ID_DIRECTIONAL:
                                                        mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
                                                        break;
                                                    case ID_SPOT:
                                                        mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
                                                        break;
                                                    default:
                                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                           (*i3)->getValue() + " is not a valid light type (point, directional, or spot)");
                                                    }
                                                }
                                                else
                                                {
                                                    mPass->setIteratePerLight(true, false);
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                                   (*i2)->getValue() + " is not a valid number");
                                            }
                                        }
                                        else
                                        {
                                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                               prop->values.front()->getValue() + " is not a valid number");
                                        }
                                    }
                                }
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_LINE_WIDTH:
                    if(getValue(prop, compiler, fval))
                        mPass->setLineWidth(fval);
                    break;
                case ID_POINT_SIZE:
                    if(getValue(prop, compiler, fval))
                        mPass->setPointSize(fval);
                    break;
                case ID_POINT_SPRITES:
                    if(getValue(prop, compiler, bval))
                        mPass->setPointSpritesEnabled(bval);
                    break;
                case ID_POINT_SIZE_ATTENUATION:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "point_size_attenuation must have at most 4 arguments");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                        {
                            if(val)
                            {
                                AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2),
                                    i3 = getNodeAt(prop->values, 3);

                                if (prop->values.size() > 1)
                                {

                                    Real constant = 0.0f, linear = 1.0f, quadratic = 0.0f;

                                    if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
                                    {
                                        AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
                                        if(StringConverter::isNumber(atom->value))
                                            constant = StringConverter::parseReal(atom->value);
                                        else
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*i1)->getValue() + " is not a valid number");
                                    }

                                    if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
                                    {
                                        AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
                                        if(StringConverter::isNumber(atom->value))
                                            linear = StringConverter::parseReal(atom->value);
                                        else
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*i2)->getValue() + " is not a valid number");
                                    }

                                    if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
                                    {
                                        AtomAbstractNode *atom = (AtomAbstractNode*)(*i3).get();
                                        if(StringConverter::isNumber(atom->value))
                                            quadratic = StringConverter::parseReal(atom->value);
                                        else
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*i3)->getValue() + " is not a valid number");
                                    }

                                    mPass->setPointAttenuation(true, constant, linear, quadratic);
                                }
                                else
                                {
                                    mPass->setPointAttenuation(true);
                                }
                            }
                            else
                            {
                                mPass->setPointAttenuation(false);
                            }
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
                    break;
                case ID_POINT_SIZE_MIN:
                    if(getValue(prop, compiler, fval))
                        mPass->setPointMinSize(fval);
                    break;
                case ID_POINT_SIZE_MAX:
                    if(getValue(prop, compiler, fval))
                        mPass->setPointMaxSize(fval);
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                ObjectAbstractNode *child = static_cast<ObjectAbstractNode*>(i.get());
                switch(child->id)
                {
                case ID_FRAGMENT_PROGRAM_REF:
                case ID_VERTEX_PROGRAM_REF:
                case ID_GEOMETRY_PROGRAM_REF:
                case ID_TESSELLATION_HULL_PROGRAM_REF:
                case ID_TESSELLATION_DOMAIN_PROGRAM_REF:
                case ID_COMPUTE_PROGRAM_REF:
                    translateProgramRef(getProgramType(child->id), compiler, child);
                    break;
                case ID_SHADOW_CASTER_VERTEX_PROGRAM_REF:
                    translateShadowCasterProgramRef(GPT_VERTEX_PROGRAM, compiler, child);
                    break;
                case ID_SHADOW_CASTER_FRAGMENT_PROGRAM_REF:
                    translateShadowCasterProgramRef(GPT_FRAGMENT_PROGRAM, compiler, child);
                    break;
                case ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF:
                    translateShadowReceiverProgramRef(GPT_VERTEX_PROGRAM, compiler, child);
                    break;
                case ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF:
                    translateShadowReceiverProgramRef(GPT_FRAGMENT_PROGRAM, compiler, child);
                    break;
                default:
                    processNode(compiler, i);
                    break;
                case ID_FRAGMENT_PROGRAM:
                case ID_VERTEX_PROGRAM:
                case ID_GEOMETRY_PROGRAM:
                case ID_TESSELLATION_HULL_PROGRAM:
                case ID_TESSELLATION_DOMAIN_PROGRAM:
                case ID_COMPUTE_PROGRAM:
                {
                    // auto assign inline defined programs
                    processNode(compiler, i);
                    GpuProgramType type = getProgramType(child->id);
                    mPass->setGpuProgram(type, GpuProgramUsage::_getProgramByName(child->name, mPass->getResourceGroup(), type));
                }
                }
            }
        }
    }

    static GpuProgramPtr getProgram(ScriptCompiler* compiler, ObjectAbstractNode* node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return nullptr;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        auto& mgr = GpuProgramManager::getSingleton();
        if (auto ret = mgr.getByName(evt.mName, compiler->getResourceGroup()))
            return ret;

        // recheck with auto resource group
        if (auto ret = mgr.getByName(evt.mName, RGN_AUTODETECT))
            return ret;

        compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line,
                           evt.mName);
        return nullptr;
    }

    //-------------------------------------------------------------------------
    void PassTranslator::translateProgramRef(GpuProgramType type, ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        auto program = getProgram(compiler, node);
        if(!program) return;
        auto pass = any_cast<Pass*>(node->parent->context);

        pass->setGpuProgram(type, program);
        if(program->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getGpuProgramParameters(type);
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateShadowCasterProgramRef(GpuProgramType type, ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        auto program = getProgram(compiler, node);
        if(!program) return;
        auto pass = any_cast<Pass*>(node->parent->context);

        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, node->file, node->line,
                           node->cls + ". Use shadow_caster_material instead");

        auto caster_mat = pass->getParent()->getShadowCasterMaterial();
        if(!caster_mat)
        {
            auto src_mat = pass->getParent()->getParent();
            // only first pass of this will be used. The caster material is technique specific.
            caster_mat = src_mat->clone(
                StringUtil::format("%s/%p/CasterFallback", src_mat->getName().c_str(), pass->getParent()));
            pass->getParent()->setShadowCasterMaterial(caster_mat);
        }
        auto caster_pass = caster_mat->getTechnique(0)->getPass(0);

        caster_pass->setGpuProgram(type, program);
        if(program->isSupported())
        {
            GpuProgramParametersSharedPtr params = caster_pass->getGpuProgramParameters(type);
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateShadowReceiverProgramRef(GpuProgramType type,ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        auto program = getProgram(compiler, node);
        if(!program) return;
        auto pass = any_cast<Pass*>(node->parent->context);

        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, node->file, node->line,
                           node->cls + ". Use shadow_receiver_material instead");

        auto receiver_mat = pass->getParent()->getShadowReceiverMaterial();
        if(!receiver_mat)
        {
            auto src_mat = pass->getParent()->getParent();
            // only first pass of this will be used
            receiver_mat = src_mat->clone(src_mat->getName()+"/ReceiverFallback");
            pass->getParent()->setShadowReceiverMaterial(receiver_mat);
        }
        auto receiver_pass = receiver_mat->getTechnique(0)->getPass(0);

        receiver_pass->setGpuProgram(type, program);
        if(program->isSupported())
        {
            GpuProgramParametersSharedPtr params = receiver_pass->getGpuProgramParameters(type);
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    /**************************************************************************
     * TextureUnitTranslator
     *************************************************************************/
    TextureUnitTranslator::TextureUnitTranslator()
        :mUnit(0)
    {
    }
    //-------------------------------------------------------------------------
    void SamplerTranslator::translateSamplerParam(ScriptCompiler* compiler, const SamplerPtr& sampler, PropertyAbstractNode* prop)
    {
        bool bval;
        Real fval;
        uint32 uival;

        switch(prop->id)
        {
        case ID_TEX_ADDRESS_MODE:
            {
                if(prop->values.empty())
                {
                    compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                }
                else
                {
                    AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                        i1 = getNodeAt(prop->values, 1),
                        i2 = getNodeAt(prop->values, 2);
                    Sampler::UVWAddressingMode mode;

                    if(!getValue(*i0, mode.u))
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                           (*i0)->getValue() + " not supported as first argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                        return;
                    }
                    mode.v = mode.u;
                    mode.w = mode.u;

                    if(i1 != prop->values.end())
                    {
                        if(!getValue(*i1, mode.v))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*i1)->getValue() + " not supported as second argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                        }
                    }

                    if(i2 != prop->values.end())
                    {
                        if(!getValue(*i2, mode.w))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*i2)->getValue() + " not supported as third argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                        }
                    }

                    sampler->setAddressingMode(mode);
                }
            }
            break;
        case ID_TEX_BORDER_COLOUR:
            if(prop->values.empty())
            {
                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
            }
            else
            {
                ColourValue val;
                if(getColour(prop->values.begin(), prop->values.end(), &val))
                    sampler->setBorderColour(val);
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                       "tex_border_colour only accepts a colour argument");
            }
            break;
        case ID_FILTERING:
            if(prop->values.empty())
            {
                compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
            }
            else if(prop->values.size() == 1)
            {
                if(prop->values.front()->type == ANT_ATOM)
                {
                    AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                    switch(atom->id)
                    {
                    case ID_NONE:
                        sampler->setFiltering(TFO_NONE);
                        break;
                    case ID_BILINEAR:
                        sampler->setFiltering(TFO_BILINEAR);
                        break;
                    case ID_TRILINEAR:
                        sampler->setFiltering(TFO_TRILINEAR);
                        break;
                    case ID_ANISOTROPIC:
                        sampler->setFiltering(TFO_ANISOTROPIC);
                        break;
                    default:
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                           prop->values.front()->getValue() + " not supported as first argument (must be \"none\", \"bilinear\", \"trilinear\", or \"anisotropic\")");
                    }
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                       prop->values.front()->getValue() + " not supported as first argument (must be \"none\", \"bilinear\", \"trilinear\", or \"anisotropic\")");
                }
            }
            else if(prop->values.size() == 3)
            {
                AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                    i1 = getNodeAt(prop->values, 1),
                    i2 = getNodeAt(prop->values, 2);
                FilterOptions tmin, tmax, tmip;
                if (getValue(*i0, tmin) && getValue(*i1, tmax) && getValue(*i2, tmip))
                {
                    sampler->setFiltering(tmin, tmax, tmip);
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                }
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                   "filtering must have either 1 or 3 arguments");
            }
            break;
        case ID_CMPTEST:
            if(getValue(prop, compiler, bval))
                sampler->setCompareEnabled(bval);
            break;
        case ID_CMPFUNC:
            compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file,
                prop->line,
                "compare_func. Use comp_func.");
            OGRE_FALLTHROUGH;
        case ID_COMP_FUNC:
            CompareFunction func;
            if(getValue(prop, compiler, func))
                sampler->setCompareFunction(func);
            break;
        case ID_MAX_ANISOTROPY:
            if(getValue(prop, compiler, uival))
                sampler->setAnisotropy(uival);
            break;
        case ID_MIPMAP_BIAS:
            if(getValue(prop, compiler, fval))
                sampler->setMipmapBias(fval);
            break;
        }
    }
    void SamplerTranslator::translate(ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        SamplerPtr sampler = TextureManager::getSingleton().createSampler(obj->name);

        // Set the properties for the material
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_TEX_ADDRESS_MODE:
                case ID_TEX_BORDER_COLOUR:
                case ID_FILTERING:
                case ID_CMPTEST:
                case ID_COMP_FUNC:
                case ID_MAX_ANISOTROPY:
                case ID_MIPMAP_BIAS:
                    translateSamplerParam(compiler, sampler, prop);
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
        }
    }

    void TextureUnitTranslator::translate(ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        Pass *pass = any_cast<Pass*>(obj->parent->context);
        mUnit = pass->createTextureUnitState();
        obj->context = mUnit;

        // Get the name of the technique
        if(!obj->name.empty())
            mUnit->setName(obj->name);

        Real fval;
        uint32 uival;
        String sval;

        // Set the properties for the material
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_TEX_ADDRESS_MODE:
                case ID_TEX_BORDER_COLOUR:
                case ID_FILTERING:
                case ID_CMPTEST:
                case ID_CMPFUNC:
                case ID_COMP_FUNC:
                case ID_MAX_ANISOTROPY:
                case ID_MIPMAP_BIAS:
                    SamplerTranslator::translateSamplerParam(compiler, mUnit->_getLocalSampler(), prop);
                    break;
                case ID_SAMPLER_REF:
                    if(getValue(prop, compiler, sval))
                    {
                        auto sampler = TextureManager::getSingleton().getSampler(sval);
                        if(sampler)
                            mUnit->setSampler(sampler);
                        else
                            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT,
                                               prop->file, prop->line, sval);
                    }
                    break;
                case ID_UNORDERED_ACCESS_MIP:
                    if(getValue(prop, compiler, uival))
                        mUnit->setUnorderedAccessMipLevel(uival);
                    break;
                case ID_TEXTURE_ALIAS:
                    compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                        "texture_alias. Use 'texture $variable'");
                    if(getValue(prop, compiler, sval))
                        mUnit->setName(sval);
                    break;
                case ID_TEXTURE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 5)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "texture must have at most 5 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();
                        String val;
                        if(getString(*j, &val))
                        {
                            TextureType texType = TEX_TYPE_2D;
                            bool isAlpha = false;
                            bool sRGBRead = false;
                            PixelFormat format = PF_UNKNOWN;
                            int mipmaps = MIP_DEFAULT;

                            ++j;
                            while(j != prop->values.end())
                            {
                                if((*j)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
                                    switch(atom->id)
                                    {
                                    case ID_1D:
                                        texType = TEX_TYPE_1D;
                                        break;
                                    case ID_2D:
                                        texType = TEX_TYPE_2D;
                                        break;
                                    case ID_3D:
                                        texType = TEX_TYPE_3D;
                                        break;
                                    case ID_CUBIC:
                                        texType = TEX_TYPE_CUBE_MAP;
                                        break;
                                    case ID_2DARRAY:
                                        texType = TEX_TYPE_2D_ARRAY;
                                        break;
                                    case ID_UNLIMITED:
                                        mipmaps = MIP_UNLIMITED;
                                        break;
                                    case ID_ALPHA:
                                        isAlpha = true;
                                        break;
                                    case ID_GAMMA:
                                        sRGBRead = true;
                                        break;
                                    default:
                                        if(!StringConverter::parse(atom->value, mipmaps))
                                        {
                                            format = PixelUtil::getFormatFromName(atom->value, true);

                                            if (format == PF_UNKNOWN)
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS,
                                                                   prop->file, prop->line, atom->value);
                                        }
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*j)->getValue() + " is not a supported argument to the texture property");
                                }
                                ++j;
                            }

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, val);
                            compiler->_fireEvent(&evt, 0);

                            if(isAlpha)
                            {
                                // format = PF_A8; should only be done, if src is luminance, which we dont know here
                                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file,
                                                   prop->line, "alpha. Use PF_A8 instead");
                            }

                            mUnit->setTextureName(evt.mName, texType);
                            mUnit->setDesiredFormat(format);
                            OGRE_IGNORE_DEPRECATED_BEGIN
                            mUnit->setIsAlpha(isAlpha);
                            OGRE_IGNORE_DEPRECATED_END
                            mUnit->setNumMipmaps(mipmaps);
                            mUnit->setHardwareGammaEnabled(sRGBRead);
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*j)->getValue() + " is not a valid texture name");
                    }
                    break;
                case ID_ANIM_TEXTURE:
                    if(prop->values.size() < 3)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
                        if((*i1)->type == ANT_ATOM && StringConverter::isNumber(((AtomAbstractNode*)(*i1).get())->value))
                        {
                            // Short form
                            AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i2 = getNodeAt(prop->values, 2);
                            if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                            {
                                String val0;
                                uint32 val1;
                                Real val2;
                                if(getString(*i0, &val0) && getUInt(*i1, &val1) && getReal(*i2, &val2))
                                {
                                    ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, val0);
                                    compiler->_fireEvent(&evt, 0);

                                    mUnit->setAnimatedTextureName(evt.mName, val1, val2);
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "anim_texture short form requires a texture name, number of frames, and animation duration");
                                }
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "anim_texture short form requires a texture name, number of frames, and animation duration");
                            }
                        }
                        else
                        {
                            // Long form has n number of frames
                            Real duration = 0;
                            AbstractNodeList::const_iterator in = getNodeAt(prop->values, static_cast<int>(prop->values.size()) - 1);
                            if(getReal(*in, &duration))
                            {
                                std::vector<String> names;

                                AbstractNodeList::iterator j = prop->values.begin();
                                while(j != in)
                                {
                                    if((*j)->type == ANT_ATOM)
                                    {
                                        String name = ((AtomAbstractNode*)(*j).get())->value;
                                        // Run the name through the listener
                                        if(compiler->getListener())
                                        {
                                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, name);
                                            compiler->_fireEvent(&evt, 0);
                                            names.push_back(evt.mName);
                                        }
                                        else
                                        {
                                            names.push_back(name);
                                        }
                                    }
                                    else
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*j)->getValue() + " is not supported as a texture name");
                                    ++j;
                                }

                                mUnit->setAnimatedTextureName(names, duration);
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   (*in)->getValue() + " is not supported for the duration argument");
                            }
                        }
                    }
                    break;
                case ID_CUBIC_TEXTURE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() == 2)
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                            i1 = getNodeAt(prop->values, 1);
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, atom0->value);
                            compiler->_fireEvent(&evt, 0);

                            mUnit->setTextureName(evt.mName, TEX_TYPE_CUBE_MAP);

                            compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                                   "'cubic_texture ..'. Use 'texture .. cubic' instead.");
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    else if(prop->values.size() == 7)
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                            i1 = getNodeAt(prop->values, 1),
                            i2 = getNodeAt(prop->values, 2),
                            i3 = getNodeAt(prop->values, 3),
                            i4 = getNodeAt(prop->values, 4),
                            i5 = getNodeAt(prop->values, 5),
                            i6 = getNodeAt(prop->values, 6);
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM &&
                           (*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM && (*i6)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get(),
                                *atom2 = (AtomAbstractNode*)(*i2).get(), *atom3 = (AtomAbstractNode*)(*i3).get(),
                                *atom4 = (AtomAbstractNode*)(*i4).get(), *atom5 = (AtomAbstractNode*)(*i5).get(),
                                *atom6 = (AtomAbstractNode*)(*i6).get();
                            std::vector<String> names(6);
                            // backward compatible order
                            names[4] = atom0->value;
                            names[5] = atom1->value;
                            names[1] = atom2->value;
                            names[0] = atom3->value;
                            names[2] = atom4->value;
                            names[3] = atom5->value;

                            if(compiler->getListener())
                            {
                                // Run each name through the listener
                                for(int j = 0; j < 6; ++j)
                                {
                                    ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, names[j]);
                                    compiler->_fireEvent(&evt, 0);
                                    names[j] = evt.mName;
                                }
                            }

                            if(atom6->id != ID_COMBINED_UVW)
                            {
                                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                                   "separateUV is no longer supported.");
                            }
                            mUnit->setLayerArrayNames(TEX_TYPE_CUBE_MAP, names);
                        }

                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "cubic_texture must have at most 7 arguments");
                    }
                    break;
                case ID_TEX_COORD_SET:
                    if(getValue(prop, compiler, uival))
                        mUnit->setTextureCoordSet(uival);
                    break;

                case ID_COLOUR_OP:
                    LayerBlendOperation cop;
                    if(getValue(prop, compiler, cop))
                        mUnit->setColourOperation(cop);
                    break;
                case ID_COLOUR_OP_EX:
                    if(prop->values.size() < 3)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "colour_op_ex must have at least 3 arguments");
                    }
                    else if(prop->values.size() > 10)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "colour_op_ex must have at most 10 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                            i1 = getNodeAt(prop->values, 1),
                            i2 = getNodeAt(prop->values, 2);
                        LayerBlendOperationEx op;
                        LayerBlendSource source1, source2;
                        if (getValue(*i0, op) && getValue(*i1, source1) && getValue(*i2, source2))
                        {
                            ColourValue arg1 = ColourValue::White, arg2 = ColourValue::White;
                            Real manualBlend = 0.0f;

                            if(op == LBX_BLEND_MANUAL)
                            {
                                AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
                                if(i3 != prop->values.end())
                                {
                                    if(!getReal(*i3, &manualBlend))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*i3)->getValue() + " is not a valid number argument");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "fourth argument expected when blend_manual is used");
                                }
                            }

                            AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
                            if(op == LBX_BLEND_MANUAL)
                                ++j;
                            if(source1 == LBS_MANUAL)
                            {
                                if(j != prop->values.end())
                                {
                                    if(!getColour(j, prop->values.end(), &arg1, 3))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "valid colour expected when src_manual is used");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "valid colour expected when src_manual is used");
                                }
                            }
                            if(source2 == LBS_MANUAL)
                            {
                                if(j != prop->values.end())
                                {
                                    if(!getColour(j, prop->values.end(), &arg2, 3))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "valid colour expected when src_manual is used");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "valid colour expected when src_manual is used");
                                }
                            }

                            mUnit->setColourOperationEx(op, source1, source2, arg1, arg2, manualBlend);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_COLOUR_OP_MULTIPASS_FALLBACK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "colour_op_multiplass_fallback must have at most 2 arguments");
                    }
                    else if(prop->values.size() == 1)
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_ADD:
                                mUnit->setColourOpMultipassFallback(SBF_ONE, SBF_ONE);
                                break;
                            case ID_MODULATE:
                                mUnit->setColourOpMultipassFallback(SBF_DEST_COLOUR, SBF_ZERO);
                                break;
                            case ID_COLOUR_BLEND:
                                mUnit->setColourOpMultipassFallback(SBF_SOURCE_COLOUR, SBF_ONE_MINUS_SOURCE_COLOUR);
                                break;
                            case ID_ALPHA_BLEND:
                                mUnit->setColourOpMultipassFallback(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
                                break;
                            case ID_REPLACE:
                                mUnit->setColourOpMultipassFallback(SBF_ONE, SBF_ZERO);
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "argument must be a valid scene blend type (add, modulate, colour_blend, alpha_blend, or replace)");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "argument must be a valid scene blend type (add, modulate, colour_blend, alpha_blend, or replace)");
                        }
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        SceneBlendFactor sbf0, sbf1;
                        if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
                            mUnit->setColourOpMultipassFallback(sbf0, sbf1);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "arguments must be valid scene blend factors");
                    }
                    break;
                case ID_ALPHA_OP_EX:
                    if(prop->values.size() < 3)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "alpha_op_ex must have at least 3 arguments");
                    }
                    else if(prop->values.size() > 6)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "alpha_op_ex must have at most 6 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                            i1 = getNodeAt(prop->values, 1),
                            i2 = getNodeAt(prop->values, 2);
                        LayerBlendOperationEx op;
                        LayerBlendSource source1, source2;
                        if (getValue(*i0, op) && getValue(*i1, source1) && getValue(*i2, source2))
                        {
                            Real arg1 = 0.0f, arg2 = 0.0f;
                            Real manualBlend = 0.0f;

                            if(op == LBX_BLEND_MANUAL)
                            {
                                AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
                                if(i3 != prop->values.end())
                                {
                                    if(!getReal(*i3, &manualBlend))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "valid number expected when blend_manual is used");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "valid number expected when blend_manual is used");
                                }
                            }

                            AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
                            if(op == LBX_BLEND_MANUAL)
                                ++j;
                            if(source1 == LBS_MANUAL)
                            {
                                if(j != prop->values.end())
                                {
                                    if(!getReal(*j, &arg1))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "valid colour expected when src_manual is used");
                                    else
                                        ++j;
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "valid colour expected when src_manual is used");
                                }
                            }
                            if(source2 == LBS_MANUAL)
                            {
                                if(j != prop->values.end())
                                {
                                    if(!getReal(*j, &arg2))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "valid colour expected when src_manual is used");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "valid colour expected when src_manual is used");
                                }
                            }

                            mUnit->setAlphaOperation(op, source1, source2, arg1, arg2, manualBlend);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_ENV_MAP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "env_map must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ScriptCompiler::ID_OFF:
                                mUnit->setEnvironmentMap(false);
                                break;
                            case ID_SPHERICAL:
                                mUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
                                break;
                            case ID_PLANAR:
                                mUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
                                break;
                            case ID_CUBIC_REFLECTION:
                                mUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
                                break;
                            case ID_CUBIC_NORMAL:
                                mUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid argument (must be \"off\", \"spherical\", \"planar\", \"cubic_reflection\", or \"cubic_normal\")");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid argument (must be \"off\", \"spherical\", \"planar\", \"cubic_reflection\", or \"cubic_normal\")");
                        }
                    }
                    break;
                case ID_SCROLL:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scroll must have at most 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        Real x, y;
                        if(getReal(*i0, &x) && getReal(*i1, &y))
                            mUnit->setTextureScroll(x, y);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*i0)->getValue() + " and/or " + (*i1)->getValue() + " is invalid; both must be numbers");
                    }
                    break;
                case ID_SCROLL_ANIM:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scroll_anim must have at most 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        Real x, y;
                        if(getReal(*i0, &x) && getReal(*i1, &y))
                            mUnit->setScrollAnimation(x, y);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*i0)->getValue() + " and/or " + (*i1)->getValue() + " is invalid; both must be numbers");
                    }
                    break;
                case ID_ROTATE:
                    if(getValue(prop, compiler, fval))
                        mUnit->setTextureRotate(Degree(fval));
                    break;
                case ID_ROTATE_ANIM:
                    if(getValue(prop, compiler, fval))
                        mUnit->setRotateAnimation(fval);
                    break;
                case ID_SCALE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scale must have at most 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        Real x, y;
                        if(getReal(*i0, &x) && getReal(*i1, &y))
                            mUnit->setTextureScale(x, y);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "first and second arguments must both be valid number values (received " + (*i0)->getValue() + ", " + (*i1)->getValue() + ")");
                    }
                    break;
                case ID_WAVE_XFORM:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 6)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "wave_xform must have at most 6 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
                            i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3),
                            i4 = getNodeAt(prop->values, 4), i5 = getNodeAt(prop->values, 5);
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM &&
                           (*i3)->type == ANT_ATOM && (*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
                            TextureUnitState::TextureTransformType type = TextureUnitState::TT_ROTATE;
                            WaveformType wave = WFT_SINE;
                            Real base = 0.0f, freq = 0.0f, phase = 0.0f, amp = 0.0f;

                            switch(atom0->id)
                            {
                            case ID_SCROLL_X:
                                type = TextureUnitState::TT_TRANSLATE_U;
                                break;
                            case ID_SCROLL_Y:
                                type = TextureUnitState::TT_TRANSLATE_V;
                                break;
                            case ID_SCALE_X:
                                type = TextureUnitState::TT_SCALE_U;
                                break;
                            case ID_SCALE_Y:
                                type = TextureUnitState::TT_SCALE_V;
                                break;
                            case ID_ROTATE:
                                type = TextureUnitState::TT_ROTATE;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom0->value + " is not a valid transform type (must be \"scroll_x\", \"scroll_y\", \"scale_x\", \"scale_y\", or \"rotate\")");
                            }

                            switch(atom1->id)
                            {
                            case ID_SINE:
                                wave = WFT_SINE;
                                break;
                            case ID_TRIANGLE:
                                wave = WFT_TRIANGLE;
                                break;
                            case ID_SQUARE:
                                wave = WFT_SQUARE;
                                break;
                            case ID_SAWTOOTH:
                                wave = WFT_SAWTOOTH;
                                break;
                            case ID_INVERSE_SAWTOOTH:
                                wave = WFT_INVERSE_SAWTOOTH;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom1->value + " is not a valid waveform type (must be \"sine\", \"triangle\", \"square\", \"sawtooth\", or \"inverse_sawtooth\")");
                            }

                            if(!getReal(*i2, &base) || !getReal(*i3, &freq) || !getReal(*i4, &phase) || !getReal(*i5, &amp))
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "arguments 3, 4, 5, and 6 must be valid numbers; received " + (*i2)->getValue() + ", " + (*i3)->getValue() + ", " + (*i4)->getValue() + ", " + (*i5)->getValue());

                            mUnit->setTransformAnimation(type, wave, base, freq, phase, amp);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_TRANSFORM:
                    {
                        Matrix4 m;
                        if(getMatrix4(prop->values.begin(), prop->values.end(), &m))
                            mUnit->setTextureTransform(m);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_CONTENT_TYPE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "content_type must have at most 4 arguments");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_NAMED:
                                mUnit->setContentType(TextureUnitState::CONTENT_NAMED);
                                break;
                            case ID_SHADOW:
                                mUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
                                break;
                            case ID_COMPOSITOR:
                                mUnit->setContentType(TextureUnitState::CONTENT_COMPOSITOR);
                                if (prop->values.size() >= 3)
                                {
                                    String compositorName;
                                    getString(*getNodeAt(prop->values, 1), &compositorName);
                                    String textureName;
                                    getString(*getNodeAt(prop->values, 2), &textureName);

                                    if (prop->values.size() == 4)
                                    {
                                        uint32 mrtIndex;
                                        getUInt(*getNodeAt(prop->values, 3), (uint32*)&mrtIndex);
                                        mUnit->setCompositorReference(compositorName, textureName, mrtIndex);
                                    }
                                    else
                                    {
                                        mUnit->setCompositorReference(compositorName, textureName);
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "content_type compositor must have an additional 2 or 3 parameters");
                                }

                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom->value + " is not a valid content type (must be \"named\" or \"shadow\" or \"compositor\")");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid content type");
                        }
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
        }
    }

    /**************************************************************************
     * TextureSourceTranslator
     **************************************************************************/
    TextureSourceTranslator::TextureSourceTranslator()
    {
    }
    //-------------------------------------------------------------------------
    void TextureSourceTranslator::translate(Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // It has to have one value identifying the texture source name
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, node->file, node->line,
                               "texture_source requires a type value");
            return;
        }

        // Set the value of the source
        ExternalTextureSourceManager::getSingleton().setCurrentPlugIn(obj->values.front()->getValue());

        if (!ExternalTextureSourceManager::getSingleton().getCurrentPlugIn())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line,
                               obj->values.front()->getValue());
            return;
        }

        // Set up the technique, pass, and texunit levels
        TextureUnitState *texunit = any_cast<TextureUnitState*>(obj->parent->context);
        Pass *pass = texunit->getParent();
        Technique *technique = pass->getParent();
        Material *material = technique->getParent();

        unsigned short techniqueIndex = 0, passIndex = 0, texUnitIndex = 0;
        for(unsigned short i = 0; i < material->getNumTechniques(); i++)
        {
            if(material->getTechnique(i) == technique)
            {
                techniqueIndex = i;
                break;
            }
        }
        for(unsigned short i = 0; i < technique->getNumPasses(); i++)
        {
            if(technique->getPass(i) == pass)
            {
                passIndex = i;
                break;
            }
        }
        for(unsigned short i = 0; i < pass->getNumTextureUnitStates(); i++)
        {
            if(pass->getTextureUnitState(i) == texunit)
            {
                texUnitIndex = i;
                break;
            }
        }

        String tps;
        tps = StringConverter::toString(techniqueIndex) + " "
            + StringConverter::toString(passIndex) + " "
            + StringConverter::toString(texUnitIndex);

        ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( "set_T_P_S", tps );

        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = (PropertyAbstractNode*)i.get();
                // Glob the property values all together
                String str = "";
                for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
                {
                    if(j != prop->values.begin())
                        str = str + " ";
                    str = str + (*j)->getValue();
                }
                ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter(prop->name, str);
            }
            else if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
        }

        ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->createDefinedTexture(material->getName(), material->getGroup());
    }

    /**************************************************************************
     * GpuProgramTranslator
     *************************************************************************/
    GpuProgramTranslator::GpuProgramTranslator()
    {
    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // Must have a name
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
                               "gpu program object must have names");
            return;
        }

        // Must have a language type
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line,
                               "gpu program object require language declarations");
            return;
        }

        // Get the language
        String language;
        for(const auto& lnode : obj->values)
        {
            if(!getString(lnode, &language))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
                return;
            }

            if (language == "asm")
            {
                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, obj->file, obj->line, "asm. Use syntax code.");
                break; // always supported
            }
            if (GpuProgramManager::getSingleton().isLanguageSupported(language))
                break;
        }

        translateGpuProgram(compiler, obj, language);
    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translateGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj, String language)
    {
        String syntax;
        std::vector<String> delegates;
        std::vector<std::pair<PropertyAbstractNode*, String> > customParameters;
        String source, profiles, target;
        AbstractNodePtr params;
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = (PropertyAbstractNode*)i.get();
                if(prop->id == ID_SOURCE)
                {
                    if(!getValue(prop, compiler, source))
                        return;
                }
                else if(prop->name == "delegate")
                {
                    String value;
                    if(!getValue(prop, compiler, value))
                        return;

                    ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, value);
                    compiler->_fireEvent(&evt, 0);
                    delegates.push_back(evt.mName);
                }
                else
                {
                    String value;
                    bool first = true;
                    for(AbstractNodeList::iterator it = prop->values.begin(); it != prop->values.end(); ++it)
                    {
                        if((*it)->type == ANT_ATOM)
                        {
                            if(!first)
                                value += " ";
                            else
                                first = false;

                            if(prop->name == "attach")
                            {
                                ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, ((AtomAbstractNode*)(*it).get())->value);
                                compiler->_fireEvent(&evt, 0);
                                value += evt.mName;

                                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file,
                                                   prop->line,
                                                   "attach. Use the #include directive instead");
                            }
                            else
                            {
                                value += ((AtomAbstractNode*)(*it).get())->value;
                            }
                        }
                    }

                    if(prop->name == "profiles")
                        profiles = value;
                    else if(prop->name == "target")
                        target = value;
                    else if(prop->id == ID_SYNTAX && language == "asm")
                        syntax = value;
                    else
                        customParameters.push_back(std::make_pair(prop, value));
                }
            }
            else if(i->type == ANT_OBJECT)
            {
                if(((ObjectAbstractNode*)i.get())->id == ID_DEFAULT_PARAMS)
                    params = i;
                else
                    processNode(compiler, i);
            }
        }

        // Allocate the program
        GpuProgramType gpt = translateIDToGpuProgramType(obj->id);
        GpuProgram *prog = 0;

        if(language == "asm")
            language = syntax;
        CreateGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), source,
                                                language, gpt);
        bool processed = compiler->_fireEvent(&evt, &prog);

        if(!processed)
        {
            prog = GpuProgramManager::getSingleton().create(obj->name, compiler->getResourceGroup(), gpt, language).get();

            if (source.empty() && language != "unified")
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line,
                                   "No 'source' provided for GPU program");
                return;
            }

            if(prog && !source.empty()) // prog=0 if duplicate definition resolved by "use previous"
                prog->setSourceFile(source);
        }

        if(!prog)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, obj->name);
            return;
        }

        obj->context = prog;

        prog->setMorphAnimationIncluded(false);
        prog->setPoseAnimationIncluded(0);
        prog->setSkeletalAnimationIncluded(false);
        prog->setVertexTextureFetchRequired(false);
        prog->_notifyOrigin(obj->file);

        // special case for Cg
        if(!profiles.empty())
            prog->setParameter("profiles", profiles);

        // special case for HLSL
        if(!target.empty())
            prog->setParameter("target", target);

        // special case for unified
        for(const auto& d : delegates)
            prog->setParameter("delegate", d);

        // Set the custom parameters
        for(const auto& p : customParameters)
        {
            if(prog->isSupported() && !prog->setParameter(p.first->name, p.second))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, p.first->file, p.first->line, p.first->name);
            }
        }

        // Set up default parameters
        if(prog->isSupported() && params)
        {
            GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, ptr, static_cast<ObjectAbstractNode*>(params.get()));
        }
    }
    //-------------------------------------------------------------------------
    static int parseProgramParameterDimensions(String& declarator, BaseConstantType& type)
    {
        // Assume 1 unless otherwise specified
        int dimensions = 1;
        type = BCT_UNKNOWN;

        // get the type
        const char* typeStrings[] = {"float", "int", "uint", "double", "bool"};
        BaseConstantType baseTypes[] = {BCT_FLOAT, BCT_INT, BCT_UINT, BCT_DOUBLE, BCT_BOOL};

        const char* typeStr = "";

        for(int i = 0; i < 5; ++i)
        {
            if(declarator.find(typeStrings[i]) == 0)
            {
                type = baseTypes[i];
                typeStr = typeStrings[i];
                break;
            }
        }

        if(type == BCT_UNKNOWN)
            return dimensions;

        size_t start = declarator.find_first_not_of(typeStr);

        if (start != String::npos)
        {
            size_t end = declarator.find_first_of('[', start);

            // int1, int2, etc.
            if (end != start)
            {
                dimensions *= StringConverter::parseInt(
                    declarator.substr(start, end - start));
                start = end;
            }

            // C-style array
            while (start != String::npos)
            {
                end = declarator.find_first_of(']', start);
                dimensions *= StringConverter::parseInt(
                    declarator.substr(start + 1, end - start - 1));
                start = declarator.find_first_of('[', end);
            }
        }
        
        return dimensions; 
    }
    //-------------------------------------------------------------------------
    template <typename T, typename It>
    static void safeSetConstant(const GpuProgramParametersPtr& params, const String& name, size_t index, It arrayStart,
                                It arrayEnd, size_t count, PropertyAbstractNode* prop, ScriptCompiler* compiler)
    {
        int roundedCount = (count + 3) / 4; // integer ceil
        roundedCount *= 4;

        std::vector<T> vals;
        if (!_getVector(arrayStart, arrayEnd, vals, roundedCount))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return;
        }

        try
        {
            if (!name.empty())
                params->setNamedConstant(name, vals.data(), count, 1);
            else
                params->setConstant(index, vals.data(), roundedCount / 4);
        }
        catch (Exception& e)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, e.getDescription());
        }
    }

    void GpuProgramTranslator::translateProgramParameters(ScriptCompiler *compiler, const GpuProgramParametersSharedPtr& params, ObjectAbstractNode *obj)
    {
        uint32 animParametricsCount = 0;

        String value;
        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_SHARED_PARAMS_REF:
                    if(getValue(prop, compiler, value))
                    {
                        try
                        {
                            params->addSharedParameters(value);
                        }
                        catch(Exception& e)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               e.getDescription());
                        }
                    }
                    break;
                //TODO Refactor this case.
                case ID_PARAM_INDEXED:
                case ID_PARAM_NAMED:
                    {
                        if(prop->values.size() >= 3)
                        {
                            bool named = (prop->id == ID_PARAM_NAMED);
                            AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
                                k = getNodeAt(prop->values, 2);

                            if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "name or index and parameter type expected");
                                return;
                            }

                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
                            if(!named && !StringConverter::isNumber(atom0->value))
                            {
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   "parameter index expected");
                                return;
                            }

                            String name;
                            size_t index = 0;
                            // Assign the name/index
                            if(named)
                                name = atom0->value;
                            else
                                index = StringConverter::parseInt(atom0->value);

                            // Determine the type
                            if(atom1->value == "matrix4x4")
                            {
                                Matrix4 m;
                                if(getMatrix4(k, prop->values.end(), &m))
                                {
                                    try
                                    {
                                        if(named)
                                            params->setNamedConstant(name, m);
                                        else
                                            params->setConstant(index, m);
                                    }
                                    catch (Exception& e)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           e.getDescription());
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "incorrect matrix4x4 declaration");
                                }
                            }
                            else
                            {
                                int count;
                                BaseConstantType type;

                                // First, clear out any offending auto constants
                                if (named)
                                    params->clearNamedAutoConstant(name);
                                else
                                    params->clearAutoConstant(index);

                                count = parseProgramParameterDimensions(atom1->value, type);

                                if (type == BCT_FLOAT)
                                {
                                    safeSetConstant<float>(params, name, index, k, prop->values.cend(), count, prop, compiler);
                                }
                                else if (type == BCT_UINT)
                                {
                                    safeSetConstant<uint>(params, name, index, k, prop->values.cend(), count, prop, compiler);
                                }
                                else if (type == BCT_INT)
                                {
                                    safeSetConstant<int>(params, name, index, k, prop->values.cend(), count, prop, compiler);
                                }
                                else if (type == BCT_DOUBLE)
                                {
                                    safeSetConstant<double>(params, name, index, k, prop->values.cend(), count, prop, compiler);
                                }                                
                                else if (type == BCT_BOOL)
                                {
                                    std::vector<bool> tmp;
                                    int roundedCount = (count + 3) / 4; // integer ceil
                                    roundedCount *= 4;
                                    if (_getVector(k, prop->values.end(), tmp, roundedCount))
                                    {
                                        std::vector<uint> vals(tmp.begin(), tmp.end());
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals.data(), count, 1);
                                            else
                                                params->setConstant(index, vals.data(), roundedCount/4);
                                        }
                                        catch (Exception& e)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               e.getDescription());
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "incorrect boolean constant declaration");
                                    }

                                    compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                                       "bool. Use uint instead");
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "incorrect type specified; only variants of int, uint, float, double, and bool allowed");
                                }
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "param_named and param_indexed properties requires at least 3 arguments");
                        }
                    }
                    break;
                case ID_PARAM_INDEXED_AUTO:
                case ID_PARAM_NAMED_AUTO:
                    {
                        bool named = (prop->id == ID_PARAM_NAMED_AUTO);
                        String name;

                        if(prop->values.size() >= 2)
                        {
                            size_t index = 0;
                            AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                                i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3);
                            if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "name or index and auto constant type expected");
                                return;
                            }
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
                            if(!named && !StringConverter::isNumber(atom0->value))
                            {
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                   "parameter index expected");
                                return;
                            }

                            if(named)
                                name = atom0->value;
                            else
                                index = StringConverter::parseInt(atom0->value);

                            // Look up the auto constant
                            StringUtil::toLowerCase(atom1->value);
                            const GpuProgramParameters::AutoConstantDefinition *def =
                                GpuProgramParameters::getAutoConstantDefinition(atom1->value);
                            if(def)
                            {
                                switch(def->dataType)
                                {
                                case GpuProgramParameters::ACDT_NONE:
                                    if (i2 != prop->values.end())
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file,
                                                           prop->line, "unexpected <extraInfo> parameter");
                                    }

                                    // Set the auto constant
                                    try
                                    {
                                        if(named)
                                            params->setNamedAutoConstant(name, def->acType);
                                        else
                                            params->setAutoConstant(index, def->acType);
                                    }
                                    catch(Exception& e)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           e.getDescription());
                                    }
                                    break;
                                case GpuProgramParameters::ACDT_INT:
                                    if(def->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
                                    {
                                        try
                                        {
                                            if(named)
                                                params->setNamedAutoConstant(name, def->acType, animParametricsCount++);
                                            else
                                                params->setAutoConstant(index, def->acType, animParametricsCount++);
                                        }
                                        catch(Exception& e)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               e.getDescription());
                                        }
                                    }
                                    else
                                    {
                                        // Only certain texture projection auto params will assume 0
                                        // Otherwise we will expect that 3rd parameter
                                        if(i2 == prop->values.end())
                                        {
                                            if(def->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX ||
                                               def->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
                                               def->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
                                               def->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX
                                            )
                                            {
                                                try
                                                {
                                                    if(named)
                                                        params->setNamedAutoConstant(name, def->acType, 0);
                                                    else
                                                        params->setAutoConstant(index, def->acType, 0);
                                                }
                                                catch (Exception& e)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       e.getDescription());
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                                   "<extraInfo> parameter required by constant definition " + atom1->value);
                                            }
                                        }
                                        else
                                        {
                                            bool success = false;
                                            uint32 extraInfo = 0;
                                            if(i3 == prop->values.end())
                                            { // Handle only one extra value
                                                if(getUInt(*i2, &extraInfo))
                                                {
                                                    success = true;
                                                }
                                            }
                                            else
                                            { // Handle two extra values
                                                uint32 extraInfo1 = 0, extraInfo2 = 0;
                                                if(getUInt(*i2, &extraInfo1) && getUInt(*i3, &extraInfo2))
                                                {
                                                    extraInfo = extraInfo1 | (extraInfo2 << 16);
                                                    success = true;
                                                }
                                            }

                                            if(success)
                                            {
                                                try
                                                {
                                                    if(named)
                                                        params->setNamedAutoConstant(name, def->acType, extraInfo);
                                                    else
                                                        params->setAutoConstant(index, def->acType, extraInfo);
                                                }
                                                catch (Exception& e)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       e.getDescription());
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                   "invalid auto constant <extraInfo> parameter");
                                            }
                                        }
                                    }
                                    break;
                                case GpuProgramParameters::ACDT_REAL:
                                    if(def->acType == GpuProgramParameters::ACT_TIME ||
                                       def->acType == GpuProgramParameters::ACT_FRAME_TIME)
                                    {
                                        Real f = 1.0f;
                                        if(i2 != prop->values.end())
                                            getReal(*i2, &f);

                                        try
                                        {
                                            if(named)
                                                params->setNamedAutoConstantReal(name, def->acType, f);
                                            else
                                                params->setAutoConstantReal(index, def->acType, f);
                                        }
                                        catch (Exception& e)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               e.getDescription());
                                        }
                                    }
                                    else
                                    {
                                        if(i2 != prop->values.end())
                                        {
                                            Real extraInfo = 0.0f;
                                            if(getReal(*i2, &extraInfo))
                                            {
                                                try
                                                {
                                                    if(named)
                                                        params->setNamedAutoConstantReal(name, def->acType, extraInfo);
                                                    else
                                                        params->setAutoConstantReal(index, def->acType, extraInfo);
                                                }
                                                catch(Exception& e)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       e.getDescription());
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                   "incorrect float argument definition in <extraInfo> parameter");
                                            }
                                        }
                                        else
                                        {
                                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                               "<extraInfo> parameter required by constant definition " + atom1->value);
                                        }
                                    }
                                    break;
                                }
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, atom1->value);
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                // case ID_COMPUTE_WORKGROUP_DIMENSIONS:
                //     if (obj->id != ID_COMPUTE_PROGRAM)
                //         break;
                //     if (prop->values.size() == 3)
                //     {
                //         AbstractNodeList::const_iterator first_dimension = getNodeAt(prop->values, 0);

                //         uint *vals = OGRE_ALLOC_T(uint, roundedCount, MEMCATEGORY_SCRIPTING);
                //         if(getUInts(k, prop->values.end(), vals, 3))
                //         {
                //             try
                //             {
                //                 params->setNamedConstant(name, m);
                //             }
                //             catch(...)
                //             {
                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                //                                    "setting workgroup_dimensions parameter failed");
                //             }
                //         }
                //         else
                //         {
                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                //                                "incorrect workgroup_dimensions declaration");
                //         }
                //     }
                //     else
                //     {
                //         compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                //                            "workgroup_dimensions property requires 3 arguments");
                //     }

                    
                //     break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }
    /**************************************************************************
     * SharedParamsTranslator
     *************************************************************************/
    SharedParamsTranslator::SharedParamsTranslator()
    {
    }

    //-------------------------------------------------------------------------
    void SharedParamsTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // Must have a name
        if (obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        GpuSharedParameters* sharedParams = 0;
        CreateGpuSharedParametersScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&sharedParams);

        if (!processed)
        {
            sharedParams = GpuProgramManager::getSingleton().createSharedParameters(obj->name).get();
        }

        if (!sharedParams)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, obj->name);
            return;
        }

        for (auto & i : obj->children)
        {
            if (i->type != ANT_PROPERTY)
                continue;

            PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
            if (prop->id != ID_SHARED_PARAM_NAMED)
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   prop->name);
                continue;
            }

            if (prop->values.size() < 2)
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   "shared_param_named - expected 2 or more arguments");
                continue;
            }

            AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);

            String pName;
            GpuConstantType constType;

            if (!getValue(*i0, pName) || !getConstantType(i1, &constType))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   "name and parameter type expected");
                continue;
            }

            AbstractNodeList::const_iterator arrayStart = getNodeAt(prop->values, 2), arrayEnd = prop->values.end();
            uint32 arraySz = 1;

            if (arrayStart != arrayEnd)
            {
                String value;
                getValue(*arrayStart, value);

                if (value.front() == '[' && value.back() == ']')
                {
                    arraySz = StringConverter::parseInt(value.substr(1, value.size() - 2), 0);
                    if(!arraySz)
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                           "invalid array size");
                        continue;
                    }
                    arrayStart++;
                }
            }

            // define constant entry
            try
            {
                sharedParams->addConstantDefinition(pName, constType, arraySz);
            }
            catch(Exception& e)
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   e.getDescription());
                continue;
            }

            // amount of individual numbers to read
            arraySz *= GpuConstantDefinition::getElementSize(constType, false);

            switch (GpuConstantDefinition::getBaseType(constType))
            {
            case BCT_FLOAT:
            {
                std::vector<float> values;
                if(_getVector(arrayStart, arrayEnd, values, arraySz))
                    sharedParams->setNamedConstant(pName, &values[0], arraySz);
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                break;
            }
            case BCT_INT:
            {
                std::vector<int> values;
                if(_getVector(arrayStart, arrayEnd, values, arraySz))
                    sharedParams->setNamedConstant(pName, &values[0], arraySz);
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                break;
            }
            case BCT_DOUBLE:
            {
                std::vector<double> values;
                if(_getVector(arrayStart, arrayEnd, values, arraySz))
                    sharedParams->setNamedConstant(pName, &values[0], arraySz);
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                break;
            }
            case BCT_UINT:
            {
                std::vector<uint> values;
                if(_getVector(arrayStart, arrayEnd, values, arraySz))
                {
                    sharedParams->setNamedConstant(pName, &values[0], arraySz);
                }
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                break;
            }
            case BCT_BOOL:
            {
                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line,
                                   "bool. Use uint instead");
                std::vector<bool> tmp;
                if(_getVector(arrayStart, arrayEnd, tmp, arraySz))
                {
                    std::vector<uint> values(tmp.begin(), tmp.end());
                    sharedParams->setNamedConstant(pName, &values[0], arraySz);
                }
                else
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                break;
            }
            default:
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   "invalid parameter type");
                break;
            }
        }
    }

    /**************************************************************************
     * ParticleSystemTranslator
     *************************************************************************/
    ParticleSystemTranslator::ParticleSystemTranslator()
        :mSystem(0)
    {
    }

    void ParticleSystemTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
        // Find the name
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Allocate the particle system
        CreateParticleSystemScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mSystem);

        if(!processed)
        {
            mSystem = ParticleSystemManager::getSingleton().createTemplate(obj->name, compiler->getResourceGroup());
        }

        if(!mSystem)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, obj->name);
            return;
        }

        mSystem->_notifyOrigin(obj->file);

        mSystem->removeAllEmitters();
        mSystem->removeAllAffectors();

        obj->context = mSystem;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_MATERIAL:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            String name = ((AtomAbstractNode*)prop->values.front().get())->value;

                            ProcessResourceNameScriptCompilerEvent locEvt(ProcessResourceNameScriptCompilerEvent::MATERIAL, name);
                            compiler->_fireEvent(&locEvt, 0);

                            if(!mSystem->setParameter("material", locEvt.mName))
                            {
                                if(mSystem->getRenderer())
                                {
                                    if(!mSystem->getRenderer()->setParameter("material", locEvt.mName))
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "material property could not be set with material \"" + locEvt.mName + "\"");
                                }
                            }
                        }
                    }
                    break;
                default:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        String name = prop->name, value;

                        // Glob the values together
                        for(auto& v : prop->values)
                        {
                            if(v->type == ANT_ATOM)
                            {
                                if(value.empty())
                                    value = ((AtomAbstractNode*)v.get())->value;
                                else
                                    value = value + " " + ((AtomAbstractNode*)v.get())->value;
                            }
                            else
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                return;
                            }
                        }

                        if(!mSystem->setParameter(name, value))
                        {
                            if(mSystem->getRenderer())
                            {
                                if(!mSystem->getRenderer()->setParameter(name, value))
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                            }
                        }
                    }
                }
            }
            else
            {
                processNode(compiler, i);
            }
        }
    }

    /**************************************************************************
     * ParticleEmitterTranslator
     *************************************************************************/
    ParticleEmitterTranslator::ParticleEmitterTranslator()
        :mEmitter(0)
    {
    }
    //-------------------------------------------------------------------------
    void ParticleEmitterTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // Must have a type as the first value
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
            return;
        }

        ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);

        try
        {
            mEmitter = system->addEmitter(obj->values.front()->getString());
        }
        catch(Exception &e)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line, e.getDescription());
            return;
        }

        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                String value;

                // Glob the values together
                for(AbstractNodeList::iterator it = prop->values.begin(); it != prop->values.end(); ++it)
                {
                    if((*it)->type == ANT_ATOM)
                    {
                        if(value.empty())
                            value = ((AtomAbstractNode*)(*it).get())->value;
                        else
                            value = value + " " + ((AtomAbstractNode*)(*it).get())->value;
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        break;
                    }
                }

                if(!mEmitter->setParameter(prop->name, value))
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                }
            }
            else
            {
                processNode(compiler, i);
            }
        }
    }

    /**************************************************************************
     * ParticleAffectorTranslator
     *************************************************************************/
    ParticleAffectorTranslator::ParticleAffectorTranslator()
        :mAffector(0)
    {
    }
    //-------------------------------------------------------------------------
    void ParticleAffectorTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // Must have a type as the first value
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
            return;
        }

        ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);
        try
        {
            mAffector = system->addAffector(obj->values.front()->getString());
        }
        catch(Exception &e)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line, e.getDescription());
            return;
        }

        for(auto & i : obj->children)
        {
            if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                String value;

                // Glob the values together
                for(AbstractNodeList::iterator it = prop->values.begin(); it != prop->values.end(); ++it)
                {
                    if((*it)->type == ANT_ATOM)
                    {
                        if(value.empty())
                            value = ((AtomAbstractNode*)(*it).get())->value;
                        else
                            value = value + " " + ((AtomAbstractNode*)(*it).get())->value;
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        break;
                    }
                }

                if(!mAffector->setParameter(prop->name, value))
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                }
            }
            else
            {
                processNode(compiler, i);
            }
        }
    }

    /**************************************************************************
     * CompositorTranslator
     *************************************************************************/
    CompositorTranslator::CompositorTranslator()
        :mCompositor(0)
    {
    }

    void CompositorTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Create the compositor
        CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mCompositor);

        if(!processed)
        {
            mCompositor = CompositorManager::getSingleton().create(obj->name, compiler->getResourceGroup()).get();
        }

        if(!mCompositor)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line, obj->name);
            return;
        }

        // Prepare the compositor
        mCompositor->removeAllTechniques();
        mCompositor->_notifyOrigin(obj->file);
        obj->context = mCompositor;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, i->file, i->line,
                                   "token not recognized");
            }
        }
    }

    /**************************************************************************
     * CompositionTechniqueTranslator
     *************************************************************************/
    CompositionTechniqueTranslator::CompositionTechniqueTranslator()
        :mTechnique(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositionTechniqueTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        Compositor *compositor = any_cast<Compositor*>(obj->parent->context);
        mTechnique = compositor->createTechnique();
        obj->context = mTechnique;

        String sval;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
            else if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_TEXTURE:
                    {
                        size_t atomIndex = 1;

                        AbstractNodeList::const_iterator it = getNodeAt(prop->values, 0);

                        if((*it)->type != ANT_ATOM)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                            return;
                        }
                        // Save the first atom, should be name
                        AtomAbstractNode *atom0 = (AtomAbstractNode*)(*it).get();

                        uint32 width = 0, height = 0;
                        float widthFactor = 1.0f, heightFactor = 1.0f;
                        bool widthSet = false, heightSet = false, formatSet = false;
                        bool pooled = false;
                        bool hwGammaWrite = false;
                        bool fsaa = true;
                        auto type = TEX_TYPE_2D;
                        uint16 depthBufferId = DepthBuffer::POOL_DEFAULT;
                        CompositionTechnique::TextureScope scope = CompositionTechnique::TS_LOCAL;
                        Ogre::PixelFormatList formats;

                        while (atomIndex < prop->values.size())
                        {
                            it = getNodeAt(prop->values, static_cast<int>(atomIndex++));
                            if((*it)->type != ANT_ATOM)
                            {
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                return;
                            }
                            AtomAbstractNode *atom = (AtomAbstractNode*)(*it).get();

                            switch(atom->id)
                            {
                            case ID_TARGET_WIDTH:
                                width = 0;
                                widthSet = true;
                                break;
                            case ID_TARGET_HEIGHT:
                                height = 0;
                                heightSet = true;
                                break;
                            case ID_TARGET_WIDTH_SCALED:
                            case ID_TARGET_HEIGHT_SCALED:
                                {
                                    bool *pSetFlag;
                                    uint32 *pSize;
                                    float *pFactor;

                                    if (atom->id == ID_TARGET_WIDTH_SCALED)
                                    {
                                        pSetFlag = &widthSet;
                                        pSize = &width;
                                        pFactor = &widthFactor;
                                    }
                                    else
                                    {
                                        pSetFlag = &heightSet;
                                        pSize = &height;
                                        pFactor = &heightFactor;
                                    }
                                    // advance to next to get scaling
                                    it = getNodeAt(prop->values, static_cast<int>(atomIndex++));
                                    if(prop->values.end() == it || (*it)->type != ANT_ATOM)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        return;
                                    }
                                    atom = (AtomAbstractNode*)(*it).get();
                                    if (!StringConverter::isNumber(atom->value))
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        return;
                                    }

                                    *pSize = 0;
                                    *pFactor = StringConverter::parseReal(atom->value);
                                    *pSetFlag = true;
                                }
                                break;
                            case ID_POOLED:
                                pooled = true;
                                break;
                            case ID_CUBIC:
                                type = TEX_TYPE_CUBE_MAP;
                                break;
                            case ID_SCOPE_LOCAL:
                                scope = CompositionTechnique::TS_LOCAL;
                                break;
                            case ID_SCOPE_CHAIN:
                                scope = CompositionTechnique::TS_CHAIN;
                                break;
                            case ID_SCOPE_GLOBAL:
                                scope = CompositionTechnique::TS_GLOBAL;
                                break;
                            case ID_GAMMA:
                                hwGammaWrite = true;
                                break;
                            case ID_NO_FSAA:
                                fsaa = false;
                                break;
                            case ID_DEPTH_POOL:
                                {
                                    // advance to next to get the ID
                                    it = getNodeAt(prop->values, static_cast<int>(atomIndex++));
                                    if(prop->values.end() == it || (*it)->type != ANT_ATOM)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        return;
                                    }
                                    atom = (AtomAbstractNode*)(*it).get();
                                    if (!StringConverter::isNumber(atom->value))
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        return;
                                    }

                                    depthBufferId = Math::uint16Cast(StringConverter::parseInt(atom->value));
                                }
                                break;
                            default:
                                if (StringConverter::isNumber(atom->value))
                                {
                                    if (atomIndex == 2)
                                    {
                                        width = StringConverter::parseInt(atom->value);
                                        widthSet = true;
                                    }
                                    else if (atomIndex == 3)
                                    {
                                        height = StringConverter::parseInt(atom->value);
                                        heightSet = true;
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        return;
                                    }
                                }
                                else
                                {
                                    // pixel format?
                                    PixelFormat format = PixelUtil::getFormatFromName(atom->value, true);
                                    if (format == PF_UNKNOWN)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, atom->value);
                                        return;
                                    }
                                    formats.push_back(format);
                                    formatSet = true;
                                }

                            }
                        }
                        if (!widthSet || !heightSet || !formatSet)
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }


                        // No errors, create
                        CompositionTechnique::TextureDefinition *def = mTechnique->createTextureDefinition(atom0->value);
                        def->width = width;
                        def->height = height;
                        def->type = type;
                        def->widthFactor = widthFactor;
                        def->heightFactor = heightFactor;
                        def->formatList = formats;
                        def->fsaa = fsaa;
                        def->hwGammaWrite = hwGammaWrite;
                        def->depthBufferId = depthBufferId;
                        def->pooled = pooled;
                        def->scope = scope;
                    }
                    break;
                case ID_TEXTURE_REF:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 3)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                           "texture_ref only supports 3 argument");
                    }
                    else
                    {
                        String texName, refCompName, refTexName;

                        AbstractNodeList::const_iterator it = getNodeAt(prop->values, 0);
                        if(!getString(*it, &texName))
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "texture_ref must have 3 string arguments");

                        it = getNodeAt(prop->values, 1);
                        if(!getString(*it, &refCompName))
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "texture_ref must have 3 string arguments");

                        it = getNodeAt(prop->values, 2);
                        if(!getString(*it, &refTexName))
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "texture_ref must have 3 string arguments");

                        CompositionTechnique::TextureDefinition* refTexDef =
                            mTechnique->createTextureDefinition(texName);

                        refTexDef->refCompName = refCompName;
                        refTexDef->refTexName = refTexName;
                    }
                    break;
                case ID_SCHEME:
                    if(getValue(prop, compiler, sval))
                        mTechnique->setSchemeName(sval);
                    break;
                case ID_COMPOSITOR_LOGIC:
                    if(getValue(prop, compiler, sval))
                        mTechnique->setCompositorLogicName(sval);
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    /**************************************************************************
     * CompositionTargetPass
     *************************************************************************/
    CompositionTargetPassTranslator::CompositionTargetPassTranslator()
        :mTarget(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositionTargetPassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        CompositionTechnique *technique = any_cast<CompositionTechnique*>(obj->parent->context);
        if(obj->id == ID_TARGET)
        {
            mTarget = technique->createTargetPass();
            if(obj->name.empty())
            {
                compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
                return;
            }
            mTarget->setOutputName(obj->name);

            if(!obj->values.empty())
            {
                int val;
                if(getInt(obj->values.front(), &val))
                    mTarget->setOutputSlice(val);
            }
        }
        else if(obj->id == ID_TARGET_OUTPUT)
        {
            mTarget = technique->getOutputTargetPass();
            if (!mTarget->getPasses().empty() || mTarget->getInputMode() != CompositionTargetPass::IM_NONE)
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line,
                                   "target_output can only be used once per technique");
            }
        }
        obj->context = mTarget;

        bool bval;
        Real fval;
        uint32 uival;
        String sval;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
            else if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_INPUT:
                    CompositionTargetPass::InputMode im;
                    if(getValue(prop, compiler, im))
                        mTarget->setInputMode(im);
                    break;
                case ID_ONLY_INITIAL:
                    if(getValue(prop, compiler, bval))
                        mTarget->setOnlyInitial(bval);
                    break;
                case ID_VISIBILITY_MASK:
                    if(getValue(prop, compiler, uival))
                        mTarget->setVisibilityMask(uival);
                    break;
                case ID_LOD_BIAS:
                    if(getValue(prop, compiler, fval))
                        mTarget->setLodBias(fval);
                    break;
                case ID_MATERIAL_SCHEME:
                    if(getValue(prop, compiler, sval))
                        mTarget->setMaterialScheme(sval);
                    break;
                case ID_SHADOWS_ENABLED:
                    if(getValue(prop, compiler, bval))
                        mTarget->setShadowsEnabled(bval);
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    /**************************************************************************
     * CompositionPassTranslator
     *************************************************************************/
    CompositionPassTranslator::CompositionPassTranslator()
        :mPass(0)
    {
    }

    void CompositionPassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        // The name is the type of the pass
        if(obj->values.empty() || obj->values.front()->type != ANT_ATOM)
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line, "pass type missing");
            return;
        }

        AtomAbstractNode* atom = static_cast<AtomAbstractNode*>(obj->values.front().get());

        CompositionPass::PassType ptype;
        switch(atom->id)
        {
            case ID_CLEAR:
                ptype = CompositionPass::PT_CLEAR;
                break;
            case ID_STENCIL:
                ptype = CompositionPass::PT_STENCIL;
                break;
            case ID_RENDER_QUAD:
                ptype = CompositionPass::PT_RENDERQUAD;
                break;
            case ID_RENDER_SCENE:
                ptype = CompositionPass::PT_RENDERSCENE;
                break;
            case ID_COMPUTE:
                ptype = CompositionPass::PT_COMPUTE;
                break;
            case ID_RENDER_CUSTOM:
                ptype = CompositionPass::PT_RENDERCUSTOM;
                break;
            default:
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line, atom->value);
                return;
        }

        CompositionTargetPass *target = any_cast<CompositionTargetPass*>(obj->parent->context);
        mPass = target->createPass(ptype);
        obj->context = mPass;

        if(mPass->getType() == CompositionPass::PT_RENDERCUSTOM) 
        {
            String customType;
            //This is the ugly one liner for safe access to the second parameter.
            if (obj->values.size() < 2 || !getString(*(++(obj->values.begin())), &customType))
            {
                compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
                return;
            }
            mPass->setCustomType(customType);
        }

        Real fval;
        bool bval;
        uint32 uival;
        String sval;
        StencilOperation sop;

        for(auto & i : obj->children)
        {
            if(i->type == ANT_OBJECT)
            {
                processNode(compiler, i);
            }
            else if(i->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>(i.get());
                switch(prop->id)
                {
                case ID_CHECK:
                    if(getValue(prop, compiler, bval))
                        mPass->setStencilCheck(bval);
                    break;
                case ID_COMP_FUNC:
                    CompareFunction func;
                    if(getValue(prop, compiler, func))
                        mPass->setStencilFunc(func);
                    break;
                case ID_REF_VALUE:
                    if(getValue(prop, compiler, uival))
                        mPass->setStencilRefValue(uival);
                    break;
                case ID_MASK:
                    if(getValue(prop, compiler, uival))
                        mPass->setStencilMask(uival);
                    break;
                case ID_FAIL_OP:
                    if(getValue(prop, compiler, sop))
                        mPass->setStencilFailOp(sop);
                    break;
                case ID_DEPTH_FAIL_OP:
                    if(getValue(prop, compiler, sop))
                        mPass->setStencilDepthFailOp(sop);
                    break;
                case ID_PASS_OP:
                    if(getValue(prop, compiler, sop))
                        mPass->setStencilPassOp(sop);
                    break;
                case ID_TWO_SIDED:
                    if(getValue(prop, compiler, bval))
                        mPass->setStencilTwoSidedOperation(bval);
                    break;
                case ID_BUFFERS:
                    {
                        uint32 buffers = 0;
                        for(AbstractNodeList::iterator k = prop->values.begin(); k != prop->values.end(); ++k)
                        {
                            if((*k)->type == ANT_ATOM)
                            {
                                switch(((AtomAbstractNode*)(*k).get())->id)
                                {
                                case ID_COLOUR:
                                    buffers |= FBT_COLOUR;
                                    break;
                                case ID_DEPTH:
                                    buffers |= FBT_DEPTH;
                                    break;
                                case ID_STENCIL:
                                    buffers |= FBT_STENCIL;
                                    break;
                                default:
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                }
                            }
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                        mPass->setClearBuffers(buffers);
                    }
                    break;
                case ID_COLOUR_VALUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                        ColourValue val;
                        if (prop->values.front()->type == ANT_ATOM &&
                            ((AtomAbstractNode*)prop->values.front().get())->id == ID_AUTO)
                            mPass->setAutomaticColour(true);
                        else if(getColour(prop->values.begin(), prop->values.end(), &val))
                            mPass->setClearColour(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_DEPTH_VALUE:
                    if(getValue(prop, compiler, fval))
                        mPass->setClearDepth(fval);
                    break;
                case ID_STENCIL_VALUE:
                    if(getValue(prop, compiler, uival))
                        mPass->setClearStencil(uival);
                    break;
                case ID_MATERIAL:
                    if(getValue(prop, compiler, sval))
                    {
                        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, sval);
                        compiler->_fireEvent(&evt, 0);
                        auto mat = MaterialManager::getSingleton().getByName(evt.mName, compiler->getResourceGroup());
                        if (mat)
                            mPass->setMaterial(mat);
                        else
                            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, prop->file,
                                               prop->line, evt.mName);
                    }
                    break;
                case ID_INPUT:
                    if(prop->values.size() < 2)
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
                        uint32 id;
                        String name;
                        if(getUInt(*i0, &id) && getString(*i1, &name))
                        {
                            uint32 index = 0;
                            if(i2 != prop->values.end())
                            {
                                if(!getUInt(*i2, &index))
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                                    return;
                                }
                            }

                            mPass->setInput(id, name, index);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_IDENTIFIER:
                    if(getValue(prop, compiler, uival))
                        mPass->setIdentifier(uival);
                    break;
                case ID_FIRST_RENDER_QUEUE:
                    if(getValue(prop, compiler, uival))
                        mPass->setFirstRenderQueue(static_cast<uint8>(uival));
                    break;
                case ID_LAST_RENDER_QUEUE:
                    if(getValue(prop, compiler, uival))
                        mPass->setLastRenderQueue(static_cast<uint8>(uival));
                    break;
                case ID_MATERIAL_SCHEME:
                    if(getValue(prop, compiler, sval))
                        mPass->setMaterialScheme(sval);
                    break;
                case ID_THREAD_GROUPS:
                {
                    std::vector<int> g;
                    if(_getVector(prop->values.begin(), prop->values.end(), g, 3))
                        mPass->setThreadGroups({g[0], g[1], g[2]});
                    break;
                }
                case ID_QUAD_NORMALS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            atom = static_cast<AtomAbstractNode*>(prop->values.front().get());
                            if(atom->id == ID_CAMERA_FAR_CORNERS_VIEW_SPACE)
                                mPass->setQuadFarCorners(true, true);
                            else if(atom->id == ID_CAMERA_FAR_CORNERS_WORLD_SPACE)
                                mPass->setQuadFarCorners(true, false);
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_CAMERA:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        if (!getValue(prop->values.front(), sval))
                            return;
                        mPass->setCameraName(sval);
                        if (prop->values.back()->type == ANT_ATOM &&
                            static_cast<AtomAbstractNode*>(prop->values.back().get())->id ==
                                ID_ALIGN_TO_FACE)
                        {
                            mPass->setAlignCameraToFace(true);
                        }
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    /**************************************************************************
     * BuiltinScriptTranslatorManager
     *************************************************************************/
    BuiltinScriptTranslatorManager::BuiltinScriptTranslatorManager()
    {
    }
    //-------------------------------------------------------------------------
    ScriptTranslator *BuiltinScriptTranslatorManager::getTranslator(const AbstractNodePtr &node)
    {
        ScriptTranslator *translator = 0;

        if(node->type == ANT_OBJECT)
        {
            ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
            ObjectAbstractNode *parent = obj->parent ? static_cast<ObjectAbstractNode*>(obj->parent) : 0;
            if(obj->id == ID_MATERIAL)
                translator = &mMaterialTranslator;
            else if(obj->id == ID_TECHNIQUE && parent && parent->id == ID_MATERIAL)
                translator = &mTechniqueTranslator;
            else if(obj->id == ID_PASS && parent && parent->id == ID_TECHNIQUE)
                translator = &mPassTranslator;
            else if(obj->id == ID_TEXTURE_UNIT && parent && parent->id == ID_PASS)
                translator = &mTextureUnitTranslator;
            else if(obj->id == ID_TEXTURE_SOURCE && parent && parent->id == ID_TEXTURE_UNIT)
                translator = &mTextureSourceTranslator;
            else if(obj->id == ID_FRAGMENT_PROGRAM ||
                    obj->id == ID_VERTEX_PROGRAM ||
                    obj->id == ID_GEOMETRY_PROGRAM ||
                    obj->id == ID_TESSELLATION_HULL_PROGRAM || 
                    obj->id == ID_TESSELLATION_DOMAIN_PROGRAM ||
                    obj->id == ID_COMPUTE_PROGRAM)
                translator = &mGpuProgramTranslator;
            else if(obj->id == ID_SHARED_PARAMS)
                translator = &mSharedParamsTranslator;
            else if(obj->id == ID_PARTICLE_SYSTEM)
                translator = &mParticleSystemTranslator;
            else if(obj->id == ID_EMITTER)
                translator = &mParticleEmitterTranslator;
            else if(obj->id == ID_AFFECTOR)
                translator = &mParticleAffectorTranslator;
            else if(obj->id == ID_COMPOSITOR)
                translator = &mCompositorTranslator;
            else if(obj->id == ID_TECHNIQUE && parent && parent->id == ID_COMPOSITOR)
                translator = &mCompositionTechniqueTranslator;
            else if((obj->id == ID_TARGET || obj->id == ID_TARGET_OUTPUT) && parent && parent->id == ID_TECHNIQUE)
                translator = &mCompositionTargetPassTranslator;
            else if(obj->id == ID_PASS && parent && (parent->id == ID_TARGET || parent->id == ID_TARGET_OUTPUT))
                translator = &mCompositionPassTranslator;
            else if(obj->id == ID_SAMPLER)
                translator = &mSamplerTranslator;
        }

        return translator;
    }
}
