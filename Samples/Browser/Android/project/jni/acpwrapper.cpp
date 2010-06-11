#include "acpwrapper.h"

#include <stdlib.h>

#include <android/log.h>

#define  LOG_TAG    "acpwrapper"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static const char MANAGER_CLASS_NAME[] = "org/ogre/ACPManager";
static const char RAWDATA_CLASS_NAME[] = "org/ogre/ACPManager$RawData";

unsigned char acp_has_file(JNIEnv *env, const char *name)
{
	// Get the asset manager
	jclass cls = env->FindClass(MANAGER_CLASS_NAME);
	if(cls == NULL)
		return 0;
		
	// Find methods to be called
	jmethodID hasFile_method = env->GetMethodID(cls, "hasFile", "(Ljava/lang/String;)Z");
	jmethodID getInstance_method = env->GetStaticMethodID(cls, "getInstance", "()Lorg/ogre/ACPManager;");
	if(hasFile_method == NULL || getInstance_method == NULL)
		return 0;
	
	// Get the instance of the manager
    jobject obj = env->CallStaticObjectMethod(cls, getInstance_method);
	if(obj == NULL)
		return 0;
	
	// Call class method
	jstring str = env->NewStringUTF(name);
	jboolean result = env->CallBooleanMethod(obj, hasFile_method, str);
	
	return (result == JNI_TRUE ? 1 : 0);
}

int acp_get_file(JNIEnv *env, const char *name, void **ptr, int *size)
{
	// Get the asset manager
	jclass cls = env->FindClass(MANAGER_CLASS_NAME);
	if(cls == NULL)
		return 1;
		
	jmethodID getFileSize_method = env->GetMethodID(cls, "getFileSize", "(Ljava/lang/String;)I");
	jmethodID beginStream_method = env->GetMethodID(cls, "beginStream", "(Ljava/lang/String;)V");
	jmethodID endStream_method = env->GetMethodID(cls, "endStream", "()V");
	jmethodID readStream_method = env->GetMethodID(cls, "readStream", "()Lorg/ogre/ACPManager$RawData;");
	if(getFileSize_method == NULL || beginStream_method == NULL || endStream_method == NULL || readStream_method == NULL)
		return 1;

	jmethodID getInstance_method = env->GetStaticMethodID(cls, "getInstance", "()Lorg/ogre/ACPManager;");
	if(getInstance_method == NULL)
		return 1;
		
    jobject obj = env->CallStaticObjectMethod(cls, getInstance_method);
	if(obj == NULL)
		return 1;
	
	jstring filename = env->NewStringUTF(name);
	
	// Get the total file size
	jint filesize = env->CallIntMethod(obj, getFileSize_method, filename);
	
	if(filesize > 0){		
		// Start the streaming		
		static jclass RawData_class = env->FindClass(RAWDATA_CLASS_NAME);
		if(RawData_class == NULL)
			return 1;
		static jfieldID lengthId = env->GetFieldID(RawData_class, "length", "I");
		static jfieldID dataId = env->GetFieldID(RawData_class, "data", "[B");
		if(lengthId == NULL || dataId == NULL)
			return 1;
		
		// Allocate the buffer
		*size = filesize;
		*ptr = malloc(filesize);
		unsigned char *ptr2 = (unsigned char*)*ptr;
			
		env->CallVoidMethod(obj, beginStream_method, filename);
		
		int offset = 0; // Store where into the buffer we're writing to
		while(offset < filesize){
			jobject rawData = env->CallObjectMethod(obj, readStream_method);		

			if(rawData != 0){
				jbyteArray buffer = (jbyteArray) env->GetObjectField(rawData, dataId);
				int length = env->GetIntField(rawData, lengthId);
				env->GetByteArrayRegion(buffer, 0, length, (jbyte*)(ptr2 + offset)); // Copy the bytes
				
				offset += length;
			}
		}
		
		env->CallVoidMethod(obj, endStream_method);
	}
	return 0;
}
