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

#include "OgreHlmsDatablock.h"
#include "OgreHlmsPropertyMap.h"
#include "OgreHlmsShaderCommon.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------------
	HlmsDatablock::HlmsDatablock(GpuProgramType type, PropertyMap* propertyMap) : mShaderType(type), mPropertyMap(propertyMap), mHash(0)
	{

	}
	//-----------------------------------------------------------------------------------
	ShaderTemplate* HlmsDatablock::getTemplate()
	{ 
		if (mHash == 0)
		{
			reload();
		}
		return &mTemplate;
	}
	//-----------------------------------------------------------------------------------
	void HlmsDatablock::setLanguage(const String& language)
	{
		if (mLanguage != language)
		{
			mHash = 0;
			mLanguage = language;
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsDatablock::setTemplateName(const String& tamplateName)
	{
		if (mTamplateName != tamplateName)
		{
			mHash = 0;
			mTamplateName = tamplateName;
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsDatablock::addProfile(const String& profile)
	{
		mHash = 0;
		mProfilesList.push_back(profile);
	}
	//-----------------------------------------------------------------------------------
	uint32 HlmsDatablock::getHash()
	{
		if (mHash == 0)
		{
			reload();
		}
		
		return mHash + mPropertyMap->getHash();
		
	}
	//-----------------------------------------------------------------------------------
	void HlmsDatablock::reload()
	{
	    StringStream ss;
	    ss << mTamplateName << FilePatterns[mShaderType] << "." << (mLanguage == "glsles" ? "glsl" : mLanguage) << "t";
		mTemplate.setTemplateFileName(ss.str());

		mHash = mTemplate.getHash() + mShaderType + calcHash(mLanguage) + calcHash(mProfilesList);
	}
	//-----------------------------------------------------------------------------------
}
