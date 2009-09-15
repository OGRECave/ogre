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
#ifndef _ShaderSubRenderState_
#define _ShaderSubRenderState_

#include "OgreShaderPrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreSceneManager.h"


namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** This class is the base interface of sub part from a shader based rendering pipeline.
* All sub parts implementations should derive from it and implement the needed methods.
A simple example of sub class of this interface will be the transform sub state of the
fixed pipeline.
*/
class OGRE_RTSHADERSYSTEM_API SubRenderState
{

// Interface.
public:

	/** Class default constructor */
	SubRenderState			();

	/** Class destructor */
	virtual ~SubRenderState	();


	/** Get the type of this sub render state.
	@remarks
	The type string should be unique among all registered sub render states.	
	*/
	virtual const String&	getType					() const = 0;


	/** Get the execution order of this sub render state.
	@remarks
	The return value should be synchronized with the predefined values
	of the FFPShaderStage enum.
	*/
	virtual int				getExecutionOrder		() const = 0;


	/** Copy details from a given sub render state to this one.
	@param rhs the source sub state to copy from.	
	*/
	virtual void			copyFrom				(const SubRenderState& rhs) = 0;

	/** Operator = declaration. Assign the given source sub state to this sub state.
	@param rhs the source sub state to copy from.	
	*/
	SubRenderState& operator=	(const SubRenderState& rhs);

	/** Get the hash code signature of this sub render state.
	@remarks
	The return value should be unique per sub state configuration.
	I.E if a two instances of a given sub class of this interface has different properties
	each one of them should produce different hash code so the system can treat each one of them
	as different source for a sub program code.
	*/
	virtual uint32			getHashCode				();

	/** Create sub programs that represents this sub render state as part of a program set.
	The given program set contains CPU programs that represents a vertex shader and pixel shader.
	One should use these program class API to create a representation of the sub state he wished to
	implement.
	@param programSet container class of CPU and GPU programs that this sub state will affect on.
	*/
	virtual bool			createCpuSubPrograms	(ProgramSet* programSet);


	/** Update GPU programs parameters before a rendering operation occurs.
	This method is called in the context of SceneManager::renderSingle object via the RenderObjectListener interface and
	lets this sub render state instance opportunity to update custom GPU program parameters before the rendering action occurs.
	@see RenderObjectListener::notifyRenderSingleObject.
	@param rend The renderable that is about to be rendered.
	@param pass The pass that used for this rendering.
	@source The auto parameter source.
	@pLightList The light list used in the current rendering context.
	*/
	virtual void			updateGpuProgramsParams	(Renderable* rend, Pass* pass,  const AutoParamDataSource* source, 	const LightList* pLightList) { }

	

// Protected methods
protected:

	/** Resolve parameters that this sub render state requires.	
	@param programSet container class of CPU and GPU programs that this sub state will affect on.
	@remarks Internal method called in the context of SubRenderState::createCpuSubPrograms implementation.
	*/
	virtual bool			resolveParameters		(ProgramSet* programSet);	

	/** Resolve dependencies that this sub render state requires.	
	@param programSet container class of CPU and GPU programs that this sub state will affect on.
	@remarks Internal method called in the context of SubRenderState::createCpuSubPrograms implementation.
	*/
	virtual bool			resolveDependencies		(ProgramSet* programSet);

	/** Add function invocation that this sub render state requires.	
	@param programSet container class of CPU and GPU programs that this sub state will affect on.
	@remarks Internal method called in the context of SubRenderState::createCpuSubPrograms implementation.
	*/
	virtual bool			addFunctionInvocations	(ProgramSet* programSet);


// Attributes.
protected:
	
};

typedef std::vector<SubRenderState*> 				SubRenderStateList;
typedef SubRenderStateList::iterator 				SubRenderStateIterator;
typedef SubRenderStateList::const_iterator			SubRenderStateConstIterator;


/** Abstract factory interface for creating SubRenderState implementation instances.
@remarks
Plugins or 3rd party applications can add new types of sub render states to the 
RTShader System by creating subclasses of the SubRenderState class. 
Because multiple instances of these sub class may be required, 
a factory class to manage the instances is also required. 
@par
SubRenderStateFactory subclasses must allow the creation and destruction of SubRenderState
subclasses. They must also be registered with the ShaderGenerator::addSubRenderStateFactory. 
All factories have a type which identifies them and the sub class of SubRenderState they creates.
*/
class SubRenderStateFactory
{

public:
	SubRenderStateFactory			() {}
	virtual ~SubRenderStateFactory	();

	/** Get the type of this sub render state factory.
	@remarks
	The type string should be the same as the type of the SubRenderState sub class it is going to create.	
	@see SubRenderState::getType.
	*/
	virtual const String&	getType				() const = 0;
	
	/** Create an instance of the SubRenderState sub class it suppose to create.	
	*/
	virtual SubRenderState*	createInstance		();

	/** Destroy the given instance.	
	@param subRenderState The instance to destroy.
	*/
	virtual void			destroyInstance		(SubRenderState* subRenderState);

	/** Destroy all the instances that created by this factory.		
	*/
	virtual void			destroyAllInstances	();

protected:
	/** Create instance implementation method. Each sub class of this interface
	must implement this method in which it will allocate the specific sub class of 
	the SubRenderState.
	*/
	virtual SubRenderState*	createInstanceImpl	() = 0;

// Attributes.
protected:
	SubRenderStateList		mSubRenderStateList;		// List of all sub render states instances this factory created.
};

/** @} */
/** @} */

}
}

#endif

