import Ogre
import Ogre.RTShader

class SGResolver(Ogre.MaterialManager_Listener):
    def __init__(self, shadergen):
        Ogre.MaterialManager_Listener.__init__(self)
        self.shadergen = shadergen

    def handleSchemeNotFound(self, idx, sname, mat, lod_idx, rend):
        if sname != Ogre.MSN_SHADERGEN:
            return None

        succ = self.shadergen.createShaderBasedTechnique(mat, Ogre.MSN_DEFAULT, sname)

        if not succ:
            return None

        self.shadergen.validateMaterial(sname, mat.getName(), mat.getGroup())

        return mat.getTechnique(1)

def main():
    root = Ogre.Root("plugins.cfg", "ogre.cfg", "")

    cfg = Ogre.ConfigFile()
    cfg.loadDirect("resources.cfg")

    rgm = Ogre.ResourceGroupManager.getSingleton()

    for sec, settings in cfg.getSettingsBySection().items():
        for kind, loc in settings.items():
            rgm.addResourceLocation(loc, kind, sec)

    if not root.restoreConfig():
        root.showConfigDialog(None)
        root.saveConfig()

    win = root.initialise(True)

    Ogre.RTShader.ShaderGenerator.initialize()
    shadergen = Ogre.RTShader.ShaderGenerator.getSingleton()

    sgres = SGResolver(shadergen)
    Ogre.MaterialManager.getSingleton().addListener(sgres)

    rgm.initialiseAllResourceGroups()

    rs = shadergen.getRenderState(Ogre.MSN_SHADERGEN)
    rs.addTemplateSubRenderState(shadergen.createSubRenderState("SGX_PerPixelLighting"))

    scn_mgr = root.createSceneManager()
    shadergen.addSceneManager(scn_mgr)

    scn_mgr.setAmbientLight((.1, .1, .1))

    light = scn_mgr.createLight("MainLight")
    lightnode = scn_mgr.getRootSceneNode().createChildSceneNode()
    lightnode.setPosition(0, 10, 15)
    lightnode.attachObject(light)

    cam = scn_mgr.createCamera("myCam")
    cam.setNearClipDistance(5)

    camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
    camnode.attachObject(cam)
    camnode.lookAt((0, 0, -1), Ogre.Node.TS_WORLD)
    camnode.setPosition(0, 0, 15)

    vp = win.addViewport(cam)
    vp.setBackgroundColour((.3, .3, .3))

    ent = scn_mgr.createEntity("Sinbad.mesh")
    node = scn_mgr.getRootSceneNode().createChildSceneNode()
    node.attachObject(ent)

    while not root.endRenderingQueued():
        root.renderOneFrame()

if __name__ == "__main__":
    main()
