/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#ifndef __RenderTargetListener_H__
#define __RenderTargetListener_H__


#include "OgrePrerequisites.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /** Struct containing information about a RenderTarget event.
    */
    struct RenderTargetEvent
    {
        /// The source of the event being raised
        RenderTarget* source;
    };

    /** Struct containing information about a RenderTarget Viewport-specific event.
    */
    struct RenderTargetViewportEvent
    {
        /// The source of the event being raised
        Viewport* source;
    };

    /** A interface class defining a listener which can be used to receive
        notifications of RenderTarget events.

            A 'listener' is an interface designed to be called back when
            particular events are called. This class defines the
            interface relating to RenderTarget events. In order to receive
            notifications of RenderTarget events, you should create a subclass of
            RenderTargetListener and override the methods for which you would like
            to customise the resulting processing. You should then call
            RenderTarget::addListener passing an instance of this class.
            There is no limit to the number of RenderTarget listeners you can register,
            allowing you to register multiple listeners for different purposes.

            RenderTarget events occur before and after the target is updated as a whole,
            and before and after each viewport on that target is updated. Each RenderTarget
            holds it's own set of listeners, but you can register the same listener on
            multiple render targets if you like since the event contains details of the
            originating RenderTarget.
    */
    class _OgreExport RenderTargetListener
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
        virtual ~RenderTargetListener() {}
        /** Called just before a RenderTarget is about to be rendered into.

            This event is raised just before any of the viewports on the target
            are rendered to. You can perform manual rendering operations here if
            you want, but please note that if the Viewport objects attached to this
            target are set up to clear the background, you will lose whatever you 
            render. If you want some kind of backdrop in this event
            you should turn off background clearing off on the viewports, and either
            clear the viewports yourself in this event handler before doing your rendering
            or just render over the top if you don't need to.
        */
        virtual void preRenderTargetUpdate(const RenderTargetEvent& evt)
        { (void)evt; }

        /** Called just after a RenderTarget has been rendered to.

            This event is called just after all the viewports attached to the target
            in question have been rendered to. You can perform your own manual rendering
            commands in this event handler if you like, these will be composited with
            the contents of the target already there (depending on the material settings 
            you use etc).
        */
        virtual void postRenderTargetUpdate(const RenderTargetEvent& evt)
        { (void)evt; }

        /** Called just before a Viewport on a RenderTarget is to be updated.

            This method is called before each viewport on the RenderTarget is
            rendered to. You can use this to perform per-viewport settings changes,
            such as showing / hiding particular overlays.
        */
        virtual void preViewportUpdate(const RenderTargetViewportEvent& evt)
        { (void)evt; }

        /** Called just after a Viewport on a RenderTarget is to be updated.

            This method is called after each viewport on the RenderTarget is
            rendered to. 
        */
        virtual void postViewportUpdate(const RenderTargetViewportEvent& evt)
        { (void)evt; }

        /** Called to notify listener that a Viewport has been added to the 
            target in question.
        */
        virtual void viewportAdded(const RenderTargetViewportEvent& evt)
                { (void)evt; }
        /** Called to notify listener that a Viewport has been removed from the 
            target in question.
        */
        virtual void viewportRemoved(const RenderTargetViewportEvent& evt)
                { (void)evt; }
    };
    /** @} */
    /** @} */
}

#endif
