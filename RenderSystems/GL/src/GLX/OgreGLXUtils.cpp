/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"
#include "OgreGLXUtils.h"

#include "OgreNoMemoryMacros.h"
#include <GL/glxext.h>

namespace Ogre {
	struct visual_attribs
	{
	   /* X visual attribs */
	   int id;
	   int klass;
	   int depth;
	   int redMask, greenMask, blueMask;
	   int colormapSize;
	   int bitsPerRGB;

	   /* GL visual attribs */
	   int supportsGL;
	   int transparentType;
	   int transparentRedValue;
	   int transparentGreenValue;
	   int transparentBlueValue;
	   int transparentAlphaValue;
	   int transparentIndexValue;
	   int bufferSize;
	   int level;
	   int rgba;
	   int doubleBuffer;
	   int stereo;
	   int auxBuffers;
	   int redSize, greenSize, blueSize, alphaSize;
	   int depthSize;
	   int stencilSize;
	   int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
	   int numSamples, numMultisample;
	   int visualCaveat;
	};



	FBConfigData::FBConfigData():
		configID(0), visualID(0), bufferSize(0), level(0), doubleBuffer(0), stereo(0),
		auxBuffers(0), renderType(0),
		redSize(0), greenSize(0), blueSize(0), alphaSize(0),
		depthSize(0), stencilSize(0),
		accumRedSize(0), accumGreenSize(0), accumBlueSize(0), accumAlphaSize(0),
		drawableType(0), caveat(0),
		maxPBufferWidth(0), maxPBufferHeight(0), maxPBufferPixels(0)
	{
		memset(this, 0, sizeof(FBConfigData));
	}
	FBConfigData::FBConfigData(Display *dpy, GLXFBConfig config)
	{
		memset(this, 0, sizeof(FBConfigData));

		glXGetFBConfigAttrib(dpy, config, GLX_FBCONFIG_ID, &configID);
		glXGetFBConfigAttrib(dpy, config, GLX_VISUAL_ID, &visualID);
		glXGetFBConfigAttrib(dpy, config, GLX_BUFFER_SIZE, &bufferSize);
		glXGetFBConfigAttrib(dpy, config, GLX_LEVEL, &level);
		glXGetFBConfigAttrib(dpy, config, GLX_DOUBLEBUFFER, &doubleBuffer);
		glXGetFBConfigAttrib(dpy, config, GLX_STEREO, &stereo);
		glXGetFBConfigAttrib(dpy, config, GLX_AUX_BUFFERS, &auxBuffers);
		glXGetFBConfigAttrib(dpy, config, GLX_RENDER_TYPE, &renderType);
		glXGetFBConfigAttrib(dpy, config, GLX_RED_SIZE, &redSize);
		glXGetFBConfigAttrib(dpy, config, GLX_GREEN_SIZE, &greenSize);
		glXGetFBConfigAttrib(dpy, config, GLX_BLUE_SIZE, &blueSize);
		glXGetFBConfigAttrib(dpy, config, GLX_ALPHA_SIZE, &alphaSize);
		glXGetFBConfigAttrib(dpy, config, GLX_DEPTH_SIZE, &depthSize);
		glXGetFBConfigAttrib(dpy, config, GLX_STENCIL_SIZE, &stencilSize);
		glXGetFBConfigAttrib(dpy, config, GLX_ACCUM_RED_SIZE, &accumRedSize);
		glXGetFBConfigAttrib(dpy, config, GLX_ACCUM_GREEN_SIZE, &accumGreenSize);
		glXGetFBConfigAttrib(dpy, config, GLX_ACCUM_BLUE_SIZE, &accumBlueSize);
		glXGetFBConfigAttrib(dpy, config, GLX_ACCUM_ALPHA_SIZE, &accumAlphaSize);
		glXGetFBConfigAttrib(dpy, config, GLX_DRAWABLE_TYPE, &drawableType);
		glXGetFBConfigAttrib(dpy, config, GLX_CONFIG_CAVEAT, &caveat);
		glXGetFBConfigAttrib(dpy, config, GLX_MAX_PBUFFER_WIDTH, &maxPBufferWidth);
		glXGetFBConfigAttrib(dpy, config, GLX_MAX_PBUFFER_HEIGHT, &maxPBufferHeight);
		glXGetFBConfigAttrib(dpy, config, GLX_MAX_PBUFFER_PIXELS, &maxPBufferPixels);
	}
	String FBConfigData::toString() const
	{
		std::stringstream ss;
		ss << "configID=" << configID;
		ss << " visualID=" << visualID;
		ss << " bufferSize=" << bufferSize;
		ss << " level=" << level;
		ss << " doubleBuffer=" << doubleBuffer;
		ss << " stereo=" << stereo;
		ss << " auxBuffers=" << auxBuffers;
		ss << " renderType=" << renderType;
		ss << " redSize=" << redSize;
		ss << " greenSize=" << greenSize;
		ss << " blueSize=" << blueSize;
		ss << " alphaSize=" << alphaSize;
		ss << " depthSize=" << depthSize;
		ss << " stencilSize=" << stencilSize;
		ss << " accumRedSize=" << accumRedSize;
		ss << " accumGreenSize=" << accumGreenSize;
		ss << " accumBlueSize=" << accumBlueSize;
		ss << " accumAlphaSize=" << accumAlphaSize;
		ss << " drawableType=" << drawableType;
		ss << " caveat=" << caveat;
		ss << " maxPBufferWidth=" << maxPBufferWidth;
		ss << " maxPBufferHeight=" << maxPBufferHeight;
		ss << " maxPBufferPixels=" << maxPBufferPixels;
		return ss.str();
	}

	bool GLXUtils::LoadIcon(Display *mDisplay, Window rootWindow, const std::string &name, Pixmap *pix, Pixmap *mask)
	{
	 Image img;
	 int mWidth, mHeight;
	   char *data, *bitmap;
	   try {
		  // Try to load image
		   img.load(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		 mWidth = img.getWidth();
		   mHeight = img.getHeight();
		 if(img.getFormat() != PF_A8R8G8B8)
			 // Image format must be RGBA
			   return 0;
	  } catch(Exception &e) {
			// Could not find image; never mind
			return false;
	  }

	 // Allocate space for image data
	   data = (char*)malloc(mWidth * mHeight * 4); // Must be allocated with malloc
	   // Allocate space for transparency bitmap
	  int wbits = (mWidth+7)/8;
	  bitmap = (char*)malloc(wbits * mHeight);

	  // Convert and copy image
	  const char *imgdata = (const char*)img.getData();
	  int sptr = 0, dptr = 0;
		for(int y=0; y<mHeight; y++) {
		 for(int x=0; x<mWidth; x++) {
			  data[dptr + 0] = 0;
				data[dptr + 1] = imgdata[sptr + 0];
				data[dptr + 2] = imgdata[sptr + 1];
				data[dptr + 3] = imgdata[sptr + 2];
				// Alpha threshold
			 if(((unsigned char)imgdata[sptr + 3])<128) {
				   bitmap[y*wbits+(x>>3)] &= ~(1<<(x&7));
			 } else {
				   bitmap[y*wbits+(x>>3)] |= 1<<(x&7);
				}
			  sptr += 4;
			 dptr += 4;
		 }
	  }

	 /* put my pixmap data into the client side X image data structure */
	   XImage *image = XCreateImage (mDisplay, NULL, 24, ZPixmap, 0,
									data,
									  mWidth, mHeight, 8,
									mWidth*4);
	   image->byte_order = MSBFirst; // 0RGB format

	  /* tell server to start managing my pixmap */
	  Pixmap retval = XCreatePixmap(mDisplay, rootWindow, mWidth,
									  mHeight, 24);

	   /* copy from client to server */
	   GC context = XCreateGC (mDisplay, rootWindow, 0, NULL);
		XPutImage(mDisplay, retval, context, image, 0, 0, 0, 0,
				  mWidth, mHeight);

	   /* free up the client side pixmap data area */
	 XDestroyImage(image); // also cleans data
	  XFreeGC(mDisplay, context);

	   *pix = retval;
	 *mask = XCreateBitmapFromData(mDisplay, rootWindow, bitmap, mWidth, mHeight);
	  free(bitmap);
	  return true;
	}



	static void
	get_visual_attribs(Display *dpy, XVisualInfo *vInfo,
					   struct visual_attribs *attribs)
	{
	 const char *ext = glXQueryExtensionsString(dpy, vInfo->screen);

	   memset(attribs, 0, sizeof(struct visual_attribs));

		attribs->id = vInfo->visualid;
	#if defined(__cplusplus) || defined(c_plusplus)

		attribs->klass = vInfo->c_class;
	#else

		attribs->klass = vInfo->class;
	#endif

	 attribs->depth = vInfo->depth;
	 attribs->redMask = vInfo->red_mask;
		attribs->greenMask = vInfo->green_mask;
		attribs->blueMask = vInfo->blue_mask;
	  attribs->colormapSize = vInfo->colormap_size;
	  attribs->bitsPerRGB = vInfo->bits_per_rgb;

		if (glXGetConfig(dpy, vInfo, GLX_USE_GL, &attribs->supportsGL) != 0)
		   return;
		glXGetConfig(dpy, vInfo, GLX_BUFFER_SIZE, &attribs->bufferSize);
	   glXGetConfig(dpy, vInfo, GLX_LEVEL, &attribs->level);
	  glXGetConfig(dpy, vInfo, GLX_RGBA, &attribs->rgba);
		glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &attribs->doubleBuffer);
		glXGetConfig(dpy, vInfo, GLX_STEREO, &attribs->stereo);
		glXGetConfig(dpy, vInfo, GLX_AUX_BUFFERS, &attribs->auxBuffers);
	   glXGetConfig(dpy, vInfo, GLX_RED_SIZE, &attribs->redSize);
	 glXGetConfig(dpy, vInfo, GLX_GREEN_SIZE, &attribs->greenSize);
	 glXGetConfig(dpy, vInfo, GLX_BLUE_SIZE, &attribs->blueSize);
	   glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &attribs->alphaSize);
	 glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &attribs->depthSize);
	 glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &attribs->stencilSize);
	 glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &attribs->accumRedSize);
	  glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &attribs->accumGreenSize);
	  glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &attribs->accumBlueSize);
		glXGetConfig(dpy, vInfo, GLX_ACCUM_ALPHA_SIZE, &attribs->accumAlphaSize);

	 /* get transparent pixel stuff */
	  glXGetConfig(dpy, vInfo,GLX_TRANSPARENT_TYPE, &attribs->transparentType);
	  if (attribs->transparentType == GLX_TRANSPARENT_RGB) {
		 glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_RED_VALUE, &attribs->transparentRedValue);
			glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_GREEN_VALUE, &attribs->transparentGreenValue);
			glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_BLUE_VALUE, &attribs->transparentBlueValue);
		  glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_ALPHA_VALUE, &attribs->transparentAlphaValue);
		} else if (attribs->transparentType == GLX_TRANSPARENT_INDEX) {
			glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_INDEX_VALUE, &attribs->transparentIndexValue);
		}

	 /* multisample attribs */
	   if (ext && strstr("GLX_ARB_multisample", ext) == 0) {
		  	glXGetConfig(dpy, vInfo, GLX_SAMPLE_BUFFERS_ARB, &attribs->numMultisample);
			glXGetConfig(dpy, vInfo, GLX_SAMPLES_ARB, &attribs->numSamples);
	   }
	   else {
		 attribs->numSamples = 0;
		   attribs->numMultisample = 0;
	   }

	  if (ext && strstr(ext, "GLX_EXT_visual_rating")) {
		 glXGetConfig(dpy, vInfo, GLX_VISUAL_CAVEAT_EXT, &attribs->visualCaveat);
	   } else {
		   attribs->visualCaveat = GLX_NONE_EXT;
	  }
	}

	/*
	 * Examine all visuals to find the so-called best one.
	 * We prefer deepest RGBA buffer with depth, stencil and accum
	 * that has no caveats.
	 * @author Brian Paul (from the glxinfo source)
	 */
	int GLXUtils::findBestVisual(Display *dpy, int scrnum, int multiSample)
	{
		XVisualInfo theTemplate;
	   XVisualInfo *visuals;
	  int numVisuals;
		long mask;
	 int i;
	 struct visual_attribs bestVis;
	 int msDiff;

		/* get list of all visuals on this screen */
	   theTemplate.screen = scrnum;
	   mask = VisualScreenMask;
	   visuals = XGetVisualInfo(dpy, mask, &theTemplate, &numVisuals);
		if(numVisuals == 0 || visuals == 0) {
		  /* something went wrong */
		 if(visuals)
				XFree(visuals);
			return -1;
	 }

	 /* init bestVis with first visual info */
	  get_visual_attribs(dpy, &visuals[0], &bestVis);

	   /* try to find a "better" visual */
		for (i = 1; i < numVisuals; i++) {
		 struct visual_attribs vis;

			get_visual_attribs(dpy, &visuals[i], &vis);

		   	/* always skip visuals that are slow */
		 	if (vis.visualCaveat == GLX_SLOW_VISUAL_EXT)
		 		continue;
		 	/* skip visual if it doesn't have the desired number of multisamples */
			if (multiSample != -1 && vis.numSamples != multiSample)
			   continue;

		 /* see if this vis is better than bestVis */

		   if ((!bestVis.supportsGL && vis.supportsGL) ||
						 (bestVis.visualCaveat != GLX_NONE_EXT) ||
						  (!bestVis.rgba && vis.rgba) ||
						 (!bestVis.doubleBuffer && vis.doubleBuffer) ||
						 (bestVis.redSize < vis.redSize) ||
						 (bestVis.greenSize < vis.greenSize) ||
						 (bestVis.blueSize < vis.blueSize) ||
						   (bestVis.alphaSize < vis.alphaSize) ||
						 (bestVis.depthSize < vis.depthSize) ||
						 (bestVis.stencilSize < vis.stencilSize) ||
						 (bestVis.accumRedSize < vis.accumRedSize))
			{
			   /* found a better visual */
				bestVis = vis;
			}

	  }

	 XFree(visuals);
		if (multiSample != -1 && bestVis.numSamples != multiSample)
			// We found no visual with the desired FSAA
			return -1;
	   return bestVis.id;
	}

	/**
	 * Class to sort FBConfig records by best match
	 */
	class FBConfigMatchSort
	{
	public:
		FBConfigMatchSort(Display *dpy, const int *idealattribs): dpy(dpy), mIdeal(idealattribs) { }

		Display *dpy;
		const int *mIdeal;
		bool operator()(const GLXFBConfig & a, const GLXFBConfig & b)
		{
			for(int idx = 0; mIdeal[idx] != None; idx += 2)
			{
				int valuea, valueb;
				int valueideal = mIdeal[idx+1];
				glXGetFBConfigAttrib(dpy, a, mIdeal[idx], &valuea);
				glXGetFBConfigAttrib(dpy, b, mIdeal[idx], &valueb);
				// Distance to a smaller than distance to b?
				if(std::abs(valuea - valueideal) < std::abs(valueb - valueideal))
					return true;
			}
			return false;
		}
	};


	GLXFBConfig GLXUtils::findBestMatch(Display *dpy, int scrnum, const int *attribs, const int *idealattribs)
	{
		// Create vector of existing config data formats
		GLXFBConfig * fbConfigs;
		int nConfigs;
		fbConfigs = glXChooseFBConfig(dpy, scrnum, attribs, &nConfigs);
		if (nConfigs == 0 || !fbConfigs)
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "glXChooseFBConfig() failed: Couldn't find a suitable pixel format", "GLRenderTexture::createPBuffer");

		// Sort by best match
		std::sort(fbConfigs, fbConfigs+nConfigs, FBConfigMatchSort(dpy, idealattribs));

		GLXFBConfig retval = fbConfigs[0];
		XFree(fbConfigs);

		return retval;
	}

};
