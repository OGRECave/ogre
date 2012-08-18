//
// BlankPage.xaml.cpp
// Implementation of the BlankPage.xaml class.
//

#include "pch.h"
#include "SampleBrowserWinRT.xaml.h"

using namespace SampleBrowserWinRTXaml;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

SampleBrowserWinRT::SampleBrowserWinRT()
{
	InitializeComponent();

	// setup timer at 60 FPS
	TimeSpan span;
	span.Duration = 10000000 / 60;   // convert to 100ns ticks
	m_timer = ref new Windows::UI::Xaml::DispatcherTimer;
	m_timer->Interval = span;
	m_timer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &SampleBrowserWinRT::DisplayCurrentFrame);

	contentView->SizeChanged += 
		ref new Windows::UI::Xaml::SizeChangedEventHandler(this, &SampleBrowserWinRT::OnContentViewSizeChanged);

	StartRendering();
}

void SampleBrowserWinRT::StartRendering()
{
	m_sampleBrowser.initAppForWinRT(this->contentView, m_inputManager.GetInputContext());
	m_sampleBrowser.initApp();

	m_timer->Start();
}

void SampleBrowserWinRT::StopRendering()
{
	m_timer->Stop();

	m_sampleBrowser.closeApp();
}

void  SampleBrowserWinRT::DisplayCurrentFrame(Platform::Object^ sender, Platform::Object^ e)
{
	if(m_windowClosed
	|| Ogre::Root::getSingleton().endRenderingQueued()
	|| !Ogre::Root::getSingleton().renderOneFrame(1.0f / 60.0f))
		StopRendering();
}

void SampleBrowserWinRT::OnContentViewSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
	m_sampleBrowser.windowMovedOrResized();
}


/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void SampleBrowserWinRT::OnNavigatedTo(NavigationEventArgs^ e)
{
}

void SampleBrowserWinRT::OnKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args)
{
	if(m_inputManager.OnKeyAction(args->Key, args->KeyStatus, true)) args->Handled = true;
}

void SampleBrowserWinRT::OnKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args)
{
	if(m_inputManager.OnKeyAction(args->Key, args->KeyStatus, false)) args->Handled = true;
}

//void SampleBrowserWinRT::OnCharacterReceived(Windows::UI::Core::CharacterReceivedEventArgs^ args)
//{
//    if(m_inputManager.OnCharacterReceived(args->KeyCode)) args->Handled = true;
//}

void SampleBrowserWinRT::OnPointerPressed(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->GetCurrentPoint(contentView), OgreBites::InputManagerWinRT::PointerPressed)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerReleased(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->GetCurrentPoint(contentView), OgreBites::InputManagerWinRT::PointerReleased)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerMoved(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->GetCurrentPoint(contentView), OgreBites::InputManagerWinRT::PointerMoved)) args->Handled = true;
}

void SampleBrowserWinRT::OnPointerWheelChanged(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args)
{
    if(m_inputManager.OnPointerAction(args->GetCurrentPoint(contentView), OgreBites::InputManagerWinRT::PointerWheelChanged)) args->Handled = true;
}
