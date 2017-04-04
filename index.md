<p class="header" align="center"><img alt="" src="ogre-logo.png"></p>

# Summary
OGRE is a scene-oriented, flexible 3D engine written in C++ designed to make it easier and more intuitive for developers to produce games and demos utilising 3D hardware. The class library abstracts all the details of using the underlying system libraries like Direct3D and OpenGL and provides an interface based on world objects and other intuitive classes.

* [Features](http://www.ogre3d.org/about/features)
* [Manual](https://ogrecave.github.io/ogre/api/1.10/manual.html)

# Latest release
The latest stable release is **v1.10.3**. For more information see:

* [Download v1.10.3](https://github.com/OGRECave/ogre/releases/tag/v1.10.3)
* [API documentation](https://ogrecave.github.io/ogre/api/1.10/)
* [New and Noteworthy](https://github.com/OGRECave/ogre/blob/v1.10.3/Docs/1.10-Notes.md) (compared to v1.9)
* [API/ ABI changes](https://abi-laboratory.pro/tracker/timeline/ogre/) (since v1.7)

## Try it
* [Online Emscripten Demo](https://ogrecave.github.io/ogre/emscripten/) - [Web Assembly & WebGL2](https://ogrecave.github.io/ogre/wasm/)
* Ubuntu Snap package: `sudo snap install ogre`

## OpenGL RenderSystem status
Currently only the legacy GL RenderSystem is able to pass all VTests from the suite. This is partly due to the tests relying on Cg which the other RenderSystems do not support. Still it is considered the gold standard. See below how the GLES2 and GL3Plus RenderSystems behave in comparison:

* [GL3Plus vs GL](https://ogrecave.github.io/ogre/gl_status/TestResults_GL3Plus.html)
* [GLES2 vs GL](https://ogrecave.github.io/ogre/gl_status/TestResults_GLES2.html)
* [GLES2 vs GL3Plus](https://ogrecave.github.io/ogre/gl_status/TestResults_GLES22.html)

# Additional information

If you want to dive deeper into the OGRE ecosystem here are your options.

## Literature
There are 2 books written on OGRE

* [OGRE 3D 1.7 Beginner's Guide](https://www.packtpub.com/game-development/ogre-3d-17-beginners-guide)
* [Pro OGRE 3D Programming](https://www.apress.com/us/book/9781590597101)

## Useful Add-Ons
Beneath the OGRECave umbrella you can also find some useful OGRE Add-Ons like

* [Ogitor SceneBuilder](https://github.com/OGRECave/ogitor)
* [Assimp importer tool](https://github.com/OGRECave/ogre-assimp)
* [Blender Exporter](https://github.com/OGRECave/blender2ogre)
* [Syntax highlighting for Ogre Scripts for Atom](https://github.com/OGRECave/language-ogre-script)
* [Particle Universe](https://github.com/OGRECave/particleuniverse)
