import Ogre
import OgreRTShader
import OgreBites

class SampleApp(OgreBites.ApplicationContext):
    def __init__(self):
        OgreBites.ApplicationContext.__init__(self, "PySample", False)

    def keyPressed(self, evt):
        if evt.keysym.scancode == OgreBites.SDL_SCANCODE_ESCAPE:
            self.getRoot().queueEndRendering()

        return True

    def setup(self):
        OgreBites.ApplicationContext.setup(self)

        root = self.getRoot()
        scn_mgr = root.createSceneManager(Ogre.ST_GENERIC)

        shadergen = OgreRTShader.ShaderGenerator.getSingleton()
        shadergen.addSceneManager(scn_mgr)  # must be done before we do anything with the scene

        # enable per pixel lighting
        rs = shadergen.getRenderState(OgreRTShader.cvar.ShaderGenerator_DEFAULT_SCHEME_NAME)
        rs.addTemplateSubRenderState(shadergen.createSubRenderState(OgreRTShader.cvar.PerPixelLighting_Type))

        scn_mgr.setAmbientLight(Ogre.ColourValue(.1, .1, .1))

        light = scn_mgr.createLight("MainLight")
        light.setPosition(0, 10, 15)

        cam = scn_mgr.createCamera("myCam")
        cam.setNearClipDistance(5)
        cam.setAutoAspectRatio(True);

        self.camman = OgreBites.CameraMan(cam)
        self.camman.setStyle(OgreBites.CS_ORBIT)

        vp = self.getRenderWindow().addViewport(cam)
        vp.setBackgroundColour(Ogre.ColourValue(.3, .3, .3))

        ent = scn_mgr.createEntity("Sinbad.mesh")
        node = scn_mgr.getRootSceneNode().createChildSceneNode()
        node.attachObject(ent)

    # forward input events to camera manager
    def mouseMoved(self, evt):
        self.camman.injectMouseMove(evt)
        return True

    def mouseWheelRolled(self, evt):
        self.camman.injectMouseWheel(evt)
        return True

    def mousePressed(self, evt):
        self.camman.injectMouseDown(evt)
        return True

    def mouseReleased(self, evt):
        self.camman.injectMouseUp(evt)
        return True

    def frameStarted(self, evt):
        self.captureInputDevices()
        return True

if __name__ == "__main__":
    app = SampleApp()
    app.initApp()
    app.getRoot().startRendering()
