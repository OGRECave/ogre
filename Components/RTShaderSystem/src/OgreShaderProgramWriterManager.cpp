/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreShaderProgramWriterManager.h"

namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::ProgramWriterManager* Singleton<RTShader::ProgramWriterManager>::ms_Singleton = 0;

namespace RTShader {
//-----------------------------------------------------------------------
ProgramWriterManager* ProgramWriterManager::getSingletonPtr(void)
{
	assert( ms_Singleton );  
	return ms_Singleton;
}
//-----------------------------------------------------------------------
ProgramWriterManager& ProgramWriterManager::getSingleton(void)
{  
	assert( ms_Singleton );  
	return ( *ms_Singleton );  
}
//-----------------------------------------------------------------------
ProgramWriterManager::ProgramWriterManager()
{

}
//-----------------------------------------------------------------------
ProgramWriterManager::~ProgramWriterManager()
{

}
//-----------------------------------------------------------------------
void ProgramWriterManager::addFactory(ProgramWriterFactory* factory)
{
	mFactories[factory->getTargetLanguage()] = factory;
}
//-----------------------------------------------------------------------
void ProgramWriterManager::removeFactory(ProgramWriterFactory* factory)
{
	// Remove only if equal to registered one, since it might overridden
	// by other plugins
	FactoryMap::iterator it = mFactories.find(factory->getTargetLanguage());
	if (it != mFactories.end() && it->second == factory)
	{
		mFactories.erase(it);
	}
}
//-----------------------------------------------------------------------
bool ProgramWriterManager::isLanguageSupported(const String& lang)
{
	FactoryMap::iterator i = mFactories.find(lang);

	return i != mFactories.end();
}
//-----------------------------------------------------------------------
ProgramWriter* ProgramWriterManager::createProgramWriter( const String& language)
{
	FactoryMap::iterator it = mFactories.find(language);

	if (it != mFactories.end())
	{
		return (it->second)->create();
	}
	
	OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, 
		"Could not create ShaderProgramWriter unknown language ", 
		"ShaderProgramWriterManager::createProgramWriter" );
}

}
}
