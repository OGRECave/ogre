[![GitHub release](https://img.shields.io/github/release/ogrecave/ogre.svg)](https://github.com/OGRECave/ogre/releases/latest)
[![Join the chat at https://gitter.im/OGRECave/ogre](https://badges.gitter.im/OGRECave/ogre.svg)](https://gitter.im/OGRECave/ogre?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Patreon](https://img.shields.io/badge/patreon-donate-blue.svg)](https://www.patreon.com/ogre1)

![](Other/ogre_header.png)

## Summary
**OGRE** (Object-Oriented Graphics Rendering Engine) is a
scene-oriented, flexible 3D engine written in C++ designed to make it
easier and more intuitive for developers to produce games and demos
utilising 3D hardware. The class library abstracts all the details of
using the underlying system libraries like Direct3D and OpenGL and
provides an interface based on world objects and other intuitive
classes.

| Build | Status |
|-------|-----------------|
| Ubuntu, OSX, Android, iOS | [![Build Status](https://travis-ci.org/OGRECave/ogre.svg?branch=master)](https://travis-ci.org/OGRECave/ogre) |
| MSVC | [![Build status](https://ci.appveyor.com/api/projects/status/kcki7y0n1ahrggdw/branch/master?svg=true)](https://ci.appveyor.com/project/paroj/ogre-bsrh7/branch/master) |

## Index Of Contents
* [What's New?](Docs/1.12-Notes.md)  
A summary of the new and altered features in this release.
* [Building the core OGRE libraries](https://ogrecave.github.io/ogre/api/latest/building-ogre.html)  
If you're using the full source release, this will help you build it. If you're using a precompiled SDK then most of the work has already been done for you, and you should use the sample projects to see how to compile your own code against OGRE.
* [The OGRE Manual](https://ogrecave.github.io/ogre/api/latest/manual.html)  
A high-level guide to the major parts of the engine and script reference.
* [API Reference](https://ogrecave.github.io/ogre/api/latest/)  
The full OGRE API documentation, as generated from the (heavily!) commented source.
* [The OGRE Tutorials](https://ogrecave.github.io/ogre/api/latest/tutorials.html)  
A gold mine of tutorials, tips and code snippets which will help you get up to speed with the engine.

## Try it
* [Online Emscripten Demo](https://ogrecave.github.io/ogre/emscripten/)
* [Linux Snap Package](https://snapcraft.io/ogre)
* [Android App on F-Droid](https://f-droid.org/packages/org.ogre.browser/)

## Features

For an exhaustive list, see the [features page](http://www.ogre3d.org/about/features) and try our Sample Browser. For a quick overview see below

| Integrated Bump Mapping | Integrated shadows |
|----|----|
| ![](Other/screenshots/bumpmap.jpg) | ![](Other/screenshots/shadows.jpg) |


| HW & SW skeletal animation | Multi-layer Terrain |
|----|----|
| ![](Other/screenshots/skeletal.jpg) | ![](Other/screenshots/terrain.jpg) |

| Automatic Rendertarget pipelining (Compositors) | Volume Rendering with CSG & Triplanar Texturing |
|----|----|
| ![](Other/screenshots/compositor.jpg) | ![](Other/screenshots/volume.jpg) |

| [Dear ImGui](https://github.com/ocornut/imgui) | Particle Effects |
|----|----|
| ![](Other/screenshots/imgui.jpg) | ![](Other/screenshots/particle.jpg) |

## Who is using it?

**Open Source**
- [Rigs of Rods - Soft Body Physics Simulator](https://rigsofrods.org/)
- [Gazebo - Robot simulation](http://gazebosim.org/)
- [ROS 3D visualization tool](http://wiki.ros.org/rviz)
- [legged robotics RAISIM](https://github.com/leggedrobotics/raisimLib#examples)

**Closed Source**
- [Hob](http://store.steampowered.com/app/404680/Hob/)
- [Torchlight II](http://store.steampowered.com/app/200710/Torchlight_II/)
- [Battlezone 98 Redux](http://store.steampowered.com/app/301650/Battlezone_98_Redux/)

## Contributing
We welcome all contributions to OGRE, be that new
plugins, bugfixes, extensions, tutorials, documentation, example
applications, artwork or pretty much anything else! If you would like
to contribute to the development of OGRE, please create a [pull request](https://github.com/OGRECave/ogre/pulls).

## Getting Support
Please use our [community support forums](http://forums.ogre3d.org/) if you need help or
think you may have found a bug.

## Licensing
Please see the [full license documentation](Docs/License.md) for details.
