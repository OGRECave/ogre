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
It has unique hash value based on it's sub state and this value maps to unique GPU programs.
The user should use this interface to define global or per material custom behavior.
I.E In order to apply per pixel to a specific material one should implement a sub class of SubRenderState that
perform a per pixel lighting model, get the render state of the target material and add the custom sub class to it.
*/
class OGRE_RTSHADERSYSTEM_API RenderState
{

// Interface.
public:
	
	/** Class default constructor. */
	RenderState();

	/** Class destructor */
	~RenderState();

	/** Reset this render state */
	void		reset						();

	/** Add sub render state to this render state.
	@param subRenderState The sub render state to add.
	*/
	void		addSubRenderState			(SubRenderState* subRenderState);

	/** Remove sub render state from this render state.
	@param subRenderState The sub render state to remove.
	*/
	void		removeSubRenderState			(SubRenderState* subRenderState);

	/** Merge the given render state into this render state.
	Only sub render states with execution order that don't exist in this render state will be added.	
	@param other The other render state to append to this state.
	@param srcPass The source pass that this render state is constructed from.
	@param dstPass The destination pass that constructed from this render state.
	*/
	void		merge						(const RenderState& other, Pass* srcPass, Pass* dstPass);
	
	/** Copy the given render state to this render state.
	@param rhs The other render state to copy to this state.
	*/
	void		copyFrom					(const RenderState& rhs);	

	/** Operator = interface. Copy the given render state to this render state.
	@param rhs The other render state to copy to this state.
	*/
	RenderState& operator=					(const RenderState& rhs);


	/** Get the hash code of this render state. */
	uint32			getHashCode				();

	/** Create CPU programs that represent this render state. 
	@param programSet The container of the CPU programs.
	*/
	bool			createCpuPrograms		(ProgramSet* programSet);

	/** Update the GPU programs constant parameters before a renderable is rendered.
	@param rend The renderable object that is going to be rendered.
	@param pass The pass that is used to do the rendering operation.
	@param source The auto parameter auto source instance.
	@param pLightList The light list used for the current rendering operation.
	*/
	void			updateGpuProgramsParams	(Renderable* rend, Pass* pass, const AutoParamDataSource* source, const LightList* pLightList);
	
	/** Get the list of the sub render states composing this render state. */
	const SubRenderStateList&	getSubStateList() const { return mSubRenderStateList; }

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
	
// Protected methods
protected:

	/** Sort the sub render states composing this render state. */
	void			sortSubRenderStates		();

	/** Comparison function of the sub render states. */
	static int		sSubRenderStateCompare	(const void * p0, const void *p1);


// Attributes.
protected:
	SubRenderStateList	mSubRenderStateList;			// The sub render states list.
	bool				mSubRenderStateSortValid;		// Tells if the list of the sub render states is sorted.
	uint32				mHashCode;						// The hash of this render states.
	bool				mHashCodeValid;					// Tells if the hash code is valid or has to be computed again.
	int					mLightCount[3];					// The light count per light type definition.
	bool				mLightCountAutoUpdate;			// True if light count was explicitly set.
		
};

typedef SharedPtr<RenderState>					RenderStatePtr;
typedef std::vector<RenderState*> 				RenderStateList;
typedef RenderStateList::iterator 				RenderStateIterator;
typedef RenderStateList::const_iterator			RenderStateConstIterator;

/** @} */
/** @} */

}
}

#endif

