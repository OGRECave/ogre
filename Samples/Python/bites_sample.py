import Ogre
from Ogre import RTShader, Overlay, Bites

class SampleApp(Bites.ApplicationContext, Bites.InputListener):
    def __init__(self):
        Bites.ApplicationContext.__init__(self, "PySample")
        Bites.InputListener.__init__(self)

    def keyPressed(self, evt):
        if evt.keysym.sym == Bites.SDLK_ESCAPE:
            self.getRoot().queueEndRendering()

        return True

    def loadResources(self):
        self.enableShaderCache()

        # load essential resources for trays/ loading bar
        Ogre.ResourceGroupManager.getSingleton().initialiseResourceGroup("Essential")
        self.createDummyScene()
        self.trays = Bites.TrayManager("Interface", self.getRenderWindow())
        self.addInputListener(self.trays)

        # show loading progress
        self.trays.showLoadingBar(1, 0)
        ret = Bites.ApplicationContext.loadResources(self)

        # clean up
        self.trays.hideLoadingBar()
        self.destroyDummyScene()
        return ret

    def setup(self):
        Bites.ApplicationContext.setup(self)
        self.addInputListener(self)

        root = self.getRoot()
        scn_mgr = root.createSceneManager()

        shadergen = RTShader.ShaderGenerator.getSingleton()
        shadergen.addSceneManager(scn_mgr)  # must be done before we do anything with the scene

        # overlay/ trays
        scn_mgr.addRenderQueueListener(self.getOverlaySystem())
        self.trays.showFrameStats(Bites.TL_TOPRIGHT)
        self.trays.refreshCursor()

        # enable per pixel lighting
        rs = shadergen.getRenderState(RTShader.cvar.ShaderGenerator_DEFAULT_SCHEME_NAME)
        rs.addTemplateSubRenderState(shadergen.createSubRenderState(RTShader.cvar.PerPixelLighting_Type))

        scn_mgr.setAmbientLight(Ogre.ColourValue(.1, .1, .1))

        light = scn_mgr.createLight("MainLight")
        lightnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        lightnode.setPosition(0, 10, 15)
        lightnode.attachObject(light)

        cam = scn_mgr.createCamera("myCam")
        cam.setNearClipDistance(5)
        cam.setAutoAspectRatio(True)
        camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        camnode.attachObject(cam)

        self.camman = Bites.CameraMan(camnode)
        self.camman.setStyle(Bites.CS_ORBIT)
        self.camman.setYawPitchDist(Ogre.Radian(0), Ogre.Radian(0.3), 15)
        self.addInputListener(self.camman)

        # must keep a reference to ctrls so it does not get deleted
        self.ctrls = Bites.AdvancedRenderControls(self.trays, cam)
        self.addInputListener(self.ctrls)

        vp = self.getRenderWindow().addViewport(cam)
        vp.setBackgroundColour(Ogre.ColourValue(.3, .3, .3))

        ent = scn_mgr.createEntity("Sinbad.mesh")
        node = scn_mgr.getRootSceneNode().createChildSceneNode()
        node.attachObject(ent)
    
    def shutdown(self):
        # manually destroy in reverse creation order
        del app.ctrls
        del app.trays
    
if __name__ == "__main__":
    app = SampleApp()
    app.initApp()
    app.getRoot().startRendering()
    app.closeApp()
