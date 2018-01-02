
from subprocess import call

call( ["python2", \
        "clone_datablock.py", \
        "../../Components/Hlms/Pbs/include/OgreHlmsPbsDatablock.h", \
        "-I", "../../Components/Hlms/Pbs/include", \
        "-I", "../../OgreMain/include/", \
        "-I", "../../build/include", \
        "-I", "../../build/Debug/include", \
        "-I", "../../build/Release/include", \
        ] )
call( ["python2", \
        "clone_datablock.py", \
        "../../Components/Hlms/Unlit/include/OgreHlmsUnlitDatablock.h", \
        "-I", "../../Components/Hlms/Unlit/include", \
        "-I", "../../OgreMain/include/", \
        "-I", "../../build/include", \
        "-I", "../../build/Debug/include", \
        "-I", "../../build/Release/include", \
        ] )
call( ["python2", \
        "clone_datablock.py", \
        "../../Samples/2.0/Tutorials/Tutorial_Terrain/include/Terra/Hlms/OgreHlmsTerraDatablock.h", \
        "-I", "../../Samples/2.0/Tutorials/Tutorial_Terrain/include", \
        "-I", "../../Samples/2.0/Tutorials/Tutorial_Terrain/include/Terra/Hlms", \
        "-I", "../../OgreMain/include/", \
        "-I", "../../build/include", \
        "-I", "../../build/Debug/include", \
        "-I", "../../build/Release/include", \
        ] )
