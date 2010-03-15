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
#ifndef _ShaderScriptTranslator_
#define _ShaderScriptTranslator_

#include "OgreShaderPrerequisites.h"
#include "OgreScriptTranslator.h"
#include "OgreScriptCompiler.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** This class responsible for translating core features of the RT Shader System for
Ogre material scripts.
*/
class _OgreRTSSExport SGScriptTranslator : public ScriptTranslator
{
public:
	/**
	*@see ScriptTranslator::translate.
	*/
	virtual void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);

	/**
	*@see ScriptTranslator::getBoolean.
	*/
	ScriptTranslator::getBoolean;
	
	/**
	*@see ScriptTranslator::getBoolean.
	*/
	ScriptTranslator::getString;
	
	/**
	*@see ScriptTranslator::getReal.
	*/
	ScriptTranslator::getReal;
	
	/**
	*@see ScriptTranslator::getFloat.
	*/
	ScriptTranslator::getFloat;

	/**
	*@see ScriptTranslator::getInt.
	*/
	ScriptTranslator::getInt; 

	/**
	*@see ScriptTranslator::getUInt.
	*/
	ScriptTranslator::getUInt; 

	/**
	*@see ScriptTranslator::getColour.
	*/
	ScriptTranslator::getColour;

	typedef std::vector<String> PropertyValues;
	typedef std::map<String, PropertyValues> Properties;
	typedef std::map<TextureUnitState*, Properties> TexturesParamCollection;
	
	TexturesParamCollection mParamCollection; // holds all properties from all texture units in the pass

	/**
	* Returns mParamCollection.
	*/	
	TexturesParamCollection getParamCollection();

	/**
	* Clears the mParamCollection.
	*/	
	void clearParamCollection();



protected:
	/**
	* Translates RT Shader System section within a pass context.
	* @param compiler The compiler invoking this translator
	* @param node The current AST node to be translated
	*/
	void translatePass(ScriptCompiler *compiler, const AbstractNodePtr &node);

	/**
	* Translates RT Shader System section within a texture_unit context.
	* @param compiler The compiler invoking this translator
	* @param node The current AST node to be translated
	*/
	void translateTextureUnit(ScriptCompiler *compiler, const AbstractNodePtr &node);

};

}
}

#endif
