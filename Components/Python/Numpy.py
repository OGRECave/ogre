# This file is part of the OGRE project.
# It is subject to the license terms in the LICENSE file found in the top-level directory
# of this distribution and at https://www.ogre3d.org/licensing.
# SPDX-License-Identifier: MIT

import Ogre

import ctypes
import numpy.ctypeslib as npc
import numpy as np

## \addtogroup Optional
# @{
# \defgroup Python Python
# Convenience functions for python
# @{

def AsDataStream(arr):
    """!
    copy numpy array to Ogre.MemoryDataStream that can be used in Ogre
    @param arr: some numpy array
    """
    size = int(np.prod(arr.shape) * arr.dtype.itemsize)
    ret = Ogre.MemoryDataStream(size)
    tp = ctypes.POINTER(ctypes.c_ubyte)
    np_view = npc.as_array(ctypes.cast(int(ret.getPtr()), tp), (size, ))
    np_view[:] = arr.ravel().view(np.ubyte)
    
    return ret

def view(o):
    """!
    writable numpy view to the ogre data types
    
    take care that the ogre type does not get released while the view is used.
    e.g. this is invalid
    
    ```py
    v = Ogre.Vector3()
    return Ogre.Numpy.view(v)
    ```

    instead do
    ```py
    return Ogre.Numpy.view(v).copy()
    ```

    to pass numpy arrays into Ogre use AsDataStream()
    """
    tp = ctypes.POINTER(ctypes.c_float)
    
    if isinstance(o, Ogre.Vector2):
        shape = (2,)
        ptr = o.this
    elif isinstance(o, Ogre.Vector3):
        shape = (3,)
        ptr = o.this
    elif isinstance(o, Ogre.Vector4):
        shape = (4,)
        ptr = o.this
    elif isinstance(o, Ogre.Matrix3):
        shape = (3, 3)
        ptr = o.this
    elif isinstance(o, Ogre.Affine3):
        shape = (3, 4)
        ptr = o.this
    elif isinstance(o, Ogre.Matrix4):
        shape = (4, 4)
        ptr = o.this
    elif isinstance(o, Ogre.PixelBox):
        tp = ctypes.POINTER(ctypes.c_uint8)
        shape = (o.getHeight(), o.getWidth(), Ogre.PixelUtil.getNumElemBytes(o.format))
        ptr = o.data
    else:
        raise TypeError("do not know how to map '{}'".format(type(o).__name__))
         
    return npc.as_array(ctypes.cast(int(ptr), tp), shape)

## @}
## @}
