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
#include "OgreCgProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include <cctype>

namespace Ogre {
	//-----------------------------------------------------------------------
	CgProgram::CmdProfiles CgProgram::msCmdProfiles;
	CgProgram::CmdArgs CgProgram::msCmdArgs;
	//-----------------------------------------------------------------------
	void CgProgram::selectProfile(void)
	{
		static const String specialCgProfiles[] = {"hlslv", "hlslf", "glslv", "glslf", "glslg"};
		static const size_t specialCgProfilesCount = sizeof(specialCgProfiles)/sizeof(String);
		static const String* specialCgProfilesEnd = specialCgProfiles + specialCgProfilesCount;

		mSelectedProfile.clear();
		mSelectedCgProfile = CG_PROFILE_UNKNOWN;

		StringVector::iterator i, iend;
		iend = mProfiles.end();
		GpuProgramManager& gpuMgr = GpuProgramManager::getSingleton();
		bool useDelegate = false;

		for (i = mProfiles.begin(); i != iend; ++i)
		{
			bool syntaxSupported = gpuMgr.isSyntaxSupported(*i);
			if (!syntaxSupported && find(specialCgProfiles, specialCgProfilesEnd, *i) != specialCgProfilesEnd)
			{
				// Cg has some "special" profiles which don't have direct equivalents
				// in the GpuProgramManager's supported syntaxes.
				// For now, the following works
				if (gpuMgr.isSyntaxSupported(i->substr(0, 4)))
				{
					syntaxSupported = true;
					useDelegate = true;
				}

			}

			if (syntaxSupported)
			{
				mSelectedProfile = *i;
				String selectedProfileForFind = mSelectedProfile;
				if(StringUtil::startsWith(mSelectedProfile,"vs_4_0_", true))
				{
					selectedProfileForFind = "vs_4_0";
				}
				if(StringUtil::startsWith(mSelectedProfile,"ps_4_0_", true))
				{
					selectedProfileForFind = "ps_4_0";
				}
				mSelectedCgProfile = cgGetProfile(selectedProfileForFind.c_str());
				// Check for errors
				checkForCgError("CgProgram::selectProfile",
					"Unable to find CG profile enum for program " + mName + ": ", mCgContext);

				// do we need a delegate?
				if (useDelegate && !mDelegate)
				{
					mDelegate =
						HighLevelGpuProgramManager::getSingleton().createProgram(
								mName+"/Delegate", mGroup, getHighLevelLanguage(), mType);
					mDelegate->setParameter("target", getHighLevelTarget());
					mDelegate->setParameter("entry_point", "main");
					// HLSL/GLSL output uses row major matrices, so need to tell Ogre that
					mDelegate->setParameter("column_major_matrices", "false");
					// HLSL output requires backwards compatibility to be enabled
					mDelegate->setParameter("backwards_compatibility", "true");
				}
				else if (!useDelegate && mDelegate)
				{
					ResourcePtr rs (mDelegate);
					HighLevelGpuProgramManager::getSingleton().remove(rs);
					mDelegate.reset();
				}

				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	void CgProgram::buildArgs(void)
	{
		StringVector args;
		if (!mPreprocessorDefines.empty())
			args = StringUtil::split(mPreprocessorDefines);

		StringVector::const_iterator i;
		if (mSelectedCgProfile == CG_PROFILE_VS_1_1)
		{
			// Need the 'dcls' argument whenever we use this profile
			// otherwise compilation of the assembler will fail
			bool dclsFound = false;
			for (i = args.begin(); i != args.end(); ++i)
			{
				if (*i == "dcls")
				{
					dclsFound = true;
					break;
				}
			}
			if (!dclsFound)
			{
				args.push_back("-profileopts");
				args.push_back("dcls");
			}
		}
		args.push_back("-DOGRE_CG");
		if (getType() == GPT_VERTEX_PROGRAM)
			args.push_back("-DOGRE_VERTEX_SHADER");

		// Now split args into that god-awful char** that Cg insists on
		freeCgArgs();
		mCgArguments = OGRE_ALLOC_T(char*, args.size() + 1, MEMCATEGORY_RESOURCE);
		int index = 0;
		for (i = args.begin(); i != args.end(); ++i, ++index)
		{
			mCgArguments[index] = OGRE_ALLOC_T(char, i->length() + 1, MEMCATEGORY_RESOURCE);
			strcpy(mCgArguments[index], i->c_str());
		}
		// Null terminate list
		mCgArguments[index] = 0;


	}
	//-----------------------------------------------------------------------
	void CgProgram::freeCgArgs(void)
	{
		if (mCgArguments)
		{
			size_t index = 0;
			char* current = mCgArguments[index];
			while (current)
			{
				OGRE_FREE(current, MEMCATEGORY_RESOURCE);
				mCgArguments[index] = 0;
				current = mCgArguments[++index];
			}
			OGRE_FREE(mCgArguments, MEMCATEGORY_RESOURCE);
			mCgArguments = 0;
		}
	}
	//-----------------------------------------------------------------------
	void CgProgram::loadFromSource(void)
	{
		selectProfile();

		// need to differentiate between target profiles
		// also HLSL and Cg shaders sources are identical
        uint32 hash = HashCombine(0, mSelectedCgProfile);
        hash = _getHash(hash);

		if ( GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(hash) )
		{
			getMicrocodeFromCache(hash);
		}
		else
		{
			compileMicrocode();

            if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache())
            {
                addMicrocodeToCache(hash);
            }
		}

		if (mDelegate)
		{
			mDelegate->setSource(mProgramString);

			if (mSelectedCgProfile == CG_PROFILE_GLSLG)
			{
				// need to set input and output operations
				if (mInputOp == CG_POINT)
				{
					mDelegate->setParameter("input_operation_type", "point_list");
				}
				else if (mInputOp == CG_LINE)
				{
					mDelegate->setParameter("input_operation_type", "line_strip");
				}
				else if (mInputOp == CG_LINE_ADJ)
				{
					mDelegate->setParameter("input_operation_type", "line_strip_adj");
				}
				else if (mInputOp == CG_TRIANGLE)
				{
					mDelegate->setParameter("input_operation_type", "triangle_strip");
				}
				else if (mInputOp == CG_TRIANGLE_ADJ)
				{
					mDelegate->setParameter("input_operation_type", "triangle_strip_adj");
				}

				if (mOutputOp == CG_POINT_OUT)
					mDelegate->setParameter("output_operation_type", "point_list");
				else if (mOutputOp == CG_LINE_OUT)
					mDelegate->setParameter("output_operation_type", "line_strip");
				else if (mOutputOp == CG_TRIANGLE_OUT)
					mDelegate->setParameter("output_operation_type", "triangle_strip");
			}
			if (getHighLevelLanguage() == "glsl")
			{
				// for GLSL, also ensure we explicitly bind samplers to their register
				// otherwise, GLSL will assign them in the order first used, which is
				// not what we want.
				GpuProgramParametersSharedPtr params = mDelegate->getDefaultParameters();
				for (std::map<String,int>::iterator i = mSamplerRegisterMap.begin(); i != mSamplerRegisterMap.end(); ++i)
					params->setNamedConstant(i->first, i->second);
			}
			mDelegate->load();
		}
	}
	//-----------------------------------------------------------------------
	void CgProgram::getMicrocodeFromCache(uint32 id)
	{
		GpuProgramManager::Microcode cacheMicrocode =
			GpuProgramManager::getSingleton().getMicrocodeFromCache(id);

		cacheMicrocode->seek(0);

		// get size of string
		size_t programStringSize = 0;
		cacheMicrocode->read(&programStringSize, sizeof(size_t));

		// get microcode
		mProgramString.resize(programStringSize);
		cacheMicrocode->read(&mProgramString[0], programStringSize);

		// get size of param map
		size_t parametersMapSize = 0;
		cacheMicrocode->read(&parametersMapSize, sizeof(size_t));

		// get params
		for(size_t i = 0 ; i < parametersMapSize ; i++)
		{
			String paramName;
			size_t stringSize = 0;
			GpuConstantDefinition def;

			// get string size
			cacheMicrocode->read(&stringSize, sizeof(size_t));

			// get string
			paramName.resize(stringSize);
			cacheMicrocode->read(&paramName[0], stringSize);

			// get def
			cacheMicrocode->read( &def, sizeof(GpuConstantDefinition));

			mParametersMap.emplace(paramName, def);
		}

		if (mDelegate)
		{
			// get sampler register mapping
			size_t samplerMapSize = 0;
			cacheMicrocode->read(&samplerMapSize, sizeof(size_t));
			for (size_t i = 0; i < samplerMapSize; ++i)
			{
				String paramName;
				size_t stringSize = 0;
				int reg = -1;

				cacheMicrocode->read(&stringSize, sizeof(size_t));
				paramName.resize(stringSize);
				cacheMicrocode->read(&paramName[0], stringSize);
				cacheMicrocode->read(&reg, sizeof(int));
			}

			// get input/output operations type
			cacheMicrocode->read(&mInputOp, sizeof(CGenum));
			cacheMicrocode->read(&mOutputOp, sizeof(CGenum));
		}

	}
	//-----------------------------------------------------------------------
	void CgProgram::compileMicrocode(void)
	{
		// Create Cg Program

		/// Program handle
		CGprogram cgProgram;

		if (mSelectedCgProfile == CG_PROFILE_UNKNOWN)
		{
			LogManager::getSingleton().logMessage(
				"Attempted to load Cg program '" + mName + "', but no supported "
				"profile was found. ");
			return;
		}
		buildArgs();
		// deal with includes
		String sourceToUse = _resolveIncludes(mSource, this, mFilename, true);

		cgProgram = cgCreateProgram(mCgContext, CG_SOURCE, sourceToUse.c_str(),
			mSelectedCgProfile, mEntryPoint.c_str(), const_cast<const char**>(mCgArguments));

		// Test
		//LogManager::getSingleton().logMessage(cgGetProgramString(mCgProgram, CG_COMPILED_PROGRAM));

		// Check for errors
		checkForCgError("CgProgram::compileMicrocode",
			"Unable to compile Cg program " + mName + ": ", mCgContext);

		CGerror error = cgGetError();
		if (error == CG_NO_ERROR)
		{
			// get program string (result of cg compile)
			mProgramString = cgGetProgramString(cgProgram, CG_COMPILED_PROGRAM);
			checkForCgError("CgProgram::compileMicrocode",
				"Unable to retrieve program code for Cg program " + mName + ": ", mCgContext);

			// get params
			mParametersMap.clear();
			mParametersMapSizeAsBuffer = 0;
			mSamplerRegisterMap.clear();
			recurseParams(cgGetFirstParameter(cgProgram, CG_PROGRAM));
			recurseParams(cgGetFirstParameter(cgProgram, CG_GLOBAL));

			if (mDelegate)
			{
				// Delegating to HLSL or GLSL, need to clean up Cg's output
				fixHighLevelOutput(mProgramString);
				if (mSelectedCgProfile == CG_PROFILE_GLSLG)
				{
					// need to determine input and output operations
					mInputOp = cgGetProgramInput(cgProgram);
					mOutputOp = cgGetProgramOutput(cgProgram);
				}
			}

			// Unload Cg Program - we don't need it anymore
			cgDestroyProgram(cgProgram);
			//checkForCgError("CgProgram::unloadImpl",
			//  "Error while unloading Cg program " + mName + ": ",
			//  mCgContext);
			cgProgram = 0;
		}


	}
	//-----------------------------------------------------------------------
	void CgProgram::addMicrocodeToCache(uint32 id)
	{
		size_t programStringSize = mProgramString.size();
		uint32 sizeOfMicrocode = static_cast<uint32>(
													 sizeof(size_t) +   // size of mProgramString
													 programStringSize + // microcode - mProgramString
													 sizeof(size_t) + // size of param map
													 mParametersMapSizeAsBuffer);

		// create microcode
		auto newMicrocode = GpuProgramManager::createMicrocode(sizeOfMicrocode);

		newMicrocode->seek(0);

		// save size of string
		newMicrocode->write(&programStringSize, sizeof(size_t));

		// save microcode
		newMicrocode->write(&mProgramString[0], programStringSize);

		// save size of param map
		size_t parametersMapSize = mParametersMap.size();
		newMicrocode->write(&parametersMapSize, sizeof(size_t));

		// save params
		GpuConstantDefinitionMap::const_iterator iter = mParametersMap.begin();
		GpuConstantDefinitionMap::const_iterator iterE = mParametersMap.end();
		for (; iter != iterE ; iter++)
		{
			const String & paramName = iter->first;
			const GpuConstantDefinition & def = iter->second;

			// save string size
			size_t stringSize = paramName.size();
			newMicrocode->write(&stringSize, sizeof(size_t));

			// save string
			newMicrocode->write(&paramName[0], stringSize);

			// save def
			newMicrocode->write(&def, sizeof(GpuConstantDefinition));
		}

		if (mDelegate)
		{
			// save additional info required for delegating
			size_t samplerMapSize = mSamplerRegisterMap.size();
			newMicrocode->write(&samplerMapSize, sizeof(size_t));

			// save sampler register mapping
			std::map<String,int>::const_iterator sampRegister = mSamplerRegisterMap.begin();
			std::map<String,int>::const_iterator sampRegisterE = mSamplerRegisterMap.end();
			for (; sampRegister != sampRegisterE; ++sampRegister)
			{
				const String& paramName = sampRegister->first;
				int reg = sampRegister->second;

				size_t stringSize = paramName.size();
				newMicrocode->write(&stringSize, sizeof(size_t));
				newMicrocode->write(paramName.data(), stringSize);
				newMicrocode->write(&reg, sizeof(int));
			}

			// save input/output operations type
			newMicrocode->write(&mInputOp, sizeof(CGenum));
			newMicrocode->write(&mOutputOp, sizeof(CGenum));
		}

		// add to the microcode to the cache
		GpuProgramManager::getSingleton().addMicrocodeToCache(id, newMicrocode);
	}
	//-----------------------------------------------------------------------
	void CgProgram::createLowLevelImpl(void)
	{
		if (mDelegate)
			return;

		// ignore any previous error
		if (mSelectedCgProfile != CG_PROFILE_UNKNOWN && !mCompileError)
		{

			if ( false
// the hlsl 4 profiles are only supported in OGRE from CG 2.2
#if(CG_VERSION_NUM >= 2200)
				|| mSelectedCgProfile == CG_PROFILE_VS_4_0
				|| mSelectedCgProfile == CG_PROFILE_PS_4_0
#endif
#if(CG_VERSION_NUM >= 3000)
				|| mSelectedCgProfile == CG_PROFILE_VS_5_0
				|| mSelectedCgProfile == CG_PROFILE_PS_5_0
				|| mSelectedCgProfile == CG_PROFILE_DS_5_0
				|| mSelectedCgProfile == CG_PROFILE_HS_5_0
#endif
				 )
			{
				// Create a high-level program, give it the same name as us
                HighLevelGpuProgramManager::getSingleton().remove(mName, mGroup);
				HighLevelGpuProgramPtr vp =
					HighLevelGpuProgramManager::getSingleton().createProgram(
					mName, mGroup, "hlsl", mType);
				vp->setSource(mProgramString);
				vp->setParameter("target", mSelectedProfile);
				vp->setParameter("entry_point", "main");

				vp->load();

				mAssemblerProgram = vp;
			}
			else
			{
				// Create a low-level program, give it the same name as us
                mAssemblerProgram =
					GpuProgramManager::getSingleton().createProgramFromString(
					mName+"/Delegate",
					mGroup,
					mProgramString,
					mType,
					mSelectedProfile);
			}
		}
	}
	//-----------------------------------------------------------------------
	String CgProgram::getHighLevelLanguage() const
	{
		switch (mSelectedCgProfile)
		{
			case CG_PROFILE_GLSLF:
			case CG_PROFILE_GLSLV:
			case CG_PROFILE_GLSLG:
			case CG_PROFILE_GLSLC:
				return "glsl";
			case CG_PROFILE_HLSLF:
			case CG_PROFILE_HLSLV:
				return "hlsl";
			default:
				return "unknown";
		}
	}
	//-----------------------------------------------------------------------
	String CgProgram::getHighLevelTarget() const
	{
		// HLSL delegates need a target to compile to.
		// Return value for GLSL delegates is ignored.
		GpuProgramManager* gpuMgr = GpuProgramManager::getSingletonPtr();

		if (mSelectedCgProfile == CG_PROFILE_HLSLF)
		{
			static const String fpProfiles[] = {
#if(CG_VERSION_NUM >= 3000)
			"ps_5_0",
#endif
#if(CG_VERSION_NUM >= 2200)
			"ps_4_0",
#endif
			"ps_3_0", "ps_2_x", "ps_2_0", "ps_1_4", "ps_1_3", "ps_1_2", "ps_1_1"};
			static const size_t numFpProfiles = sizeof(fpProfiles)/sizeof(String);
			// find the highest profile available
			for (size_t i = 0; i < numFpProfiles; ++i)
			{
				if (gpuMgr->isSyntaxSupported(fpProfiles[i]))
					return fpProfiles[i];
			}
		}
		else if (mSelectedCgProfile == CG_PROFILE_HLSLV)
		{
			static const String vpProfiles[] = {
#if(CG_VERSION_NUM >= 3000)
			"vs_5_0",
#endif
#if(CG_VERSION_NUM >= 2200)
			"vs_4_0",
#endif
			"vs_3_0", "vs_2_x", "vs_2_0", "vs_1_4", "vs_1_3", "vs_1_2", "vs_1_1"};
			static const size_t numVpProfiles = sizeof(vpProfiles)/sizeof(String);
			// find the highest profile available
			for (size_t i = 0; i < numVpProfiles; ++i)
			{
				if (gpuMgr->isSyntaxSupported(vpProfiles[i]))
					return vpProfiles[i];
			}
		}

		return "unknown";
	}
	//-----------------------------------------------------------------------
	struct HighLevelOutputFixer
	{
		const String& source;
		const GpuConstantDefinitionMap& paramMap;
		const std::map<String,int> samplerMap;
		bool glsl;
		String output;
		std::map<String,String> paramNameMap;
		String::size_type start;
		struct ReplacementMark
		{
			String::size_type pos, len;
			String replaceWith;
			bool operator<(const ReplacementMark& o) const { return pos < o.pos; }
		};
		std::vector<ReplacementMark> replacements;

		HighLevelOutputFixer(const String& src, const GpuConstantDefinitionMap& params,
			const std::map<String,int>& samplers, bool isGLSL)
			: source(src), paramMap(params), samplerMap(samplers), glsl(isGLSL), start(0)
		{
			findNameMappings();
			replaceParameterNames();
			buildOutput();
		}

		void findNameMappings()
		{
			String::size_type cur = 0, end = 0;
			while (cur < source.size())
			{
				// look for a comment line describing a parameter name mapping
				// comment format: //var type parameter : [something] : new name : [something] : [something]
				cur = source.find("//var", cur);
				if (cur == String::npos)
					break;
				end = source.find('\n', cur);
				std::vector<String> cols = StringUtil::split(source.substr(cur, end-cur), ":");
				cur = end;
				if (cols.size() < 3)
					continue;
				std::vector<String> def = StringUtil::split(cols[0], "[ ");
				if (def.size() < 3)
					continue;
				StringUtil::trim(cols[2]);
				std::vector<String> repl = StringUtil::split(cols[2], "[ ");
				String oldName = def[2];
				String newName = repl[0];
				StringUtil::trim(oldName);
				StringUtil::trim(newName);
				if (newName.empty() || newName[0] != '_')
					continue;
				// if that name is present in our lists, mark in name translation map
				GpuConstantDefinitionMap::const_iterator it = paramMap.find(oldName);
				if (it != paramMap.end() || samplerMap.find(oldName) != samplerMap.end())
				{
					LogManager::getSingleton().stream() << "Replacing parameter name: " << newName << " -> " << oldName;
					paramNameMap.insert(std::make_pair(oldName, newName));
				}
			}
			// end now points at the end of the last comment, so we can strip anything before
			start = (end == String::npos ? end : end+1);
		}

		void replaceParameterNames()
		{
			for (GpuConstantDefinitionMap::const_iterator it = paramMap.begin(); it != paramMap.end(); ++it)
			{
				const String& oldName = it->first;
				std::map<String,String>::const_iterator pi = paramNameMap.find(oldName);
				if (pi != paramNameMap.end())
				{
					const String& newName = pi->second;
					String::size_type beg = start;
					// do we need to replace the definition of the parameter? (GLSL only)
					if (glsl)
					{
					    if(it->second.arraySize > 1)
					        LogManager::getSingleton().logWarning("Incomplete Cg-GLSL mapping - '"+oldName+"' is an mat array");
					    else if (it->second.constType == GCT_MATRIX_2X2)
							beg = findAndMark("uniform vec2 "+newName+"[2]", "uniform mat2 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_3X3)
							beg = findAndMark("uniform vec3 "+newName+"[3]", "uniform mat3 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_4X4)
							beg = findAndMark("uniform vec4 "+newName+"[4]", "uniform mat4 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_2X3)
							beg = findAndMark("uniform vec3 "+newName+"[2]", "uniform mat2x3 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_2X4)
							beg = findAndMark("uniform vec4 "+newName+"[2]", "uniform mat2x4 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_3X2)
							beg = findAndMark("uniform vec2 "+newName+"[3]", "uniform mat3x2 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_3X4)
							beg = findAndMark("uniform vec4 "+newName+"[3]", "uniform mat3x4 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_4X2)
							beg = findAndMark("uniform vec2 "+newName+"[4]", "uniform mat4x2 "+oldName, beg);
						else if (it->second.constType == GCT_MATRIX_4X3)
							beg = findAndMark("uniform vec3 "+newName+"[4]", "uniform mat4x3 "+oldName, beg);
					}

					// mark all occurrences of the parameter name for replacement
					findAndMark(newName, oldName, beg);
				}
			}

			for (std::map<String,int>::const_iterator it = samplerMap.begin(); it != samplerMap.end(); ++it)
			{
				const String& oldName = it->first;
				std::map<String,String>::const_iterator pi = paramNameMap.find(oldName);
				if (pi != paramNameMap.end())
				{
					const String& newName = pi->second;
					findAndMark(newName, oldName, start);
				}
			}
		}

		String::size_type findAndMark(const String& search, const String& replaceWith, String::size_type cur)
		{
			ReplacementMark mark;
			mark.pos = String::npos;
			mark.len = search.size();
			mark.replaceWith = replaceWith;
			while (cur < source.size())
			{
				cur = source.find(search, cur);
				if (cur == String::npos)
					break;
				mark.pos = cur;
				cur += search.size();
				// check if previous or following character continue an identifier
				// in that case, skip this occurrence as it's part of a longer identifier
				if (mark.pos > 0)
				{
					char c = source[mark.pos-1];
					if (c == '_' || std::isalnum(c))
						continue;
				}
				if (mark.pos+1 < search.size())
				{
					char c = source[mark.pos+1];
					if (c == '_' || std::isalnum(c))
						continue;
				}
				replacements.push_back(mark);
			}
			if (mark.pos != String::npos)
				return mark.pos + search.size();
			else
				return String::npos;
		}

		void buildOutput()
		{
			// sort replacements in order of occurrence
			std::sort(replacements.begin(), replacements.end());
			String::size_type cur = start;
			for (std::vector<ReplacementMark>::iterator it = replacements.begin(); it != replacements.end(); ++it)
			{
				ReplacementMark& mark = *it;
				if (mark.pos > cur)
					output.append(source, cur, mark.pos-cur);
				output.append(mark.replaceWith);
				cur = mark.pos+mark.len;
			}
			if (cur < source.size())
				output.append(source, cur, String::npos);
		}
	};
	//-----------------------------------------------------------------------
	void CgProgram::fixHighLevelOutput(String& hlSource)
	{
		// Cg chooses to change parameter names when translating to another
		// high level language, possibly to avoid clashes with reserved keywords.
		// We need to revert that, otherwise Ogre parameter mappings fail.
		// Cg logs its renamings in the comments at the beginning of the
		// processed source file. We can get them from there.
		// We'll also get rid of those comments to trim down source code size.
#if OGRE_DEBUG_MODE || 1
		LogManager::getSingleton().stream() << "Cg high level output for " << getName() << ":\n" << hlSource;
#endif
		hlSource = HighLevelOutputFixer(hlSource, mParametersMap, mSamplerRegisterMap,
			mSelectedCgProfile == CG_PROFILE_GLSLV || mSelectedCgProfile == CG_PROFILE_GLSLF ||
			mSelectedCgProfile == CG_PROFILE_GLSLG).output;
#if OGRE_DEBUG_MODE || 1
		LogManager::getSingleton().stream() << "Cleaned high level output for " << getName() << ":\n" << hlSource;
#endif
	}


	//-----------------------------------------------------------------------
	void CgProgram::loadHighLevelSafe()
	{
	    safePrepare();
		OGRE_LOCK_AUTO_MUTEX;
		if (this->isSupported())
			loadHighLevel();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr CgProgram::createParameters()
	{
		loadHighLevelSafe();
		if (mDelegate)
			return mDelegate->createParameters();
		else
			return HighLevelGpuProgram::createParameters();
	}
	//-----------------------------------------------------------------------
	GpuProgram* CgProgram::_getBindingDelegate()
	{
		if (mDelegate)
			return mDelegate->_getBindingDelegate();
		else
			return HighLevelGpuProgram::_getBindingDelegate();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isSkeletalAnimationIncluded(void) const
	{
		if (mDelegate)
			return mDelegate->isSkeletalAnimationIncluded();
		else
			return HighLevelGpuProgram::isSkeletalAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isMorphAnimationIncluded(void) const
	{
		if (mDelegate)
			return mDelegate->isMorphAnimationIncluded();
		else
			return HighLevelGpuProgram::isMorphAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isPoseAnimationIncluded(void) const
	{
		if (mDelegate)
			return mDelegate->isPoseAnimationIncluded();
		else
			return HighLevelGpuProgram::isPoseAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isVertexTextureFetchRequired(void) const
	{
		if (mDelegate)
			return mDelegate->isVertexTextureFetchRequired();
		else
			return HighLevelGpuProgram::isVertexTextureFetchRequired();
	}
	//-----------------------------------------------------------------------
	const GpuProgramParametersPtr& CgProgram::getDefaultParameters(void)
	{
		loadHighLevelSafe();
		if (mDelegate)
			return mDelegate->getDefaultParameters();
		else
			return HighLevelGpuProgram::getDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::hasDefaultParameters(void) const
	{
		if (mDelegate)
			return mDelegate->hasDefaultParameters();
		else
			return HighLevelGpuProgram::hasDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassSurfaceAndLightStates(void) const
	{
		if (mDelegate)
			return mDelegate->getPassSurfaceAndLightStates();
		else
			return HighLevelGpuProgram::getPassSurfaceAndLightStates();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassFogStates(void) const
	{
		if (mDelegate)
			return mDelegate->getPassFogStates();
		else
			return HighLevelGpuProgram::getPassFogStates();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassTransformStates(void) const
	{
		if (mDelegate)
			return mDelegate->getPassTransformStates();
		else
		{
			return true; /* CG uses MVP matrix when -posinv argument passed */
		}
	}
	//-----------------------------------------------------------------------
	bool CgProgram::hasCompileError(void) const
	{
		if (mDelegate)
			return mDelegate->hasCompileError();
		else
			return HighLevelGpuProgram::hasCompileError();
	}
	//-----------------------------------------------------------------------
	void CgProgram::resetCompileError(void)
	{
		if (mDelegate)
			mDelegate->resetCompileError();
		else
			HighLevelGpuProgram::resetCompileError();
	}
	//-----------------------------------------------------------------------
	size_t CgProgram::getSize() const
	{
		if (mDelegate)
			return mDelegate->getSize();
		else
			return HighLevelGpuProgram::getSize();
	}
	//-----------------------------------------------------------------------
	void CgProgram::touch()
	{
		if (mDelegate)
			mDelegate->touch();
		else
			HighLevelGpuProgram::touch();
	}


	//-----------------------------------------------------------------------
	void CgProgram::unloadHighLevelImpl(void)
	{
        if (mDelegate)
        {
            mDelegate->getCreator()->remove(mDelegate);
            mDelegate.reset();
        }
	}
	//-----------------------------------------------------------------------
	void CgProgram::buildConstantDefinitions()
	{
		// Derive parameter names from Cg
		createParameterMappingStructures(true);

		if ( mProgramString.empty() )
			return;

		mConstantDefs->bufferSize = mLogicalToPhysical->bufferSize;

		GpuConstantDefinitionMap::const_iterator iter = mParametersMap.begin();
		GpuConstantDefinitionMap::const_iterator iterE = mParametersMap.end();
		for (; iter != iterE ; iter++)
		{
			GpuConstantDefinition def = iter->second;

			mConstantDefs->map.emplace(iter->first, iter->second);

			// Record logical / physical mapping
			OGRE_LOCK_MUTEX(mLogicalToPhysical->mutex);
			mLogicalToPhysical->map.emplace(def.logicalIndex,
					GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL, def.isFloat() ? BCT_FLOAT: BCT_INT));
			mLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
		}
	}
	//---------------------------------------------------------------------
	void CgProgram::recurseParams(CGparameter parameter, size_t contextArraySize)
	{
		while (parameter != 0)
		{
			// Look for uniform parameters only
			// Don't bother enumerating unused parameters, especially since they will
			// be optimised out and therefore not in the indexed versions
			CGtype paramType = cgGetParameterType(parameter);

			if (cgGetParameterVariability(parameter) == CG_UNIFORM &&
				paramType != CG_SAMPLER1D &&
				paramType != CG_SAMPLER2D &&
				paramType != CG_SAMPLER3D &&
				paramType != CG_SAMPLERCUBE &&
				paramType != CG_SAMPLERRECT &&
				cgGetParameterDirection(parameter) != CG_OUT &&
				cgIsParameterReferenced(parameter))
			{
				int arraySize;

				switch(paramType)
				{
				case CG_STRUCT:
					recurseParams(cgGetFirstStructParameter(parameter));
					break;
				case CG_ARRAY:
					// Support only 1-dimensional arrays
					arraySize = cgGetArraySize(parameter, 0);
					recurseParams(cgGetArrayParameter(parameter, 0), (size_t)arraySize);
					break;
				default:
					// Normal path (leaf)
					String paramName = cgGetParameterName(parameter);
					size_t logicalIndex = cgGetParameterResourceIndex(parameter);

					// Get the parameter resource, to calculate the physical index
					CGresource res = cgGetParameterResource(parameter);
					bool isRegisterCombiner = false;
					size_t regCombinerPhysicalIndex = 0;
					switch (res)
					{
					case CG_COMBINER_STAGE_CONST0:
						// register combiner, const 0
						// the index relates to the texture stage; store this as (stage * 2) + 0
						regCombinerPhysicalIndex = logicalIndex * 2;
						isRegisterCombiner = true;
						break;
					case CG_COMBINER_STAGE_CONST1:
						// register combiner, const 1
						// the index relates to the texture stage; store this as (stage * 2) + 1
						regCombinerPhysicalIndex = (logicalIndex * 2) + 1;
						isRegisterCombiner = true;
						break;
					default:
						// normal constant
						break;
					}

					// Trim the '[0]' suffix if it exists, we will add our own indexing later
					if (StringUtil::endsWith(paramName, "[0]", false))
					{
						paramName.erase(paramName.size() - 3);
					}


					GpuConstantDefinition def;
					def.arraySize = contextArraySize;
					mapTypeAndElementSize(paramType, isRegisterCombiner, def);

					if (def.constType == GCT_UNKNOWN)
					{
						LogManager::getSingleton().logMessage(
							"Problem parsing the following Cg Uniform: '"
							+ paramName + "' in file " + mName);
						// next uniform
						parameter = cgGetNextParameter(parameter);
						continue;
					}
					if (isRegisterCombiner)
					{
						def.physicalIndex = regCombinerPhysicalIndex;
					}
					else
					{
						// base position on existing buffer contents
						def.physicalIndex = mLogicalToPhysical->bufferSize*4;
					}

					def.logicalIndex = logicalIndex;
					if( mParametersMap.find(paramName) == mParametersMap.end())
					{
						mParametersMap.emplace(paramName, def);
						mParametersMapSizeAsBuffer += sizeof(size_t);
						mParametersMapSizeAsBuffer += paramName.size();
						mParametersMapSizeAsBuffer += sizeof(GpuConstantDefinition);
					}

					// Record logical / physical mapping
					OGRE_LOCK_MUTEX(mLogicalToPhysical->mutex);
					mLogicalToPhysical->map.emplace(def.logicalIndex,
							GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL, def.isFloat() ? BCT_FLOAT : BCT_INT));
					mLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;

					break;
				}
			}

			// now handle uniform samplers. This is needed to fix their register positions
			// if delegating to a GLSL shader.
			if (mDelegate && cgGetParameterVariability(parameter) == CG_UNIFORM && (
				paramType == CG_SAMPLER1D ||
				paramType == CG_SAMPLER2D ||
				paramType == CG_SAMPLER3D ||
				paramType == CG_SAMPLERCUBE ||
				paramType == CG_SAMPLERRECT) &&
				cgGetParameterDirection(parameter) != CG_OUT &&
				cgIsParameterReferenced(parameter))
			{
				String paramName = cgGetParameterName(parameter);
				CGresource res = cgGetParameterResource(parameter);
				int pos = -1;
				switch (res)
				{
				case CG_TEXUNIT0: pos = 0; break;
				case CG_TEXUNIT1: pos = 1; break;
				case CG_TEXUNIT2: pos = 2; break;
				case CG_TEXUNIT3: pos = 3; break;
				case CG_TEXUNIT4: pos = 4; break;
				case CG_TEXUNIT5: pos = 5; break;
				case CG_TEXUNIT6: pos = 6; break;
				case CG_TEXUNIT7: pos = 7; break;
				case CG_TEXUNIT8: pos = 8; break;
				case CG_TEXUNIT9: pos = 9; break;
				case CG_TEXUNIT10: pos = 10; break;
				case CG_TEXUNIT11: pos = 11; break;
				case CG_TEXUNIT12: pos = 12; break;
				case CG_TEXUNIT13: pos = 13; break;
				case CG_TEXUNIT14: pos = 14; break;
				case CG_TEXUNIT15: pos = 15; break;
#if(CG_VERSION_NUM >= 3000)
				case CG_TEXUNIT16: pos = 16; break;
				case CG_TEXUNIT17: pos = 17; break;
				case CG_TEXUNIT18: pos = 18; break;
				case CG_TEXUNIT19: pos = 19; break;
				case CG_TEXUNIT20: pos = 20; break;
				case CG_TEXUNIT21: pos = 21; break;
				case CG_TEXUNIT22: pos = 22; break;
				case CG_TEXUNIT23: pos = 23; break;
				case CG_TEXUNIT24: pos = 24; break;
				case CG_TEXUNIT25: pos = 25; break;
				case CG_TEXUNIT26: pos = 26; break;
				case CG_TEXUNIT27: pos = 27; break;
				case CG_TEXUNIT28: pos = 28; break;
				case CG_TEXUNIT29: pos = 29; break;
				case CG_TEXUNIT30: pos = 30; break;
				case CG_TEXUNIT31: pos = 31; break;
#endif
				default:
					break;
				}
				if (pos != -1)
				{
					mSamplerRegisterMap.insert(std::make_pair(paramName, pos));
				}
			}

			// Get next
			parameter = cgGetNextParameter(parameter);
		}


	}
	//-----------------------------------------------------------------------
	void CgProgram::mapTypeAndElementSize(CGtype cgType, bool isRegisterCombiner,
		GpuConstantDefinition& def) const
	{
		if (isRegisterCombiner)
		{
			// register combiners are the only single-float entries in our buffer
			def.constType = GCT_FLOAT1;
			def.elementSize = 1;
		}
		else
		{
			switch(cgType)
			{
			case CG_FLOAT:
			case CG_FLOAT1:
			case CG_HALF:
			case CG_HALF1:
				def.constType = GCT_FLOAT1;
				break;
			case CG_FLOAT2:
			case CG_HALF2:
				def.constType = GCT_FLOAT2;
				break;
			case CG_FLOAT3:
			case CG_HALF3:
				def.constType = GCT_FLOAT3;
				break;
			case CG_FLOAT4:
			case CG_HALF4:
				def.constType = GCT_FLOAT4;
				break;
			case CG_FLOAT2x2:
			case CG_HALF2x2:
				def.constType = GCT_MATRIX_2X2;
				break;
			case CG_FLOAT2x3:
			case CG_HALF2x3:
				def.constType = GCT_MATRIX_2X3;
				break;
			case CG_FLOAT2x4:
			case CG_HALF2x4:
				def.constType = GCT_MATRIX_2X4;
				break;
			case CG_FLOAT3x2:
			case CG_HALF3x2:
				def.constType = GCT_MATRIX_3X2;
				break;
			case CG_FLOAT3x3:
			case CG_HALF3x3:
				def.constType = GCT_MATRIX_3X3;
				break;
			case CG_FLOAT3x4:
			case CG_HALF3x4:
				def.constType = GCT_MATRIX_3X4;
				break;
			case CG_FLOAT4x2:
			case CG_HALF4x2:
				def.constType = GCT_MATRIX_4X2;
				break;
			case CG_FLOAT4x3:
			case CG_HALF4x3:
				def.constType = GCT_MATRIX_4X3;
				break;
			case CG_FLOAT4x4:
			case CG_HALF4x4:
				def.constType = GCT_MATRIX_4X4;
				break;
			case CG_INT:
			case CG_INT1:
				def.constType = GCT_INT1;
				break;
			case CG_INT2:
				def.constType = GCT_INT2;
				break;
			case CG_INT3:
				def.constType = GCT_INT3;
				break;
			case CG_INT4:
				def.constType = GCT_INT4;
				break;
			default:
				def.constType = GCT_UNKNOWN;
				break;
			}
			// Cg pads
			def.elementSize = GpuConstantDefinition::getElementSize(def.constType, true);
		}
	}
	//-----------------------------------------------------------------------
	CgProgram::CgProgram(ResourceManager* creator, const String& name,
		ResourceHandle handle, const String& group, bool isManual,
		ManualResourceLoader* loader, CGcontext context)
		: HighLevelGpuProgram(creator, name, handle, group, isManual, loader),
		mCgContext(context),
		mSelectedCgProfile(CG_PROFILE_UNKNOWN), mCgArguments(0), mParametersMapSizeAsBuffer(0)
	{
		if (createParamDictionary("CgProgram"))
		{
			setupBaseParamDictionary();

			ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("profiles",
				"Space-separated list of Cg profiles supported by this profile.",
				PT_STRING),&msCmdProfiles);
			dict->addParameter(ParameterDef("compile_arguments",
				"A string of compilation arguments to pass to the Cg compiler.",
				PT_STRING),&msCmdArgs);
		}

	}
	//-----------------------------------------------------------------------
	CgProgram::~CgProgram()
	{
		freeCgArgs();
		// have to call this here reather than in Resource destructor
		// since calling virtual methods in base destructors causes crash
		if (isLoaded())
		{
			unload();
		}
		else
		{
			unloadHighLevel();
		}
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isSupported(void) const
	{
		if (mDelegate)
			return mDelegate->isSupported();

		if (mCompileError || !isRequiredCapabilitiesSupported())
			return false;

		return mSelectedCgProfile != CG_PROFILE_UNKNOWN;
	}
	//-----------------------------------------------------------------------
	void CgProgram::setProfiles(const StringVector& profiles)
	{
		mProfiles = profiles;
		selectProfile();
	}

	//-----------------------------------------------------------------------
	const String& CgProgram::getLanguage(void) const
	{
		static const String language = "cg";

		return language;
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String CgProgram::CmdProfiles::doGet(const void *target) const
	{
		return StringConverter::toString(
			static_cast<const CgProgram*>(target)->getProfiles() );
	}
	void CgProgram::CmdProfiles::doSet(void *target, const String& val)
	{
		static_cast<CgProgram*>(target)->setProfiles(StringUtil::split(val));
	}
	//-----------------------------------------------------------------------
	String CgProgram::CmdArgs::doGet(const void *target) const
	{
		return static_cast<const CgProgram*>(target)->getPreprocessorDefines();
	}
	void CgProgram::CmdArgs::doSet(void *target, const String& val)
	{
		static_cast<CgProgram*>(target)->setPreprocessorDefines(val);
	}

}
