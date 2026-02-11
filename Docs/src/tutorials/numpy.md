# Working with Python {#working-with-numpy}

%Ogre features a %Python component that automatically generates bindings from the C++ headers. This allows you to use Ogre in your %Python code with an API that largely mirrors the C++ API, meaning you can often refer to the existing C++ documentation and tutorials.

Beyond this direct mapping, the bindings also support standard %Python protocols. This enables a more "pythonic" workflow, especially when integrating with other libraries like NumPy.

Additionally, %Ogre provides a high-level API called HighPy, designed for convenience and rapid development by abstracting away many of the engine's lower-level details.

@tableofcontents

# HighPy: A High-Level API {#python-highpy}

The HighPy module transforms %Ogre into a lightweight %Python renderer. As its name suggests, HighPy provides a high-level API that lets you get started quickly, even without extensive prior knowledge of the %Ogre internals.

For instance, rendering a mesh is as simple as:
```py
import Ogre.HighPy as ohi

# Add the path to your mesh files
ohi.user_resource_locations.add("your/meshes/path")

# Create a window and display the mesh
ohi.window_create("ohi_world", (800, 600))
ohi.mesh_show("ohi_world", "DamagedHelmet.glb", position=(0, 0, -3))

# Start the rendering loop
while ohi.window_draw("ohi_world") != 27: pass
```

Want to display a background image? It's just one line:
```py
ohi.imshow("ohi_world", "image.png")
```

Integrating an ImGui-based user interface is also straightforward. Simply define a callback and enable ImGui for your window:

```py
import Ogre.ImGui as ImGui

def ui_callback():
    ImGui.ShowDemoWindow()

ohi.window_use_imgui("ohi_world", ui_callback)
```

For all details, please refer to the Python.HighPy module documentation.

# NumPy Interoperability {#python-numpy}

The %Python bindings support the standard python protocols.
In this tutorial, we will look how we can advantage of this to achieve numpy Interoperability in a pythonic way.

@note this tutorial can be [run live in Google Colab here](https://colab.research.google.com/github/OGRECave/ogre/blob/master/Samples/Python/numpy_sample.ipynb).

We start with a simple 3 channel python array representing a green gradient:

@snippet Samples/Python/numpy_sample.py numpy_image

![](numpy_arr.png)

To be able to load it into %Ogre we now have to convert it to `Ogre.Image`.
The underlying C++ API takes a raw `uchar*`. However, the python bindings accept any object
implementing the Buffer Protocol. This means we can pass the numpy array as is.

@snippet Samples/Python/numpy_sample.py np_to_ogre

Note that `Ogre.Image` is merely a view on the underlying array and no data is copied.
While this is efficient, it also means that you have to ensure that the array does not get out of scope manually.
Otherwise the application will crash due to accessing an invalid pointer.

For completeness we also create a small scene where we map the texture on a screen-centred rectangle.
@snippet Samples/Python/numpy_sample.py apply_to_rect

As the rectangle does not cover the full scene, we also set a background colour
@snippet Samples/Python/numpy_sample.py py_to_primitive
Here, the standard python sequence protocol is used. Therefore, the data is copied.

Finally, we want read-back rendered image into an array. To avoid superficial copies of the data, we again allocate the memory with numpy:

@snippet Samples/Python/numpy_sample.py ogre_to_np

Now we can store the image to disk using pyplot.

@snippet Samples/Python/numpy_sample.py zero_copy_view

@note There is also Ogre::RenderTarget::writeContentsToFile if you do not need the pixel data in Python.

![](numpy_final.png)

# Running Background Threads {#python-background-threads}

By default, the Python bindings do not release the GIL (Global Interpreter Lock) during rendering.
This design choice prioritizes performance by avoiding the overhead of constantly re-acquiring the GIL. However, it means that other Python threads will be blocked and unable to run concurrently with the rendering loop.

For example, the following code will not work as you might expect:

```py
import threading

def printer():
    for _ in range(15):
        print("Thread")

threading.Thread(target=printer).start()
root.startRendering()
```

The "printer" Thread will be blocked until the rendering is finished.

To enable background threads to run, you can call `root.allowPyThread()`. This will instruct Ogre to release the GIL while waiting for vsync, allowing other Python threads to run during this idle time.