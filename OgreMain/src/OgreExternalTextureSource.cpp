/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

