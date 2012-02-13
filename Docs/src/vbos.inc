@node Hardware Buffers
@chapter Hardware Buffers
Vertex buffers, index buffers and pixel buffers inherit most of their features from the HardwareBuffer class. The general premise with a hardware buffer is that it is an area of memory with which you can do whatever you like; there is no format (vertex or otherwise) associated with the buffer itself - that is entirely up to interpretation by the methods that use it - in that way, a HardwareBuffer is just like an area of memory you might allocate using 'malloc' - the difference being that this memory is likely to be located in GPU or AGP memory.
@node The Hardware Buffer Manager
@section The Hardware Buffer Manager
The HardwareBufferManager class is the factory hub of all the objects in the new geometry system. You create and destroy the majority of the objects you use to define geometry through this class. It's a Singleton, so you access it by doing HardwareBufferManager::getSingleton() - however be aware that it is only guaranteed to exist after the RenderSystem has been initialised (after you call Root::initialise); this is because the objects created are invariably API-specific, although you will deal with them through one common interface. 
@*@*
For example:
@example
VertexDeclaration* decl = HardwareBufferManager::getSingleton().createVertexDeclaration();
@end example
@example
HardwareVertexBufferSharedPtr vbuf = 
	HardwareBufferManager::getSingleton().createVertexBuffer(
		3*sizeof(Real), // size of one whole vertex
		numVertices, // number of vertices
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer
@end example
Don't worry about the details of the above, we'll cover that in the later sections. The important thing to remember is to always create objects through the HardwareBufferManager, don't use 'new' (it won't work anyway in most cases).
@node Buffer Usage
@section Buffer Usage
Because the memory in a hardware buffer is likely to be under significant contention during the rendering of a scene, the kind of access you need to the buffer over the time it is used is extremely important; whether you need to update the contents of the buffer regularly, whether you need to be able to read information back from it, these are all important factors to how the graphics card manages the buffer. The method and exact parameters used to create a buffer depends on whether you are creating an index or vertex buffer (@xref{Hardware Vertex Buffers} and @xref{Hardware Index Buffers}), however one creation parameter is common to them both - the 'usage'. 
@*@*
The most optimal type of hardware buffer is one which is not updated often, and is never read from. The usage parameter of createVertexBuffer or createIndexBuffer can be one of the following:
@table @code
@item HBU_STATIC
This means you do not need to update the buffer very often, but you might occasionally want to read from it.

@item HBU_STATIC_WRITE_ONLY
This means you do not need to update the buffer very often, and you do not need to read from it. However, you may read from it's shadow buffer if you set one up (@xref{Shadow Buffers}). This is the optimal buffer usage setting.

@item HBU_DYNAMIC
This means you expect to update the buffer often, and that you may wish to read from it. This is the least optimal buffer setting.

@item HBU_DYNAMIC_WRITE_ONLY
This means you expect to update the buffer often, but that you never want to read from it. However, you may read from it's shadow buffer if you set one up (@xref{Shadow Buffers}). If you use this option, and replace the entire contents of the buffer every frame, then you should use HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE instead, since that has better performance characteristics on some platforms.

@item HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE
This means that you expect to replace the entire contents of the buffer on an extremely regular basis, most likely every frame. By selecting this option, you free the system up from having to be concerned about losing the existing contents of the buffer at any time, because if it does lose them, you will be replacing them next frame anyway. On some platforms this can make a significant performance difference, so you should try to use this whenever you have a buffer you need to update regularly. Note that if you create a buffer this way, you should use the HBL_DISCARD flag when locking the contents of it for writing.

@end table
Choosing the usage of your buffers carefully is important to getting optimal performance out of your geometry. If you have a situation where you need to update a vertex buffer often, consider whether you actually need to update @strong{all} the parts of it, or just some. If it's the latter, consider using more than one buffer, with only the data you need to modify in the HBU_DYNAMIC buffer.
@*@*
Always try to use the _WRITE_ONLY forms. This just means that you cannot read @emph{directly} from the hardware buffer, which is good practice because reading from hardware buffers is very slow. If you really need to read data back, use a shadow buffer, described in the next section.

@node Shadow Buffers
@section Shadow Buffers
As discussed in the previous section, reading data back from a hardware buffer performs very badly. However, if you have a cast-iron need to read the contents of the vertex buffer, you should set the 'shadowBuffer' parameter of createVertexBuffer or createIndexBuffer to 'true'. This causes the hardware buffer to be backed with a system memory copy, which you can read from with no more penalty than reading ordinary memory. The catch is that when you write data into this buffer, it will first update the system memory copy, then it will update the hardware buffer, as separate copying process - therefore this technique has an additional overhead when writing data. Don't use it unless you really need it.

@node Locking buffers
@section Locking buffers
In order to read or update a hardware buffer, you have to 'lock' it. This performs 2 functions - it tells the card that you want access to the buffer (which can have an effect on its rendering queue), and it returns a pointer which you can manipulate. Note that if you've asked to read the buffer (and remember, you really shouldn't unless you've set the buffer up with a shadow buffer), the contents of the hardware buffer will have been copied into system memory somewhere in order for you to get access to it. For the same reason, when you're finished with the buffer you must unlock it; if you locked the buffer for writing this will trigger the process of uploading the modified information to the graphics hardware.
@*@*
@subheading Lock parameters
When you lock a buffer, you call one of the following methods:
@example
// Lock the entire buffer
pBuffer->lock(lockType);
// Lock only part of the buffer
pBuffer->lock(start, length, lockType);
@end example
The first call locks the entire buffer, the second locks only the section from 'start' (as a byte offset), for 'length' bytes. This could be faster than locking the entire buffer since less is transferred, but not if you later update the rest of the buffer too, because doing it in small chunks like this means you cannot use HBL_DISCARD (see below).
@*@*
The lockType parameter can have a large effect on the performance of your application, especially if you are not using a shadow buffer. 
@table @code
@item HBL_NORMAL
This kind of lock allows reading and writing from the buffer - it's also the least optimal because basically you're telling the card you could be doing anything at all. If you're not using a shadow buffer, it requires the buffer to be transferred from the card and back again. If you're using a shadow buffer the effect is minimal.
@item HBL_READ_ONLY
This means you only want to read the contents of the buffer. Best used when you created the buffer with a shadow buffer because in that case the data does not have to be downloaded from the card.
@item HBL_DISCARD
This means you are happy for the card to discard the @emph{entire current contents} of the buffer. Implicitly this means you are not going to read the data - it also means that the card can avoid any stalls if the buffer is currently being rendered from, because it will actually give you an entirely different one. Use this wherever possible when you are locking a buffer which was not created with a shadow buffer. If you are using a shadow buffer it matters less, although with a shadow buffer it's preferable to lock the entire buffer at once, because that allows the shadow buffer to use HBL_DISCARD when it uploads the updated contents to the real buffer.
@item HBL_NO_OVERWRITE
This is useful if you are locking just part of the buffer and thus cannot use HBL_DISCARD. It tells the card that you promise not to modify any section of the buffer which has already been used in a rendering operation this frame. Again this is only useful on buffers with no shadow buffer.
@end table

Once you have locked a buffer, you can use the pointer returned however you wish (just don't bother trying to read the data that's there if you've used HBL_DISCARD, or write the data if you've used HBL_READ_ONLY). Modifying the contents depends on the type of buffer, @xref{Hardware Vertex Buffers} and @xref{Hardware Index Buffers}

@node Practical Buffer Tips
@section Practical Buffer Tips
The interplay of usage mode on creation, and locking options when reading / updating is important for performance. Here's some tips:
@enumerate
@item
Aim for the 'perfect' buffer by creating with HBU_STATIC_WRITE_ONLY, with no shadow buffer, and locking all of it once only with HBL_DISCARD to populate it. Never touch it again.
@item 
If you need to update a buffer regularly, you will have to compromise. Use HBU_DYNAMIC_WRITE_ONLY when creating (still no shadow buffer), and use HBL_DISCARD to lock the entire buffer, or if you can't then use HBL_NO_OVERWRITE to lock parts of it. 
@item
If you really need to read data from the buffer, create it with a shadow buffer. Make sure you use HBL_READ_ONLY when locking for reading because it will avoid the upload normally associated with unlocking the buffer. You can also combine this with either of the 2 previous points, obviously try for static if you can - remember that the _WRITE_ONLY' part refers to the hardware buffer so can be safely used with a shadow buffer you read from. 
@item
Split your vertex buffers up if you find that your usage patterns for different elements of the vertex are different. No point having one huge updatable buffer with all the vertex data in it, if all you need to update is the texture coordinates. Split that part out into it's own buffer and make the rest HBU_STATIC_WRITE_ONLY.
@end enumerate


@node Hardware Vertex Buffers
@section Hardware Vertex Buffers
This section covers specialised hardware buffers which contain vertex data. For a general discussion of hardware buffers, along with the rules for creating and locking them, see the @ref{Hardware Buffers} section.

@node The VertexData class
@subsection The VertexData class
The VertexData class collects together all the vertex-related information used to render geometry. The new RenderOperation requires a pointer to a VertexData object, and it is also used in Mesh and SubMesh to store the vertex positions, normals, texture coordinates etc. VertexData can either be used alone (in order to render unindexed geometry, where the stream of vertices defines the triangles), or in combination with IndexData where the triangles are defined by indexes which refer to the entries in VertexData.
@*@*
It's worth noting that you don't necessarily have to use VertexData to store your applications geometry; all that is required is that you can build a VertexData structure when it comes to rendering. This is pretty easy since all of VertexData's members are pointers, so you could maintain your vertex buffers and declarations in alternative structures if you like, so long as you can convert them for rendering.@*@*
The VertexData class has a number of important members:
@table @asis
@item vertexStart 
The position in the bound buffers to start reading vertex data from. This allows you to use a single buffer for many different renderables.
@item vertexCount
The number of vertices to process in this particular rendering group
@item vertexDeclaration
A pointer to a VertexDeclaration object which defines the format of the vertex input; note this is created for you by VertexData. @xref{Vertex Declarations} 
@item vertexBufferBinding 
A pointer to a VertexBufferBinding object which defines which vertex buffers are bound to which sources - again, this is created for you by VertexData. @xref{Vertex Buffer Bindings}
@end table

@node Vertex Declarations
@subsection Vertex Declarations
Vertex declarations define the vertex inputs used to render the geometry you want to appear on the screen. Basically this means that for each vertex, you want to feed a certain set of data into the graphics pipeline, which (you hope) will affect how it all looks when the triangles are drawn. Vertex declarations let you pull items of data (which we call vertex elements, represented by the VertexElement class) from any number of buffers, both shared and dedicated to that particular element. It's your job to ensure that the contents of the buffers make sense when interpreted in the way that your VertexDeclaration indicates that they should.@*@*
To add an element to a VertexDeclaration, you call it's addElement method. The parameters to this method are:
@table @asis
@item source
This tells the declaration which buffer the element is to be pulled from. Note that this is just an index, which may range from 0 to one less than the number of buffers which are being bound as sources of vertex data. @xref{Vertex Buffer Bindings} for information on how a real buffer is bound to a source index. Storing the source of the vertex element this way (rather than using a buffer pointer) allows you to rebind the source of a vertex very easily, without changing the declaration of the vertex format itself.
@item offset
Tells the declaration how far in bytes the element is offset from the start of each whole vertex in this buffer. This will be 0 if this is the only element being sourced from this buffer, but if other elements are there then it may be higher. A good way of thinking of this is the size of all vertex elements which precede this element in the buffer.
@item type
This defines the data type of the vertex input, including it's size. This is an important element because as GPUs become more advanced, we can no longer assume that position input will always require 3 floating point numbers, because programmable vertex pipelines allow full control over the inputs and outputs. This part of the element definition covers the basic type and size, e.g. VET_FLOAT3 is 3 floating point numbers - the meaning of the data is dealt with in the next parameter.
@item semantic
This defines the meaning of the element - the GPU will use this to determine what to use this input for, and programmable vertex pipelines will use this to identify which semantic to map the input to. This can identify the element as positional data, normal data, texture coordinate data, etc. See the API reference for full details of all the options.
@item index
This parameter is only required when you supply more than one element of the same semantic in one vertex declaration. For example, if you supply more than one set of texture coordinates, you would set first sets index to 0, and the second set to 1.
@end table

You can repeat the call to addElement for as many elements as you have in your vertex input structures. There are also useful methods on VertexDeclaration for locating elements within a declaration - see the API reference for full details.

@subheading Important Considerations
Whilst in theory you have completely full reign over the format of you vertices, in reality there are some restrictions. Older DirectX hardware imposes a fixed ordering on the elements which are pulled from each buffer; specifically any hardware prior to DirectX 9 may impose the following restrictions:
@itemize @bullet
@item
VertexElements should be added in the following order, and the order of the elements within any shared buffer should be as follows: 
@enumerate 
@item Positions
@item Blending weights
@item Normals
@item Diffuse colours
@item Specular colours
@item Texture coordinates (starting at 0, listed in order, with no gaps)
@end enumerate
@item 
You must not have unused gaps in your buffers which are not referenced by any VertexElement
@item 
You must not cause the buffer & offset settings of 2 VertexElements to overlap
@end itemize
OpenGL and DirectX 9 compatible hardware are not required to follow these strict limitations, so you might find, for example that if you broke these rules your application would run under OpenGL and under DirectX on recent cards, but it is not guaranteed to run on older hardware under DirectX unless you stick to the above rules. For this reason you're advised to abide by them!

@node Vertex Buffer Bindings
@subsection Vertex Buffer Bindings
Vertex buffer bindings are about associating a vertex buffer with a source index used in @ref{Vertex Declarations}. 
@subheading Creating the Vertex Buffer
Firstly, lets look at how you create a vertex buffer:
@example
HardwareVertexBufferSharedPtr vbuf = 
	HardwareBufferManager::getSingleton().createVertexBuffer(
		3*sizeof(Real), // size of one whole vertex
		numVertices, // number of vertices
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer
@end example

Notice that we use @ref{The Hardware Buffer Manager} to create our vertex buffer, and that a class called HardwareVertexBufferSharedPtr is returned from the method, rather than a raw pointer. This is because vertex buffers are reference counted - you are able to use a single vertex buffer as a source for multiple pieces of geometry therefore a standard pointer would not be good enough, because you would not know when all the different users of it had finished with it. The HardwareVertexBufferSharedPtr class manages its own destruction by keeping a reference count of the number of times it is being used - when the last HardwareVertexBufferSharedPtr is destroyed, the buffer itself automatically destroys itself.@*@*

The parameters to the creation of a vertex buffer are as follows:
@table @asis
@item vertexSize
The size in bytes of a whole vertex in this buffer. A vertex may include multiple elements, and in fact the contents of the vertex data may be reinterpreted by different vertex declarations if you wish. Therefore you must tell the buffer manager how large a whole vertex is, but not the internal format of the vertex, since that is down to the declaration to interpret. In the above example, the size is set to the size of 3 floating point values - this would be enough to hold a standard 3D position or normal, or a 3D texture coordinate, per vertex. 
@item numVertices
The number of vertices in this buffer. Remember, not all the vertices have to be used at once - it can be beneficial to create large buffers which are shared between many chunks of geometry because changing vertex buffer bindings is a render state switch, and those are best minimised.
@item usage
This tells the system how you intend to use the buffer. @xref{Buffer Usage}
@item useShadowBuffer
Tells the system whether you want this buffer backed by a system-memory copy. @xref{Shadow Buffers}
@end table

@subheading Binding the Vertex Buffer
The second part of the process is to bind this buffer which you have created to a source index. To do this, you call:
@example
vertexBufferBinding->setBinding(0, vbuf);
@end example
This results in the vertex buffer you created earlier being bound to source index 0, so any vertex element which is pulling its data from source index 0 will retrieve data from this buffer. @*@*
There are also methods for retrieving buffers from the binding data - see the API reference for full details.

@node Updating Vertex Buffers
@subsection Updating Vertex Buffers
The complexity of updating a vertex buffer entirely depends on how its contents are laid out. You can lock a buffer (@xref{Locking buffers}), but how you write data into it vert much depends on what it contains.@*@*
Lets start with a vert simple example. Lets say you have a buffer which only contains vertex positions, so it only contains sets of 3 floating point numbers per vertex. In this case, all you need to do to write data into it is:
@example
Real* pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
@end example
... then you just write positions in chunks of 3 reals. If you have other floating point data in there, it's a little more complex but the principle is largely the same, you just need to write alternate elements. But what if you have elements of different types, or you need to derive how to write the vertex data from the elements themselves? Well, there are some useful methods on the VertexElement class to help you out.@*@*
Firstly, you lock the buffer but assign the result to a unsigned char* rather than a specific type. Then, for each element which is sourcing from this buffer (which you can find out by calling VertexDeclaration::findElementsBySource) you call VertexElement::baseVertexPointerToElement. This offsets a pointer which points at the base of a vertex in a buffer to the beginning of the element in question, and allows you to use a pointer of the right type to boot. Here's a full example:
@example
// Get base pointer
unsigned char* pVert = static_cast<unsigned char*>(vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
Real* pReal;
for (size_t v = 0; v < vertexCount; ++v)
@{
	// Get elements
	VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufferIdx);
	VertexDeclaration::VertexElementList::iterator i, iend;
	for (i = elems.begin(); i != elems.end(); ++i)
	@{
		VertexElement& elem = *i;
		if (elem.getSemantic() == VES_POSITION)
		@{
			elem.baseVertexPointerToElement(pVert, &pReal);
			// write position using pReal

		@}
		
		...
		
		
	@}
	pVert += vbuf->getVertexSize();
@}
vbuf->unlock();
@end example

See the API docs for full details of all the helper methods on VertexDeclaration and VertexElement to assist you in manipulating vertex buffer data pointers.

@node Hardware Index Buffers
@section Hardware Index Buffers
Index buffers are used to render geometry by building triangles out of vertices indirectly by reference to their position in the buffer, rather than just building triangles by sequentially reading vertices. Index buffers are simpler than vertex buffers, since they are just a list of indexes at the end of the day, however they can be held on the hardware and shared between multiple pieces of geometry in the same way vertex buffers can, so the rules on creation and locking are the same. @xref{Hardware Buffers} for information.

@node The IndexData class
@subsection The IndexData class
This class summarises the information required to use a set of indexes to render geometry. It's members are as follows:
@table @asis
@item indexStart
The first index used by this piece of geometry; this can be useful for sharing a single index buffer among several geometry pieces.
@item indexCount
The number of indexes used by this particular renderable.
@item indexBuffer 
The index buffer which is used to source the indexes. 
@end table

@subheading Creating an Index Buffer
Index buffers are created using @xref{The Hardware Buffer Manager} just like vertex buffers, here's how:
@example
HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
	createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, // type of index
		numIndexes, // number of indexes
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer	
@end example
Once again, notice that the return type is a class rather than a pointer; this is reference counted so that the buffer is automatically destroyed when no more references are made to it. The parameters to the index buffer creation are:
@table @asis
@item indexType
There are 2 types of index; 16-bit and 32-bit. They both perform the same way, except that the latter can address larger vertex buffers. If your buffer includes more than 65526 vertices, then you will need to use 32-bit indexes. Note that you should only use 32-bit indexes when you need to, since they incur more overhead than 16-bit vertices, and are not supported on some older hardware.
@item numIndexes
The number of indexes in the buffer. As with vertex buffers, you should consider whether you can use a shared index buffer which is used by multiple pieces of geometry, since there can be performance advantages to switching index buffers less often. 
@item usage
This tells the system how you intend to use the buffer. @xref{Buffer Usage}
@item useShadowBuffer
Tells the system whether you want this buffer backed by a system-memory copy. @xref{Shadow Buffers}
@end table

@node Updating Index Buffers
@subsection Updating Index Buffers
Updating index buffers can only be done when you lock the buffer for writing; @xref{Locking buffers} for details. Locking returns a void pointer, which must be cast to the appropriate type; with index buffers this is either an unsigned short (for 16-bit indexes) or an unsigned long (for 32-bit indexes). For example:
@example
unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
@end example
You can then write to the buffer using the usual pointer semantics, just remember to unlock the buffer when you're finished!

@node Hardware Pixel Buffers
@section Hardware Pixel Buffers

Hardware Pixel Buffers are a special kind of buffer that stores graphical data in graphics card memory, generally
for use as textures. Pixel buffers can represent a one dimensional, two dimensional or three dimensional image. A texture can consist of a multiple of these buffers.

In contrary to vertex and index buffers, pixel buffers are not constructed directly. When creating a texture, the necessary pixel buffers to hold its data are constructed automatically.

@node Textures
@subsection Textures

A texture is an image that can be applied onto the surface of a three dimensional model. In Ogre, textures are represented by the Texture resource class.

@subheading Creating a texture

Textures are created through the TextureManager. In most cases they are created from image files directly by the
Ogre resource system. If you are reading this, you most probably want to create a texture manually so that you can provide it with image data yourself. This is done through TextureManager::createManual:

@example
ptex = TextureManager::getSingleton().createManual(
    "MyManualTexture", // Name of texture
    "General", // Name of resource group in which the texture should be created
    TEX_TYPE_2D, // Texture type
    256, // Width
    256, // Height
    1, // Depth (Must be 1 for two dimensional textures)
    0, // Number of mipmaps
    PF_A8R8G8B8, // Pixel format
    TU_DYNAMIC_WRITE_ONLY // usage
);
@end example

This example creates a texture named @emph{MyManualTexture} in resource group @emph{General}. It is a
square @emph{two dimensional} texture, with width 256 and height 256. It has @emph{no mipmaps}, internal
format @emph{PF_A8R8G8B8} and usage @emph{TU_DYNAMIC_WRITE_ONLY}.

The different texture types will be discussed in @ref{Texture Types}. Pixel formats are summarised in
@ref{Pixel Formats}.

@subheading Texture usages

In addition to the hardware buffer usages as described in @xref{Buffer Usage} there are some usage flags specific to textures:
@table @asis
@item TU_AUTOMIPMAP
Mipmaps for this texture will be automatically generated by the graphics hardware. The exact algorithm used is not
defined, but you can assume it to be a 2x2 box filter.
@item TU_RENDERTARGET
This texture will be a render target, i.e. used as a target for render to texture. Setting this flag will ignore all other texture usages except TU_AUTOMIPMAP.
@item TU_DEFAULT
This is actually a combination of usage flags, and is equivalent to TU_AUTOMIPMAP | TU_STATIC_WRITE_ONLY. The resource
system uses these flags for textures that are loaded from images.

@end table

@subheading Getting a PixelBuffer

A Texture can consist of multiple PixelBuffers, one for each combo if mipmap level and face number. To get a PixelBuffer from a Texture object the method Texture::getBuffer(face, mipmap) is used:

@emph{face} should be zero for non-cubemap textures. For cubemap textures it identifies the face to use, which is one of the cube faces described in @xref{Texture Types}.

@emph{mipmap} is zero for the zeroth mipmap level, one for the first mipmap level, and so on. On textures that have automatic mipmap generation (TU_AUTOMIPMAP) only level 0 should be accessed, the rest will be taken care of by the rendering API.

A simple example of using getBuffer is
@example
// Get the PixelBuffer for face 0, mipmap 0.
HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
@end example

@node Updating Pixel Buffers
@subsection Updating Pixel Buffers

Pixel Buffers can be updated in two different ways; a simple, convenient way and a more difficult (but in some cases faster) method. Both methods make use of PixelBox objects (@xref{Pixel boxes}) to represent image data in memory.

@subheading blitFromMemory

The easy method to get an image into a PixelBuffer is by using HardwarePixelBuffer::blitFromMemory. This takes a PixelBox object and does all necessary pixel format conversion and scaling for you. For example, to create a manual texture and load an image into it, all you have to do is

@example
// Manually loads an image and puts the contents in a manually created texture
Image img;
img.load("elephant.png", "General");
// Create RGB texture with 5 mipmaps
TexturePtr tex = TextureManager::getSingleton().createManual(
    "elephant",
    "General",
    TEX_TYPE_2D,
    img.getWidth(), img.getHeight(),
    5, PF_X8R8G8B8);
// Copy face 0 mipmap 0 of the image to face 0 mipmap 0 of the texture.
tex->getBuffer(0,0)->blitFromMemory(img.getPixelBox(0,0));
@end example

@subheading Direct memory locking

A more advanced method to transfer image data from and to a PixelBuffer is to use locking. By locking a PixelBuffer
you can directly access its contents in whatever the internal format of the buffer inside the GPU is.

@example
/// Lock the buffer so we can write to it
buffer->lock(HardwareBuffer::HBL_DISCARD);
const PixelBox &pb = buffer->getCurrentLock();

/// Update the contents of pb here
/// Image data starts at pb.data and has format pb.format
/// Here we assume data.format is PF_X8R8G8B8 so we can address pixels as uint32.
uint32 *data = static_cast<uint32*>(pb.data);
size_t height = pb.getHeight();
size_t width = pb.getWidth();
size_t pitch = pb.rowPitch; // Skip between rows of image
for(size_t y=0; y<height; ++y)
@{
    for(size_t x=0; x<width; ++x)
    @{
        // 0xRRGGBB -> fill the buffer with yellow pixels
        data[pitch*y + x] = 0x00FFFF00;
    @}
@}

/// Unlock the buffer again (frees it for use by the GPU)
buffer->unlock();
@end example


@node Texture Types
@subsection Texture Types

There are four types of textures supported by current hardware, three of them only differ in the amount of dimensions
they have (one, two or three). The fourth one is special. The different texture types are:

@table @asis

@item TEX_TYPE_1D
One dimensional texture, used in combination with 1D texture coordinates.
@item TEX_TYPE_2D
Two dimensional texture, used in combination with 2D texture coordinates.
@item TEX_TYPE_3D
Three dimensional volume texture, used in combination with 3D texture coordinates.
@item TEX_TYPE_CUBE_MAP
Cube map (six two dimensional textures, one for each cube face), used in combination with 3D texture coordinates.

@end table

@subheading Cube map textures

The cube map texture type (TEX_TYPE_CUBE_MAP) is a different beast from the others; a cube map texture represents a series of six two dimensional images addressed by 3D texture coordinates.
    
@table @asis
@item +X (face 0)
Represents the positive x plane (right).
@item -X (face 1)
Represents the negative x plane (left).
@item +Y (face 2)
Represents the positive y plane (top).
@item -Y (face 3)
Represents the negative y plane (bottom).
@item +Z (face 4)
Represents the positive z plane (front).
@item -Z (face 5)
Represents the negative z plane (back).
@end table

@node Pixel Formats
@subsection Pixel Formats

A pixel format described the storage format of pixel data. It defines the way pixels are encoded in memory. The following classes of pixel formats (PF_*) are defined:

@table @asis
@item Native endian formats (PF_A8R8G8B8 and other formats with bit counts)
These are native endian (16, 24 and 32 bit) integers in memory. This means that an image with format PF_A8R8G8B8 can be seen as an array of 32 bit integers, defined as 0xAARRGGBB in hexadecimal. The meaning of the letters is described below.

@item Byte formats (PF_BYTE_*)
These formats have one byte per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF_BYTE_RGBA consists of blocks of four bytes, one for red, one for green, one for blue, one for alpha.

@item Short formats (PF_SHORT_*)
These formats have one unsigned short (16 bit integer) per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF_SHORT_RGBA consists of blocks of four 16 bit integers, one for red, one for green, one for blue, one for alpha.

@item Float16 formats (PF_FLOAT16_*)
These formats have one 16 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF_FLOAT16_RGBA consists of blocks of four 16 bit floats, one for red, one for green, one for blue, one for alpha. The 16 bit floats, also called half float) are very similar to the IEEE single-precision floating-point standard of the 32 bits floats, except that they have only 5 exponent bits and 10 mantissa. Note that there is no standard C++ data type or CPU support to work with these efficiently, but GPUs can
calculate with these much more efficiently than with 32 bit floats.

@item Float32 formats (PF_FLOAT32_*)
These formats have one 32 bit floating point number per channel, and their channels in memory are organized in the order they are specified in the format name. For example, PF_FLOAT32_RGBA consists of blocks of four 32 bit floats, one for red, one for green, one for blue, one for alpha. The C++ data type for these 32 bits floats is just "float".

@item Compressed formats (PF_DXT[1-5])
S3TC compressed texture formats, a good description can be found at | Wikipedia (http://en.wikipedia.org/wiki/S3TC)

@end table

@subheading Colour channels

The meaning of the channels R,G,B,A,L and X is defined as

@table @asis
@item R
Red colour component, usually ranging from 0.0 (no red) to 1.0 (full red).
@item G
Green colour component, usually ranging from 0.0 (no green) to 1.0 (full green).
@item B
Blue colour component, usually ranging from 0.0 (no blue) to 1.0 (full blue).
@item A
Alpha component, usually ranging from 0.0 (entire transparent) to 1.0 (opaque).
@item L
Luminance component, usually ranging from 0.0 (black) to 1.0 (white). The luminance component is duplicated in the R, G, and B channels to achieve a greyscale image.
@item X
This component is completely ignored.
@end table

If none of red, green and blue components, or luminance is defined in a format, these default to 0. For the alpha channel this is different; if no alpha is defined, it defaults to 1. 

@subheading Complete list of pixel formats

This pixel formats supported by the current version of Ogre are

@table @asis

@item Byte formats
PF_BYTE_RGB, PF_BYTE_BGR, PF_BYTE_BGRA, PF_BYTE_RGBA, PF_BYTE_L, PF_BYTE_LA, PF_BYTE_A

@item Short formats
PF_SHORT_RGBA

@item Float16 formats
PF_FLOAT16_R, PF_FLOAT16_RGB, PF_FLOAT16_RGBA

@item Float32 formats
PF_FLOAT32_R, PF_FLOAT32_RGB, PF_FLOAT32_RGBA

@item 8 bit native endian formats
PF_L8, PF_A8, PF_A4L4, PF_R3G3B2

@item 16 bit native endian formats
PF_L16, PF_R5G6B5, PF_B5G6R5, PF_A4R4G4B4, PF_A1R5G5B5

@item 24 bit native endian formats
PF_R8G8B8, PF_B8G8R8

@item 32 bit native endian formats
PF_A8R8G8B8, PF_A8B8G8R8, PF_B8G8R8A8, PF_R8G8B8A8, PF_X8R8G8B8, PF_X8B8G8R8, PF_A2R10G10B10 PF_A2B10G10R10

@item Compressed formats
PF_DXT1, PF_DXT2, PF_DXT3, PF_DXT4, PF_DXT5

@end table

@node Pixel boxes
@subsection Pixel boxes

All methods in Ogre that take or return raw image data return a PixelBox object.

A PixelBox is a primitive describing a volume (3D), image (2D) or line (1D) of pixels in CPU memory. It describes the location and data format of a region of memory used for image data, but does not do any memory management in itself.

Inside the memory pointed to by the @emph{data} member of a pixel box, pixels are stored as a succession of "depth" slices (in Z), each containing "height" rows (Y) of "width" pixels (X).

Dimensions that are not used must be 1. For example, a one dimensional image will have extents (width,1,1). A two dimensional image has extents (width,height,1).

A PixelBox has the following members:
@table @asis
@item data
The pointer to the first component of the image data in memory.
@item format
The pixel format (@xref{Pixel Formats}) of the image data.
@item rowPitch
The number of elements between the leftmost pixel of one row and the left pixel of the next. This value must always be equal to getWidth() (consecutive) for compressed formats.
@item slicePitch
The number of elements between the top left pixel of one (depth) slice and the top left pixel of the next. Must be a multiple of  rowPitch. This value must always be equal to getWidth()*getHeight() (consecutive)
for compressed formats.
@item left, top, right, bottom, front, back
Extents of the box in three dimensional integer space. Note that the left, top, and front edges are included but the right, bottom and top ones are not. @emph{left} must always be smaller or equal to @emph{right}, @emph{top} must always be smaller or equal to @emph{bottom}, and @emph{front} must always be smaller or equal to @emph{back}.
@end table

It also has some useful methods:
@table @asis
@item getWidth()
Get the width of this box
@item getHeight()
Get the height of this box. This is 1 for one dimensional images.
@item getDepth()
Get the depth of this box. This is 1 for one and two dimensional images.
@item setConsecutive()
Set the rowPitch and slicePitch so that the buffer is laid out consecutive in memory.
@item getRowSkip()
Get the number of elements between one past the rightmost pixel of one row and the leftmost pixel of the next row. This is zero if rows are consecutive.
@item getSliceSkip()
Get the number of elements between one past the right bottom pixel of one slice and the left top pixel of the next slice. This is zero if slices are consecutive.
@item isConsecutive()
Return whether this buffer is laid out consecutive in memory (i.e. the pitches are equal to the dimensions)
@item getConsecutiveSize()
Return the size (in bytes) this image would take if it was laid out consecutive in memory
@item getSubVolume(const Box &def)
Return a subvolume of this PixelBox, as a PixelBox.
@end table

For more information about these methods consult the API documentation.

