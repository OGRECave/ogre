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

#include "OgreHlmsShaderPiecesManager.h"
#include "OgreHlmsShaderCommon.h"

namespace Ogre
{
	const String PieceFilePatterns[] = { "piece_vs", "piece_ps", "piece_gs", "piece_hs", "piece_ds" };

	//-----------------------------------------------------------------------------------
	ShaderPiecesManager::ShaderPiecesManager(Archive *dataFolder)
		: mDataFolder(dataFolder)
	{

	}
	//-----------------------------------------------------------------------------------
	ShaderPiecesManager::~ShaderPiecesManager()
	{

	}
	//-----------------------------------------------------------------------------------
	void ShaderPiecesManager::enumeratePieceFiles(void)
	{
		if (!mDataFolder)
			return; //Some Hlms implementations may not use template files at all

		StringVectorPtr stringVectorPtr = mDataFolder->list(false, false);
		StringVector stringVectorLowerCase(*stringVectorPtr);
		{
			StringVector::iterator itor = stringVectorLowerCase.begin();
			StringVector::iterator end = stringVectorLowerCase.end();
			while (itor != end)
			{
				std::transform(itor->begin(), itor->end(), itor->begin(), ::tolower);
				++itor;
			}
		}

		size_t numShaderTypes = sizeof(PieceFilePatterns) / sizeof(*PieceFilePatterns);
		for (size_t i = 0; i<numShaderTypes; ++i)
		{
			StringVector::const_iterator itLowerCase = stringVectorLowerCase.begin();
			StringVector::const_iterator itor = stringVectorPtr->begin();
			StringVector::const_iterator end = stringVectorPtr->end();

			while (itor != end)
			{
				if (itLowerCase->find(PieceFilePatterns[i]) != String::npos)
				{
					String outBasename;
					String outExtention;

					StringUtil::splitBaseFilename(*itor, outBasename, outExtention);
					if (!outExtention.empty())
					{
						mPieceFiles[outExtention].push_back(*itor);
					}
				}

				++itLowerCase;
				++itor;
			}
		}
	}
	//-----------------------------------------------------------------------------------
	StringVector ShaderPiecesManager::getPieces(String language, GpuProgramType shaderType)
	{
		//TODO Finish impl.
		return StringVector();
	}
	//-----------------------------------------------------------------------------------
}
