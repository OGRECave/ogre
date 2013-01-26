//
// BlankPage.xaml.h
// Declaration of the BlankPage class.
//

#pragma once

#include "pch.h"
#include "SampleBrowserWinRT.g.h"
#include "SampleBrowser.h"
#include "InputManagerWinRT.h"

namespace SampleBrowserWinRTXaml
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class SampleBrowserWinRT sealed
	{
	public:
		SampleBrowserWinRT();

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

		void StartRendering();
		void StopRendering();
        void DisplayCurrentFrame(Platform::Object^ sender, Platform::Object^ e);
		void OnContentViewSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

		virtual void OnKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args) override;
	    virtual void OnKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs^ args) override;
//		virtual void OnCharacterReceived(Windows::UI::Core::CharacterReceivedEventArgs^ args) override;
	    virtual void OnPointerPressed(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args) override;
		virtual void OnPointerReleased(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args) override;
	    virtual void OnPointerMoved(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args) override;
		virtual void OnPointerWheelChanged(Windows::UI::Xaml::Input::PointerRoutedEventArgs^ args) override;

	private:
		Windows::UI::Xaml::DispatcherTimer^ m_timer;
	    OgreBites::SampleBrowser m_sampleBrowser;
	    OgreBites::InputManagerWinRT m_inputManager;
	    bool m_windowClosed;
	};
}
