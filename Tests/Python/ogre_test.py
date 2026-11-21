"""Tests of the Ogre Python bindings, run from the directory of the _Ogre module."""

import os
import sys
import unittest

sys.path.insert(0, os.getcwd())
import _Ogre as Ogre  # noqa: E402

try:
    import _Bites as Bites  # noqa: E402
except ImportError:
    Bites = None


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


@unittest.skipUnless(Bites, "OgreBites bindings not built")
class CloseAppTest(unittest.TestCase):
    def test_close_without_root_is_safe(self):
        app = Bites.ApplicationContextBase("CloseAppTest")
        app.closeApp()
        app.closeApp()


if __name__ == "__main__":
    unittest.main()
