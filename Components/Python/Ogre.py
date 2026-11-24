# The OgreMain bindings are split into several native modules, one per part of
# OgreMain/include/Ogre.i, so the generated wrappers compile in parallel.
# Re-export them as a single module. The order matters: each part builds on the
# types of the previous ones.
import typing
from ._OgreCore import *
from ._OgreResources import *
from ._OgreMaterials import *
from ._OgreAnimation import *
from ._OgreScene import *
from ._OgreCore import __version__
