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
#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------------
SGScriptTranslator::SGScriptTranslator() :
    mGeneratedRenderState(NULL)
{
}

//-----------------------------------------------------------------------------
void SGScriptTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
    ObjectAbstractNode* obj = static_cast<ObjectAbstractNode*>(node.get());
    ObjectAbstractNode* parent = static_cast<ObjectAbstractNode*>(obj->parent);

    // Translate section within a pass context.
    if (parent->id == ID_PASS)
    {
        translatePass(compiler, node);
    }
    if (parent->id == ID_TEXTURE_UNIT)
    {
        translateTextureUnit(compiler, node);
    }
}

//-----------------------------------------------------------------------------
SubRenderState* SGScriptTranslator::getGeneratedSubRenderState(const String& typeName)
{
    //check if we are in the middle of parsing
    if (mGeneratedRenderState)
    {
        /** Get the list of the template sub render states composing this render state. */
        const SubRenderStateList& rsList =
            mGeneratedRenderState->getTemplateSubRenderStateList();
        
        SubRenderStateList::const_iterator it = rsList.begin();
        SubRenderStateList::const_iterator itEnd = rsList.end();
        for(; it != itEnd; ++it)
        {
            if ((*it)->getType() == typeName)
                return *it;
        }
    }
    return NULL;
}
    
//-----------------------------------------------------------------------------
/*
note: we can know the texture unit index by getting parent then finding it in the list of children
*/
void SGScriptTranslator::translateTextureUnit(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
    ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());    
    TextureUnitState* texState = any_cast<TextureUnitState*>(obj->parent->context);
    Pass* pass = texState->getParent();
    Technique* technique = pass->getParent();
    Material* material = technique->getParent();
    ShaderGenerator* shaderGenerator = ShaderGenerator::getSingletonPtr();
    String dstTechniqueSchemeName = obj->name;
    bool techniqueCreated;

    // Make sure the scheme name is valid - use default if none exists.
    if (dstTechniqueSchemeName.empty()) 
        dstTechniqueSchemeName = ShaderGenerator::DEFAULT_SCHEME_NAME;  


    //check if technique already created
    techniqueCreated = shaderGenerator->hasShaderBasedTechnique(material->getName(), 
        material->getGroup(),
        technique->getSchemeName(), 
        dstTechniqueSchemeName);
    
    if (techniqueCreated == false)
    {
        // Create the shader based technique.
        techniqueCreated = shaderGenerator->createShaderBasedTechnique(technique, 
            dstTechniqueSchemeName,
            shaderGenerator->getCreateShaderOverProgrammablePass());
    }


                    
    // Case technique successfully created.
    if (techniqueCreated)
    {
        //Attempt to get the render state which might have been created by the pass parsing
        mGeneratedRenderState = shaderGenerator->getRenderState(dstTechniqueSchemeName, 
                    material->getName(), material->getGroup(), pass->getIndex());
    
        // Go over all the render state properties.
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());
                SubRenderState* subRenderState = ShaderGenerator::getSingleton().createSubRenderState(compiler, prop, texState, this);
                
                if (subRenderState)
                {
                    addSubRenderState(subRenderState, dstTechniqueSchemeName, material->getName(), 
                        material->getGroup(), pass->getIndex());
                }
                else
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, prop->name);
                }
            }
            else
            {
                processNode(compiler, *i);
            }
        }

        mGeneratedRenderState = NULL;
    }   
}


//-----------------------------------------------------------------------------
void SGScriptTranslator::translatePass(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
    ObjectAbstractNode *obj = static_cast<ObjectAbstractNode*>(node.get());    
    Pass* pass = any_cast<Pass*>(obj->parent->context);
    Technique* technique = pass->getParent();
    Material* material = technique->getParent();
    ShaderGenerator* shaderGenerator = ShaderGenerator::getSingletonPtr();
    String dstTechniqueSchemeName = obj->name;
    bool techniqueCreated;

    // Make sure the scheme name is valid - use default if none exists.
    if (dstTechniqueSchemeName.empty()) 
        dstTechniqueSchemeName = ShaderGenerator::DEFAULT_SCHEME_NAME;  


    // Create the shader based technique.
    techniqueCreated = shaderGenerator->createShaderBasedTechnique(technique, 
        dstTechniqueSchemeName,
        shaderGenerator->getCreateShaderOverProgrammablePass());


    // Case technique successfully created.
    if (techniqueCreated)
    {
        // Go over all the render state properties.
        for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
        {
            if((*i)->type == ANT_PROPERTY)
            {
                PropertyAbstractNode *prop = static_cast<PropertyAbstractNode*>((*i).get());

                // Handle light count property.
                if (prop->name == "light_count")
                {
                    if (prop->values.size() != 3)
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    }
                    else
                    {
                        std::vector<int> lightCount;
                        if (getVector(prop->values.begin(), prop->values.end(), lightCount, 3))
                        {
                            shaderGenerator->createScheme(dstTechniqueSchemeName);
                            RenderState* renderState = shaderGenerator->getRenderState(dstTechniqueSchemeName, 
                                material->getName(), material->getGroup(), pass->getIndex());

                            renderState->setLightCount(Vector3i(lightCount.data()));
                            renderState->setLightCountAutoUpdate(false);
                        }
                        else
                        {
                            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                        }
                    }                   
                }

                // Handle the rest of the custom properties.
                else
                {
                    SubRenderState* subRenderState = ShaderGenerator::getSingleton().createSubRenderState(compiler, prop, pass, this);
                    if (subRenderState)
                    {
                        addSubRenderState(subRenderState, dstTechniqueSchemeName, material->getName(), material->getGroup(), pass->getIndex());
                    }
                    else
                    {
                        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line, prop->name);
                    }
                }               
            }
            else
            {
                processNode(compiler, *i);
            }
        }

        mGeneratedRenderState = NULL;
    }

}

//-----------------------------------------------------------------------------
void SGScriptTranslator::addSubRenderState(SubRenderState* newSubRenderState, 
        const String& dstTechniqueSchemeName, const String& materialName, const String& groupName, unsigned short passIndex)
{
    assert(newSubRenderState);

    //check if a different sub render state of the same type already exists
    ShaderGenerator* shaderGenerator = ShaderGenerator::getSingletonPtr();
    
    //create a new scheme if needed
    shaderGenerator->createScheme(dstTechniqueSchemeName);
    
    //update the active render state
    mGeneratedRenderState = shaderGenerator->getRenderState(dstTechniqueSchemeName, materialName, groupName, passIndex);

    //add the new sub render state
    mGeneratedRenderState->addTemplateSubRenderState(newSubRenderState);
}

}
}
