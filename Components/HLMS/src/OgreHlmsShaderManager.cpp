/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#include "OgreHlmsShaderManager.h"
#include "OgreHlmsShaderGenerator.h"
#include "OgreHlmsShaderPiecesManager.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsShaderCommon.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------------
    ShaderManager::ShaderManager(const String& pieseFilesResorceGroup)
        : mShaderPiecesManager(pieseFilesResorceGroup)
    {
        mShaderPiecesManager.enumeratePieceFiles();
	}
	//-----------------------------------------------------------------------------------
	ShaderManager::~ShaderManager()
	{
		ShaderCacheMap::iterator it = mShaderCache.begin();
		ShaderCacheMap::iterator endIt = mShaderCache.end();
		for (; it != endIt; ++it)
		{
			GpuProgramPtr gpuPrg = it->second;
			gpuPrg->unload();
			HighLevelGpuProgramManager::getSingleton().remove(gpuPrg);
			gpuPrg.reset();
		}
		mShaderCache.clear();
	}
	//-----------------------------------------------------------------------------------
	GpuProgramPtr ShaderManager::getGpuProgram(HlmsDatablock* dataBlock)
	{
		uint32 hash = dataBlock->getHash();
		std::map<uint32, GpuProgramPtr>::iterator it = mShaderCache.find(hash);
		if (it != mShaderCache.end())
		{
			return (*it).second;
		}

		String typeStr = FilePatterns[dataBlock->getShaderType()];

		std::stringstream sstream;
		sstream << std::hex << hash;
		std::string hashString = sstream.str();

		String name = hashString + typeStr;

		// generate the shader code
		String code = dataBlock->getTemplate()->getTemplate();
		const StringVector& pieces = mShaderPiecesManager.getPieces(dataBlock->getLanguage(), dataBlock->getShaderType());
		code = ShaderGenerator::parse(code, *(dataBlock->getPropertyMap()), pieces);

		GpuProgramPtr gpuProgram = createGpuProgram(name, code, dataBlock);

		mShaderCache[hash] = gpuProgram;

		return gpuProgram;
	}
	//-----------------------------------------------------------------------------------
	GpuProgramPtr ShaderManager::createGpuProgram(const String& name, const String& code, HlmsDatablock* dataBlock)
	{
		HighLevelGpuProgramPtr gpuProgram = HighLevelGpuProgramManager::getSingleton().createProgram(name,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, dataBlock->getLanguage(), dataBlock->getShaderType());

		gpuProgram->setSource(code);

		if (dataBlock->getLanguage() == "hlsl")
		{
			gpuProgram->setParameter("entry_point", "main");
		
			// HLSL program requires specific target profile settings - we have to split the profile string.
			const StringVector& profilesList = dataBlock->getProfileList();
			StringVector::const_iterator it = profilesList.begin();
			StringVector::const_iterator itEnd = profilesList.end();

			for (; it != itEnd; ++it)
			{
				if (GpuProgramManager::getSingleton().isSyntaxSupported(*it))
				{
					gpuProgram->setParameter("target", *it);
					break;
				}
			}
		}
		
		gpuProgram->load();

		// Case an error occurred.
		if (gpuProgram->hasCompileError())
		{
			gpuProgram.reset();
			return GpuProgramPtr(gpuProgram);
		}

		return gpuProgram;
	}
    //-----------------------------------------------------------------------------------
}
