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
#ifndef _ShaderMaterialSerializerListener_
#define _ShaderMaterialSerializerListener_

#include "OgreShaderPrerequisites.h"
#include "OgreMaterialSerializer.h"
#include "OgreShaderGenerator.h"

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
class SGMaterialSerializerListener : public MaterialSerializer::Listener
{

// Interface.
public:

	/** 
	@see MaterialSerializer::Listener::materialEventRaised
	*/
	virtual void materialEventRaised(MaterialSerializer* ser, 
		MaterialSerializer::SerializeEvent event, bool& skip, const Material* mat);

	/** 
	@see MaterialSerializer::Listener::techniqueEventRaised
	*/
	virtual void techniqueEventRaised(MaterialSerializer* ser, 
		MaterialSerializer::SerializeEvent event, bool& skip, const Technique* tech);

	/** 
	@see MaterialSerializer::Listener::passEventRaised
	*/
	virtual void passEventRaised(MaterialSerializer* ser, 
		MaterialSerializer::SerializeEvent event, bool& skip, const Pass* tech);

// Types.
protected:
	typedef std::vector<ShaderGenerator::SGPass*>	SGPassList;
	typedef SGPassList::iterator					SGPassListIterator;
	typedef SGPassList::const_iterator				SGPassListConstIterator;

// Protected methods.
protected:
	/** Will be create and destroyed via ShaderGenerator interface. */ 
	SGMaterialSerializerListener();
	~SGMaterialSerializerListener();

	/** Internal method that creates list of SGPass instances composing the given material. */
	void						createSGPassList		(Material* mat, SGPassList& passList);

	/** Internal method that returns SGPass instance from a given source pass. */
	ShaderGenerator::SGPass*	getShaderGeneratedPass	(const Pass* srcPass);
	

// Attributes.
protected:	
	Material*		mSourceMaterial;		// The current source material that is being written.
	SGPassList		mSGPassList;			// List of SGPass instances composing this material.
	
	
private:
	friend class ShaderGenerator;
};

}
}

#endif