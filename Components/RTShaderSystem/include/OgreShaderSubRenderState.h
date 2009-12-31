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
#include "OgreMaterialSerializer.h"
#include "OgreScriptCompiler.h"
#include "OgreShaderScriptTranslator.h"


namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

typedef SharedPtr<SubRenderStateAccessor>	SubRenderStateAccessorPtr; 


/** This class is the base interface of sub part from a shader based rendering pipeline.
* All sub parts implementations should derive from it and implement the needed methods.
A simple example of sub class of this interface will be the transform sub state of the
fixed pipeline.
*/
class _OgreRTSSExport SubRenderState : public RTShaderSystemAlloc
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

	/** Called before adding this sub render state to the given render state.
	Allows this sub render state class to configure specific parameters depending on source pass or parent render state.
	Return of false value will cause canceling the add operation.
	@param renderState The target render state container this sub render state is about to be added.	
	@param srcPass The source pass.
	@param dstPass The destination pass.
	*/
	virtual bool			preAddToRenderState		(RenderState* renderState, Pass* srcPass, Pass* dstPass) { return true; }

	/** Return the accessor object to this sub render state.
	@see SubRenderStateAccessor.
	*/
	SubRenderStateAccessorPtr	getAccessor				();

	/** Return the accessor object to this sub render state.
	@see SubRenderStateAccessor.
	*/
	SubRenderStateAccessorPtr	getAccessor				() const;

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
private:	
	mutable SubRenderStateAccessorPtr		mThisAccessor;			// The accessor of this instance.
	SubRenderStateAccessorPtr		mOtherAccessor;			// The accessor of the source instance which used as base to create this instance.
	
};

typedef vector<SubRenderState*>::type 				SubRenderStateList;
typedef SubRenderStateList::iterator 				SubRenderStateListIterator;
typedef SubRenderStateList::const_iterator			SubRenderStateListConstIterator;

typedef set<SubRenderState*>::type 					SubRenderStateSet;
typedef SubRenderStateSet::iterator 				SubRenderStateSetIterator;
typedef SubRenderStateSet::const_iterator			SubRenderStateSetConstIterator;


/** This class uses as accessor from a template SubRenderState to all of its instances that
created based on it. Since SubRenderState that added as templates to a RenderState are not directly used by the
system this class enable accessing the used instances.
A common usage will be add a SubRenderState to certain pass, obtain accessor and then call a method on the instanced SubRenderState
that will trigger some GPU uniform parameter updates.
*/
class _OgreRTSSExport SubRenderStateAccessor
{
public:
	/** Add SubRenderState instance to this accessor.
	*/
	void addSubRenderStateInstance(SubRenderState* subRenderState) const 
	{
		mSubRenderStateInstancesSet.insert(subRenderState);
	}

	/** Remove SubRenderState instance to this accessor.
	*/
	void removeSubRenderStateInstance(SubRenderState* subRenderState) const
	{
		SubRenderStateSetIterator itFind = mSubRenderStateInstancesSet.find(subRenderState);

		if (itFind != mSubRenderStateInstancesSet.end())
		{
			mSubRenderStateInstancesSet.erase(itFind);
		}
	}

	/** Return a set of all instances of the template SubRenderState. */
	SubRenderStateSet&	getSubRenderStateInstasnceSet() { return mSubRenderStateInstancesSet; }

	/** Return a set of all instances of the template SubRenderState. (const version). */
	const SubRenderStateSet&	getSubRenderStateInstasnceSet() const { return mSubRenderStateInstancesSet; }

protected:
	/** Construct SubRenderState accessor based on the given template SubRenderState.
	*/
	SubRenderStateAccessor(const SubRenderState* templateSubRenderState) : mTemplateSubRenderState(templateSubRenderState) {}


protected:
	const SubRenderState*		mTemplateSubRenderState;
	mutable SubRenderStateSet	mSubRenderStateInstancesSet;

private:
	friend class SubRenderState;

};


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
class _OgreRTSSExport SubRenderStateFactory : public RTShaderSystemAlloc
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

	/** Create an instance of the SubRenderState based on script properties.	
	This method is called in the context of script parsing and let this factory
	the opportunity to create custom SubRenderState instances based on extended script properties.
	@param compiler The compiler instance.
	@param prop The abstract property node.
	@param pass The pass that is the parent context of this node.
	*/
	virtual SubRenderState*	createInstance		(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass) { return NULL; }

	/** Destroy the given instance.	
	@param subRenderState The instance to destroy.
	*/
	virtual void			destroyInstance		(SubRenderState* subRenderState);

	/** Destroy all the instances that created by this factory.		
	*/
	virtual void			destroyAllInstances	();

	/** Write the given sub-render state instance using the material serializer.
	This method is called in the context of material serialization. It is useful for integrating into
	bigger context of material exporters from various environment that want to take advantage of the RT Shader System.
	Sub classes of this interface should override in case they need to write custom properties into the script context.
	@param ser The material serializer instace.
	@param subRenderState The sub render state instance to write down.
	@param srcPass The source pass.
	@pass dstPass The generated shader based pass.
	*/
	virtual void			writeInstance		(MaterialSerializer* ser, SubRenderState* subRenderState, Pass* srcPass, Pass* dstPass) {}

protected:
	/** Create instance implementation method. Each sub class of this interface
	must implement this method in which it will allocate the specific sub class of 
	the SubRenderState.
	*/
	virtual SubRenderState*	createInstanceImpl	() = 0;

// Attributes.
protected:
	SubRenderStateSet		mSubRenderStateList;		// List of all sub render states instances this factory created.
};

/** @} */
/** @} */

}
}

#endif

