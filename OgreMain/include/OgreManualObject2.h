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

#ifndef __OgreManualObject2_H__
#define __OgreManualObject2_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"
#include "OgreRenderOperation.h"
#include "OgreHeaderPrefix.h"
#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

namespace Ogre
{
    class _OgreExport ManualObject : public MovableObject
    {
    public:
        class ManualObjectSection;

        ManualObject( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager );
        virtual ~ManualObject();

        /** Completely clear the contents of the object.
        @remarks
            Clearing the contents of this object and rebuilding from scratch
            is not the optimal way to manage dynamic vertex data, since the 
            buffers are recreated. If you want to keep the same structure but
            update the content within that structure, use beginUpdate() instead 
            of clear() begin(). However if you do want to modify the structure 
            from time to time you can do so by clearing and re-specifying the data.
        */
        virtual_l1 void clear(void);
        
        /** Estimate the number of vertices ahead of time.
        @remarks
            Calling this helps to avoid memory reallocation when you first define
            indices with begin() -> end() sequence.
        */
        virtual_l1 void estimateVertexCount(size_t vcount);

        /** Estimate the number of indices ahead of time.
        @remarks
            Calling this helps to avoid memory reallocation when you first define
            indices with begin() -> end() sequence.
        */
        virtual_l1 void estimateIndexCount(size_t icount);

        /** Start defining a part of the object.
        @remarks
            Each time you call this method, you start a new section of the
            object with its own material and potentially its own type of
            rendering operation (triangles, points or lines for example).
        @param datablockName The name of the datablock to render this part of the
            object with.
        @param opType The type of operation to use to render. 
        */
        virtual_l1 void begin(const String& datablockName,
                           OperationType opType = OT_TRIANGLE_LIST);

        /** Start the definition of an update to a part of the object.
        @remarks
            Using this method, you can update an existing section of the object
            efficiently. You do not have the option of changing the operation type
            obviously, since it must match the one that was used before. 
        @note Changing size of the sections is not supported, you are expected to
            supply the same amount of data with the same layout when updating.
            If you want to change the data layout, call clear() and create
            new sections with begin().
        @param sectionIndex The index of the section you want to update. The first
            call to begin() would have created section 0, the second section 1, etc.
        */
        virtual_l1 void beginUpdate(size_t sectionIndex);
        /** Add a vertex position, starting a new vertex at the same time. 
        @remarks A vertex position is slightly special among the other vertex data
            methods like normal() and textureCoord(), since calling it indicates
            the start of a new vertex. All other vertex data methods you call 
            after this are assumed to be adding more information (like normals or
            texture coordinates) to the last vertex started with position().
        */
        virtual_l1 void position(const Vector3& pos);
        /// @copydoc ManualObject::position(const Vector3&)
        virtual_l1 void position(Real x, Real y, Real z);

        /** Add a vertex normal to the current vertex.
        @remarks
            Vertex normals are most often used for dynamic lighting, and 
            their components should be normalised.
        */
        virtual_l1 void normal(const Vector3& norm);
        /// @copydoc ManualObject::normal(const Vector3&)
        virtual_l1 void normal(Real x, Real y, Real z);

        /** Add a vertex tangent to the current vertex.
        @remarks
            Vertex tangents are most often used for dynamic lighting, and 
            their components should be normalised. 
            Also, using tangent() you enable VES_TANGENT vertex semantic, which is not
            supported on old non-SM2 cards.
        */
        virtual_l1 void tangent(const Vector3& tan);
        /// @copydoc ManualObject::tangent(const Vector3&)
        virtual_l1 void tangent(Real x, Real y, Real z);

        /** Add a texture coordinate to the current vertex.
        @remarks
            You can call this method multiple times between position() calls
            to add multiple texture coordinates to a vertex. Each one can have
            between 1 and 3 dimensions, depending on your needs, although 2 is
            most common. There are several versions of this method for the 
            variations in number of dimensions.
        */
        virtual_l1 void textureCoord(Real u);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(Real u, Real v);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(Real u, Real v, Real w);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(Real x, Real y, Real z, Real w);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(const Vector2& uv);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(const Vector3& uvw);
        /// @copydoc ManualObject::textureCoord(Real)
        virtual_l1 void textureCoord(const Vector4& xyzw);

        /** Add a vertex diffuse colour to a vertex.
        */
        virtual_l1 void colour(const ColourValue& col);
        /** Add a vertex diffuse colour to a vertex.
        @param r,g,b,a Colour components expressed as floating point numbers from 0-1
        */
        virtual_l1 void colour(Real r, Real g, Real b, Real a = 1.0f);

        /** Add a vertex specular colour to a vertex.
        */
        virtual_l1 void specular(const ColourValue& col);
        /** Add a vertex specular colour to a vertex.
        @param r,g,b,a Colour components expressed as floating point numbers from 0-1
        */
        virtual_l1 void specular(Real r, Real g, Real b, Real a = 1.0f);

        /** Add a vertex index to construct faces / lines / points.
        @remarks
            You will have to call this 3 times for each face for a triangle list, 
            or use the alternative 3-parameter version. Other operation types
            require different numbers of indexes, @see OperationType.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param idx A vertex index from 0 to 4294967295. 
        */
        virtual_l1 void index(uint32 idx);

        /** Add a set of 2 vertex indices to construct a line; this is a
            shortcut to calling index() 2 times. It is only valid for line
            lists.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param i1, i2 2 vertex indices from 0 to 4294967295 defining a face.
        */
        virtual_l1 void line(uint32 i1, uint32 i2);

        /** Add a set of 3 vertex indices to construct a triangle; this is a
            shortcut to calling index() 3 times. It is only valid for triangle 
            lists.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param i1, i2, i3 3 vertex indices from 0 to 4294967295 defining a face.
        */
        virtual_l1 void triangle(uint32 i1, uint32 i2, uint32 i3);

        /** Add a set of 4 vertex indices to construct a quad (out of 2 
            triangles); this is a shortcut to calling index() 6 times, 
            or triangle() twice. It's only valid for triangle list operations.
        @note
            32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
        @param i1, i2, i3, i4 4 vertex indices from 0 to 4294967295 defining a quad. 
        */
        virtual_l1 void quad(uint32 i1, uint32 i2, uint32 i3, uint32 i4);

        /// Get the number of vertices in the section currently being defined (returns 0 if no section is in progress).
        virtual_l1 size_t getCurrentVertexCount() const;

        /// Get the number of indices in the section currently being defined (returns 0 if no section is in progress).
        virtual_l1 size_t getCurrentIndexCount() const;
        
        /** Finish defining the object and compile the final renderable version. 
        @note
            Will return a pointer to the finished section or NULL if the section was discarded (i.e. has zero vertices/indices).
        */
        virtual_l1 ManualObjectSection* end(void);

        /** Alter the material for a subsection of this object after it has been
            specified.
        @remarks
            You specify the datablock to use on a section of this object during the
            call to begin(), however if you want to change the datablock afterwards
            you can do so by calling this method.
        @note This function is a shortcut of calling Renderable::setDatablock() on all
            sections in this manual object. If you want to set a datablock with pointer
            instead of ID string, you need to get the section and call the appropriate
            function.
        @param subIndex The index of the subsection to alter
        @param name The name of the new datablock to use
        */
        void setDatablock(size_t subIndex, const String& name);

        /** Gets a pointer to a ManualObjectSection, i.e. a part of a ManualObject.
        */
        ManualObjectSection* getSection(unsigned int index) const;

        /** Gets a pointer to a previously named ManualObjectSection (via ManualObjectSection::setName).
        */
        ManualObjectSection* getSection(const Ogre::String & name) const;

        /** Retrieves the number of ManualObjectSection objects making up this ManualObject.
        */
        unsigned int getNumSections(void) const;

        /** Removes the section with given index.
         * @param idx Index of section to remove.
         */
        void removeSection(unsigned int idx);

        /** Get the current count of indices when building a new buffer.
         * @return Index count
         */
        size_t currentIndexCount();

        /** Get the current count of vertices when building a new buffer.
         * @return Vertex count
         */
        size_t currentVertexCount();

        // MovableObject overrides
        /** @copydoc MovableObject::getMovableType. */
        const String& getMovableType(void) const;

        /// Built, renderable section of geometry
        class _OgreExport ManualObjectSection : public Renderable, public MovableAlloc
        {
        protected:
            ManualObject* mParent;
            Aabb mAabb;
            VertexArrayObject * mVao;
            VaoManager * mVaoManager;
            OperationType mOperationType;
            VertexElement2Vec mVertexElements;
            bool m32BitIndices;
            Ogre::String mName;

            void clear();

        public:
            friend class ManualObject;

            ManualObjectSection(ManualObject* parent, const String& datablockName, OperationType opType);
            virtual ~ManualObjectSection();
            
            // Renderable overrides
            /** @copydoc Renderable::getRenderOperation. */
            virtual void getRenderOperation(v1::RenderOperation& op, bool casterPass) OGRE_OVERRIDE;
            /** @copydoc Renderable::getWorldTransforms. */
            virtual void getWorldTransforms(Matrix4* xform) const OGRE_OVERRIDE;
            /** @copydoc Renderable::getLights. */
            virtual const LightList &getLights(void) const OGRE_OVERRIDE;
            /** @copydoc Renderable::getCastsShadows. */
            virtual bool getCastsShadows(void) const OGRE_OVERRIDE;

            /** Sets an user defined name that can serve to identify this section.
             */
            void setName(const Ogre::String & name);

            /** Gets the previously set name.
             * @return Name of this section previously set with ManualObjectSection::setName.
             */
            const Ogre::String & getName();
        };

        typedef vector<ManualObjectSection*>::type SectionList;
        
        
    protected:
        /// List of subsections
        SectionList mSectionList;
        /// Current section
        ManualObjectSection* mCurrentSection;

        /// Are we updating?
        bool mCurrentUpdating;
        String mCurrentDatablockName;

        size_t mVertices;
        size_t mIndices;

        size_t mEstimatedVertices;
        size_t mEstimatedIndices;

        /// System-memory buffer whilst we establish the size required and buffer layout
        float * mTempVertexBuffer;
        size_t mTempVertexBufferSize;

        char * mTempIndexBuffer;
        size_t mTempIndexBufferSize;

        /// Current buffer we write to
        float * mVertexBuffer;
        char * mIndexBuffer;
        float * mVertexBufferCursor;
        char * mIndexBufferCursor;

        /// Current declaration vertex size
        size_t mDeclSize;

        /// Delete temp buffers and reset init counts
        void resetBuffers(void);
        /// Resize the temp vertex buffer?
        void resizeVertexBufferIfNeeded(size_t numVerts);
        /// Resize the temp index buffer?
        void resizeIndexBufferIfNeeded(size_t numInds);
    };


    /** Factory object for creating ManualObject instances */
    class _OgreExport ManualObjectFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        ManualObjectFactory() {}
        ~ManualObjectFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);  

    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif


