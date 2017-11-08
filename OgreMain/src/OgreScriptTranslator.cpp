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
#include "OgreScriptTranslator.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreExternalTextureSourceManager.h"
#include "OgreLodStrategyManager.h"
#include "OgreDistanceLodStrategy.h"
#include "OgreDepthBuffer.h"
#include "OgreParticleSystem.h"
#include "OgreRoot.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreResourceTransition.h"

#include "OgreHlms.h"
#include "OgreHlmsManager.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorShadowNodeDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassComputeDef.h"
#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h"
#include "Compositor/Pass/PassMipmap/OgreCompositorPassMipmapDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/Pass/PassStencil/OgreCompositorPassStencilDef.h"
#include "Compositor/Pass/PassUav/OgreCompositorPassUavDef.h"
#include "Compositor/Pass/OgreCompositorPassProvider.h"

namespace Ogre{

    GpuProgramType translateIDToGpuProgramType(uint32 id)
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
    AbstractNodeList::const_iterator ScriptTranslator::getNodeAt(const AbstractNodeList &nodes, int index)
    {
        AbstractNodeList::const_iterator i = nodes.begin();
        int n = 0;
        while(i != nodes.end())
        {
            if(n == index)
                return i;
            ++i;
            ++n;
        }
        return nodes.end();
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
    bool ScriptTranslator::getIdString(const AbstractNodePtr &node, IdString *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        *result = IdString( atom->value );
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getReal(const Ogre::AbstractNodePtr &node, Ogre::Real *result)
    {
        if (node->type != ANT_ATOM)
            return false;

        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();

#if OGRE_DOUBLE_PRECISION == 0
        int n = sscanf(atom->value.c_str(), "%f", result);
#else
        int n = sscanf(atom->value.c_str(), "%lf", result);
#endif      
        
        if(n == 0 || n == EOF)
            return false; // Conversion failed
        
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getFloat(const Ogre::AbstractNodePtr &node, float *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        int n = sscanf(atom->value.c_str(), "%f", result);
        if(n == 0 || n == EOF)
            return false; // Conversion failed
            
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getDouble(const Ogre::AbstractNodePtr &node, double *result)
    {
        if(node->type != ANT_ATOM)
            return false;

        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        int n = sscanf(atom->value.c_str(), "%lf", result);
        if (n == 0 || n == EOF)
            return false; // Conversion failed

        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getInt(const Ogre::AbstractNodePtr &node, int *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        int n = sscanf(atom->value.c_str(), "%d", result);
        if(n == 0 || n == EOF)
            return false; // Conversion failed
            
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getUInt(const Ogre::AbstractNodePtr &node, uint32 *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
        int n = sscanf(atom->value.c_str(), "%u", result);
        if(n == 0 || n == EOF)
            return false; // Conversion failed
            
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getHex(const Ogre::AbstractNodePtr &node, uint32 *result)
    {
        if(node->type != ANT_ATOM)
            return false;
        
        AtomAbstractNode *atom = (AtomAbstractNode*)node.get();

        char *end;
        *result = static_cast<uint32>(strtoul( atom->value.c_str(), &end, 16 ));
        
        return !(*end);
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
        while(i != end && n < 16)
        {
            if(i != end)
            {
                Real r = 0;
                if(getReal(*i, &r))
                    (*m)[n/4][n%4] = r;
                else
                    return false;
            }
            else
            {
                return false;
            }
            ++i;
            ++n;
        }
        return true;
    }
    //-------------------------------------------------------------------------
    bool ScriptTranslator::getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count)
    {
        bool success = true;
        int n = 0;
        while(n < count)
        {
            if(i != end)
            {
                int v = 0;
                if(getInt(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if(n < count)
            success = false;

        return success;
    }
    //----------------------------------------------------------------------------
    bool ScriptTranslator::getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count)
    {
        bool success = true;
        int n = 0;
        while(n < count)
        {
            if(i != end)
            {
                float v = 0;
                if(getFloat(*i, &v))
                    vals[n] = v;
                else
                    break;
                ++i;
            }
            else
                vals[n] = 0;
            ++n;
        }

        if(n < count)
            success = false;

        return success;
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

        String val;
        getString(*i, &val);
        if(val.find("float") != String::npos)
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

        return true;
    }

    /**************************************************************************
     * HlmsTranslator
     *************************************************************************/
    /*HlmsTranslator::HlmsTranslator()
    {
    }*/

    void HlmsTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Must have an HLMS type
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line,
                               "HLMS materials require type declaration. i.e. 'hlms myMaterial pbs'.");
            return;
        }

        // Get the type
        String type;
        if( !getString(obj->values.front(), &type) )
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
            return;
        }

        HlmsManager *hlmsManager = Root::getSingleton().getHlmsManager();
        Hlms *hlms = 0;
        HlmsParamVec paramVec;

        paramVec.reserve( obj->children.size() );

        const IdString idType( type );
        for( size_t i=0; i<HLMS_MAX && !hlms; ++i )
        {
            hlms = hlmsManager->getHlms( static_cast<HlmsTypes>( i ) );

            if( hlms && idType != hlms->getTypeName() )
                hlms = 0; //This one isn't, keep looking
        }

        if( !hlms )
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, node->file, node->line,
                               type + " is not a valid hlms type");
            return;
        }

        HlmsMacroblock macroblock;
        HlmsBlendblock blendblock;
        //obj->values

        // Create the workspace definition*/
        /*CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mWorkspaceDef);

        if(!processed)
        {
            CompositorManager2 *compositorMgr = Root::getSingleton().getCompositorManager2();
            mWorkspaceDef = compositorMgr->addWorkspaceDefinition( obj->name );
        }

        if(mWorkspaceDef == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }*/

        obj->context = Any(hlms);

        AbstractNodeList::iterator i = obj->children.begin();
        try
        {
        for(i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_DEPTH_CHECK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_check must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &macroblock.mDepthCheck))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_check third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_DEPTH_WRITE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_write must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &macroblock.mDepthWrite))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_write third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }

                    }
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
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 0), i0 = i1++;
                        if(getFloat(*i0, &macroblock.mDepthBiasConstant))
                        {
                            if(i1 != prop->values.end())
                                getFloat(*i1, &macroblock.mDepthBiasSlopeScale);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_bias does not support \"" + (*i0)->getValue() + "\" for argument 1");
                        }
                    }
                    break;
                case ID_DEPTH_FUNC:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_func must have 1 argument");
                    }
                    else
                    {
                        if(!getCompareFunction(prop->values.front(), &macroblock.mDepthFunc))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid CompareFunction");
                        }
                    }
                    break;
                case ID_ALPHA_TO_COVERAGE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "alpha_to_coverage must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &blendblock.mAlphaToCoverageEnabled))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "alpha_to_coverage argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_POLYGON_MODE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "polygon_mode must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_SOLID:
                                macroblock.mPolygonMode = PM_SOLID;
                                break;
                            case ID_POINTS:
                                macroblock.mPolygonMode = PM_POINTS;
                                break;
                            case ID_WIREFRAME:
                                macroblock.mPolygonMode = PM_WIREFRAME;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
                        }
                    }
                    break;
                case ID_CULL_MODE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "cull_hardware must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_CLOCKWISE:
                                macroblock.mCullMode = CULL_CLOCKWISE;
                                break;
                            case ID_ANTICLOCKWISE:
                                macroblock.mCullMode = CULL_ANTICLOCKWISE;
                                break;
                            case ID_NONE:
                                macroblock.mCullMode = CULL_NONE;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid CullingMode");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid CullingMode");
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
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_ADD:
                                Pass::_getBlendFlags( SBT_ADD,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_MODULATE:
                                Pass::_getBlendFlags( SBT_MODULATE,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_COLOUR_BLEND:
                                Pass::_getBlendFlags( SBT_TRANSPARENT_COLOUR,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_ALPHA_BLEND:
                                Pass::_getBlendFlags( SBT_TRANSPARENT_ALPHA,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "scene_blend does not support \"" + prop->values.front()->getValue() + "\" for argument 1");
                            }
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
                        if( !getSceneBlendFactor(*i0, &blendblock.mSourceBlendFactor) ||
                            !getSceneBlendFactor(*i1, &blendblock.mDestBlendFactor) )
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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
                            SceneBlendType sbt0, sbt1;
                            switch(atom0->id)
                            {
                            case ID_ADD:
                                sbt0 = SBT_ADD;
                                break;
                            case ID_MODULATE:
                                sbt0 = SBT_MODULATE;
                                break;
                            case ID_COLOUR_BLEND:
                                sbt0 = SBT_TRANSPARENT_COLOUR;
                                break;
                            case ID_ALPHA_BLEND:
                                sbt0 = SBT_TRANSPARENT_ALPHA;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "separate_scene_blend does not support \"" + atom0->value + "\" as argument 1");
                                return;
                            }

                            switch(atom1->id)
                            {
                            case ID_ADD:
                                sbt1 = SBT_ADD;
                                break;
                            case ID_MODULATE:
                                sbt1 = SBT_MODULATE;
                                break;
                            case ID_COLOUR_BLEND:
                                sbt1 = SBT_TRANSPARENT_COLOUR;
                                break;
                            case ID_ALPHA_BLEND:
                                sbt1 = SBT_TRANSPARENT_ALPHA;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "separate_scene_blend does not support \"" + atom1->value + "\" as argument 2");
                                return;
                            }

                            Pass::_getBlendFlags( sbt0,
                                                  blendblock.mSourceBlendFactor,
                                                  blendblock.mDestBlendFactor );
                            Pass::_getBlendFlags( sbt1,
                                                  blendblock.mSourceBlendFactorAlpha,
                                                  blendblock.mDestBlendFactorAlpha );
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
                            if( !getSceneBlendFactor(*i0, &blendblock.mSourceBlendFactor) ||
                                !getSceneBlendFactor(*i1, &blendblock.mDestBlendFactor) ||
                                !getSceneBlendFactor(*i2, &blendblock.mSourceBlendFactorAlpha) ||
                                !getSceneBlendFactor(*i3, &blendblock.mDestBlendFactorAlpha) )
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
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scene_blend_op must have 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = reinterpret_cast<AtomAbstractNode*>(prop->values.front().get());
                            switch(atom->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperation = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperation = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperation = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperation = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperation = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom->value + ": unrecognized argument");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + ": unrecognized argument");
                        }
                    }
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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = reinterpret_cast<AtomAbstractNode*>((*i0).get()),
                                *atom1 = reinterpret_cast<AtomAbstractNode*>((*i1).get());
                            switch(atom0->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperation = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperation = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperation = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperation = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperation = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom0->value + ": unrecognized first argument");
                            }

                            switch(atom1->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperationAlpha = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperationAlpha = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperationAlpha = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperationAlpha = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperationAlpha = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom1->value + ": unrecognized second argument");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + ": unrecognized argument");
                        }
                    }
                    break;
                default:
                    {
                    String value;
                    AbstractNodeList::const_iterator itor = prop->values.begin();
                    AbstractNodeList::const_iterator end  = prop->values.end();
                    if( itor != end )
                    {
                        if( (*itor)->type == ANT_ATOM )
                            value += static_cast<AtomAbstractNode*>((*itor).get())->getValue();
                        ++itor;
                        while( itor != end )
                        {
                            if( (*itor)->type == ANT_ATOM )
                                value += " " + static_cast<AtomAbstractNode*>((*itor).get())->getValue();
                            ++itor;
                        }
                    }
                    paramVec.push_back( std::pair<IdString, String>( prop->name, value ) );
                    }
                }
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                    "token not recognized");
            }
        }

        std::sort( paramVec.begin(), paramVec.end(), OrderParamVecByKey );
        hlms->createDatablock( obj->name, obj->name, macroblock, blendblock, paramVec, true,
                               obj->file, compiler->getResourceGroup() );

        }
        catch( Exception & )
        {
            if( i != obj->children.end() )
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, (*i)->file, (*i)->line);
        }
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
        else
        {
            if(!mMaterial)
                compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
                                   "failed to find or create material \"" + obj->name + "\"");
        }

        mMaterial->removeAllTechniques();
        obj->context = Any(mMaterial);
        mMaterial->_notifyOrigin(obj->file);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_LOD_VALUES:
                    {
                        Material::LodValueArray lods;
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
                case ID_RECEIVE_SHADOWS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "receive_shadows only supports 1 argument");
                    }
                    else
                    {
                        bool val = true;
                        if(getBoolean(prop->values.front(), &val))
                            mMaterial->setReceiveShadows(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "receive_shadows argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                    }
                    break;
                case ID_TRANSPARENCY_CASTS_SHADOWS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "transparency_casts_shadows only supports 1 argument");
                    }
                    else
                    {
                        bool val = true;
                        if(getBoolean(prop->values.front(), &val))
                            mMaterial->setTransparencyCastsShadows(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "transparency_casts_shadows argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                    }
                    break;
                case ID_SET_TEXTURE_ALIAS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "set_texture_alias only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
                        String name, value;
                        if(getString(*i0, &name) && getString(*i1, &value))
                            mTextureAliases.insert(std::make_pair(name, value));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "set_texture_alias must have 2 string argument");
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
        }

        // Apply the texture aliases
        if(compiler->getListener())
        {
            PreApplyTextureAliasesScriptCompilerEvent locEvt(mMaterial, &mTextureAliases);
            compiler->_fireEvent(&locEvt, 0);
        }
        mMaterial->applyTextureAliases(mTextureAliases);
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
        obj->context = Any(mTechnique);

        // Get the name of the technique
        if(!obj->name.empty())
            mTechnique->setName(obj->name);

        // Set the properties for the material
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_SCHEME:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "scheme only supports 1 argument");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        String scheme;
                        if(getString(*i0, &scheme))
                            mTechnique->setSchemeName(scheme);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "scheme must have 1 string argument");
                    }
                    break;
                case ID_LOD_INDEX:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "lod_index only supports 1 argument");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        uint32 v = 0;
                        if(getUInt(*i0, &v))
                            mTechnique->setLodIndex(static_cast<unsigned short>(v));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "lod_index cannot accept argument \"" + (*i0)->getValue() + "\"");
                    }
                    break;
                case ID_SHADOW_CASTER_MATERIAL:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "shadow_caster_material only accepts 1 argument");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        String matName;
                        if(getString(*i0, &matName))
                        {
                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::MATERIAL, matName);
                            compiler->_fireEvent(&evt, 0);
                            mTechnique->setShadowCasterMaterial(evt.mName); // Use the processed name
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "shadow_caster_material cannot accept argument \"" + (*i0)->getValue() + "\"");
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
            else if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
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
        obj->context = Any(mPass);

        // Get the name of the technique
        if(!obj->name.empty())
            mPass->setName(obj->name);

        HlmsMacroblock macroblock;
        HlmsBlendblock blendblock;

        // Set the properties for the material
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
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
                                AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
                                    i1 = getNodeAt(prop->values, 1),
                                    i2 = getNodeAt(prop->values, 2);
                                ColourValue val(0.0f, 0.0f, 0.0f, 1.0f);
                                if(getFloat(*i0, &val.r) && getFloat(*i1, &val.g) && getFloat(*i2, &val.b))
                                {
                                    if(prop->values.size() == 4)
                                    {
                                        mPass->setSpecular(val);

                                        AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
                                        Real shininess = 0.0f;
                                        if(getReal(*i3, &shininess))
                                            mPass->setShininess(shininess);
                                        else
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "specular fourth argument must be a valid number for shininess attribute");
                                    }
                                    else
                                    {
                                        AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
                                        if(!getFloat(*i3, &val.a))
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "specular fourth argument must be a valid color component value");
                                        else
                                            mPass->setSpecular(val);

                                        AbstractNodeList::const_iterator i4 = getNodeAt(prop->values, 4);
                                        Real shininess = 0.0f;
                                        if(getReal(*i4, &shininess))
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
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_ADD:
                                Pass::_getBlendFlags( SBT_ADD,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_MODULATE:
                                Pass::_getBlendFlags( SBT_MODULATE,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_COLOUR_BLEND:
                                Pass::_getBlendFlags( SBT_TRANSPARENT_COLOUR,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            case ID_ALPHA_BLEND:
                                Pass::_getBlendFlags( SBT_TRANSPARENT_ALPHA,
                                                      blendblock.mSourceBlendFactor,
                                                      blendblock.mDestBlendFactor );
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "scene_blend does not support \"" + prop->values.front()->getValue() + "\" for argument 1");
                            }
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
                        if( !getSceneBlendFactor(*i0, &blendblock.mSourceBlendFactor) ||
                            !getSceneBlendFactor(*i1, &blendblock.mDestBlendFactor) )
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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
                            SceneBlendType sbt0, sbt1;
                            switch(atom0->id)
                            {
                            case ID_ADD:
                                sbt0 = SBT_ADD;
                                break;
                            case ID_MODULATE:
                                sbt0 = SBT_MODULATE;
                                break;
                            case ID_COLOUR_BLEND:
                                sbt0 = SBT_TRANSPARENT_COLOUR;
                                break;
                            case ID_ALPHA_BLEND:
                                sbt0 = SBT_TRANSPARENT_ALPHA;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "separate_scene_blend does not support \"" + atom0->value + "\" as argument 1");
                                return;
                            }

                            switch(atom1->id)
                            {
                            case ID_ADD:
                                sbt1 = SBT_ADD;
                                break;
                            case ID_MODULATE:
                                sbt1 = SBT_MODULATE;
                                break;
                            case ID_COLOUR_BLEND:
                                sbt1 = SBT_TRANSPARENT_COLOUR;
                                break;
                            case ID_ALPHA_BLEND:
                                sbt1 = SBT_TRANSPARENT_ALPHA;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   "separate_scene_blend does not support \"" + atom1->value + "\" as argument 2");
                                return;
                            }

                            Pass::_getBlendFlags( sbt0,
                                                  blendblock.mSourceBlendFactor,
                                                  blendblock.mDestBlendFactor );
                            Pass::_getBlendFlags( sbt1,
                                                  blendblock.mSourceBlendFactorAlpha,
                                                  blendblock.mDestBlendFactorAlpha );
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
                            if( !getSceneBlendFactor(*i0, &blendblock.mSourceBlendFactor) ||
                                !getSceneBlendFactor(*i1, &blendblock.mDestBlendFactor) ||
                                !getSceneBlendFactor(*i2, &blendblock.mSourceBlendFactorAlpha) ||
                                !getSceneBlendFactor(*i3, &blendblock.mDestBlendFactorAlpha) )
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
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "scene_blend_op must have 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = static_cast<AtomAbstractNode*>(prop->values.front().get());
                            switch(atom->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperation = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperation = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperation = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperation = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperation = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom->value + ": unrecognized argument");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + ": unrecognized argument");
                        }
                    }
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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = static_cast<AtomAbstractNode*>((*i0).get()),
                                             *atom1 = static_cast<AtomAbstractNode*>((*i1).get());
                            switch(atom0->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperation = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperation = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperation = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperation = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperation = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom0->value + ": unrecognized first argument");
                            }

                            switch(atom1->id)
                            {
                            case ID_ADD:
                                blendblock.mBlendOperationAlpha = SBO_ADD;
                                break;
                            case ID_SUBTRACT:
                                blendblock.mBlendOperationAlpha = SBO_SUBTRACT;
                                break;
                            case ID_REVERSE_SUBTRACT:
                                blendblock.mBlendOperationAlpha = SBO_REVERSE_SUBTRACT;
                                break;
                            case ID_MIN:
                                blendblock.mBlendOperationAlpha = SBO_MIN;
                                break;
                            case ID_MAX:
                                blendblock.mBlendOperationAlpha = SBO_MAX;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   atom1->value + ": unrecognized second argument");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + ": unrecognized argument");
                        }
                    }
                    break;
                case ID_DEPTH_CHECK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_check must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &macroblock.mDepthCheck))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_check third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_DEPTH_WRITE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_write must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &macroblock.mDepthWrite))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_write third argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }

                    }
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
                        AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 0), i0 = i1++;
                        if(getFloat(*i0, &macroblock.mDepthBiasConstant))
                        {
                            if(i1 != prop->values.end())
                                getFloat(*i1, &macroblock.mDepthBiasSlopeScale);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "depth_bias does not support \"" + (*i0)->getValue() + "\" for argument 1");
                        }
                    }
                    break;
                case ID_DEPTH_FUNC:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "depth_func must have 1 argument");
                    }
                    else
                    {
                        if(!getCompareFunction(prop->values.front(), &macroblock.mDepthFunc))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid CompareFunction");
                        }
                    }
                    break;
                case ID_ALPHA_TO_COVERAGE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "alpha_to_coverage must have 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &blendblock.mAlphaToCoverageEnabled))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "alpha_to_coverage argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_POLYGON_MODE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "polygon_mode must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_SOLID:
                                macroblock.mPolygonMode = PM_SOLID;
                                break;
                            case ID_POINTS:
                                macroblock.mPolygonMode = PM_POINTS;
                                break;
                            case ID_WIREFRAME:
                                macroblock.mPolygonMode = PM_WIREFRAME;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid polygon mode (solid, points, or wireframe)");
                        }
                    }
                    break;
                case ID_CULL_MODE:
                case ID_CULL_HARDWARE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "cull_hardware must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_CLOCKWISE:
                                macroblock.mCullMode = CULL_CLOCKWISE;
                                break;
                            case ID_ANTICLOCKWISE:
                                macroblock.mCullMode = CULL_ANTICLOCKWISE;
                                break;
                            case ID_NONE:
                                macroblock.mCullMode = CULL_NONE;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid CullingMode");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid CullingMode");
                        }
                    }
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
                                {
                                    mPass->setAlphaRejectFunction(func);
                                    mPass->setAlphaRejectValue(static_cast<unsigned char>(val));
                                }
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
                case ID_LIGHT_SCISSOR:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "light_scissor must have only 1 argument");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                            mPass->setLightScissoringEnabled(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
                    break;
                case ID_LIGHT_CLIP_PLANES:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "light_clip_planes must have at most 1 argument");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                            mPass->setLightClipPlanesEnabled(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
                    break;
                case ID_SHADING:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "shading must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_FLAT:
                                mPass->setShadingMode(SO_FLAT);
                                break;
                            case ID_GOURAUD:
                                mPass->setShadingMode(SO_GOURAUD);
                                break;
                            case ID_PHONG:
                                mPass->setShadingMode(SO_PHONG);
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid shading mode (flat, gouraud, or phong)");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid shading mode (flat, gouraud, or phong)");
                        }
                    }
                    break;
                case ID_POLYGON_MODE_OVERRIDEABLE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "polygon_mode_overrideable must have at most 1 argument");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                            mPass->setPolygonModeOverrideable(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
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
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "colour_write must have at most 1 argument");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                        {
                            blendblock.mBlendChannelMask = val ? HlmsBlendblock::BlendChannelAll : 0;
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
                    break;
                case ID_MAX_LIGHTS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "max_lights must have at most 1 argument");
                    }
                    else
                    {
                        uint32 val = 0;
                        if(getUInt(prop->values.front(), &val))
                            mPass->setMaxSimultaneousLights(static_cast<unsigned short>(val));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer");
                    }
                    break;
                case ID_START_LIGHT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "start_light must have at most 1 argument");
                    }
                    else
                    {
                        uint32 val = 0;
                        if(getUInt(prop->values.front(), &val))
                            mPass->setStartLight(static_cast<unsigned short>(val));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer");
                    }
                    break;
                case ID_LIGHT_MASK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else
                    {
                        uint32 val = 0;
                        if(getUInt(prop->values.front(), &val))
                            mPass->setLightMask(static_cast<unsigned short>(val));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer");
                    }
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
                                    else if(ID_PER_N_LIGHTS)
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
                case ID_POINT_SIZE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "point_size must have at most 1 argument");
                    }
                    else
                    {
                        Real val = 0.0f;
                        if(getReal(prop->values.front(), &val))
                            mPass->setPointSize(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number");
                    }
                    break;
                case ID_POINT_SPRITES:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "point_sprites must have at most 1 argument");
                    }
                    else
                    {
                        bool val = false;
                        if(getBoolean(prop->values.front(), &val))
                            mPass->setPointSpritesEnabled(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid boolean");
                    }
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
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "point_size_min must have at most 1 argument");
                    }
                    else
                    {
                        Real val = 0.0f;
                        if(getReal(prop->values.front(), &val))
                            mPass->setPointMinSize(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number");
                    }
                    break;
                case ID_POINT_SIZE_MAX:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "point_size_max must have at most 1 argument");
                    }
                    else
                    {
                        Real val = 0.0f;
                        if(getReal(prop->values.front(), &val))
                            mPass->setPointMaxSize(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number");
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                       "token \"" + prop->name + "\" is not recognized");
                }
            }
            else if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *child = static_cast<ObjectAbstractNode*>((*i).get());
                switch(child->id)
                {
                case ID_FRAGMENT_PROGRAM_REF:
                    translateFragmentProgramRef(compiler, child);
                    break;
                case ID_VERTEX_PROGRAM_REF:
                    translateVertexProgramRef(compiler, child);
                    break;
                case ID_GEOMETRY_PROGRAM_REF:
                    translateGeometryProgramRef(compiler, child);
                    break;
                case ID_TESSELLATION_HULL_PROGRAM_REF:
                    translateTessellationHullProgramRef(compiler, child);
                    break;
                case ID_TESSELLATION_DOMAIN_PROGRAM_REF:
                    translateTessellationDomainProgramRef(compiler, child);
                    break;
                case ID_COMPUTE_PROGRAM_REF:
                    translateComputeProgramRef(compiler, child);
                    break;
                case ID_SHADOW_CASTER_VERTEX_PROGRAM_REF:
                    translateShadowCasterVertexProgramRef(compiler, child);
                    break;
                case ID_SHADOW_CASTER_FRAGMENT_PROGRAM_REF:
                    translateShadowCasterFragmentProgramRef(compiler, child);
                    break;
                default:
                    processNode(compiler, *i);
                }
            }
        }

        mPass->setMacroblock( macroblock );
        mPass->setBlendblock( blendblock );
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateFragmentProgramRef(Ogre::ScriptCompiler *compiler, Ogre::ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setFragmentProgram(evt.mName);
        if(pass->getFragmentProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setVertexProgram(evt.mName);
        if(pass->getVertexProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateGeometryProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setGeometryProgram(evt.mName);
        if(pass->getGeometryProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getGeometryProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateTessellationHullProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setTessellationHullProgram(evt.mName);
        if(pass->getTessellationHullProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getTessellationHullProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateTessellationDomainProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setTessellationDomainProgram(evt.mName);
        if(pass->getTessellationDomainProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getTessellationDomainProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateComputeProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setComputeProgram(evt.mName);
        if(pass->getComputeProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getComputeProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateShadowCasterVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setShadowCasterVertexProgram(evt.mName);
        if(pass->getShadowCasterVertexProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getShadowCasterVertexProgramParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, params, node);
        }
    }
    //-------------------------------------------------------------------------
    void PassTranslator::translateShadowCasterFragmentProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node)
    {
        if(node->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, node->file, node->line);
            return;
        }

        ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, node->name);
        compiler->_fireEvent(&evt, 0);

        if (GpuProgramManager::getSingleton().getByName(evt.mName).isNull())
        {
            compiler->addError(ScriptCompiler::CE_REFERENCETOANONEXISTINGOBJECT, node->file, node->line);
            return;
        }

        Pass *pass = any_cast<Pass*>(node->parent->context);
        pass->setShadowCasterFragmentProgram(evt.mName);
        if(pass->getShadowCasterFragmentProgram()->isSupported())
        {
            GpuProgramParametersSharedPtr params = pass->getShadowCasterFragmentProgramParameters();
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
    void TextureUnitTranslator::translate(ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());

        Pass *pass = any_cast<Pass*>(obj->parent->context);
        mUnit = pass->createTextureUnitState();
        obj->context = Any(mUnit);

        // Get the name of the technique
        if(!obj->name.empty())
            mUnit->setName(obj->name);

        HlmsSamplerblock samplerblock;
        samplerblock.setAddressingMode( TAM_WRAP );

        // Set the properties for the material
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_TEXTURE_ALIAS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "texture_alias must have at most 1 argument");
                    }
                    else
                    {
                        String val;
                        if(getString(prop->values.front(), &val))
                            mUnit->setTextureNameAlias(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid texture alias");
                    }
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
                                        // fallback to 2d texture if 1d is not supported
                                        {
                                            // Use the current render system
                                            RenderSystem* rs = Root::getSingleton().getRenderSystem();

                                            if (rs->getCapabilities()->hasCapability(RSC_TEXTURE_1D))
                                            {
                                                texType = TEX_TYPE_1D;
                                                break;
                                            }
                                        }                                                                       case ID_2D:
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
                                        if(StringConverter::isNumber(atom->value))
                                            mipmaps = StringConverter::parseInt(atom->value);
                                        else
                                            format = PixelUtil::getFormatFromName(atom->value, true);
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

                            mUnit->setTextureName(evt.mName, texType);
                            mUnit->setDesiredFormat(format);
                            mUnit->setIsAlpha(isAlpha);
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
                                String *names = OGRE_NEW_ARRAY_T(String, prop->values.size() - 1, MEMCATEGORY_SCRIPTING);
                                int n = 0;

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
                                            names[n++] = evt.mName;
                                        }
                                        else
                                        {
                                            names[n++] = name;
                                        }
                                    }
                                    else
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           (*j)->getValue() + " is not supported as a texture name");
                                    ++j;
                                }

                                mUnit->setAnimatedTextureName(names, n, duration);

                                OGRE_DELETE_ARRAY_T(names, String, prop->values.size() - 1, MEMCATEGORY_SCRIPTING);
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
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::TEXTURE, atom0->value);
                            compiler->_fireEvent(&evt, 0);

                            mUnit->setCubicTextureName(evt.mName, atom1->id == ID_COMBINED_UVW);
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
                            String names[6];
                            names[0] = atom0->value;
                            names[1] = atom1->value;
                            names[2] = atom2->value;
                            names[3] = atom3->value;
                            names[4] = atom4->value;
                            names[5] = atom5->value;

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

                            mUnit->setCubicTextureName(names, atom6->id == ID_COMBINED_UVW);
                        }

                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "cubic_texture must have at most 7 arguments");
                    }
                    break;
                case ID_TEX_COORD_SET:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "tex_coord_set must have at most 1 argument");
                    }
                    else
                    {
                        uint32 val = 0;
                        if(getUInt(prop->values.front(), &val))
                            mUnit->setTextureCoordSet(val);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not supported as an integer argument");
                    }
                    break;
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

                            if(i0 != prop->values.end() && (*i0)->type == ANT_ATOM)
                            {
                                AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
                                switch(atom->id)
                                {
                                case ID_WRAP:
                                    samplerblock.mU = TAM_WRAP;
                                    break;
                                case ID_CLAMP:
                                    samplerblock.mU = TAM_CLAMP;
                                    break;
                                case ID_MIRROR:
                                    samplerblock.mU = TAM_MIRROR;
                                    break;
                                case ID_BORDER:
                                    samplerblock.mU = TAM_BORDER;
                                    break;
                                default:
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i0)->getValue() + " not supported as first argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                                }
                            }
                            samplerblock.mV = samplerblock.mU;
                            samplerblock.mW = samplerblock.mU;

                            if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
                            {
                                AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
                                switch(atom->id)
                                {
                                case ID_WRAP:
                                    samplerblock.mV = TAM_WRAP;
                                    break;
                                case ID_CLAMP:
                                    samplerblock.mV = TAM_CLAMP;
                                    break;
                                case ID_MIRROR:
                                    samplerblock.mV = TAM_MIRROR;
                                    break;
                                case ID_BORDER:
                                    samplerblock.mV = TAM_BORDER;
                                    break;
                                default:
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i0)->getValue() + " not supported as second argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                                }
                            }

                            if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
                            {
                                AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
                                switch(atom->id)
                                {
                                case ID_WRAP:
                                    samplerblock.mW = TAM_WRAP;
                                    break;
                                case ID_CLAMP:
                                    samplerblock.mW = TAM_CLAMP;
                                    break;
                                case ID_MIRROR:
                                    samplerblock.mW = TAM_MIRROR;
                                    break;
                                case ID_BORDER:
                                    samplerblock.mW = TAM_BORDER;
                                    break;
                                default:
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*i0)->getValue() + " not supported as third argument (must be \"wrap\", \"clamp\", \"mirror\", or \"border\")");
                                }
                            }
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
                        if(!getColour(prop->values.begin(), prop->values.end(), &samplerblock.mBorderColour))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "tex_border_colour only accepts a colour argument");
                        }
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
                                samplerblock.setFiltering(TFO_NONE);
                                break;
                            case ID_BILINEAR:
                                samplerblock.setFiltering(TFO_BILINEAR);
                                break;
                            case ID_TRILINEAR:
                                samplerblock.setFiltering(TFO_TRILINEAR);
                                break;
                            case ID_ANISOTROPIC:
                                samplerblock.setFiltering(TFO_ANISOTROPIC);
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
                        if((*i0)->type == ANT_ATOM &&
                           (*i1)->type == ANT_ATOM &&
                           (*i2)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
                                *atom1 = (AtomAbstractNode*)(*i1).get(),
                                *atom2 = (AtomAbstractNode*)(*i2).get();
                            switch(atom0->id)
                            {
                            case ID_NONE:
                                samplerblock.mMinFilter = FO_NONE;
                                break;
                            case ID_POINT:
                                samplerblock.mMinFilter = FO_POINT;
                                break;
                            case ID_LINEAR:
                                samplerblock.mMinFilter = FO_LINEAR;
                                break;
                            case ID_ANISOTROPIC:
                                samplerblock.mMinFilter = FO_ANISOTROPIC;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i0)->getValue() + " not supported as first argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
                            }

                            switch(atom1->id)
                            {
                            case ID_NONE:
                                samplerblock.mMagFilter = FO_NONE;
                                break;
                            case ID_POINT:
                                samplerblock.mMagFilter = FO_POINT;
                                break;
                            case ID_LINEAR:
                                samplerblock.mMagFilter = FO_LINEAR;
                                break;
                            case ID_ANISOTROPIC:
                                samplerblock.mMagFilter = FO_ANISOTROPIC;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i0)->getValue() + " not supported as second argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
                            }

                            switch(atom2->id)
                            {
                            case ID_NONE:
                                samplerblock.mMipFilter = FO_NONE;
                                break;
                            case ID_POINT:
                                samplerblock.mMipFilter = FO_POINT;
                                break;
                            case ID_LINEAR:
                                samplerblock.mMipFilter = FO_LINEAR;
                                break;
                            case ID_ANISOTROPIC:
                                samplerblock.mMipFilter = FO_ANISOTROPIC;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i0)->getValue() + " not supported as third argument (must be \"none\", \"point\", \"linear\", or \"anisotropic\")");
                            }
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
                case ID_CMPFUNC:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "compare_func must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_ALWAYS_FAIL:
                                samplerblock.mCompareFunction = CMPF_ALWAYS_FAIL;
                                break;
                            case ID_ALWAYS_PASS:
                                samplerblock.mCompareFunction = CMPF_ALWAYS_PASS;
                                break;
                            case ID_LESS:
                                samplerblock.mCompareFunction = CMPF_LESS;
                                break;
                            case ID_LESS_EQUAL:
                                samplerblock.mCompareFunction = CMPF_LESS_EQUAL;
                                break;
                            case ID_EQUAL:
                                samplerblock.mCompareFunction = CMPF_EQUAL;
                                break;
                            case ID_NOT_EQUAL:
                                samplerblock.mCompareFunction = CMPF_NOT_EQUAL;
                                break;
                            case ID_GREATER_EQUAL:
                                samplerblock.mCompareFunction = CMPF_GREATER_EQUAL;
                                break;
                            case ID_GREATER:
                                samplerblock.mCompareFunction = CMPF_GREATER;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + "is not a valid parameter");
                            }
                        }
                    }
                    break;
                case ID_MAX_ANISOTROPY:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "max_anisotropy must have at most 1 argument");
                    }
                    else
                    {
                        if(!getFloat(prop->values.front(), &samplerblock.mMaxAnisotropy))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer argument");
                        }
                    }
                    break;
                case ID_MIPMAP_BIAS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "mipmap_bias must have at most 1 argument");
                    }
                    else
                    {
                        if(!getReal(prop->values.front(), &samplerblock.mMipLodBias))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number argument");
                        }
                    }
                    break;
                case ID_COLOUR_OP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "colour_op must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_REPLACE:
                                mUnit->setColourOperation(LBO_REPLACE);
                                break;
                            case ID_ADD:
                                mUnit->setColourOperation(LBO_ADD);
                                break;
                            case ID_MODULATE:
                                mUnit->setColourOperation(LBO_MODULATE);
                                break;
                            case ID_ALPHA_BLEND:
                                mUnit->setColourOperation(LBO_ALPHA_BLEND);
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() + " is not a valid argument (must be \"replace\", \"add\", \"modulate\", or \"alpha_blend\")");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid argument (must be \"replace\", \"add\", \"modulate\", or \"alpha_blend\")");
                        }
                    }
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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
                                *atom1 = (AtomAbstractNode*)(*i1).get(),
                                *atom2 = (AtomAbstractNode*)(*i2).get();
                            LayerBlendOperationEx op = LBX_ADD;
                            LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
                            ColourValue arg1 = ColourValue::White, arg2 = ColourValue::White;
                            Real manualBlend = 0.0f;

                            switch(atom0->id)
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
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i0)->getValue() + " is not a valid first argument (must be \"source1\", \"source2\", \"modulate\", \"modulate_x2\", \"modulate_x4\", \"add\", \"add_signed\", \"add_smooth\", \"subtract\", \"blend_diffuse_alpha\", \"blend_texture_alpha\", \"blend_current_alpha\", \"blend_manual\", \"dot_product\", or \"blend_diffuse_colour\")");
                            }

                            switch(atom1->id)
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
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i1)->getValue() + " is not a valid second argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
                            }

                            switch(atom2->id)
                            {
                            case ID_SRC_CURRENT:
                                source2 = LBS_CURRENT;
                                break;
                            case ID_SRC_TEXTURE:
                                source2 = LBS_TEXTURE;
                                break;
                            case ID_SRC_DIFFUSE:
                                source2 = LBS_DIFFUSE;
                                break;
                            case ID_SRC_SPECULAR:
                                source2 = LBS_SPECULAR;
                                break;
                            case ID_SRC_MANUAL:
                                source2 = LBS_MANUAL;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i2)->getValue() + " is not a valid third argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
                            }

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
                        if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
                                *atom1 = (AtomAbstractNode*)(*i1).get(),
                                *atom2 = (AtomAbstractNode*)(*i2).get();
                            LayerBlendOperationEx op = LBX_ADD;
                            LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
                            Real arg1 = 0.0f, arg2 = 0.0f;
                            Real manualBlend = 0.0f;

                            switch(atom0->id)
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
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i0)->getValue() + " is not a valid first argument (must be \"source1\", \"source2\", \"modulate\", \"modulate_x2\", \"modulate_x4\", \"add\", \"add_signed\", \"add_smooth\", \"subtract\", \"blend_diffuse_alpha\", \"blend_texture_alpha\", \"blend_current_alpha\", \"blend_manual\", \"dot_product\", or \"blend_diffuse_colour\")");
                            }

                            switch(atom1->id)
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
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i1)->getValue() + " is not a valid second argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
                            }

                            switch(atom2->id)
                            {
                            case ID_SRC_CURRENT:
                                source2 = LBS_CURRENT;
                                break;
                            case ID_SRC_TEXTURE:
                                source2 = LBS_TEXTURE;
                                break;
                            case ID_SRC_DIFFUSE:
                                source2 = LBS_DIFFUSE;
                                break;
                            case ID_SRC_SPECULAR:
                                source2 = LBS_SPECULAR;
                                break;
                            case ID_SRC_MANUAL:
                                source2 = LBS_MANUAL;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   (*i2)->getValue() + " is not a valid third argument (must be \"src_current\", \"src_texture\", \"src_diffuse\", \"src_specular\", or \"src_manual\")");
                            }

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
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "rotate must have at most 1 argument");
                    }
                    else
                    {
                        Real angle;
                        if(getReal(prop->values.front(), &angle))
                            mUnit->setTextureRotate(Degree(angle));
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number value");
                    }
                    break;
                case ID_ROTATE_ANIM:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "rotate_anim must have at most 1 argument");
                    }
                    else
                    {
                        Real angle;
                        if(getReal(prop->values.front(), &angle))
                            mUnit->setRotateAnimation(angle);
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid number value");
                    }
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
                case ID_BINDING_TYPE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "binding_type must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_VERTEX:
                                mUnit->setBindingType(TextureUnitState::BT_VERTEX);
                                break;
                            case ID_FRAGMENT:
                                mUnit->setBindingType(TextureUnitState::BT_FRAGMENT);
                                break;
                            case ID_GEOMETRY:
                                mUnit->setBindingType(TextureUnitState::BT_GEOMETRY);
                                break;
                            case ID_TESSELLATION_HULL:
                                mUnit->setBindingType(TextureUnitState::BT_TESSELLATION_HULL);
                                break;
                            case ID_TESSELLATION_DOMAIN:
                                mUnit->setBindingType(TextureUnitState::BT_TESSELLATION_DOMAIN);
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                    atom->value + " is not a valid binding type (must be \"vertex\" or \"fragment\")");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                prop->values.front()->getValue() + " is not a valid binding type");
                        }
                    }
                    break;
                case ID_CONTENT_TYPE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "content_type must have at most 3 arguments");
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
                                if (prop->values.size() >= 2)
                                {
                                    String textureName;
                                    getString(*getNodeAt(prop->values, 1), &textureName);
                                    
                                    if (prop->values.size() == 3)
                                    {
                                        uint32 mrtIndex;
                                        getUInt(*getNodeAt(prop->values, 2), (uint32*)&mrtIndex);
                                        mUnit->setCompositorReference(textureName, mrtIndex);
                                    }
                                    else
                                    {
                                        mUnit->setCompositorReference(textureName);
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
            else if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
        }

        mUnit->setSamplerblock( samplerblock );
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

        // Set up the technique, pass, and texunit levels
        if(ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0)
        {
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

            for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
            {
                if((*i)->type == ANT_PROPERTY)
                {
                    PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
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
                else if((*i)->type == ANT_OBJECT)
                {
                    processNode(compiler, *i);
                }
            }

            ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->createDefinedTexture(material->getName(), material->getGroup());
        }
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
        if(!getString(obj->values.front(), &language))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
            return;
        }

        if(language == "asm")
            translateGpuProgram(compiler, obj);
        else if(language == "unified")
            translateUnifiedGpuProgram(compiler, obj);
        else
            translateHighLevelGpuProgram(compiler, obj);
    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translateGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
    {
        list<std::pair<String,String> >::type customParameters;
        String syntax, source;
        AbstractNodePtr params;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
                if(prop->id == ID_SOURCE)
                {
                    if(!prop->values.empty())
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                            source = ((AtomAbstractNode*)prop->values.front().get())->value;
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "source file expected");
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "source file expected");
                    }
                }
                else if(prop->id == ID_SYNTAX)
                {
                    if(!prop->values.empty())
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                            syntax = ((AtomAbstractNode*)prop->values.front().get())->value;
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "syntax string expected");
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "syntax string expected");
                    }
                }
                else
                {
                    String name = prop->name, value;
                    bool first = true;
                    for(AbstractNodeList::iterator it = prop->values.begin(); it != prop->values.end(); ++it)
                    {
                        if((*it)->type == ANT_ATOM)
                        {
                            if(!first)
                                value += " ";
                            else
                                first = false;
                            value += ((AtomAbstractNode*)(*it).get())->value;
                        }
                    }
                    customParameters.push_back(std::make_pair(name, value));
                }
            }
            else if((*i)->type == ANT_OBJECT)
            {
                if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
                    params = *i;
                else
                    processNode(compiler, *i);
            }
        }

        if (!GpuProgramManager::getSingleton().isSyntaxSupported(syntax))
        {
            compiler->addError(ScriptCompiler::CE_UNSUPPORTEDBYRENDERSYSTEM, obj->file, obj->line, ", Shader name: " + obj->name);
            // Register the unsupported program so that materials that use it know that
            // it exists but is unsupported.
            GpuProgramPtr unsupportedProg = GpuProgramManager::getSingleton().create(obj->name,
                                                                                     compiler->getResourceGroup(), translateIDToGpuProgramType(obj->id), syntax).staticCast<GpuProgram>();
            return;
        }

        // Allocate the program
        GpuProgram *prog = 0;
        CreateGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), source, syntax, translateIDToGpuProgramType(obj->id));
        bool processed = compiler->_fireEvent(&evt, (void*)&prog);
        if(!processed)
        {
            prog = GpuProgramManager::getSingleton().createProgram(obj->name, compiler->getResourceGroup(), source, translateIDToGpuProgramType(obj->id), syntax).get();
        }

        // Check that allocation worked
        if(prog == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
                               "gpu program \"" + obj->name + "\" could not be created");
            return;
        }

        obj->context = Any(prog);

        prog->setMorphAnimationIncluded(false);
        prog->setPoseAnimationIncluded(0);
        prog->setSkeletalAnimationIncluded(false);
        prog->setVertexTextureFetchRequired(false);
        prog->_notifyOrigin(obj->file);

        // Set the custom parameters
        for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
            prog->setParameter(i->first, i->second);

        // Set up default parameters
        if(prog->isSupported() && !params.isNull())
        {
            GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, ptr, static_cast<ObjectAbstractNode*>(params.get()));
        }
    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translateUnifiedGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
    {
        list<std::pair<String,String> >::type customParameters;
        AbstractNodePtr params;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
                if(prop->name == "delegate")
                {
                    String value;
                    if(!prop->values.empty() && prop->values.front()->type == ANT_ATOM)
                        value = ((AtomAbstractNode*)prop->values.front().get())->value;

                    ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::GPU_PROGRAM, value);
                    compiler->_fireEvent(&evt, 0);
                    customParameters.push_back(std::make_pair("delegate", evt.mName));
                }
                else
                {
                    String name = prop->name, value;
                    bool first = true;
                    for(AbstractNodeList::iterator it = prop->values.begin(); it != prop->values.end(); ++it)
                    {
                        if((*it)->type == ANT_ATOM)
                        {
                            if(!first)
                                value += " ";
                            else
                                first = false;
                            value += ((AtomAbstractNode*)(*it).get())->value;
                        }
                    }
                    customParameters.push_back(std::make_pair(name, value));
                }
            }
            else if((*i)->type == ANT_OBJECT)
            {
                if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
                    params = *i;
                else
                    processNode(compiler, *i);
            }
        }

        // Allocate the program
        HighLevelGpuProgram *prog = 0;
        CreateHighLevelGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), "", "unified", translateIDToGpuProgramType(obj->id));
        bool processed = compiler->_fireEvent(&evt, (void*)&prog);

        if(!processed)
        {
            prog = HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, compiler->getResourceGroup(), "unified", translateIDToGpuProgramType(obj->id)).get();
        }

        // Check that allocation worked
        if(prog == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
                               "gpu program \"" + obj->name + "\" could not be created");
            return;
        }

        obj->context = Any(prog);

        prog->setMorphAnimationIncluded(false);
        prog->setPoseAnimationIncluded(0);
        prog->setSkeletalAnimationIncluded(false);
        prog->setVertexTextureFetchRequired(false);
        prog->_notifyOrigin(obj->file);

        // Set the custom parameters
        for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
            prog->setParameter(i->first, i->second);

        // Set up default parameters
        if(prog->isSupported() && !params.isNull())
        {
            GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, ptr, static_cast<ObjectAbstractNode*>(params.get()));
        }

    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translateHighLevelGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj)
    {
        if(obj->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
            return;
        }
        String language;
        if(!getString(obj->values.front(), &language))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
            return;
        }

        list<std::pair<String,String> >::type customParameters;
        String source;
        AbstractNodePtr params;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
                if(prop->id == ID_SOURCE)
                {
                    if(!prop->values.empty())
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                            source = ((AtomAbstractNode*)prop->values.front().get())->value;
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "source file expected");
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                           "source file expected");
                    }
                }
                else
                {
                    String name = prop->name, value;
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
                            }
                            else
                            {
                                value += ((AtomAbstractNode*)(*it).get())->value;
                            }
                        }
                    }
                    customParameters.push_back(std::make_pair(name, value));
                }
            }
            else if((*i)->type == ANT_OBJECT)
            {
                if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
                    params = *i;
                else
                    processNode(compiler, *i);
            }
        }

        // Allocate the program
        HighLevelGpuProgram *prog = 0;
        CreateHighLevelGpuProgramScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup(), source, language,
                                                         translateIDToGpuProgramType(obj->id));
        bool processed = compiler->_fireEvent(&evt, (void*)&prog);
        if(!processed)
        {
            prog = HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, compiler->getResourceGroup(), language, translateIDToGpuProgramType(obj->id)).get();
            prog->setSourceFile(source);
        }

        // Check that allocation worked
        if(prog == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
                               "GPU program \"" + obj->name + "\" could not be created");
            return;
        }

        obj->context = Any(prog);

        prog->setMorphAnimationIncluded(false);
        prog->setPoseAnimationIncluded(0);
        prog->setSkeletalAnimationIncluded(false);
        prog->setVertexTextureFetchRequired(false);
        prog->_notifyOrigin(obj->file);

        // Set the custom parameters
        for(list<std::pair<String,String> >::type::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
            prog->setParameter(i->first, i->second);

        // Set up default parameters
        if(prog->isSupported() && !params.isNull())
        {
            GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
            GpuProgramTranslator::translateProgramParameters(compiler, ptr, static_cast<ObjectAbstractNode*>(params.get()));
        }

    }
    //-------------------------------------------------------------------------
    int parseProgramParameterDimensions(String& declarator, String type)
    {
        // Assume 1 unless otherwise specified
        int dimensions = 1;

        size_t start = declarator.find_first_not_of(type);

        if (start != String::npos)
        {
            size_t end = declarator.find_first_of("[", start);

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
                end = declarator.find_first_of("]", start);
                dimensions *= StringConverter::parseInt(
                    declarator.substr(start + 1, end - start - 1));
                start = declarator.find_first_of("[", end);
            }
        }
        
        return dimensions; 
    }
    //-------------------------------------------------------------------------
    void GpuProgramTranslator::translateProgramParameters(ScriptCompiler *compiler, GpuProgramParametersSharedPtr params, ObjectAbstractNode *obj)
    {
        size_t animParametricsCount = 0;

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_SHARED_PARAMS_REF:
                    {
                        if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "shared_params_ref requires a single parameter");
                            continue;
                        }

                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
                        if((*i0)->type != ANT_ATOM)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "shared parameter set name expected");
                            continue;
                        }
                        AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();

                        try
                        {
                            params->addSharedParameters(atom0->value);
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
                                    catch(...)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "setting matrix4x4 parameter failed");
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                       "incorrect matrix4x4 declaration");
                                }
                            }
                            else if (atom1->value == "subroutine")
                            {
                                String s;
                                if (getString(*k, &s))
                                {
                                    try
                                    {
                                        if (named)
                                            params->setNamedSubroutine(name, s);
                                        else
                                            params->setSubroutine(index, s);
                                    }
                                    catch(...)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "setting subroutine parameter failed");
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                                       "incorrect subroutine declaration");
                                }
                            }
                            // else if (atom1->value == "atomic_counter")
                            // {
                            //     //GpuProgramParameters::ElementType type = GpuProgramParameters::ET_INT; //FIXME
                            //     int count = 0;
                            //     if(atom1->value.size() > 14) {
                            //         count = StringConverter::parseInt(atom1->value.substr(3));
                            //         compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                            //                            "currently only a single atomic counter can be set at a time");
                            //         break;
                            //     }
                            //     else
                            //     {
                            //         count = 1;
                            //     }

                            //     // First, clear out any offending auto constants
                            //     if(named)
                            //         params->clearNamedAutoConstant(name);
                            //     else
                            //         params->clearAutoConstant(index);

                            //     // uint8 roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;
                            //     int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                            //     // uint32 *vals = OGRE_ALLOC_T(uint32, roundedCount, MEMCATEGORY_SCRIPTING);
                            //     int *vals = OGRE_ALLOC_T(int, roundedCount, MEMCATEGORY_SCRIPTING);
                            //     if(getInts(k, prop->values.end(), vals, roundedCount))
                            //     {
                            //         try
                            //         {
                            //             if(named)
                            //                 params->setNamedConstant(name, vals, count, 1);
                            //             else
                            //                 params->setConstant(index, vals, roundedCount/4);
                            //         }
                            //         catch(...)
                            //         {
                            //             compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                            //                                "setting of constant failed");
                            //         }
                            //     }
                            //     else
                            //     {
                            //         compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                            //                            "incorrect integer constant declaration");
                            //     }
                            //     OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                            // }
                            //TODO This should probably converted into a function of type.
                            else
                            {
                                // Find the type and number of parameters
                                // bool isValid = true;
                                // GpuProgramParameters::ElementType type = GpuProgramParameters::ET_FLOAT;
                                int count = 1;
                                if (atom1->value.find("float") != String::npos)
                                {
                                    // type = GpuProgramParameters::ET_FLOAT;
                                    if (atom1->value.size() >= 6)
                                        count = parseProgramParameterDimensions(atom1->value, String("float"));

                                    // First, clear out any offending auto constants
                                    if (named)
                                        params->clearNamedAutoConstant(name);
                                    else
                                        params->clearAutoConstant(index);

                                    int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                    float *vals = OGRE_ALLOC_T(float, roundedCount, MEMCATEGORY_SCRIPTING);
                                    if (getFloats(k, prop->values.end(), vals, roundedCount))
                                    {
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals, count, 1);
                                            else
                                                params->setConstant(index, vals, roundedCount/4);
                                        }
                                        catch (...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                           "incorrect float constant declaration");
                                    }
                                    OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                }
                                else if (atom1->value.find("uint") != String::npos)
                                {
                                    // type = GpuProgramParameters::ET_UINT;
                                    if (atom1->value.size() >= 5)
                                        count = parseProgramParameterDimensions(atom1->value, String("uint"));

                                    // First, clear out any offending auto constants
                                    if (named)
                                        params->clearNamedAutoConstant(name);
                                    else
                                        params->clearAutoConstant(index);

                                    int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                    uint *vals = OGRE_ALLOC_T(uint, roundedCount, MEMCATEGORY_SCRIPTING);
                                    if (getUInts(k, prop->values.end(), vals, roundedCount))
                                    {
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals, count, 1);
                                            else
                                                params->setConstant(index, vals, roundedCount/4);
                                        }
                                        catch (...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                           "incorrect unsigned integer constant declaration");
                                    }
                                    OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                }
                                else if (atom1->value.find("int") != String::npos)
                                {
                                    // type = GpuProgramParameters::ET_INT;
                                    if (atom1->value.size() >= 4)
                                        count = parseProgramParameterDimensions(atom1->value, String("int"));

                                    // First, clear out any offending auto constants
                                    if (named)
                                        params->clearNamedAutoConstant(name);
                                    else
                                        params->clearAutoConstant(index);

                                    int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                    int *vals = OGRE_ALLOC_T(int, roundedCount, MEMCATEGORY_SCRIPTING);
                                    if (getInts(k, prop->values.end(), vals, roundedCount))
                                    {
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals, count, 1);
                                            else
                                                params->setConstant(index, vals, roundedCount/4);
                                        }
                                        catch (...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                           "incorrect integer constant declaration");
                                    }
                                    OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                }
                                else if (atom1->value.find("double") != String::npos)
                                {
                                    // type = GpuProgramParameters::ET_DOUBLE;
                                    if (atom1->value.size() >= 7)
                                        count = parseProgramParameterDimensions(atom1->value, String("double"));

                                    // First, clear out any offending auto constants
                                    if (named)
                                        params->clearNamedAutoConstant(name);
                                    else
                                        params->clearAutoConstant(index);

                                    int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                    double *vals = OGRE_ALLOC_T(double, roundedCount, MEMCATEGORY_SCRIPTING);
                                    if (getDoubles(k, prop->values.end(), vals, roundedCount))
                                    {
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals, count, 1);
                                            else
                                                params->setConstant(index, vals, roundedCount/4);
                                        }
                                        catch (...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                           "incorrect double constant declaration");
                                    }
                                    OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                }                                
                                else if (atom1->value.find("bool") != String::npos)
                                {
                                    // type = GpuProgramParameters::ET_BOOL;
                                    if (atom1->value.size() >= 5)
                                        count = parseProgramParameterDimensions(atom1->value, String("bool"));

                                    // First, clear out any offending auto constants
                                    if (named)
                                        params->clearNamedAutoConstant(name);
                                    else
                                        params->clearAutoConstant(index);

                                    int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                    uint *vals = OGRE_ALLOC_T(uint, roundedCount, MEMCATEGORY_SCRIPTING);
                                    if (getBooleans(k, prop->values.end(), vals, roundedCount))
                                    {
                                        try
                                        {
                                            if (named)
                                                params->setNamedConstant(name, vals, count, 1);
                                            else
                                                params->setConstant(index, vals, roundedCount/4);
                                        }
                                        catch (...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
                                        }
                                    }
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                           "incorrect boolean constant declaration");
                                    }
                                    OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       "incorrect type specified; only variants of int, uint, float, double, and bool allowed");
                                    // isValid = false;
                                }

                                // if (isValid)
                                // {
                                //     // First, clear out any offending auto constants
                                //     if (named)
                                //         params->clearNamedAutoConstant(name);
                                //     else
                                //         params->clearAutoConstant(index);

                                //     int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;

                                //     if (type == GpuProgramParameters::ET_FLOAT)
                                //     {
                                //       float *vals = OGRE_ALLOC_T(float, roundedCount, MEMCATEGORY_SCRIPTING);
                                //         if (getFloats(k, prop->values.end(), vals, roundedCount))
                                //         {
                                //             try
                                //             {
                                //                 if (named)
                                //                     params->setNamedConstant(name, vals, count, 1);
                                //                 else
                                //                     params->setConstant(index, vals, roundedCount/4);
                                //             }
                                //             catch (...)
                                //             {
                                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                //                                    "setting of constant failed");
                                //             }
                                //         }
                                //         else
                                //         {
                                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                //                                "incorrect float constant declaration");
                                //         }
                                //         OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);  
                                //     }
                                //     else if (type == GpuProgramParameters::ET_INT)
                                //     {
                                //         int *vals = OGRE_ALLOC_T(int, roundedCount, MEMCATEGORY_SCRIPTING);
                                //         if (getInts(k, prop->values.end(), vals, roundedCount))
                                //         {
                                //             try
                                //             {
                                //                 if (named)
                                //                     params->setNamedConstant(name, vals, count, 1);
                                //                 else
                                //                     params->setConstant(index, vals, roundedCount/4);
                                //             }
                                //             catch (...)
                                //             {
                                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                //                                    "setting of constant failed");
                                //             }
                                //         }
                                //         else
                                //         {
                                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                //                                "incorrect integer constant declaration");
                                //         }
                                //         OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                //     }
                                //     else if (type == GpuProgramParameters::ET_DOUBLE)
                                //     {
                                //         double *vals = OGRE_ALLOC_T(double, roundedCount, MEMCATEGORY_SCRIPTING);
                                //         if (getDoubles(k, prop->values.end(), vals, roundedCount))
                                //         {
                                //             try
                                //             {
                                //                 if (named)
                                //                     params->setNamedConstant(name, vals, count, 1);
                                //                 else
                                //                     params->setConstant(index, vals, roundedCount/4);
                                //             }
                                //             catch (...)
                                //             {
                                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                //                                    "setting of constant failed");
                                //             }
                                //         }
                                //         else
                                //         {
                                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                //                                "incorrect double constant declaration");
                                //         }
                                //         OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                //     }
                                //     else if (type == GpuProgramParameters::ET_UINT)
                                //     {
                                //         uint *vals = OGRE_ALLOC_T(uint, roundedCount, MEMCATEGORY_SCRIPTING);
                                //         if (getUInts(k, prop->values.end(), vals, roundedCount))
                                //         {
                                //             try
                                //             {
                                //                 if (named)
                                //                     params->setNamedConstant(name, vals, count, 1);
                                //                 else
                                //                     params->setConstant(index, vals, roundedCount/4);
                                //             }
                                //             catch (...)
                                //             {
                                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                //                                    "setting of constant failed");
                                //             }
                                //         }
                                //         else
                                //         {
                                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                //                                "incorrect unsigned integer constant declaration");
                                //         }
                                //         OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                //     }
                                //     else if (type == GpuProgramParameters::ET_BOOL)
                                //     {
                                //         bool *vals = OGRE_ALLOC_T(bool, roundedCount, MEMCATEGORY_SCRIPTING);
                                //         if (getBooleans(k, prop->values.end(), vals, roundedCount))
                                //         {
                                //             try
                                //             {
                                //                 if (named)
                                //                     params->setNamedConstant(name, vals, count, 1);
                                //                 else
                                //                     params->setConstant(index, vals, roundedCount/4);
                                //             }
                                //             catch (...)
                                //             {
                                //                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                //                                    "setting of constant failed");
                                //             }
                                //         }
                                //         else
                                //         {
                                //             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                //                                "incorrect boolean constant declaration");
                                //         }
                                //         OGRE_FREE(vals, MEMCATEGORY_SCRIPTING);
                                //     }
                                // }
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
                                    // Set the auto constant
                                    try
                                    {
                                        if(named)
                                            params->setNamedAutoConstant(name, def->acType);
                                        else
                                            params->setAutoConstant(index, def->acType);
                                    }
                                    catch(...)
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                           "setting of constant failed");
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
                                        catch(...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
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
                                                catch(...)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       "setting of constant failed");
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                                   "extra parameters required by constant definition " + atom1->value);
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
                                                catch(...)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       "setting of constant failed");
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                   "invalid auto constant extra info parameter");
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
                                        catch(...)
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               "setting of constant failed");
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
                                                catch(...)
                                                {
                                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                       "setting of constant failed");
                                                }
                                            }
                                            else
                                            {
                                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                                   "incorrect float argument definition in extra parameters");
                                            }
                                        }
                                        else
                                        {
                                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                               "extra parameters required by constant definition " + atom1->value);
                                        }
                                    }
                                    break;
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
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
                               "shared_params must be given a name");
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
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }


        for (AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if ((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch (prop->id)
                {
                case ID_SHARED_PARAM_NAMED:
                    {
                        if (prop->values.size() < 2)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "shared_param_named - expected 2 or more arguments");
                            continue;
                        }

                        AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);

                        if ((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "name and parameter type expected");
                            continue;
                        }


                        AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get();

                        String pName = atom0->value;
                        GpuConstantType constType = GCT_UNKNOWN;
                        //size_t arraySz = 1;
                        if (!getConstantType(i1, &constType))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "invalid parameter type");
                            continue;
                        }

                        // bool isFloat = GpuConstantDefinition::isFloat(constType);
                        // bool isInt = GpuConstantDefinition::isInt(constType);
                        // bool isDouble = GpuConstantDefinition::isDouble(constType);
                        // bool isUInt = GpuConstantDefinition::isUnsignedInt(constType);
                        // bool isBool = GpuConstantDefinition::isBool(constType);

                        // FloatConstantList mFloats;
                        // IntConstantList mInts;
                        // DoubleConstantList mDoubles;
                        // UnsignedIntConstantList mUInts;

                        BaseConstantType baseType = GpuConstantDefinition::getBaseType(constType);

                        switch (baseType)
                        {
                        case BCT_FLOAT:
                            translateSharedParamNamed <float, BCT_FLOAT> (compiler, sharedParams, prop, pName, constType);
                            break;
                        case BCT_INT:
                            translateSharedParamNamed <int, BCT_INT> (compiler, sharedParams, prop, pName, constType);
                            break;
                        case BCT_DOUBLE:
                            translateSharedParamNamed <double, BCT_DOUBLE> (compiler, sharedParams, prop, pName, constType);
                            break;
                        case BCT_UINT:
                            translateSharedParamNamed <uint, BCT_UINT> (compiler, sharedParams, prop, pName, constType);
                            break;
                        case BCT_BOOL:
                            translateSharedParamNamed <uint, BCT_BOOL> (compiler, sharedParams, prop, pName, constType);
                            break;
                        default:
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               "invalid parameter type");
                            break;
                        }
                    }

                }
            }
        }
    }
    //-------------------------------------------------------------------------
    template <> float  SharedParamsTranslator::parseParameter<float,  BCT_FLOAT>  (const String& param) { return StringConverter::parseReal(param); }
    template <> int    SharedParamsTranslator::parseParameter<int,    BCT_INT>    (const String& param) { return StringConverter::parseInt(param); }
    template <> double SharedParamsTranslator::parseParameter<double, BCT_DOUBLE> (const String& param) { return StringConverter::parseReal(param); }
    template <> uint   SharedParamsTranslator::parseParameter<uint,   BCT_UINT>   (const String& param) { return StringConverter::parseUnsignedInt(param); }
    template <> uint   SharedParamsTranslator::parseParameter<uint,   BCT_BOOL>   (const String& param) { return StringConverter::parseBool(param); }
    //-------------------------------------------------------------------------
    template <class T, BaseConstantType baseType>
    void SharedParamsTranslator::translateSharedParamNamed(ScriptCompiler *compiler, GpuSharedParameters* sharedParams, PropertyAbstractNode *prop, String pName, GpuConstantType constType)
    {
        std::vector<T> values;

        size_t arraySz = 1;

        AbstractNodeList::const_iterator otherValsi = prop->values.begin();
        std::advance(otherValsi, 2);

        for (; otherValsi != prop->values.end(); ++otherValsi)
        {
            if((*otherValsi)->type != ANT_ATOM)
                continue;

            AtomAbstractNode *atom = (AtomAbstractNode*)(*otherValsi).get();

            if (atom->value.at(0) == '[' && atom->value.at(atom->value.size() - 1) == ']')
            {
                String arrayStr = atom->value.substr(1, atom->value.size() - 2);
                if(!StringConverter::isNumber(arrayStr))
                {
                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                       "invalid array size");
                    continue;
                }
                arraySz = StringConverter::parseInt(arrayStr);
            }
            else
            {
                //TODO bool no longer makes this check true - perhaps another check will do
                // if(!StringConverter::isNumber(atom->value))
                // {
                //     compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                //                        atom->value + " invalid - extra parameters to shared_param_named must be numbers");
                //     continue;
                // }

                values.push_back(parseParameter<T, baseType>(atom->value));
            }

        } // each extra param

        // define constant entry
        try
        {
            sharedParams->addConstantDefinition(pName, constType, arraySz);
        }
        catch(Exception& e)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                               e.getDescription());
            // continue;
            return;
        }

        // initial values
        size_t elemsExpected = GpuConstantDefinition::getElementSize(constType, false) * arraySz;
        // size_t elemsFound = isFloat ? mFloats.size() : mInts.size();
        size_t elemsFound = values.size();
        if (elemsFound)
        {
            if (elemsExpected != elemsFound)
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                   "Wrong number of values supplied for parameter type");
                // continue;
                return;
            }

            // if (isFloat)
            //     sharedParams->setNamedConstant(pName, &mFloats[0], elemsFound);
            // else
            //     sharedParams->setNamedConstant(pName, &mInts[0], elemsFound);

            sharedParams->setNamedConstant(pName, &values[0], elemsFound);
        }
    }

    /**************************************************************************
     * ParticleSystemTranslator
     *************************************************************************/
    ParticleSystemTranslator::ParticleSystemTranslator()
        :mSystem(0)
    {
    }

    void ParticleSystemTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
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
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }

        mSystem->_notifyOrigin(obj->file);

        mSystem->removeAllEmitters();
        mSystem->removeAllAffectors();

        obj->context = Any(mSystem);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
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
                processNode(compiler, *i);
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

        String type;
        if(!getString(obj->values.front(), &type))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
            return;
        }

        ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);
        mEmitter = system->addEmitter(type);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
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
                processNode(compiler, *i);
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

        String type;
        if(!getString(obj->values.front(), &type))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
            return;
        }

        ParticleSystem *system = any_cast<ParticleSystem*>(obj->parent->context);
        mAffector = system->addAffector(type);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
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
                processNode(compiler, *i);
            }
        }
    }

    /**************************************************************************
     * CompositorTextureBaseTranslator
     *************************************************************************/
    void CompositorTextureBaseTranslator::translateTextureProperty( TextureDefinitionBase *defBase,
                                                                    PropertyAbstractNode *prop,
                                                                    ScriptCompiler *compiler ) const
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

        TextureType textureType = TEX_TYPE_2D;
        uint width = 0, height = 0, depth = 1;
        float widthFactor = 1.0f, heightFactor = 1.0f;
        bool widthSet = false, heightSet = false, formatSet = false;
        TextureDefinitionBase::BoolSetting hwGammaWrite = TextureDefinitionBase::BoolUndefined;
        bool fsaa = true;
        bool fsaaExplicitResolve = false;
        uint16 depthBufferId = DepthBuffer::POOL_INVALID;
        PixelFormat depthBufferFormat = PF_UNKNOWN;
        int numMipmaps = 0;
        bool automipmaps = false;
        bool isUav = false;
        bool preferDepthTexture = false;
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
                    uint *pSize;
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
            case ID_TARGET_FORMAT:
                formatSet = true;
                break;
            case ID_GAMMA:
                hwGammaWrite = TextureDefinitionBase::BoolTrue;
                break;
            case ID_NO_GAMMA:
                hwGammaWrite = TextureDefinitionBase::BoolFalse;
                break;
            case ID_NO_FSAA:
                fsaa = false;
                break;
            case ID_EXPLICIT_RESOLVE:
                fsaaExplicitResolve = true;
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

                    depthBufferId = StringConverter::parseInt(atom->value);
                }
                break;
            case ID_DEPTH_TEXTURE:
                preferDepthTexture = true;
                break;
            case ID_DEPTH_FORMAT:
                {
                    // advance to next to get the ID
                    it = getNodeAt(prop->values, static_cast<int>(atomIndex++));
                    if(prop->values.end() == it || (*it)->type != ANT_ATOM)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                    atom = (AtomAbstractNode*)(*it).get();

                    depthBufferFormat = PixelUtil::getFormatFromName(atom->value, false);
                    if( depthBufferFormat == PF_UNKNOWN )
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                }
                break;
            case ID_UAV:
                isUav = true;
                break;
            case ID_MIPMAP:
            case ID_MIPMAPS:
                {
                    // advance to next to get the number of mipmaps
                    it = getNodeAt(prop->values, static_cast<int>(atomIndex++));
                    if(prop->values.end() == it || (*it)->type != ANT_ATOM)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                    atom = (AtomAbstractNode*)(*it).get();
                    if (!StringConverter::isNumber(atom->value))
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }

                    numMipmaps = StringConverter::parseInt(atom->value);
                }
                break;
            case ID_AUTOMIPMAPS:
                automipmaps = true;
                break;
            case ID_2D_ARRAY:   textureType = TEX_TYPE_2D_ARRAY; break;
            case ID_3D:         textureType = TEX_TYPE_3D; break;
            case ID_CUBEMAP:    textureType = TEX_TYPE_CUBE_MAP; break;
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
                    else if (atomIndex == 4)
                    {
                        depth = StringConverter::parseInt(atom->value);
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
                    PixelFormat format = PixelUtil::getFormatFromName(atom->value, false);
                    if (format == PF_UNKNOWN)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                           "Unrecognized PixelFormat");
                        return;
                    }
                    formats.push_back(format);
                    formatSet = true;

                    if( depthBufferId == DepthBuffer::POOL_INVALID )
                    {
                        if( PixelUtil::isDepth( format ) )
                            depthBufferId = DepthBuffer::POOL_NON_SHAREABLE;
                        else if( format == PF_NULL )
                            depthBufferId = DepthBuffer::POOL_NO_DEPTH;
                        else
                            depthBufferId = DepthBuffer::POOL_DEFAULT;
                    }
                }

            }
        }
        if (!widthSet || !heightSet || !formatSet)
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
            return;
        }

        if( textureType == TEX_TYPE_2D )
            depth = 1;
        else if( textureType == TEX_TYPE_CUBE_MAP )
            depth = 6;

        // No errors, create
        TextureDefinitionBase::TextureDefinition *td = defBase->addTextureDefinition( atom0->value );
        td->textureType     = textureType;
        td->width           = width;
        td->height          = height;
        td->depth           = depth;
        td->numMipmaps      = numMipmaps;
        td->widthFactor     = widthFactor;
        td->heightFactor    = heightFactor;
        td->formatList      = formats;
        td->fsaa            = fsaa;
        td->uav             = isUav;
        td->automipmaps     = automipmaps;
        td->hwGammaWrite    = hwGammaWrite;
        td->depthBufferId   = depthBufferId;
        td->depthBufferFormat   = depthBufferFormat;
        td->preferDepthTexture  = preferDepthTexture;
        td->fsaaExplicitResolve = fsaaExplicitResolve;
    }
    //-----------------------------------------------------------------------------------
    void CompositorTextureBaseTranslator::translateBufferProperty( TextureDefinitionBase *defBase,
                                                                   PropertyAbstractNode *prop,
                                                                   ScriptCompiler *compiler ) const
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

        size_t numElements = 0;
        uint32 bytesPerElement = 0;
        float widthFactor = 0.0f, heightFactor = 0.0f;

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
                widthFactor = 1.0f;
                break;
            case ID_TARGET_HEIGHT:
                heightFactor = 1.0f;
                break;
            case ID_TARGET_WIDTH_SCALED:
            case ID_TARGET_HEIGHT_SCALED:
                {
                    float *pFactor;

                    if (atom->id == ID_TARGET_WIDTH_SCALED)
                        pFactor = &widthFactor;
                    else
                        pFactor = &heightFactor;

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

                    *pFactor = StringConverter::parseReal(atom->value);
                }
                break;
            default:
                if (StringConverter::isNumber(atom->value))
                {
                    if (atomIndex == 2)
                    {
                        numElements = StringConverter::parseUnsignedInt(atom->value);
                    }
                    else if (atomIndex == 3)
                    {
                        bytesPerElement = StringConverter::parseUnsignedInt(atom->value);
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    return;
                }
            }
        }

        if( numElements * bytesPerElement == 0u )
        {
            compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                "numElements * bytesPerElement must be non-zero for buffer '" +
                                atom0->value + "'. Syntax is: buffer name size bytesPerElement "
                                "[target_width] [target_height]" );
            return;
        }

        // No errors, create.
        //TODO: Support more flags?
        defBase->addBufferDefinition( atom0->value, numElements, bytesPerElement,
                                      BB_FLAG_UAV, widthFactor, heightFactor );
    }

    /**************************************************************************
     * CompositorWorkspaceTranslator
     *************************************************************************/
    CompositorWorkspaceTranslator::CompositorWorkspaceTranslator() : mWorkspaceDef(0)
    {
    }

    void CompositorWorkspaceTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Create the workspace definition
        CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mWorkspaceDef);
        
        if(!processed)
        {
            CompositorManager2 *compositorMgr = Root::getSingleton().getCompositorManager2();
            mWorkspaceDef = compositorMgr->addWorkspaceDefinition( obj->name );
        }

        if(mWorkspaceDef == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }

        // Prepare the compositor
        obj->context = Any(mWorkspaceDef);

        size_t numTextureDefinitions = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                if( prop->id == ID_TEXTURE )
                    ++numTextureDefinitions;
            }
        }

        mWorkspaceDef->setNumLocalTextureDefinitions( numTextureDefinitions );

        AbstractNodeList::iterator i = obj->children.begin();
        try
        {
        for(i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_TEXTURE:
                    translateTextureProperty( mWorkspaceDef, prop, compiler );
                    break;
                case ID_BUFFER:
                    translateBufferProperty( mWorkspaceDef, prop, compiler );
                    break;
                case ID_ALIAS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "alias only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        IdString aliasName, nodeName;
                        if( getIdString( *it0, &aliasName ) && getIdString( *it1, &nodeName ) )
                            mWorkspaceDef->addNodeAlias( aliasName, nodeName );
                    }
                    break;
                case ID_CONNECT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() < 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect needs at least 2 argument");
                    }
                    else if( prop->values.size() & 0x01 )
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect must have an even number of arguments");
                    }
                    else
                    {
                        size_t numStrings = 0;
                        IdString outNode, inNode;

                        //Find out the names of the out & in nodes.
                        AbstractNodeList::const_iterator itor = prop->values.begin();
                        AbstractNodeList::const_iterator end  = prop->values.end();

                        AbstractNodeList::const_iterator inNodeStart = itor;

                        while( itor != end )
                        {
                            uint32 unused;
                            if( !getUInt( *itor, &unused ) )
                            {
                                if( numStrings >= 2 )
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                    numStrings = 3; //Flag the error somehow
                                }
                                else
                                {
                                    if( !getIdString( *itor, numStrings == 0 ? &outNode : &inNode ) )
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        numStrings = 3; //Flag the error somehow
                                    }
                                    else
                                    {
                                        ++numStrings;
                                        inNodeStart = itor;
                                    }
                                }
                            }
                            ++itor;
                        }

                        if( numStrings != 2 )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "The only non-numeric arguments expected are the 'out node' and 'in node' names");
                        }
                        else
                        {
                            itor = prop->values.begin();
                            ++itor;
                            ++inNodeStart;
                            uint32 outChannel, inChannel;

                            while( itor != prop->values.end() && inNodeStart != prop->values.end() )
                            {
                                getUInt( *itor, &outChannel );
                                getUInt( *inNodeStart, &inChannel );
                                mWorkspaceDef->connect( outNode, outChannel, inNode, inChannel );
                                ++itor;
                                ++inNodeStart;
                            }

                            //No explicit numeric channels provided.
                            if( prop->values.size() == 2 )
                                mWorkspaceDef->connect( outNode, inNode );
                        }
                    }
                    break;
                case ID_CONNECT_BUFFER:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() < 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect_buffer needs at least 2 argument");
                    }
                    else if( prop->values.size() & 0x01 )
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect_buffer must have an even number of arguments");
                    }
                    else
                    {
                        size_t numStrings = 0;
                        IdString outNode, inNode;

                        //Find out the names of the out & in nodes.
                        AbstractNodeList::const_iterator itor = prop->values.begin();
                        AbstractNodeList::const_iterator end  = prop->values.end();

                        AbstractNodeList::const_iterator inNodeStart = itor;

                        while( itor != end )
                        {
                            uint32 unused;
                            if( !getUInt( *itor, &unused ) )
                            {
                                if( numStrings >= 2 )
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                    numStrings = 3; //Flag the error somehow
                                }
                                else
                                {
                                    if( !getIdString( *itor, numStrings == 0 ? &outNode : &inNode ) )
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                                        numStrings = 3; //Flag the error somehow
                                    }
                                    else
                                    {
                                        ++numStrings;
                                        inNodeStart = itor;
                                    }
                                }
                            }
                            ++itor;
                        }

                        if( numStrings != 2 )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "The only non-numeric arguments expected are the 'out node' and 'in node' names");
                        }
                        else
                        {
                            itor = prop->values.begin();
                            ++itor;
                            ++inNodeStart;
                            uint32 outChannel, inChannel;

                            while( itor != prop->values.end() && inNodeStart != prop->values.end() )
                            {
                                getUInt( *itor, &outChannel );
                                getUInt( *inNodeStart, &inChannel );
                                mWorkspaceDef->connectBuffer( outNode, outChannel, inNode, inChannel );
                                ++itor;
                                ++inNodeStart;
                            }

                            //No explicit numeric channels provided.
                            if( prop->values.size() == 2 )
                                mWorkspaceDef->connectBuffer( outNode, inNode );
                        }
                    }
                    break;
                case ID_CONNECT_OUTPUT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2 )
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect_output only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 inChannel;
                        IdString inNode;
                        if( getIdString( *it0, &inNode ) && getUInt( *it1, &inChannel ) )
                            mWorkspaceDef->connectExternal( 0, inNode, inChannel );
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_CONNECT_EXTERNAL:
                case ID_CONNECT_BUFFER_EXTERNAL:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 3 )
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "connect_external & connect_buffer_external only support 3 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it2 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it2++;
                        AbstractNodeList::const_iterator it1 = it2++;

                        uint32 externalChannel;
                        uint32 inChannel;
                        IdString inNode;
                        if( getUInt( *it0, &externalChannel ) && getIdString( *it1, &inNode ) &&
                            getUInt( *it2, &inChannel ) )
                        {
                            if( prop->id == ID_CONNECT_EXTERNAL )
                            {
                                mWorkspaceDef->connectExternal( externalChannel,
                                                                inNode, inChannel );
                            }
                            else
                            {
                                mWorkspaceDef->connectExternalBuffer( externalChannel,
                                                                      inNode, inChannel );
                            }
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                    "token not recognized");
            }
        }
        }
        catch( Exception &e )
        {
            if( i != obj->children.end() )
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, (*i)->file, (*i)->line);
            throw e;
        }
    }

    /**************************************************************************
     * CompositorNodeTranslator
     *************************************************************************/
    CompositorNodeTranslator::CompositorNodeTranslator() : mNodeDef(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositorNodeTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Create the Node definition
        CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mNodeDef);
        
        if(!processed)
        {
            CompositorManager2 *compositorMgr = Root::getSingleton().getCompositorManager2();
            mNodeDef = compositorMgr->addNodeDefinition( obj->name );
        }

        if(mNodeDef == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }

        // Prepare the compositor
        obj->context = Any(mNodeDef);

        size_t numTextureDefinitions = 0;
        size_t numTargetPasses = 0;
        size_t numOutputChannels = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                if( prop->id == ID_TEXTURE )
                    ++numTextureDefinitions;
                else if( prop->id == ID_OUT )
                    ++numOutputChannels;
            }
            else if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *objNode = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !objNode->abstract && objNode->id == ID_TARGET )
                    ++numTargetPasses;
            }
        }

        mNodeDef->setNumLocalTextureDefinitions( numTextureDefinitions );
        mNodeDef->setNumTargetPass( numTargetPasses );
        mNodeDef->setNumOutputChannels( numOutputChannels );

        AbstractNodeList::iterator i = obj->children.begin();
        try
        {
        for(i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_TEXTURE:
                    translateTextureProperty( mNodeDef, prop, compiler );
                    break;
                case ID_BUFFER:
                    translateBufferProperty( mNodeDef, prop, compiler );
                    break;
                case ID_IN:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'in' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 inChannel;
                        String textureName;
                        if( getUInt( *it0, &inChannel ) && getString( *it1, &textureName ) )
                        {
                            mNodeDef->addTextureSourceName( textureName, inChannel,
                                                            TextureDefinitionBase::TEXTURE_INPUT );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_OUT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'out' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 outChannel;
                        String textureName;
                        if( getUInt( *it0, &outChannel ) && getString( *it1, &textureName ) )
                            mNodeDef->mapOutputChannel( outChannel, textureName );
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_IN_BUFFER:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'in_buffer' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 inChannel;
                        IdString bufferName;
                        if( getUInt( *it0, &inChannel ) && getIdString( *it1, &bufferName ) )
                        {
                            mNodeDef->addBufferInput( inChannel, bufferName );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_OUT_BUFFER:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'out_buffer' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 outChannel;
                        IdString bufferName;
                        if( getUInt( *it0, &outChannel ) && getIdString( *it1, &bufferName ) )
                            mNodeDef->mapOutputBufferChannel( outChannel, bufferName );
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_CUSTOM_ID:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getIdString( *it0, &mNodeDef->mCustomIdentifier ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                    "token not recognized");
            }
        }
        }
        catch( Exception &e )
        {
            if( i != obj->children.end() )
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, (*i)->file, (*i)->line);
            throw e;
        }
    }

    /**************************************************************************
     * CompositorShadowNodeTranslator
     *************************************************************************/
    CompositorShadowNodeTranslator::CompositorShadowNodeTranslator() : mShadowNodeDef(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositorShadowNodeTranslator::translateShadowMapProperty(
            PropertyAbstractNode *prop, ScriptCompiler *compiler,
            const ShadowTextureDefinition &defaultParams ) const
    {
        size_t atomIndex = 0;
        AbstractNodeList::const_iterator it = getNodeAt(prop->values, 0);

        if((*it)->type != ANT_ATOM)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return;
        }

        // Save the first atom, should be shadow map's texture name.
        //AtomAbstractNode *atom0 = (AtomAbstractNode*)(*it).get();

        String texName = "";
        uint8       mrtIndex = 0;
        Vector2     uvOffset( Vector2::ZERO );
        Vector2     uvLength( Vector2::UNIT_SCALE );
        uint8       arrayIdx = 0;
        int lightIdx = ~0;
        size_t splitIdx = 0;

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
            case ID_UV:
            {
                if( atomIndex + 4u >= prop->values.size() )
                {
                    compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                        "4 numeric arguments expected" );
                    return;
                }

                // advance to next 4 to get the values.
                AbstractNodeList::const_iterator it0 = ++it;
                AbstractNodeList::const_iterator it1 = ++it;
                AbstractNodeList::const_iterator it2 = ++it;
                AbstractNodeList::const_iterator it3 = ++it;
                atomIndex += 4;

                if( !getReal( *it0, &uvOffset.x ) || !getReal( *it1, &uvOffset.y ) ||
                        !getReal( *it2, &uvLength.x ) || !getReal( *it3, &uvLength.y ) )
                {
                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                }
                break;
            }
            case ID_ARRAY_INDEX:
            {
                if( atomIndex + 1u >= prop->values.size() )
                {
                    compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                        "1 numeric argument expected" );
                    return;
                }

                // advance to next to get the value.
                AbstractNodeList::const_iterator it0 = it++;
                atomIndex += 1;

                uint32 val;
                if( getUInt( *it0, &val ) )
                {
                    arrayIdx = static_cast<uint32>( val );
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                }
            }
                break;
            case ID_LIGHT:
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
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }

                    lightIdx = StringConverter::parseInt(atom->value);
                }
                break;
            case ID_SPLIT:
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
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }

                    splitIdx = StringConverter::parseInt(atom->value);
                }
                break;
            default:
                {
                    bool isValid = false;
                    if( atomIndex == 1 )
                        isValid = true;
                    else if( atomIndex == 2 )
                    {
                        if( getString( *it, &texName ) )
                            isValid = true;
                    }
                    else if( atomIndex == 3 )
                    {
                        uint32 val = 0;
                        if( getUInt( *it, &val ) )
                        {
                            isValid = true;
                            mrtIndex = static_cast<uint32>( val );
                        }
                    }

                    if( !isValid )
                    {
                        compiler->addError( ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                                            "token \"" + prop->name + "\" is not recognized" );
                    }
                }
            }
        }
        if( texName.empty() || lightIdx == ~0 )
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
            return;
        }

        ShadowTextureDefinition *td = mShadowNodeDef->addShadowTextureDefinition(
                    lightIdx, splitIdx, texName, mrtIndex, uvOffset, uvLength, arrayIdx );
        // No errors, create
        td->pssmLambda      = defaultParams.pssmLambda;
        td->splitPadding    = defaultParams.splitPadding;
        td->splitBlend      = defaultParams.splitBlend;
        td->splitFade       = defaultParams.splitFade;
        td->numSplits       = defaultParams.numSplits;
    }
    //-------------------------------------------------------------------------
    void CompositorShadowNodeTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
            return;
        }

        // Create the Node definition
        CreateCompositorScriptCompilerEvent evt(obj->file, obj->name, compiler->getResourceGroup());
        bool processed = compiler->_fireEvent(&evt, (void*)&mShadowNodeDef);
        
        if(!processed)
        {
            CompositorManager2 *compositorMgr = Root::getSingleton().getCompositorManager2();
            mShadowNodeDef = compositorMgr->addShadowNodeDefinition( obj->name );
        }

        if(mShadowNodeDef == 0)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
            return;
        }

        // Prepare the compositor
        obj->context = Any(static_cast<CompositorNodeDef*>( mShadowNodeDef ));

        size_t numTextureDefinitions = 0;
        size_t numShadowMaps = 0;
        size_t numTargetPasses = 0;
        size_t numOutputChannels = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                if( prop->id == ID_TEXTURE )
                    ++numTextureDefinitions;
                else if( prop->id == ID_SHADOW_MAP )
                    ++numShadowMaps;
                else if( prop->id == ID_OUT )
                    ++numOutputChannels;
            }
            else if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract )
                {
                    if( nodeObj->id == ID_TARGET )
                        ++numTargetPasses;
                    else if( nodeObj->id == ID_SHADOW_MAP_TARGET_TYPE )
                    {
                        numTargetPasses +=
                                CompositorShadowMapTargetTypeTranslator::calculateNumTargets( *i );
                    }
                }
            }
        }

        mShadowNodeDef->setNumLocalTextureDefinitions( numTextureDefinitions );
        mShadowNodeDef->setNumShadowTextureDefinitions( numShadowMaps );
        mShadowNodeDef->setNumTargetPass( numTargetPasses );
        mShadowNodeDef->setNumOutputChannels( numOutputChannels );

        ShadowTextureDefinition defaultParams( SHADOWMAP_UNIFORM, "", 0, Vector2::ZERO,
                                               Vector2::UNIT_SCALE, 0, 0, 0 );

        AbstractNodeList::iterator i = obj->children.begin();
        try
        {
        for(i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_TEXTURE:
                    translateTextureProperty( mShadowNodeDef, prop, compiler );
                    break;
                case ID_BUFFER:
                    translateBufferProperty( mShadowNodeDef, prop, compiler );
                    break;
                case ID_TECHNIQUE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it0 = prop->values.begin();

                        String str;
                        if( getString( *it0, &str ) )
                        {
                            if( str == "uniform" )
                                mShadowNodeDef->setDefaultTechnique( SHADOWMAP_UNIFORM );
                            else if( str == "planeoptimal" )
                                mShadowNodeDef->setDefaultTechnique( SHADOWMAP_PLANEOPTIMAL );
                            else if( str == "focused" )
                                mShadowNodeDef->setDefaultTechnique( SHADOWMAP_FOCUSED );
                            else if( str == "pssm" )
                                mShadowNodeDef->setDefaultTechnique( SHADOWMAP_PSSM );
                            else
                            {
                                 compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS,
                                     prop->file, prop->line, "shadow techniques can be: technique "
                                    "[uniform|planeoptimal|focused|lispsm|pssm]");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_NUM_SPLITS:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        }

                        uint32 val;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getUInt( *it0, &val ) )
                        {
                            defaultParams.numSplits = val;
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_PSSM_SPLIT_PADDING:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getReal( *it0, &defaultParams.splitPadding ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_PSSM_SPLIT_BLEND:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getReal( *it0, &defaultParams.splitBlend ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_PSSM_SPLIT_FADE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getReal( *it0, &defaultParams.splitFade ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_PSSM_LAMBDA:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getReal( *it0, &defaultParams.pssmLambda ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                    }
                    break;
                case ID_SHADOW_MAP:
                    translateShadowMapProperty( prop, compiler, defaultParams );
                    break;
                case ID_OUT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'in' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 outChannel;
                        String textureName;
                        if( getUInt( *it0, &outChannel ) && getString( *it1, &textureName ) )
                            mShadowNodeDef->mapOutputChannel( outChannel, textureName );
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                case ID_OUT_BUFFER:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "'out_buffer' only supports 2 arguments");
                    }
                    else
                    {
                        AbstractNodeList::const_iterator it1 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it1++;

                        uint32 outChannel;
                        IdString bufferName;
                        if( getUInt( *it0, &outChannel ) && getIdString( *it1, &bufferName ) )
                            mShadowNodeDef->mapOutputBufferChannel( outChannel, bufferName );
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                    "token not recognized");
            }
        }
        }
        catch( Exception &e )
        {
            if( i != obj->children.end() )
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, (*i)->file, (*i)->line);
            throw e;
        }
    }

    /**************************************************************************
     * CompositorTargetTranslator
     *************************************************************************/
    CompositorTargetTranslator::CompositorTargetTranslator() : mTargetDef(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositorTargetTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        mTargetDef = 0;
        CompositorNodeDef *nodeDef = 0;

        ObjectAbstractNode *parentObj = reinterpret_cast<ObjectAbstractNode*>(obj->parent);

        if( parentObj->id == ID_SHADOW_MAP_REPEAT )
            nodeDef = any_cast<CompositorNodeDef*>(obj->parent->parent->parent->context);
        else
            nodeDef = any_cast<CompositorNodeDef*>(obj->parent->context);

        if( !obj->name.empty() )
        {
            uint32 rtIndex = 0;
            if( !obj->values.empty() )
            {
                AbstractNodeList::const_iterator sliceIt = obj->values.begin();

                if( !getUInt( *sliceIt, &rtIndex ) )
                {
                    String cubemapHint;
                    if( getString( *sliceIt, &cubemapHint ) )
                    {
                        StringUtil::toUpperCase( cubemapHint );

                        if( cubemapHint == "+X" || cubemapHint == "X" )
                        {
                            rtIndex = 0;
                        }
                        else if( cubemapHint == "-X" )
                        {
                            rtIndex = 1;
                        }
                        else if( cubemapHint == "+Y" || cubemapHint == "Y" )
                        {
                            rtIndex = 2;
                        }
                        else if( cubemapHint == "-Y" )
                        {
                            rtIndex = 3;
                        }
                        else if( cubemapHint == "+Z" || cubemapHint == "Z" )
                        {
                            rtIndex = 4;
                        }
                        else if( cubemapHint == "-Z" )
                        {
                            rtIndex = 5;
                        }
                    }
                }
            }

            mTargetDef = nodeDef->addTargetPass( obj->name, rtIndex );
        }
        else
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, node->file, node->line);
            return;
        }

        obj->context = Any(mTargetDef);

        size_t numPasses = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract && nodeObj->id == ID_PASS )
                    ++numPasses;
            }
        }

        mTargetDef->setNumPasses( numPasses );

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else
            {
                compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                                    "token not recognized");
            }
        }
    }

    /**************************************************************************
     * CompositorShadowMapTargetTypeTranslator
     *************************************************************************/
    CompositorShadowMapTargetTypeTranslator::CompositorShadowMapTargetTypeTranslator()
    {
    }
    //-------------------------------------------------------------------------
    size_t CompositorShadowMapTargetTypeTranslator::calculateNumTargets( const AbstractNodePtr &node )
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        size_t numTargetPasses = 0;

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract )
                {
                    if( nodeObj->id == ID_TARGET )
                        ++numTargetPasses;
                    else if( nodeObj->id == ID_SHADOW_MAP )
                    {
                        numTargetPasses += nodeObj->values.size() + 1;
                    }
                    else if( nodeObj->id == ID_SHADOW_MAP_REPEAT )
                    {
                        numTargetPasses +=
                                CompositorShadowMapRepeatTranslator::calculateNumTargets( *i );
                    }
                }
            }
        }

        return numTargetPasses;
    }
    //-------------------------------------------------------------------------
    void CompositorShadowMapTargetTypeTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        uint8 shadowMapSupportedLightTypes = 0;

        size_t numPasses = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract && nodeObj->id == ID_PASS )
                    ++numPasses;
            }
        }

        String lightTypeStr;
        AbstractNodeList::const_iterator namesIt = obj->values.begin();
        for( size_t j=0; j<obj->values.size() + 1; ++j )
        {
            if( !j )
                lightTypeStr = obj->name;
            else
            {
                if( !getString( *namesIt++, &lightTypeStr ) )
                {
                    compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line );
                    return;
                }
            }

            if( lightTypeStr == "directional" )
                shadowMapSupportedLightTypes |= 1u << Light::LT_DIRECTIONAL;
            else if( lightTypeStr == "point" )
                shadowMapSupportedLightTypes |= 1u << Light::LT_POINT;
            else if( lightTypeStr == "spot" )
                shadowMapSupportedLightTypes |= 1u << Light::LT_SPOTLIGHT;
            else
            {
                compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line,
                                    "Unknown light type: '" + lightTypeStr +"'. "
                                    "Valid values for shadow_map_target_type are: "
                                    "directional point spot" );
            }
        }

        obj->context = Any(shadowMapSupportedLightTypes);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else
            {
                compiler->addError( ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                                    "token not recognized" );
            }
        }
    }

    /**************************************************************************
     * CompositorShadowMapRepeatTranslator
     *************************************************************************/
    CompositorShadowMapRepeatTranslator::CompositorShadowMapRepeatTranslator()
    {
    }
    //-------------------------------------------------------------------------
    size_t CompositorShadowMapRepeatTranslator::calculateNumTargets( const AbstractNodePtr &node )
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        size_t numTargetPasses = 0;

        size_t numRepeats = obj->values.size() + 1;

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract )
                {
                    if( nodeObj->id == ID_TARGET || nodeObj->id == ID_SHADOW_MAP )
                        ++numTargetPasses;
                }
            }
        }

        return numTargetPasses * numRepeats;
    }
    //-------------------------------------------------------------------------
    void CompositorShadowMapRepeatTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        const uint8 shadowMapSupportedLightTypes = any_cast<uint8>(obj->parent->context);

        CompositorShadowNodeDef *nodeDef = static_cast<CompositorShadowNodeDef*>(
                    any_cast<CompositorNodeDef*>(obj->parent->parent->context) );

        String targetPassName;
        AbstractNodeList::const_iterator namesIt = obj->values.begin();
        for( size_t j=0; j<obj->values.size() + 1; ++j )
        {
            if( !j )
                targetPassName = obj->name;
            else
            {
                if( !getString( *namesIt++, &targetPassName ) )
                {
                    compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line );
                    return;
                }
            }

            const size_t shadowMapIdx = StringConverter::parseUnsignedInt(
                        targetPassName, std::numeric_limits<size_t>::max() );

            if( shadowMapIdx >= nodeDef->getNumShadowTextureDefinitions() )
            {
                compiler->addError( ScriptCompiler::CE_UNEXPECTEDTOKEN, obj->file, obj->line,
                                    "Shadow map '" + targetPassName + "' not found." );
                return;
            }

            size_t lastNumTargetPasses = nodeDef->getNumTargetPasses();

            obj->context = shadowMapIdx;

            for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
            {
                if((*i)->type == ANT_OBJECT)
                {
                    processNode(compiler, *i);
                }
                else
                {
                    compiler->addError( ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                                        "token not recognized" );
                }
            }

            size_t newNumTargetPasses = nodeDef->getNumTargetPasses();

            for( size_t i=lastNumTargetPasses; i<newNumTargetPasses; ++i )
            {
                CompositorTargetDef *tagetDef = nodeDef->getTargetPass( i );

                tagetDef->setShadowMapSupportedLightTypes( shadowMapSupportedLightTypes );

                const CompositorPassDefVec &compositorPasses = tagetDef->getCompositorPasses();
                CompositorPassDefVec::const_iterator itor = compositorPasses.begin();
                CompositorPassDefVec::const_iterator end  = compositorPasses.end();

                while( itor != end )
                {
                    (*itor)->mShadowMapIdx      = static_cast<uint32>(shadowMapIdx);
                    (*itor)->mIncludeOverlays   = false;
                    ++itor;
                }
            }
        }
    }

    /**************************************************************************
     * CompositorShadowMapTargetTranslator
     *************************************************************************/
    CompositorShadowMapTargetTranslator::CompositorShadowMapTargetTranslator() : mTargetDef(0)
    {
    }
    //-------------------------------------------------------------------------
    void CompositorShadowMapTargetTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        mTargetDef = 0;
        CompositorShadowNodeDef *nodeDef = 0;

        ObjectAbstractNode *parentObj = reinterpret_cast<ObjectAbstractNode*>(obj->parent);
        AbstractNode *targetTypeParent = parentObj->id == ID_SHADOW_MAP_REPEAT ?
                    obj->parent->parent : obj->parent;

        const uint8 shadowMapSupportedLightTypes = any_cast<uint8>(targetTypeParent->context);
        nodeDef = static_cast<CompositorShadowNodeDef*>(
                    any_cast<CompositorNodeDef*>(targetTypeParent->parent->context) );

        if( parentObj->id != ID_SHADOW_MAP_REPEAT )
        {
            if( obj->name.empty() )
            {
                compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, node->file, node->line);
                return;
            }
        }
        else
        {
            if( !obj->name.empty() )
            {
                compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
                return;
            }
        }

        size_t numPasses = 0;
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract && nodeObj->id == ID_PASS )
                    ++numPasses;
            }
        }

        const size_t numShadowMaps = parentObj->id == ID_SHADOW_MAP_REPEAT ?
                    1 : obj->values.size() + 1;

        String targetPassName;
        AbstractNodeList::const_iterator namesIt = obj->values.begin();
        for( size_t j=0; j<numShadowMaps; ++j )
        {
            size_t shadowMapIdx = 0;

            if( parentObj->id != ID_SHADOW_MAP_REPEAT )
            {
                if( !j )
                    targetPassName = obj->name;
                else
                {
                    if( !getString( *namesIt++, &targetPassName ) )
                    {
                        compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line );
                        return;
                    }
                }

                shadowMapIdx = StringConverter::parseUnsignedInt( targetPassName,
                                                                  std::numeric_limits<size_t>::max() );

                if( shadowMapIdx >= nodeDef->getNumShadowTextureDefinitions() )
                {
                    compiler->addError( ScriptCompiler::CE_UNEXPECTEDTOKEN, obj->file, obj->line,
                                        "Shadow map '" + targetPassName + "' not found." );
                    return;
                }
            }
            else
            {
                shadowMapIdx = any_cast<size_t>( obj->parent->context );
            }

            const ShadowTextureDefinition *shadowMapDef =
                    nodeDef->getShadowTextureDefinition( shadowMapIdx );

            mTargetDef = nodeDef->addTargetPass( shadowMapDef->getTextureNameStr() );
            mTargetDef->setNumPasses( numPasses );
            mTargetDef->setShadowMapSupportedLightTypes( shadowMapSupportedLightTypes );
            obj->context = Any(mTargetDef);

            for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
            {
                if((*i)->type == ANT_OBJECT)
                {
                    processNode(compiler, *i);
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, (*i)->file, (*i)->line,
                                        "token not recognized");
                }
            }

            const CompositorPassDefVec &compositorPasses = mTargetDef->getCompositorPasses();
            CompositorPassDefVec::const_iterator itor = compositorPasses.begin();
            CompositorPassDefVec::const_iterator end  = compositorPasses.end();

            while( itor != end )
            {
                (*itor)->mShadowMapIdx      = static_cast<uint32>(shadowMapIdx);
                (*itor)->mIncludeOverlays   = false;
                ++itor;
            }
        }
    }
        
    /**************************************************************************
     * CompositorPassTranslator
     *************************************************************************/
    CompositorPassTranslator::CompositorPassTranslator() : mPassDef(0)
    {
    }

    void CompositorPassTranslator::translateClear(ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                    CompositorTargetDef *targetDef)
    {
        mPassDef = targetDef->addPass( PASS_CLEAR );
        CompositorPassClearDef *passClear = static_cast<CompositorPassClearDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
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
                        passClear->mClearBufferFlags = buffers;
                    }
                    break;
                case ID_COLOUR_VALUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                        if( !getColour(prop->values.begin(), prop->values.end(), &passClear->mColourValue) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_DEPTH_VALUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                        if( !getReal(prop->values.front(), &passClear->mDepthValue) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_STENCIL_VALUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }
                        if( !getUInt(prop->values.front(), &passClear->mStencilValue) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_DISCARD_ONLY:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passClear->mDiscardOnly ) )
                        {
                             compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "discard_only argument must be 'true', 'false', 'yes', 'no', 'on', or 'off'");
                        }
                    }
                    break;
                case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_USES_UAV:
                case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateDepthCopy( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                       CompositorTargetDef *targetDef )
    {
        mPassDef = targetDef->addPass( PASS_DEPTHCOPY );
        CompositorPassDepthCopyDef *passDepthCopy = static_cast<CompositorPassDepthCopyDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        String srcTextureName;
        String dstTextureName;

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_ALIAS_ON_COPY_FAILURE:
                    {
                        if(prop->values.size() != 1)
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line,
                                               "Boolean value expected");
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passDepthCopy->mAliasDepthBufferOnCopyFailure ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_IN:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getString( *it0, &srcTextureName ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_OUT:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getString( *it0, &dstTextureName ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_USES_UAV:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }

        passDepthCopy->setDepthTextureCopy( srcTextureName, dstTextureName );
    }

    void CompositorPassTranslator::translateQuad(ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                    CompositorTargetDef *targetDef)
    {
        mPassDef = targetDef->addPass( PASS_QUAD );
        CompositorPassQuadDef *passQuad = static_cast<CompositorPassQuadDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_USE_QUAD:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passQuad->mUseQuad ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_QUAD_NORMALS:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = reinterpret_cast<AtomAbstractNode*>(prop->values.front().get());
                            if(atom->id == ID_CAMERA_FAR_CORNERS_VIEW_SPACE)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::VIEW_SPACE_CORNERS;
                            else if(atom->id == ID_CAMERA_FAR_CORNERS_VIEW_SPACE_NORMALIZED)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::VIEW_SPACE_CORNERS_NORMALIZED;
                            else if(atom->id == ID_CAMERA_FAR_CORNERS_VIEW_SPACE_NORMALIZED_LH)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::VIEW_SPACE_CORNERS_NORMALIZED_LH;
                            else if(atom->id == ID_CAMERA_FAR_CORNERS_WORLD_SPACE)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::WORLD_SPACE_CORNERS;
                            else if(atom->id == ID_CAMERA_FAR_CORNERS_WORLD_SPACE_CENTERED)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::WORLD_SPACE_CORNERS_CENTERED;
                            else if(atom->id == ID_CAMERA_DIRECTION)
                                passQuad->mFrustumCorners = CompositorPassQuadDef::CAMERA_DIRECTION;
                            else
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_CAMERA:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getIdString( *it0, &passQuad->mCameraName ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_HLMS:
                case ID_MATERIAL:
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
                        passQuad->mMaterialIsHlms = prop->id == ID_HLMS;

                        String val;
                        if( !getString(prop->values.front(), &passQuad->mMaterialName) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
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
                        AbstractNodeList::const_iterator it2 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it2++;
                        AbstractNodeList::const_iterator it1 = it2++;

                        uint32 id;
                        String name;
                        if( getUInt(*it0, &id) && getString(*it1, &name) )
                        {
                            uint32 index = 0;
                            if(it2 != prop->values.end())
                            {
                                if(!getUInt(*it2, &index))
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                                    return;
                                }
                            }

                            passQuad->addQuadTextureSource( id, name, index );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_CAMERA_CUBEMAP_REORIENT:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passQuad->mCameraCubemapReorient ) )
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_USES_UAV:
                case ID_EXPOSE:
                case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateScene(ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                    CompositorTargetDef *targetDef)
    {
        mPassDef = targetDef->addPass( PASS_SCENE );
        CompositorPassSceneDef *passScene = static_cast<CompositorPassSceneDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_LOD_BIAS:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        Real var;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getReal( *it0, &var ) )
                        {
                            passScene->mLodBias = var;
                        }
                        else
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_LOD_UPDATE_LIST:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        bool var;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getBoolean( *it0, &var ) )
                        {
                            passScene->mUpdateLodLists = var;
                        }
                        else
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_CULL_REUSE_DATA:
                    {
                        if( prop->values.empty() )
                        {
                            compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line );
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passScene->mReuseCullData ) )
                        {
                             compiler->addError( ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line );
                        }
                    }
                    break;
                case ID_VISIBILITY_MASK:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        uint32 var;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getHex( *it0, &var ) )
                        {
                            passScene->setVisibilityMask( var );
                        }
                        else
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_LIGHT_VISIBILITY_MASK:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }

                    uint32 var;
                    AbstractNodeList::const_iterator it0 = prop->values.begin();
                    if( getHex( *it0, &var ) )
                    {
                        passScene->setLightVisibilityMask( var );
                    }
                    else
                    {
                         compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                }
                break;
                case ID_SHADOWS_ENABLED:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }
                        else if(prop->values.size() > 2)
                        {
                            compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        AbstractNodeList::const_iterator it1 = it0;
                        if( prop->values.size() > 1 )
                            ++it1;

                        String str;
                        if( getString( *it0, &str ) )
                        {
                            if( str == "off" )
                                passScene->mShadowNode = IdString();
                            else
                            {
                                passScene->mShadowNode = IdString( str );
                                passScene->mShadowNodeRecalculation = SHADOW_NODE_FIRST_ONLY;

                                if( prop->values.size() > 1 && getString( *it1, &str ) )
                                {
                                    if( str == "reuse" )
                                        passScene->mShadowNodeRecalculation = SHADOW_NODE_REUSE;
                                    else if( str == "recalculate" )
                                        passScene->mShadowNodeRecalculation = SHADOW_NODE_RECALCULATE;
                                    else if( str == "first" )
                                        passScene->mShadowNodeRecalculation = SHADOW_NODE_FIRST_ONLY;
                                    else
                                    {
                                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS,
                                                prop->file, prop->line,
                                                "Valid options are reuse, recalculate and first");
                                    }
                                }
                            }
                        }
                        else
                        {
                             compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "shadow property can be either 'shadow off' or 'shadow myNodeName "
                                "[first|reuse|recalculate]'");
                        }
                    }
                    break;
                case ID_CAMERA:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getIdString( *it0, &passScene->mCameraName ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_LOD_CAMERA:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getIdString( *it0, &passScene->mLodCameraName ) )
                        {
                             compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_CULL_CAMERA:
                    {
                        if( prop->values.empty() )
                        {
                            compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line );
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getIdString( *it0, &passScene->mCullCameraName ) )
                        {
                             compiler->addError( ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line );
                        }
                    }
                    break;
                case ID_FIRST_RENDER_QUEUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        uint32 val;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getUInt( *it0, &val ) )
                        {
                            passScene->mFirstRQ = val;
                        }
                        else
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_LAST_RENDER_QUEUE:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        uint32 val;
                        String str;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( getUInt( *it0, &val ) )
                        {
                            passScene->mLastRQ = std::min<uint32>( val, std::numeric_limits<uint8>::max() );
                        }
                        else if( getString( *it0, &str ) && str == "max" )
                        {
                            passScene->mLastRQ = std::numeric_limits<uint8>::max();
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line,
                                                "Expected a number between 0 & 255, or the word 'max'" );
                        }
                    }
                    break;
                case ID_CAMERA_CUBEMAP_REORIENT:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passScene->mCameraCubemapReorient ) )
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_ENABLE_FORWARDPLUS:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &passScene->mEnableForwardPlus ) )
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_IS_PREPASS:
                    {
                        if(prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        bool createPrePass = false;
                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if( !getBoolean( *it0, &createPrePass ) )
                        {
                             compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }
                        else if( createPrePass )
                        {
                            passScene->mPrePassMode = PrePassCreate;
                        }
                    }
                    break;
                case ID_USE_PREPASS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() != 2 && prop->values.size() != 3)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "use_prepass only supports 2 or 3 arguments: the GBuffer's' "
                                           "texture name, the depth texture, & optionally the SSR texture name");
                    }
                    else
                    {
                        IdString gbuffer, gbufDepthTexture, ssr;

                        AbstractNodeList::const_iterator it2 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it2++;
                        AbstractNodeList::const_iterator it1 = it2++;

                        if( !getIdString( *it0, &gbuffer ) ||
                            !getIdString( *it1, &gbufDepthTexture ) ||
                            (it2 != prop->values.end() && !getIdString( *it2, &ssr )) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "use_prepass must be the name of a texture available in the local or global scope");
                        }
                        else
                        {
                            passScene->setUseDepthPrePass( gbuffer, gbufDepthTexture, ssr );
                        }
                    }
                    break;
                case ID_MATERIAL_SCHEME:
                    {
                        if (prop->values.empty())
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                            return;
                        }

                        AbstractNodeList::const_iterator it0 = prop->values.begin();
                        if (!getString(*it0, &passScene->mMaterialScheme))
                        {
                            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_USES_UAV:
                case ID_EXPOSE:
                case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateStencil(ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                    CompositorTargetDef *targetDef)
    {
        mPassDef = targetDef->addPass( PASS_STENCIL );
        CompositorPassStencilDef *passStencil = static_cast<CompositorPassStencilDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                ObjectAbstractNode *nodeObj = reinterpret_cast<ObjectAbstractNode*>( i->get() );
                if( !nodeObj->abstract && (nodeObj->id == ID_BOTH ||
                    nodeObj->id == ID_FRONT || nodeObj->id == ID_BACK) )
                {
                    StencilStateOp *stencilStateOp = 0;
                    switch ( nodeObj->id )
                    {
                    case ID_BOTH:
                        stencilStateOp = &passStencil->mStencilParams.stencilFront;
                        translateStencilFace( compiler, *i, stencilStateOp );
                        stencilStateOp = &passStencil->mStencilParams.stencilBack;
                        translateStencilFace( compiler, *i, stencilStateOp );
                        break;
                    case ID_FRONT:
                        stencilStateOp = &passStencil->mStencilParams.stencilFront;
                        translateStencilFace( compiler, *i, stencilStateOp );
                        break;
                    case ID_BACK:
                        stencilStateOp = &passStencil->mStencilParams.stencilBack;
                        translateStencilFace( compiler, *i, stencilStateOp );
                        break;
                    }
                }
                else
                {
                    processNode(compiler, *i);
                }
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_CHECK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getBoolean(prop->values.front(), &passStencil->mStencilParams.enabled))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                case ID_REF_VALUE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getUInt(prop->values.front(), &passStencil->mStencilRef))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                case ID_MASK:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }
                    uint32 mask = passStencil->mStencilParams.writeMask;
                    if(!getHex(prop->values.front(), &mask))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    passStencil->mStencilParams.writeMask = static_cast<uint8>( mask );
                }
                    break;
                case ID_READ_MASK:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }
                    uint32 mask = passStencil->mStencilParams.readMask;
                    if(!getHex(prop->values.front(), &mask))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);

                    passStencil->mStencilParams.readMask = static_cast<uint8>( mask );
                }
                    break;
                case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_USES_UAV:
                case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line, 
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateUav( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                 CompositorTargetDef *targetDef )
    {
        mPassDef = targetDef->addPass( PASS_UAV );
        CompositorPassUavDef *passUav = static_cast<CompositorPassUavDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_UAV:
                case ID_UAV_EXTERNAL:
                    if(prop->values.size() < 1)
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 8)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if( prop->values.size() == 1 )
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        // Clearing the UAV
                        passUav->setUav( slot, false, "", 0, ResourceAccess::Read, 0, PF_UNKNOWN );
                    }
                    else
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        ++j;

                        String val;
                        if(getString(*j, &val))
                        {
                            uint32 access = 0;
                            uint32 mrtIndex = 0;
                            PixelFormat format = PF_UNKNOWN;
                            int32 mipmap = 0;
                            bool mipmapFollows = false;

                            bool isExternal = prop->id == ID_UAV_EXTERNAL;

                            ++j;
                            while(j != prop->values.end())
                            {
                                if((*j)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
                                    switch(atom->id)
                                    {
                                    case ID_READ:
                                        access |= ResourceAccess::Read;
                                    case ID_WRITE:
                                        access |= ResourceAccess::Write;
                                        break;
                                    case ID_MIPMAP:
                                        mipmapFollows = true;
                                        break;
                                    default:
                                        if(StringConverter::isNumber(atom->value))
                                        {
                                            if( mipmapFollows )
                                            {
                                                mipmap = StringConverter::parseInt(atom->value);
                                                mipmapFollows = false;
                                            }
                                            else
                                                mrtIndex = StringConverter::parseInt(atom->value);
                                        }
                                        else
                                            format = PixelUtil::getFormatFromName(atom->value, true);
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*j)->getValue() + " is not a supported argument to the texture property");
                                }
                                ++j;
                            }

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::UAV, val);
                            compiler->_fireEvent(&evt, 0);

                            if( !access )
                            {
                                compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                    "UAV must have the 'read' and/or 'write' access tokens." );
                            }

                            passUav->setUav( slot, isExternal, evt.mName, mrtIndex,
                                             static_cast<ResourceAccess::ResourceAccess>(access),
                                             mipmap, format );
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*j)->getValue() + " is not a valid texture name");
                    }
                    break;
                case ID_UAV_BUFFER:
                    if(prop->values.size() < 1)
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 6)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else if( prop->values.size() == 1 )
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        // Clearing the UAV
                        passUav->addUavBuffer( slot, IdString(), ResourceAccess::Read, 0, 0 );
                    }
                    else
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        ++j;

                        String val;
                        if(getString(*j, &val))
                        {
                            uint32 access = 0;
                            size_t offset = 0;
                            bool offsetSet = false;
                            size_t sizeBytes = 0;

                            ++j;
                            while(j != prop->values.end())
                            {
                                if((*j)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
                                    switch(atom->id)
                                    {
                                    case ID_READ:
                                        access |= ResourceAccess::Read;
                                    case ID_WRITE:
                                        access |= ResourceAccess::Write;
                                        break;
                                    default:
                                        if(StringConverter::isNumber(atom->value))
                                        {
                                            if( !offsetSet )
                                            {
                                                offset = StringConverter::parseUnsignedInt(atom->value);
                                                offsetSet = true;
                                            }
                                            else
                                            {
                                                sizeBytes = StringConverter::parseUnsignedInt(atom->value);
                                            }
                                        }
                                        else
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               (*j)->getValue() + " is not a supported argument to the uav_buffer property");
                                        }
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*j)->getValue() + " is not a supported argument to the uav_buffer property");
                                }
                                ++j;
                            }

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::UAV_BUFFER, val);
                            compiler->_fireEvent(&evt, 0);

                            if( !access )
                            {
                                compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                    "UAV must have the 'read' and/or 'write' access tokens." );
                            }
                            passUav->addUavBuffer( slot, evt.mName,
                                                   static_cast<ResourceAccess::ResourceAccess>(access),
                                                   offset, sizeBytes );
                        }
                        else
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*j)->getValue() + " is not a valid texture name");
                    }
                    break;
                case ID_STARTING_SLOT:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }

                    uint32 val;
                    AbstractNodeList::const_iterator it0 = prop->values.begin();
                    if( getUInt( *it0, &val ) )
                    {
                        passUav->mStartingSlot = static_cast<uint8>( val );
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                }
                break;
                case ID_KEEP_PREVIOUS_UAV:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }

                    AbstractNodeList::const_iterator it0 = prop->values.begin();
                    if( !getBoolean( *it0, &passUav->mKeepPreviousUavs ) )
                    {
                         compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                }
                break;
                //case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                //case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                //case ID_USES_UAV:
                //case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateCompute( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                     CompositorTargetDef *targetDef )
    {
        mPassDef = targetDef->addPass( PASS_COMPUTE );
        CompositorPassComputeDef *passCompute = static_cast<CompositorPassComputeDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_JOB:
                {
                    if(prop->values.size() != 1)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }

                    String jobName;
                    if( !getString(prop->values.front(), &jobName) )
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }

                    passCompute->mJobName = jobName;
                }
                    break;
                case ID_CAMERA:
                {
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }

                    AbstractNodeList::const_iterator it0 = prop->values.begin();
                    if( !getIdString( *it0, &passCompute->mCameraName ) )
                    {
                         compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                }
                break;
                case ID_UAV:
                    if(prop->values.size() < 3)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 5)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        uint32 mrtIndex = 0;
                        PixelFormat pixelFormat = PF_UNKNOWN;
                        int32 slice = 0;
                        int32 mipmap = 0;
                        uint32 access = ResourceAccess::Undefined;
                        bool allowWriteAfterWrite = false;

                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        ++j;

                        String val;
                        if(getString(*j, &val))
                        {
                            bool mipmapFollows = false;

                            ++j;
                            while(j != prop->values.end())
                            {
                                if((*j)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
                                    switch(atom->id)
                                    {
                                    case ID_READ:
                                        access |= ResourceAccess::Read;
                                    case ID_WRITE:
                                        access |= ResourceAccess::Write;
                                        break;
                                    case ID_ALLOW_WRITE_AFTER_WRITE:
                                        allowWriteAfterWrite = true;
                                        break;
                                    case ID_MIPMAP:
                                        mipmapFollows = true;
                                        break;
                                    default:
                                        if(StringConverter::isNumber(atom->value))
                                        {
                                            if( mipmapFollows )
                                            {
                                                mipmap = StringConverter::parseInt(atom->value);
                                                mipmapFollows = false;
                                            }
                                            else
                                                mrtIndex = StringConverter::parseInt(atom->value);
                                        }
                                        else
                                            pixelFormat = PixelUtil::getFormatFromName(atom->value, true);
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*j)->getValue() + " is not a supported argument to the uav property");
                                }
                                ++j;
                            }

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::UAV, val);
                            compiler->_fireEvent(&evt, 0);

                            if( !access )
                            {
                                compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                    "UAV must have the 'read' and/or 'write' access tokens." );
                            }

                            passCompute->addUavSource( slot, evt.mName, mrtIndex,
                                                       static_cast<ResourceAccess::ResourceAccess>(access),
                                                       slice, mipmap, pixelFormat,
                                                       allowWriteAfterWrite );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*j)->getValue() + " is not a valid uav name");
                        }
                    }
                    break;
                case ID_UAV_BUFFER:
                    if(prop->values.size() < 3)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        return;
                    }
                    else if (prop->values.size() > 5)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
                        return;
                    }
                    else
                    {
                        AbstractNodeList::const_iterator j = prop->values.begin();

                        uint32 slot = ~0u;
                        size_t offset = 0;
                        bool offsetSet = false;
                        size_t sizeBytes = 0;
                        uint32 access = ResourceAccess::Undefined;
                        bool allowWriteAfterWrite = false;

                        if( !getUInt( *j, &slot ) )
                        {
                            compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            return;
                        }

                        ++j;

                        String val;
                        if(getString(*j, &val))
                        {
                            ++j;
                            while(j != prop->values.end())
                            {
                                if((*j)->type == ANT_ATOM)
                                {
                                    AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
                                    switch(atom->id)
                                    {
                                    case ID_READ:
                                        access |= ResourceAccess::Read;
                                    case ID_WRITE:
                                        access |= ResourceAccess::Write;
                                        break;
                                    case ID_ALLOW_WRITE_AFTER_WRITE:
                                        allowWriteAfterWrite = true;
                                        break;
                                    default:
                                        if(StringConverter::isNumber(atom->value))
                                        {
                                            if( !offsetSet )
                                            {
                                                offset = StringConverter::parseUnsignedInt(atom->value);
                                                offsetSet = true;
                                            }
                                            else
                                            {
                                                sizeBytes = StringConverter::parseUnsignedInt(atom->value);
                                            }
                                        }
                                        else
                                        {
                                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                               (*j)->getValue() + " is not a supported argument to the uav_buffer property");
                                        }
                                    }
                                }
                                else
                                {
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                       (*j)->getValue() + " is not a supported argument to the uav_buffer property");
                                }
                                ++j;
                            }

                            ProcessResourceNameScriptCompilerEvent evt(ProcessResourceNameScriptCompilerEvent::UAV_BUFFER, val);
                            compiler->_fireEvent(&evt, 0);

                            if( !access )
                            {
                                compiler->addError( ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                    "UAV must have the 'read' and/or 'write' access tokens." );
                            }

                            passCompute->addUavBuffer( slot, evt.mName,
                                                       static_cast<ResourceAccess::ResourceAccess>(access),
                                                       offset, sizeBytes, allowWriteAfterWrite );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               (*j)->getValue() + " is not a valid buffer name");
                        }
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
                        AbstractNodeList::const_iterator it2 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it2++;
                        AbstractNodeList::const_iterator it1 = it2++;

                        uint32 id;
                        String name;
                        if( getUInt(*it0, &id) && getString(*it1, &name) )
                        {
                            uint32 index = 0;
                            if(it2 != prop->values.end())
                            {
                                if(!getUInt(*it2, &index))
                                {
                                    compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                                    return;
                                }
                            }

                            passCompute->addTextureSource( id, name, index );
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                //case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                //case ID_OVERLAYS:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                //case ID_USES_UAV:
                //case ID_COLOUR_WRITE:
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateMipmap( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                    CompositorTargetDef *targetDef )
    {
        mPassDef = targetDef->addPass( PASS_MIPMAP );
        CompositorPassMipmapDef *passMipmap = static_cast<CompositorPassMipmapDef*>( mPassDef );

        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_MIPMAP_METHOD:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "mipmap_method must have at most 1 argument");
                    }
                    else
                    {
                        if(prop->values.front()->type == ANT_ATOM)
                        {
                            AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
                            switch(atom->id)
                            {
                            case ID_API_DEFAULT:
                                passMipmap->mMipmapGenerationMethod = CompositorPassMipmapDef::ApiDefault;
                                break;
                            case ID_COMPUTE:
                                passMipmap->mMipmapGenerationMethod = CompositorPassMipmapDef::Compute;
                                break;
                            case ID_COMPUTE_HQ:
                                passMipmap->mMipmapGenerationMethod = CompositorPassMipmapDef::ComputeHQ;
                                break;
                            default:
                                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                   prop->values.front()->getValue() +
                                                   " is not a valid miprmap_method (api_default, compute, or compute_hq)");
                            }
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() +
                                               " is not a valid miprmap_method (api_default, compute, or compute_hq)");
                        }
                    }
                break;
                case ID_KERNEL_RADIUS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "kernel_radius must have at most 1 argument");
                    }
                    else
                    {
                        int value = 8;
                        if( !getInt( prop->values.front(), &value ) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer argument");
                        }
                        else
                        {
                            passMipmap->mKernelRadius = (uint8)Math::Clamp( value, 0, 255 );
                        }
                    }
                break;
                case ID_GAUSS_DEVIATION:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "gauss_deviation must have at most 1 argument");
                    }
                    else
                    {
                        if( !getFloat( prop->values.front(), &passMipmap->mGaussianDeviationFactor ) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                               prop->values.front()->getValue() + " is not a valid integer argument");
                        }
                    }
                break;

                //case ID_VIEWPORT:
                case ID_IDENTIFIER:
                case ID_NUM_INITIAL:
                case ID_EXECUTION_MASK:
                case ID_VIEWPORT_MODIFIER_MASK:
                case ID_PROFILING_ID:
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translateStencilFace( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                                         StencilStateOp *stencilStateOp )
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_OBJECT)
            {
                processNode(compiler, *i);
            }
            else if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_COMP_FUNC:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getCompareFunction(prop->values.front(), &stencilStateOp->compareOp))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                case ID_PASS_OP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getStencilOp(prop->values.front(), &stencilStateOp->stencilPassOp))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                case ID_FAIL_OP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getStencilOp(prop->values.front(), &stencilStateOp->stencilFailOp))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                case ID_DEPTH_FAIL_OP:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                        return;
                    }
                    if(!getStencilOp(prop->values.front(), &stencilStateOp->stencilDepthFailOp))
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                default:
                    compiler->addError(ScriptCompiler::CE_UNEXPECTEDTOKEN, prop->file, prop->line,
                        "token \"" + prop->name + "\" is not recognized");
                }
            }
        }
    }

    void CompositorPassTranslator::translate(ScriptCompiler *compiler, const AbstractNodePtr &node)
    {
        ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());

        CompositorTargetDef *target = any_cast<CompositorTargetDef*>(obj->parent->context);

        // The name is the type of the pass
        if(obj->name.empty())
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, obj->file, obj->line);
            return;
        }

        if(obj->name == "clear")
            translateClear( compiler, node, target );
        else if(obj->name == "stencil")
            translateStencil( compiler, node, target );
        else if(obj->name == "render_quad")
            translateQuad( compiler, node, target );
        else if(obj->name == "render_scene")
            translateScene( compiler, node, target );
        else if(obj->name == "depth_copy")
            translateDepthCopy( compiler, node, target );
        else if(obj->name == "bind_uav")
            translateUav( compiler, node, target );
        else if(obj->name == "compute")
            translateCompute( compiler, node, target );
        else if(obj->name == "generate_mipmaps")
            translateMipmap( compiler, node, target );
        else if(obj->name == "custom")
        {
            IdString customId;
            if( !obj->values.empty() )
            {
                // Get the custom name
                String type;
                if( !getString(obj->values.front(), &type) )
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line);
                    return;
                }

                customId = type;
            }

            mPassDef = target->addPass( PASS_CUSTOM, customId );

            //allow the custom pass provider to add any custom properties to pass definition
            CompositorManager2* compMgr = Root::getSingleton().getCompositorManager2();
            CompositorPassProvider* passProv = compMgr->getCompositorPassProvider();
            if (passProv)
            {
                passProv->translateCustomPass(node, mPassDef);
            }
        }
        else
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line,
                "pass types must be \"clear\", \"stencil\", \"render_quad\", "
                "\"render_scene\", \"uav_queue\" or \"custom\".");
            return;
        }

        obj->context = Any(mPassDef);

        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
                switch(prop->id)
                {
                case ID_VIEWPORT:
                    {
                        if(prop->values.size() != 4 && prop->values.size() != 8)
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                                "4 or 8 numeric arguments expected");
                            return;
                        }
                        AbstractNodeList::const_iterator it3 = prop->values.begin();
                        AbstractNodeList::const_iterator it0 = it3++;
                        AbstractNodeList::const_iterator it1 = it3++;
                        AbstractNodeList::const_iterator it2 = it3++;

                        if( !getFloat( *it0, &mPassDef->mVpLeft ) || !getFloat( *it1, &mPassDef->mVpTop ) ||
                            !getFloat( *it2, &mPassDef->mVpWidth ) || !getFloat( *it3, &mPassDef->mVpHeight ) )
                        {
                                compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                        }

                        if( prop->values.size() == 8 )
                        {
                            AbstractNodeList::const_iterator it7 = ++it3;
                            AbstractNodeList::const_iterator it4 = it7++;
                            AbstractNodeList::const_iterator it5 = it7++;
                            AbstractNodeList::const_iterator it6 = it7++;

                            if( !getFloat( *it4, &mPassDef->mVpScissorLeft ) ||
                                !getFloat( *it5, &mPassDef->mVpScissorTop ) ||
                                !getFloat( *it6, &mPassDef->mVpScissorWidth ) ||
                                !getFloat( *it7, &mPassDef->mVpScissorHeight ) )
                            {
                                 compiler->addError(ScriptCompiler::CE_NUMBEREXPECTED, prop->file, prop->line);
                            }
                        }
                        else
                        {
                            mPassDef->mVpScissorLeft    = mPassDef->mVpLeft;
                            mPassDef->mVpScissorTop     = mPassDef->mVpTop;
                            mPassDef->mVpScissorWidth   = mPassDef->mVpWidth;
                            mPassDef->mVpScissorHeight  = mPassDef->mVpHeight;
                        }
                    }
                    break;

                case ID_IDENTIFIER:
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
                        if( !getUInt(prop->values.front(), &mPassDef->mIdentifier) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_NUM_INITIAL:
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
                        if( !getUInt(prop->values.front(), &mPassDef->mNumInitialPasses) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
                case ID_OVERLAYS:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "overlays only supports 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &mPassDef->mIncludeOverlays))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "overlays argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_EXECUTION_MASK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "execution_mask only supports 1 argument");
                    }
                    else
                    {
                        uint32 val;
                        if( !getHex(prop->values.front(), &val) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "execution_mask argument must be an 8-bit hexadecimal value");
                        }
                        else
                        {
                            mPassDef->mExecutionMask = static_cast<uint8>( val & 0xFF );
                        }
                    }
                    break;
                case ID_VIEWPORT_MODIFIER_MASK:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "viewport_mask only supports 1 argument");
                    }
                    else
                    {
                        uint32 val;
                        if( !getHex(prop->values.front(), &val) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "viewport_mask argument must be an 8-bit hexadecimal value");
                        }
                        else
                        {
                            mPassDef->mViewportModifierMask = static_cast<uint8>( val & 0xFF );
                        }
                    }
                    break;
                case ID_USES_UAV:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() < 2 || prop->values.size() > 4)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                                           "uses_uav needs between 2 and 4 arguments");
                    }
                    else
                    {
                        uint32 uavSlot              = 0;
                        uint32 access               = ResourceAccess::Undefined;
                        bool   allowWriteAfterWrite = false;

                        if(!getUInt( prop->values.front(), &uavSlot ))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "uses_uav should be 'uses_uav <number between 0 and 63> "
                                "<[read] [write]> [allow_write_after_write]'");
                        }

                        //Skip the slot #
                        AbstractNodeList::const_iterator it = ++prop->values.begin();
                        AbstractNodeList::const_iterator en = prop->values.end();

                        while( it != en )
                        {
                            if( (*it)->type == ANT_ATOM )
                            {
                                AtomAbstractNode *atom = (AtomAbstractNode*)(*it).get();
                                switch( atom->id )
                                {
                                case ID_READ:
                                    access |= ResourceAccess::Read;
                                case ID_WRITE:
                                    access |= ResourceAccess::Write;
                                    break;
                                case ID_ALLOW_WRITE_AFTER_WRITE:
                                    allowWriteAfterWrite = true;
                                    break;
                                default:
                                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                        "unrecognized token in uses_uav: " + atom->getValue() );
                                    break;
                                }
                            }

                            ++it;
                        }

                        CompositorPassDef::UavDependency uavDep(
                                    uavSlot, static_cast<ResourceAccess::ResourceAccess>(access),
                                    allowWriteAfterWrite );
                        mPassDef->mUavDependencies.push_back( uavDep );
                    }
                    break;
                case ID_EXPOSE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "expose only supports 1 argument");
                    }
                    else
                    {
                        IdString val;
                        if( !getIdString(prop->values.front(), &val) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "expose must be the name of a texture available in the local or global scope");
                        }
                        else
                        {
                            mPassDef->mExposedTextures.push_back( val );
                        }
                    }
                    break;
                case ID_COLOUR_WRITE:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "colour_write only supports 1 argument");
                    }
                    else
                    {
                        if(!getBoolean(prop->values.front(), &mPassDef->mColourWrite))
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "colour_write argument must be \"true\", \"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_SHADOW_MAP_FULL_VIEWPORT:
                    if(prop->values.empty())
                    {
                        compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
                    }
                    else if(prop->values.size() > 1)
                    {
                        compiler->addError(ScriptCompiler::CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line,
                            "shadow_map_full_viewport only supports 1 argument");
                    }
                    else
                    {
                        if( !getBoolean( prop->values.front(), &mPassDef->mShadowMapFullViewport ) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line,
                                "shadow_map_full_viewport argument must be \"true\", "
                                "\"false\", \"yes\", \"no\", \"on\", or \"off\"");
                        }
                    }
                    break;
                case ID_PROFILING_ID:
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
                        if( !getString(prop->values.front(), &mPassDef->mProfilingId) )
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }
                    break;
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
    size_t BuiltinScriptTranslatorManager::getNumTranslators() const
    {
        return 12;
    }
    //-------------------------------------------------------------------------
    ScriptTranslator *BuiltinScriptTranslatorManager::getTranslator(const AbstractNodePtr &node)
    {
        ScriptTranslator *translator = 0;

        if(node->type == ANT_OBJECT)
        {
            ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());
            ObjectAbstractNode *parent = obj->parent ? static_cast<ObjectAbstractNode*>(obj->parent) : 0;
            if(obj->id == ID_HLMS)
                translator = &mHlmsTranslator;
            else if(obj->id == ID_MATERIAL)
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
            else if(obj->id == ID_WORKSPACE)
                translator = &mCompositorWorkspaceTranslator;
            else if(obj->id == ID_COMPOSITOR_NODE)
                translator = &mCompositorNodeTranslator;
            else if(obj->id == ID_SHADOW_NODE)
                translator = &mCompositorShadowNodeTranslator;
            else if(obj->id == ID_TARGET && parent &&
                    (parent->id == ID_COMPOSITOR_NODE || parent->id == ID_SHADOW_NODE ||
                     parent->id == ID_SHADOW_MAP_REPEAT))
                translator = &mCompositorTargetTranslator;
            else if(obj->id == ID_SHADOW_MAP_TARGET_TYPE && parent && parent->id == ID_SHADOW_NODE)
                translator = &mCompositorShadowMapTargetTypeTranslator;
            else if(obj->id == ID_SHADOW_MAP_REPEAT && parent && parent->id == ID_SHADOW_MAP_TARGET_TYPE)
                translator = &mCompositorShadowMapRepeatTranslator;
            else if(obj->id == ID_SHADOW_MAP && parent &&
                    (parent->id == ID_SHADOW_MAP_TARGET_TYPE || parent->id == ID_SHADOW_MAP_REPEAT))
                translator = &mCompositorShadowMapTargetTranslator;
            else if(obj->id == ID_PASS && parent && (parent->id == ID_TARGET || parent->id == ID_SHADOW_MAP))
                translator = &mCompositorPassTranslator;
        }

        return translator;
    }
}
