import Ogre

import ctypes
import numpy.ctypeslib as npc
import numpy as np

def AsDataStream(arr):
    """
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
    """
    writable numpy view to the ogre data types
    
    take care that the ogre type does not get released while the view is used.
    e.g. this is invalid
    
    v = Ogre.Vector3()
    return OgreNumpy.view(v)
    
    instead do
    return OgreNumpy.view(v).copy()
    
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
