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

#ifndef __BillboardSet_H__
#define __BillboardSet_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreRadixSort.h"
#include "OgreCommon.h"
#include "OgreResourceGroupManager.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** Enum covering what exactly a billboard's position means (center,
        top-left etc).
    @see
        BillboardSet::setBillboardOrigin
    */
    enum BillboardOrigin
    {
        BBO_TOP_LEFT,
        BBO_TOP_CENTER,
        BBO_TOP_RIGHT,
        BBO_CENTER_LEFT,
        BBO_CENTER,
        BBO_CENTER_RIGHT,
        BBO_BOTTOM_LEFT,
        BBO_BOTTOM_CENTER,
        BBO_BOTTOM_RIGHT
    };
    /** The rotation type of billboard. */
    enum BillboardRotationType
    {
        /// Rotate the billboard's vertices around their facing direction
        BBR_VERTEX,
        /// Rotate the billboard's texture coordinates
        BBR_TEXCOORD
    };
    /** The type of billboard to use. */
    enum BillboardType
    {
        /// Standard point billboard (default), always faces the camera completely and is always upright
        BBT_POINT,
        /// Billboards are oriented around a shared direction vector (used as Y axis) and only rotate around this to face the camera
        BBT_ORIENTED_COMMON,
        /// Billboards are oriented around their own direction vector (their own Y axis) and only rotate around this to face the camera
        BBT_ORIENTED_SELF,
        /// Billboards are perpendicular to a shared direction vector (used as Z axis, the facing direction) and X, Y axis are determined by a shared up-vertor
        BBT_PERPENDICULAR_COMMON,
        /// Billboards are perpendicular to their own direction vector (their own Z axis, the facing direction) and X, Y axis are determined by a shared up-vertor
        BBT_PERPENDICULAR_SELF
    };

    /** A collection of billboards (faces which are always facing the given direction) with the same (default) dimensions, material
        and which are fairly close proximity to each other.
    @remarks
        Billboards are rectangles made up of 2 tris which are always facing the given direction. They are typically used
        for special effects like particles. This class collects together a set of billboards with the same (default) dimensions,
        material and relative locality in order to process them more efficiently. The entire set of billboards will be
        culled as a whole (by default, although this can be changed if you want a large set of billboards
        which are spread out and you want them culled individually), individual Billboards have locations which are relative to the set (which itself derives it's
        position from the SceneNode it is attached to since it is a MoveableObject), they will be rendered as a single rendering operation,
        and some calculations will be sped up by the fact that they use the same dimensions so some workings can be reused.
    @par
        A BillboardSet can be created using the SceneManager::createBillboardSet method. They can also be used internally
        by other classes to create effects.
    @note
        Billboard bounds are only automatically calculated when you create them.
        If you modify the position of a billboard you may need to call 
        _updateBounds if the billboard moves outside the original bounds. 
        Similarly, the bounds do no shrink when you remove a billboard, 
        if you want them to call _updateBounds, but note this requires a
        potentially expensive examination of every billboard in the set.
    */
    class _OgreExport BillboardSet : public MovableObject, public Renderable
    {
    protected:
        /** Private constructor (instances cannot be created directly).
        */
        BillboardSet();

        /// Bounds of all billboards in this set
        AxisAlignedBox mAABB;
        /// Bounding radius
        Real mBoundingRadius;

        /// Origin of each billboard
        BillboardOrigin mOriginType;
        /// Rotation type of each billboard
        BillboardRotationType mRotationType;

        /// Default width of each billboard
        Real mDefaultWidth;
        /// Default height of each billboard
        Real mDefaultHeight;

        /// Name of the material to use
        String mMaterialName;
        /// Pointer to the material to use
        MaterialPtr mMaterial;

        /// True if no billboards in this set have been resized - greater efficiency.
        bool mAllDefaultSize;

        /// Flag indicating whether to autoextend pool
        bool mAutoExtendPool;

        /// Flag indicating whether the billboards has to be sorted
        bool mSortingEnabled;

        /// Use 'true' billboard to cam position facing, rather than camera direcion
        bool mAccurateFacing;

        bool mAllDefaultRotation;
        bool mWorldSpace;

        typedef list<Billboard*>::type ActiveBillboardList;
        typedef list<Billboard*>::type FreeBillboardList;
        typedef vector<Billboard*>::type BillboardPool;

        /** Active billboard list.
        @remarks
            This is a linked list of pointers to billboards in the billboard pool.
        @par
            This allows very fast insertions and deletions from anywhere in the list to activate / deactivate billboards
            (required for particle systems etc.) as well as reuse of Billboard instances in the pool
            without construction & destruction which avoids memory thrashing.
        */
        ActiveBillboardList mActiveBillboards;

        /** Free billboard queue.
        @remarks
            This contains a list of the billboards free for use as new instances
            as required by the set. Billboard instances are preconstructed up to the estimated size in the
            mBillboardPool vector and are referenced on this deque at startup. As they get used this deque
            reduces, as they get released back to to the set they get added back to the deque.
        */
        FreeBillboardList mFreeBillboards;

        /** Pool of billboard instances for use and reuse in the active billboard list.
        @remarks
            This vector will be preallocated with the estimated size of the set,and will extend as required.
        */
        BillboardPool mBillboardPool;

        /// The vertex position data for all billboards in this set.
        VertexData* mVertexData;
        /// Shortcut to main buffer (positions, colours, texture coords)
        HardwareVertexBufferSharedPtr mMainBuf;
        /// Locked pointer to buffer
        float* mLockPtr;
        /// Boundary offsets based on origin and camera orientation
        /// Vector3 vLeftOff, vRightOff, vTopOff, vBottomOff;
        /// Final vertex offsets, used where sizes all default to save calcs
        Vector3 mVOffset[4];
        /// Current camera
        Camera* mCurrentCamera;
        /// Parametric offsets of origin
        Real mLeftOff, mRightOff, mTopOff, mBottomOff;
        /// Camera axes in billboard space
        Vector3 mCamX, mCamY;
        /// Camera direction in billboard space
        Vector3 mCamDir;
        /// Camera orientation in billboard space
        Quaternion mCamQ;
        /// Camera position in billboard space
        Vector3 mCamPos;

        /// The vertex index data for all billboards in this set (1 set only)
        IndexData* mIndexData;

        /// Flag indicating whether each billboard should be culled separately (default: false)
        bool mCullIndividual;

        typedef vector< Ogre::FloatRect >::type TextureCoordSets;
        TextureCoordSets mTextureCoords;

        /// The type of billboard to render
        BillboardType mBillboardType;

        /// Common direction for billboards of type BBT_ORIENTED_COMMON and BBT_PERPENDICULAR_COMMON
        Vector3 mCommonDirection;
        /// Common up-vector for billboards of type BBT_PERPENDICULAR_SELF and BBT_PERPENDICULAR_COMMON
        Vector3 mCommonUpVector;

        /// Internal method for culling individual billboards
        inline bool billboardVisible(Camera* cam, const Billboard& bill);

        /// Number of visible billboards (will be == getNumBillboards if mCullIndividual == false)
        unsigned short mNumVisibleBillboards;

        /// Internal method for increasing pool size
        virtual void increasePool(size_t size);


        //-----------------------------------------------------------------------
        // The internal methods which follow are here to allow maximum flexibility as to 
        //  when various components of the calculation are done. Depending on whether the
        //  billboards are of fixed size and whether they are point or oriented type will
        //  determine how much calculation has to be done per-billboard. NOT a one-size fits all approach.
        //-----------------------------------------------------------------------
        /** Internal method for generating billboard corners. 
        @remarks
            Optional parameter pBill is only present for type BBT_ORIENTED_SELF and BBT_PERPENDICULAR_SELF
        */
        void genBillboardAxes(Vector3* pX, Vector3 *pY, const Billboard* pBill = 0);

        /** Internal method, generates parametric offsets based on origin.
        */
        void getParametricOffsets(Real& left, Real& right, Real& top, Real& bottom);

        /** Internal method for generating vertex data. 
        @param offsets Array of 4 Vector3 offsets
        @param pBillboard Reference to billboard
        */
        void genVertices(const Vector3* const offsets, const Billboard& pBillboard);

        /** Internal method generates vertex offsets.
        @remarks
            Takes in parametric offsets as generated from getParametericOffsets, width and height values
            and billboard x and y axes as generated from genBillboardAxes. 
            Fills output array of 4 vectors with vector offsets
            from origin for left-top, right-top, left-bottom, right-bottom corners.
        */
        void genVertOffsets(Real inleft, Real inright, Real intop, Real inbottom,
            Real width, Real height,
            const Vector3& x, const Vector3& y, Vector3* pDestVec);


        /** Sort by direction functor */
        struct SortByDirectionFunctor
        {
            /// Direction to sort in
            Vector3 sortDir;

            SortByDirectionFunctor(const Vector3& dir);
            float operator()(Billboard* bill) const;
        };

        /** Sort by distance functor */
        struct SortByDistanceFunctor
        {
            /// Position to sort in
            Vector3 sortPos;

            SortByDistanceFunctor(const Vector3& pos);
            float operator()(Billboard* bill) const;
        };

        static RadixSort<ActiveBillboardList, Billboard*, float> mRadixSorter;

        /// Use point rendering?
        bool mPointRendering;



    private:
        /// Flag indicating whether the HW buffers have been created.
        bool mBuffersCreated;
        /// The number of billboard in the pool.
        size_t mPoolSize;
        /// Is external billboard data in use?
        bool mExternalData;
        /// Tell if vertex buffer should be update automatically.
        bool mAutoUpdate;
        /// True if the billboard data changed. Will cause vertex buffer update.
        bool mBillboardDataChanged;

        /** Internal method creates vertex and index buffers.
        */
        void _createBuffers(void);
        /** Internal method destroys vertex and index buffers.
        */
        void _destroyBuffers(void);

    public:

        /** Usual constructor - this is called by the SceneManager.
        @param name
            The name to give the billboard set (must be unique)
        @param poolSize
            The initial size of the billboard pool. Estimate of the number of billboards
            which will be required, and pass it using this parameter. The set will
            preallocate this number to avoid memory fragmentation. The default behaviour
            once this pool has run out is to double it.
        @param externalDataSource
            If @c true, the source of data for drawing the 
            billboards will not be the internal billboard list, but external 
            data. When driving the billboard from external data, you must call
            _notifyCurrentCamera to reorient the billboards, setPoolSize to set
            the maximum billboards you want to use, beginBillboards to 
            start the update, and injectBillboard per billboard, 
            followed by endBillboards.
        @see
            BillboardSet::setAutoextend
        */
        BillboardSet( const String& name, unsigned int poolSize = 20, 
            bool externalDataSource = false);

        virtual ~BillboardSet();

        /** Creates a new billboard and adds it to this set.
        @remarks
            Behaviour once the billboard pool has been exhausted depends on the
            BillboardSet::setAutoextend option.
        @param position
            The position of the new billboard realtive to the certer of the set
        @param colour
            Optional base colour of the billboard.
        @return
            On success, a pointer to a newly created Billboard is
            returned.
        @par
            On failure (i.e. no more space and can't autoextend),
            @c NULL is returned.
        @see
            BillboardSet::setAutoextend
        */
        Billboard* createBillboard(
            const Vector3& position,
            const ColourValue& colour = ColourValue::White );

        /** Creates a new billboard and adds it to this set.
        @remarks
            Behaviour once the billboard pool has been exhausted depends on the
            BillboardSet::setAutoextend option.
        @param x
            The @c x position of the new billboard relative to the center of the set
        @param y
            The @c y position of the new billboard relative to the center of the set
        @param z
            The @c z position of the new billboard relative to the center of the set
        @param colour
            Optional base colour of the billboard.
        @return
            On success, a pointer to a newly created Billboard is
            returned.
        @par
            On failure (i.e. no more space and can't autoextend),
            @c NULL is returned.
        @see
            BillboardSet::setAutoextend
        */
        Billboard* createBillboard(
            Real x, Real y, Real z,
            const ColourValue& colour = ColourValue::White );

        /** Returns the number of active billboards which currently make up this set.
        */
        virtual int getNumBillboards(void) const;

        /** Tells the set whether to allow automatic extension of the pool of billboards.
        @remarks
            A BillboardSet stores a pool of pre-constructed billboards which are used as needed when
            a new billboard is requested. This allows applications to create / remove billboards efficiently
            without incurring construction / destruction costs (a must for sets with lots of billboards like
            particle effects). This method allows you to configure the behaviour when a new billboard is requested
            but the billboard pool has been exhausted.
        @par
            The default behaviour is to allow the pool to extend (typically this allocates double the current
            pool of billboards when the pool is expended), equivalent to calling this method with
            autoExtend = true. If you set the parameter to false however, any attempt to create a new billboard
            when the pool has expired will simply fail silently, returning a null pointer.
        @param autoextend
            @c true to double the pool every time it runs out, @c false to fail silently.
        */
        virtual void setAutoextend(bool autoextend);

        /** Returns true if the billboard pool automatically extends.
        @see
            BillboardSet::setAutoextend
        */
        virtual bool getAutoextend(void) const;

        /** Enables sorting for this BillboardSet. (default: off)
        @param sortenable true to sort the billboards according to their distance to the camera
        */
        virtual void setSortingEnabled(bool sortenable);

        /** Returns true if sorting of billboards is enabled based on their distance from the camera
        @see
            BillboardSet::setSortingEnabled
        */
        virtual bool getSortingEnabled(void) const;

        /** Adjusts the size of the pool of billboards available in this set.
        @remarks
            See the BillboardSet::setAutoextend method for full details of the billboard pool. This method adjusts
            the preallocated size of the pool. If you try to reduce the size of the pool, the set has the option
            of ignoring you if too many billboards are already in use. Bear in mind that calling this method will
            incur significant construction / destruction calls so should be avoided in time-critical code. The same
            goes for auto-extension, try to avoid it by estimating the pool size correctly up-front.
        @param size
            The new size for the pool.
        */
        virtual void setPoolSize(size_t size);

        /** Returns the current size of the billboard pool.
        @return
            The current size of the billboard pool.
        @see
            BillboardSet::setAutoextend
        */
        virtual unsigned int getPoolSize(void) const;


        /** Empties this set of all billboards.
        */
        virtual void clear();

        /** Returns a pointer to the billboard at the supplied index.
        @note
            This method requires linear time since the billboard list is a linked list.
        @param index
            The index of the billboard that is requested.
        @return
            On success, a valid pointer to the requested billboard is
            returned.
        @par
            On failure, @c NULL is returned.
        */
        virtual Billboard* getBillboard(unsigned int index) const;

        /** Removes the billboard at the supplied index.
        @note
            This method requires linear time since the billboard list is a linked list.
        */
        virtual void removeBillboard(unsigned int index);

        /** Removes a billboard from the set.
        @note
            This version is more efficient than removing by index.
        */
        virtual void removeBillboard(Billboard* pBill);

        /** Sets the point which acts as the origin point for all billboards in this set.
        @remarks
            This setting controls the fine tuning of where a billboard appears in relation to it's
            position. It could be that a billboard's position represents it's center (e.g. for fireballs),
            it could mean the center of the bottom edge (e.g. a tree which is positioned on the ground),
            the top-left corner (e.g. a cursor).
        @par
            The default setting is BBO_CENTER.
        @param origin
            A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        virtual void setBillboardOrigin(BillboardOrigin origin);

        /** Gets the point which acts as the origin point for all billboards in this set.
        @return
            A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        virtual BillboardOrigin getBillboardOrigin(void) const;

        /** Sets billboard rotation type.
        @remarks
            This setting controls the billboard rotation type, you can deciding rotate the billboard's vertices
            around their facing direction or rotate the billboard's texture coordinates.
        @par
            The default settings is BBR_TEXCOORD.
        @param rotationType
            A member of the BillboardRotationType enum specifying the rotation type for all the billboards in this set.
        */
        virtual void setBillboardRotationType(BillboardRotationType rotationType);

        /** Sets billboard rotation type.
        @return
            A member of the BillboardRotationType enum specifying the rotation type for all the billboards in this set.
        */
        virtual BillboardRotationType getBillboardRotationType(void) const;

        /** Sets the default dimensions of the billboards in this set.
        @remarks
            All billboards in a set are created with these default dimensions. The set will render most efficiently if
            all the billboards in the set are the default size. It is possible to alter the size of individual
            billboards at the expense of extra calculation. See the Billboard class for more info.
        @param width
            The new default width for the billboards in this set.
        @param height
            The new default height for the billboards in this set.
        */
        virtual void setDefaultDimensions(Real width, Real height);

        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultWidth(Real width);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultWidth(void) const;
        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultHeight(Real height);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultHeight(void) const;

        /** Sets the name of the material to be used for this billboard set.
        @param name
            The new name of the material to use for this set.
        */
        virtual void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

        /** Sets the name of the material to be used for this billboard set.
        @return The name of the material that is used for this set.
        */
        virtual const String& getMaterialName(void) const;

        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual void _notifyCurrentCamera(Camera* cam);

        /** Begin injection of billboard data; applicable when 
            constructing the BillboardSet for external data use.
        @param numBillboards If you know the number of billboards you will be 
            issuing, state it here to make the update more efficient.
        */
        void beginBillboards(size_t numBillboards = 0);
        /** Define a billboard. */
        void injectBillboard(const Billboard& bb);
        /** Finish defining billboards. */
        void endBillboards(void);
        /** Set the bounds of the BillboardSet.
        @remarks
            You may need to call this if you're injecting billboards manually, 
            and you're relying on the BillboardSet to determine culling.
        */
        void setBounds(const AxisAlignedBox& box, Real radius);


        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const;

        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual Real getBoundingRadius(void) const;
        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual const MaterialPtr& getMaterial(void) const;

        /** Sets the name of the material to be used for this billboard set.
        @param material
            The new material to use for this set.
         */
        virtual void setMaterial( const MaterialPtr& material );

        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual void getRenderOperation(RenderOperation& op);

        /** Overridden from MovableObject
        @see
            MovableObject
        */
        virtual void getWorldTransforms(Matrix4* xform) const;

        /** Internal callback used by Billboards to notify their parent that they have been resized.
        */
        virtual void _notifyBillboardResized(void);

        /** Internal callback used by Billboards to notify their parent that they have been rotated.
        */
        virtual void _notifyBillboardRotated(void);

        /** Returns whether or not billboards in this are tested individually for culling. */
        virtual bool getCullIndividually(void) const;
        /** Sets whether culling tests billboards in this individually as well as in a group.
        @remarks
            Billboard sets are always culled as a whole group, based on a bounding box which 
            encloses all billboards in the set. For fairly localised sets, this is enough. However, you
            can optionally tell the set to also cull individual billboards in the set, i.e. to test
            each individual billboard before rendering. The default is not to do this.
        @par
            This is useful when you have a large, fairly distributed set of billboards, like maybe 
            trees on a landscape. You probably still want to group them into more than one
            set (maybe one set per section of landscape), which will be culled coarsely, but you also
            want to cull the billboards individually because they are spread out. Whilst you could have
            lots of single-tree sets which are culled separately, this would be inefficient to render
            because each tree would be issued as it's own rendering operation.
        @par
            By calling this method with a parameter of true, you can have large billboard sets which 
            are spaced out and so get the benefit of batch rendering and coarse culling, but also have
            fine-grained culling so unnecessary rendering is avoided.
        @param cullIndividual If true, each billboard is tested before being sent to the pipeline as well 
            as the whole set having to pass the coarse group bounding test.
        */
        virtual void setCullIndividually(bool cullIndividual);

        /** Sets the type of billboard to render.
        @remarks
            The default sort of billboard (BBT_POINT), always has both x and y axes parallel to 
            the camera's local axes. This is fine for 'point' style billboards (e.g. flares,
            smoke, anything which is symmetrical about a central point) but does not look good for
            billboards which have an orientation (e.g. an elongated raindrop). In this case, the
            oriented billboards are more suitable (BBT_ORIENTED_COMMON or BBT_ORIENTED_SELF) since
            they retain an independent Y axis and only the X axis is generated, perpendicular to both
            the local Y and the camera Z.
        @par
            In some case you might want the billboard has fixed Z axis and doesn't need to face to
            camera (e.g. an aureola around the player and parallel to the ground). You can use
            BBT_PERPENDICULAR_SELF which the billboard plane perpendicular to the billboard own
            direction. Or BBT_PERPENDICULAR_COMMON which the billboard plane perpendicular to the
            common direction.
        @note
            BBT_PERPENDICULAR_SELF and BBT_PERPENDICULAR_COMMON can't guarantee counterclockwise, you might
            use double-side material (<b>cull_hardware node</b>) to ensure no billboard are culled.
        @param bbt The type of billboard to render
        */
        virtual void setBillboardType(BillboardType bbt);

        /** Returns the billboard type in use. */
        virtual BillboardType getBillboardType(void) const;

        /** Use this to specify the common direction given to billboards of type BBT_ORIENTED_COMMON or BBT_PERPENDICULAR_COMMON.
        @remarks
            Use BBT_ORIENTED_COMMON when you want oriented billboards but you know they are always going to 
            be oriented the same way (e.g. rain in calm weather). It is faster for the system to calculate
            the billboard vertices if they have a common direction.
        @par
            The common direction also use in BBT_PERPENDICULAR_COMMON, in this case the common direction
            treat as Z axis, and an additional common up-vector was use to determine billboard X and Y
            axis.
            @see setCommonUpVector
        @param vec The direction for all billboards.
        @note
            The direction are use as is, never normalised in internal, user are supposed to normalise it himself.
        */
        virtual void setCommonDirection(const Vector3& vec);

        /** Gets the common direction for all billboards (BBT_ORIENTED_COMMON) */
        virtual const Vector3& getCommonDirection(void) const;

        /** Use this to specify the common up-vector given to billboards of type BBT_PERPENDICULAR_SELF or BBT_PERPENDICULAR_COMMON.
        @remarks
            Use BBT_PERPENDICULAR_SELF or BBT_PERPENDICULAR_COMMON when you want oriented billboards
            perpendicular to specify direction vector (or, Z axis), and doesn't face to camera.
            In this case, we need an additional up-vector to determine the billboard X and Y axis.
            The generated billboard plane and X-axis guarantee perpendicular to specify direction.
            @see setCommonDirection
        @par
            The specify direction is billboard own direction when billboard type is BBT_PERPENDICULAR_SELF,
            and it's shared common direction when billboard type is BBT_PERPENDICULAR_COMMON.
        @param vec The up-vector for all billboards.
        @note
            The up-vector are use as is, never normalised in internal, user are supposed to normalise it himself.
        */
        virtual void setCommonUpVector(const Vector3& vec);

        /** Gets the common up-vector for all billboards (BBT_PERPENDICULAR_SELF and BBT_PERPENDICULAR_COMMON) */
        virtual const Vector3& getCommonUpVector(void) const;
        
        /** Sets whether or not billboards should use an 'accurate' facing model
            based on the vector from each billboard to the camera, rather than 
            an optimised version using just the camera direction.
        @remarks
            By default, the axes for all billboards are calculated using the 
            camera's view direction, not the vector from the camera position to
            the billboard. The former is faster, and most of the time the difference
            is not noticeable. However for some purposes (e.g. very large, static
            billboards) the changing billboard orientation when rotating the camera
            can be off putting, therefore you can enable this option to use a
            more expensive, but more accurate version.
        @param acc True to use the slower but more accurate model. Default is false.
        */
        virtual void setUseAccurateFacing(bool acc) { mAccurateFacing = acc; }
        /** Gets whether or not billboards use an 'accurate' facing model
            based on the vector from each billboard to the camera, rather than 
            an optimised version using just the camera direction.
        */
        virtual bool getUseAccurateFacing(void) const { return mAccurateFacing; }

        /** Overridden from MovableObject */
        virtual const String& getMovableType(void) const;

        /** Overridden, see Renderable */
        Real getSquaredViewDepth(const Camera* cam) const;

        /** Update the bounds of the billboardset */
        virtual void _updateBounds(void);
        /** @copydoc Renderable::getLights */
        const LightList& getLights(void) const;

        /// @copydoc MovableObject::visitRenderables
        void visitRenderables(Renderable::Visitor* visitor, 
            bool debugRenderables = false);

        /** Sort the billboard set. Only called when enabled via setSortingEnabled */
        virtual void _sortBillboards( Camera* cam);

        /** Gets the sort mode of this billboard set */
        virtual SortMode _getSortMode(void) const;

        /** Sets whether billboards should be treated as being in world space. 
        @remarks
            This is most useful when you are driving the billboard set from 
            an external data source.
        */
        virtual void setBillboardsInWorldSpace(bool ws) { mWorldSpace = ws; }

        /** Gets whether billboards are treated as being in world space.
         */
        bool getBillboardsInWorldSpace() { return mWorldSpace; }

        /** BillboardSet can use custom texture coordinates for various billboards.
            This is useful for selecting one of many particle images out of a tiled 
            texture sheet, or doing flipbook animation within a single texture.
        @par
            The generic functionality is setTextureCoords(), which will copy the 
            texture coordinate rects you supply into internal storage for the 
            billboard set. If your texture sheet is a square grid, you can also 
            use setTextureStacksAndSlices() for more convenience, which will construct 
            the set of texture coordinates for you.
        @par
            When a Billboard is created, it can be assigned a texture coordinate 
            set from within the sets you specify (that set can also be re-specified 
            later). When drawn, the billboard will use those texture coordinates, 
            rather than the full 0-1 range.

        @param coords is a vector of texture coordinates (in UV space) to choose 
            from for each billboard created in the set.
        @param numCoords is how many such coordinate rectangles there are to 
            choose from.
        @remarks
            Set 'coords' to 0 and/or 'numCoords' to 0 to reset the texture coord 
            rects to the initial set of a single rectangle spanning 0 through 1 in 
            both U and V (i e, the entire texture).
        @see
            BillboardSet::setTextureStacksAndSlices()
            Billboard::setTexcoordIndex()
        */
        virtual void setTextureCoords( Ogre::FloatRect const * coords, uint16 numCoords );

        /** setTextureStacksAndSlices() will generate texture coordinate rects as if the 
            texture for the billboard set contained 'stacks' rows of 'slices' 
            images each, all equal size. Thus, if the texture size is 512x512 
            and 'stacks' is 4 and 'slices' is 8, each sub-rectangle of the texture 
            would be 128 texels tall and 64 texels wide.
        @remarks
            This function is short-hand for creating a regular set and calling 
            setTextureCoords() yourself. The numbering used for Billboard::setTexcoordIndex() 
            counts first across, then down, so top-left is 0, the one to the right 
            of that is 1, and the lower-right is stacks*slices-1.
        @see
            BillboardSet::setTextureCoords()
        */
        virtual void setTextureStacksAndSlices( uchar stacks, uchar slices );

        /** getTextureCoords() returns the current texture coordinate rects in 
            effect. By default, there is only one texture coordinate rect in the 
            set, spanning the entire texture from 0 through 1 in each direction.
        @see
            BillboardSet::setTextureCoords()
        */
        virtual Ogre::FloatRect const * getTextureCoords( uint16 * oNumCoords );

        /** Set whether or not the BillboardSet will use point rendering
            rather than manually generated quads.
        @remarks
            By default a billboardset is rendered by generating geometry for a
            textured quad in memory, taking into account the size and 
            orientation settings, and uploading it to the video card. 
            The alternative is to use hardware point rendering, which means that
            only one position needs to be sent per billboard rather than 4 and
            the hardware sorts out how this is rendered based on the render
            state.
        @par
            Using point rendering is faster than generating quads manually, but
            is more restrictive. The following restrictions apply:
            \li Only the BBT_POINT type is supported
            \li Size and appearance of each billboard is controlled by the 
                material (Pass::setPointSize, Pass::setPointSizeAttenuation, 
                Pass::setPointSpritesEnabled)
            \li Per-billboard size is not supported (stems from the above)
            \li Per-billboard rotation is not supported, this can only be 
                controlled through texture unit rotation
            \li Only BBO_CENTER origin is supported
            \li Per-billboard texture coordinates are not supported

        @par
            You will almost certainly want to enable in your material pass
            both point attenuation and point sprites if you use this option. 
        @param enabled True to enable point rendering, false otherwise
        */
        virtual void setPointRenderingEnabled(bool enabled);

        /** Returns whether point rendering is enabled. */
        virtual bool isPointRenderingEnabled(void) const
        { return mPointRendering; }
        
        /// Override to return specific type flag
        uint32 getTypeFlags(void) const;

        /** Set the auto update state of this billboard set.
        @remarks
            This methods controls the updating policy of the vertex buffer.
            By default auto update is true so the vertex buffer is being update every time this billboard set
            is about to be rendered. This behavior best fit when the billboards of this set changes frequently.
            When using static or semi-static billboards, it is recommended to set auto update to false.
            In that case one should call notifyBillboardDataChanged method to reflect changes made to the
            billboards data.
        */
        void setAutoUpdate(bool autoUpdate);

        /** Return the auto update state of this billboard set.*/
        bool getAutoUpdate(void) const { return mAutoUpdate; }

        /** When billboard set is not auto updating its GPU buffer, the user is responsible to inform it
            about any billboard changes in order to reflect them at the rendering stage.
            Calling this method will cause GPU buffers update in the next render queue update.
        */
        void notifyBillboardDataChanged(void) { mBillboardDataChanged = true; }

    };

    /** Factory object for creating BillboardSet instances */
    class _OgreExport BillboardSetFactory : public MovableObjectFactory
    {
    protected:
        MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
    public:
        BillboardSetFactory() {}
        ~BillboardSetFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);  

    };
    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __BillboardSet_H__
