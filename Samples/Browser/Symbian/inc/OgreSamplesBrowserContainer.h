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
/*
 * ==============================================================================
 *  Name        : OgreSamplesBrowserContainer.h
 *  Part of     : OgreSamplesBrowser
 *
 * ==============================================================================
 */

#ifndef OGRE_SAMPLE_BROWSER_CONTAINER_H
#define OGRE_SAMPLE_BROWSER_CONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <akndef.h>
#include <aknappui.h> 
#include <AknUtils.h> // Pen support. 

#include "SampleBrowser.h"

// CLASS DECLARATION



/**
 * Container control class that handles the OpenGL ES initialization and deinitializations.
 * Also uses the COgreSamplesBrowser class to do the actual OpenGL ES rendering.
 */
class COgreSamplesBrowserContainer : public CCoeControl, MCoeControlObserver
    {
    public: // Constructors and destructor

        /**
         * Second phase constructor that can call methods that may leave.
         * Initializes the OpenGL ES for rendering to the window surface.
         * @param aRect Screen rectangle for container.
         */
        void ConstructL(const ::TRect& aRect, CAknAppUi* aAppUi);

        /**
         * Destructor. Destroys the CPeriodic, COgreSamplesBrowser and uninitializes OpenGL ES.
         */
        virtual ~COgreSamplesBrowserContainer();

    public: // New functions

        /**
         * Callback function for the CPeriodic. Calculates the current frame, keeps the background
         * light from turning off and orders the COgreSamplesBrowser to do the rendering for each frame.
         *@param aInstance Pointer to this instance of COgreSamplesBrowserContainer.
         */
        static TInt DrawCallBack( TAny* aInstance );

        /**
         * Returns the current system time.
         * @return System time.
         */
        static TTime GetTimeTick();
        
        void HandlePointerEventL( const TPointerEvent& aPointerEvent );


    private: // Functions from base classes

        /**
         * Method from CoeControl that gets called when the display size changes.
         * If OpenGL has been initialized, notifies the renderer class that the screen
         * size has changed.
         */
        void SizeChanged();

         /**
          * Handles a change to the control's resources. This method
          * reacts to the KEikDynamicLayoutVariantSwitch event (that notifies of
          * screen size change) by calling the SetExtentToWholeScreen() again so that
          * this control fills the new screen size. This will then trigger a call to the
          * SizeChanged() method.
          * @param aType Message UID value, only KEikDynamicLayoutVariantSwitch is handled by this method.
          */
        void HandleResourceChange(TInt aType);

        /**
         * Method from CoeControl. Does nothing in this implementation.
         */
        TInt CountComponentControls() const;

        /**
         * Method from CCoeControl. Does nothing in this implementation.
         */
        CCoeControl* ComponentControl(TInt aIndex) const;

        /**
         * Method from CCoeControl. Does nothing in this implementation.
         * All rendering is done in the DrawCallBack() method.
         */
        void Draw(const ::TRect& aRect) const;

        /**
         * Method from MCoeControlObserver that handles an event from the observed control.
         * Does nothing in this implementation.
         * @param aControl   Control changing its state.
         * @param aEventType Type of the control event.
         */
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
        /**
        * From CCoeControl  Handle key events. When a key event occurs, 
        *                   CONE calls this function for each control on the control stack, 
        *                   until one of them returns EKeyWasConsumed to indicate that it processed the key event.  
        * @param aKeyEvent  The key event.
        * @param aType      The type of the event: EEventKey, EEventKeyUp or EEventKeyDown.
        * @return           Indicates whether or not the key event was used by this control.
        */
        IMPORT_C TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType);

    private: //data

        /** Active object that is the timing source for the animation. */
        CPeriodic*  iPeriodic;

        /** Time when the example started running (rendering frames). */
        TUint iStartTime;
        /** Time when the previous frame was rendered. */
        float iLastFrameTime;
        /** Counts the number of frames rendered. */
        TUint iFrame;

/** Application UI class for querying the client rectangle size. */ 
        CAknAppUi* iAppUi;
        
		//! The state of the mouse
		OIS::MouseState mState;
        
        bool _doMouseClick( int mouseButton, bool isDown );
        bool _doKeyPress( int key, unsigned int txt, bool isDown );


    public:  //data

        /** Used in DrawCallBack() method to do the actual OpenGL ES rendering.  */
        OgreBites::SampleBrowser* iOgreSamplesBrowser;
    };

#endif

// End of File
