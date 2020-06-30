import Ogre
import Ogre.Bites
import Ogre.RTShader

import numpy as np
#from matplotlib import pyplot

def main():
    app = Ogre.Bites.ApplicationContext("PySample")
    app.initApp()
    
    root = app.getRoot()
    scn_mgr = root.createSceneManager()
    
    shadergen = Ogre.RTShader.ShaderGenerator.getSingleton()
    shadergen.addSceneManager(scn_mgr)
    
    ## [numpy_image]
    arr = np.zeros((256, 256, 3), dtype=np.uint8)
    arr[:,:,1] = np.mgrid[0:256,0:256][1]
    ## [numpy_image]
    
    ## [np_to_ogre]
    ogre_img = Ogre.Image()
    ogre_img.loadDynamicImage(arr, 256, 256, Ogre.PF_BYTE_RGB)
    
    Ogre.TextureManager.getSingleton().loadImage("gradient", "General", ogre_img)
    ## [np_to_ogre]

    ## [apply_to_rect]
    mat = Ogre.MaterialManager.getSingleton().create("gradient_mat", "General")
    rpass = mat.getTechniques()[0].getPasses()[0]
    rpass.setLightingEnabled(False)
    rpass.createTextureUnitState("gradient")

    rect = Ogre.Rectangle2D(True)
    rect.setCorners(-0.5, 0.5, 0.5, -0.5) # in normalized screen space
    rect.setMaterial(mat)
    rect.setBoundingBox(Ogre.AxisAlignedBox.BOX_INFINITE)

    scn_mgr.getRootSceneNode().createChildSceneNode().attachObject(rect)
    ## [apply_to_rect]
    
    cam = scn_mgr.createCamera("myCam")
    win = app.getRenderWindow()
    vp = win.addViewport(cam)
    
    ## [py_to_primitive]
    gray = np.array([0.3, 0.3, 0.3])
    vp.setBackgroundColour(gray)
    ## [py_to_primitive]

    root.startRendering()
    
    ## [ogre_to_np]
    mem = np.empty((win.getHeight(), win.getWidth(), 3), dtype=np.uint8)
    pb = Ogre.PixelBox(win.getWidth(), win.getHeight(), 1, Ogre.PF_BYTE_RGB, mem)
    win.copyContentsToMemory(pb, pb)
    ## [ogre_to_np]
    
    ## [zero_copy_view]
    pyplot.imsave("screenshot.png", mem)
    ## [zero_copy_view]

if __name__ == "__main__":
    main()
