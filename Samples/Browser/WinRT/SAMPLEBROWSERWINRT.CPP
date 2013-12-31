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
#include <directxmath.h>

#include "SampleBrowserWinRT.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

SampleBrowserWinRT::SampleBrowserWinRT() :
    m_windowClosed(false)
{
}

void SampleBrowserWinRT::Initialize(CoreApplicationView^ applicationView)
{
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &SampleBrowserWinRT::OnActivated);

    CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &SampleBrowserWinRT::OnSuspending);

    CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &SampleBrowserWinRT::OnResuming);

	m_inputManager.Initialize();
}

void SampleBrowserWinRT::SetWindow(CoreWindow^ window)
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_PC_APP)
	window->PointerCursor = nullptr;
#endif

    window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &SampleBrowserWinRT::OnWindowSizeChanged);

    window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &SampleBrowserWinRT::OnWindowClosed);

    window->CharacterReceived +=
        ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &SampleBrowserWinRT::OnCharacterReceived);

    window->KeyDown += 
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &SampleBrowserWinRT::OnKeyDown);

    window->KeyUp += 
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &SampleBrowserWinRT::OnKeyUp);

    window->PointerPressed += 
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &SampleBrowserWinRT::OnPointerPressed);

    window->PointerReleased +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &SampleBrowserWinRT::OnPointerReleased);

    window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &SampleBrowserWinRT::OnPointerMoved);

    window->PointerWheelChanged +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &SampleBrowserWinRT::OnPointerWheelChanged);

#if (OGRE_WINRT_TARGET_TYPE == PHONE)
	Windows::Phone::UI::Input::HardwareButtons::BackPressed +=
		ref new EventHandler<Windows::Phone::UI::Input::BackPressedEventArgs^>(this, &SampleBrowserWinRT::OnBackPressed);
#endif
}

void SampleBrowserWinRT::Load(Platform::String^ entryPoint)
{
}

void SampleBrowserWinRT::Run()
{
    BasicTimer^ timer = ref new BasicTimer();

    CoreWindow::GetForCurrentThread()->Activate();

    m_sampleBrowser.initAppForWinRT(CoreWindow::GetForCurrentThread(), m_inputManager.GetInputContext());
    m_sampleBrowser.initApp();

    while (!m_windowClosed && !Ogre::Root::getSingleton().endRenderingQueued())
    {
        timer->Update();
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		m_inputManager.Update(timer->Total, timer->Delta);
        if(!Ogre::Root::getSingleton().renderOneFrame(timer->Delta))
			break;
    }

    m_sampleBrowser.closeApp();
}

void SampleBrowserWinRT::Uninitialize()
{
}

void SampleBrowserWinRT::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    m_sampleBrowser.windowMovedOrResized();
}

void SampleBrowserWinRT::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	Ogre::Root::getSingleton().queueEndRendering();
    m_windowClosed = true;
}

void SampleBrowserWinRT::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void SampleBrowserWinRT::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    // Save application state after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations.
    // Be aware that a deferral may not be held indefinitely. After about five
    // seconds, the application will be forced to exit.
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

    // Insert your code here

    deferral->Complete();
}
 
void SampleBrowserWinRT::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
}

void SampleBrowserWinRT::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    if(m_inputManager.OnKeyAction(args->VirtualKey, args->KeyStatus, true)) args->Handled = true;
}

void SampleBrowserWinRT::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    if(m_inputManager.OnKeyAction(args->VirtualKey, args->KeyStatus, false)) args->Handled = true;
}

void SampleBrowserWinRT::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
    if(m_inputManager.OnCharacterReceived(args->KeyCode)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->CurrentPoint, OgreBites::InputManagerWinRT::PointerPressed)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->CurrentPoint, OgreBites::InputManagerWinRT::PointerReleased)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->CurrentPoint, OgreBites::InputManagerWinRT::PointerMoved)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->CurrentPoint, OgreBites::InputManagerWinRT::PointerWheelChanged)) args->Handled = true;
}

#if (OGRE_WINRT_TARGET_TYPE == PHONE)
void SampleBrowserWinRT::OnBackPressed( Platform::Object ^sender, Windows::Phone::UI::Input::BackPressedEventArgs^ args)
{
    // Send the back to main menu event unless we are already there in which case we exit all the way.
	Windows::System::VirtualKey vkey = Windows::System::VirtualKey::Back;
	Windows::UI::Core::CorePhysicalKeyStatus keystatus;
	keystatus.ScanCode = OIS::KC_ESCAPE; // Imitate the 'esc' being pressed.
	if(m_inputManager.OnKeyAction( vkey, keystatus, true)) args->Handled = true;
}
#endif

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new SampleBrowserWinRT();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto direct3DApplicationSource = ref new Direct3DApplicationSource();
    CoreApplication::Run(direct3DApplicationSource);
    return 0;
}
