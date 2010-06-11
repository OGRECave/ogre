//----------------------------------------------------------------------------------
// File:            libs\src\com\nvidia\devtech\NvUtil.java
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
package com.nvidia.devtech;

import java.util.HashMap;

import android.app.Activity;
import android.os.Environment;

/**
 * A helper class used to aid native code.
 * Normally you would not use this code in Java, but rather use the NvUtil* functions
 * in your native code by including <nv_util/nv_util.h>  
 */
public class NvUtil
{
	private HashMap<String, String> appLocalValues;
	private static NvUtil instance = new NvUtil();
	private Activity activity = null;

	private NvUtil()
	{
		appLocalValues = new HashMap<String, String>();
		appLocalValues.put("STORAGE_ROOT", 
			Environment.getExternalStorageDirectory().getAbsolutePath());
	}

	public void setActivity(Activity activity)
	{
		this.activity = activity; 
	}
	public static NvUtil getInstance()
	{
		return instance;
	}

	/**
	 * Checks whether a key is in the app local value list
	 * @param key The key to test
	 * @return Whether or not the key is in the app local value list
	 */
    public boolean hasAppLocalValue(String key)
    {
		return appLocalValues.containsKey(key);
    }

    /**
     * Get the specified key value from the app local value list
     * @param key The key to get the value of
     * @return The key's value
     */
    public String getAppLocalValue(String key)
    {
		return appLocalValues.get(key);
    }

    /**
     * Set the specified key value in the app local value list
     * @param key The key to set the value of
     * @param value The value
     */
    public void setAppLocalValue(String key, String value)
    {
		appLocalValues.put(key, value);
    }

    /**
     * This function is used to get the parameters used to start the Activity via, for example:
     * <pre>
     * adb shell am start -a android.intent.action.MAIN -n com.nvidia.devtech.water/com.nvidia.devtech.water.Water -e param1 1 -e param2 2
     * </pre>
     * Where "param1" and "param2" are the parameter names and "1" and "2" are the parameter values.
     *  
     * @param paramName The name of the parameter to get
     * @return The parameter
     */
    public String getParameter(String paramName)
    {
    	return activity.getIntent().getStringExtra(paramName);
    }
}