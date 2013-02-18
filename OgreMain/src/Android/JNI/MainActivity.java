package org.ogre3d.android;

import org.ogre3d.android.OgreActivityJNI;

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

public class MainActivity extends Activity implements SensorEventListener {
	protected Handler handler = null;
	protected SurfaceView surfaceView = null;
	protected Surface lastSurface = null;

	private Runnable renderer = null;
	private boolean paused = false;
	private boolean initOGRE = false;

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
	
	

	private void sysInit() {
		final Runnable initRunnable = new Runnable() {
			public void run() {
				if (!initOGRE) {
					initOGRE = true;
					OgreActivityJNI.create();

					renderer = new Runnable() {
						public void run() {

							if (paused)
								return;

							if (!wndCreate && lastSurface != null) {
								wndCreate = true;
								OgreActivityJNI.initWindow(lastSurface);
								handler.post(this);
								return;
							}

							if (initOGRE && wndCreate)
								OgreActivityJNI.renderOneFrame();

							handler.post(this);
						}
					};

					handler.post(renderer);
				}
			}

		};

		SurfaceView view = new SurfaceView(this);
		SurfaceHolder holder = view.getHolder();
		// holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
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
							OgreActivityJNI.termWindow();
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
