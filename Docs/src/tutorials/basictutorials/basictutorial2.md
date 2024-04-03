# Lights, Cameras, and Shadows {#tut_LightsCamerasShadows}

@tableofcontents

This tutorial will expand on the use of Lights in a scene and using them to cast shadows.

The full source for this tutorial can be found in samples directory **Samples/Tutorials/BasicTutorial2.cpp**.

@note Refer to @ref setup for instructions how set up an Ogre project and compile it successfully.

# The Ogre Camera Class {#bt2TheOgreCameraClass}
A Camera is the object we use to view our scene. A Ogre::Camera is a special object that must be attached to a
Ogre::SceneNode. You can then use that for movement and rotation. For example, you might want to use a SceneNode that follows a path in the sky to create an impressive aerial cutscene

# Creating a Camera {#bt2CreatingaCamera}
We will now cover camera creation part which we just applied in previous tutorial. We remember that now we need to have SceneNode for camera. The first step will be doing is creating that SceneNode and asking the SceneManager to create a new Camera. Add the following to create SceneNode and Camera:

@snippet Samples/Tutorials/BasicTutorial2.cpp cameracreate
You can retrieve the Camera by name using the SceneManager's getCamera method.

Next, we will position the Camera and use a method called lookAt to set its direction using camNode.
@snippet Samples/Tutorials/BasicTutorial2.cpp cameraposition
The Ogre::SceneNode::lookAt method is very useful. It does exactly what it says. It rotates the SceneNode so that its line of sight focuses on the vector you give it. It makes the Camera "look at" the point.

[//]: <> (TODO: add explanation about second argument of lookAt method)

The last thing we'll do (apart from attaching camera to a SceneNode) is set the near clipping distance to 5 units. This is the distance at which the Camera will no longer render any mesh. If you get very close to a mesh, this will sometimes cut the mesh and allow you to see inside of it. The alternative is filling the entire screen with a tiny, highly magnified piece of the mesh's texture. It's up to you what you want in your scene. For demonstration, we'll set it here.

@snippet Samples/Tutorials/BasicTutorial2.cpp cameralaststep

# Viewports {#bt2Viewports}
When dealing with multiple Cameras in a scene, the concept of a Viewport becomes very useful. We will touch on it now, because it will help you understand more about how Ogre decides which Camera to use when rendering a scene. Ogre makes it possible to have multiple SceneManagers running at the same time. It also allows you to break up the screen and use separate Cameras to render different views of a scene. This would allow the creation of things like splitscreens and minimaps. These kinds of things will be covered in later tutorials.

There are three constructs that are crucial to understanding how Ogre renders a scene: the Camera, the SceneManager, and the RenderWindow. We have not yet covered the RenderWindow. It basically represents the whole window we are rendering to. The SceneManager will create Cameras to view the scene, and then we tell the RenderWindow where to display each Camera's view. The way we tell the RenderWindow which area of the screen to use is by giving it a Ogre::Viewport. For many circumstances, we will simply create one Camera and create a Viewport which represents the whole screen.

## Creating a Viewport {#bt2CreatingaViewport}
Let's create a Viewport for our scene. To do this, we will use the addViewport method of the RenderWindow.
@snippet Samples/Tutorials/BasicTutorial2.cpp addviewport
getRenderWindow() is a method defined for us in OgreBites::ApplicationContext which returns Ogre::RenderWindow.

Now let's set the background color of the Viewport.
@snippet Samples/Tutorials/BasicTutorial2.cpp viewportback

We've set it to black because we are going to add colored lighting later, and we don't want the background color affecting how we see the lighting.

The last thing we are going to do is set the aspect ratio of our Camera. If you are using something other than a standard full-window viewport, then failing to set this can result in a distorted scene. We will set it here for demonstration even though we are using the default aspect ratio.

@snippet Samples/Tutorials/BasicTutorial2.cpp cameraratio

We have retrieved the width and height from the Viewport to set the aspect ratio. As we mentioned, the default is already set to use the full screen's dimensions.

Compile and run your application. You should still only see a black screen, just make sure it runs.

# Building the Scene {#bt2BuildingtheScene}
Before we get to shadows and lighting, let's add some elements to our scene. Let's put a ninja right in the middle of things. Add the following code right after we set the ambient light:
@snippet Samples/Tutorials/BasicTutorial2.cpp ninja

This should look familiar, except we are asking the mesh to cast shadows this time. And notice that we have created a child scene node and attached the ninjaEntity all in one call this time.

We will also create something for the ninja to be standing on. We can use the Ogre::MeshManager to create meshes from scratch. We will use it to generate a textured plane to use as the ground.

The first thing we'll do is create an abstract Plane object. This is not the mesh, it is more of a blueprint.
@snippet Samples/Tutorials/BasicTutorial2.cpp plane

We create a plane by supplying a vector that is normal to our plane and its distance from the origin. So we have created a plane that is perpendicular to the y-axis and zero units from the origin. Here's a picture:

![](bt2_plane_normal.png)

There are other overloads of the Plane constructor that let us pass a second vector instead of a distance from the origin. This allows us to build any plane in 3D space we want.

Now we'll ask the MeshManager to create us a mesh using our Plane blueprint. The MeshManager is already keeping track of the resources we loaded when initializing our application. On top of this, it can create new meshes for us.

@snippet Samples/Tutorials/BasicTutorial2.cpp planedefine
This is a complicated method, and we're not entirely equipped to understand all of it yet. You can read through the Ogre::MeshManager class specification if you want to learn more now. Basically, we've created a new mesh called "ground" with a size of 1500x1500.

Now we will create a new Entity using this mesh.
@snippet Samples/Tutorials/BasicTutorial2.cpp planecreate

Be careful that you don't confuse the parameter given to createEntity for the Entity's name. It is actually the name of the mesh we just created. We're used to seeing mesh names end with '.mesh'.

We want to tell our SceneManager not to cast shadows from our ground Entity. It would just be a waste. Don't get confused, this means the ground won't cast a shadow, it doesn't mean we can't cast shadows on to the *ground*.
@snippet Samples/Tutorials/BasicTutorial2.cpp planenoshadow

And finally we need to give our ground a material. For now, it will be easiest to use a material from the script that Ogre includes with its samples. You should have these resources in your SDK or the source directory you downloaded to build Ogre.
@snippet Samples/Tutorials/BasicTutorial2.cpp planesetmat

Make sure you add the texture for the material and the Examples.material script to your resource loading path. In our case, the texture is called 'rockwall.tga'. You can find the name yourself by reading the entry in the material script.

# Using Shadows in Ogre {#bt2UsingShadowsinOgre}
Enabling shadows in Ogre is easy. The SceneManager class has a Ogre::SceneManager::setShadowTechnique method we can use. Then whenever we create an Entity, we call Ogre::Entity::setCastShadows to choose which ones will cast shadows. setShadowTechnique method takes several of different techniques. Refer to Ogre::ShadowTechnique for more details.

Let's turn off the ambient light so we can see the full effect of our lights. Add the following changes:

@snippet Samples/Tutorials/BasicTutorial2.cpp lightingsset

Now the SceneManager will use shadows. Let's add some lights to see this in action.

# Lights {#bt2Lights}
%Ogre provides different types of lights as listed in Ogre::Light::LightTypes.

The Ogre::Light class has a wide range of properties. Two of the most important are the [diffuse](https://learn.microsoft.com/en-us/windows/win32/direct3d9/diffuse-lighting#example) and [specular](https://learn.microsoft.com/en-us/windows/win32/direct3d9/specular-lighting#example) color. Each material script defines how much specular and diffuse lighting a material reflects. These properties will be covered in some of the later tutorials.

## Creating a Light {#CreatingaLight}
Let's add a Light to our scene. We do this by calling the Ogre::SceneManager::createLight method. Add the following code right after we finish creating the groundEntity:
@snippet Samples/Tutorials/BasicTutorial2.cpp spotlight

We'll set the diffuse and specular colors to pure blue.
@snippet Samples/Tutorials/BasicTutorial2.cpp spotlightcolor

Next we will set the type of the light to spotlight.

@snippet Samples/Tutorials/BasicTutorial2.cpp spotlighttype

The spotlight requires both a position and a direction - remember it acts like a flashlight. We'll place the spotlight above the right shoulder of the ninja shining down on him at a 45 degree angle.

@snippet Samples/Tutorials/BasicTutorial2.cpp spotlightposrot

![](bt2_light_dir_1.png)

Finally, we set what is called the spotlight range. These are the angles that determine where the light fades from bright in the middle to dimmer on the outside edges.

@snippet Samples/Tutorials/BasicTutorial2.cpp spotlightrange
Compile and run the application. You should see the shadowy blue figure of a ninja.

![](bt2_ninja1.jpg)

## Creating More Lights {#CreatingMoreLights}
Next we'll add a directional light to our scene. This type of light essentially simulates daylight or moonlight. The light is cast at the same angle across the entire scene equally. As before, we'll start by creating the Light and setting its type.

@snippet Samples/Tutorials/BasicTutorial2.cpp directlight

Now we'll set the diffuse and specular colors to a dark red.
@snippet Samples/Tutorials/BasicTutorial2.cpp directlightcolor

Finally, we need to set the Light's direction. A directional light does not have a position because it is modeled as a point light that is infinitely far away.

@snippet Samples/Tutorials/BasicTutorial2.cpp directlightdir

![](bt2_light_dir_2.png)

The Light class also defines a Ogre::Light::setAttenuation function which allows you to control how the light dissipates as you get farther away from it. After you finish this tutorial, try using this method in your scene to see how it affects your lights.

Compile and run the application. Your ninja should now have a shadow cast behind him, and the scene should be filled with red light.

![](bt2_ninja2.jpg)

To complete the set, we will now add a point light to our scene.
@snippet Samples/Tutorials/BasicTutorial2.cpp pointlight

We'll set the the specular and diffuse colors to a dark gray.

@snippet Samples/Tutorials/BasicTutorial2.cpp pointlightcolor

A point light has no direction. It only has a position. We will place our last light above and behind the ninja.

@snippet Samples/Tutorials/BasicTutorial2.cpp pointlightpos

Compile and run the application. You should see a long shadow cast in front of the ninja now. And you should see the effects of the point light brightening up the area behind the ninja. Try to think about why the colors turn out the way they do. For instance, why does the shadow behind the ninja appear to have no red at all?

![](bt2_ninja3.jpg)

# Shadow Types {#ShadowTypes}
Ogre supports set of different shadow types. Please refer to Ogre::ShadowTechnique enumerator for more details.

Try experimenting with the different shadow types. There are also other shadow-related methods in the Ogre::SceneManager class that you can play with.

Ogre does not provide soft shadows as part of the engine. You can write your own vertex and fragment programs to implement soft shadows and many other things. The Ogre Manual has a full description of @ref Shadows.

# Conclusion {#Conclusion2}

This tutorial introduced the use of lights and shadows into the scene. To begin, we covered how to use the MeshManager to generate meshes from scratch. We then chose which shadow type Ogre should use. Finally, we begin adding an example of each type of Light to our scene. We created a spotlight, a directional light, and a point light. You can even extend Ogre's lighting and shadow systems by writing your own vertex and fragment programs. Refer to the Ogre Manual for more details.

There are a lot of different settings we've covered that allow you to customize how Ogre renders light and shadow. After you've finished each tutorial, it is a good idea to play around with the new tools you have. This will greatly increase your comfort level working with the library, and it is an excellent way to learn how to navigate API documentation.



