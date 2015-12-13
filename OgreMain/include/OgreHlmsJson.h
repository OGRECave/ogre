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
#ifndef _OgreHlmsJson_H_
#define _OgreHlmsJson_H_

#if !OGRE_NO_JSON

#include "OgreHlmsCommon.h"
#include "OgreHlmsDatablock.h"
#include "OgreHlmsSamplerblock.h"
#include "OgreLwConstString.h"
#include "OgreHeaderPrefix.h"

// Forward declaration for |Document|.
namespace rapidjson
{
    class CrtAllocator;
    template <typename> class MemoryPoolAllocator;
    template <typename> struct UTF8;
    //template <typename, typename, typename> class GenericDocument;
    //typedef GenericDocument< UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator > Document;

    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename>  class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */
    /** HLMS stands for "High Level Material System". */
    class _OgreExport HlmsJson : public PassAlloc
    {
    public:
        struct NamedBlocks
        {
            map<LwConstString, const HlmsMacroblock*>::type macroblocks;
            map<LwConstString, const HlmsBlendblock*>::type blendblocks;
            map<LwConstString, const HlmsSamplerblock*>::type samplerblocks;
        };

    protected:
        HlmsManager *mHlmsManager;

        static FilterOptions parseFilterOptions( const char *value );
        static TextureAddressingMode parseTextureAddressingMode( const char *value );
        static CompareFunction parseCompareFunction( const char *value );
        static CullingMode parseCullMode( const char *value );
        static PolygonMode parsePolygonMode( const char *value );
        static SceneBlendFactor parseBlendFactor( const char *value );
        static SceneBlendOperation parseBlendOperation( const char *value );

        static void loadSampler( const rapidjson::Value &samplers, HlmsSamplerblock &samplerblock );
        static void loadMacroblock( const rapidjson::Value &macroblocksJson, HlmsMacroblock &macroblock );
        static void loadBlendblock( const rapidjson::Value &blendblocksJson, HlmsBlendblock &blendblock );
        static void loadDatablockCommon( const rapidjson::Value &json, const NamedBlocks &blocks,
                                         HlmsDatablock *datablock );

        void loadDatablocks( const rapidjson::Value &json, const NamedBlocks &blocks, Hlms *hlms );

    public:
        HlmsJson( HlmsManager *hlmsManager );
        ~HlmsJson();

        void loadMaterials( const String &filename, const char *jsonString );
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

#endif
