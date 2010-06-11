#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ogrewrapper.h"
#include <OIS.h>

#include <android/log.h>

#include "AndroidArchive.h"

#define  LOG_TAG    "libogresamplebrowser"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class AndroidMultiTouch : public OIS::MultiTouch
{
public:
	AndroidMultiTouch():OIS::MultiTouch("DWM", false, 0, 0){}
	
	/** @copydoc Object::setBuffered */
	virtual void setBuffered(bool buffered){}

	/** @copydoc Object::capture */
	virtual void capture(){}

	/** @copydoc Object::queryInterface */
	virtual OIS::Interface* queryInterface(OIS::Interface::IType type) {return 0;}

	/** @copydoc Object::_initialize */
	virtual void _initialize(){}
	
	OIS::MultiTouchState &getMultiTouchState(int i){
		while(i >= mStates.size()){
			OIS::MultiTouchState state;
			state.width = getRenderWindow()->getWidth();
			state.height = getRenderWindow()->getHeight();
			mStates.push_back(state);
		}
		return mStates[i];
	}
};

class AndroidKeyboard : public OIS::Keyboard
{
public:
	AndroidKeyboard():OIS::Keyboard("DWM", false, 1, 0){}
	
	/** @copydoc Object::setBuffered */
	virtual void setBuffered(bool buffered){}

	/** @copydoc Object::capture */
	virtual void capture(){}

	/** @copydoc Object::queryInterface */
	virtual OIS::Interface* queryInterface(OIS::Interface::IType type) {return 0;}

	/** @copydoc Object::_initialize */
	virtual void _initialize(){}
	
	virtual bool isKeyDown( OIS::KeyCode key ) const{
		return false;
	}
	
	virtual const std::string& getAsString( OIS::KeyCode kc ){
		static std::string defstr = "";
		return defstr;
	}
	
	virtual void copyKeyStates( char keys[256] ) const{
	
	}
};

#include "SampleBrowser.h"

static AndroidMultiTouch *g_multiTouch = 0;
static AndroidKeyboard *g_keyboard = 0;
static OgreBites::SampleBrowser *g_browser = 0;
static bool g_rootInit = false;
static int g_xOffset = 0, g_yOffset = 0;
static Ogre::AndroidArchiveFactory *g_archiveFactory = 0;
static Ogre::map<Ogre::String,Ogre::String>::type g_resourceMap;

static void injectTouchEvent(int pointerId, int action, float x, float y){
	if(g_browser){
		OIS::MultiTouchState &state = g_multiTouch->getMultiTouchState(pointerId);
		
		switch(action){
		case 0:
			state.touchType = OIS::MT_Pressed;
			break;
		case 1:
			state.touchType = OIS::MT_Released;
			break;
		case 2:
			state.touchType = OIS::MT_Moved;
			break;
		case 3:
			state.touchType = OIS::MT_Cancelled;
			break;
		default:
			state.touchType = OIS::MT_None;
		}
		
		if(state.touchType != OIS::MT_None){
			int last = state.X.abs;
			state.X.abs = g_xOffset + (int)x;
			state.X.rel = state.X.abs - last;
			
			last = state.Y.abs;
			state.Y.abs = g_yOffset + (int)y;
			state.Y.rel = state.Y.abs - last;
			
			//last = state.Z.abs;
			state.Z.abs = 0;
			state.Z.rel = 0;
			
			OIS::MultiTouchEvent evt(g_multiTouch, state);
			
			switch(state.touchType){
			case OIS::MT_Pressed:
				g_browser->touchPressed(evt);
				break;
			case OIS::MT_Released:
				g_browser->touchReleased(evt);
				break;
			case OIS::MT_Moved:
				g_browser->touchMoved(evt);
				break;
			case OIS::MT_Cancelled:
				g_browser->touchCancelled(evt); 
				break;
			}
		}
	}
}

static void injectKeyEvent(int action, int keyCode){
	if(g_browser){
		OIS::KeyCode kc = OIS::KC_UNASSIGNED;
		unsigned int txt = 0;
		
		switch(keyCode){
		case 7:
			kc = OIS::KC_0;
			txt = '0';
			break;
		case 8:
			kc = OIS::KC_1;
			txt = '1';
			break;
		case 9:
			kc = OIS::KC_2;
			txt = '2';
			break;
		case 10:
			kc = OIS::KC_3;
			txt = '3';
			break;
		case 11:
			kc = OIS::KC_4;
			txt = '4';
			break;
		case 12:
			kc = OIS::KC_5;
			txt = '5';
			break;
		case 13:
			kc = OIS::KC_6;
			txt = '6';
			break;
		case 14:
			kc = OIS::KC_7;
			txt = '7';
			break;
		case 15:
			kc = OIS::KC_8;
			txt = '8';
			break;
		case 16:
			kc = OIS::KC_9;
			txt = '9';
			break;
		case 69:
			kc = OIS::KC_MINUS;
			txt = '-';
			break;
		case 70:
			kc = OIS::KC_EQUALS;
			txt = '=';
			break;
		case 61:
			kc = OIS::KC_TAB;
			txt = '\t';
			break;
		case 67:
			kc = OIS::KC_BACK;
			txt = '\b';
			break;
		case 29:
			kc = OIS::KC_A;
			txt = 'A';
			break;
		case 30:
			kc = OIS::KC_B;
			txt = 'B';
			break;
		case 31:
			kc = OIS::KC_C;
			txt = 'C';
			break;
		case 32:
			kc = OIS::KC_D;
			txt = 'D';
			break;
		case 33:
			kc = OIS::KC_E;
			txt = 'E';
			break;
		case 34:
			kc = OIS::KC_F;
			txt = 'F';
			break;
		case 35:
			kc = OIS::KC_G;
			txt = 'G';
			break;
		case 36:
			kc = OIS::KC_H;
			txt = 'H';
			break;
		case 37:
			kc = OIS::KC_I;
			txt = 'I';
			break;
		case 38:
			kc = OIS::KC_J;
			txt = 'J';
			break;
		case 39:
			kc = OIS::KC_K;
			txt = 'K';
			break;
		case 40:
			kc = OIS::KC_L;
			txt = 'L';
			break;
		case 41:
			kc = OIS::KC_M;
			txt = 'M';
			break;
		case 42:
			kc = OIS::KC_N;
			txt = 'N';
			break;
		case 43:
			kc = OIS::KC_O;
			txt = 'O';
			break;
		case 44:
			kc = OIS::KC_P;
			txt = 'P';
			break;
		case 45:
			kc = OIS::KC_Q;
			txt = 'Q';
			break;
		case 46:
			kc = OIS::KC_R;
			txt = 'R';
			break;
		case 47:
			kc = OIS::KC_S;
			txt = 'S';
			break;
		case 48:
			kc = OIS::KC_T;
			txt = 'T';
			break;
		case 49:
			kc = OIS::KC_U;
			txt = 'U';
			break;
		case 50:
			kc = OIS::KC_V;
			txt = 'V';
			break;
		case 51:
			kc = OIS::KC_W;
			txt = 'W';
			break;
		case 52:
			kc = OIS::KC_X;
			txt = 'X';
			break;
		case 53:
			kc = OIS::KC_Y;
			txt = 'Y';
			break;
		case 54:
			kc = OIS::KC_Z;
			txt = 'Z';
			break;
		case 71:
			kc = OIS::KC_LBRACKET;
			txt = '[';
			break;
		case 72:
			kc = OIS::KC_RBRACKET;
			txt = ']';
			break;
		case 66:
			kc = OIS::KC_RETURN;
			txt = '\n';
			break;
		case 74:
			kc = OIS::KC_SEMICOLON;
			txt = ';';
			break;
		case 75:
			kc = OIS::KC_APOSTROPHE;
			txt = '\'';
			break;
		case 73:
			kc = OIS::KC_BACKSLASH;
			txt = '\\';
			break;
		case 55:
			kc = OIS::KC_COMMA;
			txt = ','; 
			break;
		case 56:
			kc = OIS::KC_PERIOD;
			txt = '.';
			break;
		case 76:
			kc = OIS::KC_SLASH;
			txt = '/';
			break;
		case 19:
			kc = OIS::KC_UP;
			break;
		case 20:
			kc = OIS::KC_DOWN;
			break;
		case 21:
			kc = OIS::KC_LEFT;
			break;
		case 22:
			kc = OIS::KC_RIGHT;
			break;
		}
		
		OIS::KeyEvent evt(g_keyboard, kc, txt);
		if(action == 0)
		{
			g_browser->keyPressed(evt);
		}
		else
		{
			g_browser->keyReleased(evt);
		}
	}
}
 
jboolean init(JNIEnv* env, jobject thiz)
{
	bool ret = initOgreRoot();
	if(!ret)
		return JNI_FALSE;
	
	g_rootInit = true;

	g_archiveFactory = OGRE_NEW Ogre::AndroidArchiveFactory(env);
	Ogre::ArchiveManager::getSingleton().addArchiveFactory(g_archiveFactory);
			
	LOGI("Adding resource locations");
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Popular");
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Essential");
	
	for(Ogre::map<Ogre::String,Ogre::String>::type::iterator i = g_resourceMap.begin(); i != g_resourceMap.end(); ++i)
	{
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(i->first, "Android", i->second);  
	}
}

jboolean render(JNIEnv* env, jobject thiz, jint drawWidth, jint drawHeight, jboolean forceRedraw)
{
	// Check that a render window even exists
	if(getRenderWindow() == 0){
		initRenderWindow(0, drawWidth, drawHeight, 0);
	}
	
	// Initialize the sample browser
	if(g_multiTouch == 0){
		g_multiTouch = new AndroidMultiTouch();
	}
	
	if(g_keyboard == 0){
		g_keyboard = new AndroidKeyboard();
	}
	
	if(g_browser == 0){	
		LOGI("Initializing the sample browser");
		g_browser = OGRE_NEW OgreBites::SampleBrowser();
		g_browser->initAppForAndroid(getRenderWindow(), g_multiTouch, g_keyboard);
		g_browser->initApp();
		LOGI("Browser initialized"); 
		
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}
	
	renderOneFrame();
	
    return JNI_TRUE;
}

void cleanup(JNIEnv* env)
{
	if(g_browser){ 
		g_browser->closeApp();
		OGRE_DELETE g_browser;
		
		g_browser = 0;
	}
	
	if(g_multiTouch){
		delete g_multiTouch;
		g_multiTouch = 0;
	}
	
	if(g_keyboard){
		delete g_keyboard;
		g_keyboard = 0;
	}
	
	if(getRenderWindow()){
		destroyRenderWindow();
	}
		
	LOGI("deleting ogre root");
	if(g_rootInit){
		destroyOgreRoot();
		g_rootInit = false;
	}
	
	LOGI("deleting archive stuff");
	if(g_archiveFactory){
		OGRE_DELETE g_archiveFactory; 
		g_archiveFactory = 0;
	}
}

jboolean inputEvent(JNIEnv* env, jobject thiz, jint action, jfloat mx, jfloat my)
{
    injectTouchEvent(0, action, mx, my);
	return JNI_TRUE;
}

jboolean keyEvent(JNIEnv* env, jobject thiz, jint action, jint unicodeChar, jint keyCode, jobject keyEvent)
{
    injectKeyEvent(action, keyCode);
    return JNI_TRUE;  
}

void setOffsets(JNIEnv* env, jobject thiz, jint x, jint y)
{
    g_xOffset = x;
	g_yOffset = y;
}

void addResourceLocation(JNIEnv *env, jobject thiz, jstring name, jstring group)
{
	LOGI("Adding resource location");
	
	const char *str1 = env->GetStringUTFChars(name, 0);
	const char *str2 = env->GetStringUTFChars(group, 0);
	g_resourceMap[str1] = str2;
	env->ReleaseStringUTFChars(name, str1);
	env->ReleaseStringUTFChars(group, str2);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;

    LOGI("JNI_OnLoad called");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
    	LOGE("Failed to get the environment using GetEnv()");
        return -1;
    }
    JNINativeMethod methods[] =
    {
		{
            "init",
            "()Z",
            (void *) init
        },
        {
        	"render",
			"(IIZ)Z",
			(void *) render
        },
        {
			"inputEvent",
			"(IFFLandroid/view/MotionEvent;)Z",
			(void *) inputEvent

        },
        {
            "keyEvent",
            "(IIILandroid/view/KeyEvent;)Z",
            (void *) keyEvent
        },
        {
            "cleanup",
            "()V",
            (void *) cleanup
        },
		{
			"setOffsets",
			"(II)V",
			(void *) setOffsets
		},
		{
			"addResourceLocation",
			"(Ljava/lang/String;Ljava/lang/String;)V",
			(void *) addResourceLocation
		},
    };
    jclass k;
    k = (env)->FindClass ("org/ogre/samples/OgreSampleBrowserActivity");
    (env)->RegisterNatives(k, methods, 7);

    return JNI_VERSION_1_4;
}