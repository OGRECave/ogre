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


if __name__ == "__main__":
    unittest.main()
