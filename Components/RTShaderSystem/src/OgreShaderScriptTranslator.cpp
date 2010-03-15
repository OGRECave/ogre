/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
#include "OgreShaderScriptTranslator.h"
#include "OgrePass.h"
#include "OgreTechnique.h"
#include "OgreMaterial.h"
#include "OgreShaderGenerator.h"


namespace Ogre {
namespace RTShader {

//-----------------------------------------------------------------------------
void SGScriptTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
	ObjectAbstractNode* obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
	ObjectAbstractNode* parent = reinterpret_cast<ObjectAbstractNode*>(obj->parent);

	// Translate section within a pass context.
	if (parent->cls == "pass")
	{
		translatePass(compiler, node);
	}
	if (parent->cls == "texture_unit")
	{
		translateTextureUnit(compiler, node);
	}
}
	
//-----------------------------------------------------------------------------
SGScriptTranslator::TexturesParamCollection SGScriptTranslator::getParamCollection()
{
	return mParamCollection;
}

//-----------------------------------------------------------------------------
void SGScriptTranslator::clearParamCollection()
{
	mParamCollection.clear();
}

//-----------------------------------------------------------------------------
/*
note: we can know the texture unit index by getting parent then finding it in the list of children
*/
void SGScriptTranslator::translateTextureUnit(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
	ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());
	TextureUnitState* textureUnitState = any_cast<TextureUnitState*>(obj->parent->context);	
	String strValue = "";
	
	Properties properties;
	for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
	{
		if((*i)->type == ANT_PROPERTY)
		{
			PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
			AbstractNodeList::const_iterator it = prop->values.begin();
			PropertyValues popertyValues;
			for( ;it != prop->values.end(); ++it)
			{
				if(true == SGScriptTranslator::getString(*it, &strValue))
				{
					popertyValues.push_back(strValue);
				}
			}
			properties[prop->name] = popertyValues;
		}
	}
	mParamCollection[textureUnitState] = properties;
}


//-----------------------------------------------------------------------------
void SGScriptTranslator::translatePass(ScriptCompiler* compiler, const AbstractNodePtr &node)
{
	ObjectAbstractNode *obj = reinterpret_cast<ObjectAbstractNode*>(node.get());	
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
	techniqueCreated = shaderGenerator->createShaderBasedTechnique(material->getName(), 
		technique->getSchemeName(), 
		dstTechniqueSchemeName);


	// Case technique successfully created.
	if (techniqueCreated)
	{
		// Go over all the render state properties.
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = reinterpret_cast<PropertyAbstractNode*>((*i).get());
				SubRenderState* subRenderState;

				// Handle light count property.
				if (prop->name == "light_count")
				{
					if (prop->values.size() != 3)
					{
						compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					else
					{
						int lightCount[3];

						if (false == SGScriptTranslator::getInts(prop->values.begin(), prop->values.end(), lightCount, 3))
						{
							compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							shaderGenerator->createScheme(dstTechniqueSchemeName);
							RenderState* renderState = shaderGenerator->getRenderState(dstTechniqueSchemeName, material->getName(), pass->getIndex());

							renderState->setLightCount(lightCount);
							renderState->setLightCountAutoUpdate(false);
						}
					}					
				}

				// Handle the rest of the custom properties.
				else
				{
					subRenderState = ShaderGenerator::getSingleton().createSubRenderState(compiler, prop, pass, this);
					if (subRenderState != NULL)
					{
						shaderGenerator->createScheme(dstTechniqueSchemeName);
						RenderState* renderState = shaderGenerator->getRenderState(dstTechniqueSchemeName, material->getName(), pass->getIndex());

						renderState->addTemplateSubRenderState(subRenderState);
					}
				}				
			}
			else
			{
				processNode(compiler, *i);
			}
		}
	}	
}

}
}
