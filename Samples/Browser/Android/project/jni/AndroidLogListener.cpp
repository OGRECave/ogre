#include "AndroidLogListener.h"

#include <android/log.h>

#define  LOG_TAG    "ogre"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace Ogre;

AndroidLogListener::AndroidLogListener()
{
}

void AndroidLogListener::messageLogged(const String& message, LogMessageLevel lml, bool maskDebug, const String &logName)
{
	if(lml < Ogre::LML_CRITICAL)
	{
		LOGI(message.c_str());
	}
	else
	{
		LOGE(message.c_str());
	}
}