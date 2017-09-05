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
	//-----------------------------------------------------------------------------------
	void ShaderPiecesManager::enumeratePieceFiles(void)
	{
		// remove all pieces
		for (size_t i = 0; i < mNumShaderTypes; i++)
		{
			mPieceFileNames[i].clear();
			mLoadedPieces[i].clear();
		}

		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();

		for (size_t i = 0; i < mNumShaderTypes; ++i)
		{
			StringVecMap& pieceFileNames = mPieceFileNames[i];

			FileInfoListPtr list = rgm.findResourceFileInfo(mResorceGroup, "*_piece" + FilePatterns[i] + ".*");
			FileInfoList::iterator it = list->begin();
			FileInfoList::iterator end = list->end();

			for (; it != end; it++)
			{
				String name, ext;
				StringUtil::splitBaseFilename(it->filename, name, ext);
				pieceFileNames[ext].push_back(it->filename);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	const StringVector& ShaderPiecesManager::getPieces(const String& language, GpuProgramType shaderType, bool reload)
	{
		String languageTemplateExtension = language + "t";

		ResourceGroupManager& rgm = ResourceGroupManager::getSingleton();

		StringVecMap& loadedPieces = mLoadedPieces[(int)shaderType];
		StringVecMap& pieceFileNames = mPieceFileNames[(int)shaderType];

		if (reload)
		{
			StringVecMap::iterator it = loadedPieces.find(languageTemplateExtension);
			if (it == loadedPieces.end())
				loadedPieces.erase(it);
		}
		
		StringVecMap::iterator loadedFilesIt = loadedPieces.find(languageTemplateExtension);
		if (loadedFilesIt != loadedPieces.end())
		{
		    return loadedFilesIt->second;
		}

        // the piece files for the given shader type and languarge are not loaded yet.
        StringVector& pieces = loadedPieces[languageTemplateExtension];
        StringVecMap::iterator pieceFileNamesIt = pieceFileNames.find(languageTemplateExtension);
        if (pieceFileNamesIt != pieceFileNames.end())
        {
            StringVector::iterator it = pieceFileNamesIt->second.begin();
            StringVector::iterator end = pieceFileNamesIt->second.end();

            for (; it != end; it++)
            {
                pieces.push_back(rgm.openResource(*it, mResorceGroup)->getAsString());
            }
        }

        return pieces;
	}
	//-----------------------------------------------------------------------------------
}
