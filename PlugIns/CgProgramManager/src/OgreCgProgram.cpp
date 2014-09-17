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
	CgProgram::CmdEntryPoint CgProgram::msCmdEntryPoint;
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
		bool foundProfile = false;
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
				if (useDelegate && mDelegate.isNull())
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
				else if (!useDelegate && !mDelegate.isNull())
				{
					ResourcePtr rs (mDelegate);
					HighLevelGpuProgramManager::getSingleton().remove(rs);
					mDelegate.setNull();
				}
				
				foundProfile = true;
				break;
			}
		}

		if (!foundProfile)
		{
			LogManager::getSingleton().logMessage(String("Error: ") + mName +  "'s syntax is not supported", LML_CRITICAL);
		}
	}
	//-----------------------------------------------------------------------
	void CgProgram::buildArgs(void)
	{
		StringVector args;
		if (!mCompileArgs.empty())
			args = StringUtil::split(mCompileArgs);

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

		if ( GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(String("CG_") + mName) )
		{
			getMicrocodeFromCache();
		}
		else
		{
			compileMicrocode();
		}

		if (!mDelegate.isNull())
		{
			mDelegate->setSource(mProgramString);
			mDelegate->setAdjacencyInfoRequired(isAdjacencyInfoRequired());
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
					mDelegate->setParameter("input_operation_type", "line_strip");
					mDelegate->setAdjacencyInfoRequired(true);
				}
				else if (mInputOp == CG_TRIANGLE)
				{
					mDelegate->setParameter("input_operation_type", "triangle_strip");
				}
				else if (mInputOp == CG_TRIANGLE_ADJ)
				{
					mDelegate->setParameter("input_operation_type", "triangle_strip");
					mDelegate->setAdjacencyInfoRequired(true);
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
				for (map<String,int>::type::iterator i = mSamplerRegisterMap.begin(); i != mSamplerRegisterMap.end(); ++i)
					params->setNamedConstant(i->first, i->second);
			}
			mDelegate->load();
		}
	}
	//-----------------------------------------------------------------------
	void CgProgram::getMicrocodeFromCache(void)
	{
		GpuProgramManager::Microcode cacheMicrocode = 
			GpuProgramManager::getSingleton().getMicrocodeFromCache(String("CG_") + mName);
		
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

			mParametersMap.insert(GpuConstantDefinitionMap::value_type(paramName, def));
		}

		if (!mDelegate.isNull())
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
		String sourceToUse = resolveCgIncludes(mSource, this, mFilename);

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

			if (!mDelegate.isNull())
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

			if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache())
			{
				addMicrocodeToCache();
			}
		}


	}
	//-----------------------------------------------------------------------
	void CgProgram::addMicrocodeToCache()
	{
		String name = String("CG_") + mName;
		size_t programStringSize = mProgramString.size();
		uint32 sizeOfMicrocode = static_cast<uint32>(
													 sizeof(size_t) +   // size of mProgramString
													 programStringSize + // microcode - mProgramString
													 sizeof(size_t) + // size of param map
													 mParametersMapSizeAsBuffer);

		// create microcode
		GpuProgramManager::Microcode newMicrocode = 
			GpuProgramManager::getSingleton().createMicrocode(sizeOfMicrocode);

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

		if (!mDelegate.isNull())
		{
			// save additional info required for delegating
			size_t samplerMapSize = mSamplerRegisterMap.size();
			newMicrocode->write(&samplerMapSize, sizeof(size_t));

			// save sampler register mapping
			map<String,int>::type::const_iterator sampRegister = mSamplerRegisterMap.begin();
			map<String,int>::type::const_iterator sampRegisterE = mSamplerRegisterMap.end();
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
		GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
	}
	//-----------------------------------------------------------------------
	void CgProgram::createLowLevelImpl(void)
	{
		if (!mDelegate.isNull())
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
				if (mType == GPT_FRAGMENT_PROGRAM) {
					//HACK : http://developer.nvidia.com/forums/index.php?showtopic=1063&pid=2378&mode=threaded&start=#entry2378
					//Still happens in CG 2.2. Remove hack when fixed.
					mProgramString = StringUtil::replaceAll(mProgramString, "oDepth.z", "oDepth");
				}
				// Create a low-level program, give it the same name as us
				mAssemblerProgram = 
					GpuProgramManager::getSingleton().createProgramFromString(
					mName, 
					mGroup,
					mProgramString,
					mType, 
					mSelectedProfile);
			}
			// Shader params need to be forwarded to low level implementation
			mAssemblerProgram->setAdjacencyInfoRequired(isAdjacencyInfoRequired());
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
		const map<String,int>::type samplerMap;
		bool glsl;
		String output;
		map<String,String>::type paramNameMap;
		String::size_type start;
		struct ReplacementMark
		{
			String::size_type pos, len;
			String replaceWith;
			bool operator<(const ReplacementMark& o) const { return pos < o.pos; }
		};
		vector<ReplacementMark>::type replacements;

		HighLevelOutputFixer(const String& src, const GpuConstantDefinitionMap& params, 
			const map<String,int>::type& samplers, bool isGLSL) 
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
				vector<String>::type cols = StringUtil::split(source.substr(cur, end-cur), ":");
				cur = end;
				if (cols.size() < 3)
					continue;
				vector<String>::type def = StringUtil::split(cols[0], "[ ");
				if (def.size() < 3)
					continue;
				StringUtil::trim(cols[2]);
				vector<String>::type repl = StringUtil::split(cols[2], "[ ");
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
				map<String,String>::type::const_iterator pi = paramNameMap.find(oldName);
				if (pi != paramNameMap.end())
				{
					const String& newName = pi->second;
					String::size_type beg = start;
					// do we need to replace the definition of the parameter? (GLSL only)
					if (glsl)
					{
						if (it->second.constType == GCT_MATRIX_2X2)
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

			for (map<String,int>::type::const_iterator it = samplerMap.begin(); it != samplerMap.end(); ++it)
			{
				const String& oldName = it->first;
				map<String,String>::type::const_iterator pi = paramNameMap.find(oldName);
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
			for (vector<ReplacementMark>::type::iterator it = replacements.begin(); it != replacements.end(); ++it)
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
		LogManager::getSingleton().stream() << "Cg high level output for " << getName() << ":\n" << hlSource;
		hlSource = HighLevelOutputFixer(hlSource, mParametersMap, mSamplerRegisterMap, 
			mSelectedCgProfile == CG_PROFILE_GLSLV || mSelectedCgProfile == CG_PROFILE_GLSLF || 
			mSelectedCgProfile == CG_PROFILE_GLSLG).output;
		LogManager::getSingleton().stream() << "Cleaned high level output for " << getName() << ":\n" << hlSource;
	}


	//-----------------------------------------------------------------------
	void CgProgram::loadHighLevelSafe()
	{
		OGRE_LOCK_AUTO_MUTEX;
		if (this->isSupported())
			loadHighLevel();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr CgProgram::createParameters()
	{
		loadHighLevelSafe();
		if (!mDelegate.isNull())
			return mDelegate->createParameters();
		else
			return HighLevelGpuProgram::createParameters();
	}
	//-----------------------------------------------------------------------
	GpuProgram* CgProgram::_getBindingDelegate()
	{
		if (!mDelegate.isNull())
			return mDelegate->_getBindingDelegate();
		else
			return HighLevelGpuProgram::_getBindingDelegate();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isSkeletalAnimationIncluded(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->isSkeletalAnimationIncluded();
		else
			return HighLevelGpuProgram::isSkeletalAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isMorphAnimationIncluded(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->isMorphAnimationIncluded();
		else
			return HighLevelGpuProgram::isMorphAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isPoseAnimationIncluded(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->isPoseAnimationIncluded();
		else
			return HighLevelGpuProgram::isPoseAnimationIncluded();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::isVertexTextureFetchRequired(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->isVertexTextureFetchRequired();
		else
			return HighLevelGpuProgram::isVertexTextureFetchRequired();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr CgProgram::getDefaultParameters(void)
	{
		loadHighLevelSafe();
		if (!mDelegate.isNull())
			return mDelegate->getDefaultParameters();
		else
			return HighLevelGpuProgram::getDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::hasDefaultParameters(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->hasDefaultParameters();
		else
			return HighLevelGpuProgram::hasDefaultParameters();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassSurfaceAndLightStates(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->getPassSurfaceAndLightStates();
		else
			return HighLevelGpuProgram::getPassSurfaceAndLightStates();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassFogStates(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->getPassFogStates();
		else
			return HighLevelGpuProgram::getPassFogStates();
	}
	//-----------------------------------------------------------------------
	bool CgProgram::getPassTransformStates(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->getPassTransformStates();
		else
		{
			return true; /* CG uses MVP matrix when -posinv argument passed */
		}
	}
	//-----------------------------------------------------------------------
	bool CgProgram::hasCompileError(void) const
	{
		if (!mDelegate.isNull())
			return mDelegate->hasCompileError();
		else
			return HighLevelGpuProgram::hasCompileError();
	}
	//-----------------------------------------------------------------------
	void CgProgram::resetCompileError(void)
	{
		if (!mDelegate.isNull())
			mDelegate->resetCompileError();
		else
			HighLevelGpuProgram::resetCompileError();
	}
	//-----------------------------------------------------------------------
	size_t CgProgram::getSize() const
	{
		if (!mDelegate.isNull())
			return mDelegate->getSize();
		else
			return HighLevelGpuProgram::getSize();
	}
	//-----------------------------------------------------------------------
	void CgProgram::touch()
	{
		if (!mDelegate.isNull())
			mDelegate->touch();
		else
			HighLevelGpuProgram::touch();
	}


	//-----------------------------------------------------------------------
	void CgProgram::unloadHighLevelImpl(void)
	{
	}
	//-----------------------------------------------------------------------
	void CgProgram::buildConstantDefinitions() const
	{
		// Derive parameter names from Cg
		createParameterMappingStructures(true);

		if ( mProgramString.empty() )
			return;
				
		mConstantDefs->floatBufferSize = mFloatLogicalToPhysical->bufferSize;
		mConstantDefs->intBufferSize = mIntLogicalToPhysical->bufferSize;

		GpuConstantDefinitionMap::const_iterator iter = mParametersMap.begin();
		GpuConstantDefinitionMap::const_iterator iterE = mParametersMap.end();
		for (; iter != iterE ; iter++)
		{
			const String & paramName = iter->first;
			GpuConstantDefinition def = iter->second;

			mConstantDefs->map.insert(GpuConstantDefinitionMap::value_type(iter->first, iter->second));

			// Record logical / physical mapping
			if (def.isFloat())
			{
							OGRE_LOCK_MUTEX(mFloatLogicalToPhysical->mutex);
				mFloatLogicalToPhysical->map.insert(
					GpuLogicalIndexUseMap::value_type(def.logicalIndex, 
						GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL)));
				mFloatLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
			}
			else
			{
							OGRE_LOCK_MUTEX(mIntLogicalToPhysical->mutex);
				mIntLogicalToPhysical->map.insert(
					GpuLogicalIndexUseMap::value_type(def.logicalIndex, 
						GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL)));
				mIntLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
			}

			// Deal with array indexing
			mConstantDefs->generateConstantDefinitionArrayEntries(paramName, def);
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
						if (def.isFloat())
						{
							def.physicalIndex = mFloatLogicalToPhysical->bufferSize;
						}
						else
						{
							def.physicalIndex = mIntLogicalToPhysical->bufferSize;
						}
					}

					def.logicalIndex = logicalIndex;
					if( mParametersMap.find(paramName) == mParametersMap.end())
					{
						mParametersMap.insert(GpuConstantDefinitionMap::value_type(paramName, def));
						mParametersMapSizeAsBuffer += sizeof(size_t);
						mParametersMapSizeAsBuffer += paramName.size();
						mParametersMapSizeAsBuffer += sizeof(GpuConstantDefinition);
					}

					// Record logical / physical mapping
					if (def.isFloat())
					{
											OGRE_LOCK_MUTEX(mFloatLogicalToPhysical->mutex);
						mFloatLogicalToPhysical->map.insert(
							GpuLogicalIndexUseMap::value_type(def.logicalIndex, 
								GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL)));
						mFloatLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
					}
					else
					{
											OGRE_LOCK_MUTEX(mIntLogicalToPhysical->mutex);
						mIntLogicalToPhysical->map.insert(
							GpuLogicalIndexUseMap::value_type(def.logicalIndex, 
								GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize, GPV_GLOBAL)));
						mIntLogicalToPhysical->bufferSize += def.arraySize * def.elementSize;
					}

					break;
				}                   
			}

			// now handle uniform samplers. This is needed to fix their register positions
			// if delegating to a GLSL shader.
			if (!mDelegate.isNull() && cgGetParameterVariability(parameter) == CG_UNIFORM && (
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

			dict->addParameter(ParameterDef("entry_point", 
				"The entry point for the Cg program.",
				PT_STRING),&msCmdEntryPoint);
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
		if (!mDelegate.isNull())
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
	String CgProgram::resolveCgIncludes(const String& inSource, Resource* resourceBeingLoaded, const String& fileName)
	{
		String outSource;
		// output will be at least this big
		outSource.reserve(inSource.length());

		size_t startMarker = 0;
		size_t i = inSource.find("#include");
		while (i != String::npos)
		{
			size_t includePos = i;
			size_t afterIncludePos = includePos + 8;
			size_t newLineBefore = inSource.rfind("\n", includePos);

			// check we're not in a comment
			size_t lineCommentIt = inSource.rfind("//", includePos);
			if (lineCommentIt != String::npos)
			{
				if (newLineBefore == String::npos || lineCommentIt > newLineBefore)
				{
					// commented
					i = inSource.find("#include", afterIncludePos);
					continue;
				}

			}
			size_t blockCommentIt = inSource.rfind("/*", includePos);
			if (blockCommentIt != String::npos)
			{
				size_t closeCommentIt = inSource.rfind("*/", includePos);
				if (closeCommentIt == String::npos || closeCommentIt < blockCommentIt)
				{
					// commented
					i = inSource.find("#include", afterIncludePos);
					continue;
				}

			}

			// find following newline (or EOF)
			size_t newLineAfter = inSource.find("\n", afterIncludePos);
			// find include file string container
			String endDelimeter = "\"";
			size_t startIt = inSource.find("\"", afterIncludePos);
			if (startIt == String::npos || startIt > newLineAfter)
			{
				// try <>
				startIt = inSource.find("<", afterIncludePos);
				if (startIt == String::npos || startIt > newLineAfter)
				{
					OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
						"Badly formed #include directive (expected \" or <) in file "
						+ fileName + ": " + inSource.substr(includePos, newLineAfter-includePos),
						"CgProgram::preprocessor");
				}
				else
				{
					endDelimeter = ">";
				}
			}
			size_t endIt = inSource.find(endDelimeter, startIt+1);
			if (endIt == String::npos || endIt <= startIt)
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					"Badly formed #include directive (expected " + endDelimeter + ") in file "
					+ fileName + ": " + inSource.substr(includePos, newLineAfter-includePos),
					"CgProgram::preprocessor");
			}

			// extract filename
			String filename(inSource.substr(startIt+1, endIt-startIt-1));

			// open included file
			DataStreamPtr resource = ResourceGroupManager::getSingleton().
				openResource(filename, resourceBeingLoaded->getGroup(), true, resourceBeingLoaded);

			// replace entire include directive line
			// copy up to just before include
			if (newLineBefore != String::npos && newLineBefore >= startMarker)
				outSource.append(inSource.substr(startMarker, newLineBefore-startMarker+1));

			size_t lineCount = 0;
			size_t lineCountPos = 0;
			
			// Count the line number of #include statement
			lineCountPos = outSource.find('\n');
			while(lineCountPos != String::npos)
			{
				lineCountPos = outSource.find('\n', lineCountPos+1);
				lineCount++;
			}

			// Add #line to the start of the included file to correct the line count
			outSource.append("#line 1 \"" + filename + "\"\n");

			outSource.append(resource->getAsString());

			// Add #line to the end of the included file to correct the line count
			outSource.append("\n#line " + Ogre::StringConverter::toString(lineCount) + 
				"\"" + fileName + "\"\n");

			startMarker = newLineAfter;

			if (startMarker != String::npos)
				i = inSource.find("#include", startMarker);
			else
				i = String::npos;

		}
		// copy any remaining characters
		outSource.append(inSource.substr(startMarker));

		return outSource;
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
	String CgProgram::CmdEntryPoint::doGet(const void *target) const
	{
		return static_cast<const CgProgram*>(target)->getEntryPoint();
	}
	void CgProgram::CmdEntryPoint::doSet(void *target, const String& val)
	{
		static_cast<CgProgram*>(target)->setEntryPoint(val);
	}
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
		return static_cast<const CgProgram*>(target)->getCompileArguments();
	}
	void CgProgram::CmdArgs::doSet(void *target, const String& val)
	{
		static_cast<CgProgram*>(target)->setCompileArguments(val);
	}

}
