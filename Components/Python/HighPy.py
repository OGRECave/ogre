# This file is part of the OGRE project.
# It is subject to the license terms in the LICENSE file found in the top-level directory
# of this distribution and at https://www.ogre3d.org/licensing.
# SPDX-License-Identifier: MIT

import os
import math

import Ogre
import Ogre.Bites
import Ogre.RTShader
import Ogre.Overlay

## \cond PRIVATE

class _InputListener(Ogre.Bites.InputListener):
    def __init__(self, window_data):
        Ogre.Bites.InputListener.__init__(self)
        self.window_data = window_data

    def keyPressed(self, evt):
        self.window_data.last_key = evt.keysym.sym
        return True

class _ImGuiDispatcher(Ogre.RenderTargetListener):
    def __init__(self, callback):
        Ogre.RenderTargetListener.__init__(self)
        self.callback = callback

    def preRenderTargetUpdate(self, _):
        Ogre.Overlay.ImGuiOverlay.NewFrame()
        self.callback()

class _WindowData:
    def __init__(self, window):
        self.window = window.render
        self.camera = None
        self.scn_mgr = None
        self.background = None
        self.last_key = 0

        self._input_listener = None
        self._native = window.native

    def listen_to_input(self):
        assert self._input_listener is None
        self._input_listener = _InputListener(self)
        _ctx.addInputListener(self._native, self._input_listener)

class _Application(Ogre.Bites.ApplicationContext):
    def __init__(self):
        # warnings and errors only
        if not os.environ.get("OGRE_MIN_LOGLEVEL"):
            os.environ["OGRE_MIN_LOGLEVEL"] = "3"

        Ogre.Bites.ApplicationContext.__init__(self, "OgreHighPy")

        self.name = None
        self.imsize = None
        self.imgui_dispatcher = None

        self.windows = {}

        self.createRoot()

    def createWindow(self, name, w, h, *_args):
        if not self.windows:
            name = self.name
            w, h = self.imsize

        misc = Ogre.NameValueMap()
        ret = Ogre.Bites.ApplicationContext.createWindow(self, name, w, h, misc)
        self.windows[name] = _WindowData(ret)
        return ret

    def createRoot(self):
        if self.getRoot():
            return
        Ogre.Bites.ApplicationContext.createRoot(self)

        rgm = Ogre.ResourceGroupManager.getSingleton()
        assert isinstance(user_resource_locations, set)
        if not user_resource_locations:
            rgm.addResourceLocation(".", "FileSystem", Ogre.RGN_DEFAULT, True)

        for loc in user_resource_locations:
            rgm.addResourceLocation(loc, "FileSystem", Ogre.RGN_DEFAULT)

    def oneTimeConfig(self):
        self.getRoot().restoreConfig()
        rs = self.getRoot().getRenderSystemByName("OpenGL 3+ Rendering Subsystem")
        self.getRoot().setRenderSystem(rs)
        self.getRoot().saveConfig()
        return True

def _create_image_background(scn_mgr, name):
    tex = Ogre.TextureManager.getSingleton().create(name, Ogre.RGN_DEFAULT, True)
    tex.setNumMipmaps(0)

    mat = Ogre.MaterialManager.getSingleton().create(name, Ogre.RGN_DEFAULT)
    mat.getTechnique(0).getPass(0).createTextureUnitState().setTexture(tex)
    mat.getTechnique(0).getPass(0).setDepthWriteEnabled(False)
    mat.getTechnique(0).getPass(0).setLightingEnabled(False)

    rect = scn_mgr.createScreenSpaceRect(True)
    rect.setMaterial(mat)
    rect.setRenderQueueGroup(Ogre.RENDER_QUEUE_BACKGROUND)
    scn_mgr.getRootSceneNode().attachObject(rect)

    return tex

_ctx = None

def _init_ogre(window_name, imsize):
    global _ctx

    _ctx = _Application()
    _ctx.name = window_name
    _ctx.imsize = imsize
    _ctx.initApp()

def _init_window(window_data, flags):
    scn_mgr = _ctx.getRoot().createSceneManager()
    window_data.scn_mgr = scn_mgr
    Ogre.RTShader.ShaderGenerator.getSingleton().addSceneManager(scn_mgr)

    window_data.camera = scn_mgr.createCamera("camera")
    window_data.camera.setAutoAspectRatio(True)

    camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
    camnode.attachObject(window_data.camera)
    if flags & AXES_ZFORWARD_YDOWN:
         # convert from AXES_ZBACKWARD_YUP (Ogre default)
        camnode.setOrientation(Ogre.Quaternion(Ogre.Math.PI, (1, 0, 0)))
    window_data.window.addViewport(window_data.camera)

    window_data.listen_to_input()

## \endcond

## \addtogroup Optional
# @{
# \addtogroup Python
# @{


## %Ogre & OpenGL coordinate system
AXES_ZBACKWARD_YUP  = 0
## OpenCV & colmap coordinate system
AXES_ZFORWARD_YDOWN = 1

## add paths here to override the default resource location @c "."
user_resource_locations = set()

def window_create(window_name: str, window_size, flags: int = AXES_ZBACKWARD_YUP):
    """!
    create a window
    @param window_name: name of the window
    @param window_size: size of the window in pixels
    @param flags: window create options
    """
    if _ctx is None:
        _init_ogre(window_name, window_size)
    else:
        _ctx.createWindow(window_name, *window_size)

    _init_window(_ctx.windows[window_name], flags)

def window_draw(window_name: str) -> int:
    """!
    draw the window
    @param window_name: name of the window
    @return last key pressed inside the window
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"
    _ctx.pollEvents()

    window_data = _ctx.windows[window_name]
    window_data.window.update()
    ret = window_data.last_key
    window_data.last_key = 0
    return ret

def window_use_imgui(window_name: str, callback):
    """!
    enable imgui for the window
    @param window_name: name of the window
    @param callback: function to call for imgui rendering
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"
    assert _ctx.imgui_dispatcher is None, "imgui already enabled. Only one window can have imgui enabled at a time."

    _ctx.imgui_dispatcher = _ImGuiDispatcher(callback)

    overlay = _ctx.initialiseImGui()
    window_data = _ctx.windows[window_name]
    window_data.scn_mgr.addRenderQueueListener(_ctx.getOverlaySystem())
    window_data.window.addListener(_ctx.imgui_dispatcher)
    overlay.show()
    _ctx.addInputListener(_ctx.getImGuiInputListener())

def window_compositor(window_name: str, compositor_name: str):
    """!
    set a compositor for the window
    @param window_name: name of the window
    @param compositor_name: name of the compositor
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    window_data = _ctx.windows[window_name]

    cm = Ogre.CompositorManager.getSingleton()
    vp = window_data.window.getViewport(0)

    cm.addCompositor(vp, compositor_name)
    cm.setCompositorEnabled(vp, compositor_name, True)

def window_pixel_data(window_name: str, compositor_name: str | None = None, texture_name: str | None = None, mrt_index: int = 0):
    """!
    get the pixel data from the window or the active compositor
    @param window_name: name of the window
    @param compositor_name: name of the compositor
    @param texture_name: name of the texture
    @param mrt_index: index of the MRT
    @retval 0: bytearray holding the pixel data
    @retval 1: tuple `(bytes per pixel, height, width, channels)`
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    window_data = _ctx.windows[window_name]
    rtarget = window_data.window
    dst_type = Ogre.PF_BYTE_RGB

    if compositor_name:
        cm = Ogre.CompositorManager.getSingleton()
        chain = cm.getCompositorChain(rtarget.getViewport(0))
        assert chain, "no active compositors"
        inst = chain.getCompositor(compositor_name)
        assert inst, f"no compositor named: {compositor_name}"
        tex = inst.getTextureInstance(texture_name, mrt_index)
        assert tex, f"no texture named: {texture_name}"

        dst_type = tex.getFormat()
        rtarget = tex.getBuffer().getRenderTarget()

    if dst_type == Ogre.PF_BYTE_RGB:
        shape = (1, rtarget.getHeight(), rtarget.getWidth(), 3)
    elif dst_type == Ogre.PF_BYTE_RGBA:
        shape = (1, rtarget.getHeight(), rtarget.getWidth(), 4)
    elif dst_type == Ogre.PF_L16 or dst_type == Ogre.PF_DEPTH16:
        shape = (2, rtarget.getHeight(), rtarget.getWidth(), 1)
    elif dst_type == Ogre.PF_FLOAT32_R or dst_type == Ogre.PF_DEPTH32F:
        shape = (4, rtarget.getHeight(), rtarget.getWidth(), 1)
    else:
        raise ValueError(f"unsupported format: {Ogre.PixelUtil.getFormatName(dst_type)}")

    ret = bytearray(math.prod(shape))
    pb = Ogre.PixelBox(shape[2], shape[1], 1, dst_type, ret)
    rtarget.copyContentsToMemory(pb, pb)

    return ret, shape

def imshow(window_name: str, img_path: str | os.PathLike):
    """!
    show an image in the window
    @param window_name: name of the window
    @param img_path: path to the image file
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    window_data = _ctx.windows[window_name]
    if window_data.background is None:
        window_data.background = _create_image_background(_ctx.windows[window_name].scn_mgr, window_name)

    img = Ogre.Image()
    img.load(str(img_path), Ogre.RGN_DEFAULT)

    window_data.background.loadImage(img)

def camera_intrinsics(window_name: str, K, imsize):
    """!
    set camera intrinsics
    @param window_name: name of the window
    @param K: 3x3 camera matrix
    @param imsize: image size in pixels
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    cam = _ctx.windows[window_name].camera
    cam.setAspectRatio(imsize[0]/imsize[1])

    zNear = cam.getNearClipDistance()
    top = zNear * K[1][2] / K[1][1]
    left = -zNear * K[0][2] / K[0][0]
    right = zNear * (imsize[0] - K[0][2]) / K[0][0]
    bottom = -zNear * (imsize[1] - K[1][2]) / K[1][1]

    cam.setFrustumExtents(left, right, top, bottom)

    fovy = math.atan2(K[1][2], K[1][1]) + math.atan2(imsize[1] - K[1][2], K[1][1])
    cam.setFOVy(fovy)

def mesh_show(window_name: str, mesh_path: str | os.PathLike, rot_mat = None, position = (0, 0, 0), material_name: str | None = None):
    """!
    show a mesh in the window
    @param window_name: name of the window
    @param mesh_path: path to the mesh file
    @param rot_mat: 3x3 rotation matrix
    @param position: 3x1 translation vector
    @param material_name: optional material name to use instead of the default
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    mesh_path = str(mesh_path)
    scn_mgr = _ctx.windows[window_name].scn_mgr
    if scn_mgr.hasEntity(mesh_path):
        ent = scn_mgr.getEntity(mesh_path)
    else:
        ent = scn_mgr.createEntity(mesh_path, mesh_path, Ogre.RGN_DEFAULT)
        scn_mgr.getRootSceneNode().createChildSceneNode().attachObject(ent)

    if material_name:
        ent.setMaterialName(material_name, Ogre.RGN_DEFAULT)

    scn_node = ent.getParentSceneNode()
    ent.setVisible(True)

    scn_node._update(True, False)
    diam = scn_node._getWorldAABB().getSize().length()

    cam = _ctx.windows[window_name].camera
    cam.setNearClipDistance(min(cam.getNearClipDistance(), diam*0.01))

    if rot_mat is not None:
        flat = [v for row in rot_mat for v in row]
        rot_mat = Ogre.Matrix3(*flat)
        scn_node.setOrientation(Ogre.Quaternion(rot_mat))

    scn_node.setPosition(position)

def mesh_hide(window_name: str, mesh_path: str | os.PathLike):
    """!
    hide a mesh in the window
    @param window_name: name of the window
    @param mesh_path: path to the mesh file
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    scn_mgr = _ctx.windows[window_name].scn_mgr
    mesh_path = str(mesh_path)
    # hiding non-existent mesh has no effect
    if scn_mgr.hasEntity(mesh_path):
        ent = scn_mgr.getEntity(mesh_path)
        ent.setVisible(False)

def point_light(window_name: str, position = (0, 0, 0)):
    """!
    create a point light in the window
    @param window_name: name of the window
    @param position: position of the light in world coordinates
    """
    assert _ctx is not None, "call window_create first"
    assert window_name in _ctx.windows, f"no window named: {window_name}"

    scn_mgr = _ctx.windows[window_name].scn_mgr
    if scn_mgr.hasLight("point_light"):
        light_node = _ctx.scn_mgr.getLight("point_light").getParentSceneNode()
    else:
        light = scn_mgr.createLight("point_light", Ogre.Light.LT_POINT)
        light_node = scn_mgr.getRootSceneNode().createChildSceneNode()
        light_node.attachObject(light)

    light_node.setPosition(*position)

## @}
## @}
