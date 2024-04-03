# Hardware Buffers {#Hardware-Buffers}

The general premise with a hardware buffer is that it is an area of memory with which you can do whatever you like; there is no format (vertex or otherwise) associated with the buffer itself - that is entirely up to interpretation by the methods that use it - in that way, a Ogre::HardwareBuffer is just like an area of memory you might allocate using @c malloc - the difference being that this memory is accessible by the GPU driver. Vertex buffers, index buffers and pixel buffers inherit most of their features from the HardwareBuffer class.

@tableofcontents

<a name="The-Hardware-Buffer-Manager-1"></a>

# The Hardware Buffer Manager {#The-Hardware-Buffer-Manager}

The HardwareBufferManager class is the factory hub of all the objects in the new geometry system. You create and destroy the majority of the objects you use to define geometry through this class. It’s a Singleton, so you access it by doing HardwareBufferManager::getSingleton() - however be aware that it is only guaranteed to exist after the RenderSystem has been initialised (after you call Root::initialise); this is because the objects created are invariably API-specific, although you will deal with them through one common interface.  For example:

```cpp
Ogre::VertexDeclaration* decl = HardwareBufferManager::getSingleton().createVertexDeclaration();

auto vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
    3 * sizeof(float),  // size of one whole vertex
    numVertices,        // number of vertices
    Ogre::HBU_GPU_ONLY, // usage
    false);             // no shadow buffer
```

Don’t worry about the details of the above, we’ll cover that in the later sections. The important thing to remember is to always create objects through the HardwareBufferManager, don’t use ’new’ (it won’t work anyway in most cases).

# Buffer Usage {#Buffer-Usage}

Because the memory in a hardware buffer is likely to be under significant contention during the rendering of a scene, the kind of access you need to the buffer over the time it is used is extremely important; whether you need to update the contents of the buffer regularly, whether you need to be able to read information back from it, these are all important factors to how the graphics card manages the buffer. The method and exact parameters used to create a buffer depends on whether you are creating an [index buffer](@ref Hardware-Index-Buffers) or [vertex buffer](@ref Hardware-Vertex-Buffers), however one creation parameter is common to them both - the @c usage.  The most optimal type of hardware buffer is one which is not updated often, and is never read from. The usage parameter of @c createVertexBuffer or @c createIndexBuffer can be one of Ogre::HardwareBufferUsage.

Choosing the usage of your buffers carefully is important to getting optimal performance out of your geometry. If you have a situation where you need to update a vertex buffer often, consider whether you actually need to update **all** the parts of it, or just some. If it’s the latter, consider using more than one buffer, with only the data you need to modify in the HBU_CPU_TO_GPU buffer.  Always try to use the HBU_GPU_ONLY form. This just means that you cannot read *directly* from the hardware buffer, which is good practice because reading from hardware buffers is very slow. If you really need to read data back, use a shadow buffer, described in the next section.

The following table shows how the descriptive usage names map to the legacy terminology used in older versions of %Ogre as well as rendering APIs like D3D11 and OpenGL.

| Usage          | Legacy name |
|----------------|-------------|
| HBU_GPU_ONLY   | HBU_STATIC_WRITE_ONLY |
| HBU_CPU_TO_GPU | HBU_DYNAMIC_WRITE_ONLY |
| HBU_GPU_TO_CPU | HBU_STATIC  |
| HBU_CPU_ONLY   | HBU_DYNAMIC |

# Shadow Buffers {#Shadow-Buffers}

Reading data from a buffer in the GPU memory is very expensive. However, if you have a cast-iron need to read the contents of the buffer, you should set the @c shadowBuffer parameter of @c createVertexBuffer or @c createIndexBuffer to @c true.
This causes the hardware buffer to be shadowed with a *staging* system-memory copy, which will be synchronised with the GPU buffer at locking (@c HBL_READ_ONLY) or unlocking (@c HBL_WRITE_ONLY) time.
You can read from with no more penalty than reading ordinary memory. The catch is that you now have two copies of the buffer - one in system memory and one on the GPU. Therefore do not use it, unless you need it.

# Data Transfer {#Data-Transfer}
In order to read or update a hardware buffer, you have to notify the card about it as it can have an effect on its rendering queue. %Ogre provides two ways of doing this as described below.

## writeData and readData

If your data is already somewhere in system memory, you can use the simple @c writeData and @c readData methods. These can be thought of as locking the buffer (as described below) and doing a @c memcpy. Some APIs, like OpenGL, implement this more efficiently though.

```cpp
std::vector<float> vec = ...;
Ogre::HardwareBufferPtr pBuffer = ...;
// the last parameter means we discard all previous content on writing
pBuffer->writeData(0, vec.size() * sizeof(float), vec.data(), true);
```

However, this approach has a noticeable overhead, if you update small portions of the buffer at different locations. Also, it obviously requires system memory to be allocated for the data.

## Locking buffers {#Locking-buffers}

Locking performs 2 functions - it tells the card that you want access to the buffer, and it returns a pointer which you can manipulate. Note that if you’ve asked to read the buffer (and remember, you really shouldn’t unless you’ve set the buffer up with a shadow buffer), the contents of the hardware buffer will have been copied into system memory somewhere in order for you to get access to it. For the same reason, when you’re finished with the buffer you must unlock it; if you locked the buffer for writing this will trigger the process of uploading the modified information to the graphics hardware.

@note It is strongly recommended to use Ogre::HardwareBufferLockGuard instead of manually locking and unlocking buffers. This guarantees unlocking even in case of exception.

<a name="Lock-parameters"></a>

### Lock parameters

When you lock a buffer, you call one of the following methods:

```cpp
HardwareBufferPtr pBuffer = ...;
// Lock the entire buffer
pBuffer->lock(lockType);
// Lock only part of the buffer
pBuffer->lock(start, length, lockType);
```

The first call locks the entire buffer, the second locks only the section from @c start (as a byte offset), for @c length bytes. This could be faster than locking the entire buffer since less is transferred, but not if you later update the rest of the buffer too, because doing it in small chunks like this means you cannot use Ogre::HardwareBuffer::HBL_DISCARD.  The @c lockType parameter can have a large effect on the performance of your application, especially if you are not using a shadow buffer. See Ogre::HardwareBuffer::LockOptions.

Once you have locked a buffer, you can use the pointer returned however you wish (just don’t bother trying to read the data that’s there if you’ve used @c HBL_DISCARD, or write the data if you’ve used @c HBL_READ_ONLY). Modifying the contents depends on the type of buffer.
@see @ref Updating-Vertex-Buffers
@see @ref Updating-Index-Buffers

# Practical Buffer Tips {#Practical-Buffer-Tips}

The interplay of usage mode on creation, and locking options when reading / updating is important for performance. Here’s some tips:

1.  Aim for the ’perfect’ buffer by creating with @c HBU_GPU_ONLY, with no shadow buffer, and locking all of it once only with @c HBL_DISCARD to populate it. Never touch it again.
2.  If you need to update a buffer regularly, you will have to compromise. Use @c HBU_CPU_TO_GPU when creating (still no shadow buffer), and use @c HBL_DISCARD to lock the entire buffer, or if you can’t then use @c HBL_NO_OVERWRITE to lock parts of it.
3.  If you really need to read data from the buffer, create it with a shadow buffer. Make sure you use @c HBL_READ_ONLY when locking for reading because it will avoid the upload normally associated with unlocking the buffer. You can also combine this with either of the 2 previous points, obviously try for @c HBU_GPU_ONLY if you can - remember that the usage refers to the hardware buffer so can be safely used with a shadow buffer you read from.
4.  Split your vertex buffers up if you find that your usage patterns for different elements of the vertex are different. No point having one huge updatable buffer with all the vertex data in it, if all you need to update is the texture coordinates. Split that part out into its own buffer and make the rest @c HBU_GPU_ONLY.

## Vulkan specific notes

When it comes to Vulkan, the above tips become rules. %Ogre does not try to emulate the behaviour of older APIs and gets out of the way when you are about to shoot yourself in the foot.
- As the GPU might not yet be done processing the data, you should always use @c HBL_DISCARD or multiple buffers to avoid rendering glitches
- @c HBU_GPU_ONLY buffer updates must be done before rendering of the current frame starts (i.e. @c RenderSystem::_render has been called)

# Hardware Vertex Buffers {#Hardware-Vertex-Buffers}

This section covers specialised hardware buffers which contain vertex data.

## The VertexData class {#The-VertexData-class}

The Ogre::VertexData class @copybrief Ogre::VertexData
@copydetails Ogre::VertexData
The VertexData class has a number of important members:

<dl compact="compact">
<dt>vertexStart</dt> <dd>
@copybrief Ogre::VertexData::vertexStart

</dd> <dt>vertexCount</dt> <dd>
@copybrief Ogre::VertexData::vertexCount

</dd> <dt>vertexDeclaration</dt> <dd>

@copybrief Ogre::VertexData::vertexDeclaration
@copydetails Ogre::VertexData::vertexDeclaration
See @ref Vertex-Declarations

</dd> <dt>vertexBufferBinding</dt> <dd>
@copybrief Ogre::VertexData::vertexBufferBinding
@copydetails Ogre::VertexData::vertexBufferBinding
See @ref Vertex-Buffer-Bindings

</dd> </dl>

## Vertex Declarations {#Vertex-Declarations}

Vertex declarations define the vertex inputs used to render the geometry you want to appear on the screen. Basically this means that for each vertex, you want to feed a certain set of data into the graphics pipeline, which (you hope) will affect how it all looks when the triangles are drawn. Vertex declarations let you pull items of data (which we call vertex elements, represented by the VertexElement class) from any number of buffers, both shared and dedicated to that particular element. It’s your job to ensure that the contents of the buffers make sense when interpreted in the way that your VertexDeclaration indicates that they should. To add an element to a VertexDeclaration, you call Ogre::VertexDeclaration::addElement method. The parameters to this method are:

<dl compact="compact">
<dt>source</dt> <dd>

This tells the declaration which buffer the element is to be pulled from. Note that this is just an index, which may range from 0 to one less than the number of buffers which are being bound as sources of vertex data. See [Vertex Buffer Bindings](#Vertex-Buffer-Bindings) for information on how a real buffer is bound to a source index. Storing the source of the vertex element this way (rather than using a buffer pointer) allows you to rebind the source of a vertex very easily, without changing the declaration of the vertex format itself.

</dd> <dt>offset</dt> <dd>

Tells the declaration how far in bytes the element is offset from the start of each whole vertex in this buffer. This will be 0 if this is the only element being sourced from this buffer, but if other elements are there then it may be higher. A good way of thinking of this is the size of all vertex elements which precede this element in the buffer.

</dd> <dt>type</dt> <dd>

This defines the data type of the vertex input, including its size. This is an important element because as GPUs become more advanced, we can no longer assume that position input will always require 3 floating point numbers, because programmable vertex pipelines allow full control over the inputs and outputs. This part of the element definition covers the basic type and size, e.g. VET\_FLOAT3 is 3 floating point numbers - the meaning of the data is dealt with in the next parameter.

</dd> <dt>semantic</dt> <dd>

This defines the meaning of the element - the GPU will use this to determine what to use this input for, and programmable vertex pipelines will use this to identify which semantic to map the input to. This can identify the element as positional data, normal data, texture coordinate data, etc. See the API reference for full details of all the options.

</dd> <dt>index</dt> <dd>

This parameter is only required when you supply more than one element of the same semantic in one vertex declaration. For example, if you supply more than one set of texture coordinates, you would set first sets index to 0, and the second set to 1.

</dd> </dl>

You can repeat the call to addElement for as many elements as you have in your vertex input structures. There are also useful methods on VertexDeclaration for locating elements within a declaration - see the API reference for full details.

## Vertex Buffer Bindings {#Vertex-Buffer-Bindings}

Vertex buffer bindings are about associating a vertex buffer with a source index used in [Vertex Declarations](#Vertex-Declarations).

## Creating the Vertex Buffer {#Creating-the-Vertex-Buffer}

Firstly, lets look at how you create a vertex buffer:

```cpp
HardwareVertexBufferPtr vbuf =
    Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
    3 * sizeof(float),  // size of one whole vertex
    numVertices,        // number of vertices
    Ogre::HBU_GPU_ONLY, // usage
    false);             // no shadow buffer
```

Notice that we use @ref The-Hardware-Buffer-Manager to create our vertex buffer, and that a class called Ogre::HardwareVertexBufferPtr is returned from the method, rather than a raw pointer. This is because vertex buffers are reference counted - you are able to use a single vertex buffer as a source for multiple pieces of geometry therefore a standard pointer would not be good enough, because you would not know when all the different users of it had finished with it. The @c HardwareVertexBufferPtr class manages its own destruction by keeping a reference count of the number of times it is being used - when the last @c HardwareVertexBufferPtr is destroyed, the buffer itself automatically destroys itself.

The parameters to the creation of a vertex buffer are as follows:

<dl compact="compact">
<dt>vertexSize</dt> <dd>

The size in bytes of a whole vertex in this buffer. A vertex may include multiple elements, and in fact the contents of the vertex data may be reinterpreted by different vertex declarations if you wish. Therefore you must tell the buffer manager how large a whole vertex is, but not the internal format of the vertex, since that is down to the declaration to interpret. In the above example, the size is set to the size of 3 floating point values - this would be enough to hold a standard 3D position or normal, or a 3D texture coordinate, per vertex.

</dd> <dt>numVertices</dt> <dd>

The number of vertices in this buffer. Remember, not all the vertices have to be used at once - it can be beneficial to create large buffers which are shared between many chunks of geometry because changing vertex buffer bindings is a render state switch, and those are best minimised.

</dd> <dt>usage</dt> <dd>

This tells the system how you intend to use the buffer. See @ref Buffer-Usage

</dd> <dt>useShadowBuffer</dt> <dd>

Tells the system whether you want this buffer backed by a system-memory copy. See @ref Shadow-Buffers

</dd> </dl>

## Binding the Vertex Buffer {#Binding-the-Vertex-Buffer}

The second part of the process is to bind this buffer which you have created to a source index. To do this, you call:

```cpp
vertexBufferBinding->setBinding(0, vbuf);
```

This results in the vertex buffer you created earlier being bound to source index 0, so any vertex element which is pulling its data from source index 0 will retrieve data from this buffer.  There are also methods for retrieving buffers from the binding data - see the API reference for full details.

## Updating Vertex Buffers {#Updating-Vertex-Buffers}

The complexity of updating a vertex buffer entirely depends on how its contents are laid out. You can [lock a buffer](@ref Locking-buffers), but how you write data into it vert much depends on what it contains. Let's start with a vert simple example. Let's say you have a buffer which only contains vertex positions, so it only contains sets of 3 floating point numbers per vertex. In this case, all you need to do to write data into it is:

```cpp
auto pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
```

... then you just write positions in chunks of 3 reals. If you have other floating point data in there, it’s a little more complex but the principle is largely the same, you just need to write alternate elements. But what if you have elements of different types, or you need to derive how to write the vertex data from the elements themselves? Well, there are some useful methods on the VertexElement class to help you out. Firstly, you lock the buffer but assign the result to a unsigned char\* rather than a specific type. Then, for each element which is sourcing from this buffer (which you can find out by calling VertexDeclaration::findElementsBySource) you call VertexElement::baseVertexPointerToElement. This offsets a pointer which points at the base of a vertex in a buffer to the beginning of the element in question, and allows you to use a pointer of the right type to boot. Here’s a full example:

```cpp
// will automatically release the lock
Ogre::HardwareBufferLockGuard vbufLock(vbuf, HardwareBuffer::HBL_READ_ONLY);
// Get base pointer
auto pVert = static_cast<unsigned char*>(vbufLock.pData);
float* pFloat;
for (size_t v = 0; v < vertexCount; ++v)
{
    // Get elements
    for (VertexElement& elem : decl->findElementsBySource(bufferIdx))
    {
        if (elem.getSemantic() == VES_POSITION)
        {
            elem.baseVertexPointerToElement(pVert, &pFloat);
            // write position using pFloat

        }
        
        ...
        
        
    }
    pVert += vbuf->getVertexSize();
}
```

See the API docs for full details of all the helper methods on VertexDeclaration and VertexElement to assist you in manipulating vertex buffer data pointers.

# Hardware Index Buffers {#Hardware-Index-Buffers}

Index buffers are used to render geometry by building triangles out of vertices indirectly by reference to their position in the buffer, rather than just building triangles by sequentially reading vertices. Index buffers are simpler than vertex buffers, since they are just a list of indexes at the end of the day, however they can be held on the hardware and shared between multiple pieces of geometry in the same way vertex buffers can, so the rules on creation and locking are the same.

## The IndexData class {#The-IndexData-class}

This class summarises the information required to use a set of indexes to render geometry. Its members are as follows:

<dl compact="compact">
<dt>indexStart</dt> <dd>

The first index used by this piece of geometry; this can be useful for sharing a single index buffer among several geometry pieces.

</dd> <dt>indexCount</dt> <dd>

The number of indexes used by this particular renderable.

</dd> <dt>indexBuffer</dt> <dd>

The index buffer which is used to source the indexes.

</dd> </dl> <a name="Creating-an-Index-Buffer"></a>

## Creating an Index Buffer

Index buffers are created using See [The Hardware Buffer Manager](#The-Hardware-Buffer-Manager) just like vertex buffers, here’s how:

```cpp
HardwareIndexBufferPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
    createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numIndexes, // number of indexes
        Ogre::HBU_GPU_ONLY, // usage
        false); // no shadow buffer 
```

Once again, notice that the return type is a class rather than a pointer; this is reference counted so that the buffer is automatically destroyed when no more references are made to it. The parameters to the index buffer creation are:

<dl compact="compact">
<dt>indexType</dt> <dd>

There are 2 types of index; 16-bit and 32-bit. They both perform the same way, except that the latter can address larger vertex buffers. If your buffer includes more than 65526 vertices, then you will need to use 32-bit indexes. Note that you should only use 32-bit indexes when you need to, since they incur more overhead than 16-bit vertices, and are not supported on some older hardware.

</dd> <dt>numIndexes</dt> <dd>

The number of indexes in the buffer. As with vertex buffers, you should consider whether you can use a shared index buffer which is used by multiple pieces of geometry, since there can be performance advantages to switching index buffers less often.

</dd> <dt>usage</dt> <dd>

This tells the system how you intend to use the buffer. See @ref Buffer-Usage

</dd> <dt>useShadowBuffer</dt> <dd>

Tells the system whether you want this buffer backed by a system-memory copy. See @ref Shadow-Buffers

</dd> </dl>

## Updating Index Buffers {#Updating-Index-Buffers}

Updating index buffers can only be done when you [lock the buffer](@ref Locking-buffers) for writing; Locking returns a void pointer, which must be cast to the appropriate type; with index buffers this is either an @c uint16 (for 16-bit indexes) or an @c uint32 (for 32-bit indexes). For example:

```cpp
uint16* pIdx = static_cast<uint16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
```

You can then write to the buffer using the usual pointer semantics, just remember to unlock the buffer when you’re finished!

# Hardware Pixel Buffers {#Hardware-Pixel-Buffers}

Hardware Pixel Buffers are a special kind of buffer that stores graphical data in graphics card memory, generally for use as textures. Pixel buffers can represent a one dimensional, two dimensional or three dimensional image. A texture can consist of a multiple of these buffers.

In contrary to vertex and index buffers, pixel buffers are not constructed directly. When creating @ref Textures, the necessary pixel buffers to hold the data are constructed automatically.

## Pixel boxes {#Pixel-boxes}

All methods in Ogre that take or return raw image data return a Ogre::PixelBox object.

A PixelBox is a primitive describing a volume (3D), image (2D) or line (1D) of pixels in CPU memory. It describes the location and data format of a region of memory used for image data, but does not do any memory management in itself.

Inside the memory pointed to by the *data* member of a pixel box, pixels are stored as a succession of "depth" slices (in Z), each containing "height" rows (Y) of "width" pixels (X).

Dimensions that are not used must be 1. For example, a one dimensional image will have extents (width,1,1). A two dimensional image has extents (width,height,1).

## Updating Pixel Buffers {#Updating-Pixel-Buffers}

Pixel Buffers can be updated in two different ways; a simple, convenient way and a more difficult (but in some cases faster) method. Both methods make use of Ogre::PixelBox objects to represent image data in memory.

### Blit from memory {#blitFromMemory}

The easy method to get an image into a PixelBuffer is by using Ogre::HardwarePixelBuffer::blitFromMemory. This takes a PixelBox object and does all necessary pixel format conversion and scaling for you. For example, to create a manual texture and load an image into it, all you have to do is

```cpp
// Manually loads an image and puts the contents in a manually created texture
Ogre::Image img;
img.load("elephant.png", "General");
// Create RGB texture with 5 mipmaps
TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(
    "elephant",
    "General",
    Ogre::TEX_TYPE_2D,
    img.getWidth(), img.getHeight(),
    5, Ogre::PF_X8R8G8B8);
// Copy face 0 mipmap 0 of the image to face 0 mipmap 0 of the texture.
tex->getBuffer(0,0)->blitFromMemory(img.getPixelBox(0,0));
```

### Direct memory locking {#Direct-memory-locking}

A more advanced method to transfer image data from and to a PixelBuffer is to use locking. By locking a PixelBuffer you can directly access its contents in whatever the internal format of the buffer inside the GPU is.

```cpp
/// Lock the buffer so we can write to it
buffer->lock(HardwareBuffer::HBL_DISCARD);
const Ogre::PixelBox &pb = buffer->getCurrentLock();

/// Update the contents of pb here
/// Image data starts at pb.data and has format pb.format
/// Here we assume data.format is PF_X8R8G8B8 so we can address pixels as uint32.
uint32 *data = static_cast<uint32*>(pb.data);
size_t height = pb.getHeight();
size_t width = pb.getWidth();
size_t pitch = pb.rowPitch; // Skip between rows of image
for(size_t y=0; y<height; ++y)
{
    for(size_t x=0; x<width; ++x)
    {
        // 0xRRGGBB -> fill the buffer with yellow pixels
        data[pitch*y + x] = 0x00FFFF00;
    }
}

/// Unlock the buffer again (frees it for use by the GPU)
buffer->unlock();
```


# Textures {#Textures}

A texture is an image that can be applied onto the surface of a three dimensional model. In %Ogre, textures are represented by the Ogre::Texture resource class.

## Creating a texture {#Creating-a-texture}

Textures are created through the TextureManager. In most cases they are created from image files directly by the Ogre resource system. If you are reading this, you most probably want to create a texture manually so that you can provide it with image data yourself. This is done through Ogre::TextureManager::createManual:

```cpp
tex = Ogre::TextureManager::getSingleton().createManual(
    "MyManualTexture", // Name of texture
    "General", // Name of resource group in which the texture should be created
    TEX_TYPE_2D, // Texture type
    256, // Width
    256, // Height
    1, // Depth (Must be 1 for two dimensional textures)
    0, // No mipmaps
    PF_A8R8G8B8, // internal Pixel format hint
    HBU_GPU_ONLY | TU_AUTOMIPMAP // usage
);
```

This example creates a texture named *MyManualTexture* in resource group *General*. It is a square *two dimensional* texture, with width 256 and height 256.

The available texture types are specified in Ogre::TextureType. Pixel formats are summarised in @ref Pixel-Formats.
@note The format specified here is only a hint. If the hardware does not support the requested format, you will get the closest supported alternative format as returned by Ogre::TextureManager::getNativeFormat.

In addition to the hardware buffer usages as described in Ogre::HardwareBufferUsage there are some usage flags specific to textures defined in Ogre::TextureUsage.

## Getting a PixelBuffer {#Getting-a-PixelBuffer}

A Texture can consist of multiple @ref Hardware-Pixel-Buffers, one for each combo if mipmap level and face number. To get a PixelBuffer from a Texture object the method Ogre::Texture::getBuffer is used:

@c face should be zero for non-cubemap textures. For @ref Cube-map-textures it identifies which one of the cube faces to use.

@c mipmap is zero for the zeroth mipmap level, one for the first mipmap level, and so on. On textures that have automatic mipmap generation (@c TU_AUTOMIPMAP) only level 0 should be accessed, the rest will be taken care of by the rendering API.

A simple example of using getBuffer is

```cpp
// Get the PixelBuffer for face 0, mipmap 0.
HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
```

## Cube map textures {#Cube-map-textures}

The cube map texture type (Ogre::TEX_TYPE_CUBE_MAP) is a different beast from the others; a cube map texture represents a series of six two dimensional images addressed by 3D texture coordinates.

@par +X (face 0)
Represents the positive x plane (right).

@par -X (face 1)
Represents the negative x plane (left).

@par +Y (face 2)
Represents the positive y plane (top).

@par -Y (face 3)
Represents the negative y plane (bottom).

@par +Z (face 4)
Represents the positive z plane (front).

@par -Z (face 5)
Represents the negative z plane (back).

## Pixel Formats {#Pixel-Formats}

@copydetails Ogre::PixelFormat

A pixel format described the storage format of pixel data. It defines the way pixels are encoded in memory. The following classes of pixel formats (PF\_\*) are defined:

@par Native endian formats (PF\_A8R8G8B8 and other formats with bit counts)
These are native endian (16, 24 and 32 bit) integers in memory. The meaning of the letters is described below.

@par Byte formats (PF\_BYTE\_\*)
These formats have one byte per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_BYTE\_RGBA consists of blocks of four bytes, one for red, one for green, one for blue, one for alpha.

@par Short formats (PF\_SHORT\_\*)
These formats have one unsigned short (16 bit integer) per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_SHORT\_RGBA consists of blocks of four 16 bit integers, one for red, one for green, one for blue, one for alpha.

@par Float16 formats (PF\_FLOAT16\_\*)
These formats have one 16 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_FLOAT16\_RGBA consists of blocks of four 16 bit floats, one for red, one for green, one for blue, one for alpha. The 16 bit floats, also called half float) are very similar to the IEEE single-precision floating-point standard of the 32 bits floats, except that they have only 5 exponent bits and 10 mantissa. Note that there is no standard C++ data type or CPU support to work with these efficiently, but GPUs can calculate with these much more efficiently than with 32 bit floats.

@par Float32 formats (PF\_FLOAT32\_\*)
These formats have one 32 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_FLOAT32\_RGBA consists of blocks of four 32 bit floats, one for red, one for green, one for blue, one for alpha. The C++ data type for these 32 bits floats is just "float".

@par Compressed formats (PF\_DXT\[1-5\])
S3TC compressed texture formats, [a good description can be found at Wikipedia](http://en.wikipedia.org/wiki/S3TC)

For a complete list see Ogre::PixelFormat.

### Colour channels {#Colour-channels}

The meaning of the channels R,G,B,A,L and X is defined as

@par R
Red colour component, usually ranging from 0.0 (no red) to 1.0 (full red).

@par G
Green colour component, usually ranging from 0.0 (no green) to 1.0 (full green).

@par B
Blue colour component, usually ranging from 0.0 (no blue) to 1.0 (full blue).

@par A
Alpha component, usually ranging from 0.0 (entire transparent) to 1.0 (opaque).

@par L
Luminance component, usually ranging from 0.0 (black) to 1.0 (white). The luminance component is duplicated in the R, G, and B channels to achieve a greyscale image.

@par X
This component is completely ignored.

If none of red, green and blue components, or luminance is defined in a format, these default to 0. For the alpha channel this is different; if no alpha is defined, it defaults to 1.