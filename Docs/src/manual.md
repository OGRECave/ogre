Manual {#manual}
======

- @subpage Introduction
- @subpage The-Core-Objects
- @subpage Scripts
- @subpage Mesh-Tools
- @subpage Hardware-Buffers
- @subpage External-Texture-Sources
- @subpage Shadows
- @subpage Animation

@page Introduction Introduction

This chapter is intended to give you an overview of the main components of OGRE and why they have been put together that way.

# Object Orientation - more than just a buzzword

The name is a dead giveaway. It says Object-Oriented Graphics Rendering Engine, and that’s exactly what it is. Ok, but why? Why did I choose to make such a big deal about this?

Well, nowadays graphics engines are like any other large software system. They start small, but soon they balloon into monstrously complex beasts which just can’t be all understood at once. It’s pretty hard to manage systems of this size, and even harder to make changes to them reliably, and that’s pretty important in a field where new techniques and approaches seem to appear every other week. Designing systems around huge files full of C function calls just doesn’t cut it anymore - even if the whole thing is written by one person (not likely) they will find it hard to locate that elusive bit of code after a few months and even harder to work out how it all fits together.

Object orientation is a very popular approach to addressing the complexity problem. It’s a step up from decomposing your code into separate functions, it groups function and state data together in classes which are designed to represent real concepts. It allows you to hide complexity inside easily recognised packages with a conceptually simple interface so they are easy to recognise and have a feel of ’building blocks’ which you can plug together again later. You can also organise these blocks so that some of them look the same on the outside, but have very different ways of achieving their objectives on the inside, again reducing the complexity for the developers because they only have to learn one interface.

I’m not going to teach you OO here, that’s a subject for many other books, but suffice to say I’d seen enough benefits of OO in business systems that I was surprised most graphics code seemed to be written in C function style. I was interested to see whether I could apply my design experience in other types of software to an area which has long held a place in my heart - 3D graphics engines. Some people I spoke to were of the opinion that using full C++ wouldn’t be fast enough for a real-time graphics engine, but others (including me) were of the opinion that, with care, and object-oriented framework can be performant. We were right.

In summary, here’s the benefits an object-oriented approach brings to OGRE:

<dl compact="compact">
<dt>Abstraction</dt> <dd>

Common interfaces hide the nuances between different implementations of 3D API and operating systems

</dd> <dt>Encapsulation</dt> <dd>

There is a lot of state management and context-specific actions to be done in a graphics engine - encapsulation allows me to put the code and data nearest to where it is used which makes the code cleaner and easier to understand, and more reliable because duplication is avoided

</dd> <dt>Polymorphism</dt> <dd>

The behaviour of methods changes depending on the type of object you are using, even if you only learn one interface, e.g. a class specialised for managing indoor levels behaves completely differently from the standard scene manager, but looks identical to other classes in the system and has the same methods called on it

</dd> </dl>



<a name="Multi_002deverything"></a> <a name="Multi_002deverything-1"></a>

# Multi-everything

I wanted to do more than create a 3D engine that ran on one 3D API, on one platform, with one type of scene (indoor levels are most popular). I wanted OGRE to be able to extend to any kind of scene (but yet still implement scene-specific optimisations under the surface), any platform and any 3D API.

Therefore all the ’visible’ parts of OGRE are completely independent of platform, 3D API and scene type. There are no dependencies on Windows types, no assumptions about the type of scene you are creating, and the principles of the 3D aspects are based on core maths texts rather than one particular API implementation.

Now of course somewhere OGRE has to get down to the nitty-gritty of the specifics of the platform, API and scene, but it does this in subclasses specially designed for the environment in question, but which still expose the same interface as the abstract versions.

For example, there is a ’Win32Window’ class which handles all the details about rendering windows on a Win32 platform - however the application designer only has to manipulate it via the superclass interface ’RenderWindow’, which will be the same across all platforms.

Similarly the ’SceneManager’ class looks after the arrangement of objects in the scene and their rendering sequence. Applications only have to use this interface, but there is a ’BspSceneManager’ class which optimises the scene management for indoor levels, meaning you get both performance and an easy to learn interface. All applications have to do is hint about the kind of scene they will be creating and let OGRE choose the most appropriate implementation - this is covered in a later tutorial.

OGRE’s object-oriented nature makes all this possible. Currently OGRE runs on Windows, Linux and Mac OSX using plugins to drive the underlying rendering API (currently Direct3D or OpenGL). Applications use OGRE at the abstract level, thus ensuring that they automatically operate on all platforms and rendering subsystems that OGRE provides without any need for platform or API specific code.

@page The-Core-Objects The Core Objects

@tableofcontents

This tutorial gives you a quick summary of the core objects that you will use in OGRE and what they are used for.

<a name="A-Word-About-Namespaces"></a>

# A Word About Namespaces

OGRE uses a C++ feature called namespaces. This lets you put classes, enums, structures, anything really within a ’namespace’ scope which is an easy way to prevent name clashes, i.e. situations where you have 2 things called the same thing. Since OGRE is designed to be used inside other applications, I wanted to be sure that name clashes would not be a problem. Some people prefix their classes/types with a short code because some compilers don’t support namespaces, but I chose to use them because they are the ’right’ way to do it. Sorry if you have a non-compliant compiler, but hey, the C++ standard has been defined for years, so compiler writers really have no excuse anymore. If your compiler doesn’t support namespaces then it’s probably because it’s sh\*t - get a better one. ;)

This means every class, type etc should be prefixed with Ogre, e.g. Ogre::Camera, Ogre::Vector3 etc which means if elsewhere in your application you have used a Vector3 type you won’t get name clashes. To avoid lots of extra typing you can add a ’using namespace Ogre;’ statement to your code which means you don’t have to type the Ogre prefix unless there is ambiguity (in the situation where you have another definition with the same name).

<a name="Overview-from-10_002c000-feet"></a>

# Overview from 10,000 feet

Shown below is a diagram of some of the core objects and where they ’sit’ in the grand scheme of things. This is not all the classes by a long shot, just a few examples of the more more significant ones to give you an idea of how it slots together. ![](images/uml-overview.png)

At the very top of the diagram is the Root object. This is your ’way in’ to the OGRE system, and it’s where you tend to create the top-level objects that you need to deal with, like scene managers, rendering systems and render windows, loading plugins, all the fundamental stuff. If you don’t know where to start, Root is it for almost everything, although often it will just give you another object which will actually do the detail work, since Root itself is more of an organiser and facilitator object.

The majority of rest of OGRE’s classes fall into one of 3 roles:

<dl compact="compact">
<dt>Scene Management</dt> <dd>

This is about the contents of your scene, how it’s structured, how it’s viewed from cameras, etc. Objects in this area are responsible for giving you a natural declarative interface to the world you’re building; i.e. you don’t tell OGRE "set these render states and then render 3 polygons", you tell it "I want an object here, here and here, with these materials on them, rendered from this view", and let it get on with it.

</dd> <dt>Resource Management</dt> <dd>

All rendering needs resources, whether it’s geometry, textures, fonts, whatever. It’s important to manage the loading, re-use and unloading of these things carefully, so that’s what classes in this area do.

</dd> <dt>Rendering</dt> <dd>

Finally, there’s getting the visuals on the screen - this is about the lower-level end of the rendering pipeline, the specific rendering system API objects like buffers, render states and the like and pushing it all down the pipeline. Classes in the Scene Management subsystem use this to get their higher-level scene information onto the screen.

</dd> </dl>

You’ll notice that scattered around the edge are a number of plugins. OGRE is designed to be extended, and plugins are the usual way to go about it. Many of the classes in OGRE can be subclassed and extended, whether it’s changing the scene organisation through a custom SceneManager, adding a new render system implementation (e.g. Direct3D or OpenGL), or providing a way to load resources from another source (say from a web location or a database). Again this is just a small smattering of the kinds of things plugins can do, but as you can see they can plug in to almost any aspect of the system. This way, OGRE isn’t just a solution for one narrowly defined problem, it can extend to pretty much anything you need it to do.


# The Root object {#The-Root-Object}

The Ogre::Root object is the entry point to the OGRE system. This object MUST be the first one to be created, and the last one to be destroyed. In the example applications I chose to make an instance of Root a member of my application object which ensured that it was created as soon as my application object was, and deleted when the application object was deleted.

The root object lets you configure the system, for example through the showConfigDialog() method which is an extremely handy method which performs all render system options detection and shows a dialog for the user to customise resolution, colour depth, full screen options etc. It also sets the options the user selects so that you can initialise the system directly afterwards.

The root object is also your method for obtaining pointers to other objects in the system, such as the Ogre::SceneManager, Ogre::RenderSystem and various other resource managers. See below for details.

Finally, if you run OGRE in continuous rendering mode, i.e. you want to always refresh all the rendering targets as fast as possible (the norm for games and demos, but not for windowed utilities), the root object has a method called startRendering, which when called will enter a continuous rendering loop which will only end when all rendering windows are closed, or any Ogre::FrameListener objects indicate that they want to stop the cycle (see below for details of Ogre::FrameListener objects).

# The RenderSystem object {#The-RenderSystem-object}

The Ogre::RenderSystem object is actually an abstract class which defines the interface to the underlying 3D API. It is responsible for sending rendering operations to the API and setting all the various rendering options. This class is abstract because all the implementation is rendering API specific - there are API-specific subclasses for each rendering API (e.g. D3DRenderSystem for Direct3D). After the system has been initialised through Ogre::Root::initialise, the Ogre::RenderSystem object for the selected rendering API is available via the Ogre::Root::getRenderSystem() method.

However, a typical application should not normally need to manipulate the Ogre::RenderSystem object directly - everything you need for rendering objects and customising settings should be available on the Ogre::SceneManager, Material and other scene-oriented classes. It’s only if you want to create multiple rendering windows (completely separate windows in this case, not multiple viewports like a split-screen effect which is done via the RenderWindow class) or access other advanced features that you need access to the RenderSystem object.

For this reason I will not discuss the Ogre::RenderSystem object further in these tutorials. You can assume the Ogre::SceneManager handles the calls to the Ogre::RenderSystem at the appropriate times.


# The SceneManager object {#The-SceneManager-object}

Apart from the Ogre::Root object, this is probably the most critical part of the system from the application’s point of view. Certainly it will be the object which is most used by the application. The Ogre::SceneManager is in charge of the contents of the scene which is to be rendered by the engine. It is responsible for organising the contents using whatever technique it deems best, for creating and managing all the cameras, movable objects (entities), lights and materials (surface properties of objects), and for managing the ’world geometry’ which is the sprawling static geometry usually used to represent the immovable parts of a scene.

It is to the SceneManager that you go when you want to create a camera for the scene. It’s also where you go to retrieve or to remove a light from the scene. There is no need for your application to keep lists of objects, the SceneManager keeps a named set of all of the scene objects for you to access, should you need them. Look in the main documentation under the getCamera, getLight, getEntity etc methods.

The SceneManager also sends the scene to the RenderSystem object when it is time to render the scene. You never have to call the Ogre::SceneManager::\_renderScene method directly though - it is called automatically whenever a rendering target is asked to update.

So most of your interaction with the SceneManager is during scene setup. You’re likely to call a great number of methods (perhaps driven by some input file containing the scene data) in order to set up your scene. You can also modify the contents of the scene dynamically during the rendering cycle if you create your own FrameListener object (see later).

Because different scene types require very different algorithmic approaches to deciding which objects get sent to the RenderSystem in order to attain good rendering performance, the SceneManager class is designed to be subclassed for different scene types. The default SceneManager object will render a scene, but it does little or no scene organisation and you should not expect the results to be high performance in the case of large scenes. The intention is that specialisations will be created for each type of scene such that under the surface the subclass will optimise the scene organisation for best performance given assumptions which can be made for that scene type. An example is the BspSceneManager which optimises rendering for large indoor levels based on a Binary Space Partition (BSP) tree.

The application using OGRE does not have to know which subclasses are available. The application simply calls Ogre::Root::createSceneManager(..) passing as a parameter one of a number of scene types (e.g. Ogre::ST_GENERIC, Ogre::ST_INTERIOR etc). OGRE will automatically use the best SceneManager subclass available for that scene type, or default to the basic SceneManager if a specialist one is not available. This allows the developers of OGRE to add new scene specialisations later and thus optimise previously unoptimised scene types without the user applications having to change any code.

# The ResourceGroupManager Object {#The-ResourceGroupManager-Object}

The Ogre::ResourceGroupManager class is actually a ’hub’ for loading of reusable resources like textures and meshes. It is the place that you define groups for your resources, so they may be unloaded and reloaded when you want. Servicing it are a number of ResourceManagers which manage the individual types of resource, like Ogre::TextureManager or Ogre::MeshManager. In this context, resources are sets of data which must be loaded from somewhere to provide OGRE with the data it needs. 

ResourceManagers ensure that resources are only loaded once and shared throughout the OGRE engine. They also manage the memory requirements of the resources they look after. They can also search in a number of locations for the resources they need, including multiple search paths and compressed archives (ZIP files).

Most of the time you won’t interact with resource managers directly. Resource managers will be called by other parts of the OGRE system as required, for example when you request for a texture to be added to a Material, the Ogre::TextureManager will be called for you. If you like, you can call the appropriate resource manager directly to preload resources (if for example you want to prevent disk access later on) but most of the time it’s ok to let OGRE decide when to do it.

One thing you will want to do is to tell the resource managers where to look for resources. You do this via Root::getSingleton().addResourceLocation, which actually passes the information on to Ogre::ResourceGroupManager. 

Because there is only ever 1 instance of each resource manager in the engine, if you do want to get a reference to a resource manager use the following syntax:

```cpp
Ogre::TextureManager::getSingleton().someMethod()
Ogre::MeshManager::getSingleton().someMethod()
```
# The Mesh Object {#The-Mesh-Object}

A Ogre::Mesh object represents a discrete model, a set of geometry which is self-contained and is typically fairly small on a world scale. Ogre::Mesh objects are assumed to represent movable objects and are not used for the sprawling level geometry typically used to create backgrounds.

Ogre::Mesh objects are a type of resource, and are managed by the MeshManager resource manager. They are typically loaded from OGRE’s custom object format, the ’.mesh’ format. Mesh files are typically created by exporting from a modelling tool See [Exporters](@ref Exporters) and can be manipulated through various [Mesh Tools](@ref Mesh-Tools)

You can also create Mesh objects manually by calling the Ogre::MeshManager::createManual method. This way you can define the geometry yourself, but this is outside the scope of this manual.

Mesh objects are the basis for the individual movable objects in the world, which are called [Entities](#Entities).

Mesh objects can also be animated using See [Skeletal Animation](@ref Skeletal-Animation).

# Entities {#Entities}

An entity is an instance of a movable object in the scene. It could be a car, a person, a dog, a shuriken, whatever. The only assumption is that it does not necessarily have a fixed position in the world.

Entities are based on discrete meshes, i.e. collections of geometry which are self-contained and typically fairly small on a world scale, which are represented by the Ogre::Mesh object. Multiple entities can be based on the same mesh, since often you want to create multiple copies of the same type of object in a scene.

You create an entity by calling the Ogre::SceneManager::createEntity method, giving it a name and specifying the name of the mesh object which it will be based on (e.g. ’muscleboundhero.mesh’). The Ogre::SceneManager will ensure that the mesh is loaded by calling the Ogre::MeshManager resource manager for you. Only one copy of the Mesh will be loaded.

Ogre::Entities are not deemed to be a part of the scene until you attach them to a Ogre::SceneNode (see the section below). By attaching entities to SceneNodes, you can create complex hierarchical relationships between the positions and orientations of entities. You then modify the positions of the nodes to indirectly affect the entity positions.

When a Ogre::Mesh is loaded, it automatically comes with a number of materials defined. It is possible to have more than one material attached to a mesh - different parts of the mesh may use different materials. Any entity created from the mesh will automatically use the default materials. However, you can change this on a per-entity basis if you like so you can create a number of entities based on the same mesh but with different textures etc.

To understand how this works, you have to know that all Mesh objects are actually composed of Ogre::SubMesh objects, each of which represents a part of the mesh using one Material. If a Ogre::Mesh uses only one Ogre::Material, it will only have one Ogre::SubMesh.

When an Ogre::Entity is created based on this Mesh, it is composed of (possibly) multiple Ogre::SubEntity objects, each matching 1 for 1 with the Ogre::SubMesh objects from the original Mesh. You can access the Ogre::SubEntity objects using the Ogre::Entity::getSubEntity method. Once you have a reference to a Ogre::SubEntity, you can change the material it uses by calling it’s setMaterialName method. In this way you can make an Ogre::Entity deviate from the default materials and thus create an individual looking version of it.

# Materials {#Materials}

The Ogre::Material object controls how objects in the scene are rendered. It specifies what basic surface properties objects have such as reflectance of colours, shininess etc, how many texture layers are present, what images are on them and how they are blended together, what special effects are applied such as environment mapping, what culling mode is used, how the textures are filtered etc.

Materials can either be set up programmatically, by calling Ogre::MaterialManager::create and tweaking the settings, or by specifying it in a ’script’ which is loaded at runtime. See [Material Scripts](@ref Material-Scripts) for more info.

Basically everything about the appearance of an object apart from it’s shape is controlled by the Material class.

The SceneManager class manages the master list of materials available to the scene. The list can be added to by the application by calling Ogre::MaterialManager::create, or by loading a Mesh (which will in turn load material properties). Whenever materials are added to the SceneManager, they start off with a default set of properties; these are defined by OGRE as the following:

-   ambient reflectance = ColourValue::White (full)
-   diffuse reflectance = ColourValue::White (full)
-   specular reflectance = ColourValue::Black (none)
-   emissive = ColourValue::Black (none)
-   shininess = 0 (not shiny)
-   No texture layers (& hence no textures)
-   SourceBlendFactor = SBF\_ONE, DestBlendFactor = SBF\_ZERO (opaque)
-   Depth buffer checking on
-   Depth buffer writing on
-   Depth buffer comparison function = CMPF\_LESS\_EQUAL
-   Culling mode = CULL\_CLOCKWISE
-   Ambient lighting in scene = ColourValue(0.5, 0.5, 0.5) (mid-grey)
-   Dynamic lighting enabled
-   Gourad shading mode
-   Solid polygon mode
-   Bilinear texture filtering

You can alter these settings by calling Ogre::MaterialManager::getDefaultSettings() and making the required changes to the Material which is returned.

Entities automatically have Material’s associated with them if they use a Ogre::Mesh object, since the Ogre::Mesh object typically sets up it’s required materials on loading. You can also customise the material used by an entity as described in [Entities](#Entities). Just create a new Material, set it up how you like (you can copy an existing material into it if you like using a standard assignment statement) and point the SubEntity entries at it using Ogre::SubEntity::setMaterialName().



# Overlays {#Overlays}

Overlays allow you to render 2D and 3D elements on top of the normal scene contents to create effects like heads-up displays (HUDs), menu systems, status panels etc. The frame rate statistics panel which comes as standard with OGRE is an example of an overlay. Overlays can contain 2D or 3D elements. 2D elements are used for HUDs, and 3D elements can be used to create cockpits or any other 3D object which you wish to be rendered on top of the rest of the scene.

You can create overlays either through the Ogre::SceneManager::createOverlay method, or you can define them in an .overlay script. In reality the latter is likely to be the most practical because it is easier to tweak (without the need to recompile the code). Note that you can define as many overlays as you like: they all start off life hidden, and you display them by calling their ’show()’ method. You can also show multiple overlays at once, and their Z order is determined by the Ogre::Overlay::setZOrder() method.

<a name="Notes-on-Integration"></a>

## Notes on Integration

The OverlaySystem is now its own component, you need to manually initialize it, with the following two lines of code (mSceneMgr is a pointer to your current Ogre::SceneManager):

```cpp
Ogre::OverlaySystem* pOverlaySystem = new Ogre::OverlaySystem();
mSceneMgr->addRenderQueueListener(pOverlaySystem);
```

One Ogre::OverlaySystem per application is enough but you need to call addRenderQueueListener once per SceneManager.

<a name="Creating-2D-Elements"></a>

## Creating 2D Elements

The OverlayElement class abstracts the details of 2D elements which are added to overlays. All items which can be added to overlays are derived from this class. It is possible (and encouraged) for users of OGRE to define their own custom subclasses of OverlayElement in order to provide their own user controls. The key common features of all OverlayElements are things like size, position, basic material name etc. Subclasses extend this behaviour to include more complex properties and behaviour.

An important built-in subclass of OverlayElement is OverlayContainer. OverlayContainer is the same as a OverlayElement, except that it can contain other OverlayElements, grouping them together (allowing them to be moved together for example) and providing them with a local coordinate origin for easier lineup.

The third important class is OverlayManager. Whenever an application wishes to create a 2D element to add to an overlay (or a container), it should call OverlayManager::createOverlayElement. The type of element you wish to create is identified by a string, the reason being that it allows plugins to register new types of OverlayElement for you to create without you having to link specifically to those libraries. For example, to create a panel (a plain rectangular area which can contain other OverlayElements) you would call OverlayManager::getSingleton().createOverlayElement("Panel", "myNewPanel");

<a name="Adding-2D-Elements-to-the-Overlay"></a>

## Adding 2D Elements to the Overlay

Only OverlayContainers can be added direct to an overlay. The reason is that each level of container establishes the Zorder of the elements contained within it, so if you nest several containers, inner containers have a higher Zorder than outer ones to ensure they are displayed correctly. To add a container (such as a Panel) to the overlay, simply call Ogre::Overlay::add2D.

If you wish to add child elements to that container, call Ogre::OverlayContainer::addChild. Child elements can be Ogre::OverlayElements or Ogre::OverlayContainer instances themselves. Remember that the position of a child element is relative to the top-left corner of it’s parent.

<a name="A-word-about-2D-coordinates"></a>

## A word about 2D coordinates

OGRE allows you to place and size elements based on 2 coordinate systems: **relative** and **pixel** based.

<dl compact="compact">
<dt>Pixel Mode</dt> <dd>

This mode is useful when you want to specify an exact size for your overlay items, and you don’t mind if those items get smaller on the screen if you increase the screen resolution (in fact you might want this). In this mode the only way to put something in the middle or at the right or bottom of the screen reliably in any resolution is to use the aligning options, whilst in relative mode you can do it just by using the right relative coordinates. This mode is very simple, the top-left of the screen is (0,0) and the bottom-right of the screen depends on the resolution. As mentioned above, you can use the aligning options to make the horizontal and vertical coordinate origins the right, bottom or center of the screen if you want to place pixel items in these locations without knowing the resolution.

</dd> <dt>Relative Mode</dt> <dd>

This mode is useful when you want items in the overlay to be the same size on the screen no matter what the resolution. In relative mode, the top-left of the screen is (0,0) and the bottom-right is (1,1). So if you place an element at (0.5, 0.5), it’s top-left corner is placed exactly in the center of the screen, no matter what resolution the application is running in. The same principle applies to sizes; if you set the width of an element to 0.5, it covers half the width of the screen. Note that because the aspect ratio of the screen is typically 1.3333 : 1 (width : height), an element with dimensions (0.25, 0.25) will not be square, but it will take up exactly 1/16th of the screen in area terms. If you want square-looking areas you will have to compensate using the typical aspect ratio e.g. use (0.1875, 0.25) instead.

</dd> </dl>

<a name="Transforming-Overlays"></a>
## Transforming Overlays

Another nice feature of overlays is being able to rotate, scroll and scale them as a whole. You can use this for zooming in / out menu systems, dropping them in from off screen and other nice effects. See the Ogre::Overlay::scroll, Ogre::Overlay::rotate and Ogre::Overlay::scale methods for more information.

<a name="Scripting-overlays"></a>

## Scripting overlays

Overlays can also be defined in scripts. See [Overlay Scripts](@ref Overlay-Scripts) for details.

<a name="GUI-systems"></a>

## GUI systems

Overlays are only really designed for non-interactive screen elements, although you can use them as a crude GUI. For a far more complete GUI solution, we recommend [CEGui](<http://www.cegui.org.uk>), [MyGUI](<http://mygui.info/>) or [libRocket](<http://librocket.com/>)

@page Scripts Scripts

OGRE drives many of its features through scripts in order to make it easier to set up. The scripts are simply plain text files which can be edited in any standard text editor, and modifying them immediately takes effect on your OGRE-based applications, without any need to recompile. This makes prototyping a lot faster. Here are the items that OGRE lets you script:

- @subpage Material-Scripts
- @subpage Compositor-Scripts
- @subpage Particle-Scripts
- @subpage Overlay-Scripts
- @subpage Font-Definition-Scripts

@page Compositor-Scripts Compositor Scripts

The compositor framework is a subsection of the OGRE API that allows you to easily define full screen post-processing effects. Compositor scripts offer you the ability to define compositor effects in a script which can be reused and modified easily, rather than having to use the API to define them. You still need to use code to instantiate a compositor against one of your visible viewports, but this is a much simpler process than actually defining the compositor itself.

@tableofcontents

# Compositor Fundamentals {#Compositor-Fundamentals}

Performing post-processing effects generally involves first rendering the scene to a texture, either in addition to or instead of the main window. Once the scene is in a texture, you can then pull the scene image into a fragment program and perform operations on it by rendering it through full screen quad. The target of this post processing render can be the main result (e.g. a window), or it can be another render texture so that you can perform multi-stage convolutions on the image. You can even ’ping-pong’ the render back and forth between a couple of render textures to perform convolutions which require many iterations, without using a separate texture for each stage. Eventually you’ll want to render the result to the final output, which you do with a full screen quad. This might replace the whole window (thus the main window doesn’t need to render the scene itself), or it might be a combinational effect. 

So that we can discuss how to implement these techniques efficiently, a number of definitions are required:

<dl compact="compact">
<dt>Compositor</dt> <dd>

Definition of a fullscreen effect that can be applied to a user viewport. This is what you’re defining when writing compositor scripts as detailed in this section.

</dd> <dt>Compositor Instance</dt> <dd>

An instance of a compositor as applied to a single viewport. You create these based on compositor definitions, See [Applying a Compositor](#Applying-a-Compositor).

</dd> <dt>Compositor Chain</dt> <dd>

It is possible to enable more than one compositor instance on a viewport at the same time, with one compositor taking the results of the previous one as input. This is known as a compositor chain. Every viewport which has at least one compositor attached to it has a compositor chain. See [Applying a Compositor](#Applying-a-Compositor)

</dd> <dt>Target</dt> <dd>

This is a RenderTarget, i.e. the place where the result of a series of render operations is sent. A target may be the final output (and this is implicit, you don’t have to declare it), or it may be an intermediate render texture, which you declare in your script with the [texture line](#compositor_005ftexture). A target which is not the output target has a defined size and pixel format which you can control.

</dd> <dt>Output Target</dt> <dd>

As Target, but this is the single final result of all operations. The size and pixel format of this target cannot be controlled by the compositor since it is defined by the application using it, thus you don’t declare it in your script. However, you do declare a Target Pass for it, see below.

</dd> <dt>Target Pass</dt> <dd>

A Target may be rendered to many times in the course of a composition effect. In particular if you ’ping pong’ a convolution between a couple of textures, you will have more than one Target Pass per Target. Target passes are declared in the script using a [target or target\_output line](#Compositor-Target-Passes), the latter being the final output target pass, of which there can be only one.

</dd> <dt>Pass</dt> <dd>

Within a Target Pass, there are one or more individual [passes](#Compositor-Passes), which perform a very specific action, such as rendering the original scene (or pulling the result from the previous compositor in the chain), rendering a fullscreen quad, or clearing one or more buffers. Typically within a single target pass you will use the either a ’render scene’ pass or a ’render quad’ pass, not both. Clear can be used with either type.

</dd> </dl>

# Loading scripts {#Loading-scripts-1}

Compositor scripts are loaded when resource groups are initialised: OGRE looks in all resource locations associated with the group (see Root::addResourceLocation) for files with the ’.compositor’ extension and parses them. If you want to parse files manually, use CompositorSerializer::parseScript.

# Format {#Format-1}

Several compositors may be defined in a single script. The script format is pseudo-C++, with sections delimited by curly braces (’{’, ’}’), and comments indicated by starting a line with ’//’ (note, no nested form comments allowed). The general format is shown below in the example below:

```cpp
// This is a comment
// Black and white effect
compositor B&W
{
    technique
    {
        // Temporary textures
        texture rt0 target_width target_height PF_A8R8G8B8

        target rt0
        {
            // Render output from previous compositor (or original scene)
            input previous
        }

        target_output
        {
            // Start with clear output
            input none
            // Draw a fullscreen quad with the black and white image
            pass render_quad
            {
                // Renders a fullscreen quad with a material
                material Ogre/Compositor/BlackAndWhite
                input 0 rt0
            }
        }
    }
}
```

Every compositor in the script must be given a name, which is the line ’compositor &lt;name&gt;’ before the first opening ’{’. This name must be globally unique. It can include path characters (as in the example) to logically divide up your compositors, and also to avoid duplicate names, but the engine does not treat the name as hierarchical, just as a string. Names can include spaces but must be surrounded by double quotes i.e. compositor "My Name".

The major components of a compositor are the [techniques](#Compositor-Techniques), the [target passes](#Compositor-Target-Passes) and the [passes](#Compositor-Passes), which are covered in detail in the following sections.

## Techniques {#Compositor-Techniques}

A compositor technique is much like a [material technique](@ref Techniques) in that it describes one approach to achieving the effect you’re looking for. A compositor definition can have more than one technique if you wish to provide some fallback should the hardware not support the technique you’d prefer to use. Techniques are evaluated for hardware support based on 2 things:

<dl compact="compact">
<dt>Material support</dt> <dd>

All [passes](#Compositor-Passes) that render a fullscreen quad use a material; for the technique to be supported, all of the materials referenced must have at least one supported material technique. If they don’t, the compositor technique is marked as unsupported and won’t be used.

</dd> <dt>Texture format support</dt> <dd>

This one is slightly more complicated. When you request a [texture](#compositor_005ftexture) in your technique, you request a pixel format. Not all formats are natively supported by hardware, especially the floating point formats. However, in this case the hardware will typically downgrade the texture format requested to one that the hardware does support - with compositor effects though, you might want to use a different approach if this is the case. So, when evaluating techniques, the compositor will first look for native support for the exact pixel format you’ve asked for, and will skip onto the next technique if it is not supported, thus allowing you to define other techniques with simpler pixel formats which use a different approach. If it doesn’t find any techniques which are natively supported, it tries again, this time allowing the hardware to downgrade the texture format and thus should find at least some support for what you’ve asked for.

</dd> </dl>

As with material techniques, compositor techniques are evaluated in the order you define them in the script, so techniques declared first are preferred over those declared later.

Format: technique { }

Techniques can have the following nested elements:

-   [texture](#compositor_005ftexture)
-   [texture\_ref](#compositor_005ftexture_005fref)
-   [scheme](#compositor_005fscheme)
-   [compositor\_logic](#compositor_005flogic)
-   [target](#Compositor-Target-Passes)
-   [target\_output](#Compositor-Target-Passes)

<a name="compositor_005ftexture"></a><a name="texture-2"></a>

## texture

This declares a render texture for use in subsequent [target passes](#Compositor-Target-Passes).



Format: texture &lt;Name&gt; &lt;Width&gt; &lt;Height&gt; &lt;Pixel Format&gt; \[&lt;MRT Pixel Format2&gt;\] \[&lt;MRT Pixel FormatN&gt;\] \[pooled\] \[gamma\] \[no\_fsaa\] \[depth\_pool &lt;poolId&gt;\] \[&lt;scope&gt;\]

Here is a description of the parameters:

<dl compact="compact">
<dt>Name</dt> <dd>

A name to give the render texture, which must be unique within this compositor. This name is used to reference the texture in [target passes](#Compositor-Target-Passes), when the texture is rendered to, and in [passes](#Compositor-Passes), when the texture is used as input to a material rendering a fullscreen quad.

</dd> <dt>Width, Height</dt> <dd>

The dimensions of the render texture. You can either specify a fixed width and height, or you can request that the texture is based on the physical dimensions of the viewport to which the compositor is attached. The options for the latter are ’target\_width’, ’target\_height’, ’target\_width\_scaled &lt;factor&gt;’ and ’target\_height\_scaled &lt;factor&gt;’ - where ’factor’ is the amount by which you wish to multiply the size of the main target to derive the dimensions.

</dd> <dt>Pixel Format</dt> <dd>

The pixel format of the render texture. This affects how much memory it will take, what colour channels will be available, and what precision you will have within those channels. The available options are PF\_A8R8G8B8, PF\_R8G8B8A8, PF\_R8G8B8, PF\_FLOAT16\_RGBA, PF\_FLOAT16\_RGB, PF\_FLOAT16\_R, PF\_FLOAT32\_RGBA, PF\_FLOAT32\_RGB, and PF\_FLOAT32\_R.

</dd> <dt>pooled</dt> <dd>

If present, this directive makes this texture ’pooled’ among compositor instances, which can save some memory.

</dd> <dt>gamma</dt> <dd>

If present, this directive means that sRGB gamma correction will be enabled on writes to this texture. You should remember to include the opposite sRGB conversion when you read this texture back in another material, such as a quad. This option will automatically enabled if you use a render\_scene pass on this texture and the viewport on which the compositor is based has sRGB write support enabled.

</dd> <dt>no\_fsaa</dt> <dd>

If present, this directive disables the use of anti-aliasing on this texture. FSAA is only used if this texture is subject to a render\_scene pass and FSAA was enabled on the original viewport on which this compositor is based; this option allows you to override it and disable the FSAA if you wish.

</dd> <dt>depth\_pool</dt> <dd>

When present, this directive has to be followed by an integer. This directive is unrelated to the "pooled" directive. This one sets from which Depth buffer pool the depth buffer will be chosen from. All RTs from all compositors (including render windows if the render system API allows it) with the same pool ID share the same depth buffers (following the rules of the current render system APIs, (check RenderSystemCapabilities flags to find the rules). When the pool ID is 0, no depth buffer is used. This can be helpful for passes that don’t require a Depth buffer at all, potentially saving performance and memory. Default value is 1.

</dd> <dt>scope</dt> <dd>

If present, this directive sets the scope for the texture for being accessed by other compositors using the [texture\_ref](#compositor_005ftexture_005fref) directive. There are three options : ’local\_scope’ (which is also the default) means that only the compositor defining the texture can access it. ’chain\_scope’ means that the compositors after this compositor in the chain can reference its textures, and ’global\_scope’ means that the entire application can access the texture. This directive also affects the creation of the textures (global textures are created once and thus can’t be used with the pooled directive, and can’t rely on viewport size).

</dd> </dl>

Example: texture rt0 512 512 PF\_R8G8B8A8<br> Example: texture rt1 target\_width target\_height PF\_FLOAT32\_RGB

You can in fact repeat this element if you wish. If you do so, that means that this render texture becomes a Multiple Render Target (MRT), when the GPU writes to multiple textures at once. It is imperative that if you use MRT that the shaders that render to it render to ALL the targets. Not doing so can cause undefined results. It is also important to note that although you can use different pixel formats for each target in a MRT, each one should have the same total bit depth since most cards do not support independent bit depths. If you try to use this feature on cards that do not support the number of MRTs you’ve asked for, the technique will be skipped (so you ought to write a fallback technique).

Example : texture mrt\_output target\_width target\_height PF\_FLOAT16\_RGBA PF\_FLOAT16\_RGBA chain\_scope

<a name="compositor_005ftexture_005fref"></a><a name="texture_005fref"></a>

## texture\_ref

This declares a reference of a texture from another compositor to be used in this compositor.



Format: texture\_ref &lt;Local Name&gt; &lt;Reference Compositor&gt; &lt;Reference Texture Name&gt;

Here is a description of the parameters:

<dl compact="compact">
<dt>Local Name</dt> <dd>

A name to give the referenced texture, which must be unique within this compositor. This name is used to reference the texture in [target passes](#Compositor-Target-Passes), when the texture is rendered to, and in [passes](#Compositor-Passes), when the texture is used as input to a material rendering a fullscreen quad.

</dd> <dt>Reference Compositor</dt> <dd>

The name of the compositor that we are referencing a texture from

</dd> <dt>Reference Texture Name</dt> <dd>

The name of the texture in the compositor that we are referencing

</dd> </dl>

Make sure that the texture being referenced is scoped accordingly (either chain or global scope) and placed accordingly during chain creation (if referencing a chain-scoped texture, the compositor must be present in the chain and placed before the compositor referencing it).

Example : texture\_ref GBuffer GBufferCompositor mrt\_output

<a name="compositor_005fscheme"></a><a name="scheme-2"></a>

## scheme

This gives a compositor technique a scheme name, allowing you to manually switch between different techniques for this compositor when instantiated on a viewport by calling CompositorInstance::setScheme.



Format: material\_scheme &lt;Name&gt; 

<a name="compositor_005flogic"></a><a name="compositor_005flogic-1"></a>

## compositor\_logic

This connects between a compositor and code that it requires in order to function correctly. When an instance of this compositor will be created, the compositor logic will be notified and will have the chance to prepare the compositor’s operation (for example, adding a listener).



Format: compositor\_logic &lt;Name&gt;

Registration of compositor logics is done by name through CompositorManager::registerCompositorLogic.

## Target Passes {#Compositor-Target-Passes}

A target pass is the action of rendering to a given target, either a render texture or the final output. You can update the same render texture multiple times by adding more than one target pass to your compositor script - this is very useful for ’ping pong’ renders between a couple of render textures to perform complex convolutions that cannot be done in a single render, such as blurring.

There are two types of target pass, the sort that updates a render texture: Format: target &lt;Name&gt; { } ... and the sort that defines the final output render: Format: target\_output { }

The contents of both are identical, the only real difference is that you can only have a single target\_output entry, whilst you can have many target entries. Here are the attributes you can use in a ’target’ or ’target\_output’ section of a .compositor script:

-   [input](#compositor_005ftarget_005finput)
-   [only\_initial](#only_005finitial)
-   [visibility\_mask](#visibility_005fmask)
-   [lod\_bias](#compositor_005flod_005fbias)
-   [material\_scheme](#material_005fscheme)
-   [shadows](#compositor_005fshadows)
-   [pass](#Compositor-Passes)

<a name="Attribute-Descriptions-2"></a>

# Attribute Descriptions

<a name="compositor_005ftarget_005finput"></a><a name="input"></a>

## input

Sets input mode of the target, which tells the target pass what is pulled in before any of its own passes are rendered. Format: input (none | previous) Default: input none

<dl compact="compact">
<dt>none</dt> <dd>

The target will have nothing as input, all the contents of the target must be generated using its own passes. Note this does not mean the target will be empty, just no data will be pulled in. For it to truly be blank you’d need a ’clear’ pass within this target.

</dd> <dt>previous</dt> <dd>

The target will pull in the previous contents of the viewport. This will be either the original scene if this is the first compositor in the chain, or it will be the output from the previous compositor in the chain if the viewport has multiple compositors enabled.

</dd> </dl> <a name="only_005finitial"></a><a name="only_005finitial-1"></a>

## only\_initial

If set to on, this target pass will only execute once initially after the effect has been enabled. This could be useful to perform once-off renders, after which the static contents are used by the rest of the compositor. Format: only\_initial (on | off) Default: only\_initial off

<a name="visibility_005fmask"></a><a name="visibility_005fmask-1"></a>

## visibility\_mask

Sets the visibility mask for any render\_scene passes performed in this target pass. This is a bitmask (although it must be specified as decimal, not hex) and maps to Viewport::setVisibilityMask. Format: visibility\_mask &lt;mask&gt; Default: visibility\_mask 4294967295

<a name="compositor_005flod_005fbias"></a><a name="lod_005fbias"></a>

## lod\_bias

Set the scene LOD bias for any render\_scene passes performed in this target pass. The default is 1.0, everything below that means lower quality, higher means higher quality. Format: lod\_bias &lt;lodbias&gt; Default: lod\_bias 1.0

<a name="compositor_005fshadows"></a><a name="shadows"></a>

## shadows

Sets whether shadows should be rendered during any render\_scene pass performed in this target pass. The default is ’on’. Format: shadows (on | off) Default: shadows on

<a name="material_005fscheme"></a><a name="material_005fscheme-1"></a>

## material\_scheme

If set, indicates the material scheme to use for any render\_scene pass. Useful for performing special-case rendering effects. Format: material\_scheme &lt;scheme name&gt; Default: None

## Compositor Passes {#Compositor-Passes}

A pass is a single rendering action to be performed in a target pass.  Format: ’pass’ (render\_quad | clear | stencil | render\_scene | render\_custom) \[custom name\] { }

There are four types of pass:

<dl compact="compact">
<dt>clear</dt> <dd>

This kind of pass sets the contents of one or more buffers in the target to a fixed value. So this could clear the colour buffer to a fixed colour, set the depth buffer to a certain set of contents, fill the stencil buffer with a value, or any combination of the above.

</dd> <dt>stencil</dt> <dd>

This kind of pass configures stencil operations for the subsequent passes. It can set the stencil compare function, operations and reference values for you to perform your own stencil effects.

</dd> <dt>render\_scene</dt> <dd>

This kind of pass performs a regular rendering of the scene. It will use the [visibility\_mask](#visibility_005fmask), [lod\_bias](#compositor_005flod_005fbias), and [material\_scheme](#material_005fscheme) from the parent target pass.

</dd> <dt>render\_quad</dt> <dd>

This kind of pass renders a quad over the entire render target, using a given material. You will undoubtedly want to pull in the results of other target passes into this operation to perform fullscreen effects.

</dd> <dt>render\_custom</dt> <dd>

This kind of pass is just a callback to user code for the composition pass specified in the custom name (and registered via CompositorManager::registerCustomCompositionPass) and allows the user to create custom render operations for more advanced effects. This is the only pass type that requires the custom name parameter.

</dd> </dl>

Here are the attributes you can use in a ’pass’ section of a .compositor script:

<a name="Available-Pass-Attributes"></a>

# Available Pass Attributes

-   [material](#material)
-   [input](#compositor_005fpass_005finput)
-   [identifier](#compositor_005fpass_005fidentifier)
-   [first\_render\_queue](#first_005frender_005fqueue)
-   [last\_render\_queue](#last_005frender_005fqueue)
-   [material\_scheme](#compositor_005fpass_005fmaterial_005fscheme)
-   [clear](#compositor_005fclear)
-   [stencil](#compositor_005fstencil)

<a name="material"></a><a name="material-1"></a>

## material

For passes of type ’render\_quad’, sets the material used to render the quad. You will want to use shaders in this material to perform fullscreen effects, and use the [input](#compositor_005fpass_005finput) attribute to map other texture targets into the texture bindings needed by this material. Format: material &lt;Name&gt;

<a name="compositor_005fpass_005finput"></a><a name="input-1"></a>

## input

For passes of type ’render\_quad’, this is how you map one or more local render textures (See [compositor\_texture](#compositor_005ftexture)) into the material you’re using to render the fullscreen quad. To bind more than one texture, repeat this attribute with different sampler indexes. Format: input &lt;sampler&gt; &lt;Name&gt; \[&lt;MRTIndex&gt;\]

<dl compact="compact">
<dt>sampler</dt> <dd>

The texture sampler to set, must be a number in the range \[0, OGRE\_MAX\_TEXTURE\_LAYERS-1\].

</dd> <dt>Name</dt> <dd>

The name of the local render texture to bind, as declared in [compositor\_texture](#compositor_005ftexture) and rendered to in one or more [target pass](#Compositor-Target-Passes).

</dd> <dt>MRTIndex</dt> <dd>

If the local texture that you’re referencing is a Multiple Render Target (MRT), this identifies the surface from the MRT that you wish to reference (0 is the first surface, 1 the second etc).

</dd> </dl>

Example: input 0 rt0

<a name="compositor_005fpass_005fidentifier"></a><a name="identifier"></a>

## identifier

Associates a numeric identifier with the pass. This is useful for registering a listener with the compositor (CompositorInstance::addListener), and being able to identify which pass it is that’s being processed when you get events regarding it. Numbers between 0 and 2^32 are allowed. Format: identifier &lt;number&gt; Example: identifier 99945 Default: identifier 0

<a name="first_005frender_005fqueue"></a><a name="first_005frender_005fqueue-1"></a>

## first\_render\_queue

For passes of type ’render\_scene’, this sets the first render queue id that is included in the render. Defaults to the value of RENDER\_QUEUE\_SKIES\_EARLY. Format: first\_render\_queue &lt;id&gt; Default: first\_render\_queue 0

<a name="last_005frender_005fqueue"></a><a name="last_005frender_005fqueue-1"></a>

## last\_render\_queue

For passes of type ’render\_scene’, this sets the last render queue id that is included in the render. Defaults to the value of RENDER\_QUEUE\_SKIES\_LATE. Format: last\_render\_queue &lt;id&gt; Default: last\_render\_queue 95

<a name="compositor_005fpass_005fmaterial_005fscheme"></a><a name="material_005fscheme-2"></a>

## material\_scheme

If set, indicates the material scheme to use for this pass only. Useful for performing special-case rendering effects. This will overwrite the scheme if set at the target scope as well. Format: material\_scheme &lt;scheme name&gt; Default: None

# Clear Section {#Clear-Section}

For passes of type ’clear’, this section defines the buffer clearing parameters.  Format: clear { }

Here are the attributes you can use in a ’clear’ section of a .compositor script:

-   [buffers](#compositor_005fclear_005fbuffers)
-   [colour\_value](#compositor_005fclear_005fcolour_005fvalue)
-   [depth\_value](#compositor_005fclear_005fdepth_005fvalue)
-   [stencil\_value](#compositor_005fclear_005fstencil_005fvalue) <a name="compositor_005fclear_005fbuffers"></a><a name="buffers"></a>

    ## buffers

    Sets the buffers cleared by this pass.

    Format: buffers \[colour\] \[depth\] \[stencil\] Default: buffers colour depth

    <a name="compositor_005fclear_005fcolour_005fvalue"></a><a name="colour_005fvalue"></a>

    ## colour\_value

    Set the colour used to fill the colour buffer by this pass, if the colour buffer is being cleared ([buffers](#compositor_005fclear_005fbuffers)).  Format: colour\_value &lt;red&gt; &lt;green&gt; &lt;blue&gt; &lt;alpha&gt; Default: colour\_value 0 0 0 0

    <a name="compositor_005fclear_005fdepth_005fvalue"></a><a name="depth_005fvalue"></a>

    ## depth\_value

    Set the depth value used to fill the depth buffer by this pass, if the depth buffer is being cleared ([buffers](#compositor_005fclear_005fbuffers)).  Format: depth\_value &lt;depth&gt; Default: depth\_value 1.0

    <a name="compositor_005fclear_005fstencil_005fvalue"></a><a name="stencil_005fvalue"></a>

    ## stencil\_value

    Set the stencil value used to fill the stencil buffer by this pass, if the stencil buffer is being cleared ([buffers](#compositor_005fclear_005fbuffers)).  Format: stencil\_value &lt;value&gt; Default: stencil\_value 0.0

# Stencil Section {#Stencil-Section}

For passes of type ’stencil’, this section defines the stencil operation parameters. 

Format: stencil { }

Here are the attributes you can use in a ’stencil’ section of a .compositor script:

-   [check](#compositor_005fstencil_005fcheck)
-   [comp\_func](#compositor_005fstencil_005fcomp_005ffunc)
-   [ref\_value](#compositor_005fstencil_005fref_005fvalue)
-   [mask](#compositor_005fstencil_005fmask)
-   [fail\_op](#compositor_005fstencil_005ffail_005fop)
-   [depth\_fail\_op](#compositor_005fstencil_005fdepth_005ffail_005fop)
-   [pass\_op](#compositor_005fstencil_005fpass_005fop)
-   [two\_sided](#compositor_005fstencil_005ftwo_005fsided) <a name="compositor_005fstencil_005fcheck"></a><a name="check"></a>

    ## check

    Enables or disables the stencil check, thus enabling the use of the rest of the features in this section. The rest of the options in this section do nothing if the stencil check is off. Format: check (on | off)

    <a name="compositor_005fstencil_005fcomp_005ffunc"></a><a name="comp_005ffunc"></a>

    ## comp\_func

    Sets the function used to perform the following comparison: (ref\_value & mask) comp\_func (Stencil Buffer Value & mask)

    What happens as a result of this comparison will be one of 3 actions on the stencil buffer, depending on whether the test fails, succeeds but with the depth buffer check still failing, or succeeds with the depth buffer check passing too. You set the actions in the [fail\_op](#compositor_005fstencil_005ffail_005fop), [depth\_fail\_op](#compositor_005fstencil_005fdepth_005ffail_005fop) and [pass\_op](#compositor_005fstencil_005fpass_005fop) respectively. If the stencil check fails, no colour or depth are written to the frame buffer. Format: comp\_func (always\_fail | always\_pass | less | less\_equal | not\_equal | greater\_equal | greater) Default: comp\_func always\_pass

    <a name="compositor_005fstencil_005fref_005fvalue"></a><a name="ref_005fvalue"></a>

    ## ref\_value

    Sets the reference value used to compare with the stencil buffer as described in [comp\_func](#compositor_005fstencil_005fcomp_005ffunc). Format: ref\_value &lt;value&gt; Default: ref\_value 0.0

    <a name="compositor_005fstencil_005fmask"></a><a name="mask"></a>

    ## mask

    Sets the mask used to compare with the stencil buffer as described in [comp\_func](#compositor_005fstencil_005fcomp_005ffunc). Format: mask &lt;value&gt; Default: mask 4294967295

    <a name="compositor_005fstencil_005ffail_005fop"></a><a name="fail_005fop"></a>

    ## fail\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) and depth comparison is that both fail. Format: fail\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert) Default: depth\_fail\_op keep These actions mean:

    <dl compact="compact">
    <dt>keep</dt> <dd>

    Leave the stencil buffer unchanged.

    </dd> <dt>zero</dt> <dd>

    Set the stencil value to zero.

    </dd> <dt>replace</dt> <dd>

    Set the stencil value to the reference value.

    </dd> <dt>increment</dt> <dd>

    Add one to the stencil value, clamping at the maximum value.

    </dd> <dt>decrement</dt> <dd>

    Subtract one from the stencil value, clamping at 0.

    </dd> <dt>increment\_wrap</dt> <dd>

    Add one to the stencil value, wrapping back to 0 at the maximum.

    </dd> <dt>decrement\_wrap</dt> <dd>

    Subtract one from the stencil value, wrapping to the maximum below 0.

    </dd> <dt>invert</dt> <dd>

    invert the stencil value.

    </dd> </dl> <a name="compositor_005fstencil_005fdepth_005ffail_005fop"></a><a name="depth_005ffail_005fop"></a>

    ## depth\_fail\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) passes but the depth comparison fails. 

    Format: depth\_fail\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert) Default: depth\_fail\_op keep

    <a name="compositor_005fstencil_005fpass_005fop"></a><a name="pass_005fop"></a>

    ## pass\_op

    Sets what to do with the stencil buffer value if the result of the stencil comparison ([comp\_func](#compositor_005fstencil_005fcomp_005ffunc)) and the depth comparison pass.  Format: pass\_op (keep | zero | replace | increment | decrement | increment\_wrap | decrement\_wrap | invert) Default: pass\_op keep

    <a name="compositor_005fstencil_005ftwo_005fsided"></a><a name="two_005fsided"></a>

    ## two\_sided

    Enables or disables two-sided stencil operations, which means the inverse of the operations applies to back-facing polygons. Format: two\_sided (on | off) Default: two\_sided off


# Applying a Compositor {#Applying-a-Compositor}

Adding a compositor instance to a viewport is very simple. All you need to do is:

```cpp
CompositorManager::getSingleton().addCompositor(viewport, compositorName);
```



Where viewport is a pointer to your viewport, and compositorName is the name of the compositor to create an instance of. By doing this, a new instance of a compositor will be added to a new compositor chain on that viewport. You can call the method multiple times to add further compositors to the chain on this viewport. By default, each compositor which is added is disabled, but you can change this state by calling:

```cpp
CompositorManager::getSingleton().setCompositorEnabled(viewport, compositorName, enabledOrDisabled);
```

For more information on defining and using compositors, see Demo\_Compositor in the Samples area, together with the Examples.compositor script in the media area.

@page Overlay-Scripts Overlay Scripts

Overlay scripts offer you the ability to define overlays in a script which can be reused easily. Whilst you could set up all overlays for a scene in code using the methods of the SceneManager, Overlay and OverlayElement classes, in practice it’s a bit unwieldy. Instead you can store overlay definitions in text files which can then be loaded whenever required.

@tableofcontents

# Loading scripts {#Loading-scripts-3}

Overlay scripts are loaded at initialisation time by the system: by default it looks in all common resource locations (see Root::addResourceLocation) for files with the ’.overlay’ extension and parses them. If you want to parse files with a different extension, use the OverlayManager::getSingleton().parseAllSources method with your own extension, or if you want to parse an individual file, use OverlayManager::getSingleton().parseScript.

# Format {#Format-3}

Several overlays may be defined in a single script. The script format is pseudo-C++, with sections delimited by curly braces ({}), comments indicated by starting a line with ’//’ (note, no nested form comments allowed), and inheritance through the use of templates. The general format is shown below in a typical example:

```cpp
// The name of the overlay comes first
MyOverlays/ANewOverlay
{
    zorder 200

    container Panel(MyOverlayElements/TestPanel)
    {
        // Center it horizontally, put it at the top
        left 0.25
        top 0
        width 0.5
        height 0.1
        material MyMaterials/APanelMaterial

        // Another panel nested in this one
        container Panel(MyOverlayElements/AnotherPanel)
        {
             left 0
             top 0
             width 0.1
             height 0.1
             material MyMaterials/NestedPanel
        }
    }
}
```

The above example defines a single overlay called ’MyOverlays/ANewOverlay’, with 2 panels in it, one nested under the other. It uses relative metrics (the default if no metrics\_mode option is found).

Every overlay in the script must be given a name, which is the line before the first opening ’{’. This name must be globally unique. It can include path characters (as in the example) to logically divide up your overlays, and also to avoid duplicate names, but the engine does not treat the name a hierarchical, just as a string. Within the braces are the properties of the overlay, and any nested elements. The overlay itself only has a single property ’zorder’ which determines how ’high’ it is in the stack of overlays if more than one is displayed at the same time. Overlays with higher zorder values are displayed on top.

# Adding elements to the overlay {#Adding-elements-to-the-overlay}

Within an overlay, you can include any number of 2D or 3D elements. You do this by defining a nested block headed by:

<dl compact="compact">
<dt>’element’</dt> <dd>

if you want to define a 2D element which cannot have children of it’s own

</dd> <dt>’container’</dt> <dd>

if you want to define a 2D container object (which may itself have nested containers or elements)

</dd> </dl> <br>

The element and container blocks are pretty identical apart from their ability to store nested blocks.

## ’container’ / ’element’ blocks

These are delimited by curly braces. The format for the header preceding the first brace is:

\[container | element\] &lt;type\_name&gt; ( &lt;instance\_name&gt;) \[: &lt;template\_name&gt;\]<br> { ...

<dl compact="compact">
<dt>type\_name</dt> <dd>

Must resolve to the name of a OverlayElement type which has been registered with the OverlayManager. Plugins register with the OverlayManager to advertise their ability to create elements, and at this time advertise the name of the type. OGRE comes preconfigured with types ’Panel’, ’BorderPanel’ and ’TextArea’.

</dd> <dt>instance\_name</dt> <dd>

Must be a name unique among all other elements / containers by which to identify the element. Note that you can obtain a pointer to any named element by calling OverlayManager::getSingleton().getOverlayElement(name).

</dd> <dt>template\_name</dt> <dd>

Optional template on which to base this item. See templates.

</dd> </dl>

The properties which can be included within the braces depend on the custom type. However the following are always valid:

-   [metrics\_mode](#metrics_005fmode)
-   [horz\_align](#horz_005falign)
-   [vert\_align](#vert_005falign)
-   [left](#left)
-   [top](#overlaytopelement)
-   [width](#width)
-   [height](#height)
-   [material](#overlay_005fmaterial)
-   [caption](#caption)

# Templates {#Templates}

You can use templates to create numerous elements with the same properties. A template is an abstract element and it is not added to an overlay. It acts as a base class that elements can inherit and get its default properties. To create a template, the keyword ’template’ must be the first word in the element definition (before container or element). The template element is created in the topmost scope - it is NOT specified in an Overlay. It is recommended that you define templates in a separate overlay though this is not essential. Having templates defined in a separate file will allow different look & feels to be easily substituted.

Elements can inherit a template in a similar way to C++ inheritance - by using the : operator on the element definition. The : operator is placed after the closing bracket of the name (separated by a space). The name of the template to inherit is then placed after the : operator (also separated by a space).

A template can contain template children which are created when the template is subclassed and instantiated. Using the template keyword for the children of a template is optional but recommended for clarity, as the children of a template are always going to be templates themselves.

```cpp
template container BorderPanel(MyTemplates/BasicBorderPanel)
{
    left 0
    top 0
    width 1
    height 1

// setup the texture UVs for a borderpanel

// do this in a template so it doesn't need to be redone everywhere
    material Core/StatsBlockCenter
    border_size 0.05 0.05 0.06665 0.06665
    border_material Core/StatsBlockBorder
    border_topleft_uv 0.0000 1.0000 0.1914 0.7969
    border_top_uv 0.1914 1.0000 0.8086 0.7969
    border_topright_uv 0.8086 1.0000 1.0000 0.7969
    border_left_uv 0.0000 0.7969 0.1914 0.2148
    border_right_uv 0.8086 0.7969 1.0000 0.2148
    border_bottomleft_uv 0.0000 0.2148 0.1914 0.0000
    border_bottom_uv 0.1914 0.2148 0.8086 0.0000
    border_bottomright_uv 0.8086 0.2148 1.0000 0.0000
}
template container Button(MyTemplates/BasicButton) : MyTemplates/BasicBorderPanel
{
    left 0.82
    top 0.45
    width 0.16
    height 0.13
    material Core/StatsBlockCenter
    border_up_material Core/StatsBlockBorder/Up
    border_down_material Core/StatsBlockBorder/Down
}
template element TextArea(MyTemplates/BasicText)
{
    font_name Ogre
    char_height 0.08
    colour_top 1 1 0
    colour_bottom 1 0.2 0.2
    left 0.03
    top 0.02
    width 0.12
    height 0.09
}

MyOverlays/AnotherOverlay
{
    zorder 490
    container BorderPanel(MyElements/BackPanel) : MyTemplates/BasicBorderPanel
    {
        left 0
        top 0
        width 1
        height 1

        container Button(MyElements/HostButton) : MyTemplates/BasicButton
        {
            left 0.82
            top 0.45
            caption MyTemplates/BasicText HOST
        }

        container Button(MyElements/JoinButton) : MyTemplates/BasicButton
        {
            left 0.82
            top 0.60
            caption MyTemplates/BasicText JOIN
        }
    }
}
```

The above example uses templates to define a button. Note that the button template inherits from the borderPanel template. This reduces the number of attributes needed to instantiate a button.

Also note that the instantiate of a Button needs a template name for the caption attribute. So templates can also be used by elements that need dynamic creation of children elements (the button creates a TextAreaElement in this case for its caption).

See [OverlayElement Attributes](#OverlayElement-Attributes), [Standard OverlayElements](#Standard-OverlayElements)



<a name="OverlayElement-Attributes"></a> <a name="OverlayElement-Attributes-1"></a>

## OverlayElement Attributes

These attributes are valid within the braces of a ’container’ or ’element’ block in an overlay script. They must each be on their own line. Ordering is unimportant.

<a name="metrics_005fmode"></a><a name="metrics_005fmode-1"></a>

## metrics\_mode

Sets the units which will be used to size and position this element.

Format: metrics\_mode &lt;pixels|relative&gt;<br> Example: metrics\_mode pixels<br>

This can be used to change the way that all measurement attributes in the rest of this element are interpreted. In relative mode, they are interpreted as being a parametric value from 0 to 1, as a proportion of the width / height of the screen. In pixels mode, they are simply pixel offsets.

Default: metrics\_mode relative<br>

<a name="horz_005falign"></a><a name="horz_005falign-1"></a>

## horz\_align

Sets the horizontal alignment of this element, in terms of where the horizontal origin is.

Format: horz\_align &lt;left|center|right&gt;<br> Example: horz\_align center

This can be used to change where the origin is deemed to be for the purposes of any horizontal positioning attributes of this element. By default the origin is deemed to be the left edge of the screen, but if you change this you can center or right-align your elements. Note that setting the alignment to center or right does not automatically force your elements to appear in the center or the right edge, you just have to treat that point as the origin and adjust your coordinates appropriately. This is more flexible because you can choose to position your element anywhere relative to that origin. For example, if your element was 10 pixels wide, you would use a ’left’ property of -10 to align it exactly to the right edge, or -20 to leave a gap but still make it stick to the right edge.

Note that you can use this property in both relative and pixel modes, but it is most useful in pixel mode.

Default: horz\_align left<br>

<a name="vert_005falign"></a><a name="vert_005falign-1"></a>

## vert\_align

Sets the vertical alignment of this element, in terms of where the vertical origin is.

Format: vert\_align &lt;top|center|bottom&gt;<br> Example: vert\_align center

This can be used to change where the origin is deemed to be for the purposes of any vertical positioning attributes of this element. By default the origin is deemed to be the top edge of the screen, but if you change this you can center or bottom-align your elements. Note that setting the alignment to center or bottom does not automatically force your elements to appear in the center or the bottom edge, you just have to treat that point as the origin and adjust your coordinates appropriately. This is more flexible because you can choose to position your element anywhere relative to that origin. For example, if your element was 50 pixels high, you would use a ’top’ property of -50 to align it exactly to the bottom edge, or -70 to leave a gap but still make it stick to the bottom edge.

Note that you can use this property in both relative and pixel modes, but it is most useful in pixel mode.

Default: vert\_align top<br>

<a name="left"></a><a name="left-1"></a>

## left

Sets the horizontal position of the element relative to it’s parent.

Format: left &lt;value&gt;<br> Example: left 0.5

Positions are relative to the parent (the top-left of the screen if the parent is an overlay, the top-left of the parent otherwise) and are expressed in terms of a proportion of screen size. Therefore 0.5 is half-way across the screen.

Default: left 0<br>

<a name="overlaytopelement"></a><a name="top"></a>

## top

Sets the vertical position of the element relative to it’s parent.

Format: top &lt;value&gt;<br> Example: top 0.5

Positions are relative to the parent (the top-left of the screen if the parent is an overlay, the top-left of the parent otherwise) and are expressed in terms of a proportion of screen size. Therefore 0.5 is half-way down the screen.

Default: top 0<br>

<a name="width"></a><a name="width-1"></a>

## width

Sets the width of the element as a proportion of the size of the screen.

Format: width &lt;value&gt;<br> Example: width 0.25

Sizes are relative to the size of the screen, so 0.25 is a quarter of the screen. Sizes are not relative to the parent; this is common in windowing systems where the top and left are relative but the size is absolute.

Default: width 1<br>

<a name="height"></a><a name="height-1"></a>

## height

Sets the height of the element as a proportion of the size of the screen.

Format: height &lt;value&gt;<br> Example: height 0.25

Sizes are relative to the size of the screen, so 0.25 is a quarter of the screen. Sizes are not relative to the parent; this is common in windowing systems where the top and left are relative but the size is absolute.

Default: height 1<br>

<a name="overlay_005fmaterial"></a><a name="material-3"></a>

## material

Sets the name of the material to use for this element.

Format: material &lt;name&gt;<br> Example: material Examples/TestMaterial

This sets the base material which this element will use. Each type of element may interpret this differently; for example the OGRE element ’Panel’ treats this as the background of the panel, whilst ’BorderPanel’ interprets this as the material for the center area only. Materials should be defined in .material scripts. Note that using a material in an overlay element automatically disables lighting and depth checking on this material. Therefore you should not use the same material as is used for real 3D objects for an overlay.

Default: none<br>

<a name="caption"></a><a name="caption-1"></a>

## caption

Sets a text caption for the element.

Format: caption &lt;string&gt;<br> Example: caption This is a caption

Not all elements support captions, so each element is free to disregard this if it wants. However, a general text caption is so common to many elements that it is included in the generic interface to make it simpler to use. This is a common feature in GUI systems.

Default: blank<br>

<a name="rotation"></a><a name="rotation-1"></a>

## rotation

Sets the rotation of the element.

Format: rotation &lt;angle\_in\_degrees&gt; &lt;axis\_x&gt; &lt;axis\_y&gt; &lt;axis\_z&gt; Example: rotation 30 0 0 1

Default: none

# Standard OverlayElements {#Standard-OverlayElements}

Although OGRE’s OverlayElement and OverlayContainer classes are designed to be extended by applications developers, there are a few elements which come as standard with Ogre. These include:

-   [Panel](#Panel)
-   [BorderPanel](#BorderPanel)
-   [TextArea](#TextArea)

This section describes how you define their custom attributes in an .overlay script, but you can also change these custom properties in code if you wish. You do this by calling setParameter(param, value). You may wish to use the StringConverter class to convert your types to and from strings.

## Panel (container) {#Panel}

This is the most bog-standard container you can use. It is a rectangular area which can contain other elements (or containers) and may or may not have a background, which can be tiled however you like. The background material is determined by the material attribute, but is only displayed if transparency is off.

Attributes:

<dl compact="compact">
<dt>transparent &lt;true | false&gt;</dt> <dd>

If set to ’true’ the panel is transparent and is not rendered itself, it is just used as a grouping level for it’s children.

</dd> <dt>tiling &lt;layer&gt; &lt;x\_tile&gt; &lt;y\_tile&gt;</dt> <dd>

Sets the number of times the texture(s) of the material are tiled across the panel in the x and y direction. &lt;layer&gt; is the texture layer, from 0 to the number of texture layers in the material minus one. By setting tiling per layer you can create some nice multitextured backdrops for your panels, this works especially well when you animate one of the layers.

</dd> <dt>uv\_coords &lt;topleft\_u&gt; &lt;topleft\_v&gt; &lt;bottomright\_u&gt; &lt;bottomright\_v&gt;</dt> <dd>

Sets the texture coordinates to use for this panel.

</dd> </dl>

## BorderPanel (container) {#BorderPanel}

This is a slightly more advanced version of Panel, where instead of just a single flat panel, the panel has a separate border which resizes with the panel. It does this by taking an approach very similar to the use of HTML tables for bordered content: the panel is rendered as 9 square areas, with the center area being rendered with the main material (as with Panel) and the outer 8 areas (the 4 corners and the 4 edges) rendered with a separate border material. The advantage of rendering the corners separately from the edges is that the edge textures can be designed so that they can be stretched without distorting them, meaning the single texture can serve any size panel.

Attributes:

<dl compact="compact">
<dt>border\_size &lt;left&gt; &lt;right&gt; &lt;top&gt; &lt;bottom&gt;</dt> <dd>

The size of the border at each edge, as a proportion of the size of the screen. This lets you have different size borders at each edge if you like, or you can use the same value 4 times to create a constant size border.

</dd> <dt>border\_material &lt;name&gt;</dt> <dd>

The name of the material to use for the border. This is normally a different material to the one used for the center area, because the center area is often tiled which means you can’t put border areas in there. You must put all the images you need for all the corners and the sides into a single texture.

</dd> <dt>border\_topleft\_uv &lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</dt> <dd>

\[also border\_topright\_uv, border\_bottomleft\_uv, border\_bottomright\_uv\]; The texture coordinates to be used for the corner areas of the border. 4 coordinates are required, 2 for the top-left corner of the square, 2 for the bottom-right of the square.

</dd> <dt>border\_left\_uv &lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</dt> <dd>

\[also border\_right\_uv, border\_top\_uv, border\_bottom\_uv\]; The texture coordinates to be used for the edge areas of the border. 4 coordinates are required, 2 for the top-left corner, 2 for the bottom-right. Note that you should design the texture so that the left & right edges can be stretched / squashed vertically and the top and bottom edges can be stretched / squashed horizontally without detrimental effects.

</dd> </dl>

## TextArea (element) {#TextArea}

This is a generic element that you can use to render text. It uses fonts which can be defined in code using the FontManager and Font classes, or which have been predefined in .fontdef files. See the font definitions section for more information.

Attributes:

<dl compact="compact">
<dt>font\_name &lt;name&gt;</dt> <dd>

The name of the font to use. This font must be defined in a .fontdef file to ensure it is available at scripting time.

</dd> <dt>char\_height &lt;height&gt;</dt> <dd>

The height of the letters as a proportion of the screen height. Character widths may vary because OGRE supports proportional fonts, but will be based on this constant height.

</dd> <dt>colour &lt;red&gt; &lt;green&gt; &lt;blue&gt;</dt> <dd>

A solid colour to render the text in. Often fonts are defined in monochrome, so this allows you to colour them in nicely and use the same texture for multiple different coloured text areas. The colour elements should all be expressed as values between 0 and 1. If you use predrawn fonts which are already full colour then you don’t need this.

</dd> <dt>colour\_bottom &lt;red&gt; &lt;green&gt; &lt;blue&gt; / colour\_top &lt;red&gt; &lt;green&gt; &lt;blue&gt;</dt> <dd>

As an alternative to a solid colour, you can colour the text differently at the top and bottom to create a gradient colour effect which can be very effective.

</dd> <dt>alignment &lt;left | center | right&gt;</dt> <dd>

Sets the horizontal alignment of the text. This is different from the horz\_align parameter.

</dd> <dt>space\_width &lt;width&gt;</dt> <dd>

Sets the width of a space in relation to the screen.

</dd> </dl>

@page Font-Definition-Scripts Font Definition Scripts

Ogre uses texture-based fonts to render the TextAreaOverlayElement. You can also use the Font object for your own purpose if you wish. The final form of a font is a Material object generated by the font, and a set of ’glyph’ (character) texture coordinate information.

There are 2 ways you can get a font into OGRE:

1.  Design a font texture yourself using an art package or font generator tool
2.  Ask OGRE to generate a font texture based on a truetype font

The former gives you the most flexibility and the best performance (in terms of startup times), but the latter is convenient if you want to quickly use a font without having to generate the texture yourself. I suggest prototyping using the latter and change to the former for your final solution.

All font definitions are held in .fontdef files, which are parsed by the system at startup time. Each .fontdef file can contain multiple font definitions. The basic format of an entry in the .fontdef file is:

```cpp
<font_name>
{
    type <image | truetype>
    source <image file | truetype font file>
    ...
    ... custom attributes depending on type
}
```

<a name="Using-an-existing-font-texture"></a>

# Using an existing font texture

If you have one or more artists working with you, no doubt they can produce you a very nice font texture. OGRE supports full colour font textures, or alternatively you can keep them monochrome / greyscale and use TextArea’s colouring feature. Font textures should always have an alpha channel, preferably an 8-bit alpha channel such as that supported by TGA and PNG files, because it can result in much nicer edges. To use an existing texture, here are the settings you need:

<dl compact="compact">
<dt>type image</dt> <dd>

This just tells OGRE you want a pre-drawn font.

</dd> <dt>source &lt;filename&gt;</dt> <dd>

This is the name of the image file you want to load. This will be loaded from the standard TextureManager resource locations and can be of any type OGRE supports, although JPEG is not recommended because of the lack of alpha and the lossy compression. I recommend PNG format which has both good lossless compression and an 8-bit alpha channel.

</dd> <dt>glyph &lt;character&gt; &lt;u1&gt; &lt;v1&gt; &lt;u2&gt; &lt;v2&gt;</dt> <dd>

This provides the texture coordinates for the specified character. You must repeat this for every character you have in the texture. The first 2 numbers are the x and y of the top-left corner, the second two are the x and y of the bottom-right corner. Note that you really should use a common height for all characters, but widths can vary because of proportional fonts.

’character’ is either an ASCII character for non-extended 7-bit ASCII, or for extended glyphs, a unicode decimal value, which is identified by preceding the number with a ’u’ - e.g. ’u0546’ denotes unicode value 546.

</dd> </dl>

A note for Windows users: I recommend using BitmapFontBuilder (<http://www.lmnopc.com/bitmapfontbuilder/>), a free tool which will generate a texture and export character widths for you, you can find a tool for converting the binary output from this into ’glyph’ lines in the Tools folder.<br>

<a name="Generating-a-font-texture"></a>

# Generating a font texture

You can also generate font textures on the fly using truetype fonts. I don’t recommend heavy use of this in production work because rendering the texture can take a several seconds per font which adds to the loading times. However it is a very nice way of quickly getting text output in a font of your choice.

Here are the attributes you need to supply:

<dl compact="compact">
<dt>type truetype</dt> <dd>

Tells OGRE to generate the texture from a font

</dd> <dt>source &lt;ttf file&gt;</dt> <dd>

The name of the ttf file to load. This will be searched for in the common resource locations and in any resource locations added to FontManager.

</dd> <dt>size &lt;size\_in\_points&gt;</dt> <dd>

The size at which to generate the font, in standard points. Note this only affects how big the characters are in the font texture, not how big they are on the screen. You should tailor this depending on how large you expect to render the fonts because generating a large texture will result in blurry characters when they are scaled very small (because of the mipmapping), and conversely generating a small font will result in blocky characters if large text is rendered.

</dd> <dt>resolution &lt;dpi&gt;</dt> <dd>

The resolution in dots per inch, this is used in conjunction with the point size to determine the final size. 72 / 96 dpi is normal.

</dd> <dt>antialias\_colour &lt;true|false&gt;</dt> <dd>

This is an optional flag, which defaults to ’false’. The generator will antialias the font by default using the alpha component of the texture, which will look fine if you use alpha blending to render your text (this is the default assumed by TextAreaOverlayElement for example). If, however you wish to use a colour based blend like add or modulate in your own code, you should set this to ’true’ so the colour values are anti-aliased too. If you set this to true and use alpha blending, you’ll find the edges of your font are antialiased too quickly resulting in a ’thin’ look to your fonts, because not only is the alpha blending the edges, the colour is fading too. Leave this option at the default if in doubt.

</dd> <dt>code\_points nn-nn \[nn-nn\] ..</dt> <dd>

This directive allows you to specify which unicode code points should be generated as glyphs into the font texture. If you don’t specify this, code points 33-166 will be generated by default which covers the basic Latin 1 glyphs. If you use this flag, you should specify a space-separated list of inclusive code point ranges of the form ’start-end’. Numbers must be decimal.

</dd> <dt>character\_spacer &lt;spacing\_in\_points&gt;</dt> <dd>

This option can be useful for fonts that are atypically wide, e.g. calligraphy fonts, where you may see artifacts from characters overlapping. The default value is 5.

</dd> </dl> 

You can also create new fonts at runtime by using the FontManager if you wish.

@page Mesh-Tools Mesh Tools

There are a number of mesh tools available with OGRE to help you manipulate your meshes.

<dl compact="compact">
<dt>[Exporters](@ref Exporters)</dt> <dd>

For getting data out of modellers and into OGRE.

</dd> <dt>[XMLConverter](#XMLConverter)</dt> <dd>

For converting meshes and skeletons to/from XML.

</dd> <dt>[MeshUpgrader](#MeshUpgrader)</dt> <dd>

For upgrading binary meshes from one version of OGRE to another.

</dd> </dl>



# Exporters {#Exporters}

Exporters are plugins to 3D modelling tools which write meshes and skeletal animation to file formats which OGRE can use for realtime rendering. The files the exporters write end in .mesh and .skeleton respectively.

Each exporter has to be written specifically for the modeller in question, although they all use a common set of facilities provided by the classes MeshSerializer and SkeletonSerializer. They also normally require you to own the modelling tool.

All the exporters here can be built from the source code, or you can download precompiled versions from the OGRE web site.

# A Note About Modelling / Animation For OGRE

There are a few rules when creating an animated model for OGRE:

-   You must have no more than 4 weighted bone assignments per vertex. If you have more, OGRE will eliminate the lowest weighted assignments and re-normalise the other weights. This limit is imposed by hardware blending limitations.
-   All vertices must be assigned to at least one bone - assign static vertices to the root bone.
-   At the very least each bone must have a keyframe at the beginning and end of the animation.

If you’re creating non-animated meshes, then you do not need to be concerned with the above.

Full documentation for each exporter is provided along with the exporter itself, and there is a list of the currently supported modelling tools in the OGRE Wiki at <http://www.ogre3d.org/tikiwiki/tiki-index.php?page=OGRE+Exporters&structure=Tools>.



# XMLConverter {#XMLConverter}

The OgreXMLConverter tool can converter binary .mesh and .skeleton files to XML and back again - this is a very useful tool for debugging the contents of meshes, or for exchanging mesh data easily - many of the modeller mesh exporters export to XML because it is simpler to do, and OgreXMLConverter can then produce a binary from it. Other than simplicity, the other advantage is that OgreXMLConverter can generate additional information for the mesh, like bounding regions and level-of-detail reduction. 

Syntax:

```
Usage: OgreXMLConverter sourcefile [destfile]
sourcefile = name of file to convert
destfile   = optional name of file to write to. If you don't
             specify this OGRE works it out through the extension
             and the XML contents if the source is XML. For example
             test.mesh becomes test.xml, test.xml becomes test.mesh
             if the XML document root is <mesh> etc.
```

When converting XML to .mesh, you will be prompted to (re)generate level-of-detail(LOD) information for the mesh - you can choose to skip this part if you wish, but doing it will allow you to make your mesh reduce in detail automatically when it is loaded into the engine. The engine uses a complex algorithm to determine the best parts of the mesh to reduce in detail depending on many factors such as the curvature of the surface, the edges of the mesh and seams at the edges of textures and smoothing groups - taking advantage of it is advised to make your meshes more scalable in real scenes.



# MeshUpgrader {#MeshUpgrader}

This tool is provided to allow you to upgrade your meshes when the binary format changes - sometimes we alter it to add new features and as such you need to keep your own assets up to date. This tools has a very simple syntax:

```cpp
OgreMeshUpgrader <oldmesh> <newmesh>
```

The OGRE release notes will notify you when this is necessary with a new release.

@page External-Texture-Sources External Texture Sources

This tutorial will provide a brief introduction of ExternalTextureSource and ExternalTextureSourceManager classes, their relationship, and how the PlugIns work. For those interested in developing a Texture Source Plugin or maybe just wanting to know more about this system, take a look the ffmpegVideoSystem plugin, which you can find more about on the OGRE forums.

<a name="What-Is-An-External-Texture-Source_003f"></a>

# What Is An External Texture Source?

What is a texture source? Well, a texture source could be anything - png, bmp, jpeg, etc. However, loading textures from traditional bitmap files is already handled by another part OGRE. There are, however, other types of sources to get texture data from - i.e. mpeg/avi/etc movie files, flash, run-time generated source, user defined, etc.

How do external texture source plugins benefit OGRE? Well, the main answer is: adding support for any type of texture source does not require changing OGRE to support it... all that is involved is writing a new plugin. Additionally, because the manager uses the StringInterface class to issue commands/params, no change to the material script reader is needs to be made. As a result, if a plugin needs a special parameter set, it just creates a new command in it’s Parameter Dictionary. - see ffmpegVideoSystem plugin for an example. To make this work, two classes have been added to OGRE: ExternalTextureSource & ExternalTextureSourceManager.

<a name="ExternalTextureSource-Class"></a>

# ExternalTextureSource Class

The ExternalTextureSource class is the base class that Texture Source PlugIns must be derived from. It provides a generic framework (via StringInterface class) with a very limited amount of functionality. The most common of parameters can be set through the TexturePlugInSource class interface or via the StringInterface commands contained within this class. While this may seem like duplication of code, it is not. By using the string command interface, it becomes extremely easy for derived plugins to add any new types of parameters that it may need.

Default Command Parameters defined in ExternalTextureSource base class are:

-   Parameter Name: "filename" Argument Type: Ogre::String Sets a filename plugin will read from
-   Parameter Name: "play\_mode" Argument Type: Ogre::String Sets initial play mode to be used by the plugin - "play", "loop", "pause"
-   Parameter Name: "set\_T\_P\_S" Argument Type: Ogre::String Used to set the technique, pass, and texture unit level to apply this texture to. As an example: To set a technique level of 1, a pass level of 2, and a texture unit level of 3, send this string "1 2 3".
-   Parameter Name: "frames\_per\_second" Argument Type: Ogre::String Set a Frames per second update speed. (Integer Values only)

<a name="ExternalTextureSourceManager-Class"></a>

# ExternalTextureSourceManager Class

ExternalTextureSourceManager is responsible for keeping track of loaded Texture Source PlugIns. It also aids in the creation of texture source textures from scripts. It also is the interface you should use when dealing with texture source plugins.

Note: The function prototypes shown below are mockups - param names are simplified to better illustrate purpose here... Steps needed to create a new texture via ExternalTextureSourceManager:

-   Obviously, the first step is to have the desired plugin included in plugin.cfg for it to be loaded.
-   Set the desired PlugIn as Active via AdvancedTextureManager::getSingleton().SetCurrentPlugIn( String Type ); – type is whatever the plugin registers as handling (e.g. "video", "flash", "whatever", etc).
-   Note: Consult Desired PlugIn to see what params it needs/expects. Set params/value pairs via AdvancedTextureManager::getSingleton().getCurrentPlugIn()-&gt;setParameter( String Param, String Value );
-   After required params are set, a simple call to AdvancedTextureManager::getSingleton().getCurrentPlugIn()-&gt;createDefinedTexture( sMaterialName ); will create a texture to the material name given.



The manager also provides a method for deleting a texture source material: AdvancedTextureManager::DestroyAdvancedTexture( String sTextureName ); The destroy method works by broadcasting the material name to all loaded TextureSourcePlugIns, and the PlugIn who actually created the material is responsible for the deletion, while other PlugIns will just ignore the request. What this means is that you do not need to worry about which PlugIn created the material, or activating the PlugIn yourself. Just call the manager method to remove the material. Also, all texture plugins should handle cleanup when they are shutdown.

<a name="Texture-Source-Material-Script"></a>

# Texture Source Material Script

As mentioned earlier, the process of defining/creating texture sources can be done within material script file. Here is an example of a material script definition - Note: This example is based off the ffmpegVideoSystem plugin parameters.

```cpp
material Example/MyVideoExample
{
    technique
    {
        pass
        {
            texture_unit
            {
                texture_source video
                {
                    filename mymovie.mpeg
                    play_mode play
                    sound_mode on
                }
            }
        }
    }
}
```

Notice that the first two param/value pairs are defined in the ExternalTextureSource base class and that the third parameter/value pair is not defined in the base class... That parameter is added to the param dictionary by the ffmpegVideoPlugin... This shows that extending the functionality with the plugins is extremely easy. Also, pay particular attention to the line: texture\_source video. This line identifies that this texture unit will come from a texture source plugin. It requires one parameter that determines which texture plugin will be used. In the example shown, the plugin requested is one that registered with "video" name.

<a name="Simplified-Diagram-of-Process"></a>

# Simplified Diagram of Process

This diagram uses ffmpegVideoPlugin as example, but all plug ins will work the same in how they are registered/used here. Also note that TextureSource Plugins are loaded/registered before scripts are parsed. This does not mean that they are initialized... Plugins are not initialized until they are set active! This is to ensure that a rendersystem is set up before the plugins might make a call the rendersystem.

![](images/TextureSource.jpg)

@page Shadows Shadows

Shadows are clearly an important part of rendering a believable scene - they provide a more tangible feel to the objects in the scene, and aid the viewer in understanding the spatial relationship between objects. Unfortunately, shadows are also one of the most challenging aspects of 3D rendering, and they are still very much an active area of research. Whilst there are many techniques to render shadows, none is perfect and they all come with advantages and disadvantages. For this reason, Ogre provides multiple shadow implementations, with plenty of configuration settings, so you can choose which technique is most appropriate for your scene.

Shadow implementations fall into basically 2 broad categories: [Stencil Shadows](#Stencil-Shadows) and [Texture-based Shadows](#Texture_002dbased-Shadows). This describes the method by which the shape of the shadow is generated. In addition, there is more than one way to render the shadow into the scene: [Modulative Shadows](#Modulative-Shadows), which darkens the scene in areas of shadow, and [Additive Light Masking](#Additive-Light-Masking) which by contrast builds up light contribution in areas which are not in shadow. You also have the option of [Integrated Texture Shadows](#Integrated-Texture-Shadows) which gives you complete control over texture shadow application, allowing for complex single-pass shadowing shaders. Ogre supports all these combinations.

@tableofcontents

# Enabling shadows {#Enabling-shadows}

Shadows are disabled by default, here’s how you turn them on and configure them in the general sense:

1.  Enable a shadow technique on the SceneManager as the **first** thing you doing your scene setup. It is important that this is done first because the shadow technique can alter the way meshes are loaded. Here’s an example:

    `mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);`

2.  Create one or more lights. Note that not all light types are necessarily supported by all shadow techniques, you should check the sections about each technique to check. Note that if certain lights should not cast shadows, you can turn that off by calling setCastShadows(false) on the light, the default is true.
3.  Disable shadow casting on objects which should not cast shadows. Call setCastShadows(false) on objects you don’t want to cast shadows, the default for all objects is to cast shadows.
4.  Configure shadow far distance. You can limit the distance at which shadows are considered for performance reasons, by calling SceneManager::setShadowFarDistance.
5.  Turn off the receipt of shadows on materials that should not receive them. You can turn off the receipt of shadows (note, not the casting of shadows - that is done per-object) by calling Material::setReceiveShadows or using the receive\_shadows material attribute. This is useful for materials which should be considered self-illuminated for example. Note that transparent materials are typically excluded from receiving and casting shadows, although see the [transparency\_casts\_shadows](#transparency_005fcasts_005fshadows) option for exceptions.

# Opting out of shadows {#Opting-out-of-shadows}

By default Ogre treats all non-transparent objects as shadow casters and receivers (depending on the shadow technique they may not be able to be both at once, check the docs for your chosen technique first). You can disable shadows in various ways:

<dl compact="compact">
<dt>Turning off shadow casting on the light</dt> <dd>

Calling Light::setCastsShadows(false) will mean this light casts no shadows at all.

</dd> <dt>Turn off shadow receipt on a material</dt> <dd>

Calling Material::setReceiveShadows(false) will prevent any objects using this material from receiving shadows.

</dd> <dt>Turn off shadow casting on individual objects</dt> <dd>

Calling MovableObject::setCastsShadows(false) will disable shadow casting for this object.

</dd> <dt>Turn off shadows on an entire rendering queue group</dt> <dd>

Calling RenderQueueGroup::setShadowsEnabled(false) will turn off both shadow casting and receiving on an entire rendering queue group. This is useful because Ogre has to do light setup tasks per group in order to preserve the inter-group ordering. Ogre automatically disables shadows on a number of groups automatically, such as RENDER\_QUEUE\_BACKGROUND, RENDER\_QUEUE\_OVERLAY, RENDER\_QUEUE\_SKIES\_EARLY and RENDER\_QUEUE\_SKIES\_LATE. If you choose to use more rendering queues (and by default, you won’t be using any more than this plus the ’standard’ queue, so ignore this if you don’t know what it means!), be aware that each one can incur a light setup cost, and you should disable shadows on the additional ones you use if you can.

</dd> </dl>

# Stencil Shadows {#Stencil-Shadows}

Stencil shadows are a method by which a ’mask’ is created for the screen using a feature called the stencil buffer. This mask can be used to exclude areas of the screen from subsequent renders, and thus it can be used to either include or exclude areas in shadow. They are enabled by calling SceneManager::setShadowTechnique with a parameter of either `SHADOWTYPE_STENCIL_ADDITIVE` or `SHADOWTYPE_STENCIL_MODULATIVE`. Because the stencil can only mask areas to be either ’enabled’ or ’disabled’, stencil shadows have ’hard’ edges, that is to say clear dividing lines between light and shadow - it is not possible to soften these edges.

In order to generate the stencil, ’shadow volumes’ are rendered by extruding the silhouette of the shadow caster away from the light. Where these shadow volumes intersect other objects (or the caster, since self-shadowing is supported using this technique), the stencil is updated, allowing subsequent operations to differentiate between light and shadow. How exactly this is used to render the shadows depends on whether [Modulative Shadows](#Modulative-Shadows) or [Additive Light Masking](#Additive-Light-Masking) is being used. Objects can both cast and receive stencil shadows, so self-shadowing is inbuilt. 

The advantage of stencil shadows is that they can do self-shadowing simply on low-end hardware, provided you keep your poly count under control. In contrast doing self-shadowing with texture shadows requires a fairly modern machine (See [Texture-based Shadows](#Texture_002dbased-Shadows)). For this reason, you’re likely to pick stencil shadows if you need an accurate shadowing solution for an application aimed at older or lower-spec machines.

The disadvantages of stencil shadows are numerous though, especially on more modern hardware. Because stencil shadows are a geometric technique, they are inherently more costly the higher the number of polygons you use, meaning you are penalized the more detailed you make your meshes. The fillrate cost, which comes from having to render shadow volumes, also escalates the same way. Since more modern applications are likely to use higher polygon counts, stencil shadows can start to become a bottleneck. In addition, the visual aspects of stencil shadows are pretty primitive - your shadows will always be hard-edged, and you have no possibility of doing clever things with shaders since the stencil is not available for manipulation there. Therefore, if your application is aimed at higher-end machines you should definitely consider switching to texture shadows (See [Texture-based Shadows](#Texture_002dbased-Shadows)).

There are a number of issues to consider which are specific to stencil shadows:

-   [CPU Overhead](#CPU-Overhead)
-   [Extrusion distance](#Extrusion-distance)
-   [Camera far plane positioning](#Camera-far-plane-positioning)
-   [Mesh edge lists](#Mesh-edge-lists)
-   [The Silhouette Edge](#The-Silhouette-Edge)
-   [Be realistic](#Be-realistic)
-   [Stencil Optimisations Performed By Ogre](#Stencil-Optimisations-Performed-By-Ogre)

<a name="CPU-Overhead"></a><a name="CPU-Overhead-1"></a>

## CPU Overhead

Calculating the shadow volume for a mesh can be expensive, and it has to be done on the CPU, it is not a hardware accelerated feature. Therefore, you can find that if you overuse this feature, you can create a CPU bottleneck for your application. Ogre quite aggressively eliminates objects which cannot be casting shadows on the frustum, but there are limits to how much it can do, and large, elongated shadows (e.g. representing a very low sun position) are very difficult to cull efficiently. Try to avoid having too many shadow casters around at once, and avoid long shadows if you can. Also, make use of the ’shadow far distance’ parameter on the SceneManager, this can eliminate distant shadow casters from the shadow volume construction and save you some time, at the expense of only having shadows for closer objects. Lastly, make use of Ogre’s Level-Of-Detail (LOD) features; you can generate automatically calculated LODs for your meshes in code (see the Mesh API docs) or when using the mesh tools such as OgreXMLConverter and OgreMeshUpgrader. Alternatively, you can assign your own manual LODs by providing alternative mesh files at lower detail levels. Both methods will cause the shadow volume complexity to decrease as the object gets further away, which saves you valuable volume calculation time.

<a name="Extrusion-distance"></a><a name="Extrusion-distance-1"></a>

## Extrusion distance

When vertex programs are not available, Ogre can only extrude shadow volumes a finite distance from the object. If an object gets too close to a light, any finite extrusion distance will be inadequate to guarantee all objects will be shadowed properly by this object. Therefore, you are advised not to let shadow casters pass too close to light sources if you can avoid it, unless you can guarantee that your target audience will have vertex program capable hardware (in this case, Ogre extrudes the volume to infinity using a vertex program so the problem does not occur). When infinite extrusion is not possible, Ogre uses finite extrusion, either derived from the attenuation range of a light (in the case of a point light or spotlight), or a fixed extrusion distance set in the application in the case of directional lights. To change the directional light extrusion distance, use SceneManager::setShadowDirectionalLightExtrusionDistance.

<a name="Camera-far-plane-positioning"></a><a name="Camera-far-plane-positioning-1"></a>

## Camera far plane positioning

Stencil shadow volumes rely very much on not being clipped by the far plane. When you enable stencil shadows, Ogre internally changes the far plane settings of your cameras such that there is no far plane - i.e. it is placed at infinity (Camera::setFarClipDistance(0)). This avoids artifacts caused by clipping the dark caps on shadow volumes, at the expense of a (very) small amount of depth precision.

<a name="Mesh-edge-lists"></a><a name="Mesh-edge-lists-1"></a>

## Mesh edge lists

Stencil shadows can only be calculated when an ’edge list’ has been built for all the geometry in a mesh. The official exporters and tools automatically build this for you (or have an option to do so), but if you create your own meshes, you must remember to build edge lists for them before using them with stencil shadows - you can do that by using OgreMeshUpgrade or OgreXmlConverter, or by calling Mesh::buildEdgeList before you export or use the mesh. If a mesh doesn’t have edge lists, OGRE assumes that it is not supposed to cast stencil shadows.

<a name="The-Silhouette-Edge"></a><a name="The-Silhouette-Edge-1"></a>

## The Silhouette Edge

Stencil shadowing is about finding a silhouette of the mesh, and projecting it away to form a volume. What this means is that there is a definite boundary on the shadow caster between light and shadow; a set of edges where where the triangle on one side is facing toward the light, and one is facing away. This produces a sharp edge around the mesh as the transition occurs. Provided there is little or no other light in the scene, and the mesh has smooth normals to produce a gradual light change in its underlying shading, the silhouette edge can be hidden - this works better the higher the tessellation of the mesh. However, if the scene includes ambient light, then the difference is far more marked. This is especially true when using [Modulative Shadows](#Modulative-Shadows), because the light contribution of each shadowed area is not taken into account by this simplified approach, and so using 2 or more lights in a scene using modulative stencil shadows is not advisable; the silhouette edges will be very marked. Additive lights do not suffer from this as badly because each light is masked individually, meaning that it is only ambient light which can show up the silhouette edges.

<a name="Be-realistic"></a><a name="Be-realistic-1"></a>

## Be realistic

Don’t expect to be able to throw any scene using any hardware at the stencil shadow algorithm and expect to get perfect, optimum speed results. Shadows are a complex and expensive technique, so you should impose some reasonable limitations on your placing of lights and objects; they’re not really that restricting, but you should be aware that this is not a complete free-for-all.

-   Try to avoid letting objects pass very close (or even through) lights - it might look nice but it’s one of the cases where artifacts can occur on machines not capable of running vertex programs.
-   Be aware that shadow volumes do not respect the ’solidity’ of the objects they pass through, and if those objects do not themselves cast shadows (which would hide the effect) then the result will be that you can see shadows on the other side of what should be an occluding object.
-   Make use of SceneManager::setShadowFarDistance to limit the number of shadow volumes constructed
-   Make use of LOD to reduce shadow volume complexity at distance
-   Avoid very long (dusk and dawn) shadows - they exacerbate other issues such as volume clipping, fillrate, and cause many more objects at a greater distance to require volume construction.

 <a name="Stencil-Optimisations-Performed-By-Ogre"></a><a name="Stencil-Optimisations-Performed-By-Ogre-1"></a>

## Stencil Optimisations Performed By Ogre

Despite all that, stencil shadows can look very nice (especially with [Additive Light Masking](#Additive-Light-Masking)) and can be fast if you respect the rules above. In addition, Ogre comes pre-packed with a lot of optimisations which help to make this as quick as possible. This section is more for developers or people interested in knowing something about the ’under the hood’ behaviour of Ogre.

<dl compact="compact">
<dt>Vertex program extrusion</dt> <dd>

As previously mentioned, Ogre performs the extrusion of shadow volumes in hardware on vertex program-capable hardware (e.g. GeForce3, Radeon 8500 or better). This has 2 major benefits; the obvious one being speed, but secondly that vertex programs can extrude points to infinity, which the fixed-function pipeline cannot, at least not without performing all calculations in software. This leads to more robust volumes, and also eliminates more than half the volume triangles on directional lights since all points are projected to a single point at infinity.

</dd> <dt>Scissor test optimisation</dt> <dd>

Ogre uses a scissor rectangle to limit the effect of point / spot lights when their range does not cover the entire viewport; that means we save fillrate when rendering stencil volumes, especially with distant lights

</dd> <dt>Z-Pass and Z-Fail algorithms</dt> <dd>

The Z-Fail algorithm, often attributed to John Carmack, is used in Ogre to make sure shadows are robust when the camera passes through the shadow volume. However, the Z-Fail algorithm is more expensive than the traditional Z-Pass; so Ogre detects when Z-Fail is required and only uses it then, Z-Pass is used at all other times.

</dd> <dt>2-Sided stenciling and stencil wrapping</dt> <dd>

Ogre supports the 2-Sided stenciling / stencil wrapping extensions, which when supported allow volumes to be rendered in a single pass instead of having to do one pass for back facing tris and another for front-facing tris. This doesn’t save fillrate, since the same number of stencil updates are done, but it does save primitive setup and the overhead incurred in the driver every time a render call is made.

</dd> <dt>Aggressive shadow volume culling</dt> <dd>

Ogre is pretty good at detecting which lights could be affecting the frustum, and from that, which objects could be casting a shadow on the frustum. This means we don’t waste time constructing shadow geometry we don’t need. Setting the shadow far distance is another important way you can reduce stencil shadow overhead since it culls far away shadow volumes even if they are visible, which is beneficial in practice since you’re most interested in shadows for close-up objects.

</dd> </dl>

# Texture-based Shadows {#Texture_002dbased-Shadows}

Texture shadows involve rendering shadow casters from the point of view of the light into a texture, which is then projected onto shadow receivers. The main advantage of texture shadows as opposed to [Stencil Shadows](#Stencil-Shadows) is that the overhead of increasing the geometric detail is far lower, since there is no need to perform per-triangle calculations. Most of the work in rendering texture shadows is done by the graphics card, meaning the technique scales well when taking advantage of the latest cards, which are at present outpacing CPUs in terms of their speed of development. In addition, texture shadows are **much** more customisable - you can pull them into shaders to apply as you like (particularly with [Integrated Texture Shadows](#Integrated-Texture-Shadows), you can perform filtering to create softer shadows or perform other special effects on them. Basically, most modern engines use texture shadows as their primary shadow technique simply because they are more powerful, and the increasing speed of GPUs is rapidly amortizing the fillrate / texture access costs of using them.

The main disadvantage to texture shadows is that, because they are simply a texture, they have a fixed resolution which means if stretched, the pixellation of the texture can become obvious. There are ways to combat this though:

<dl compact="compact">
<dt>Choosing a projection basis</dt> <dd>

The simplest projection is just to render the shadow casters from the lights perspective using a regular camera setup. This can look bad though, so there are many other projections which can help to improve the quality from the main camera’s perspective. OGRE supports pluggable projection bases via it’s ShadowCameraSetup class, and comes with several existing options - **Uniform** (which is the simplest), **Uniform Focussed** (which is still a normal camera projection, except that the camera is focussed into the area that the main viewing camera is looking at), LiSPSM (Light Space Perspective Shadow Mapping - which both focusses and distorts the shadow frustum based on the main view camera) and Plan Optimal (which seeks to optimise the shadow fidelity for a single receiver plane).

</dd> <dt>Filtering</dt> <dd>

You can also sample the shadow texture multiple times rather than once to soften the shadow edges and improve the appearance. Percentage Closest Filtering (PCF) is the most popular approach, although there are multiple variants depending on the number and pattern of the samples you take. Our shadows demo includes a 5-tap PCF example combined with depth shadow mapping.

</dd> <dt>Using a larger texture</dt> <dd>

Again as GPUs get faster and gain more memory, you can scale up to take advantage of this.

</dd> </dl>

If you combine all 3 of these techniques you can get a very high quality shadow solution.

The other issue is with point lights. Because texture shadows require a render to texture in the direction of the light, omnidirectional lights (point lights) would require 6 renders to totally cover all the directions shadows might be cast. For this reason, Ogre primarily supports directional lights and spotlights for generating texture shadows; you can use point lights but they will only work if off-camera since they are essentially turned into a spotlight shining into your camera frustum for the purposes of texture shadows.

<a name="Directional-Lights"></a>

## Directional Lights

Directional lights in theory shadow the entire scene from an infinitely distant light. Now, since we only have a finite texture which will look very poor quality if stretched over the entire scene, clearly a simplification is required. Ogre places a shadow texture over the area immediately in front of the camera, and moves it as the camera moves (although it rounds this movement to multiples of texels so that the slight ’swimming shadow’ effect caused by moving the texture is minimised). The range to which this shadow extends, and the offset used to move it in front of the camera, are configurable (See [Configuring Texture Shadows](#Configuring-Texture-Shadows)). At the far edge of the shadow, Ogre fades out the shadow based on other configurable parameters so that the termination of the shadow is softened.

<a name="Spotlights"></a>

## Spotlights

Spotlights are much easier to represent as renderable shadow textures than directional lights, since they are naturally a frustum. Ogre represents spotlight directly by rendering the shadow from the light position, in the direction of the light cone; the field-of-view of the texture camera is adjusted based on the spotlight falloff angles. In addition, to hide the fact that the shadow texture is square and has definite edges which could show up outside the spotlight, Ogre uses a second texture unit when projecting the shadow onto the scene which fades out the shadow gradually in a projected circle around the spotlight.

<a name="Point-Lights"></a>

## Point Lights

As mentioned above, to support point lights properly would require multiple renders (either 6 for a cubic render or perhaps 2 for a less precise parabolic mapping), so rather than do that we approximate point lights as spotlights, where the configuration is changed on the fly to make the light shine from its position over the whole of the viewing frustum. This is not an ideal setup since it means it can only really work if the point light’s position is out of view, and in addition the changing parameterisation can cause some ’swimming’ of the texture. Generally we recommend avoiding making point lights cast texture shadows.

<a name="Shadow-Casters-and-Shadow-Receivers"></a>

## Shadow Casters and Shadow Receivers

To enable texture shadows, use the shadow technique SHADOWTYPE\_TEXTURE\_MODULATIVE or SHADOWTYPE\_TEXTURE\_ADDITIVE; as the name suggests this produces [Modulative Shadows](#Modulative-Shadows) or [Additive Light Masking](#Additive-Light-Masking) respectively. The cheapest and simplest texture shadow techniques do not use depth information, they merely render casters to a texture and render this onto receivers as plain colour - this means self-shadowing is not possible using these methods. This is the default behaviour if you use the automatic, fixed-function compatible (and thus usable on lower end hardware) texture shadow techniques. You can however use shaders-based techniques through custom shadow materials for casters and receivers to perform more complex shadow algorithms, such as depth shadow mapping which does allow self-shadowing. OGRE comes with an example of this in its shadows demo, although it’s only usable on Shader Model 2 cards or better. Whilst fixed-function depth shadow mapping is available in OpenGL, it was never standardised in Direct3D so using shaders in custom caster & receiver materials is the only portable way to do it. If you use this approach, call SceneManager::setShadowTextureSelfShadow with a parameter of ’true’ to allow texture shadow casters to also be receivers.  If you’re not using depth shadow mapping, OGRE divides shadow casters and receivers into 2 disjoint groups. Simply by turning off shadow casting on an object, you automatically make it a shadow receiver (although this can be disabled by setting the ’receive\_shadows’ option to ’false’ in a material script. Similarly, if an object is set as a shadow caster, it cannot receive shadows.

# Configuring Texture Shadows {#Configuring-Texture-Shadows}

There are a number of settings which will help you configure your texture-based shadows so that they match your requirements.

-   [Maximum number of shadow textures](#Maximum-number-of-shadow-textures)
-   [Shadow texture size](#Shadow-texture-size)
-   [Shadow far distance](#Shadow-far-distance)
-   [Shadow texture offset (Directional Lights)](#Shadow-texture-offset-Directional-Lights_0029)
-   [Shadow fade settings](#Shadow-fade-settings)
-   [Custom shadow camera setups](#Custom-shadow-camera-setups)
-   [Shadow texture Depth Buffer sharing](#Shadow-texture-Depth-Buffer-sharing)
-   [Integrated Texture Shadows](#Integrated-Texture-Shadows)

<a name="Maximum-number-of-shadow-textures"></a><a name="Maximum-number-of-shadow-textures-1"></a>

## Maximum number of shadow textures

Shadow textures take up texture memory, and to avoid stalling the rendering pipeline Ogre does not reuse the same shadow texture for multiple lights within the same frame. This means that each light which is to cast shadows must have its own shadow texture. In practice, if you have a lot of lights in your scene you would not wish to incur that sort of texture overhead. You can adjust this manually by simply turning off shadow casting for lights you do not wish to cast shadows. In addition, you can set a maximum limit on the number of shadow textures Ogre is allowed to use by calling SceneManager::setShadowTextureCount. Each frame, Ogre determines the lights which could be affecting the frustum, and then allocates the number of shadow textures it is allowed to use to the lights on a first-come-first-served basis. Any additional lights will not cast shadows that frame. Note that you can set the number of shadow textures and their size at the same time by using the SceneManager::setShadowTextureSettings method; this is useful because both the individual calls require the potential creation / destruction of texture resources.

<a name="Shadow-texture-size"></a><a name="Shadow-texture-size-1"></a>

## Shadow texture size

The size of the textures used for rendering the shadow casters into can be altered; clearly using larger textures will give you better quality shadows, but at the expense of greater memory usage. Changing the texture size is done by calling SceneManager::setShadowTextureSize - textures are assumed to be square and you must specify a texture size that is a power of 2. Be aware that each modulative shadow texture will take size\*size\*3 bytes of texture memory.  **Important**: if you use the GL render system your shadow texture size can only be larger (in either dimension) than the size of your primary window surface if the hardware supports the Frame Buffer Object (FBO) or Pixel Buffer Object (PBO) extensions. Most modern cards support this now, but be careful of older cards - you can check the ability of the hardware to manage this through ogreRoot-&gt;getRenderSystem()-&gt;getCapabilities()-&gt;hasCapability(RSC\_HWRENDER\_TO\_TEXTURE). If this returns false, if you create a shadow texture larger in any dimension than the primary surface, the rest of the shadow texture will be blank.

<a name="Shadow-far-distance"></a><a name="Shadow-far-distance-1"></a>

## Shadow far distance

This determines the distance at which shadows are terminated; it also determines how far into the distance the texture shadows for directional lights are stretched - by reducing this value, or increasing the texture size, you can improve the quality of shadows from directional lights at the expense of closer shadow termination or increased memory usage, respectively.

<a name="Shadow-texture-offset-Directional-Lights_0029"></a>

## Shadow texture offset (Directional Lights)

As mentioned above in the directional lights section, the rendering of shadows for directional lights is an approximation that allows us to use a single render to cover a largish area with shadows. This offset parameter affects how far from the camera position the center of the shadow texture is offset, as a proportion of the shadow far distance. The greater this value, the more of the shadow texture is ’useful’ to you since it’s ahead of the camera, but also the further you offset it, the more chance there is of accidentally seeing the edge of the shadow texture at more extreme angles. You change this value by calling SceneManager::setShadowDirLightTextureOffset, the default is 0.6.

<a name="Shadow-fade-settings"></a><a name="Shadow-fade-settings-1"></a>

## Shadow fade settings

Shadows fade out before the shadow far distance so that the termination of shadow is not abrupt. You can configure the start and end points of this fade by calling the SceneManager::setShadowTextureFadeStart and SceneManager::setShadowTextureFadeEnd methods, both take distances as a proportion of the shadow far distance. Because of the inaccuracies caused by using a square texture and a radial fade distance, you cannot use 1.0 as the fade end, if you do you’ll see artifacts at the extreme edges. The default values are 0.7 and 0.9, which serve most purposes but you can change them if you like.

# Texture shadows and vertex / fragment programs

When rendering shadow casters into a modulative shadow texture, Ogre turns off all textures, and all lighting contributions except for ambient light, which it sets to the colour of the shadow ([Shadow Colour](#Shadow-Colour)). For additive shadows, it render the casters into a black & white texture instead. This is enough to render shadow casters for fixed-function material techniques, however where a vertex program is used Ogre doesn’t have so much control. If you use a vertex program in the **first pass** of your technique, then you must also tell ogre which vertex program you want it to use when rendering the shadow caster; see [Shadows and Vertex Programs](#Shadows-and-Vertex-Programs) for full details.

<a name="Custom-shadow-camera-setups"></a><a name="Custom-shadow-camera-setups-1"></a>

## Custom shadow camera setups

As previously mentioned, one of the downsides of texture shadows is that the texture resolution is finite, and it’s possible to get aliasing when the size of the shadow texel is larger than a screen pixel, due to the projection of the texture. In order to address this, you can specify alternative projection bases by using or creating subclasses of the ShadowCameraSetup class. The default version is called DefaultShadowCameraSetup and this sets up a simple regular frustum for point and spotlights, and an orthographic frustum for directional lights. There is also a PlaneOptimalShadowCameraSetup class which specialises the projection to a plane, thus giving you much better definition provided your shadow receivers exist mostly in a single plane. Other setup classes (e.g. you might create a perspective or trapezoid shadow mapping version) can be created and plugged in at runtime, either on individual lights or on the SceneManager as a whole.

<a name="Shadow-texture-Depth-Buffer-sharing"></a><a name="Shadow-texture-Depth-Buffer-sharing-1"></a>

## Shadow texture Depth Buffer sharing

Shadow textures need a depth buffer like many other RTs (Render Textures). Prior to Ogre 1.8, the depth buffer behavior was left undefined leaving a very small possibility of causing inconsistencies across different window resolutions and render systems. Depending on the render window’s resolutions and/or rendersystem being used, the depth buffer might been shared with the render window or a new one could get created to suite the shadow textures. If the application was depending on the depth buffer contents from the previous scene render (that is, no clear was performed) where a shadow texture render pass was in the middle; then the depth buffer would’ve contained garbage (but not consistent on all machines) causing graphical glitches hard to spot.

From Ogre 1.8 onwards the depth buffer usage & sharing can be flexible controlled through the use of depth pool IDs. These pool IDs are not specifically part of shadow textures, but rather anything involving RTs. All RTs with the same pool ID share the same depth buffers when possible (following RenderSystem API rules, check RenderSystemCapabilities flags to find out what the behavior will be). The default ID for shadow textures is 1; which is the same default value for render windows, and RTTs; thus maintaining the same behavior with older applications while achieving maximum memory saving and performance efficiency because the number of created depth buffers is as lowest as possible.

However there are some reasons to put shadow textures in a separate pool. This holds specially true if the application depends on the previous contents from the depth buffer before the shadow pass, instead of doing a clear:

-   In Direct3D9, the shadow texture is more likely to share the depth buffer with the render window at high resolutions (when the window is bigger than the shadow texture resolution), but at low resolutions it won’t be shared, thus causing two different behaviors. Also probably the shadow texture will share the depth buffers with most other RTTs (i.e. compositors)
-   In OpenGL 2.1, the shadow texture can’t be shared with the main render window; and most likely will NOT be shared with many other RTTs (i.e. compositors) since OGL 2.1 has a requirement that texture resolutions should exactly match, while D3D9 specifies depth buffers can be shared as long as the resolutions are equal or less.

For example, the DeferredShading sample suffers from this problem. If this is a problem for a particular effect you’re trying to achieve, you can specify a custom pool ID so that shadow textures get their own depth buffer(s), ensuring they aren’t shared with other RTs. You can set the poolId parameter from either SceneManager::setShadowTextureSettings or setShadowTextureConfig

```cpp
mSceneMgr->setShadowTextureSettings( size, count, format, PoolId );
mSceneMgr->setShadowTextureConfig( 0, 512, 512, PF_FLOAT16_R, 50 );
```

Note a poolId of 0 will make the shadow textures not to use a depth buffer, which isn’t usually a desired behavior.

<a name="Integrated-Texture-Shadows"></a><a name="Integrated-Texture-Shadows-1"></a>

## Integrated Texture Shadows

Texture shadows have one major advantage over stencil shadows - the data used to represent them can be referenced in regular shaders. Whilst the default texture shadow modes (SHADOWTYPE\_TEXTURE\_MODULATIVE and SHADOWTYPE\_TEXTURE\_ADDITIVE) automatically render shadows for you, their disadvantage is that because they are generalised add-ons to your own materials, they tend to take more passes of the scene to use. In addition, you don’t have a lot of control over the composition of the shadows.

Here is where ’integrated’ texture shadows step in. Both of the texture shadow types above have alternative versions called SHADOWTYPE\_TEXTURE\_MODULATIVE\_INTEGRATED and SHADOWTYPE\_TEXTURE\_ADDITIVE\_INTEGRATED, where instead of rendering the shadows for you, it just creates the texture shadow and then expects you to use that shadow texture as you see fit when rendering receiver objects in the scene. The downside is that you have to take into account shadow receipt in every one of your materials if you use this option - the upside is that you have total control over how the shadow textures are used. The big advantage here is that you can can perform more complex shading, taking into account shadowing, than is possible using the generalised bolt-on approaches, AND you can probably write them in a smaller number of passes, since you know precisely what you need and can combine passes where possible. When you use one of these shadowing approaches, the only difference between additive and modulative is the colour of the casters in the shadow texture (the shadow colour for modulative, black for additive) - the actual calculation of how the texture affects the receivers is of course up to you. No separate modulative pass will be performed, and no splitting of your materials into ambient / per-light / decal etc will occur - absolutely everything is determined by your original material (which may have modulative passes or per-light iteration if you want of course, but it’s not required).

You reference a shadow texture in a material which implements this approach by using the ’[content\_type](#content_005ftype) shadow’ directive in your [texture\_unit](#Texture-Units). It implicitly references a shadow texture based on the number of times you’ve used this directive in the same pass, and the light\_start option or light-based pass iteration, which might start the light index higher than 0.

# Modulative Shadows {#Modulative-Shadows}

Modulative shadows work by darkening an already rendered scene with a fixed colour. First, the scene is rendered normally containing all the objects which will be shadowed, then a modulative pass is done per light, which darkens areas in shadow. Finally, objects which do not receive shadows are rendered.

There are 2 modulative shadow techniques; stencil-based (See [Stencil Shadows](#Stencil-Shadows) : SHADOWTYPE\_STENCIL\_MODULATIVE) and texture-based (See [Texture-based Shadows](#Texture_002dbased-Shadows) : SHADOWTYPE\_TEXTURE\_MODULATIVE). Modulative shadows are an inaccurate lighting model, since they darken the areas of shadow uniformly, irrespective of the amount of light which would have fallen on the shadow area anyway. However, they can give fairly attractive results for a much lower overhead than more ’correct’ methods like [Additive Light Masking](#Additive-Light-Masking), and they also combine well with pre-baked static lighting (such as pre-calculated lightmaps), which additive lighting does not. The main thing to consider is that using multiple light sources can result in overly dark shadows (where shadows overlap, which intuitively looks right in fact, but it’s not physically correct) and artifacts when using stencil shadows (See [The Silhouette Edge](#The-Silhouette-Edge)). 

<a name="Shadow-Colour"></a><a name="Shadow-Colour-1"></a>

## Shadow Colour

The colour which is used to darken the areas in shadow is set by SceneManager::setShadowColour; it defaults to a dark grey (so that the underlying colour still shows through a bit).

Note that if you’re using texture shadows you have the additional option of using [Integrated Texture Shadows](#Integrated-Texture-Shadows) rather than being forced to have a separate pass of the scene to render shadows. In this case the ’modulative’ aspect of the shadow technique just affects the colour of the shadow texture. 

# Additive Light Masking {#Additive-Light-Masking}

Additive light masking is about rendering the scene many times, each time representing a single light contribution whose influence is masked out in areas of shadow. Each pass is combined with (added to) the previous one such that when all the passes are complete, all the light contribution has correctly accumulated in the scene, and each light has been prevented from affecting areas which it should not be able to because of shadow casters. This is an effective technique which results in very realistic looking lighting, but it comes at a price: more rendering passes.

As many technical papers (and game marketing) will tell you, rendering realistic lighting like this requires multiple passes. Being a friendly sort of engine, Ogre frees you from most of the hard work though, and will let you use the exact same material definitions whether you use this lighting technique or not (for the most part, see [Pass Classification and Vertex Programs](#Pass-Classification-and-Vertex-Programs)). In order to do this technique, Ogre automatically categorises the [Passes](#Passes) you define in your materials into 3 types:

1.  ambient Passes categorised as ’ambient’ include any base pass which is not lit by any particular light, i.e. it occurs even if there is no ambient light in the scene. The ambient pass always happens first, and sets up the initial depth value of the fragments, and the ambient colour if applicable. It also includes any emissive / self illumination contribution. Only textures which affect ambient light (e.g. ambient occlusion maps) should be rendered in this pass.
2.  diffuse/specular Passes categorised as ’diffuse/specular’ (or ’per-light’) are rendered once per light, and each pass contributes the diffuse and specular colour from that single light as reflected by the diffuse / specular terms in the pass. Areas in shadow from that light are masked and are thus not updated. The resulting masked colour is added to the existing colour in the scene. Again, no textures are used in this pass (except for textures used for lighting calculations such as normal maps).
3.  decal Passes categorised as ’decal’ add the final texture colour to the scene, which is modulated by the accumulated light built up from all the ambient and diffuse/specular passes.

In practice, [Passes](#Passes) rarely fall nicely into just one of these categories. For each Technique, Ogre compiles a list of ’Illumination Passes’, which are derived from the user defined passes, but can be split, to ensure that the divisions between illumination pass categories can be maintained. For example, if we take a very simple material definition:

```cpp
material TestIllumination
{
    technique
    {
        pass
        {
            ambient 0.5 0.2 0.2 
            diffuse 1 0 0  
            specular 1 0.8 0.8 15
            texture_unit
            {
                texture grass.png
            }
        }
    }
}
```

Ogre will split this into 3 illumination passes, which will be the equivalent of this:

```cpp
material TestIlluminationSplitIllumination
{
    technique
    {
        // Ambient pass
        pass
        {
            ambient 0.5 0.2 0.2 
            diffuse 0 0 0
            specular 0 0 0
        }

        // Diffuse / specular pass
        pass
        {
            scene_blend add
            iteration once_per_light
            diffuse 1 0 0  
            specular 1 0.8 0.8 15
        }

        // Decal pass
        pass
        {
            scene_blend modulate
            lighting off
            texture_unit
            {
                texture grass.png
            }
        }
    }
}
```

So as you can see, even a simple material requires a minimum of 3 passes when using this shadow technique, and in fact it requires (num\_lights + 2) passes in the general sense. You can use more passes in your original material and Ogre will cope with that too, but be aware that each pass may turn into multiple ones if it uses more than one type of light contribution (ambient vs diffuse/specular) and / or has texture units. The main nice thing is that you get the full multipass lighting behaviour even if you don’t define your materials in terms of it, meaning that your material definitions can remain the same no matter what lighting approach you decide to use.

<a name="Manually-Categorising-Illumination-Passes"></a><a name="Manually-Categorising-Illumination-Passes-1"></a>

## Manually Categorising Illumination Passes

Alternatively, if you want more direct control over the categorisation of your passes, you can use the [illumination\_stage](#illumination_005fstage) option in your pass to explicitly assign a pass unchanged to an illumination stage. This way you can make sure you know precisely how your material will be rendered under additive lighting conditions.

<a name="Pass-Classification-and-Vertex-Programs"></a><a name="Pass-Classification-and-Vertex-Programs-1"></a>

## Pass Classification and Vertex Programs

Ogre is pretty good at classifying and splitting your passes to ensure that the multipass rendering approach required by additive lighting works correctly without you having to change your material definitions. However, there is one exception; when you use vertex programs, the normal lighting attributes ambient, diffuse, specular etc are not used, because all of that is determined by the vertex program. Ogre has no way of knowing what you’re doing inside that vertex program, so you have to tell it.

In practice this is very easy. Even though your vertex program could be doing a lot of complex, highly customised processing, it can still be classified into one of the 3 types listed above. All you need to do to tell Ogre what you’re doing is to use the pass attributes ambient, diffuse, specular and self\_illumination, just as if you were not using a vertex program. Sure, these attributes do nothing (as far as rendering is concerned) when you’re using vertex programs, but it’s the easiest way to indicate to Ogre which light components you’re using in your vertex program. Ogre will then classify and potentially split your programmable pass based on this information - it will leave the vertex program as-is (so that any split passes will respect any vertex modification that is being done). 

Note that when classifying a diffuse/specular programmable pass, Ogre checks to see whether you have indicated the pass can be run once per light (iteration once\_per\_light). If so, the pass is left intact, including it’s vertex and fragment programs. However, if this attribute is not included in the pass, Ogre tries to split off the per-light part, and in doing so it will disable the fragment program, since in the absence of the ’iteration once\_per\_light’ attribute it can only assume that the fragment program is performing decal work and hence must not be used per light.

So clearly, when you use additive light masking as a shadow technique, you need to make sure that programmable passes you use are properly set up so that they can be classified correctly. However, also note that the changes you have to make to ensure the classification is correct does not affect the way the material renders when you choose not to use additive lighting, so the principle that you should be able to use the same material definitions for all lighting scenarios still holds. Here is an example of a programmable material which will be classified correctly by the illumination pass classifier:

```cpp
// Per-pixel normal mapping Any number of lights, diffuse and specular
material Examples/BumpMapping/MultiLightSpecular
{
    technique
    {
        // Base ambient pass
        pass
        {
            // ambient only, not needed for rendering, but as information
            // to lighting pass categorisation routine
            ambient 1 1 1
            diffuse 0 0 0 
            specular 0 0 0 0
            // Really basic vertex program
            vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
            {
                param_named_auto worldViewProj worldviewproj_matrix
                param_named_auto ambient ambient_light_colour
            }
        }
        // Now do the lighting pass
        // NB we don't do decal texture here because this is repeated per light
        pass
        {
            // set ambient off, not needed for rendering, but as information
            // to lighting pass categorisation routine
            ambient 0 0 0 
            // do this for each light
            iteration once_per_light
            scene_blend add

            // Vertex program reference
            vertex_program_ref Examples/BumpMapVPSpecular
            {
                param_named_auto lightPosition light_position_object_space 0
                param_named_auto eyePosition camera_position_object_space
                param_named_auto worldViewProj worldviewproj_matrix
            }

            // Fragment program
            fragment_program_ref Examples/BumpMapFPSpecular
            {
                param_named_auto lightDiffuse light_diffuse_colour 0 
                param_named_auto lightSpecular light_specular_colour 0
            }
            
            // Base bump map
            texture_unit
            {
                texture NMBumpsOut.png
                colour_op replace
            }
            // Normalisation cube map
            texture_unit
            {
                cubic_texture nm.png combinedUVW
                tex_coord_set 1
                tex_address_mode clamp
            }
            // Normalisation cube map #2
            texture_unit
            {
                cubic_texture nm.png combinedUVW
                tex_coord_set 1
                tex_address_mode clamp
            }
        }
        
        // Decal pass
        pass
        {
            lighting off
            // Really basic vertex program
            vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
            {
                param_named_auto worldViewProj worldviewproj_matrix
                param_named ambient float4 1 1 1 1
            }
            scene_blend dest_colour zero
            texture_unit
            {
                texture RustedMetal.jpg 
            }
        }
    }
}
```

Note that if you’re using texture shadows you have the additional option of using [Integrated Texture Shadows](#Integrated-Texture-Shadows) rather than being forced to use this explicit sequence - allowing you to compress the number of passes into a much smaller number at the expense of defining an upper number of shadow casting lights. In this case the ’additive’ aspect of the shadow technique just affects the colour of the shadow texture and it’s up to you to combine the shadow textures in your receivers however you like. 

<a name="Static-Lighting"></a>

## Static Lighting

Despite their power, additive lighting techniques have an additional limitation; they do not combine well with pre-calculated static lighting in the scene. This is because they are based on the principle that shadow is an absence of light, but since static lighting in the scene already includes areas of light and shadow, additive lighting cannot remove light to create new shadows. Therefore, if you use the additive lighting technique you must either use it exclusively as your lighting solution (and you can combine it with per-pixel lighting to create a very impressive dynamic lighting solution), or you must use [Integrated Texture Shadows](#Integrated-Texture-Shadows) to combine the static lighting according to your chosen approach.

@page Animation Animation

OGRE supports a pretty flexible animation system that allows you to script animation for several different purposes:

<dl compact="compact">
<dt>[Skeletal Animation](#Skeletal-Animation)</dt> <dd>

Mesh animation using a skeletal structure to determine how the mesh deforms. <br>

</dd> <dt>[Vertex Animation](#Vertex-Animation)</dt> <dd>

Mesh animation using snapshots of vertex data to determine how the shape of the mesh changes.<br>

</dd> <dt>[SceneNode Animation](#SceneNode-Animation)</dt> <dd>

Animating SceneNodes automatically to create effects like camera sweeps, objects following predefined paths, etc.<br>

</dd> <dt>[Numeric Value Animation](#Numeric-Value-Animation)</dt> <dd>

Using OGRE’s extensible class structure to animate any value.

</dd> </dl>



# Skeletal Animation {#Skeletal-Animation}

Skeletal animation is a process of animating a mesh by moving a set of hierarchical bones within the mesh, which in turn moves the vertices of the model according to the bone assignments stored in each vertex. An alternative term for this approach is ’skinning’. The usual way of creating these animations is with a modelling tool such as Softimage XSI, Milkshape 3D, Blender, 3D Studio or Maya among others. OGRE provides exporters to allow you to get the data out of these modellers and into the engine See [Exporters](@ref Exporters).

There are many grades of skeletal animation, and not all engines (or modellers for that matter) support all of them. OGRE supports the following features:

-   Each mesh can be linked to a single skeleton
-   Unlimited bones per skeleton
-   Hierarchical forward-kinematics on bones
-   Multiple named animations per skeleton (e.g. ’Walk’, ’Run’, ’Jump’, ’Shoot’ etc)
-   Unlimited keyframes per animation
-   Linear or spline-based interpolation between keyframes
-   A vertex can be assigned to multiple bones and assigned weightings for smoother skinning
-   Multiple animations can be applied to a mesh at the same time, again with a blend weighting

<br>

Skeletons and the animations which go with them are held in .skeleton files, which are produced by the OGRE exporters. These files are loaded automatically when you create an Entity based on a Mesh which is linked to the skeleton in question. You then use [Animation State](#Animation-State) to set the use of animation on the entity in question.

Skeletal animation can be performed in software, or implemented in shaders (hardware skinning). Clearly the latter is preferable, since it takes some of the work away from the CPU and gives it to the graphics card, and also means that the vertex data does not need to be re-uploaded every frame. This is especially important for large, detailed models. You should try to use hardware skinning wherever possible; this basically means assigning a material which has a vertex program powered technique. See [Skeletal Animation in Vertex Programs](#Skeletal-Animation-in-Vertex-Programs) for more details. Skeletal animation can be combined with vertex animation, See [Combining Skeletal and Vertex Animation](#Combining-Skeletal-and-Vertex-Animation).


<a name="Animation-State"></a>
## Animation State

When an entity containing animation of any type is created, it is given an ’animation state’ object per animation to allow you to specify the animation state of that single entity (you can animate multiple entities using the same animation definitions, OGRE sorts the reuse out internally).

You can retrieve a pointer to the AnimationState object by calling Entity::getAnimationState. You can then call methods on this returned object to update the animation, probably in the frameStarted event. Each AnimationState needs to be enabled using the setEnabled method before the animation it refers to will take effect, and you can set both the weight and the time position (where appropriate) to affect the application of the animation using correlating methods. AnimationState also has a very simple method ’addTime’ which allows you to alter the animation position incrementally, and it will automatically loop for you. addTime can take positive or negative values (so you can reverse the animation if you want).



# Vertex Animation {#Vertex-Animation}

Vertex animation is about using information about the movement of vertices directly to animate the mesh. Each track in a vertex animation targets a single VertexData instance. Vertex animation is stored inside the .mesh file since it is tightly linked to the vertex structure of the mesh.

There are actually two subtypes of vertex animation, for reasons which will be discussed in a moment.

<dl compact="compact">
<dt>[Morph Animation](#Morph-Animation)</dt> <dd>

Morph animation is a very simple technique which interpolates mesh snapshots along a keyframe timeline. Morph animation has a direct correlation to old-school character animation techniques used before skeletal animation was widely used.<br>

</dd> <dt>[Pose Animation](#Pose-Animation)</dt> <dd>

Pose animation is about blending multiple discrete poses, expressed as offsets to the base vertex data, with different weights to provide a final result. Pose animation’s most obvious use is facial animation.

</dd> </dl> <a name="Why-two-subtypes_003f"></a>

## Why two subtypes?

So, why two subtypes of vertex animation? Couldn’t both be implemented using the same system? The short answer is yes; in fact you can implement both types using pose animation. But for very good reasons we decided to allow morph animation to be specified separately since the subset of features that it uses is both easier to define and has lower requirements on hardware shaders, if animation is implemented through them. If you don’t care about the reasons why these are implemented differently, you can skip to the next part.

Morph animation is a simple approach where we have a whole series of snapshots of vertex data which must be interpolated, e.g. a running animation implemented as morph targets. Because this is based on simple snapshots, it’s quite fast to use when animating an entire mesh because it’s a simple linear change between keyframes. However, this simplistic approach does not support blending between multiple morph animations. If you need animation blending, you are advised to use skeletal animation for full-mesh animation, and pose animation for animation of subsets of meshes or where skeletal animation doesn’t fit - for example facial animation. For animating in a vertex shader, morph animation is quite simple and just requires the 2 vertex buffers (one the original position buffer) of absolute position data, and an interpolation factor. Each track in a morph animation references a unique set of vertex data. 

Pose animation is more complex. Like morph animation each track references a single unique set of vertex data, but unlike morph animation, each keyframe references 1 or more ’poses’, each with an influence level. A pose is a series of offsets to the base vertex data, and may be sparse - i.e. it may not reference every vertex. Because they’re offsets, they can be blended - both within a track and between animations. This set of features is very well suited to facial animation. 

For example, let’s say you modelled a face (one set of vertex data), and defined a set of poses which represented the various phonetic positions of the face. You could then define an animation called ’SayHello’, containing a single track which referenced the face vertex data, and which included a series of keyframes, each of which referenced one or more of the facial positions at different influence levels - the combination of which over time made the face form the shapes required to say the word ’hello’. Since the poses are only stored once, but can be referenced may times in many animations, this is a very powerful way to build up a speech system.

The downside of pose animation is that it can be more difficult to set up, requiring poses to be separately defined and then referenced in the keyframes. Also, since it uses more buffers (one for the base data, and one for each active pose), if you’re animating in hardware using vertex shaders you need to keep an eye on how many poses you’re blending at once. You define a maximum supported number in your vertex program definition, via the includes\_pose\_animation material script entry, See [Pose Animation in Vertex Programs](@ref Pose-Animation-in-Vertex-Programs).

So, by partitioning the vertex animation approaches into 2, we keep the simple morph technique easy to use, whilst still allowing all the powerful techniques to be used. Note that morph animation cannot be blended with other types of vertex animation on the same vertex data (pose animation or other morph animation); pose animation can be blended with other pose animation though, and both types can be combined with skeletal animation. This combination limitation applies per set of vertex data though, not globally across the mesh (see below). Also note that all morph animation can be expressed (in a more complex fashion) as pose animation, but not vice versa.

<a name="Subtype-applies-per-track"></a>

## Subtype applies per track

It’s important to note that the subtype in question is held at a track level, not at the animation or mesh level. Since tracks map onto VertexData instances, this means that if your mesh is split into SubMeshes, each with their own dedicated geometry, you can have one SubMesh animated using pose animation, and others animated with morph animation (or not vertex animated at all). 

For example, a common set-up for a complex character which needs both skeletal and facial animation might be to split the head into a separate SubMesh with its own geometry, then apply skeletal animation to both submeshes, and pose animation to just the head. 

To see how to apply vertex animation, See [Animation State](#Animation-State).

<a name="Vertex-buffer-arrangements"></a>

## Vertex buffer arrangements

When using vertex animation in software, vertex buffers need to be arranged such that vertex positions reside in their own hardware buffer. This is to avoid having to upload all the other vertex data when updating, which would quickly saturate the GPU bus. When using the OGRE .mesh format and the tools / exporters that go with it, OGRE organises this for you automatically. But if you create buffers yourself, you need to be aware of the layout arrangements.

To do this, you have a set of helper functions in Ogre::Mesh. See API Reference entries for Ogre::VertexData::reorganiseBuffers() and Ogre::VertexDeclaration::getAutoOrganisedDeclaration(). The latter will turn a vertex declaration into one which is recommended for the usage you’ve indicated, and the former will reorganise the contents of a set of buffers to conform to that layout.



<a name="Morph-Animation"></a> <a name="Morph-Animation-1"></a>

## Morph Animation

Morph animation works by storing snapshots of the absolute vertex positions in each keyframe, and interpolating between them. Morph animation is mainly useful for animating objects which could not be adequately handled using skeletal animation; this is mostly objects that have to radically change structure and shape as part of the animation such that a skeletal structure isn’t appropriate. 

Because absolute positions are used, it is not possible to blend more than one morph animation on the same vertex data; you should use skeletal animation if you want to include animation blending since it is much more efficient. If you activate more than one animation which includes morph tracks for the same vertex data, only the last one will actually take effect. This also means that the ’weight’ option on the animation state is not used for morph animation. 

Morph animation can be combined with skeletal animation if required See [Combining Skeletal and Vertex Animation](#Combining-Skeletal-and-Vertex-Animation). Morph animation can also be implemented in hardware using vertex shaders, See [Morph Animation in Vertex Programs](#Morph-Animation-in-Vertex-Programs).



<a name="Pose-Animation"></a> <a name="Pose-Animation-1"></a>

## Pose Animation

Pose animation allows you to blend together potentially multiple vertex poses at different influence levels into final vertex state. A common use for this is facial animation, where each facial expression is placed in a separate animation, and influences used to either blend from one expression to another, or to combine full expressions if each pose only affects part of the face.

In order to do this, pose animation uses a set of reference poses defined in the mesh, expressed as offsets to the original vertex data. It does not require that every vertex has an offset - those that don’t are left alone. When blending in software these vertices are completely skipped - when blending in hardware (which requires a vertex entry for every vertex), zero offsets for vertices which are not mentioned are automatically created for you.

Once you’ve defined the poses, you can refer to them in animations. Each pose animation track refers to a single set of geometry (either the shared geometry of the mesh, or dedicated geometry on a submesh), and each keyframe in the track can refer to one or more poses, each with its own influence level. The weight applied to the entire animation scales these influence levels too. You can define many keyframes which cause the blend of poses to change over time. The absence of a pose reference in a keyframe when it is present in a neighbouring one causes it to be treated as an influence of 0 for interpolation. 

You should be careful how many poses you apply at once. When performing pose animation in hardware (See [Pose Animation in Vertex Programs](@ ref Pose-Animation-in-Vertex-Programs)), every active pose requires another vertex buffer to be added to the shader, and in when animating in software it will also take longer the more active poses you have. Bear in mind that if you have 2 poses in one keyframe, and a different 2 in the next, that actually means there are 4 active keyframes when interpolating between them. 

You can combine pose animation with skeletal animation, See [Combining Skeletal and Vertex Animation](#Combining-Skeletal-and-Vertex-Animation), and you can also hardware accelerate the application of the blend with a vertex shader, See [Pose Animation in Vertex Programs](@ref Pose-Animation-in-Vertex-Programs).



<a name="Combining-Skeletal-and-Vertex-Animation"></a> <a name="Combining-Skeletal-and-Vertex-Animation-1"></a>

## Combining Skeletal and Vertex Animation

Skeletal animation and vertex animation (of either subtype) can both be enabled on the same entity at the same time (See [Animation State](#Animation-State)). The effect of this is that vertex animation is applied first to the base mesh, then skeletal animation is applied to the result. This allows you, for example, to facially animate a character using pose vertex animation, whilst performing the main movement animation using skeletal animation.

Combining the two is, from a user perspective, as simple as just enabling both animations at the same time. When it comes to using this feature efficiently though, there are a few points to bear in mind:

-   [Combined Hardware Skinning](#Combined-Hardware-Skinning)
-   [Submesh Splits](#Submesh-Splits)

<a name="Combined-Hardware-Skinning"></a><a name="Combined-Hardware-Skinning-1"></a>

## Combined Hardware Skinning

For complex characters it is a very good idea to implement hardware skinning by including a technique in your materials which has a vertex program which can perform the kinds of animation you are using in hardware. See [Skeletal Animation in Vertex Programs](@ref Skeletal-Animation-in-Vertex-Programs), [Morph Animation in Vertex Programs](@ref Morph-Animation-in-Vertex-Programs), [Pose Animation in Vertex Programs](@ref Pose-Animation-in-Vertex-Programs). 

When combining animation types, your vertex programs must support both types of animation that the combined mesh needs, otherwise hardware skinning will be disabled. You should implement the animation in the same way that OGRE does, i.e. perform vertex animation first, then apply skeletal animation to the result of that. Remember that the implementation of morph animation passes 2 absolute snapshot buffers of the from & to keyframes, along with a single parametric, which you have to linearly interpolate, whilst pose animation passes the base vertex data plus ’n’ pose offset buffers, and ’n’ parametric weight values. 

<a name="Submesh-Splits"></a><a name="Submesh-Splits-1"></a>

## Submesh Splits

If you only need to combine vertex and skeletal animation for a small part of your mesh, e.g. the face, you could split your mesh into 2 parts, one which needs the combination and one which does not, to reduce the calculation overhead. Note that it will also reduce vertex buffer usage since vertex keyframe / pose buffers will also be smaller. Note that if you use hardware skinning you should then implement 2 separate vertex programs, one which does only skeletal animation, and the other which does skeletal and vertex animation.



# SceneNode Animation {#SceneNode-Animation}

SceneNode animation is created from the SceneManager in order to animate the movement of SceneNodes, to make any attached objects move around automatically. You can see this performing a camera swoop in Demo\_CameraTrack, or controlling how the fish move around in the pond in Demo\_Fresnel.

At it’s heart, scene node animation is mostly the same code which animates the underlying skeleton in skeletal animation. After creating the main Animation using SceneManager::createAnimation you can create a NodeAnimationTrack per SceneNode that you want to animate, and create keyframes which control its position, orientation and scale which can be interpolated linearly or via splines. You use [Animation State](#Animation-State) in the same way as you do for skeletal/vertex animation, except you obtain the state from SceneManager instead of from an individual Entity.Animations are applied automatically every frame, or the state can be applied manually in advance using the \_applySceneAnimations() method on SceneManager. See the API reference for full details of the interface for configuring scene animations.



# Numeric Value Animation {#Numeric-Value-Animation}

Apart from the specific animation types which may well comprise the most common uses of the animation framework, you can also use animations to alter any value which is exposed via the [AnimableObject](#AnimableObject) interface. 

<a name="AnimableObject"></a><a name="AnimableObject-1"></a>

## AnimableObject

AnimableObject is an abstract interface that any class can extend in order to provide access to a number of [AnimableValue](#AnimableValue)s. It holds a ’dictionary’ of the available animable properties which can be enumerated via the getAnimableValueNames method, and when its createAnimableValue method is called, it returns a reference to a value object which forms a bridge between the generic animation interfaces, and the underlying specific object property.

One example of this is the Light class. It extends AnimableObject and provides AnimableValues for properties such as "diffuseColour" and "attenuation". Animation tracks can be created for these values and thus properties of the light can be scripted to change. Other objects, including your custom objects, can extend this interface in the same way to provide animation support to their properties.

<a name="AnimableValue"></a><a name="AnimableValue-1"></a>

## AnimableValue

When implementing custom animable properties, you have to also implement a number of methods on the AnimableValue interface - basically anything which has been marked as unimplemented. These are not pure virtual methods simply because you only have to implement the methods required for the type of value you’re animating. Again, see the examples in Light to see how this is done.
