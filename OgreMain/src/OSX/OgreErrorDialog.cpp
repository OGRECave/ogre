#include "OgreErrorDialog.h"
#include <Carbon/Carbon.h>


using namespace Ogre;

ErrorDialog::ErrorDialog()
{
}

void ErrorDialog::display(const String& errorMessage, String logName)
{
	CFMutableStringRef errorStr = NULL;
	errorStr = CFStringCreateMutable( kCFAllocatorDefault, 0 );
	CFStringAppendCString( errorStr, "ERROR: ", kCFStringEncodingASCII );
	CFStringAppendCString( errorStr, errorMessage.c_str(), kCFStringEncodingASCII );
	DialogRef alertDialod;
	CreateStandardAlert( kAlertStopAlert, errorStr, NULL, NULL, &alertDialod );
	RunStandardAlert( alertDialod, NULL, NULL );
}
