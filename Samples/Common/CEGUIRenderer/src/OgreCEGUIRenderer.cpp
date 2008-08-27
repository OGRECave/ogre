/************************************************************************
	filename: 	OgreCEGUIRenderer.cpp
	created:	11/5/2004
	author:		Paul D Turner
	
	purpose:	Implementation of Renderer class for Ogre engine
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

#include <CEGUI/CEGUIImagesetManager.h>
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUIImage.h>
#include <CEGUI/CEGUIExceptions.h>
#include <CEGUI/CEGUISystem.h>

#include "OgreCEGUIRenderer.h"
#include "OgreCEGUITexture.h"
#include "OgreCEGUIResourceProvider.h"

#include <OgreRenderSystem.h>
#include <OgreRoot.h>
#include <OgreHardwareBufferManager.h>
#include <OgreRenderWindow.h>

// Start of CEGUI namespace section
namespace CEGUI
{
/*************************************************************************
	Constants definitions
*************************************************************************/
const size_t	OgreCEGUIRenderer::VERTEX_PER_QUAD			= 6;
const size_t	OgreCEGUIRenderer::VERTEX_PER_TRIANGLE		= 3;
const size_t	OgreCEGUIRenderer::VERTEXBUFFER_INITIAL_CAPACITY	= 256;
const size_t    OgreCEGUIRenderer::UNDERUSED_FRAME_THRESHOLD = 50000; // halfs buffer every 8 minutes on 100fps

/*************************************************************************
	Utility function to create a render operation and vertex buffer to render quads
*************************************************************************/
static void createQuadRenderOp(Ogre::RenderOperation &d_render_op, 
    Ogre::HardwareVertexBufferSharedPtr &d_buffer, size_t nquads)
{
    using namespace Ogre;
    // Create and initialise the Ogre specific parts required for use in rendering later.
	d_render_op.vertexData = new VertexData;
	d_render_op.vertexData->vertexStart = 0;

	// setup vertex declaration for the vertex format we use
	VertexDeclaration* vd = d_render_op.vertexData->vertexDeclaration;
	size_t vd_offset = 0;
	vd->addElement(0, vd_offset, VET_FLOAT3, VES_POSITION);
	vd_offset += VertexElement::getTypeSize(VET_FLOAT3);
	vd->addElement(0, vd_offset, VET_COLOUR, VES_DIFFUSE);
	vd_offset += VertexElement::getTypeSize(VET_COLOUR);
	vd->addElement(0, vd_offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);

	// create hardware vertex buffer
	d_buffer = HardwareBufferManager::getSingleton().createVertexBuffer(vd->getVertexSize(0), nquads,  
        HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, false);

	// bind vertex buffer
	d_render_op.vertexData->vertexBufferBinding->setBinding(0, d_buffer);

	// complete render operation basic initialisation
	d_render_op.operationType = RenderOperation::OT_TRIANGLE_LIST;
	d_render_op.useIndexes = false;
}

static void destroyQuadRenderOp(Ogre::RenderOperation &d_render_op, 
    Ogre::HardwareVertexBufferSharedPtr &d_buffer)
{
    delete d_render_op.vertexData;
    d_render_op.vertexData = 0;
    d_buffer.setNull();
}

/*************************************************************************
	Constructor
*************************************************************************/
OgreCEGUIRenderer::OgreCEGUIRenderer(Ogre::RenderWindow* window, Ogre::uint8 queue_id, bool post_queue, uint max_quads)
{
	constructor_impl(window, queue_id, post_queue, max_quads);
}


/*************************************************************************
	Constructor (specifying scene manager)
*************************************************************************/
OgreCEGUIRenderer::OgreCEGUIRenderer(Ogre::RenderWindow* window, Ogre::uint8 queue_id, bool post_queue, uint max_quads, Ogre::SceneManager* scene_manager)
{
	constructor_impl(window, queue_id, post_queue, max_quads);

	// hook into ogre rendering system
	setTargetSceneManager(scene_manager);
}


/*************************************************************************
	Destructor
*************************************************************************/
OgreCEGUIRenderer::~OgreCEGUIRenderer(void)
{
	setTargetSceneManager(NULL);

	if (d_ourlistener)
	{
		delete d_ourlistener;
	}

	// cleanup vertex data we allocated in constructor
	destroyQuadRenderOp(d_render_op, d_buffer);
    destroyQuadRenderOp(d_direct_render_op, d_direct_buffer);

	destroyAllTextures();
}


/*************************************************************************
	add's a quad to the list to be rendered
*************************************************************************/
void OgreCEGUIRenderer::addQuad(const Rect& dest_rect, float z, const Texture* tex, const Rect& texture_rect, const ColourRect& colours, QuadSplitMode quad_split_mode)
{
	// if not queueing, render directly (as in, right now!). This is used for the mouse cursor.
	if (!d_queueing)
	{
		renderQuadDirect(dest_rect, z, tex, texture_rect, colours, quad_split_mode);
	}
	else
	{
		d_sorted = false;
		QuadInfo quad;
		
		// set quad position, flipping y co-ordinates, and applying appropriate texel origin offset
		quad.position.d_left	= dest_rect.d_left;
		quad.position.d_right	= dest_rect.d_right;
		quad.position.d_top		= d_display_area.getHeight() - dest_rect.d_top;
		quad.position.d_bottom	= d_display_area.getHeight() - dest_rect.d_bottom;
		quad.position.offset(d_texelOffset);

		// convert quad co-ordinates for a -1 to 1 co-ordinate system.
		quad.position.d_left	/= (d_display_area.getWidth() * 0.5f);
		quad.position.d_right	/= (d_display_area.getWidth() * 0.5f);
		quad.position.d_top		/= (d_display_area.getHeight() * 0.5f);
		quad.position.d_bottom	/= (d_display_area.getHeight() * 0.5f);
		quad.position.offset(Point(-1.0f, -1.0f));

		quad.z				= -1 + z;
		quad.texture		= ((OgreCEGUITexture*)tex)->getOgreTexture();
		quad.texPosition	= texture_rect;

		// covert colours for ogre, note that top / bottom are switched.
		quad.topLeftCol		= colourToOgre(colours.d_bottom_left);
		quad.topRightCol	= colourToOgre(colours.d_bottom_right);
		quad.bottomLeftCol	= colourToOgre(colours.d_top_left);
		quad.bottomRightCol	= colourToOgre(colours.d_top_right);
		
		// set quad split mode
		quad.splitMode = quad_split_mode;

		d_quadlist.insert(quad);
	}
}



/*************************************************************************
perform final rendering for all queued renderable quads.
*************************************************************************/
void OgreCEGUIRenderer::doRender(void)
{
    // Render if overlays enabled and the quad list is not empty
	if (d_render_sys->_getViewport()->getOverlaysEnabled() && !d_quadlist.empty())
	{
        /// Quad list needs to be sorted and thus the vertex buffer rebuilt. If not, we can
        /// reuse the vertex buffer resulting in a nice speed gain.
        if(!d_sorted)
        {
            sortQuads();
            /// Resize vertex buffer if it is too small
            size_t size = d_buffer->getNumVertices();
            size_t requestedSize = d_quadlist.size()*VERTEX_PER_QUAD;
            if(size < requestedSize)
            {
                /// Double buffer size until smaller than requested size
                while(size < requestedSize)
                    size = size * 2;
                /// Reallocate the buffer
                destroyQuadRenderOp(d_render_op, d_buffer);
                createQuadRenderOp(d_render_op, d_buffer, size);
            }
            else if(requestedSize < size/2 && d_underused_framecount >= UNDERUSED_FRAME_THRESHOLD)
            {
                /// Resize vertex buffer if it has been to big for too long
                size = size / 2;
                destroyQuadRenderOp(d_render_op, d_buffer);
                createQuadRenderOp(d_render_op, d_buffer, size);
                /// Reset underused framecount so it takes another UNDERUSED_FRAME_THRESHOLD to half again
                d_underused_framecount = 0;
            }
            /// Fill the buffer
            QuadVertex*	buffmem;
            buffmem = (QuadVertex*)d_buffer->lock(Ogre::HardwareVertexBuffer::HBL_DISCARD);
            // iterate over each quad in the list
            for (QuadList::iterator i = d_quadlist.begin(); i != d_quadlist.end(); ++i)
            {
                const QuadInfo& quad = (*i);
                // setup Vertex 1...
                buffmem->x = quad.position.d_left;
                buffmem->y = quad.position.d_bottom;
                buffmem->z = quad.z;
                buffmem->diffuse = quad.topLeftCol;
                buffmem->tu1 = quad.texPosition.d_left;
                buffmem->tv1 = quad.texPosition.d_bottom;
                ++buffmem;
    
                // setup Vertex 2...
                
                // top-left to bottom-right diagonal
                if (quad.splitMode == TopLeftToBottomRight)
                {
                    buffmem->x = quad.position.d_right;
                    buffmem->y = quad.position.d_bottom;
                    buffmem->z = quad.z;
                    buffmem->diffuse = quad.topRightCol;
                    buffmem->tu1 = quad.texPosition.d_right;
                    buffmem->tv1 = quad.texPosition.d_bottom;
                }
                // bottom-left to top-right diagonal
                else
                {
                    buffmem->x = quad.position.d_right;
                    buffmem->y = quad.position.d_top;
                    buffmem->z = quad.z;
                    buffmem->diffuse = quad.bottomRightCol;
                    buffmem->tu1 = quad.texPosition.d_right;
                    buffmem->tv1 = quad.texPosition.d_top;
                }
                ++buffmem;
    
                // setup Vertex 3...
                buffmem->x = quad.position.d_left;
                buffmem->y = quad.position.d_top;
                buffmem->z = quad.z;
                buffmem->diffuse = quad.bottomLeftCol;
                buffmem->tu1 = quad.texPosition.d_left;
                buffmem->tv1 = quad.texPosition.d_top;
                ++buffmem;
    
                // setup Vertex 4...
                buffmem->x = quad.position.d_right;
                buffmem->y = quad.position.d_bottom;
                buffmem->z = quad.z;
                buffmem->diffuse = quad.topRightCol;
                buffmem->tu1 = quad.texPosition.d_right;
                buffmem->tv1 = quad.texPosition.d_bottom;
                ++buffmem;
    
                // setup Vertex 5...
                buffmem->x = quad.position.d_right;
                buffmem->y = quad.position.d_top;
                buffmem->z = quad.z;
                buffmem->diffuse = quad.bottomRightCol;
                buffmem->tu1 = quad.texPosition.d_right;
                buffmem->tv1 = quad.texPosition.d_top;
                ++buffmem;
    
                // setup Vertex 6...
                
                // top-left to bottom-right diagonal
                if (quad.splitMode == TopLeftToBottomRight)
                {
                    buffmem->x = quad.position.d_left;
                    buffmem->y = quad.position.d_top;
                    buffmem->z = quad.z;
                    buffmem->diffuse = quad.bottomLeftCol;
                    buffmem->tu1 = quad.texPosition.d_left;
                    buffmem->tv1 = quad.texPosition.d_top;
                }
                // bottom-left to top-right diagonal
                else
                {
                    buffmem->x = quad.position.d_left;
                    buffmem->y = quad.position.d_bottom;
                    buffmem->z = quad.z;
                    buffmem->diffuse = quad.topLeftCol;
                    buffmem->tu1 = quad.texPosition.d_left;
                    buffmem->tv1 = quad.texPosition.d_bottom;
                }
                ++buffmem;
            }
    
            // ensure we leave the buffer in the unlocked state
            d_buffer->unlock();
        }
        
        /// Render the buffer
        d_bufferPos = 0;
		bool first = true;

        // Iterate over each quad in the list and render it
        QuadList::iterator i = d_quadlist.begin();
        while(i != d_quadlist.end())
        {
            
            d_currTexture = i->texture;
            d_render_op.vertexData->vertexStart = d_bufferPos;
            for (; i != d_quadlist.end(); ++i)
            {
                const QuadInfo& quad = (*i);
                if (d_currTexture != quad.texture)
				{
                    /// If it has a different texture, render this quad in next operation
					/// also need to reset render states
					first = true;
		            break;
				}
                d_bufferPos += VERTEX_PER_QUAD;
            }
            d_render_op.vertexData->vertexCount = d_bufferPos - d_render_op.vertexData->vertexStart;
            /// Set texture, and do the render
            d_render_sys->_setTexture(0, true, d_currTexture);
			if (first)
			{
				initRenderStates();
				first = false;
			}
            d_render_sys->_render(d_render_op);
        }

	}
    /// Count frames to check if utilization of vertex buffer was below half the capacity for 500,000 frames
    if(d_bufferPos < d_buffer->getNumVertices()/2)
       d_underused_framecount++;
    else
       d_underused_framecount = 0;
}


/*************************************************************************
	clear the queue
*************************************************************************/
void OgreCEGUIRenderer::clearRenderList(void)
{
	d_sorted = true;
	d_quadlist.clear();
}


/*************************************************************************
	create an empty texture
*************************************************************************/
Texture* OgreCEGUIRenderer::createTexture(void)
{
	OgreCEGUITexture* tex = new OgreCEGUITexture(this);
	d_texturelist.push_back(tex);
	return tex;
}


/*************************************************************************
	create a texture and load it with the specified file.
*************************************************************************/
Texture* OgreCEGUIRenderer::createTexture(const String& filename, const String& resourceGroup)
{
	OgreCEGUITexture* tex = (OgreCEGUITexture*)createTexture();
	tex->loadFromFile(filename, resourceGroup);

	return tex;
}


/*************************************************************************
	create a texture and set it to the specified size
*************************************************************************/
Texture* OgreCEGUIRenderer::createTexture(float size)
{
	OgreCEGUITexture* tex = (OgreCEGUITexture*)createTexture();
	tex->setOgreTextureSize((uint)size);

	return tex;
}


/*************************************************************************
	destroy the given texture
*************************************************************************/
void OgreCEGUIRenderer::destroyTexture(Texture* texture)
{
	if (texture != NULL)
	{
		OgreCEGUITexture* tex = (OgreCEGUITexture*)texture;

		d_texturelist.remove(tex);
		delete tex;
	}
}


/*************************************************************************
	destroy all textures still active
*************************************************************************/
void OgreCEGUIRenderer::destroyAllTextures(void)
{
	while (!d_texturelist.empty())
	{
		destroyTexture(*(d_texturelist.begin()));
	}
}


/*************************************************************************
	setup states etc	
*************************************************************************/
void OgreCEGUIRenderer::initRenderStates(void)
{
	using namespace Ogre;

	// set-up matrices
	d_render_sys->_setWorldMatrix(Matrix4::IDENTITY);
	d_render_sys->_setViewMatrix(Matrix4::IDENTITY);
	d_render_sys->_setProjectionMatrix(Matrix4::IDENTITY);

	// initialise render settings
	d_render_sys->setLightingEnabled(false);
	d_render_sys->_setDepthBufferParams(false, false);
	d_render_sys->_setDepthBias(0, 0);
	d_render_sys->_setCullingMode(CULL_NONE);
	d_render_sys->_setFog(FOG_NONE);
	d_render_sys->_setColourBufferWriteEnabled(true, true, true, true);
	d_render_sys->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
	d_render_sys->unbindGpuProgram(GPT_VERTEX_PROGRAM);
	d_render_sys->setShadingType(SO_GOURAUD);
	d_render_sys->_setPolygonMode(PM_SOLID);

	// initialise texture settings
	d_render_sys->_setTextureCoordCalculation(0, TEXCALC_NONE);
	d_render_sys->_setTextureCoordSet(0, 0);
	d_render_sys->_setTextureUnitFiltering(0, FO_LINEAR, FO_LINEAR, FO_POINT);
	d_render_sys->_setTextureAddressingMode(0, d_uvwAddressMode);
	d_render_sys->_setTextureMatrix(0, Matrix4::IDENTITY);
	d_render_sys->_setAlphaRejectSettings(CMPF_ALWAYS_PASS, 0, false);
	d_render_sys->_setTextureBlendMode(0, d_colourBlendMode);
	d_render_sys->_setTextureBlendMode(0, d_alphaBlendMode);
	d_render_sys->_disableTextureUnitsFrom(1);

	// enable alpha blending
	d_render_sys->_setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
}


 
/*************************************************************************
	sort quads list according to texture	
*************************************************************************/
void OgreCEGUIRenderer::sortQuads(void)
{
	if (!d_sorted)
	{
		d_sorted = true;
	}

}

/*************************************************************************
render a quad directly to the display
*************************************************************************/
void OgreCEGUIRenderer::renderQuadDirect(const Rect& dest_rect, float z, const Texture* tex, const Rect& texture_rect, const ColourRect& colours, QuadSplitMode quad_split_mode)
{
	if (d_render_sys->_getViewport()->getOverlaysEnabled())
	{
		z = -1 + z;

		Rect final_rect;

		// set quad position, flipping y co-ordinates, and applying appropriate texel origin offset
		final_rect.d_left	= dest_rect.d_left;
		final_rect.d_right	= dest_rect.d_right;
		final_rect.d_top	= d_display_area.getHeight() - dest_rect.d_top;
		final_rect.d_bottom	= d_display_area.getHeight() - dest_rect.d_bottom;
		final_rect.offset(d_texelOffset);

		// convert quad co-ordinates for a -1 to 1 co-ordinate system.
		final_rect.d_left	/= (d_display_area.getWidth() * 0.5f);
		final_rect.d_right	/= (d_display_area.getWidth() * 0.5f);
		final_rect.d_top	/= (d_display_area.getHeight() * 0.5f);
		final_rect.d_bottom	/= (d_display_area.getHeight() * 0.5f);
		final_rect.offset(Point(-1.0f, -1.0f));

		// convert colours for ogre, note that top / bottom are switched.
        uint32 topLeftCol	= colourToOgre(colours.d_bottom_left);
        uint32 topRightCol	= colourToOgre(colours.d_bottom_right);
        uint32 bottomLeftCol	= colourToOgre(colours.d_top_left);
        uint32 bottomRightCol= colourToOgre(colours.d_top_right);

		QuadVertex*	buffmem = (QuadVertex*)d_direct_buffer->lock(Ogre::HardwareVertexBuffer::HBL_DISCARD);

		// setup Vertex 1...
		buffmem->x	= final_rect.d_left;
		buffmem->y	= final_rect. d_bottom;
		buffmem->z	= z;
		buffmem->diffuse = topLeftCol;
		buffmem->tu1	= texture_rect.d_left;
		buffmem->tv1	= texture_rect.d_bottom;
		++buffmem;

		// setup Vertex 2...
		
		// top-left to bottom-right diagonal
		if (quad_split_mode == TopLeftToBottomRight)
		{
			buffmem->x	= final_rect.d_right;
			buffmem->y = final_rect.d_bottom;
			buffmem->z	= z;
			buffmem->diffuse = topRightCol;
			buffmem->tu1	= texture_rect.d_right;
			buffmem->tv1	= texture_rect.d_bottom;
			++buffmem;
		}
		// bottom-left to top-right diagonal
		else
		{
			buffmem->x	= final_rect.d_right;
			buffmem->y = final_rect.d_top;
			buffmem->z	= z;
			buffmem->diffuse = bottomRightCol;
			buffmem->tu1	= texture_rect.d_right;
			buffmem->tv1	= texture_rect.d_top;
			++buffmem;
		}

		// setup Vertex 3...
		buffmem->x	= final_rect.d_left;
		buffmem->y	= final_rect.d_top;
		buffmem->z	= z;
		buffmem->diffuse = bottomLeftCol;
		buffmem->tu1	= texture_rect.d_left;
		buffmem->tv1	= texture_rect.d_top;
		++buffmem;

		// setup Vertex 4...
		buffmem->x	= final_rect.d_right;
		buffmem->y	= final_rect.d_bottom;
		buffmem->z	= z;
		buffmem->diffuse = topRightCol;
		buffmem->tu1	= texture_rect.d_right;
		buffmem->tv1	= texture_rect.d_bottom;
		++buffmem;

		// setup Vertex 5...
		buffmem->x	= final_rect.d_right;
		buffmem->y	= final_rect.d_top;
		buffmem->z	= z;
		buffmem->diffuse = bottomRightCol;
		buffmem->tu1	= texture_rect.d_right;
		buffmem->tv1	= texture_rect.d_top;
		++buffmem;

		// setup Vertex 6...
		
		// top-left to bottom-right diagonal
		if (quad_split_mode == TopLeftToBottomRight)
		{
			buffmem->x	= final_rect.d_left;
			buffmem->y = final_rect.d_top;
			buffmem->z	= z;
			buffmem->diffuse = bottomLeftCol;
			buffmem->tu1	= texture_rect.d_left;
			buffmem->tv1	= texture_rect.d_top;
		}
		// bottom-left to top-right diagonal
		else
		{
			buffmem->x	= final_rect.d_left;
			buffmem->y = final_rect.d_bottom;
			buffmem->z	= z;
			buffmem->diffuse = topLeftCol;
			buffmem->tu1	= texture_rect.d_left;
			buffmem->tv1	= texture_rect.d_bottom;
		}

		d_direct_buffer->unlock();

        //
		// perform rendering...
		//
		d_render_sys->_setTexture(0, true, ((OgreCEGUITexture*)tex)->getOgreTexture()->getName());
		initRenderStates();
		d_direct_render_op.vertexData->vertexCount = VERTEX_PER_QUAD;
		d_render_sys->_render(d_direct_render_op);
	}

}

/*************************************************************************
	convert ARGB colour value to whatever the Ogre render system is
	expecting.	
*************************************************************************/
uint32 OgreCEGUIRenderer::colourToOgre(const colour& col) const
{
	Ogre::ColourValue cv(col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());

    uint32 final;
	d_render_sys->convertColourValue(cv, &final);

	return final;
}


/*************************************************************************
	Set the scene manager to be used for rendering the GUI.	
*************************************************************************/
void OgreCEGUIRenderer::setTargetSceneManager(Ogre::SceneManager* scene_manager)
{
	// unhook from current scene manager.
	if (d_sceneMngr != NULL)
	{
		d_sceneMngr->removeRenderQueueListener(d_ourlistener);
		d_sceneMngr = NULL;
	}

	// hook new scene manager if that is not NULL
	if (scene_manager != NULL)
	{
		d_sceneMngr = scene_manager;
		d_sceneMngr->addRenderQueueListener(d_ourlistener);
	}

}


/*************************************************************************
	Set the target render queue for GUI rendering.	
*************************************************************************/
void OgreCEGUIRenderer::setTargetRenderQueue(Ogre::uint8 queue_id, bool post_queue)
{
	d_queue_id		= queue_id;
	d_post_queue	= post_queue;

	if (d_ourlistener != NULL)
	{
		d_ourlistener->setTargetRenderQueue(queue_id);
		d_ourlistener->setPostRenderQueue(post_queue);
	}

}


/*************************************************************************
	perform main work of the constructor
*************************************************************************/
void OgreCEGUIRenderer::constructor_impl(Ogre::RenderWindow* window, Ogre::uint8 queue_id, bool post_queue, uint max_quads)
{
	using namespace Ogre;

	// initialise the renderer fields
	d_queueing		= true;
	d_queue_id		= queue_id;
	d_currTexture.isNull();
	d_post_queue	= post_queue;
	d_sceneMngr		= NULL;
	d_bufferPos		= 0;
	d_sorted		= true;
	d_ogre_root		= Root::getSingletonPtr();
	d_render_sys	= d_ogre_root->getRenderSystem();
    // set ID string
    d_identifierString = "CEGUI::OgreRenderer - Official Ogre based renderer module for CEGUI";

	// Create and initialise the Ogre specific parts required for use in rendering later.
    // Main GUI
    createQuadRenderOp(d_render_op, d_buffer, VERTEXBUFFER_INITIAL_CAPACITY);
    d_underused_framecount = 0;

    // Mouse cursor
    createQuadRenderOp(d_direct_render_op, d_direct_buffer, VERTEX_PER_QUAD);

	// Discover display settings and setup d_display_area
	d_display_area.d_left	= 0;
	d_display_area.d_top	= 0;
	d_display_area.d_right	= window->getWidth();
	d_display_area.d_bottom	= window->getHeight();

	// initialise required texel offset
	d_texelOffset = Point((float)d_render_sys->getHorizontalTexelOffset(), -(float)d_render_sys->getVerticalTexelOffset());

	// create listener which will handler the rendering side of things for us.
	d_ourlistener = new CEGUIRQListener(this, queue_id, post_queue);

	// Initialise blending modes to be used.
	d_colourBlendMode.blendType	= Ogre::LBT_COLOUR;
	d_colourBlendMode.source1	= Ogre::LBS_TEXTURE;
	d_colourBlendMode.source2	= Ogre::LBS_DIFFUSE;
	d_colourBlendMode.operation	= Ogre::LBX_MODULATE;

	d_alphaBlendMode.blendType	= Ogre::LBT_ALPHA;
	d_alphaBlendMode.source1	= Ogre::LBS_TEXTURE;
	d_alphaBlendMode.source2	= Ogre::LBS_DIFFUSE;
	d_alphaBlendMode.operation	= Ogre::LBX_MODULATE;

	d_uvwAddressMode.u = Ogre::TextureUnitState::TAM_CLAMP;
	d_uvwAddressMode.v = Ogre::TextureUnitState::TAM_CLAMP;
	d_uvwAddressMode.w = Ogre::TextureUnitState::TAM_CLAMP;
}


/*************************************************************************
	Create a texture from an existing Ogre::TexturePtr object	
*************************************************************************/
Texture* OgreCEGUIRenderer::createTexture(Ogre::TexturePtr& texture)
{
	OgreCEGUITexture* t = (OgreCEGUITexture*)createTexture();

	if (!texture.isNull())
	{
		t->setOgreTexture(texture);
	}

	return t;

}

/*************************************************************************
	Create a resource provider object
*************************************************************************/
ResourceProvider* OgreCEGUIRenderer::createResourceProvider(void)
{
    d_resourceProvider = new OgreCEGUIResourceProvider();
    return d_resourceProvider;
}

/*************************************************************************
Set the size of the display in pixels.	
*************************************************************************/
void OgreCEGUIRenderer::setDisplaySize(const Size& sz)
{
	if (d_display_area.getSize() != sz)
	{
		d_display_area.setSize(sz);

		EventArgs args;
		fireEvent(EventDisplaySizeChanged, args, EventNamespace);
	}

}

/*************************************************************************
	Callback from Ogre invoked before other stuff in our target queue
	is rendered
*************************************************************************/
void CEGUIRQListener::renderQueueStarted(Ogre::uint8 id, const Ogre::String& invocation, 
										 bool& skipThisQueue)
{
	if ((!d_post_queue) && (d_queue_id == id))
	{
		CEGUI::System::getSingleton().renderGUI();
	}

}


/*************************************************************************
Callback from Ogre invoked after other stuff in our target queue
is rendered
*************************************************************************/
void CEGUIRQListener::renderQueueEnded(Ogre::uint8 id, const Ogre::String& invocation, bool& repeatThisQueue)
{
	if ((d_post_queue) && (d_queue_id == id))
	{
		CEGUI::System::getSingleton().renderGUI();
	}

}

} // End of  CEGUI namespace section
