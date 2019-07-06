# Manual mesh creation {#manual-mesh-creation}

There are two ways to create your own mesh. The first way is to create a Ogre::Mesh instance and provide it with the vertex and index buffers directly.

The second way is the high level Ogre::ManualObject interface. Instead of filling position and color buffers, you simply call the "position" and "colour" functions.

# Using Manual Object

@copydetails Ogre::ManualObject

## Example

We will use the ManualObject to create a single textured plane.
After creating the object, we start a new geometry block that will use the given material

@snippet Tests/VisualTests/PlayPen/src/PlayPenTests.cpp manual_plane_begin

Next we specify the vertices of the plane

@snippet Tests/VisualTests/PlayPen/src/PlayPenTests.cpp manual_plane_vertices

Now we can define the face. %Ogre will split the quad into triangles for us.

@snippet Tests/VisualTests/PlayPen/src/PlayPenTests.cpp manual_plane_faces

Calling @c end() creates the actual Hardware Buffers to be used for rendering and we can attach the Object to a Ogre::SceneNode.

@snippet Tests/VisualTests/PlayPen/src/PlayPenTests.cpp manual_plane_end

In case you need multiple Ogre::Entities of the plane, you should call Ogre::ManualObject::convertToMesh first and then use Ogre::SceneManager::createEntity as usual.

# Using vertex and index buffers directly

This time we are going to create a plane using the lower level Ogre::HardwareBuffer primitives.

We start by creating a Mesh object. As this is a manual Mesh, we have to set the bounds of it explicitly.
```cpp
using namespace Ogre;

        MeshPtr mesh = MeshManager::getSingleton().createManual(yourMeshName, RGN_DEFAULT);
        mesh->_setBounds(AxisAlignedBox({-100,-100,0}, {100,100,0});
```

Next we define what should end up in our vertex and index buffer. We will store all data interleaved in one buffer. This typically has some advantages due to cache coherency and also is what ManualObject does automatically for us.

@snippet OgreMain/src/OgrePrefabFactory.cpp manual_plane_geometry

However we could also split the data into multiple buffers with lower precision to save some bytes on texture coordinates and normals.

To describe the vertex sources, we have to create a Ogre::VertexData object. Notably it stores how many vertices we have.

@snippet OgreMain/src/OgrePrefabFactory.cpp vertex_data

The actual description of our vertex buffer however is stored inside the Ogre::VertexDeclaration.

@snippet OgreMain/src/OgrePrefabFactory.cpp vertex_decl

Now we can continue to create the Hardware Buffers and upload our data.

@snippet OgreMain/src/OgrePrefabFactory.cpp vertex_buffer

Note how we used the symbolical constant @c 0 to link the Ogre::HardwareVertexBuffer to the Ogre::VertexDeclaration.
This allows the underlying RenderSystem to swap VertexBuffers without changing the VertexDeclaration. i.e. render different Meshes that share the same vertex layout, without changing the state.

Finally we create the Ogre::SubMesh that will be ultimately rendered.

@snippet OgreMain/src/OgrePrefabFactory.cpp sub_mesh

Note that while our VertexBuffer is shared, the IndexBuffer is not. This allows rendering different faces of the same object using different Materials. Here, each SubMesh links the faces (IndexBuffer) to the according material.

Finally, we have to update the loading state of the mesh as
```cpp
        mesh->load();
```
If you have registered a Ogre::ManualResourceLoader, the resource loading would only happen now.

@note Using the Ogre::ManualResourceLoader is highly recommended. It allows lazy-loading the data on demand as well as unloading  and re-loading resources when running out of memory.