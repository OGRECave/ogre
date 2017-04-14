# Hardware Buffers {#Hardware-Buffers}

Vertex buffers, index buffers and pixel buffers inherit most of their features from the HardwareBuffer class. The general premise with a hardware buffer is that it is an area of memory with which you can do whatever you like; there is no format (vertex or otherwise) associated with the buffer itself - that is entirely up to interpretation by the methods that use it - in that way, a HardwareBuffer is just like an area of memory you might allocate using ’malloc’ - the difference being that this memory is likely to be located in GPU or AGP memory.

@tableofcontents

<a name="The-Hardware-Buffer-Manager"></a> <a name="The-Hardware-Buffer-Manager-1"></a>

# The Hardware Buffer Manager

The HardwareBufferManager class is the factory hub of all the objects in the new geometry system. You create and destroy the majority of the objects you use to define geometry through this class. It’s a Singleton, so you access it by doing HardwareBufferManager::getSingleton() - however be aware that it is only guaranteed to exist after the RenderSystem has been initialised (after you call Root::initialise); this is because the objects created are invariably API-specific, although you will deal with them through one common interface.  For example:

```cpp
Ogre::VertexDeclaration* decl = HardwareBufferManager::getSingleton().createVertexDeclaration();
```

```cpp
Ogre::HardwareVertexBufferSharedPtr vbuf = 
    Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        3*sizeof(Real), // size of one whole vertex
        numVertices, // number of vertices
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer
```

Don’t worry about the details of the above, we’ll cover that in the later sections. The important thing to remember is to always create objects through the HardwareBufferManager, don’t use ’new’ (it won’t work anyway in most cases).

# Buffer Usage {#Buffer-Usage}

Because the memory in a hardware buffer is likely to be under significant contention during the rendering of a scene, the kind of access you need to the buffer over the time it is used is extremely important; whether you need to update the contents of the buffer regularly, whether you need to be able to read information back from it, these are all important factors to how the graphics card manages the buffer. The method and exact parameters used to create a buffer depends on whether you are creating an index or vertex buffer (See [Hardware Vertex Buffers](#Hardware-Vertex-Buffers) and See [Hardware Index Buffers](#Hardware-Index-Buffers)), however one creation parameter is common to them both - the ’usage’.  The most optimal type of hardware buffer is one which is not updated often, and is never read from. The usage parameter of createVertexBuffer or createIndexBuffer can be one of Ogre::HardwareBuffer::Usage.

Choosing the usage of your buffers carefully is important to getting optimal performance out of your geometry. If you have a situation where you need to update a vertex buffer often, consider whether you actually need to update **all** the parts of it, or just some. If it’s the latter, consider using more than one buffer, with only the data you need to modify in the HBU\_DYNAMIC buffer.  Always try to use the \_WRITE\_ONLY forms. This just means that you cannot read *directly* from the hardware buffer, which is good practice because reading from hardware buffers is very slow. If you really need to read data back, use a shadow buffer, described in the next section.

## Shadow Buffers {#Shadow-Buffers}

As discussed in the previous section, reading data back from a hardware buffer performs very badly. However, if you have a cast-iron need to read the contents of the vertex buffer, you should set the ’shadowBuffer’ parameter of createVertexBuffer or createIndexBuffer to ’true’. This causes the hardware buffer to be backed with a system memory copy, which you can read from with no more penalty than reading ordinary memory. The catch is that when you write data into this buffer, it will first update the system memory copy, then it will update the hardware buffer, as separate copying process - therefore this technique has an additional overhead when writing data. Don’t use it unless you really need it.

## Locking buffers {#Locking-buffers}

In order to read or update a hardware buffer, you have to ’lock’ it. This performs 2 functions - it tells the card that you want access to the buffer (which can have an effect on its rendering queue), and it returns a pointer which you can manipulate. Note that if you’ve asked to read the buffer (and remember, you really shouldn’t unless you’ve set the buffer up with a shadow buffer), the contents of the hardware buffer will have been copied into system memory somewhere in order for you to get access to it. For the same reason, when you’re finished with the buffer you must unlock it; if you locked the buffer for writing this will trigger the process of uploading the modified information to the graphics hardware. 

<a name="Lock-parameters"></a>

### Lock parameters

When you lock a buffer, you call one of the following methods:

```cpp
// Lock the entire buffer
pBuffer->lock(lockType);
// Lock only part of the buffer
pBuffer->lock(start, length, lockType);
```

The first call locks the entire buffer, the second locks only the section from ’start’ (as a byte offset), for ’length’ bytes. This could be faster than locking the entire buffer since less is transferred, but not if you later update the rest of the buffer too, because doing it in small chunks like this means you cannot use Ogre::HardwareBuffer::HBL_DISCARD.  The lockType parameter can have a large effect on the performance of your application, especially if you are not using a shadow buffer. See Ogre::HardwareBuffer::LockOptions.

Once you have locked a buffer, you can use the pointer returned however you wish (just don’t bother trying to read the data that’s there if you’ve used HBL\_DISCARD, or write the data if you’ve used HBL\_READ\_ONLY). Modifying the contents depends on the type of buffer, See [Hardware Vertex Buffers](#Hardware-Vertex-Buffers) and See [Hardware Index Buffers](#Hardware-Index-Buffers)

## Practical Buffer Tips {#Practical-Buffer-Tips}

The interplay of usage mode on creation, and locking options when reading / updating is important for performance. Here’s some tips:

1.  Aim for the ’perfect’ buffer by creating with HBU\_STATIC\_WRITE\_ONLY, with no shadow buffer, and locking all of it once only with HBL\_DISCARD to populate it. Never touch it again.
2.  If you need to update a buffer regularly, you will have to compromise. Use HBU\_DYNAMIC\_WRITE\_ONLY when creating (still no shadow buffer), and use HBL\_DISCARD to lock the entire buffer, or if you can’t then use HBL\_NO\_OVERWRITE to lock parts of it.
3.  If you really need to read data from the buffer, create it with a shadow buffer. Make sure you use HBL\_READ\_ONLY when locking for reading because it will avoid the upload normally associated with unlocking the buffer. You can also combine this with either of the 2 previous points, obviously try for static if you can - remember that the \_WRITE\_ONLY’ part refers to the hardware buffer so can be safely used with a shadow buffer you read from.
4.  Split your vertex buffers up if you find that your usage patterns for different elements of the vertex are different. No point having one huge updatable buffer with all the vertex data in it, if all you need to update is the texture coordinates. Split that part out into it’s own buffer and make the rest HBU\_STATIC\_WRITE\_ONLY.

# Hardware Vertex Buffers {#Hardware-Vertex-Buffers}

This section covers specialised hardware buffers which contain vertex data. For a general discussion of hardware buffers, along with the rules for creating and locking them, see the [Hardware Buffers](#Hardware-Buffers) section.

## The VertexData class {#The-VertexData-class}

The Ogre::VertexData class collects together all the vertex-related information used to render geometry. The new RenderOperation requires a pointer to a VertexData object, and it is also used in Mesh and SubMesh to store the vertex positions, normals, texture coordinates etc. VertexData can either be used alone (in order to render unindexed geometry, where the stream of vertices defines the triangles), or in combination with IndexData where the triangles are defined by indexes which refer to the entries in VertexData.  It’s worth noting that you don’t necessarily have to use VertexData to store your applications geometry; all that is required is that you can build a VertexData structure when it comes to rendering. This is pretty easy since all of VertexData’s members are pointers, so you could maintain your vertex buffers and declarations in alternative structures if you like, so long as you can convert them for rendering. The VertexData class has a number of important members:

<dl compact="compact">
<dt>vertexStart</dt> <dd>

The position in the bound buffers to start reading vertex data from. This allows you to use a single buffer for many different renderables.

</dd> <dt>vertexCount</dt> <dd>

The number of vertices to process in this particular rendering group

</dd> <dt>vertexDeclaration</dt> <dd>

A pointer to a VertexDeclaration object which defines the format of the vertex input; note this is created for you by VertexData. See [Vertex Declarations](#Vertex-Declarations)

</dd> <dt>vertexBufferBinding</dt> <dd>

A pointer to a VertexBufferBinding object which defines which vertex buffers are bound to which sources - again, this is created for you by VertexData. See [Vertex Buffer Bindings](#Vertex-Buffer-Bindings)

</dd> </dl>

## Vertex Declarations {#Vertex-Declarations}

Vertex declarations define the vertex inputs used to render the geometry you want to appear on the screen. Basically this means that for each vertex, you want to feed a certain set of data into the graphics pipeline, which (you hope) will affect how it all looks when the triangles are drawn. Vertex declarations let you pull items of data (which we call vertex elements, represented by the VertexElement class) from any number of buffers, both shared and dedicated to that particular element. It’s your job to ensure that the contents of the buffers make sense when interpreted in the way that your VertexDeclaration indicates that they should. To add an element to a VertexDeclaration, you call Ogre::VertexDeclaration::addElement method. The parameters to this method are:

<dl compact="compact">
<dt>source</dt> <dd>

This tells the declaration which buffer the element is to be pulled from. Note that this is just an index, which may range from 0 to one less than the number of buffers which are being bound as sources of vertex data. See [Vertex Buffer Bindings](#Vertex-Buffer-Bindings) for information on how a real buffer is bound to a source index. Storing the source of the vertex element this way (rather than using a buffer pointer) allows you to rebind the source of a vertex very easily, without changing the declaration of the vertex format itself.

</dd> <dt>offset</dt> <dd>

Tells the declaration how far in bytes the element is offset from the start of each whole vertex in this buffer. This will be 0 if this is the only element being sourced from this buffer, but if other elements are there then it may be higher. A good way of thinking of this is the size of all vertex elements which precede this element in the buffer.

</dd> <dt>type</dt> <dd>

This defines the data type of the vertex input, including it’s size. This is an important element because as GPUs become more advanced, we can no longer assume that position input will always require 3 floating point numbers, because programmable vertex pipelines allow full control over the inputs and outputs. This part of the element definition covers the basic type and size, e.g. VET\_FLOAT3 is 3 floating point numbers - the meaning of the data is dealt with in the next parameter.

</dd> <dt>semantic</dt> <dd>

This defines the meaning of the element - the GPU will use this to determine what to use this input for, and programmable vertex pipelines will use this to identify which semantic to map the input to. This can identify the element as positional data, normal data, texture coordinate data, etc. See the API reference for full details of all the options.

</dd> <dt>index</dt> <dd>

This parameter is only required when you supply more than one element of the same semantic in one vertex declaration. For example, if you supply more than one set of texture coordinates, you would set first sets index to 0, and the second set to 1.

</dd> </dl>

You can repeat the call to addElement for as many elements as you have in your vertex input structures. There are also useful methods on VertexDeclaration for locating elements within a declaration - see the API reference for full details.

## Important Considerations {#Important-Considerations}

Whilst in theory you have completely full reign over the format of you vertices, in reality there are some restrictions. Older DirectX hardware imposes a fixed ordering on the elements which are pulled from each buffer; specifically any hardware prior to DirectX 9 may impose the following restrictions:

-   VertexElements should be added in the following order, and the order of the elements within any shared buffer should be as follows:
    1.  Positions
    2.  Blending weights
    3.  Normals
    4.  Diffuse colours
    5.  Specular colours
    6.  Texture coordinates (starting at 0, listed in order, with no gaps)
-   You must not have unused gaps in your buffers which are not referenced by any VertexElement
-   You must not cause the buffer & offset settings of 2 VertexElements to overlap

OpenGL and DirectX 9 compatible hardware are not required to follow these strict limitations, so you might find, for example that if you broke these rules your application would run under OpenGL and under DirectX on recent cards, but it is not guaranteed to run on older hardware under DirectX unless you stick to the above rules. For this reason you’re advised to abide by them!

## Vertex Buffer Bindings {#Vertex-Buffer-Bindings}

Vertex buffer bindings are about associating a vertex buffer with a source index used in [Vertex Declarations](#Vertex-Declarations).

## Creating the Vertex Buffer {#Creating-the-Vertex-Buffer}

Firstly, lets look at how you create a vertex buffer:

```cpp
HardwareVertexBufferSharedPtr vbuf = 
    Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        3*sizeof(Real), // size of one whole vertex
        numVertices, // number of vertices
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer
```

Notice that we use [The Hardware Buffer Manager](#The-Hardware-Buffer-Manager) to create our vertex buffer, and that a class called HardwareVertexBufferSharedPtr is returned from the method, rather than a raw pointer. This is because vertex buffers are reference counted - you are able to use a single vertex buffer as a source for multiple pieces of geometry therefore a standard pointer would not be good enough, because you would not know when all the different users of it had finished with it. The HardwareVertexBufferSharedPtr class manages its own destruction by keeping a reference count of the number of times it is being used - when the last HardwareVertexBufferSharedPtr is destroyed, the buffer itself automatically destroys itself.

The parameters to the creation of a vertex buffer are as follows:

<dl compact="compact">
<dt>vertexSize</dt> <dd>

The size in bytes of a whole vertex in this buffer. A vertex may include multiple elements, and in fact the contents of the vertex data may be reinterpreted by different vertex declarations if you wish. Therefore you must tell the buffer manager how large a whole vertex is, but not the internal format of the vertex, since that is down to the declaration to interpret. In the above example, the size is set to the size of 3 floating point values - this would be enough to hold a standard 3D position or normal, or a 3D texture coordinate, per vertex.

</dd> <dt>numVertices</dt> <dd>

The number of vertices in this buffer. Remember, not all the vertices have to be used at once - it can be beneficial to create large buffers which are shared between many chunks of geometry because changing vertex buffer bindings is a render state switch, and those are best minimised.

</dd> <dt>usage</dt> <dd>

This tells the system how you intend to use the buffer. See [Buffer Usage](#Buffer-Usage)

</dd> <dt>useShadowBuffer</dt> <dd>

Tells the system whether you want this buffer backed by a system-memory copy. See [Shadow Buffers](#Shadow-Buffers)

</dd> </dl>

## Binding the Vertex Buffer {#Binding-the-Vertex-Buffer}

The second part of the process is to bind this buffer which you have created to a source index. To do this, you call:

```cpp
vertexBufferBinding->setBinding(0, vbuf);
```

This results in the vertex buffer you created earlier being bound to source index 0, so any vertex element which is pulling its data from source index 0 will retrieve data from this buffer.  There are also methods for retrieving buffers from the binding data - see the API reference for full details.

## Updating Vertex Buffers {#Updating-Vertex-Buffers}

The complexity of updating a vertex buffer entirely depends on how its contents are laid out. You can lock a buffer (See [Locking buffers](#Locking-buffers)), but how you write data into it vert much depends on what it contains. Lets start with a vert simple example. Lets say you have a buffer which only contains vertex positions, so it only contains sets of 3 floating point numbers per vertex. In this case, all you need to do to write data into it is:

```cpp
Real* pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
```

... then you just write positions in chunks of 3 reals. If you have other floating point data in there, it’s a little more complex but the principle is largely the same, you just need to write alternate elements. But what if you have elements of different types, or you need to derive how to write the vertex data from the elements themselves? Well, there are some useful methods on the VertexElement class to help you out. Firstly, you lock the buffer but assign the result to a unsigned char\* rather than a specific type. Then, for each element which is sourcing from this buffer (which you can find out by calling VertexDeclaration::findElementsBySource) you call VertexElement::baseVertexPointerToElement. This offsets a pointer which points at the base of a vertex in a buffer to the beginning of the element in question, and allows you to use a pointer of the right type to boot. Here’s a full example:

```cpp
// Get base pointer
unsigned char* pVert = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
Real* pReal;
for (size_t v = 0; v < vertexCount; ++v)
{
    // Get elements
    VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufferIdx);
    VertexDeclaration::VertexElementList::iterator i, iend;
    for (i = elems.begin(); i != elems.end(); ++i)
    {
        VertexElement& elem = *i;
        if (elem.getSemantic() == VES_POSITION)
        {
            elem.baseVertexPointerToElement(pVert, &pReal);
            // write position using pReal

        }
        
        ...
        
        
    }
    pVert += vbuf->getVertexSize();
}
vbuf->unlock();
```

See the API docs for full details of all the helper methods on VertexDeclaration and VertexElement to assist you in manipulating vertex buffer data pointers.

# Hardware Index Buffers {#Hardware-Index-Buffers}

Index buffers are used to render geometry by building triangles out of vertices indirectly by reference to their position in the buffer, rather than just building triangles by sequentially reading vertices. Index buffers are simpler than vertex buffers, since they are just a list of indexes at the end of the day, however they can be held on the hardware and shared between multiple pieces of geometry in the same way vertex buffers can, so the rules on creation and locking are the same. See [Hardware Buffers](#Hardware-Buffers) for information.

## The IndexData class {#The-IndexData-class}

This class summarises the information required to use a set of indexes to render geometry. It’s members are as follows:

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
HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
    createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, // type of index
        numIndexes, // number of indexes
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
        false); // no shadow buffer 
```

Once again, notice that the return type is a class rather than a pointer; this is reference counted so that the buffer is automatically destroyed when no more references are made to it. The parameters to the index buffer creation are:

<dl compact="compact">
<dt>indexType</dt> <dd>

There are 2 types of index; 16-bit and 32-bit. They both perform the same way, except that the latter can address larger vertex buffers. If your buffer includes more than 65526 vertices, then you will need to use 32-bit indexes. Note that you should only use 32-bit indexes when you need to, since they incur more overhead than 16-bit vertices, and are not supported on some older hardware.

</dd> <dt>numIndexes</dt> <dd>

The number of indexes in the buffer. As with vertex buffers, you should consider whether you can use a shared index buffer which is used by multiple pieces of geometry, since there can be performance advantages to switching index buffers less often.

</dd> <dt>usage</dt> <dd>

This tells the system how you intend to use the buffer. See [Buffer Usage](#Buffer-Usage)

</dd> <dt>useShadowBuffer</dt> <dd>

Tells the system whether you want this buffer backed by a system-memory copy. See [Shadow Buffers](#Shadow-Buffers)

</dd> </dl>

## Updating Index Buffers {#Updating-Index-Buffers}

Updating index buffers can only be done when you lock the buffer for writing; See [Locking buffers](#Locking-buffers) for details. Locking returns a void pointer, which must be cast to the appropriate type; with index buffers this is either an unsigned short (for 16-bit indexes) or an unsigned long (for 32-bit indexes). For example:

```cpp
unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
```

You can then write to the buffer using the usual pointer semantics, just remember to unlock the buffer when you’re finished!

# Hardware Pixel Buffers {#Hardware-Pixel-Buffers}

Hardware Pixel Buffers are a special kind of buffer that stores graphical data in graphics card memory, generally for use as textures. Pixel buffers can represent a one dimensional, two dimensional or three dimensional image. A texture can consist of a multiple of these buffers.

In contrary to vertex and index buffers, pixel buffers are not constructed directly. When creating a texture, the necessary pixel buffers to hold its data are constructed automatically.

## Textures {#Textures}

A texture is an image that can be applied onto the surface of a three dimensional model. In Ogre, textures are represented by the Texture resource class.

### Creating a texture {#Creating-a-texture}

Textures are created through the TextureManager. In most cases they are created from image files directly by the Ogre resource system. If you are reading this, you most probably want to create a texture manually so that you can provide it with image data yourself. This is done through TextureManager::createManual:

```cpp
ptex = Ogre::TextureManager::getSingleton().createManual(
    "MyManualTexture", // Name of texture
    "General", // Name of resource group in which the texture should be created
    Ogre::TEX_TYPE_2D, // Texture type
    256, // Width
    256, // Height
    1, // Depth (Must be 1 for two dimensional textures)
    0, // No mipmaps
    PF_A8R8G8B8, // internal Pixel format
    Ogre::TU_DYNAMIC_WRITE_ONLY // usage
);
```

This example creates a texture named *MyManualTexture* in resource group *General*. It is a square *two dimensional* texture, with width 256 and height 256.

The different texture types will be discussed in Ogre::TextureType. Pixel formats are summarised in [Pixel Formats](#Pixel-Formats).

In addition to the hardware buffer usages as described in Ogre::HardwareBuffer::Usage there are some usage flags specific to textures: Ogre::TextureUsage.

## Getting a PixelBuffer {#Getting-a-PixelBuffer}

A Texture can consist of multiple PixelBuffers, one for each combo if mipmap level and face number. To get a PixelBuffer from a Texture object the method Texture::getBuffer(face, mipmap) is used:

*face* should be zero for non-cubemap textures. For cubemap textures it identifies the face to use, which is one of the cube faces described in See [Texture Types](#Texture-Types).

*mipmap* is zero for the zeroth mipmap level, one for the first mipmap level, and so on. On textures that have automatic mipmap generation (TU\_AUTOMIPMAP) only level 0 should be accessed, the rest will be taken care of by the rendering API.

A simple example of using getBuffer is

```cpp
// Get the PixelBuffer for face 0, mipmap 0.
HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
```

## Updating Pixel Buffers {#Updating-Pixel-Buffers}

Pixel Buffers can be updated in two different ways; a simple, convenient way and a more difficult (but in some cases faster) method. Both methods make use of Ogre::PixelBox objects to represent image data in memory.

## Blit from memory {#blitFromMemory}

The easy method to get an image into a PixelBuffer is by using HardwarePixelBuffer::blitFromMemory. This takes a PixelBox object and does all necessary pixel format conversion and scaling for you. For example, to create a manual texture and load an image into it, all you have to do is

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

## Direct memory locking {#Direct-memory-locking}

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

## Texture Types {#Texture-Types}

There are several types of textures supported by current hardware (see Ogre::TextureType), the first three only differ in the amount of dimensions they have (one, two or three).

## Cube map textures {#Cube-map-textures}

The cube map texture type (Ogre::TEX_TYPE_CUBE_MAP) is a different beast from the others; a cube map texture represents a series of six two dimensional images addressed by 3D texture coordinates.

<dl compact="compact">
<dt>+X (face 0)</dt> <dd>

Represents the positive x plane (right).

</dd> <dt>-X (face 1)</dt> <dd>

Represents the negative x plane (left).

</dd> <dt>+Y (face 2)</dt> <dd>

Represents the positive y plane (top).

</dd> <dt>-Y (face 3)</dt> <dd>

Represents the negative y plane (bottom).

</dd> <dt>+Z (face 4)</dt> <dd>

Represents the positive z plane (front).

</dd> <dt>-Z (face 5)</dt> <dd>

Represents the negative z plane (back).

</dd> </dl>

## Pixel Formats {#Pixel-Formats}

A pixel format described the storage format of pixel data. It defines the way pixels are encoded in memory. The following classes of pixel formats (PF\_\*) are defined:

<dl compact="compact">
<dt>Native endian formats (PF\_A8R8G8B8 and other formats with bit counts)</dt> <dd>

These are native endian (16, 24 and 32 bit) integers in memory. This means that an image with format PF\_A8R8G8B8 can be seen as an array of 32 bit integers, defined as 0xAARRGGBB in hexadecimal. The meaning of the letters is described below.

</dd> <dt>Byte formats (PF\_BYTE\_\*)</dt> <dd>

These formats have one byte per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_BYTE\_RGBA consists of blocks of four bytes, one for red, one for green, one for blue, one for alpha.

</dd> <dt>Short formats (PF\_SHORT\_\*)</dt> <dd>

These formats have one unsigned short (16 bit integer) per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_SHORT\_RGBA consists of blocks of four 16 bit integers, one for red, one for green, one for blue, one for alpha.

</dd> <dt>Float16 formats (PF\_FLOAT16\_\*)</dt> <dd>

These formats have one 16 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_FLOAT16\_RGBA consists of blocks of four 16 bit floats, one for red, one for green, one for blue, one for alpha. The 16 bit floats, also called half float) are very similar to the IEEE single-precision floating-point standard of the 32 bits floats, except that they have only 5 exponent bits and 10 mantissa. Note that there is no standard C++ data type or CPU support to work with these efficiently, but GPUs can calculate with these much more efficiently than with 32 bit floats.

</dd> <dt>Float32 formats (PF\_FLOAT32\_\*)</dt> <dd>

These formats have one 32 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF\_FLOAT32\_RGBA consists of blocks of four 32 bit floats, one for red, one for green, one for blue, one for alpha. The C++ data type for these 32 bits floats is just "float".

</dd> <dt>Compressed formats (PF\_DXT\[1-5\])</dt> <dd>

S3TC compressed texture formats, [a good description can be found at Wikipedia](http://en.wikipedia.org/wiki/S3TC)

</dd> </dl>

## Colour channels {#Colour-channels}

The meaning of the channels R,G,B,A,L and X is defined as

<dl compact="compact">
<dt>R</dt> <dd>

Red colour component, usually ranging from 0.0 (no red) to 1.0 (full red).

</dd> <dt>G</dt> <dd>

Green colour component, usually ranging from 0.0 (no green) to 1.0 (full green).

</dd> <dt>B</dt> <dd>

Blue colour component, usually ranging from 0.0 (no blue) to 1.0 (full blue).

</dd> <dt>A</dt> <dd>

Alpha component, usually ranging from 0.0 (entire transparent) to 1.0 (opaque).

</dd> <dt>L</dt> <dd>

Luminance component, usually ranging from 0.0 (black) to 1.0 (white). The luminance component is duplicated in the R, G, and B channels to achieve a greyscale image.

</dd> <dt>X</dt> <dd>

This component is completely ignored.

</dd> </dl>

If none of red, green and blue components, or luminance is defined in a format, these default to 0. For the alpha channel this is different; if no alpha is defined, it defaults to 1.

## List of pixel formats {#Complete-list-of-pixel-formats}

This pixel formats supported by the current version of Ogre are

<dl compact="compact">
<dt>Byte formats</dt> <dd>

PF\_BYTE\_RGB, PF\_BYTE\_BGR, PF\_BYTE\_BGRA, PF\_BYTE\_RGBA, PF\_BYTE\_L, PF\_BYTE\_LA, PF\_BYTE\_A

</dd> <dt>Short formats</dt> <dd>

PF\_SHORT\_RGBA

</dd> <dt>Float16 formats</dt> <dd>

PF\_FLOAT16\_R, PF\_FLOAT16\_RGB, PF\_FLOAT16\_RGBA

</dd> <dt>Float32 formats</dt> <dd>

PF\_FLOAT32\_R, PF\_FLOAT32\_RGB, PF\_FLOAT32\_RGBA

</dd> <dt>8 bit native endian formats</dt> <dd>

PF\_L8, PF\_A8, PF\_A4L4, PF\_R3G3B2

</dd> <dt>16 bit native endian formats</dt> <dd>

PF\_L16, PF\_R5G6B5, PF\_B5G6R5, PF\_A4R4G4B4, PF\_A1R5G5B5

</dd> <dt>24 bit native endian formats</dt> <dd>

PF\_R8G8B8, PF\_B8G8R8

</dd> <dt>32 bit native endian formats</dt> <dd>

PF\_A8R8G8B8, PF\_A8B8G8R8, PF\_B8G8R8A8, PF\_R8G8B8A8, PF\_X8R8G8B8, PF\_X8B8G8R8, PF\_A2R10G10B10 PF\_A2B10G10R10

</dd> <dt>Compressed formats</dt> <dd>

PF\_DXT1, PF\_DXT2, PF\_DXT3, PF\_DXT4, PF\_DXT5

</dd> </dl>

For a complete list see Ogre::PixelFormat.

## Pixel boxes {#Pixel-boxes}

All methods in Ogre that take or return raw image data return a Ogre::PixelBox object.

A PixelBox is a primitive describing a volume (3D), image (2D) or line (1D) of pixels in CPU memory. It describes the location and data format of a region of memory used for image data, but does not do any memory management in itself.

Inside the memory pointed to by the *data* member of a pixel box, pixels are stored as a succession of "depth" slices (in Z), each containing "height" rows (Y) of "width" pixels (X).

Dimensions that are not used must be 1. For example, a one dimensional image will have extents (width,1,1). A two dimensional image has extents (width,height,1).

For more information about the members consult the API documentation Ogre::PixelBox.