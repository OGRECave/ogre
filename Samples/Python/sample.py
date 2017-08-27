import Ogre
import OgreRTShader

class SGResolver(Ogre.MaterialManager_Listener):
    def __init__(self, shadergen):
        Ogre.MaterialManager_Listener.__init__(self)
        self.shadergen = shadergen

    def handleSchemeNotFound(self, idx, name, mat, lod_idx, rend):
        if name != OgreRTShader.cvar.ShaderGenerator_DEFAULT_SCHEME_NAME:
            return None

        def_name = Ogre.cvar.MaterialManager_DEFAULT_SCHEME_NAME
        succ = self.shadergen.createShaderBasedTechnique(mat, def_name, name)

        if not succ:
            return None

        self.shadergen.validateMaterial(name, mat.getName(), mat.getGroup())

        return mat.getTechnique(1)

def main():
    root = Ogre.Root("plugins.cfg", "ogre.cfg", "")

    cfg = Ogre.ConfigFile()
    cfg.loadDirect("resources.cfg")

    rgm = Ogre.ResourceGroupManager.getSingleton()

    for sec, settings in cfg.getSettingsBySection().items():
        for kind, loc in settings.items():
            rgm.addResourceLocation(loc, kind, sec)

    arch = cfg.getSettings("General").values()[0]
    rgm.addResourceLocation(arch + "/materials/programs/GLSL", "FileSystem", "General")
    arch += "/RTShaderLib"
    rgm.addResourceLocation(arch + "/materials", "FileSystem", "General")
    rgm.addResourceLocation(arch + "/GLSL", "FileSystem", "General")

    if not root.restoreConfig():
        root.showConfigDialog(Ogre.ConfigDialog())

    win = root.initialise(True)

    OgreRTShader.ShaderGenerator.initialize()
    shadergen = OgreRTShader.ShaderGenerator.getSingleton()

    sgres = SGResolver(shadergen)
    Ogre.MaterialManager.getSingleton().addListener(sgres)

    rgm.initialiseAllResourceGroups()

    rs = shadergen.getRenderState(OgreRTShader.cvar.ShaderGenerator_DEFAULT_SCHEME_NAME)
    rs.addTemplateSubRenderState(shadergen.createSubRenderState(OgreRTShader.cvar.PerPixelLighting_Type));

    scn_mgr = root.createSceneManager(Ogre.ST_GENERIC)
    shadergen.addSceneManager(scn_mgr)

    scn_mgr.setAmbientLight(Ogre.ColourValue(.1, .1, .1))

    light = scn_mgr.createLight("MainLight")
    lightnode = scn_mgr.getRootSceneNode().createChildSceneNode()
    lightnode.setPosition(0, 10, 15)
    lightnode.attachObject(light)

    cam = scn_mgr.createCamera("myCam")
    cam.setPosition(0, 0, 15)
    cam.setNearClipDistance(5)
    cam.lookAt(0, 0, -1)
    vp = win.addViewport(cam)
    vp.setBackgroundColour(Ogre.ColourValue(.3, .3, .3))

    ent = scn_mgr.createEntity("Sinbad.mesh")
    node = scn_mgr.getRootSceneNode().createChildSceneNode()
    node.attachObject(ent)

    root.startRendering()

if __name__ == "__main__":
    main()
