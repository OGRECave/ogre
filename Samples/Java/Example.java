import org.ogre.*;

public class Example {
    static {
        System.loadLibrary("OgreJNI");
    }

    public static void main(String[] args) {
        final ApplicationContext ctx = new ApplicationContext();
        ctx.initApp();

        ctx.addInputListener(new InputListener() {
            public boolean keyPressed(KeyboardEvent evt)
            {
                if (evt.getKeysym().getSym() == BitesConstants.SDLK_ESCAPE)
                    ctx.getRoot().queueEndRendering();
                return true;
            }
        });

        Root root = ctx.getRoot();
        SceneManager scnMgr = root.createSceneManager();
        ShaderGenerator.getSingleton().addSceneManager(scnMgr);

        scnMgr.setAmbientLight(new ColourValue(.1f, .1f, .1f));

        Light light = scnMgr.createLight("MainLight");
        SceneNode lightnode = scnMgr.getRootSceneNode().createChildSceneNode();
        lightnode.setPosition(0, 10, 15);
        lightnode.attachObject(light);

        Camera cam = scnMgr.createCamera("myCam");
        SceneNode camnode = scnMgr.getRootSceneNode().createChildSceneNode();
        camnode.attachObject(cam);
        camnode.setPosition(0, 0, 15);

        cam.setNearClipDistance(5);
        cam.setAutoAspectRatio(true);

        CameraMan camman = new CameraMan(camnode);
        camman.setStyle(CameraStyle.CS_ORBIT);
        camman.setYawPitchDist(new Radian(0f), new Radian(0.3f), 15);
        ctx.addInputListener(camman);

        Viewport vp = ctx.getRenderWindow().addViewport(cam);
        vp.setBackgroundColour(new ColourValue(.3f, .3f, .3f));

        Entity ent = scnMgr.createEntity("Sinbad.mesh");
        SceneNode node = scnMgr.getRootSceneNode().createChildSceneNode();
        node.attachObject(ent);

        ctx.getRoot().startRendering();
        ctx.closeApp();
    }
}
