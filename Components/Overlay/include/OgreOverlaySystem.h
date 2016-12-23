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

#ifndef __OverlaySystem_H__
#define __OverlaySystem_H__

#include "OgreOverlayPrerequisites.h"
#include "OgreRenderQueueListener.h"
#include "OgreRenderSystem.h"

#if OGRE_PROFILING
#include "OgreOverlayProfileSessionListener.h"
#endif

namespace Ogre {
    class OverlayManager;
    class FontManager;

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Overlays
    *  @{
    */
    /** This class simplify initialization / finalization of the overlay system. 
        OGRE root did this steps before the overlay system transformed into a component.
    @remarks
        Before you create a concrete instance of the OverlaySystem the OGRE::Root must be created
        but not initialized. In the ctor all relevant systems are created and registered. The dtor
        must be called before you delete OGRE::Root.
        To make the overlays visible (= render into your viewports) you have to register this
        instance as a RenderQueueListener in your scenemanager(s).
    */
    class _OgreOverlayExport OverlaySystem
        : public OverlayAlloc
        , public Ogre::RenderQueueListener
        , public Ogre::RenderSystem::Listener
    {
    public:
        OverlaySystem();
        virtual ~OverlaySystem();

        /// @see RenderQueueListener
        virtual void renderQueueStarted(uint8 queueGroupId, const String& invocation, 
            bool& skipThisInvocation);

        /// @see RenderSystem::Listener
        virtual void eventOccurred(const String& eventName, const NameValuePairList* parameters);

    private:
        OverlayManager* mOverlayManager;
        FontManager* mFontManager;

#if OGRE_PROFILING
        OverlayProfileSessionListener* mProfileListener;
#endif
    };

}
#endif
