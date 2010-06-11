//----------------------------------------------------------------------------------
// File:            libs\jni\nv_util\nv_util.h
// Samples Version: Android NVIDIA samples 1.0 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2010-2011 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
#ifndef __INCLUDED_NV_UTIL_H
#define __INCLUDED_NV_UTIL_H

#include <jni.h>

void NvUtilInit(JNIEnv *env);
void NvUtilGetLocalAppValue(char *buffer, int bufferLength, const char* name);
bool NvUtilHasLocalAppValue(const char* name);
void NvUtilSetLocalAppValue(const char* name, const char* value);
void NvUtilGetParameter(char *buffer, int bufferLength, const char *parameter);


#endif
