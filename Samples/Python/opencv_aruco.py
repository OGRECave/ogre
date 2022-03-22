import math

import numpy as np
import cv2

import Ogre
import Ogre.Bites
import Ogre.RTShader

def set_camera_intrinsics(cam, K, imsize):
    cam.setAspectRatio(imsize[0]/imsize[1])

    zNear = cam.getNearClipDistance()
    top = zNear * K[1, 2] / K[1, 1]
    left = -zNear * K[0, 2] / K[0, 0]
    right = zNear * (imsize[0] - K[0, 2]) / K[0, 0]
    bottom = -zNear * (imsize[1] - K[1, 2]) / K[1, 1]

    cam.setFrustumExtents(left, right, top, bottom)

    fovy = math.atan2(K[1, 2], K[1, 1]) + math.atan2(imsize[1] - K[1, 2], K[1, 1])
    cam.setFOVy(fovy)

def create_image_background(scn_mgr):
    tex = Ogre.TextureManager.getSingleton().create("bgtex", Ogre.RGN_DEFAULT, True)
    tex.setNumMipmaps(0)

    mat = Ogre.MaterialManager.getSingleton().create("bgmat", Ogre.RGN_DEFAULT)
    mat.getTechnique(0).getPass(0).createTextureUnitState().setTexture(tex)
    mat.getTechnique(0).getPass(0).setDepthWriteEnabled(False)
    mat.getTechnique(0).getPass(0).setLightingEnabled(False)

    rect = scn_mgr.createScreenSpaceRect(True)
    rect.setMaterial(mat)
    rect.setRenderQueueGroup(Ogre.RENDER_QUEUE_BACKGROUND)
    scn_mgr.getRootSceneNode().attachObject(rect)

    return tex

def main(ctx):
    ## random calibration data. your mileage may vary
    imsize = (800, 600)
    K = cv2.getDefaultNewCameraMatrix(np.diag([800, 800, 1]), imsize, True)

    ## setup Ogre for AR
    scn_mgr = ctx.getRoot().createSceneManager()
    Ogre.RTShader.ShaderGenerator.getSingleton().addSceneManager(scn_mgr)

    cam = scn_mgr.createCamera("camera")
    cam.setNearClipDistance(5)
    ctx.getRenderWindow().addViewport(cam)

    camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
    # convert OpenCV to OGRE coordinate system
    camnode.rotate((1, 0, 0), math.pi)
    camnode.attachObject(cam)

    set_camera_intrinsics(cam, K, imsize)
    bgtex = create_image_background(scn_mgr)

    ## setup 3D scene
    scn_mgr.setAmbientLight((.1, .1, .1))
    scn_mgr.getRootSceneNode().createChildSceneNode().attachObject(scn_mgr.createLight())

    marker_node = scn_mgr.getRootSceneNode().createChildSceneNode()
    mesh_node = marker_node.createChildSceneNode()
    mesh_node.attachObject(scn_mgr.createEntity("Sinbad.mesh"))
    mesh_node.rotate((1, 0, 0), math.pi/2)
    mesh_node.translate((0, 0, 5))
    mesh_node.setVisible(False)

    ## video capture
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, imsize[0])
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, imsize[1])

    ## aruco
    adict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
    cv2.imshow("marker", cv2.aruco.drawMarker(adict, 0, 400))

    rvec, tvec = None, None
    while cv2.waitKey(1) != 27:
        cont, img = cap.read()
        if not cont: break

        im = Ogre.Image(Ogre.PF_BYTE_BGR, img.shape[1], img.shape[0], 1, img, False)
        if bgtex.getBuffer():
            bgtex.getBuffer().blitFromMemory(im.getPixelBox())
        else:
            bgtex.loadImage(im)

        corners, ids = cv2.aruco.detectMarkers(img, adict)[:2]

        if ids is not None:
            rvecs, tvecs = cv2.aruco.estimatePoseSingleMarkers(corners, 5, K, None)[:2]
            rvec, tvec = rvecs[0].ravel(), tvecs[0].ravel()

            ax = Ogre.Vector3(*rvec)
            ang = ax.normalise()
            marker_node.setOrientation(Ogre.Quaternion(ang, ax))
            marker_node.setPosition(tvec)
            mesh_node.setVisible(True)

        ctx.getRoot().renderOneFrame()

ctx = Ogre.Bites.ApplicationContext()
ctx.initApp()
main(ctx)
ctx.closeApp()