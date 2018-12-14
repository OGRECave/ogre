/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

package org.ogre.example;

import org.ogre.*;

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.os.Bundle;
import android.os.Handler;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.content.res.AssetManager;

public class BasicSample extends Activity implements SensorEventListener {
	protected Handler handler = null;
	protected SurfaceView surfaceView = null;
	protected Surface lastSurface = null;

	private Runnable renderer = null;
	private boolean paused = false;
	private boolean initOGRE = false;
	private AssetManager assetMgr = null;
	
	ApplicationContext ogreApp = null;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		handler = new Handler();
		sysInit();
	}

	@Override
	protected void onPause() {
		super.onPause();
		handler.removeCallbacks(renderer);
		paused = true;
	}

	@Override
	protected void onResume() {
		super.onResume();
		paused = false;
		handler.post(renderer);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();

		Runnable destroyer = new Runnable() {
			public void run() {
			    ogreApp.shutdown();
			}
		};
		handler.post(destroyer);
	}

	private void sysInit() {
		final Runnable initRunnable = new Runnable() {
			public void run() {
				if (!initOGRE) {
					initOGRE = true;
					
					if(assetMgr == null) {
						assetMgr = getResources().getAssets();
					}
					
					renderer = new Runnable() {
						public void run() {

							if (paused)
								return;

							if (!wndCreate && lastSurface != null) {
								wndCreate = true;
								
								if(ogreApp == null) {
                                    ogreApp = new ApplicationContext();
                                    ogreApp.initAppForAndroid(assetMgr, lastSurface);
                                    
                                    SceneManager scnMgr = ogreApp.getRoot().createSceneManager();
                                    ShaderGenerator.getSingleton().addSceneManager(scnMgr);
                                    
                                    Light light = scnMgr.createLight("MainLight");
                                    SceneNode lightnode = scnMgr.getRootSceneNode().createChildSceneNode();
                                    lightnode.setPosition(0, 10, 15);
                                    lightnode.attachObject(light);
                                    
                                    Camera cam = scnMgr.createCamera("myCam");
                                    cam.setNearClipDistance(5);
                                    cam.setAutoAspectRatio(true);
                                    
                                    SceneNode camnode = scnMgr.getRootSceneNode().createChildSceneNode();
                                    camnode.attachObject(cam);
                                    camnode.setPosition(0, 0, 15);
                                    
                                    Entity ent = scnMgr.createEntity("Sinbad.mesh");
                                    SceneNode node = scnMgr.getRootSceneNode().createChildSceneNode();
                                    node.attachObject(ent);
                                    
                                    Viewport vp = ogreApp.getRenderWindow().addViewport(cam);
                                    vp.setBackgroundColour(new ColourValue(0.3f, 0.3f, 0.3f));
								} else {
								    ogreApp.getRenderWindow()._notifySurfaceCreated(lastSurface);
								}
                                
								handler.post(this);
								return;
							}

                            if (initOGRE && wndCreate) {
                                ogreApp.getRoot().renderOneFrame();
                            }

							handler.post(this);
						}
					};

					handler.post(renderer);
				}
			}

		};

		SurfaceView view = new SurfaceView(this);
		SurfaceHolder holder = view.getHolder();
		surfaceView = view;

		holder.addCallback(new Callback() {
			public void surfaceCreated(SurfaceHolder holder) {
				if (holder.getSurface() != null
						&& holder.getSurface().isValid()) {
					lastSurface = holder.getSurface();
					handler.post(initRunnable);
				}
			}

			public void surfaceDestroyed(SurfaceHolder holder) {
				if (initOGRE && wndCreate) {
					wndCreate = false;
					lastSurface = null;
					handler.post(new Runnable() {
						public void run() {
						    ogreApp.getRenderWindow()._notifySurfaceDestroyed();
						}
					});
				}
			}

			public void surfaceChanged(SurfaceHolder holder, int format,
					int width, int height) {

			}
		});
		setContentView(surfaceView);
	}

	boolean wndCreate = false;

	public void onAccuracyChanged(Sensor sensor, int accuracy) {

	}

	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
		}
	}

	static {
		System.loadLibrary("OgreJNI");
	}
}
