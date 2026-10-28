"""Tests of the Ogre Python bindings, run from the directory of the _Ogre module."""

import os
import sys
import unittest

sys.path.insert(0, os.getcwd())
import _Ogre as Ogre  # noqa: E402


class SequenceTest(unittest.TestCase):
    def test_index_out_of_range(self):
        for value in (Ogre.Vector3(1, 2, 3), Ogre.Quaternion(1, 0, 0, 0), Ogre.ColourValue(1, 0, 0, 1)):
            n = len(value)
            self.assertEqual(len(list(value)), n)
            with self.assertRaises(IndexError):
                value[n]
            with self.assertRaises(IndexError):
                value[n] = 1

    def test_sequence_arguments(self):
        plane = Ogre.Plane(Ogre.Vector3(0, 1, 0), 0)
        self.assertEqual(plane.getDistance((0, 5, 0)), 5)
        self.assertEqual(plane.getDistance([0, 5, 0]), 5)

        # getDistance is not overloaded, so only the argument conversion checks the input
        for invalid in ((0, 5), (0, 5, 0, 9), 5, (x for x in (0, 5, 0)), ("a", 1, 1)):
            with self.assertRaises(TypeError):
                plane.getDistance(invalid)


if __name__ == "__main__":
    unittest.main()
