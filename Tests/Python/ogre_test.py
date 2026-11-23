"""Tests of the Ogre Python bindings, run from the directory of the _Ogre module."""

import gc
import os
import sys
import tempfile
import unittest
import weakref
from pathlib import Path

sys.path.insert(0, os.getcwd())
import _Ogre as Ogre  # noqa: E402

try:
    import _Bites as Bites  # noqa: E402
except ImportError:
    Bites = None

try:
    import numpy as np  # noqa: E402
except ImportError:
    np = None


class SequenceTest(unittest.TestCase):
    def test_index_out_of_range(self):
        for value in (Ogre.Vector3(1, 2, 3), Ogre.Quaternion(1, 0, 0, 0), Ogre.ColourValue(1, 0, 0, 1)):
            n = len(value)
            self.assertEqual(len(list(value)), n)
            with self.assertRaises(IndexError):
                value[n]
            with self.assertRaises(IndexError):
                value[n] = 1

    def test_matrix_index(self):
        m = Ogre.Matrix4(1, 0, 0, 1, 0, 1, 0, 2, 0, 0, 1, 3, 0, 0, 0, 1)
        self.assertEqual(m[0, 0], 1)
        self.assertEqual(m[1, 3], 2)
        with self.assertRaises(IndexError):
            m[4, 0]
        for invalid in (0, (0,), (0, 1, 2), ("a", 0)):
            with self.assertRaises(TypeError):
                m[invalid]

    def test_sequence_arguments(self):
        plane = Ogre.Plane(Ogre.Vector3(0, 1, 0), 0)
        self.assertEqual(plane.getDistance((0, 5, 0)), 5)
        self.assertEqual(plane.getDistance([0, 5, 0]), 5)

        # getDistance is not overloaded, so only the argument conversion checks the input
        for invalid in ((0, 5), (0, 5, 0, 9), 5, (x for x in (0, 5, 0)), ("a", 1, 1)):
            with self.assertRaises(TypeError):
                plane.getDistance(invalid)


@unittest.skipUnless(np, "numpy not installed")
class NumpyTest(unittest.TestCase):
    def test_scalar_arguments(self):
        # np.float32 and np.int64 are not float or int subclasses
        pts = np.array([[1, 2, 3]], dtype=np.float32)
        self.assertEqual(Ogre.Vector3(*pts[0]), Ogre.Vector3(1, 2, 3))
        self.assertEqual(Ogre.Vector3(*np.arange(3)), Ogre.Vector3(0, 1, 2))
        self.assertEqual(Ogre.Radian(np.float32(0.5)).valueRadians(), 0.5)
        self.assertEqual(Ogre.MemoryDataStream(np.uint32(16)).size(), 16)

        # floats select the float overload, integers the uint8 overload
        buf = bytearray(4)
        Ogre.PixelUtil.packColour(*np.array([1, 0, 0, 1], np.float32), Ogre.PF_BYTE_RGBA, buf)
        self.assertEqual(list(buf), [255, 0, 0, 255])
        Ogre.PixelUtil.packColour(*np.array([1, 0, 0, 1], np.uint8), Ogre.PF_BYTE_RGBA, buf)
        self.assertEqual(list(buf), [1, 0, 0, 1])

        with self.assertRaises(TypeError):
            Ogre.MemoryDataStream(np.float32(16))


class BufferLifetimeTest(unittest.TestCase):
    """Image and PixelBox only borrow the pixels, so the wrapper must keep the buffer alive."""

    def test_buffer_lifetime(self):
        import gc
        import weakref

        for wrap in ("Image", "ImageConstructor", "PixelBox"):
            # a memoryview is the stdlib buffer type that supports both ndim and weak references
            data = memoryview(bytearray(8 * 8 * 4)).cast("B", (8, 8, 4))
            ref = weakref.ref(data)
            if wrap == "Image":
                obj = Ogre.Image()
                obj.loadDynamicImage(data)
            elif wrap == "ImageConstructor":
                # must not take ownership of the buffer, which is not allocated by Ogre
                obj = Ogre.Image(Ogre.PF_BYTE_RGBA, 8, 8, 1, data)
            else:
                obj = Ogre.PixelBox(data)
            del data
            gc.collect()
            self.assertIsNotNone(ref())
            self.assertEqual(obj.getWidth(), 8)
            del obj
            gc.collect()
            self.assertIsNone(ref())

    def test_pixelbox_keeps_image_alive(self):
        img = Ogre.Image(Ogre.PF_BYTE_RGBA, 8, 8)
        img.setColourAt(Ogre.ColourValue(1, 0, 0, 1), 0, 0, 0)
        ref = weakref.ref(img)
        box = img.getPixelBox()
        del img
        gc.collect()
        self.assertIsNotNone(ref())
        sub = box.getSubVolume(Ogre.Box(0, 0, 4, 4))
        del box
        gc.collect()
        self.assertIsNotNone(ref())
        self.assertEqual(sub.getColourAt(0, 0, 0), Ogre.ColourValue(1, 0, 0, 1))
        del sub
        gc.collect()
        self.assertIsNone(ref())


@unittest.skipUnless(Bites, "OgreBites bindings not built")
class ListenerChainTest(unittest.TestCase):
    def test_chain_keeps_its_listeners_alive(self):
        class Listener(Bites.InputListener):
            pass

        listener = Listener()
        ref = weakref.ref(listener)
        chain = Bites.InputListenerChain([listener])
        del listener
        gc.collect()
        self.assertIsNotNone(ref())

        del chain
        gc.collect()
        self.assertIsNone(ref())


@unittest.skipUnless(Bites, "OgreBites bindings not built")
class CloseAppTest(unittest.TestCase):
    def test_close_without_root_is_safe(self):
        app = Bites.ApplicationContextBase("CloseAppTest")
        app.closeApp()
        app.closeApp()


def _plugins_cfg():
    cwd = Path.cwd()
    for path in (cwd / "plugins.cfg", cwd.parent / "bin" / "plugins.cfg"):
        if path.is_file():
            return path
    return None


@unittest.skipUnless(Bites and _plugins_cfg(), "plugins.cfg or OgreBites not found")
class CameraManTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        root = Path(cls.tmp.name)
        cls.root = Ogre.Root(str(_plugins_cfg()), str(root / "ogre.cfg"),
                             str(root / "ogre.log"))
        cls.scene = cls.root.createSceneManager("DefaultSceneManager")

    @classmethod
    def tearDownClass(cls):
        cls.root.shutdown()
        del cls.root
        cls.tmp.cleanup()

    def test_python_mouse_handlers_are_called_through_the_listener(self):
        class Cam(Bites.CameraMan):
            def __init__(self, node):
                super().__init__(node)
                self.calls = []

            def mouseMoved(self, evt):
                self.calls.append("moved")
                return True

            def mousePressed(self, evt):
                self.calls.append("pressed")
                return True

        node = self.scene.getRootSceneNode().createChildSceneNode()
        cam = Cam(node)
        chain = Bites.InputListenerChain([cam])

        moved = Bites.MouseMotionEvent()
        moved.x, moved.y, moved.xrel, moved.yrel = 1, 2, 3, 4
        pressed = Bites.MouseButtonEvent()
        pressed.x, pressed.y, pressed.button, pressed.clicks = 1, 2, Bites.BUTTON_LEFT, 1

        self.assertTrue(chain.mouseMoved(moved))
        self.assertTrue(chain.mousePressed(pressed))
        self.assertEqual(cam.calls, ["moved", "pressed"])


if __name__ == "__main__":
    unittest.main()
