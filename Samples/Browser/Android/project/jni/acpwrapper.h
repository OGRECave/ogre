#ifndef __ACP_WRAPPER_H__
#define __ACP_WRAPPER_H__

#include <jni.h>

// Returns 1 if the file exists in the assets
unsigned char acp_has_file(JNIEnv *env, const char *name);

// Allocates and fills the data with the package of the given name
// Returns 0 for success
int acp_get_file(JNIEnv *env, const char *name, void **ptr, int *size);

#endif
