using org.ogre;
using System.Windows.Forms;
using System.Runtime.InteropServices;

public class Example : ApplicationContextBase
{
    private Form form;

    public Example()
    {
        form = new Form();
        form.Resize += Resize;
        form.KeyDown += KeyDown;
        form.Closing += Closing;
        // TODO: prevent drawing of Form, since Ogre draws over anyway

        form.Show();
    }

    private void Resize(object sender, System.EventArgs e)
    {
        var win = getRenderWindow();
        win.windowMovedOrResized();
        windowResized(win);
    }

    private void KeyDown(object sender, KeyEventArgs e)
    {
        if(e.KeyValue == 27)
            getRoot().queueEndRendering();
    }

    private void Closing(object sender, System.ComponentModel.CancelEventArgs e)
    {
        getRoot().queueEndRendering();
    }

    public override NativeWindowPair createWindow(string title, uint w, uint h, NameValuePairList miscParams)
    {
        miscParams["externalWindowHandle"] = form.Handle.ToString();

        var ret = base.createWindow(title, w, h, miscParams);

        w = ret.render.getWidth();
        h = ret.render.getHeight();

        // update Form
        form.Text = title;
        form.Width = (int)w;
        form.Height = (int)h;

        return ret;
    }

    public override void setup()
    {
        base.setup();

        var root = getRoot();
        var scnMgr = root.createSceneManager();

        var shadergen = ShaderGenerator.getSingleton();
        shadergen.addSceneManager(scnMgr); // must be done before we do anything with the scene

        scnMgr.setAmbientLight(new ColourValue(.1f, .1f, .1f));

        var light = scnMgr.createLight("MainLight");
        var lightnode = scnMgr.getRootSceneNode().createChildSceneNode();
        lightnode.setPosition(0f, 10f, 15f);
        lightnode.attachObject(light);

        var cam = scnMgr.createCamera("myCam");
        cam.setAutoAspectRatio(true);
        cam.setNearClipDistance(5);
        var camnode = scnMgr.getRootSceneNode().createChildSceneNode();
        camnode.attachObject(cam);

        var camman = new CameraMan(camnode);
        camman.setStyle(CameraStyle.CS_ORBIT);
        camman.setYawPitchDist(new Radian(0), new Radian(0.3f), 15f);

        var vp = getRenderWindow().addViewport(cam);
        vp.setBackgroundColour(new ColourValue(.3f, .3f, .3f));

        var ent = scnMgr.createEntity("Sinbad.mesh");
        var node = scnMgr.getRootSceneNode().createChildSceneNode();
        node.attachObject(ent);
    }

    static void Main()
    {
        var app = new Example();
        app.initApp();

        while(!app.getRoot().endRenderingQueued())
        {
            Application.DoEvents();
            app.getRoot().renderOneFrame();
        }

        app.closeApp();
    }
}