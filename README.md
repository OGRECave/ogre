[![GitHub release](https://img.shields.io/github/release/ogrecave/ogre.svg)](https://github.com/OGRECave/ogre/releases/latest)
[![CI Build](https://github.com/OGRECave/ogre/actions/workflows/ci-build.yml/badge.svg)](https://github.com/OGRECave/ogre/actions/workflows/ci-build.yml)
[![Downloads](https://static.pepy.tech/badge/ogre-python)](https://pepy.tech/project/ogre-python)
[![Join the chat at https://gitter.im/OGRECave/ogre](https://badges.gitter.im/OGRECave/ogre.svg)](https://gitter.im/OGRECave/ogre?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Patreon](https://img.shields.io/badge/patreon-donate-blue.svg)](https://www.patreon.com/ogre1)

![](Other/ogre_header.svg)

## OGRE - scene-oriented, flexible 3D engine

**OGRE (Object-Oriented Graphics Rendering Engine)** is a powerful, open-source 3D rendering engine that empowers you to create stunning games, simulations, and visualizations without getting bogged down in low-level graphics APIs.

Focus on creating your world, not on boilerplate code. OGRE's scene-oriented approach and clean C++ architecture provide an intuitive framework, abstracting the complexities of Direct3D and OpenGL so you can be more productive.

[Get Started](#get-started-now) -
[Tutorials](https://ogrecave.github.io/ogre/api/latest/tutorials.html) -
[Documentation](https://ogrecave.github.io/ogre/api/latest/manual.html) -
[Community Support](http://forums.ogre3d.org/) -
[What's New?](Docs/14-Notes.md)


## Features

OGRE provides the tools you need to build immersive experiences, from advanced lighting and shadow effects to complex character animations and rich particle systems.

| Physically Based Shading | Dynamic Shadows |
|----|----|
| ![](Other/screenshots/pbr.webp) | ![](Other/screenshots/shadows.jpg) |
| Achieve stunning surfaces with PBR workflows | Stencil and texture-based shadows for any environment |

| Character Animation| Particle Effects |
|----|----|
| ![](Other/screenshots/skeletal.jpg) | ![](Other/screenshots/particle.jpg) |
| Hardware & software skeletal animation support | Flexible particle systems for fire, smoke, sparks & more. |

| Advanced Compositor Pipeline | Terrain Rendering |
|----|----|
| ![](Other/screenshots/compositor.jpg) | ![](Other/screenshots/terrain.jpg) |
| Streamline post-processing like bloom and HDR | Multi-layered, textured landscapes with LOD |

| UI Toolkit | Physics Engine Integration |
|----|----|
| ![](Other/screenshots/imgui.jpg) | ![](Other/screenshots/bullet.webp) |
| Seamless integration with [Dear ImGui](https://github.com/ocornut/imgui) for in-game UI | Use [Bullet Physics](https://pybullet.org/) for rigid body dynamics |

| Realistic Surface Details | Volumetric Rendering |
|----|----|
| ![](Other/screenshots/bumpmap.webp) | ![](Other/screenshots/volume.jpg) |
| Bump and offset mapping for enhanced textures | With CSG and triplanar texturing |

For a complete list of capabilities, see our [features page](http://www.ogre3d.org/about/features).

## Get started now

Ready to try OGRE? You can be up and running in minutes.

* **Try it Online:** [Launch the Emscripten Demo](https://ogrecave.github.io/ogre/emscripten/) right in your browser.
* **Download for Windows:** [Get the latest SDK](https://dl.cloudsmith.io/public/ogrecave/ogre/raw/versions/master/ogre-sdk-master-msvc142-x64.zip) with pre-compiled demos.
* **Install on Linux:** Use our [Snap Package](https://snapcraft.io/ogre) for easy installation.
* **Get it on Android:** Find our sample browser on [F-Droid](https://f-droid.org/packages/org.ogre.browser/).


For detailed instructions on compiling from source, see our [**Building OGRE guide**](https://ogrecave.github.io/ogre/api/latest/building-ogre.html).

## Who is using it?

Trusted by both open-source communities and commercial studios:

**Open Source & Research**
- [Stunt Rally 2.x - 3D Racing Game with Track Editor](https://github.com/stuntrally/stuntrally/)
- [Rigs of Rods - Soft Body Physics Simulator](https://rigsofrods.org/)
- [Gazebo - Robot simulation](http://gazebosim.org/)
- [OpenCV OVIS visualization module](https://docs.opencv.org/master/d2/d17/group__ovis.html)
- [ROS 3D visualization tool](http://wiki.ros.org/rviz)
- [Surgical Image Toolkit](https://github.com/IRCAD/sight#applications)

**Commercial Games**
- [Hob](http://store.steampowered.com/app/404680/Hob/)
- [Torchlight II](http://store.steampowered.com/app/200710/Torchlight_II/)
- [Battlezone 98 Redux](http://store.steampowered.com/app/301650/Battlezone_98_Redux/)

## Join Our Community
We believe in the power of collaboration. Whether you're a seasoned developer or just starting, you are welcome in the OGRE community.

* **Ask a question** in our [Forums](http://forums.ogre3d.org/) or on [Gitter](https://gitter.im/OGRECave/ogre).
* **Contribute to the engine** by creating a [pull request](https://github.com/OGRECave/ogre/pulls). We welcome everything from bug fixes and documentation to new features.
* **Support the project** via [Patreon](https://www.patreon.com/ogre1) to help fund continued development.

## Licensing
OGRE is licensed under the **MIT License**. Please see the [full license documentation](Docs/License.md) for details.

## Citing OGRE in Research
If you use OGRE in your academic work, please cite it:

```bibtex
  @misc{rojtberg2024ogre,
    author = "{Rojtberg, Pavel and Rogers, David and Streeting, Steve and others}",
    title = "OGRE scene-oriented, flexible 3D engine",
    year = "2001 -- 2024",
    howpublished = "\url{https://www.ogre3d.org/}",
  }
```