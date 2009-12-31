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
#ifndef _ShaderRenderState_
#define _ShaderRenderState_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreSharedPtr.h"

namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** This is a container class for sub render state class.
A render state is defined by the sub render states that compose it.
The user should use this interface to define global or per material custom behavior.
I.E In order to apply per pixel to a specific material one should implement a sub class of SubRenderState that
perform a per pixel lighting model, get the render state of the target material and add the custom sub class to it.
*/
class _OgreRTSSExport RenderState : public RTShaderSystemAlloc
{

	// Interface.
public:

	/** Class default constructor. */
	RenderState();

	/** Class destructor */
	virtual ~RenderState();

	/** Reset this render state */
	void		reset						();

	/** Add a template sub render state to this render state.
	@param subRenderState The sub render state template to add.
	Return sub render state access
	*/
	void		addTemplateSubRenderState	(SubRenderState* subRenderState);

	/** Remove a template sub render state from this render state.
	@param subRenderState The sub render state to remove.
	*/
	void		removeTemplateSubRenderState (SubRenderState* subRenderState);

	/** 
	Set the light count per light type.
	@param 
	lightCount The light count per type.
	lightCount[0] defines the point light count.
	lightCount[1] defines the directional light count.
	lightCount[2] defines the spot light count.
	*/
	void			setLightCount			(const int lightCount[3]);

	/** 
	Get the light count per light type.
	@param 
	lightCount The light count per type.
	lightCount[0] defines the point light count.
	lightCount[1] defines the directional light count.
	lightCount[2] defines the spot light count.
	*/
	void			getLightCount			(int lightCount[3]) const;

	/** 
	Set the light count auto update state.
	If the value is false the light count will remain static for the values that were set by the user.
	If the value is true the light count will be updated from the owner shader generator scheme based on current scene lights.
	The default is true.
	*/
	void			setLightCountAutoUpdate	(bool autoUpdate) { mLightCountAutoUpdate = autoUpdate; }

	/** 
	Return true if this render state override the light count. 
	If light count is not overridden it will be updated from the shader generator based on current scene lights.
	*/
	bool			getLightCountAutoUpdate	() const { return mLightCountAutoUpdate; }

	/** Get the list of the sub render states composing this render state. */
	const SubRenderStateList&	getSubStateList() const { return mSubRenderStateList; }


	// Protected methods
protected:
	

	// Attributes.
protected:
	SubRenderStateList	mSubRenderStateList;			// The sub render states list.	
	int					mLightCount[3];					// The light count per light type definition.
	bool				mLightCountAutoUpdate;			// True if light count was explicitly set.

private:
	friend class ProgramManager;
	friend class FFPRenderStateBuilder;
};


typedef vector<RenderState*>::type 				RenderStateList;
typedef RenderStateList::iterator 				RenderStateIterator;
typedef RenderStateList::const_iterator			RenderStateConstIterator;


/** This is the target render state. This class will hold the actual generated CPU/GPU programs.
It will be initially build from the FFP state of a given Pass by the FFP builder and then will be linked
with the custom pass render state and the global scheme render state. See ShaderGenerator::SGPass::buildTargetRenderState().
*/
class TargetRenderState : public RenderState
{

// Interface.
public:
	
	/** Class default constructor. */
	TargetRenderState();

	/** Class destructor */
	virtual ~TargetRenderState();

	/** Link this target render state with the given render state.
	Only sub render states with execution order that don't exist in this render state will be added.	
	@param other The other render state to append to this state.
	@param srcPass The source pass that this render state is constructed from.
	@param dstPass The destination pass that constructed from this render state.
	*/
	void		link				(const RenderState& other, Pass* srcPass, Pass* dstPass);

	/** Update the GPU programs constant parameters before a renderable is rendered.
	@param rend The renderable object that is going to be rendered.
	@param pass The pass that is used to do the rendering operation.
	@param source The auto parameter auto source instance.
	@param pLightList The light list used for the current rendering operation.
	*/
	void updateGpuProgramsParams	(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);
	
// Protected methods
protected:

	/** Sort the sub render states composing this render state. */
	void			sortSubRenderStates		();

	/** Comparison function of the sub render states. */
	static int		sSubRenderStateCompare	(const void * p0, const void *p1);

	
	/** Create CPU programs that represent this render state. 	
	*/
	bool		createCpuPrograms		();

	/** Create the program set of this render state.
	*/
	ProgramSet*	createProgramSet			();

	/* Destroy the program set of this render state. */
	void		destroyProgramSet			();

	/** Return the program set of this render state.
	*/
	ProgramSet*	getProgramSet				() { return mProgramSet; }

	/** Add sub render state to this render state.
	@param subRenderState The sub render state to add.
	*/
	void		addSubRenderState			(SubRenderState* subRenderState);

	/** Remove sub render state from this render state.
	@param subRenderState The sub render state to remove.
	*/
	void		removeSubRenderState			(SubRenderState* subRenderState);

	
// Attributes.
protected:
	bool				mSubRenderStateSortValid;		// Tells if the list of the sub render states is sorted.
	ProgramSet*			mProgramSet;					// The program set of this RenderState.
	

private:
	friend class ProgramManager;
	friend class FFPRenderStateBuilder;
};


/** @} */
/** @} */

}
}

#endif

