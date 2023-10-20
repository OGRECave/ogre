/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
ProgramProcessor::ProgramProcessor()
{
    mMaxTexCoordSlots = 16;
    mMaxTexCoordFloats = mMaxTexCoordSlots * 4;

}

//-----------------------------------------------------------------------------
ProgramProcessor::~ProgramProcessor()
{

}

//-----------------------------------------------------------------------------
bool ProgramProcessor::preCreateGpuPrograms(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuProgram(GPT_VERTEX_PROGRAM);
    Program* fsProgram = programSet->getCpuProgram(GPT_FRAGMENT_PROGRAM);
    Function* vsMain   = vsProgram->getEntryPointFunction();
    Function* fsMain   = fsProgram->getEntryPointFunction();

    // Compact vertex shader outputs.
    return compactVsOutputs(vsMain, fsMain);
}


bool ProgramProcessor::postCreateGpuPrograms(ProgramSet* programSet)
{
    // Bind vertex auto parameters.
    for(auto type : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
        bindAutoParameters(programSet->getCpuProgram(type), programSet->getGpuProgram(type));

    if(ShaderGenerator::getSingleton().getTargetLanguage().find("glsl") == String::npos)
        return true;

    for(auto type : {GPT_VERTEX_PROGRAM, GPT_FRAGMENT_PROGRAM})
        bindTextureSamplers(programSet->getCpuProgram(type), programSet->getGpuProgram(type));

    return true;
}

//-----------------------------------------------------------------------------
void ProgramProcessor::bindAutoParameters(Program* pCpuProgram, GpuProgramPtr pGpuProgram)
{
    GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();

    for (const auto& p : pCpuProgram->getParameters())
    {
        const GpuConstantDefinition* gpuConstDef = pGpuParams->_findNamedConstantDefinition(p->getName());

        if (gpuConstDef != NULL)
        {
            // Handle auto parameters.
            if (p->isAutoConstantParameter())
            {
                if (p->isAutoConstantRealParameter())
                {
                    pGpuParams->setNamedAutoConstantReal(p->getName(),
                        p->getAutoConstantType(),
                        p->getAutoConstantRealData());

                }
                else if (p->isAutoConstantIntParameter())
                {
                    pGpuParams->setNamedAutoConstant(p->getName(),
                        p->getAutoConstantType(),
                        p->getAutoConstantIntData());
                }
            }

            // Case this is not auto constant - we have to update its variability ourself.
            else
            {
                gpuConstDef->variability |= p->getVariability();

                // Update variability in the float map.
                if (gpuConstDef->isSampler() == false)
                {
                    GpuLogicalBufferStructPtr floatLogical = pGpuParams->getLogicalBufferStruct();
                    if (floatLogical.get())
                    {
                        for (GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin(); i != floatLogical->map.end(); ++i)
                        {
                            if (i->second.physicalIndex == gpuConstDef->physicalIndex)
                            {
                                i->second.variability |= gpuConstDef->variability;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ProgramProcessor::bindTextureSamplers(Program* pCpuProgram, GpuProgramPtr pGpuProgram)
{
    if (StringConverter::parseBool(pGpuProgram->getParameter("has_sampler_binding")))
        return;

    GpuProgramParametersSharedPtr pGpuParams = pGpuProgram->getDefaultParameters();

    // Bind the samplers.
    for (const auto& pCurParam : pCpuProgram->getParameters())
    {
        if (pCurParam->isSampler() && pCurParam->isUsed())
        {
            // The optimizer may remove some unnecessary parameters, so we should ignore them
            pGpuParams->setIgnoreMissingParams(true);
            pGpuParams->setNamedConstant(pCurParam->getName(), pCurParam->getIndex());
        }
    }
}

//-----------------------------------------------------------------------------
bool ProgramProcessor::compactVsOutputs(Function* vsMain, Function* fsMain)
{

    int outTexCoordSlots;
    int outTexCoordFloats;

    // Count vertex shader texcoords outputs.
    countVsTexcoordOutputs(vsMain, outTexCoordSlots, outTexCoordFloats);

    // Case the total number of used floats is bigger than maximum - nothing we can do.
    if (outTexCoordFloats > mMaxTexCoordFloats)
        return false;

    // Only one slot used -> nothing to compact.
    if (outTexCoordSlots <= 1)
        return true;

    // Case compact policy is low and output slots are enough -> quit compacting process.
    if (ShaderGenerator::getSingleton().getVertexShaderOutputsCompactPolicy() == VSOCP_LOW && outTexCoordSlots <= mMaxTexCoordSlots)
        return true;

    // Build output parameter tables - each row represents different parameter type (GCT_FLOAT1-4).
    ShaderParameterList vsOutParamsTable[4];
    ShaderParameterList fsInParamsTable[4];

    buildTexcoordTable(vsMain->getOutputParameters(), vsOutParamsTable);
    buildTexcoordTable(fsMain->getInputParameters(), fsInParamsTable);


    // Create merge parameters entries using the predefined merge combinations.
    MergeParameterList vsMergedParamsList;
    MergeParameterList fsMergedParamsList;
    ShaderParameterList vsSplitParams;
    ShaderParameterList fsSplitParams;
    bool hasMergedParameters = false;

    mergeParameters(vsOutParamsTable, vsMergedParamsList, vsSplitParams);


    // Check if any parameter has been merged - means at least two parameters takes the same slot.
    for (auto & i : vsMergedParamsList)
    {
        if (i.getSourceParameterCount() > 1)
        {
            hasMergedParameters = true;
            break;
        }
    }

    // Case no parameter has been merged -> quit compacting process.
    if (hasMergedParameters == false)
        return true;

    mergeParameters(fsInParamsTable, fsMergedParamsList, fsSplitParams);

    // Generate local params for split source parameters.
    LocalParameterMap vsLocalParamsMap;
    LocalParameterMap fsLocalParamsMap;

    generateLocalSplitParameters(vsMain, GPT_VERTEX_PROGRAM, vsMergedParamsList, vsSplitParams, vsLocalParamsMap);
    generateLocalSplitParameters(fsMain, GPT_FRAGMENT_PROGRAM, fsMergedParamsList, fsSplitParams, fsLocalParamsMap);


    // Rebuild functions parameter lists.
    rebuildParameterList(vsMain, Operand::OPS_OUT, vsMergedParamsList);
    rebuildParameterList(fsMain, Operand::OPS_IN, fsMergedParamsList);

    // Adjust function invocations operands to reference the new merged parameters.
    rebuildFunctionInvocations(vsMain->getAtomInstances(), vsMergedParamsList, vsLocalParamsMap);
    rebuildFunctionInvocations(fsMain->getAtomInstances(), fsMergedParamsList, fsLocalParamsMap);


    return true;
}

//-----------------------------------------------------------------------------
void ProgramProcessor::countVsTexcoordOutputs(Function* vsMain,
                                              int& outTexCoordSlots,
                                              int& outTexCoordFloats)
{
    outTexCoordSlots = 0;
    outTexCoordFloats = 0;

    // Grab vertex shader output information.
    for (const auto& p : vsMain->getOutputParameters())
    {
        if (p->getSemantic() == Parameter::SPS_TEXTURE_COORDINATES)
        {
            outTexCoordSlots++;
            outTexCoordFloats += getParameterFloatCount(p->getType());
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::buildTexcoordTable(const ShaderParameterList& paramList, ShaderParameterList outParamsTable[4])
{
    for (const auto& p : paramList)
    {
        if (p->getSemantic() == Parameter::SPS_TEXTURE_COORDINATES)
        {

            switch (p->getType())
            {
            case GCT_FLOAT1:
                outParamsTable[0].push_back(p);
                break;

            case GCT_FLOAT2:
                outParamsTable[1].push_back(p);
                break;

            case GCT_FLOAT3:
                outParamsTable[2].push_back(p);
                break;

            case GCT_FLOAT4:
                outParamsTable[3].push_back(p);
                break;
            case GCT_SAMPLER1D:
            case GCT_SAMPLER2D:
            case GCT_SAMPLER2DARRAY:
            case GCT_SAMPLER3D:
            case GCT_SAMPLERCUBE:
            case GCT_SAMPLER1DSHADOW:
            case GCT_SAMPLER2DSHADOW:
            case GCT_MATRIX_2X2:
            case GCT_MATRIX_2X3:
            case GCT_MATRIX_2X4:
            case GCT_MATRIX_3X2:
            case GCT_MATRIX_3X3:
            case GCT_MATRIX_3X4:
            case GCT_MATRIX_4X2:
            case GCT_MATRIX_4X3:
            case GCT_MATRIX_4X4:
            case GCT_INT1:
            case GCT_INT2:
            case GCT_INT3:
            case GCT_INT4:
            case GCT_UINT1:
            case GCT_UINT2:
            case GCT_UINT3:
            case GCT_UINT4:
            case GCT_UNKNOWN:
            default:
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::mergeParameters(ShaderParameterList paramsTable[4], MergeParameterList& mergedParams,
                                      ShaderParameterList& splitParams)
{
    // Merge using the predefined combinations.
    mergeParametersByPredefinedCombinations(paramsTable, mergedParams);

    // Merge the reminders parameters if such left.
    if (paramsTable[0].size() + paramsTable[1].size() +
        paramsTable[2].size() + paramsTable[3].size() > 0)
    {
        mergeParametersReminders(paramsTable, mergedParams, splitParams);
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::mergeParametersByPredefinedCombinations(ShaderParameterList paramsTable[4],
                                                               MergeParameterList& mergedParams)
{

    // Make sure the merge combinations are ready.
    if (mParamMergeCombinations.empty())
    {
        buildMergeCombinations();
    }

    // Create the full used merged params - means FLOAT4 params that all of their components are used.
    for (auto & curCombination : mParamMergeCombinations)
    {
        // Case all parameters have been merged.
        if (paramsTable[0].empty() && paramsTable[1].empty() &&
            paramsTable[2].empty() && paramsTable[3].empty())
            return;

        MergeParameter curMergeParam;

        while (mergeParametersByCombination(curCombination, paramsTable, &curMergeParam))
        {
            mergedParams.push_back(curMergeParam);
            curMergeParam.clear();
        }
    }

    // Case low/medium compacting policy -> use these simplified combinations in order to prevent splits.
    if (ShaderGenerator::getSingleton().getVertexShaderOutputsCompactPolicy() == VSOCP_LOW ||
        ShaderGenerator::getSingleton().getVertexShaderOutputsCompactPolicy() == VSOCP_MEDIUM)
    {
        const int curUsedSlots = static_cast<int>(mergedParams.size());
        const int float1ParamCount = static_cast<int>(paramsTable[0].size());
        const int float2ParamCount = static_cast<int>(paramsTable[1].size());
        const int float3ParamCount = static_cast<int>(paramsTable[2].size());
        int       reqSlots = 0;

        // Compute the required slots.

        // Add all float3 since each one of them require one slot for himself.
        reqSlots += float3ParamCount;

        // Add the float2 count -> at max it will be 1 since all pairs have been merged previously.
        if (float2ParamCount > 1)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Invalid float2 reminder count.",
                "ProgramProcessor::mergeParametersByPredefinedCombinations");
        }

        reqSlots += float2ParamCount;

        // Compute how much space needed for the float1(s) that left -> at max it will be 3.
        if (float1ParamCount > 0)
        {
            if (float2ParamCount > 3)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                    "Invalid float1 reminder count.",
                    "ProgramProcessor::mergeParametersByPredefinedCombinations");
            }

            // No float2 -> we need one more slot for these float1(s).
            if (float2ParamCount == 0)
            {
                reqSlots += 1;
            }
            else
            {
                // Float2 exists -> there must be at max 1 float1.
                if (float1ParamCount > 1)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Invalid float1 reminder count.",
                        "ProgramProcessor::mergeParametersByPredefinedCombinations");
                }
            }
        }

        // Case maximum slot count will be exceeded -> fall back to full compaction.
        if (curUsedSlots + reqSlots > mMaxTexCoordSlots)
            return;

        MergeCombination simpleCombinations[6] =
        {
            // Deal with the float3 parameters.
            MergeCombination(
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            1, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

            // Deal with float2 + float1 combination.
            MergeCombination(
            1, Operand::OPM_ALL,
            1, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

            // Deal with the float2 parameter.
            MergeCombination(
            0, Operand::OPM_ALL,
            1, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

            // Deal with the 3 float1 combination.
            MergeCombination(
            3, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

            // Deal with the 2 float1 combination.
            MergeCombination(
            2, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

            // Deal with the 1 float1 combination.
            MergeCombination(
            1, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL,
            0, Operand::OPM_ALL),

        };

        for (const auto & curCombination : simpleCombinations)
        {
            // Case all parameters have been merged.
            if (paramsTable[0].size() + paramsTable[1].size() + paramsTable[2].size() + paramsTable[3].empty())
                break;

            MergeParameter curMergeParam;

            while (mergeParametersByCombination(curCombination, paramsTable, &curMergeParam))
            {
                mergedParams.push_back(curMergeParam);
                curMergeParam.clear();
            }
        }
    }
}

//-----------------------------------------------------------------------------
bool ProgramProcessor::mergeParametersByCombination(const MergeCombination& combination,
                                    ShaderParameterList paramsTable[4],
                                    MergeParameter* mergedParameter)
{
    // Make sure we have enough parameters to combine.
    if (combination.srcParameterTypeCount[0] > paramsTable[0].size() ||
        combination.srcParameterTypeCount[1] > paramsTable[1].size() ||
        combination.srcParameterTypeCount[2] > paramsTable[2].size() ||
        combination.srcParameterTypeCount[3] > paramsTable[3].size())
    {
        return false;
    }

    // Create the new output parameter.
    for (int i=0; i < 4; ++i)
    {
        ShaderParameterList& curParamList = paramsTable[i];
        int srcParameterTypeCount = static_cast<int>(combination.srcParameterTypeCount[i]);
        int srcParameterCount = 0;

        while (srcParameterTypeCount > 0)
        {
            mergedParameter->addSourceParameter(curParamList.back(), combination.srcParameterMask[srcParameterCount]);
            curParamList.pop_back();
            srcParameterCount++;
            --srcParameterTypeCount;
        }
    }


    return true;
}

//-----------------------------------------------------------------------------
void ProgramProcessor::mergeParametersReminders(ShaderParameterList paramsTable[4], MergeParameterList& mergedParams, ShaderParameterList& splitParams)
{

    // Handle reminders parameters - All of the parameters that could not packed perfectly.
    const size_t mergedParamsBaseIndex      = mergedParams.size();
    const size_t remindersFloatCount        = (1 * paramsTable[0].size()) + (2 * paramsTable[1].size()) + (3 * paramsTable[2].size()) + (4 * paramsTable[3].size());
    const size_t remindersFloatMod          = remindersFloatCount % 4;
    const size_t remindersFullSlotCount     = remindersFloatCount / 4;
    const size_t remindersPartialSlotCount  = remindersFloatMod > 0 ? 1 : 0;
    const size_t remindersTotalSlotCount    = remindersFullSlotCount + remindersPartialSlotCount;

    // First pass -> fill the slots with the largest reminders parameters.
    for (unsigned int slot=0; slot < remindersTotalSlotCount; ++slot)
    {
        MergeParameter curMergeParam;

        for (unsigned int row=0; row < 4; ++row)
        {
            ShaderParameterList& curParamList = paramsTable[3 - row];

            // Case this list contains parameters -> pop it out and add to merged params.
            if (curParamList.size() > 0)
            {
                curMergeParam.addSourceParameter(curParamList.back(), Operand::OPM_ALL);
                curParamList.pop_back();
                mergedParams.push_back(curMergeParam);
                break;
            }
        }
    }

    // Second pass -> merge the reminders parameters.
    for (unsigned int row=0; row < 4; ++row)
    {
        ShaderParameterList& curParamList = paramsTable[3 - row];

        // Merge the all the parameters of the current list.
        while (curParamList.size() > 0)
        {
            ParameterPtr srcParameter  = curParamList.back();
            int splitCount             = 0;     // How many times the source parameter has been split.
            int srcParameterComponents;
            int srcParameterFloats;
            int curSrcParameterFloats;

            srcParameterFloats = getParameterFloatCount(srcParameter->getType());
            curSrcParameterFloats = srcParameterFloats;
            srcParameterComponents = getParameterMaskByType(srcParameter->getType());


            // While this parameter has remaining components -> split it.
            while (curSrcParameterFloats > 0)
            {
                for (unsigned int slot=0; slot < remindersTotalSlotCount && curSrcParameterFloats > 0; ++slot)
                {
                    MergeParameter& curMergeParam = mergedParams[mergedParamsBaseIndex + slot];
                    const int freeFloatCount = 4 - curMergeParam.getUsedFloatCount();

                    // Case this slot has free space.
                    if (freeFloatCount > 0)
                    {
                        // Case current components of source parameter can go all into this slot without split.
                        if (srcParameterFloats < freeFloatCount && splitCount == 0)
                        {
                            curMergeParam.addSourceParameter(srcParameter, Operand::OPM_ALL);
                        }

                        // Case we have to split the current source parameter.
                        else
                        {
                            int srcComponentsMask;

                            // Create the mask that tell us which part of the source component is added to the merged parameter.
                            srcComponentsMask = getParameterMaskByFloatCount(freeFloatCount) << splitCount;

                            // Add the partial source parameter to merged parameter.
                            curMergeParam.addSourceParameter(srcParameter, Operand::OpMask(srcComponentsMask & srcParameterComponents));
                        }
                        splitCount++;

                        // Update left floats count.
                        if (srcParameterFloats < freeFloatCount)
                        {
                            curSrcParameterFloats -= srcParameterFloats;
                        }
                        else
                        {
                            curSrcParameterFloats -= freeFloatCount;
                        }
                    }
                }
            }

            // Add to split params list.
            if (splitCount > 1)
                splitParams.push_back(srcParameter);


            curParamList.pop_back();
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::rebuildParameterList(Function* func, int paramsUsage, MergeParameterList& mergedParams)
{
    // Delete the old merged parameters.
    for (auto & curMergeParameter : mergedParams)
    {
        for (unsigned int j=0; j < curMergeParameter.getSourceParameterCount(); ++j)
        {
            ParameterPtr curSrcParam = curMergeParameter.getSourceParameter(j);

            if (paramsUsage == Operand::OPS_OUT)
            {
                func->deleteOutputParameter(curSrcParam);
            }
            else if (paramsUsage == Operand::OPS_IN)
            {
                func->deleteInputParameter(curSrcParam);
            }
        }
    }

    // Add the new combined parameters.
    for (unsigned int i=0; i < mergedParams.size(); ++i)
    {
        MergeParameter& curMergeParameter = mergedParams[i];

        if (paramsUsage == Operand::OPS_OUT)
        {
            func->addOutputParameter(curMergeParameter.getDestinationParameter(paramsUsage, i));
        }
        else if (paramsUsage == Operand::OPS_IN)
        {
            func->addInputParameter(curMergeParameter.getDestinationParameter(paramsUsage, i));
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::generateLocalSplitParameters(Function* func, GpuProgramType progType,
                                                   MergeParameterList& mergedParams,
                                                   ShaderParameterList& splitParams, LocalParameterMap& localParamsMap)
{
    // No split params created.
    if (splitParams.empty())
        return;

    // Create the local parameters + map from source to local.
    for (const auto& srcParameter : splitParams)
    {
        ParameterPtr localParameter = func->resolveLocalParameter(srcParameter->getType(), "lsplit_" + srcParameter->getName());

        localParamsMap[srcParameter.get()] = localParameter;
    }

    // Establish link between the local parameter to the merged parameter.
    for (unsigned int i=0; i < mergedParams.size(); ++i)
    {
        MergeParameter& curMergeParameter = mergedParams[i];

        for (unsigned int p=0; p < curMergeParameter.getSourceParameterCount(); ++p)
        {
            ParameterPtr srcMergedParameter    = curMergeParameter.getSourceParameter(p);
            LocalParameterMap::iterator itFind = localParamsMap.find(srcMergedParameter.get());

            // Case the source parameter is split parameter.
            if (itFind != localParamsMap.end())
            {
                // Case it is the vertex shader -> assign the local parameter to the output merged parameter.
                if (progType == GPT_VERTEX_PROGRAM)
                {
                    FunctionAtom* curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_VS_POST_PROCESS);

                    curFuncInvocation->pushOperand(itFind->second, Operand::OPS_IN, curMergeParameter.getSourceParameterMask(p));
                    curFuncInvocation->pushOperand(curMergeParameter.getDestinationParameter(Operand::OPS_OUT, i), Operand::OPS_OUT, curMergeParameter.getDestinationParameterMask(p));
                    func->addAtomInstance(curFuncInvocation);
                }
                else if (progType == GPT_FRAGMENT_PROGRAM)
                {
                    FunctionAtom* curFuncInvocation = OGRE_NEW AssignmentAtom(FFP_PS_PRE_PROCESS);

                    curFuncInvocation->pushOperand(curMergeParameter.getDestinationParameter(Operand::OPS_IN, i), Operand::OPS_IN, curMergeParameter.getDestinationParameterMask(p));
                    curFuncInvocation->pushOperand(itFind->second, Operand::OPS_OUT, curMergeParameter.getSourceParameterMask(p));
                    func->addAtomInstance(curFuncInvocation);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::rebuildFunctionInvocations(const FunctionAtomInstanceList& funcAtomList,
                                                  MergeParameterList& mergedParams,
                                                  LocalParameterMap& localParamsMap)
{
    ParameterOperandMap paramsRefMap;

    // Build reference map of source parameters.
    buildParameterReferenceMap(funcAtomList, paramsRefMap);

    // Replace references to old parameters with references to new parameters.
    replaceParametersReferences(mergedParams, paramsRefMap);


    // Replace references to old parameters with references to new split parameters.
    replaceSplitParametersReferences(localParamsMap, paramsRefMap);

}

//-----------------------------------------------------------------------------
void ProgramProcessor::buildParameterReferenceMap(const FunctionAtomInstanceList& funcAtomList, ParameterOperandMap& paramsRefMap)
{
    for (const auto& func : funcAtomList)
    {
        for (Operand& curOperand : func->getOperandList())
        {
            paramsRefMap[curOperand.getParameter().get()].push_back(&curOperand);
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::replaceParametersReferences(MergeParameterList& mergedParams, ParameterOperandMap& paramsRefMap)
{
    for (unsigned int i=0; i < mergedParams.size(); ++i)
    {
        MergeParameter& curMergeParameter = mergedParams[i];
        int paramBitMaskOffset = 0;

        for (unsigned int j=0; j < curMergeParameter.getSourceParameterCount(); ++j)
        {
            ParameterPtr      curSrcParam  = curMergeParameter.getSourceParameter(j);
            ParameterOperandMap::iterator itParamRefs = paramsRefMap.find(curSrcParam.get());

            // Case the source parameter has some references
            if (itParamRefs != paramsRefMap.end())
            {
                OperandPtrVector& srcParamRefs = itParamRefs->second;
                ParameterPtr dstParameter;

                // Case the source parameter is fully contained within the destination merged parameter.
                if (curMergeParameter.getSourceParameterMask(j) == Operand::OPM_ALL)
                {
                    dstParameter = curMergeParameter.getDestinationParameter(Operand::OPS_INOUT, i);

                    for (auto srcOperandPtr : srcParamRefs)
                    {
                        int       dstOpMask;

                        if (srcOperandPtr->getMask() == Operand::OPM_ALL)
                        {
                            // Case the merged parameter contains only one source - no point in adding special mask.
                            if (curMergeParameter.getSourceParameterCount() == 1)
                            {
                                dstOpMask = Operand::OPM_ALL;
                            }
                            else
                            {
                                dstOpMask = getParameterMaskByType(curSrcParam->getType()) << paramBitMaskOffset;
                            }
                        }
                        else
                        {
                            dstOpMask = srcOperandPtr->getMask() << paramBitMaskOffset;
                        }

                        // Replace the original source operand with a new operand the reference the new merged parameter.
                        *srcOperandPtr = Operand(dstParameter, srcOperandPtr->getSemantic(), Operand::OpMask(dstOpMask));
                    }
                }
            }


            // Update the bit mask offset.
            paramBitMaskOffset += getParameterFloatCount(curSrcParam->getType());
        }
    }
}

//-----------------------------------------------------------------------------
void ProgramProcessor::replaceSplitParametersReferences(LocalParameterMap& localParamsMap, ParameterOperandMap& paramsRefMap)
{
    for (const auto& p : localParamsMap)
    {
        Parameter* curSrcParam = p.first;
        ParameterOperandMap::iterator itParamRefs = paramsRefMap.find(curSrcParam);

        if (itParamRefs != paramsRefMap.end())
        {
            ParameterPtr dstParameter      = p.second;
            OperandPtrVector& srcParamRefs = itParamRefs->second;

            for (auto srcOperandPtr : srcParamRefs)
            {
                Operand::OpMask dstOpMask;

                if (srcOperandPtr->getMask() == Operand::OPM_ALL)
                {
                    dstOpMask = getParameterMaskByType(curSrcParam->getType());
                }
                else
                {
                    dstOpMask = srcOperandPtr->getMask();
                }

                // Replace the original source operand with a new operand the reference the new merged parameter.
                *srcOperandPtr = Operand(dstParameter, srcOperandPtr->getSemantic(), dstOpMask);
            }
        }
    }
}

//-----------------------------------------------------------------------------
int ProgramProcessor::getParameterFloatCount(GpuConstantType type)
{
    int floatCount = 0;

    switch (type)
    {
    case GCT_FLOAT1: floatCount = 1; break;
    case GCT_FLOAT2: floatCount = 2; break;
    case GCT_FLOAT3: floatCount = 3; break;
    case GCT_FLOAT4: floatCount = 4; break;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            "Invalid parameter float type.",
            "ProgramProcessor::getParameterFloatCount");
    }

    return floatCount;
}

//-----------------------------------------------------------------------------
Operand::OpMask ProgramProcessor::getParameterMaskByType(GpuConstantType type)
{
    switch (type)
    {
    case GCT_FLOAT1: return Operand::OPM_X;
    case GCT_FLOAT2: return Operand::OPM_XY;
    case GCT_FLOAT3: return Operand::OPM_XYZ;
    case GCT_FLOAT4: return Operand::OPM_XYZW;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid parameter type.");
    }
}

//-----------------------------------------------------------------------------
Operand::OpMask ProgramProcessor::getParameterMaskByFloatCount(int floatCount)
{
    switch (floatCount)
    {
    case 1: return Operand::OPM_X;
    case 2: return Operand::OPM_XY;
    case 3: return Operand::OPM_XYZ;
    case 4: return Operand::OPM_XYZW;
    default:
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid parameter float type");
    }
}



//-----------------------------------------------------------------------------
void ProgramProcessor::buildMergeCombinations()
{
    mParamMergeCombinations.push_back(
        MergeCombination(
        1, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        1, Operand::OPM_ALL,
        0, Operand::OPM_ALL));

    mParamMergeCombinations.push_back(
        MergeCombination(
        2, Operand::OPM_ALL,
        1, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL));

    mParamMergeCombinations.push_back(
        MergeCombination(
        4, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL));

    mParamMergeCombinations.push_back(
        MergeCombination(
        0, Operand::OPM_ALL,
        2, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL));

    mParamMergeCombinations.push_back(
        MergeCombination(
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        0, Operand::OPM_ALL,
        1, Operand::OPM_ALL));
}

//-----------------------------------------------------------------------------
ProgramProcessor::MergeParameter::MergeParameter()
{
    clear();
}

//-----------------------------------------------------------------------------
void ProgramProcessor::MergeParameter::addSourceParameter(ParameterPtr srcParam, Operand::OpMask mask)
{
    // Case source count exceeded maximum
    if (mSrcParameterCount >= 4)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
            "Merged parameter source parameters overflow",
            "MergeParameter::addSourceParameter");
    }

    mSrcParameter[mSrcParameterCount]     = srcParam;
    mSrcParameterMask[mSrcParameterCount] = mask;

    if (mask == Operand::OPM_ALL)
    {
        mDstParameterMask[mSrcParameterCount] = mask;

        mUsedFloatCount += getParameterFloatCount(srcParam->getType());
    }
    else
    {
        int srcParamFloatCount = Operand::getFloatCount(mask);

        mDstParameterMask[mSrcParameterCount] = Operand::OpMask(getParameterMaskByFloatCount(srcParamFloatCount) << mUsedFloatCount);
        mUsedFloatCount += srcParamFloatCount;
    }

    mSrcParameterCount++;


    // Case float count exceeded maximum
    if (mUsedFloatCount > 4)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
            "Merged parameter floats overflow",
            "MergeParameter::addSourceParameter");
    }

}

//-----------------------------------------------------------------------------
int ProgramProcessor::MergeParameter::getUsedFloatCount()
{
    return mUsedFloatCount;
}

//-----------------------------------------------------------------------------
void ProgramProcessor::MergeParameter::createDestinationParameter(int usage, int index)
{
    GpuConstantType dstParamType = GCT_UNKNOWN;

    switch (getUsedFloatCount())
    {
    case 1:
        dstParamType = GCT_FLOAT1;
        break;

    case 2:
        dstParamType = GCT_FLOAT2;
        break;

    case 3:
        dstParamType = GCT_FLOAT3;
        break;

    case 4:
        dstParamType = GCT_FLOAT4;
        break;

    }


    if (usage == Operand::OPS_IN)
    {
        mDstParameter = ParameterFactory::createInTexcoord(dstParamType, index, Parameter::SPC_UNKNOWN);
    }
    else if (usage == Operand::OPS_OUT)
    {
        mDstParameter = ParameterFactory::createOutTexcoord(dstParamType, index, Parameter::SPC_UNKNOWN);
    }
}

//-----------------------------------------------------------------------------
Ogre::RTShader::ParameterPtr ProgramProcessor::MergeParameter::getDestinationParameter(int usage, int index)
{
    if (!mDstParameter)
        createDestinationParameter(usage, index);

    return mDstParameter;
}

//-----------------------------------------------------------------------------
void ProgramProcessor::MergeParameter::clear()
{
    mDstParameter.reset();
    for (unsigned int i=0; i < 4; ++i)
    {
        mSrcParameter[i].reset();
        mSrcParameterMask[i] = Operand::OPM_NONE;
        mDstParameterMask[i] = Operand::OPM_NONE;
    }
    mSrcParameterCount = 0;
    mUsedFloatCount = 0;
}

}
}
