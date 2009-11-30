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
#ifndef _ShaderProgramProcessor_
#define _ShaderProgramProcessor_

#include "OgreShaderPrerequisites.h"
#include "OgreShaderParameter.h"
#include "OgreShaderFunctionAtom.h"


namespace Ogre {
namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** A class that provides extra processing services on CPU based programs.
The base class perform only the generic processing. In order to provide target language specific services and 
optimization one should derive from this class and register its factory via the ProgramManager instance.
*/
class ProgramProcessor : public RTShaderSystemAlloc
{

// Interface.
public:	

	/** Class constructor.
	@param type The type of this program.
	*/
	ProgramProcessor			();

	/** Class destructor */
	virtual ~ProgramProcessor	();

	/** Return the target language of this processor. */
	virtual const String&		getTargetLanguage	() const = 0;
	
	/** Called before creation of the GPU programs.
	Do several preparation operation such as validation, register compaction and specific target language optimizations.
	@param programSet The program set container.
	Return true on success.
	*/
	virtual bool				preCreateGpuPrograms	(ProgramSet* programSet) = 0;

	/** Called after creation of the GPU programs.
	@param programSet The program set container.
	Return true on success.
	*/
	virtual bool				postCreateGpuPrograms	(ProgramSet* programSet) = 0;
 
// Protected types.
protected:
	
	//-----------------------------------------------------------------------------
	// Class that holds merge parameter information.
	class MergeParameter 
	{
	// Interface.
	public:
		/** Class constructor. */
		MergeParameter();


		/** Clear the state of this merge parameter. */
		void			clear						();
		
		/** Add source parameter to this merged */
		void			addSourceParameter			(ParameterPtr srcParam, int mask);

		/** Return the source parameter count. */
		size_t			getSourceParameterCount		() const { return mSrcParameterCount; }

		/** Return source parameter by index. */
		ParameterPtr	getSourceParameter			(unsigned int index) { return mSrcParameter[index]; }

		/** Return source parameter mask by index. */
		int				getSourceParameterMask		(unsigned int index) const { return mSrcParameterMask[index]; }

		/** Return destination parameter mask by index. */
		int				getDestinationParameterMask	(unsigned int index) const { return mDstParameterMask[index]; }

		/** Return the number of used floats. */ 
		int				getUsedFloatCount			();
		
		/** Return the destination parameter. */
		ParameterPtr	getDestinationParameter		(int usage, int index);

	protected:
	
		/** Creates the destination parameter by a given class and index. */
		void			createDestinationParameter	(int usage, int index);


	protected:
		ParameterPtr	mDstParameter;			// Destination merged parameter.
		ParameterPtr	mSrcParameter[4];		// Source parameters - 4 source at max 1,1,1,1 -> 4.
		int				mSrcParameterMask[4];	// Source parameters mask. OPM_ALL means all fields used, otherwise it is split source parameter.
		int				mDstParameterMask[4];	// Destination parameters mask. OPM_ALL means all fields used, otherwise it is split source parameter.
		size_t			mSrcParameterCount;		// The actual source parameters count.
		size_t			mUsedFloatCount;		// The number of used floats.
	};
	typedef vector<MergeParameter>::type	MergeParameterList;

	
	//-----------------------------------------------------------------------------
	// A struct that defines merge parameters combination.
	struct MergeCombination
	{		
		size_t			srcParamterTypeCount[4];	// The count of each source type. I.E (1 FLOAT1, 0 FLOAT2, 1 FLOAT3, 0 FLOAT4).
		int				srcParameterMask[4];		// Source parameters mask. OPM_ALL means all fields used, otherwise it is split source parameter.

		MergeCombination(
			int float1Count, int float1Mask,
			int float2Count, int float2Mask,
			int float3Count, int float3Mask,
			int float4Count, int float4Mask)
		{
			srcParamterTypeCount[0] = float1Count;
			srcParamterTypeCount[1] = float2Count;
			srcParamterTypeCount[2] = float3Count;
			srcParamterTypeCount[3] = float4Count;
			srcParameterMask[0] 	= float1Mask;
			srcParameterMask[1] 	= float2Mask;
			srcParameterMask[2] 	= float3Mask;
			srcParameterMask[3] 	= float4Mask;

		}
	};
	typedef vector<MergeCombination>::type	MergeCombinationList;

	//-----------------------------------------------------------------------------
	typedef vector<Operand*>::type						OperandPtrVector;
	typedef map<Parameter*, OperandPtrVector>::type		ParameterOperandMap;
	typedef map<Parameter*, ParameterPtr>::type			LocalParameterMap;

protected:

	/** Build parameter merging combinations. */
	void			buildMergeCombinations			();

	/** Compact the vertex shader output registers.
	@param vsMain The vertex shader entry function.
	@param fsMain The fragment shader entry function.
	Return true on success.
	*/
	virtual bool	compactVsOutputs				(Function* vsMain, Function* fsMain);

	/** Internal method that counts vertex shader texcoord output slots and output floats.
	@param vsMain The vertex shader entry function.
	@param outTexCoordSlots Will hold the number of used output texcoord slots.
	@param outTexCoordFloats Will hold the total number of floats used by output texcoord slots.
	*/
	void			countVsTexcoordOutputs			(Function* vsMain, int& outTexCoordSlots, int& outTexCoordFloats);

	/** Internal function that builds parameters table.
	@param paramList The parameter list.
	@param outParamsTable Will hold the texcoord params sorted by types in each row.
	*/
	void			buildTexcoordTable				(const ShaderParameterList& paramList, ShaderParameterList outParamsTable[4]);


	/** Merge the parameters from the given table. 
	@param paramsTable Source parameters table.
	@param mergedParams Will hold the merged parameters list.
	*/
	void			mergeParameters					(ShaderParameterList paramsTable[4], MergeParameterList& mergedParams, ShaderParameterList& splitParams);


	/** Internal function that creates merged parameter using pre defined combinations. 
	@param paramsTable Source parameters table.
	@param mergedParams The merged parameters list.
	*/
	void			mergeParametersByPredefinedCombinations(ShaderParameterList paramsTable[4], MergeParameterList& mergedParams);

	/** Internal function that creates merged parameter from given combination.
	@param combination The merge combination to try.
	@param paramsTable The params table sorted by types in each row.	
	@param mergedParameter Will hold the merged parameter.
	*/
	bool			mergeParametersByCombination			(const MergeCombination& combination, ShaderParameterList paramsTable[4], 
																 MergeParameter* mergedParameter);

	/** Merge reminders parameters that could not be merged into one slot using the predefined combinations.
	@param paramsTable The params table sorted by types in each row.	
	@param mergedParams The merged parameters list.
	@param splitParams The split parameters list.
	*/
	void			mergeParametersReminders				(ShaderParameterList paramsTable[4], MergeParameterList& mergedParams, ShaderParameterList& splitParams);


	/** Generates local parameters for the split parameters and perform packing/unpacking operation using them. */
	void			generateLocalSplitParameters				(Function* func, GpuProgramType progType, MergeParameterList& mergedParams, ShaderParameterList& splitParams, LocalParameterMap& localParamsMap);
	
	/** Rebuild the given parameters list using the merged parameters.	
	*/
	void			rebuildParameterList				(Function* func, int paramsUsage, MergeParameterList& mergedParams);

	/** Rebuild function invocations by replacing references to old source parameters with the matching merged parameters components. */
	void			rebuildFunctionInvocations			(FunctionAtomInstanceList& funcAtomList, MergeParameterList& mergedParams, LocalParameterMap& localParamsMap);

	/** Builds a map between parameter and all the references to it. */
	void			buildParameterReferenceMap			(FunctionAtomInstanceList& funcAtomList, ParameterOperandMap& paramsRefMap);

	/** Replace references to old parameters with the new merged parameters. */
	void			replaceParametersReferences			(MergeParameterList& mergedParams, ParameterOperandMap& paramsRefMap);

	/** Replace references to old parameters that have been split with the new local parameters that represents them. */
	void			replaceSplitParametersReferences	(LocalParameterMap& localParamsMap, ParameterOperandMap& paramsRefMap);

	/** Return number of floats needed by the given type. */
	static int		getParameterFloatCount				(GpuConstantType type);		

	/** Return the parameter mask of by the given parameter type (I.E: X|Y for FLOAT2 etc..) */
	static int		getParameterMaskByType				(GpuConstantType type);
	
	/** Return the parameter mask of by the float count type (I.E: X|Y for 2 etc..) */
	static int		getParameterMaskByFloatCount		(int floatCount);
	
	/** Bind the auto parameters for a given CPU and GPU program set. */
	void			bindAutoParameters					(Program* pCpuProgram, GpuProgramPtr pGpuProgram);

protected:
	MergeCombinationList	mParamMergeCombinations;		// Merging combinations defs.
	int						mMaxTexCoordSlots;				// Maximum texcoord slots.
	int						mMaxTexCoordFloats;				// Maximum texcoord floats count.
	
};


/** @} */
/** @} */

}
}

#endif

