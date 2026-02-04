[![GitHub release](https://img.shields.io/github/release/ogrecave/ogre.svg)](https://github.com/OGRECave/ogre/releases/latest)
[![CI Build](https://github.com/OGRECave/ogre/actions/workflows/ci-build.yml/badge.svg)](https://github.com/OGRECave/ogre/actions/workflows/ci-build.yml)
[![Downloads](https://static.pepy.tech/badge/ogre-python)](https://pepy.tech/project/ogre-python)
[![Join the chat at https://gitter.im/OGRECave/ogre](https://badges.gitter.im/OGRECave/ogre.svg)](https://gitter.im/OGRECave/ogre?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Patreon](https://img.shields.io/badge/patreon-donate-blue.svg)](https://www.patreon.com/ogre1)

![](Other/ogre_header.svg)

## The High-Performance Rendering Backend

**OGRE (Object-Oriented Graphics Rendering Engine)** is a proven, modular C++ renderer for custom engine development. It empowers engine programmers and industrial simulation developers to build high-performance 3D applications in C++ and Python efficiently.

Start with a battle-tested, flexible rendering backend that scales from embedded robotics to high-end visualization tools. OGRE abstracts Vulkan, Direct3D, and OpenGL so you can focus on your engine's logic.

[Get Started](#get-started-now) -
[Tutorials](https://ogrecave.github.io/ogre/api/latest/tutorials.html) -
[Documentation](https://ogrecave.github.io/ogre/api/latest/manual.html) -
[Community Support](http://forums.ogre3d.org/) -
[What's New?](Docs/14-Notes.md)

## Instant Prototyping with Python

Experience rapid prototyping with our high-level Python bindings ([HighPy](https://ogrecave.github.io/ogre/api/latest/namespace_python_1_1_high_py.html)). Get a PBR scene running in seconds:

```python
# pip install ogre-python
import Ogre.HighPy as ohi

# Create a window
ohi.window_create("Ogre", window_size=(1280, 720))

# Load a mesh (glTF 2.0, OBJ, or Ogre Mesh)
ohi.mesh_show("Ogre", "DamagedHelmet.glb", position=(0, 0, -3))
ohi.point_light("Ogre", position=(0, 10, 0))
# Main Loop
while ohi.window_draw("Ogre") != 27: # Press ESC to exit
    pass
```


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

Ready to try OGRE? Get up and running in minutes.

* **Try it Online:** [Launch the WebAssembly Demo](https://ogrecave.github.io/ogre/emscripten/) right in your browser.
* **Download for Windows:** [Get the latest SDK](https://dl.cloudsmith.io/public/ogrecave/ogre/raw/versions/master/ogre-sdk-master-msvc142-x64.zip) with pre-compiled demos.
* **Get it on Android:** Find our sample browser on [F-Droid](https://f-droid.org/packages/org.ogre.browser/).


For detailed instructions on compiling from source, see our [**Building OGRE guide**](https://ogrecave.github.io/ogre/api/latest/building-ogre.html).

## Who is using it?

Trusted by commercial studios and open-source communities for games, simulators, and tools:

**Industrial & Robotics**
- [Gazebo - Robot simulation](http://gazebosim.org/)
- [ROS 3D visualization tool (rviz)](http://wiki.ros.org/rviz)
- [Surgical Image Toolkit](https://github.com/IRCAD/sight#applications)
- [OpenCV OVIS visualization module](https://docs.opencv.org/master/d2/d17/group__ovis.html)

**Open Source Games**
- [Stunt Rally 2.x - 3D Racing Game with Track Editor](https://github.com/stuntrally/stuntrally/)
- [Rigs of Rods - Soft Body Physics Simulator](https://rigsofrods.org/)

**Commercial Games**
- [Hob](http://store.steampowered.com/app/404680/Hob/)
- [Torchlight II](http://store.steampowered.com/app/200710/Torchlight_II/)
- [Battlezone 98 Redux](http://store.steampowered.com/app/301650/Battlezone_98_Redux/)

## Join Our Community
We believe in the power of collaboration.

* **Engine Programming Q&A**: Ask technical questions in our [Forums](http://forums.ogre3d.org/) or on [Gitter](https://gitter.im/OGRECave/ogre).
* **Contribute**: Check out [good first issues](https://github.com/OGRECave/ogre/labels/good%20first%20issue) to get started. We welcome everything from bug fixes to new features.
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