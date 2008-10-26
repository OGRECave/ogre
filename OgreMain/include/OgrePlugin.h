/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __OGREPLUGIN_H__
#define __OGREPLUGIN_H__

#include "OgrePrerequisites.h"

namespace Ogre
{
	/** Class defining a generic OGRE plugin.
	@remarks
		OGRE is very plugin-oriented and you can customise much of its behaviour
		by registering new plugins, dynamically if you are using dynamic linking.
		This class abstracts the generic interface that all plugins must support.
		Within the implementations of this interface, the plugin must call other
		OGRE classes in order to register the detailed customisations it is
		providing, e.g. registering a new SceneManagerFactory, a new
		MovableObjectFactory, or a new RenderSystem.
	@par
		Plugins can be linked statically or dynamically. If they are linked
		dynamically (ie the plugin is in a DLL or Shared Object file), then you
		load the plugin by calling the Root::loadPlugin method (or some other
		mechanism which leads to that call, e.g. plugins.cfg), passing the name of
		the DLL. OGRE will then call a global init function on that DLL, and it
		will be expected to register one or more Plugin implementations using
		Root::installPlugin. The procedure is very similar if you use a static
		linked plugin, except that you simply instantiate the Plugin implementation
		yourself and pass it to Root::installPlugin.
	@note
		Lifecycle of a Plugin instance is very important. The Plugin instance must
		remain valid until the Plugin is uninstalled. Here are the things you
		must bear in  mind:
		<ul><li>If your plugin is in a DLL:
		<ul><li>Create the Plugin instance and call Root::installPlugin in dllStartPlugin</li>
		<li>Call Root::uninstallPlugin, then delete it in dllStopPlugin</li></ul>
		<li>If your plugin is statically linked in your app:
		<ul><li>Create the Plugin anytime you like</li>
		<li>Call Root::installPlugin any time whilst Root is valid</li>
		<li>Call Root::uninstallPlugin if you like so long as Root is valid. However,
			it will be done for you when Root is destroyed, so the Plugin instance must
			still be valid at that point if you haven't manually uninstalled it.</li></ul>
		</ul>
		The install and uninstall methods will be called when the plugin is
		installed or uninstalled. The initialise and shutdown will be called when
		there is a system initialisation or shutdown, e.g. when Root::initialise 
		or Root::shutdown are called.
	*/
	class _OgreExport Plugin : public PluginAlloc
	{
	public:
		Plugin() {}
		virtual ~Plugin() {}

		/** Get the name of the plugin. 
		@remarks An implementation must be supplied for this method to uniquely
			identify the plugin.
		*/
		virtual const String& getName() const = 0;

		/** Perform the plugin initial installation sequence. 
		@remarks An implementation must be supplied for this method. It must perform
		the startup tasks necessary to install any rendersystem customisations 
		or anything else that is not dependent on system initialisation, ie
		only dependent on the core of Ogre. It must not perform any
		operations that would create rendersystem-specific objects at this stage,
		that should be done in initialise().
		*/
		virtual void install() = 0;

		/** Perform any tasks the plugin needs to perform on full system
			initialisation.
		@remarks An implementation must be supplied for this method. It is called 
			just after the system is fully initialised (either after Root::initialise
			if a window is created then, or after the first window is created)
			and therefore all rendersystem functionality is available at this
			time. You can use this hook to create any resources which are 
			dependent on a rendersystem or have rendersystem-specific implementations.
		*/
		virtual void initialise() = 0;

		/** Perform any tasks the plugin needs to perform when the system is shut down.
		@remarks An implementation must be supplied for this method.
		This method is called just before key parts of the system are unloaded, 
		such as rendersystems being shut down. You should use this hook to free up 
		resources and decouple custom objects from the OGRE system, whilst all the
		instances of other plugins (e.g. rendersystems) still exist.
		*/
		virtual void shutdown() = 0;

		/** Perform the final plugin uninstallation sequence. 
		@remarks An implementation must be supplied for this method. It must perform
		the cleanup tasks which haven't already been performed in shutdown()
		(e.g. final deletion of custom instances, if you kept them around incase
		the system was reinitialised). At this stage you cannot be sure what other
		plugins are still loaded or active. It must therefore not perform any
		operations that would reference any rendersystem-specific objects - those
		should have been sorted out in the 'shutdown' method.
		*/
		virtual void uninstall() = 0;
	};

}

#endif


