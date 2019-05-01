<p class="header" align="center"><img alt="" src="ogre-logo.png"></p>

# Summary
OGRE is a scene-oriented, flexible 3D engine written in C++ designed to make it easier and more intuitive for developers to produce games and demos utilising 3D hardware. The class library abstracts all the details of using the underlying system libraries like Direct3D and OpenGL and provides an interface based on world objects and other intuitive classes.

* [Features](http://www.ogre3d.org/about/features)
* [Manual](https://ogrecave.github.io/ogre/api/latest/manual.html)

# Latest release
The latest release is [![GitHub release](https://img.shields.io/github/release/ogrecave/ogre.svg)](https://github.com/OGRECave/ogre/releases/latest). For more information see:

* [API documentation](https://ogrecave.github.io/ogre/api/latest/)
* [New and Noteworthy](https://github.com/OGRECave/ogre/blob/master/Docs/1.12-Notes.md)
* [API/ ABI changes](https://abi-laboratory.pro/tracker/timeline/ogre/) (since v1.7)

## Try it
* [Online Emscripten Demo](https://ogrecave.github.io/ogre/emscripten/) - [Web Assembly & WebGL2](https://ogrecave.github.io/ogre/wasm/)
* Ubuntu Snap package: `sudo snap install ogre`
* [Android App on F-Droid](https://f-droid.org/packages/org.ogre.browser/)

## OpenGL RenderSystem status
Currently only the legacy GL RenderSystem is able to pass all VTests from the suite. This is partly due to the tests relying on Cg which the other RenderSystems do not support. Still it is considered the gold standard. See below how the GLES2 and GL3Plus RenderSystems behave in comparison:

* [GL3Plus vs GL](https://ogrecave.github.io/ogre/gl_status/TestResults_GL3Plus.html)
* [GLES2 vs GL](https://ogrecave.github.io/ogre/gl_status/TestResults_GLES2.html)
* [GLES2 vs GL3Plus](https://ogrecave.github.io/ogre/gl_status/TestResults_GLES22.html)
* [GL vs D3D9](https://ogrecave.github.io/ogre/gl_status/TestResults_GL9.html)

# Additional information

If you want to dive deeper into the OGRE ecosystem here are your options.

## Who is using it?

**Open Source**
- [OpenDungeons - real time strategy game](https://opendungeons.github.io/)
- [Rigs of Rods - Soft Body Physics Simulator](https://rigsofrods.org/)
- [Gazebo - Robot simulation](http://gazebosim.org/)
- [ROS 3D Robot Visualizer](http://wiki.ros.org/rviz)
- [OpenSpace 3D - Open Source Platform For 3D Environments](http://www.openspace3d.com/)

**Closed Source**
- [Hob](http://store.steampowered.com/app/404680/Hob/)
- [Torchlight II](http://store.steampowered.com/app/200710/Torchlight_II/)
- [Battlezone 98 Redux](http://store.steampowered.com/app/301650/Battlezone_98_Redux/)
- [Live Home 3D - Home Design Software](https://www.livehome3d.com/)

## Literature
There are 3 books written on OGRE

* [OGRE 3D 1.7 Beginner's Guide](https://www.packtpub.com/game-development/ogre-3d-17-beginners-guide)
* [OGRE 3D 1.7 Application Development Cookbook](https://www.packtpub.com/game-development/ogre-3d-17-application-development-cookbook)
* [Pro OGRE 3D Programming](https://www.apress.com/us/book/9781590597101)

## Useful Add-Ons
Beneath the OGRECave umbrella you can also find some useful OGRE Add-Ons like

* [Ogitor SceneBuilder](https://github.com/OGRECave/ogitor)
* [Assimp importer tool](https://github.com/OGRECave/ogre-assimp)
* [Blender Exporter](https://github.com/OGRECave/blender2ogre)
* [Syntax highlighting for Ogre Scripts for Atom](https://github.com/OGRECave/language-ogre-script)
* [Particle Universe](https://github.com/OGRECave/particleuniverse)
