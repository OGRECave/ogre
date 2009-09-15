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
#ifndef __RenderQueueListener_H__
#define __RenderQueueListener_H__

#include "OgrePrerequisites.h"
#include "OgreRenderQueue.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
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

		/** Event raised before all render queues are processed. 
		*/
		virtual void preRenderQueues() {}
		/** Event raised after all render queues are processed. 
		*/
		virtual void postRenderQueues() {}

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
			bool& skipThisInvocation) {}

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
			bool& repeatThisInvocation) {}
    };
	/** @} */
	/** @} */

}

#endif

