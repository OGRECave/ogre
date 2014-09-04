/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _OgreMeshManager2_H__
#define _OgreMeshManager2_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"
#include "OgreVector3.h"
#include "Vao/OgreBufferPacked.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** Handles the management of mesh resources.
        @remarks
            This class deals with the runtime management of
            mesh data; like other resource managers it handles
            the creation of resources (in this case mesh data),
            working within a fixed memory budget.
    */
    class _OgreExport MeshManager: public ResourceManager, public Singleton<MeshManager>
    {
    protected:
        /// @copydoc ResourceManager::createImpl
        Resource* createImpl(const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* createParams);

        VaoManager *mVaoManager;

        //the factor by which the bounding box of an entity is padded
        Real mBoundsPaddingFactor;

        // The listener to pass to serializers
        //MeshSerializerListener *mListener;

    public:
        MeshManager();
        ~MeshManager();

        /** Initialises the manager, only to be called by OGRE internally. */
        void _initialise(void);
        void _setVaoManager( VaoManager *vaoManager );

        /// Get a resource by name
        /// @see ResourceManager::getResourceByName
        MeshPtr getByName(const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);


        /// Create a new mesh
        /// @see ResourceManager::createResource
        MeshPtr create (const String& name, const String& group,
                            bool isManual = false, ManualResourceLoader* loader = 0,
                            const NameValuePairList* createParams = 0);

        /** Create a new mesh, or retrieve an existing one with the same
            name if it already exists.
            @param vertexBufferUsage The usage flags with which the vertex buffer(s)
                will be created
            @param indexBufferUsage The usage flags with which the index buffer(s) created for 
                this mesh will be created with.
            @param vertexBufferShadowed If true, the vertex buffers will be shadowed by system memory 
                copies for faster read access
            @param indexBufferShadowed If true, the index buffers will be shadowed by system memory 
                copies for faster read access
        @see ResourceManager::createOrRetrieve
        */
        ResourceCreateOrRetrieveResult createOrRetrieve(const String& name,
            const String& group,
            bool isManual=false, ManualResourceLoader* loader=0,
            const NameValuePairList* params=0,
            BufferType vertexBufferType = BT_IMMUTABLE,
            BufferType indexBufferType = BT_IMMUTABLE,
            bool vertexBufferShadowed = true, bool indexBufferShadowed = true);

        /** Prepares a mesh for loading from a file.  This does the IO in advance of the call to load().
            @note
                If the model has already been created (prepared or loaded), the existing instance
                will be returned.
            @remarks
                Ogre loads model files from it's own proprietary
                format called .mesh. This is because having a single file
                format is better for runtime performance, and we also have
                control over pre-processed data (such as
                collision boxes, LOD reductions etc).
            @param filename The name of the .mesh file
            @param groupName The name of the resource group to assign the mesh to 
            @param vertexBufferUsage The usage flags with which the vertex buffer(s)
                will be created
            @param indexBufferUsage The usage flags with which the index buffer(s) created for 
                this mesh will be created with.
            @param vertexBufferShadowed If true, the vertex buffers will be shadowed by system memory 
                copies for faster read access
            @param indexBufferShadowed If true, the index buffers will be shadowed by system memory 
                copies for faster read access
        */
        MeshPtr prepare( const String& filename, const String& groupName,
                         BufferType vertexBufferType = BT_IMMUTABLE,
                         BufferType indexBufferType = BT_IMMUTABLE,
                         bool vertexBufferShadowed = true, bool indexBufferShadowed = true );

        /** Loads a mesh from a file, making it immediately available for use.
            @note
                If the model has already been created (prepared or loaded), the existing instance
                will be returned.
            @remarks
                Ogre loads model files from it's own proprietary
                format called .mesh. This is because having a single file
                format is better for runtime performance, and we also have
                control over pre-processed data (such as
                collision boxes, LOD reductions etc).
            @param filename The name of the .mesh file
            @param groupName The name of the resource group to assign the mesh to 
            @param vertexBufferUsage The usage flags with which the vertex buffer(s)
                will be created
            @param indexBufferUsage The usage flags with which the index buffer(s) created for 
                this mesh will be created with.
            @param vertexBufferShadowed If true, the vertex buffers will be shadowed by system memory 
                copies for faster read access
            @param indexBufferShadowed If true, the index buffers will be shadowed by system memory 
                copies for faster read access
        */
        MeshPtr load( const String& filename, const String& groupName,
                      BufferType vertexBufferType = BT_IMMUTABLE,
                      BufferType indexBufferType = BT_IMMUTABLE,
                      bool vertexBufferShadowed = true, bool indexBufferShadowed = true );


        /** Creates a new Mesh specifically for manual definition rather
            than loading from an object file. 
        @remarks
            Note that once you've defined your mesh, you must call Mesh::_setBounds and
            Mesh::_setBoundingRadius in order to define the bounds of your mesh. In previous
            versions of OGRE you could call Mesh::_updateBounds, but OGRE's support of 
            write-only vertex buffers makes this no longer appropriate.
        @param name The name to give the new mesh
        @param groupName The name of the resource group to assign the mesh to 
        @param loader ManualResourceLoader which will be called to load this mesh
            when the time comes. It is recommended that you populate this field
            in order that the mesh can be rebuilt should the need arise
        */
        MeshPtr createManual( const String& name, const String& groupName, 
            ManualResourceLoader* loader = 0);

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MeshManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MeshManager* getSingletonPtr(void);

        /** Gets the factor by which the bounding box of an entity is padded.
            Default is 0.01
        */
        Real getBoundsPaddingFactor(void);
    
        /** Sets the factor by which the bounding box of an entity is padded
        */
        void setBoundsPaddingFactor(Real paddingFactor);

        /** Sets the listener used to control mesh loading through the serializer.
        */
        //void setListener(MeshSerializerListener *listener);
        
        /** Gets the listener used to control mesh loading through the serializer.
        */
        //MeshSerializerListener *getListener();
    };

    /** @} */
    /** @} */

} //namespace

#include "OgreHeaderSuffix.h"

#endif
