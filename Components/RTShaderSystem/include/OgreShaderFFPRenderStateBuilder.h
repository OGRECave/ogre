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
#ifndef _ShaderFFPRenderStateBuilder_
#define _ShaderFFPRenderStateBuilder_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {
namespace RTShader {


/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Fixed Function Pipeline render state builder.
This class builds RenderState from a given pass that represents the fixed function pipeline
that the source pass describes.
*/
class OGRE_RTSHADERSYSTEM_INTERNAL FFPRenderStateBuilder : public Singleton<FFPRenderStateBuilder>
{
// Interface.
public:
	FFPRenderStateBuilder	();
	~FFPRenderStateBuilder	();

	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static FFPRenderStateBuilder&			getSingleton	();	

	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static FFPRenderStateBuilder*			getSingletonPtr	();


	/** 
	Initialize the FFP builder instance.
	Return true upon success.
	*/
	bool		initialize					();

	/** 
	Finalize the FFP builder instance.	
	*/
	void		finalize					();

	/** 
	Build render state from the given pass that emulates the fixed function pipeline behaviour.	
	@param sgPass The shader generator pass representation. Contains both source and destination pass.
	@param renderState The target render state that will hold the given pass FFP representation.
	*/
	void		buildRenderState			(ShaderGenerator::SGPass* sgPass, RenderState* renderState);

	
// Protected types.
protected:
	typedef std::vector<SubRenderStateFactory*> 				SubRenderStateFactoryList;
	typedef SubRenderStateFactoryList::iterator 				SubRenderStateFactoryIterator;
	typedef SubRenderStateFactoryList::const_iterator			SubRenderStateFactoryConstIterator;

// Protected methods.
protected:

	/** 
	Internal method that builds FFP sub render state.
	*/
	void		buildFFPSubRenderState			(int subRenderStateOrder, const String& subRenderStateType, 
												 ShaderGenerator::SGPass* sgPass, RenderState* renderState);
	
	/** 
	Internal method that resolves the colour stage flags.
	*/
	void		resolveColourStageFlags		(ShaderGenerator::SGPass* sgPass, RenderState* renderState);


// Attributes.
protected:
	SubRenderStateFactoryList	mFFPSubRenderStateFactoyList;		// All factories needed by the FFP.
	
};


/** @} */
/** @} */

}
}

#endif

