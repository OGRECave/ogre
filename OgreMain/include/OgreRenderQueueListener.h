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
#ifndef __RenderQueueListener_H__
#define __RenderQueueListener_H__

#include "OgrePrerequisites.h"
#include "OgreRenderQueue.h"

namespace Ogre {

    /** Abstract interface which classes must implement if they wish to receive
        events from the render queue. 
    @remarks
        The OGRE render queue is divided into several queue groups, as defined by
        uint8. A class may implement this interface, and register itself
        as a listener by calling SceneManager::addRenderQueueListener. After doing so,
        the class will receive an event before and after each queue group is sent to 
        the rendering system.
    @par
        The event listeners have an option to make a queue either be skipped, or to repeat.
        Note that if multiple listeners are registered, the one registered last has the final
        say, although options set by previous listeners will not be changed if the latest
        does not express a preference.
    */
    class _OgreExport RenderQueueListener
    {
    public:
		virtual ~RenderQueueListener() {}
        /** Event raised before a queue group is rendered. 
        @remarks
            This method is called by the SceneManager before each queue group is
            rendered. 
        @param queueGroupId The id of the queue group which is about to be rendered
		@param invocation Name of the invocation which is causing this to be 
			called (@see RenderQueueInvocation)
		@param skipThisInvocation A boolean passed by reference which is by default set to 
			false. If the event sets this to true, the queue will be skipped and not
			rendered. Note that in this case the renderQueueEnded event will not be raised
			for this queue group.
        */
        virtual void renderQueueStarted(uint8 queueGroupId, const String& invocation, 
			bool& skipThisInvocation) = 0;

        /** Event raised after a queue group is rendered. 
        @remarks
            This method is called by the SceneManager after each queue group is
            rendered. 
        @param queueGroupId The id of the queue group which has just been rendered
		@param invocation Name of the invocation which is causing this to be 
			called (@see RenderQueueInvocation)
		@param repeatThisInvocation A boolean passed by reference which is by default set to 
			false. If the event sets this to true, the queue which has just been
			rendered will be repeated, and the renderQueueStarted and renderQueueEnded
			events will also be fired for it again.
        */
        virtual void renderQueueEnded(uint8 queueGroupId, const String& invocation, 
			bool& repeatThisInvocation) = 0;
    };

}

#endif

