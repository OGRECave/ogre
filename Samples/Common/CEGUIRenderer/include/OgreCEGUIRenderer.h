/************************************************************************
	filename: 	OgreCEGUIRenderer.h
	created:	11/5/2004
	author:		Paul D Turner

	purpose:	Interface for main Ogre GUI renderer class
*************************************************************************/
/*************************************************************************
    Crazy Eddie's GUI System (http://www.cegui.org.uk)
    Copyright (C)2004 - 2005 Paul D Turner (paul@cegui.org.uk)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*************************************************************************/
/*************************************************************************
	This file contains code that is specific to Ogre (http://www.ogre3d.org)
*************************************************************************/
#ifndef _OgreCEGUIRenderer_h_
#define _OgreCEGUIRenderer_h_

#include <CEGUI/CEGUIBase.h>
#include <CEGUI/CEGUIRenderer.h>
#include <CEGUI/CEGUITexture.h>

#include <OgreRenderQueueListener.h>
#include <OgreSceneManagerEnumerator.h>
#include <OgreTextureUnitState.h>

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#   ifdef OGRE_GUIRENDERER_EXPORTS
#       define OGRE_GUIRENDERER_API __declspec(dllexport)
#   else
#       if defined(__MINGW32__)
#           define OGRE_GUIRENDERER_API
#       else
#           define OGRE_GUIRENDERER_API __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define OGRE_GUIRENDERER_API  __attribute__ ((visibility("default")))
#else
#    define OGRE_GUIRENDERER_API
#endif


// Start of CEGUI namespace section
namespace CEGUI
{
/*************************************************************************
	Forward refs
*************************************************************************/
class OgreCEGUITexture;
class OgreCEGUIRenderer;


/*!
\brief
	RenderQueueListener based class used to hook into the ogre rendering system
*/
class _OgrePrivate CEGUIRQListener : public Ogre::RenderQueueListener
{
public:
	CEGUIRQListener(OgreCEGUIRenderer* renderer, Ogre::uint8 queue_id, bool post_queue)
	{
		d_renderer		= renderer;
		d_queue_id		= queue_id;
		d_post_queue	= post_queue;
	}

	virtual ~CEGUIRQListener() {}

	virtual void	renderQueueStarted(Ogre::uint8 id, const Ogre::String& invocation, bool& skipThisQueue);
	virtual void	renderQueueEnded(Ogre::uint8 id, const Ogre::String& invocation, bool& repeatThisQueue);

	// methods for adjusting target queue settings
	void	setTargetRenderQueue(Ogre::uint8 queue_id)		{d_queue_id = queue_id;}
	void	setPostRenderQueue(bool post_queue)		{d_post_queue = post_queue;}

private:
	/*************************************************************************
		Implementation Data
	*************************************************************************/
	OgreCEGUIRenderer*				d_renderer;		//!< CEGUI renderer object for Ogre.
	Ogre::uint8	d_queue_id;		//!< ID of the queue that we are hooked into
	bool						d_post_queue;	//!< true if we render after everything else in our queue.
};


/*!
\brief
	Renderer class to interface with Ogre engine.
*/
class OGRE_GUIRENDERER_API OgreCEGUIRenderer : public Renderer
{
public:
	/*!
	\brief
		Constructor for renderer class that uses Ogre for rendering. Note that if
		you use this option you must call setTargetSceneManager before rendering.

	\param window
		Pointer to an Ogre::RenderWindow object.

	\param queue_id
		Ogre::uint8 value that specifies where the GUI should appear in the ogre rendering output.

	\param post_queue
		set to true to have GUI rendered after render queue \a queue_id, or false to have the GUI rendered before render queue
		\a queue_id.

	\param max_quads
		Obsolete.  Set to 0.

	*/
	OgreCEGUIRenderer(Ogre::RenderWindow* window,
		Ogre::uint8 queue_id = Ogre::RENDER_QUEUE_OVERLAY,
		bool post_queue = false, uint max_quads = 0);


	/*!
	\brief
		Constructor for renderer class that uses Ogre for rendering.

	\param window
		Pointer to an Ogre::RenderWindow object.

	\param queue_id
		Ogre::uint8 value that specifies where the GUI should appear in the ogre rendering output.

	\param post_queue
		set to true to have GUI rendered after render queue \a queue_id, or false to have the GUI rendered before render queue
		\a queue_id.

	\param max_quads
		Obsolete.  Set to 0.

	\param scene_manager
		Pointer to an Ogre::SceneManager object that is to be used for GUI rendering.
	*/
	OgreCEGUIRenderer(Ogre::RenderWindow* window, Ogre::uint8 queue_id, bool post_queue, uint max_quads, Ogre::SceneManager* scene_manager);


	/*!
	\brief
		Destructor for Ogre renderer.
	*/
	virtual ~OgreCEGUIRenderer(void);



	// add's a quad to the list to be rendered
	virtual	void	addQuad(const Rect& dest_rect, float z, const Texture* tex, const Rect& texture_rect, const ColourRect& colours, QuadSplitMode quad_split_mode);

	// perform final rendering for all queued renderable quads.
	virtual	void	doRender(void);

	// clear the queue
	virtual	void	clearRenderList(void);


	/*!
	\brief
		Enable or disable the queueing of quads from this point on.

		This only affects queueing.  If queueing is turned off, any calls to addQuad will cause the quad to be rendered directly.  Note that
		disabling queueing will not cause currently queued quads to be rendered, nor is the queue cleared - at any time the queue can still
		be drawn by calling doRender, and the list can be cleared by calling clearRenderList.  Re-enabling the queue causes subsequent quads
		to be added as if queueing had never been disabled.

	\param setting
		true to enable queueing, or false to disable queueing (see notes above).

	\return
		Nothing
	*/
	virtual void	setQueueingEnabled(bool setting)		{d_queueing = setting;}


	// create an empty texture
	virtual	Texture*	createTexture(void);

	// create a texture and load it with the specified file.
	virtual	Texture*	createTexture(const String& filename, const String& resourceGroup = "General");

	// create a texture and set it to the specified size
	virtual	Texture*	createTexture(float size);

    // create an OGRE resource provider.
    virtual ResourceProvider* createResourceProvider(void);

	// destroy the given texture
	virtual	void		destroyTexture(Texture* texture);

	// destroy all textures still active
	virtual void		destroyAllTextures(void);


	/*!
	\brief
		Return whether queueing is enabled.

	\return
		true if queueing is enabled, false if queueing is disabled.
	*/
	virtual bool	isQueueingEnabled(void) const	{return d_queueing;}


	/*!
	\brief
	Return the current width of the display in pixels

	\return
	float value equal to the current width of the display in pixels.
	*/
	virtual float	getWidth(void) const		{return d_display_area.getWidth();}


	/*!
	\brief
	Return the current height of the display in pixels

	\return
	float value equal to the current height of the display in pixels.
	*/
	virtual float	getHeight(void) const		{return d_display_area.getHeight();}


	/*!
	\brief
	Return the size of the display in pixels

	\return
	Size object describing the dimensions of the current display.
	*/
	virtual Size	getSize(void) const			{return d_display_area.getSize();}


	/*!
	\brief
	Return a Rect describing the screen

	\return
	A Rect object that describes the screen area.  Typically, the top-left values are always 0, and the size of the area described is
	equal to the screen resolution.
	*/
	virtual Rect	getRect(void) const			{return d_display_area;}


	/*!
	\brief
		Return the maximum texture size available

	\return
		Size of the maximum supported texture in pixels (textures are always assumed to be square)
	*/
	virtual	uint	getMaxTextureSize(void) const		{return 2048;}		// TODO: Change to proper value


	/*!
	\brief
		Return the horizontal display resolution dpi

	\return
		horizontal resolution of the display in dpi.
	*/
	virtual	uint	getHorzScreenDPI(void) const	{return 96;}


	/*!
	\brief
		Return the vertical display resolution dpi

	\return
		vertical resolution of the display in dpi.
	*/
	virtual	uint	getVertScreenDPI(void) const	{return 96;}


	/*!
	\brief
		Set the scene manager to be used for rendering the GUI.

		The GUI system will be unhooked from the current scene manager and attached to what ever
		is specified here.

	\param scene_manager
		Pointer to an Ogre::SceneManager object that is the new target Ogre::SceneManager to be
		used for GUI rendering.

	\return
		Nothing.
	*/
	void	setTargetSceneManager(Ogre::SceneManager* scene_manager);


	/*!
	\brief
		Set the target render queue for GUI rendering.

	\param queue_id
		Ogre::uint8 value specifying the render queue that the GUI system should attach to.

	\param post_queue
		- true to specify that the GUI should render after everything else in render queue \a queue_id.
		- false to specify the GUI should render before everything else in render queue \a queue_id.

	\return
		Nothing.
	*/
	void	setTargetRenderQueue(Ogre::uint8 queue_id, bool post_queue);


	/*!
	\brief
		Create a texture from an existing Ogre::TexturePtr object.

	\note
		If you want to use an Ogre::RenderTexture (for putting rendered output onto Gui elements or other
		advanced techniques), you can get the Ogre::TexturePtr to be used by calling Ogre::TextureManager::getByName()
		passing the name returned from Ogre::RenderTexture::getName() (and casting the result as necessary).

	\param texture
		pointer to an Ogre::TexturePtr object to be used as the basis for the new CEGUI::Texture

	\return
		Pointer to the newly created CEGUI::TexturePtr object.
	*/
	Texture*	createTexture(Ogre::TexturePtr& texture);


	/*!
	\brief
	Set the size of the display in pixels.

	You do not have to call this method under normal operation as the system
	will automatically extract the size from the current view port.

	\note
	This method will cause the EventDisplaySizeChanged event to fire if the
	display size has changed.

	\param sz
	Size object describing the size of the display.

	\return
	Nothing.
	*/
	void	setDisplaySize(const Size& sz);


private:
	/************************************************************************
		Implementation Constants
	************************************************************************/
	static const size_t    VERTEX_PER_QUAD;						 //!< number of vertices per quad
	static const size_t    VERTEX_PER_TRIANGLE;					 //!< number of vertices for a triangle
    static const size_t    VERTEXBUFFER_INITIAL_CAPACITY;		 //!< initial capacity of the allocated vertex buffer
    static const size_t    UNDERUSED_FRAME_THRESHOLD;            //!< number of frames to wait before shrinking buffer

	/*************************************************************************
	    Implementation Structs & classes
	*************************************************************************/
	/*!
	\brief
		structure used for all vertices.
	*/
	struct QuadVertex {
		float x, y, z;			//!< The position for the vertex.
		Ogre::RGBA diffuse;		//!< colour of the vertex
		float tu1, tv1;			//!< texture coordinates
	};

	/*!
	\brief
		structure holding details about a quad to be drawn
	*/
	struct QuadInfo
	{
		Ogre::TexturePtr		texture;
		Rect				position;
		float				z;
		Rect				texPosition;
        uint32		        topLeftCol;
        uint32		        topRightCol;
        uint32		        bottomLeftCol;
        uint32		        bottomRightCol;

        QuadSplitMode		splitMode;

		bool operator<(const QuadInfo& other) const
		{
			// this is intentionally reversed.
			return z > other.z;
		}
	};


	/*************************************************************************
	    Implementation Methods
	*************************************************************************/
	// setup states etc
	void	initRenderStates(void);

	// sort quads list according to texture
	void	sortQuads(void);

	// render a quad directly to the display
	void	renderQuadDirect(const Rect& dest_rect, float z, const Texture* tex, const Rect& texture_rect, const ColourRect& colours, QuadSplitMode quad_split_mode);

	// convert colour value to whatever the Ogre render system is expecting.
    uint32    colourToOgre(const colour& col) const;

	// perform main work of the constructor.  This does everything except the final hook into the render system.
	void	constructor_impl(Ogre::RenderWindow* window, Ogre::uint8 queue_id, bool post_queue, uint max_quads);


	/*************************************************************************
	    Implementation Data
	*************************************************************************/
	Rect				d_display_area;

	typedef std::multiset<QuadInfo>		QuadList;
	QuadList d_quadlist;
	bool	 d_queueing;		//!< setting for queueing control.

	// Ogre specific bits.
	Ogre::Root*					d_ogre_root;		//!< pointer to the Ogre root object that we attach to
	Ogre::RenderSystem*			d_render_sys;		//!< Pointer to the render system for Ogre.
	Ogre::uint8	d_queue_id;			//!< ID of the queue that we are hooked into
	Ogre::TexturePtr			d_currTexture;		//!< currently set texture;
	Ogre::RenderOperation		d_render_op;		//!< Ogre render operation we use to do our stuff.
	Ogre::HardwareVertexBufferSharedPtr	d_buffer;	//!< vertex buffer to queue sprite rendering
    size_t d_underused_framecount;                  //!< Number of frames elapsed since buffer utilization was above half the capacity
    Ogre::RenderOperation		d_direct_render_op;		//!< Renderop for cursor
	Ogre::HardwareVertexBufferSharedPtr	d_direct_buffer;	//!< Renderop for cursor
	Ogre::SceneManager*			d_sceneMngr;		//!< The scene manager we are hooked into.
	Ogre::LayerBlendModeEx		d_colourBlendMode;	//!< Controls colour blending mode used.
	Ogre::LayerBlendModeEx		d_alphaBlendMode;	//!< Controls alpha blending mode used.
	Ogre::TextureUnitState::UVWAddressingMode d_uvwAddressMode;

	CEGUIRQListener*			d_ourlistener;
	bool						d_post_queue;		//!< true if we render after everything else in our queue.
	size_t						d_bufferPos;		//!< index into buffer where next vertex should be put.
	bool						d_sorted;			//!< true when data in quad list is sorted.
	Point						d_texelOffset;		//!< Offset required for proper texel mapping.

	Ogre::list<OgreCEGUITexture*>::type d_texturelist;		//!< List used to track textures.
};

} // End of  CEGUI namespace section


#endif	// end of guard _OgreCEGUIRenderer_h_
