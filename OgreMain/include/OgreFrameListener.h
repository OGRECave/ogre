/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#ifndef __FrameListener_H__
#define __FrameListener_H__


#include "OgrePrerequisites.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/
	/** Struct containing information about a frame event.
    */
    struct FrameEvent
    {
        /** Elapsed time in seconds since the last event.
            This gives you time between frame start & frame end,
            and between frame end and next frame start.
            @remarks
                This may not be the elapsed time but the average
                elapsed time between recently fired events.
        */
        Real timeSinceLastEvent;
        /** Elapsed time in seconds since the last event of the same type,
            i.e. time for a complete frame.
            @remarks
                This may not be the elapsed time but the average
                elapsed time between recently fired events of the same type.
        */
        Real timeSinceLastFrame;
    };


    /** A interface class defining a listener which can be used to receive
        notifications of frame events.
        @remarks
            A 'listener' is an interface designed to be called back when
            particular events are called. This class defines the
            interface relating to frame events. In order to receive
            notifications of frame events, you should create a subclass of
            FrameListener and override the methods for which you would like
            to customise the resulting processing. You should then call
            Root::addFrameListener passing an instance of this class.
            There is no limit to the number of frame listeners you can register,
            allowing you to register multiple listeners for different purposes.
            Note that a frame event occurs once for all rendering targets,
            not once per target.
    */
    class _OgreExport FrameListener
    {
        /*
        Note that this could have been an abstract class, but I made
        the explicit choice not to do this, because I wanted to give
        people the option of only implementing the methods they wanted,
        rather than having to create 'do nothing' implementations for
        those they weren't interested in. As such this class follows
        the 'Adapter' classes in Java rather than pure interfaces.
        */
    public:
        /** Called when a frame is about to begin rendering.
		@remarks
			This event happens before any render targets have begun updating. 
            @return
                True to go ahead, false to abort rendering and drop
                out of the rendering loop.
        */
        virtual bool frameStarted(const FrameEvent& evt)
        { (void)evt; return true; }
		
		/** Called after all render targets have had their rendering commands 
			issued, but before render windows have been asked to flip their 
			buffers over.
		@remarks
			The usefulness of this event comes from the fact that rendering 
			commands are queued for the GPU to process. These can take a little
			while to finish, and so while that is happening the CPU can be doing
			useful things. Once the request to 'flip buffers' happens, the thread
			requesting it will block until the GPU is ready, which can waste CPU
			cycles. Therefore, it is often a good idea to use this callback to 
			perform per-frame processing. Of course because the frame's rendering
			commands have already been issued, any changes you make will only
			take effect from the next frame, but in most cases that's not noticeable.
		@return
			True to continue rendering, false to drop out of the rendering loop.
		*/
		virtual bool frameRenderingQueued(const FrameEvent& evt)
                { (void)evt; return true; }

        /** Called just after a frame has been rendered.
		@remarks
			This event happens after all render targets have been fully updated
			and the buffers switched.
            @return
                True to continue with the next frame, false to drop
                out of the rendering loop.
        */
        virtual bool frameEnded(const FrameEvent& evt)
        { (void)evt; return true; }

		virtual ~FrameListener() {}
		
    };
	/** @} */
	/** @} */
}

#endif
