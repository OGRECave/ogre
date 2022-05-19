# Working with NumPy {#working-with-numpy}

Ogre includes a Python component which automatically generates Python bindings from the C++ headers.
However, with Python, you most likely do not only want to just use Ogre, but connect it to other components.
For this, the Component uses standard python protocols, that offer exposing the API in a pythonic way.
In this tutorial, we will look how %Ogre integrates with numpy.

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

Note, that the convention of specifying width and height is swapped between %Ogre and numpy.

Now we can store the image to disk using pyplot.

@snippet Samples/Python/numpy_sample.py zero_copy_view

@note There is also Ogre::RenderTarget::writeContentsToFile if you do not need the pixel data in Python.

![](numpy_final.png)