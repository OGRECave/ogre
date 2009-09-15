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

/***************************************************************************
OgreExternalTextureSource.cpp  -  
	Implementation of texture controller class

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/

#include "OgreStableHeaders.h"
#include "OgreExternalTextureSource.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre
{
	//String interface commands for setting some basic commands
	ExternalTextureSource::CmdInputFileName ExternalTextureSource::msCmdInputFile;
	ExternalTextureSource::CmdFPS			ExternalTextureSource::msCmdFramesPerSecond;
	ExternalTextureSource::CmdPlayMode		ExternalTextureSource::msCmdPlayMode;
	ExternalTextureSource::CmdTecPassState	ExternalTextureSource::msCmdTecPassState;

	//---------------------------------------------------------------------------------------//

	ExternalTextureSource::ExternalTextureSource()
	{
		mInputFileName = "None";
		mDictionaryName = "NotAssigned";
		mUpdateEveryFrame = false;
		mFramesPerSecond = 24;
		mMode = TextureEffectPause;
	}

	//---------------------------------------------------------------------------------------//

	void ExternalTextureSource::addBaseParams()
	{
		if( mDictionaryName == "NotAssigned" )
            OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, 
                "Plugin " + mPlugInName + 
				" needs to override default mDictionaryName", 
                "ExternalTextureSource::addBaseParams");

		//Create Dictionary Here
        if (createParamDictionary( mDictionaryName ))
		{
	        ParamDictionary* dict = getParamDictionary();
			
			dict->addParameter(ParameterDef("filename", 
			    "A source for the texture effect (only certain plugins require this)"
				, PT_STRING),
				&ExternalTextureSource::msCmdInputFile);
			dict->addParameter(ParameterDef("frames_per_second", 
			    "How fast should playback be (only certain plugins use this)"
				, PT_INT),
				&ExternalTextureSource::msCmdFramesPerSecond);
			dict->addParameter(ParameterDef("play_mode", 
			    "How the playback starts(only certain plugins use this)"
				, PT_STRING),
				&ExternalTextureSource::msCmdPlayMode);
			dict->addParameter(ParameterDef("set_T_P_S", 
			    "Set the technique, pass, and state level of this texture_unit (eg. 0 0 0 )"
				, PT_STRING),
				&ExternalTextureSource::msCmdTecPassState);
		}
	}

	//---------------------------------------------------------------------------------------//
	//*** String Interface Command Class Definitions *****************************************/
	String ExternalTextureSource::CmdInputFileName::doGet(const void* target) const
	{
		return static_cast<const ExternalTextureSource*>(target)->getInputName();
	}
	void ExternalTextureSource::CmdInputFileName::doSet(void* target, const String& val)
	{
		static_cast<ExternalTextureSource*>(target)->setInputName( val );
	}
	
	//------------------------------------------------------------------------------//
	String ExternalTextureSource::CmdFPS::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const ExternalTextureSource*>(target)->getFPS() );
	}
	void ExternalTextureSource::CmdFPS::doSet(void* target, const String& val)
	{
		static_cast<ExternalTextureSource*>(target)->setFPS(StringConverter::parseInt(val));
	}
	//------------------------------------------------------------------------------//
	String ExternalTextureSource::CmdPlayMode::doGet(const void* target) const
	{
		eTexturePlayMode eMode = static_cast<const ExternalTextureSource*>(target)->getPlayMode();
		String val;

		switch(eMode)
		{
		case TextureEffectPlay_ASAP:
			val = "play";
			break;
		case TextureEffectPlay_Looping: 
			val = "loop";
			break;
		case TextureEffectPause:
			val = "pause";
			break;
		default: 
			val = "error"; 
			break;
		}

		return val;
	}
	void ExternalTextureSource::CmdPlayMode::doSet(void* target, const String& val)
	{
		eTexturePlayMode eMode = TextureEffectPause;

		if( val == "play" )
			eMode = TextureEffectPlay_ASAP;
		if( val == "loop" )
			eMode = TextureEffectPlay_Looping;
		if( val == "pause" )
			eMode = TextureEffectPause;

		static_cast<ExternalTextureSource*>(target)->setPlayMode( eMode );
	}

	//------------------------------------------------------------------------------//
	String ExternalTextureSource::CmdTecPassState::doGet(const void* target) const
	{
		int t = 0, p = 0, s = 0;

		static_cast<const ExternalTextureSource*>(target)->getTextureTecPassStateLevel(t, p, s);

		String ret = StringConverter::toString( t ) + " " 
					+ StringConverter::toString( p ) + " " 
					+ StringConverter::toString( s );
		
		return ret;			
	}

	void ExternalTextureSource::CmdTecPassState::doSet(void* target, const String& val)
	{
		int t = 0, p = 0, s = 0;

		StringVector vecparams = StringUtil::split(val, " \t");

		if( vecparams.size() == 3 )
		{
			t = StringConverter::parseInt( vecparams[0] );
			p = StringConverter::parseInt( vecparams[1] );
			s = StringConverter::parseInt( vecparams[2] );
		}
		else
		{
			LogManager::getSingleton().logMessage("Texture controller had problems extracting technique, pass, and state level... Default to 0, 0, 0");
			t = p = s = 0;
		}

		static_cast<ExternalTextureSource*>(target)->setTextureTecPassStateLevel(t,p,s);
	}
}

