package org.ogre.example;

import android.content.pm.ActivityInfo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

import android.graphics.SurfaceTexture;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.ogre.*;

public class AndroidTextureOES extends Activity {

    static {
        System.loadLibrary("OgreJNI");
    }

    private Handler handler = new Handler();
    private ApplicationContext ogre = new ApplicationContext();
    private SurfaceTexture cameraTexture;

    private Surface lastSurface;
    private SurfaceView surfaceView;
    private Rectangle2D rect;

    private boolean windowCreated = false;
    private boolean renderSessionPaused = true;
    private boolean sceneCreated = false;

    private long textureId = -1L;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        surfaceView = new SurfaceView(this);
        setContentView(surfaceView);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                if (holder.getSurface() != null && holder.getSurface().isValid()) {
                    lastSurface = holder.getSurface();
                    handler.post(renderer);
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                windowCreated = false;
                ogre.getRenderWindow()._notifySurfaceDestroyed();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        renderSessionPaused = false;
        handler.post(renderer);
    }

    @Override
    protected void onPause() {
        renderSessionPaused = true;
        super.onPause();
    }

    private void initializeScene() {
        sceneCreated = true;

        final SceneManager scnMgr = ogre.getRoot().createSceneManager();
        ShaderGenerator.getSingleton().addSceneManager(scnMgr);

        final Light light = scnMgr.createLight("MainLight");

        final SceneNode lightNode = scnMgr.getRootSceneNode().createChildSceneNode();
        lightNode.setPosition(0f, 10f, 15f);
        lightNode.attachObject(light);

        final Camera camera = scnMgr.createCamera("Camera");
        camera.setNearClipDistance(5f);
        camera.setAutoAspectRatio(true);

        final SceneNode cameraNode = scnMgr.getRootSceneNode().createChildSceneNode();
        cameraNode.attachObject(camera);
        cameraNode.setPosition(0f, 0f, 15f);

        final MaterialPtr material = MaterialManager.getSingleton().create("PlaneMat", Ogre.getRGN_DEFAULT());

        final Pass pass = material.getTechniques().get(0).getPass(0);
        pass.setDepthCheckEnabled(false);
        pass.setDepthWriteEnabled(false);
        pass.setLightingEnabled(false);

        final TexturePtr texturePtr = TextureManager.getSingleton().createManual(
                "SomeTexture", Ogre.getRGN_DEFAULT(),
                TextureType.TEX_TYPE_EXTERNAL_OES,
                1080,
                1920,
                0,
                PixelFormat.PF_UNKNOWN
        );

        textureId = texturePtr.getCustomAttribute("GLID");

        pass.createTextureUnitState().setTexture(texturePtr);

        final AxisAlignedBox bb = new AxisAlignedBox();
        bb.setInfinite();

        rect = new Rectangle2D(true);
        rect.setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
        // 90 deg rotate:
        rect.setUVs(new Vector2(0, 1), new Vector2(1, 1), new Vector2(0, 0), new Vector2(1, 0));
        rect.setMaterial(material);
        rect.setRenderQueueGroup((short)RenderQueueGroupID.RENDER_QUEUE_BACKGROUND.swigValue());
        rect.setBoundingBox(bb);

        final SceneNode rectNode = scnMgr.getRootSceneNode().createChildSceneNode();
        rectNode.attachObject(rect);

        final Entity ogreEntity = scnMgr.createEntity("Sinbad.mesh");
        final SceneNode ogreNode = scnMgr.getRootSceneNode().createChildSceneNode();
        ogreNode.attachObject(ogreEntity);
        ogreNode.setPosition(new Vector3(0f, 0f, 0f));
        ogreNode.scale(0.5f, 0.5f, 0.5f);

        ogre.getRenderWindow().addViewport(camera);
    }

    private Runnable renderer = new Runnable() {
        @Override
        public void run() {
            if (renderSessionPaused) {
                return;
            }

            if (!windowCreated && lastSurface != null) {
                windowCreated = true;

                if (!sceneCreated) {
                    ogre.initAppForAndroid(getAssets(), lastSurface);
                    initializeScene();

                    if (ContextCompat.checkSelfPermission(AndroidTextureOES.this, Manifest.permission.CAMERA)
                            == PackageManager.PERMISSION_DENIED)
                    {
                        ActivityCompat.requestPermissions(AndroidTextureOES.this, new String[] {Manifest.permission.CAMERA}, 100);
                    }

                    final android.hardware.Camera camera = android.hardware.Camera.open();
                    try {
                        cameraTexture = new SurfaceTexture((int) textureId);
                        camera.setPreviewTexture(cameraTexture);
                        final android.hardware.Camera.Parameters params = camera.getParameters();
                        final android.hardware.Camera.Size size = params.getSupportedPreviewSizes().get(0);
                        params.setPreviewSize(size.width, size.height);
                        camera.setParameters(params);
                        camera.startPreview();
                    } catch (Exception e) {
                        throw new RuntimeException("An error occurred during camera preview initialization");
                    }
                } else {
                    ogre.getRenderWindow()._notifySurfaceCreated(lastSurface);
                }

                handler.post(this);
                return;
            }

            if (windowCreated) {
                cameraTexture.updateTexImage();
                ogre.getRoot().renderOneFrame();
            }

            handler.post(this);
        }
    };
}
