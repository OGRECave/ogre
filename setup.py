import os
import sys
import os.path
import re
import skbuild
from skbuild import cmaker

# This creates a list which is empty but returns a length of 1.
# Should make the wheel a binary distribution and platlib compliant.
class EmptyListWithLength(list):
    def __len__(self):
        return 1

def cmake_process_manifest_hook(cmake_manifest):
    return [name for name in cmake_manifest
            if os.path.splitext(name)[1] not in (".a", ".h", ".i", ".pc", ".cmake", ".lib")
            or "media/" in name.replace("\\", "/").lower()]

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    os.makedirs("Ogre", exist_ok=True)

    cmaker.CMaker.check_for_bad_installs = lambda: None # barks at our self-built deps

    long_description = open("README.md", encoding="utf-8").read()
    long_description = long_description.replace("![](Other", "![](https://raw.githubusercontent.com/OGRECave/ogre/master/Other")
    long_description = long_description.replace("](Docs", "](https://github.com/OGRECave/ogre/blob/master/Docs")

    cmake_source_dir = "."
    cmake_args = [
        "-DOGRE_LIB_DIRECTORY=Ogre",  # install into Python package
        "-DOGRE_BITES_STATIC_PLUGINS=ON",
        "-DOGRE_NODELESS_POSITIONING=OFF",
        "-DOGRE_BUILD_DEPENDENCIES=ON",
        "-DOGRE_INSTALL_DEPENDENCIES=ON",
        "-DOGRE_BUILD_SAMPLES=OFF",
        "-DOGRE_BUILD_TOOLS=OFF",
        "-DOGRE_BUILD_COMPONENT_CSHARP=OFF",
        "-DOGRE_BUILD_COMPONENT_JAVA=OFF",
        "-DOGRE_BUILD_COMPONENT_PROPERTY=OFF",
        "-DOGRE_BUILD_RENDERSYSTEM_GL=ON",
        "-DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON",
        "-DOGRE_BUILD_RENDERSYSTEM_GLES2=ON",
        "-DOGRE_BUILD_RENDERSYSTEM_TINY=ON",
        "-DOGRE_BUILD_PLUGIN_ASSIMP=ON",
        "-DOGRE_BUILD_PLUGIN_FREEIMAGE=OFF",
        "-DOGRE_BUILD_PLUGIN_EXRCODEC=OFF",
        "-DOGRE_BUILD_PLUGIN_CG=OFF",
        "-DOGRE_BUILD_PLUGIN_BSP=OFF",
        "-DOGRE_BUILD_PLUGIN_PCZ=OFF",
        # not yet wrapped components
        "-DOGRE_BUILD_COMPONENT_MESHLODGENERATOR=OFF",
        "-DOGRE_BUILD_COMPONENT_VOLUME=OFF"
    ]

    if sys.platform == "win32":
        cmake_args += ["-DSWIG_EXECUTABLE=C:/ProgramData/chocolatey/bin/swig.exe",
                       "-DOGRE_BIN_DIRECTORY=Ogre", # direct dlls into python package
                       "-DOGRE_CFG_INSTALL_PATH=bin", # but keep config files in bin, relative to Media
                       "-DOGRE_BUILD_RENDERSYSTEM_D3D9=OFF" # do not require old runtime
                       ]
    elif sys.platform == "linux":
        cmake_args += ["-DOGRE_GLSUPPORT_USE_EGL=ON",
                       "-DCMAKE_CXX_FLAGS=-s", # strip assimp
                       "-DCMAKE_INSTALL_RPATH=$ORIGIN;$ORIGIN/OGRE"]
    elif sys.platform == "darwin":
        cmake_args += ["-DOGRE_BUILD_LIBS_AS_FRAMEWORKS=OFF",
                       "-DCMAKE_INSTALL_RPATH=@loader_path;@loader_path/OGRE"]

    version = re.search("project\(OGRE VERSION (\S+)\)", open("CMakeLists.txt").read()).group(1)
    # version += ".dev0"

    skbuild.setup(
        name="ogre-python",
        version=version,
        url="https://www.ogre3d.org",
        project_urls={
            'Documentation': 'https://ogrecave.github.io/ogre/api/latest/manual.html',
            'Source code': 'https://github.com/OGRECave/ogre',
            'Issues': 'https://github.com/OGRECave/ogre/issues',
            "Funding": "https://www.patreon.com/ogre1"
        },
        license="MIT",
        description="Object-Oriented Graphics Rendering Engine - python package",
        long_description=long_description,
        long_description_content_type="text/markdown",
        packages=["Ogre"],
        package_data={},
        maintainer="Pavel Rojtberg",
        ext_modules=EmptyListWithLength(),
        python_requires=">=3.6",
        classifiers=[
            "Development Status :: 5 - Production/Stable",
            "Environment :: Console",
            "Intended Audience :: Developers",
            "Intended Audience :: Education",
            "Intended Audience :: Information Technology",
            "Intended Audience :: Science/Research",
            "License :: OSI Approved :: MIT License",
            "Operating System :: MacOS",
            "Operating System :: Microsoft :: Windows",
            "Operating System :: POSIX",
            "Operating System :: Unix",
            "Programming Language :: Python",
            "Programming Language :: Python :: 3",
            "Programming Language :: Python :: 3 :: Only",
            "Programming Language :: Python :: 3.8",
            "Programming Language :: C++",
            "Programming Language :: Python :: Implementation :: CPython",
            "Topic :: Scientific/Engineering",
            "Topic :: Multimedia :: Graphics :: 3D Rendering",
            "Topic :: Software Development",
        ],
        cmake_args=cmake_args,
        cmake_source_dir=cmake_source_dir,
        cmake_process_manifest_hook=cmake_process_manifest_hook
    )

if __name__ == "__main__":
    main()
