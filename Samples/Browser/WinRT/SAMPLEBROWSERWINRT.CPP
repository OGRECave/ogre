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
}

void SampleBrowserWinRT::SetWindow(CoreWindow^ window)
{
	window->PointerCursor = nullptr;

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
