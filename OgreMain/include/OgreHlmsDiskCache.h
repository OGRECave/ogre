/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2018 Torus Knot Software Ltd

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
#ifndef _OgreHlmsDiskCache_H_
#define _OgreHlmsDiskCache_H_

#include "OgreHlms.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** @class HlmsDiskCache
    */
    class _OgreExport HlmsDiskCache : public HlmsAlloc
    {
    public:
        struct SourceCode
        {
            Hlms::RenderableCache   mergedCache;
            String                  sourceFile[NumShaderTypes];

            SourceCode();
            SourceCode( const Hlms::ShaderCodeCache &shaderCodeCache );
        };

        typedef vector<SourceCode>::type SourceCodeVec;

        struct Pso
        {
            Hlms::RenderableCache   renderableCache;
            HlmsPropertyVec         passProperties;
            HlmsPso                 pso;
            HlmsMacroblock          macroblock;
            HlmsBlendblock          blendblock;

            Pso();
            Pso( const Hlms::RenderableCache &srcRenderableCache, const Hlms::PassCache &srcPassCache,
                 const HlmsCache *srcPsoCache );
        };

        typedef vector<Pso>::type PsoVec;

        struct Cache
        {
            uint64          templateHash[2]; //128 bit hash
            uint8           type;           /// See HlmsTypes
            SourceCodeVec   sourceCode;
            PsoVec          pso;
        };

        bool        mTemplatesOutOfDate;
        Cache       mCache;
        HlmsManager *mHlmsManager;

        void save( DataStreamPtr &dataStream, const String &string );
        void save( DataStreamPtr &dataStream, const HlmsPropertyVec &properties );
        void save( DataStreamPtr &dataStream, const Hlms::RenderableCache &renderableCache );

        void load( DataStreamPtr &dataStream, String &string );
        void load( DataStreamPtr &dataStream, HlmsPropertyVec &properties );
        void load( DataStreamPtr &dataStream, Hlms::RenderableCache &renderableCache );

    public:
        HlmsDiskCache();
        ~HlmsDiskCache();

        void clearCache(void);

        void copyFrom( Hlms *hlms );
        void applyTo( Hlms *hlms );

        void saveTo( DataStreamPtr &dataStream );
        void loadFrom( DataStreamPtr &dataStream );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
