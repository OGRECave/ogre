/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __CompositorWorkspaceDef_H__
#define __CompositorWorkspaceDef_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreTextureDefinition.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	/** TODO: Describe!!!
	@author
		Matias N. Goldberg
    @version
        1.0
    */
	class _OgreExport CompositorWorkspaceDef : public TextureDefinitionBase
	{
	protected:
		struct ChannelRoute
		{
			uint32		outChannel;
			IdString	outNode;
			uint32		inChannel;
			IdString	inNode;
			ChannelRoute( uint32 _outChannel, IdString _outNode, uint32 _inChannel, IdString _inNode ) :
						outChannel( _outChannel ), outNode( _outNode ),
						inChannel( _inChannel ), inNode( _inNode ) {}
		};

		IdString			mName;

		typedef map<IdString, IdString>::type	NodeAliasMap;
		typedef list<ChannelRoute>::type		ChannelRouteList;

		NodeAliasMap		mNodeAlias;
		ChannelRouteList	mChannelRoutes;

	public:
		CompositorWorkspaceDef( IdString name ) :
			TextureDefinitionBase( TEXTURE_GLOBAL ), mName( name ) {}

		void connect( uint32 outChannel, IdString outNode, uint32 inChannel, IdString inNode );
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
