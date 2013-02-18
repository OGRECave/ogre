package org.ogre3d.android;

import android.view.Surface;

public class OgreActivityJNI {	
	public native static void create();	
	public native static void destroy();	
	public native static void initWindow(Surface surface);
	public native static void termWindow();
	public native static void renderOneFrame();
}
