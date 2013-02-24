#ifndef __GLX_glxtokens_h__
#define __GLX_glxtokens_h__

/* $XFree86: xc/include/GL/glxtokens.h,v 1.5 2001/03/21 15:51:38 dawes Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define GLX_VERSION_1_1 1
#define GLX_VERSION_1_2 1
#define GLX_VERSION_1_3 1
#define GLX_VERSION_1_4 1

/*
** Visual Config Attributes (glXGetConfig, glXGetFBConfigAttrib)
*/
#define GLX_USE_GL		1	/* support GLX rendering */
#define GLX_BUFFER_SIZE		2	/* depth of the color buffer */
#define GLX_LEVEL		3	/* level in plane stacking */
#define GLX_RGBA		4	/* true if RGBA mode */
#define GLX_DOUBLEBUFFER	5	/* double buffering supported */
#define GLX_STEREO		6	/* stereo buffering supported */
#define GLX_AUX_BUFFERS		7	/* number of aux buffers */
#define GLX_RED_SIZE		8	/* number of red component bits */
#define GLX_GREEN_SIZE		9	/* number of green component bits */
#define GLX_BLUE_SIZE		10	/* number of blue component bits */
#define GLX_ALPHA_SIZE		11	/* number of alpha component bits */
#define GLX_DEPTH_SIZE		12	/* number of depth bits */
#define GLX_STENCIL_SIZE	13	/* number of stencil bits */
#define GLX_ACCUM_RED_SIZE	14	/* number of red accum bits */
#define GLX_ACCUM_GREEN_SIZE	15	/* number of green accum bits */
#define GLX_ACCUM_BLUE_SIZE	16	/* number of blue accum bits */
#define GLX_ACCUM_ALPHA_SIZE	17	/* number of alpha accum bits */
/*
** FBConfig-specific attributes
*/
#define GLX_X_VISUAL_TYPE		0x22
#define GLX_CONFIG_CAVEAT		0x20	/* Like visual_info VISUAL_CAVEAT_EXT */
#define GLX_TRANSPARENT_TYPE		0x23
#define GLX_TRANSPARENT_INDEX_VALUE	0x24
#define GLX_TRANSPARENT_RED_VALUE	0x25
#define GLX_TRANSPARENT_GREEN_VALUE	0x26
#define GLX_TRANSPARENT_BLUE_VALUE	0x27
#define GLX_TRANSPARENT_ALPHA_VALUE	0x28
#define GLX_DRAWABLE_TYPE		0x8010
#define GLX_RENDER_TYPE			0x8011
#define GLX_X_RENDERABLE		0x8012
#define GLX_FBCONFIG_ID			0x8013
#define GLX_MAX_PBUFFER_WIDTH		0x8016
#define GLX_MAX_PBUFFER_HEIGHT		0x8017
#define GLX_MAX_PBUFFER_PIXELS		0x8018
#define GLX_VISUAL_ID			0x800B

/*
** Error return values from glXGetConfig.  Success is indicated by
** a value of 0.
*/
#define GLX_BAD_SCREEN		1	/* screen # is bad */
#define GLX_BAD_ATTRIBUTE	2	/* attribute to get is bad */
#define GLX_NO_EXTENSION	3	/* no glx extension on server */
#define GLX_BAD_VISUAL		4	/* visual # not known by GLX */
#define GLX_BAD_CONTEXT		5	/* returned only by import_context EXT? */
#define GLX_BAD_VALUE		6	/* returned only by glXSwapIntervalSGI? */
#define GLX_BAD_ENUM		7	/* unused? */

/* FBConfig attribute values */

/*
** Generic "don't care" value for glX ChooseFBConfig attributes (except
** GLX_LEVEL)
*/
#define GLX_DONT_CARE			0xFFFFFFFF

/* GLX_RENDER_TYPE bits */
#define GLX_RGBA_BIT			0x00000001
#define GLX_COLOR_INDEX_BIT		0x00000002

/* GLX_DRAWABLE_TYPE bits */
#define GLX_WINDOW_BIT			0x00000001
#define GLX_PIXMAP_BIT			0x00000002
#define GLX_PBUFFER_BIT			0x00000004

/* GLX_CONFIG_CAVEAT attribute values */
#define GLX_NONE			0x8000
#define GLX_SLOW_CONFIG			0x8001
#define GLX_NON_CONFORMANT_CONFIG	0x800D

/* GLX_X_VISUAL_TYPE attribute values */
#define GLX_TRUE_COLOR			0x8002
#define GLX_DIRECT_COLOR		0x8003
#define GLX_PSEUDO_COLOR		0x8004
#define GLX_STATIC_COLOR		0x8005
#define GLX_GRAY_SCALE			0x8006
#define GLX_STATIC_GRAY			0x8007

/* GLX_TRANSPARENT_TYPE attribute values */
/* #define GLX_NONE			   0x8000 */
#define GLX_TRANSPARENT_RGB		0x8008
#define GLX_TRANSPARENT_INDEX		0x8009

/* glXCreateGLXPbuffer attributes */
#define GLX_PRESERVED_CONTENTS		0x801B
#define GLX_LARGEST_PBUFFER		0x801C
#define GLX_PBUFFER_HEIGHT		0x8040	/* New for GLX 1.3 */
#define GLX_PBUFFER_WIDTH		0x8041	/* New for GLX 1.3 */

/* glXQueryGLXPBuffer attributes */
#define GLX_WIDTH			0x801D
#define GLX_HEIGHT			0x801E
#define GLX_EVENT_MASK			0x801F

/* glXCreateNewContext render_type attribute values */
#define GLX_RGBA_TYPE			0x8014
#define GLX_COLOR_INDEX_TYPE		0x8015

/* glXQueryContext attributes */
/* #define GLX_FBCONFIG_ID		  0x8013 */
/* #define GLX_RENDER_TYPE		  0x8011 */
#define GLX_SCREEN			0x800C

/* glXSelectEvent event mask bits */
#define GLX_PBUFFER_CLOBBER_MASK	0x08000000

/* GLXPbufferClobberEvent event_type values */
#define GLX_DAMAGED			0x8020
#define GLX_SAVED			0x8021

/* GLXPbufferClobberEvent draw_type values */
#define GLX_WINDOW			0x8022
#define GLX_PBUFFER			0x8023

/* GLXPbufferClobberEvent buffer_mask bits */
#define GLX_FRONT_LEFT_BUFFER_BIT	0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT	0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT	0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT	0x00000008
#define GLX_AUX_BUFFERS_BIT		0x00000010
#define GLX_DEPTH_BUFFER_BIT		0x00000020
#define GLX_STENCIL_BUFFER_BIT		0x00000040
#define GLX_ACCUM_BUFFER_BIT		0x00000080

/*
** Extension return values from glXGetConfig.  These are also
** accepted as parameter values for glXChooseVisual.
*/

#define GLX_X_VISUAL_TYPE_EXT	0x22	/* visual_info extension type */
#define GLX_TRANSPARENT_TYPE_EXT 0x23	/* visual_info extension */
#define GLX_TRANSPARENT_INDEX_VALUE_EXT 0x24	/* visual_info extension */
#define GLX_TRANSPARENT_RED_VALUE_EXT	0x25	/* visual_info extension */
#define GLX_TRANSPARENT_GREEN_VALUE_EXT 0x26	/* visual_info extension */
#define GLX_TRANSPARENT_BLUE_VALUE_EXT	0x27	/* visual_info extension */
#define GLX_TRANSPARENT_ALPHA_VALUE_EXT 0x28	/* visual_info extension */

/* Property values for visual_type */
#define GLX_TRUE_COLOR_EXT	0x8002
#define GLX_DIRECT_COLOR_EXT	0x8003
#define GLX_PSEUDO_COLOR_EXT	0x8004
#define GLX_STATIC_COLOR_EXT	0x8005
#define GLX_GRAY_SCALE_EXT	0x8006
#define GLX_STATIC_GRAY_EXT	0x8007

/* Property values for transparent pixel */
#define GLX_NONE_EXT		0x8000
#define GLX_TRANSPARENT_RGB_EXT		0x8008
#define GLX_TRANSPARENT_INDEX_EXT	0x8009

/* Property values for visual_rating */
#define GLX_VISUAL_CAVEAT_EXT		0x20  /* visual_rating extension type */
#define GLX_SLOW_VISUAL_EXT		0x8001
#define GLX_NON_CONFORMANT_VISUAL_EXT	0x800D

/*
** Names for attributes to glXGetClientString.
*/
#define GLX_VENDOR		0x1
#define GLX_VERSION		0x2
#define GLX_EXTENSIONS		0x3

/*
** Names for attributes to glXQueryContextInfoEXT.
*/
#define GLX_SHARE_CONTEXT_EXT	0x800A	/* id of share context */
#define GLX_VISUAL_ID_EXT	0x800B	/* id of context's visual */
#define GLX_SCREEN_EXT		0x800C	/* screen number */

/*
 * GLX 1.4 and later:
 */
#define GLX_SAMPLE_BUFFERS_SGIS            100000
#define GLX_SAMPLES_SGIS                   100001

#ifdef __cplusplus
}
#endif

#endif /* !__GLX_glxtokens_h__ */
