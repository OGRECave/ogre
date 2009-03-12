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
/*
-----------------------------------------------------------------------------
Filename:    WindowEmbedding.cpp
Description: Stuff your windows full of OGRE
-----------------------------------------------------------------------------
*/
#include "Ogre.h"

using namespace Ogre;

void setupResources(void)
{
	// Load resource paths from config file
	ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}


//---------------------------------------------------------------------
// Windows Test
//---------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "windows.h"

RenderWindow* renderWindow = 0;
bool winActive = false;
bool winSizing = false;

LRESULT CALLBACK TestWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_CREATE)
	{
		return 0;
	}


	if (!renderWindow)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	switch( uMsg )
	{
	case WM_ACTIVATE:
		winActive = (LOWORD(wParam) != WA_INACTIVE);
		break;

	case WM_ENTERSIZEMOVE:
		winSizing = true;
		break;

	case WM_EXITSIZEMOVE:
		renderWindow->windowMovedOrResized();
		renderWindow->update();
		winSizing = false;
		break;

	case WM_MOVE:
	case WM_SIZE:
		if (!winSizing)
			renderWindow->windowMovedOrResized();
		break;

	case WM_GETMINMAXINFO:
		// Prevent the window from going smaller than some min size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;

	case WM_CLOSE:
		renderWindow->destroy(); // cleanup and call DestroyWindow
		PostQuitMessage(0); 
		return 0;
	case WM_PAINT:
		if (!winSizing)
		{
			renderWindow->update();
			return 0;
		}
		break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


INT WINAPI EmbeddedMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
	try 
	{
		// Create a new window

		// Style & size
		DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW;

		// Register the window class
		WNDCLASS wc = { 0, TestWndProc, 0, 0, hInst,
			LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "TestWnd" };
		RegisterClass(&wc);

		HWND hwnd = CreateWindow("TestWnd", "Test embedding", dwStyle,
				0, 0, 800, 600, 0, 0, hInst, 0);

		Root root("", "");

		root.loadPlugin("RenderSystem_GL");
		//root.loadPlugin("RenderSystem_Direct3D9");
		root.loadPlugin("Plugin_ParticleFX");
		root.loadPlugin("Plugin_CgProgramManager");

		// select first renderer & init with no window
		root.setRenderSystem(*(root.getAvailableRenderers().begin()));
		root.initialise(false);

		// create first window manually
		NameValuePairList options;
		options["externalWindowHandle"] = 
			StringConverter::toString((size_t)hwnd);

		renderWindow = root.createRenderWindow("embedded", 800, 600, false, &options);

		setupResources();
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		SceneManager *scene = root.createSceneManager(Ogre::ST_GENERIC, "default");


		Camera *cam = scene->createCamera("cam");


		Viewport* vp = renderWindow->addViewport(cam);
		vp->setBackgroundColour(Ogre::ColourValue(0.5, 0.5, 0.7));
		cam->setAutoAspectRatio(true);
		cam->setPosition(0,0,300);
		cam->setDirection(0,0,-1);

		Entity* e = scene->createEntity("1", "ogrehead.mesh");
		scene->getRootSceneNode()->createChildSceneNode()->attachObject(e);
		Light* l = scene->createLight("l");
		l->setPosition(300, 100, -100);

		// message loop
		MSG msg;
		while(GetMessage(&msg, NULL, 0, 0 ) != 0)
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 

	} 
	catch( Exception& e ) 
	{
		MessageBox( NULL, e.getFullDescription().c_str(), 
			"An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}


	return 0;
}

#endif




