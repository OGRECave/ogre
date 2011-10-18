package org.ogre.samples;

import java.io.IOException;

import org.ogre.ACPManager;

import com.nvidia.devtech.NvGLES2Activity;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class OgreSampleBrowserActivity extends NvGLES2Activity {	

	@Override
	public native boolean render(int drawWidth, int drawHeight, boolean forceRedraw);

	@Override
	public native void cleanup();

	@Override
	public native boolean init();

	@Override
	public native boolean inputEvent(int action, float x, float y, MotionEvent event);

	@Override
	public native boolean keyEvent(int action, int unicodeChar, int keyCode, KeyEvent event);
	
	public native void setOffsets(int x, int y);
	
	public native void addResourceLocation(String name, String group);
	
	public void onCreate(Bundle savedInstanceState){
		// Set up asset manager
		ACPManager.getInstance().setAssets(getAssets(), "packs/");
		
		addResourceLocation("SdkTrays.m3", "Essential");
		addResourceLocation("thumbnails.m3", "Essential");
		addResourceLocation("RTShaderLib.m3", "Popular");
			
		// Call parent
		super.onCreate(savedInstanceState);
		
		//Force landscape
	    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	}
	
	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		int screenHeight = this.getWindowManager().getDefaultDisplay().getHeight();
		int viewHeight = surfaceView.getHeight();
		
		// Use the difference as the cursor offset
		setOffsets(0, viewHeight - screenHeight);
		
		super.onWindowFocusChanged(hasFocus);
	}
	
	static
    {
		System.loadLibrary("ogre");
		System.loadLibrary("ois");
		System.loadLibrary("ogresamplebrowser");
    }
}