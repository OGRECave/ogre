/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "MaterialGenerator.h"

#include "OgreStringConverter.h"
#include "OgreException.h"

#include "OgrePass.h"
#include "OgreTechnique.h"

#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

using namespace Ogre;

MaterialGenerator::MaterialGenerator():
	vsMask(0), fsMask(0), matMask(0), mImpl(0)
{
}
MaterialGenerator::~MaterialGenerator()
{
	delete mImpl;
}

const MaterialPtr &MaterialGenerator::getMaterial(Perm permutation)
{
	/// Check input validity
	size_t totalBits = bitNames.size();
	size_t totalPerms = 1<<totalBits;
	assert(permutation < totalPerms);
    (void)totalPerms; // Silence warning

	/// Check if material/shader permutation already was generated
	MaterialMap::iterator i = mMaterials.find(permutation);
	if(i != mMaterials.end())
	{
		return i->second;
	}
	else
	{
		/// Create it
		MaterialPtr templ = getTemplateMaterial(permutation & matMask);
		GpuProgramPtr vs = getVertexShader(permutation & vsMask);
		GpuProgramPtr fs = getFragmentShader(permutation & fsMask);
		
		/// Create material name
		String name=materialBaseName;
		for(size_t bit=0; bit<totalBits; ++bit)
			if(permutation & (1<<bit))
				name += bitNames[bit];

		std::cerr << name << " " << vs->getName() << " " << fs->getName() << std::endl;
		/// Create material from template, and set shaders
		MaterialPtr mat = templ->clone(name);
		Technique *tech = mat->getTechnique(0);
		Pass *pass = tech->getPass(0);
		pass->setFragmentProgram(fs->getName());
		pass->setVertexProgram(vs->getName());
	
		/// And store it
		mMaterials[permutation] = mat;
		return mMaterials[permutation];
	}
}

const GpuProgramPtr &MaterialGenerator::getVertexShader(Perm permutation)
{
	ProgramMap::iterator i = mVs.find(permutation);
	if(i != mVs.end())
	{
		return i->second;
	}
	else
	{
		/// Create it
		mVs[permutation] = mImpl->generateVertexShader(permutation);
		return mVs[permutation];
	}
}

const GpuProgramPtr &MaterialGenerator::getFragmentShader(Perm permutation)
{
	ProgramMap::iterator i = mFs.find(permutation);
	if(i != mFs.end())
	{
		return i->second;
	}
	else
	{
		/// Create it
		mFs[permutation] = mImpl->generateFragmentShader(permutation);
		return mFs[permutation];
	}
}

const MaterialPtr &MaterialGenerator::getTemplateMaterial(Perm permutation)
{
	MaterialMap::iterator i = mTemplateMat.find(permutation);
	if(i != mTemplateMat.end())
	{
		return i->second;
	}
	else
	{
		/// Create it
		mTemplateMat[permutation] = mImpl->generateTemplateMaterial(permutation);
		return mTemplateMat[permutation];
	}
}

MaterialGenerator::Impl::~Impl()
{
}

