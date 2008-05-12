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



#include "Ogre.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;
using namespace Ogre;


void help(void)
{
    // Print help message
    cout << endl << "rcapsdump: Queries GPU capabilities and dumps them into .rendercaps files" << endl;

    cout << endl << "Usage: rcapsdump" << endl;
}



void setUpGLRenderSystemOptions(Ogre::RenderSystem* rs)
{
	using namespace Ogre;
	ConfigOptionMap options = rs->getConfigOptions();
	// set default options
	// this should work on every semi-normal system
	rs->setConfigOption(String("Colour Depth"), String("32"));
	rs->setConfigOption(String("FSAA"), String("0"));
	rs->setConfigOption(String("Full Screen"), String("No"));
	rs->setConfigOption(String("VSync"), String("No"));
	rs->setConfigOption(String("Video Mode"), String("800 x 600"));
	
	// use the best RTT
	ConfigOption optionRTT = options["RTT Preferred Mode"];
	
	if(find(optionRTT.possibleValues.begin(), optionRTT.possibleValues.end(), "FBO") != optionRTT.possibleValues.end())
	{
		rs->setConfigOption(String("RTT Preferred Mode"), String("FBO"));
	}
	else if(find(optionRTT.possibleValues.begin(), optionRTT.possibleValues.end(), "PBuffer") != optionRTT.possibleValues.end())
	{
		rs->setConfigOption(String("RTT Preferred Mode"), String("PBuffer"));
	}
	else
		rs->setConfigOption(String("RTT Preferred Mode"), String("Copy"));
}


void setUpD3D9RenderSystemOptions(Ogre::RenderSystem* rs)
{
	using namespace Ogre;
	ConfigOptionMap options = rs->getConfigOptions();
	// set default options
	// this should work on every semi-normal system
	rs->setConfigOption(String("Anti aliasing"), String("None"));
	rs->setConfigOption(String("Full Screen"), String("No"));
	rs->setConfigOption(String("VSync"), String("No"));
	rs->setConfigOption(String("Video Mode"), String("800 x 600 @ 32-bit colour"));
	
	// pick first available device
	ConfigOption optionDevice = options["Rendering Device"];

	rs->setConfigOption(optionDevice.name, optionDevice.currentValue);
}


int main(int numargs, char** args)
{
    if (numargs != 1)
    {
        help();
        return -1;
    }
    
    RenderSystemCapabilities* glCaps = 0;
    RenderSystemCapabilities* d3d9Caps = 0;
    
    RenderSystemCapabilitiesSerializer serializer;
    
    // query openGL for caps if available
    Root* root = new Root("plugins.cfg");
    RenderSystem* rsGL = root->getRenderSystemByName("OpenGL Rendering Subsystem");
    if(rsGL)
    {
        setUpGLRenderSystemOptions(rsGL);
		root->setRenderSystem(rsGL);
		root->initialise(true, "OGRE rcapsdump GL Window");
		glCaps = const_cast<RenderSystemCapabilities *>(rsGL->getCapabilities());	
    }
    if(glCaps)
    {
        serializer.writeScript(glCaps, glCaps->getDeviceName(), "rcapsdump_gl.rendercaps");
    }
    
    delete root;
    
    // query D3D9 for caps if available
    root = new Root("plugins.cfg");
    RenderSystem* rsD3D9 = root->getRenderSystemByName("Direct3D9 Rendering Subsystem");
    if(rsD3D9)
    {
        setUpD3D9RenderSystemOptions(rsD3D9);
		root->setRenderSystem(rsD3D9);
		root->initialise(true, "OGRE rcapsdump D3D9 Window");
		d3d9Caps = const_cast<RenderSystemCapabilities *>(rsD3D9->getCapabilities());
    }
    if(d3d9Caps)
    {
        serializer.writeScript(d3d9Caps, d3d9Caps->getDeviceName(), "rcapsdump_d3d9.rendercaps");
    }

    delete root;
  
    return 0;

}

