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

    class _OgreExport HlmsJsonListener
    {
    public:
        /// Gives you a chance to completely change the name of the texture when saving a material
        virtual void savingChangeTextureName( String &inOutTexName ) {}
    };

    /** HLMS stands for "High Level Material System". */
    class _OgreExport HlmsJson : public HlmsAlloc
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
        HlmsJsonListener    *mListener;

    public:
        static FilterOptions parseFilterOptions( const char *value );
        static TextureAddressingMode parseTextureAddressingMode( const char *value );
        static CompareFunction parseCompareFunction( const char *value );
        static CullingMode parseCullMode( const char *value );
        static PolygonMode parsePolygonMode( const char *value );
        static SceneBlendFactor parseBlendFactor( const char *value );
        static SceneBlendOperation parseBlendOperation( const char *value );

    protected:
        static void loadSampler( const rapidjson::Value &samplers, HlmsSamplerblock &samplerblock );
        static void loadMacroblock( const rapidjson::Value &macroblocksJson,
                                    HlmsMacroblock &macroblock );
        static void loadBlendblock( const rapidjson::Value &blendblocksJson,
                                    HlmsBlendblock &blendblock );
        static void loadDatablockCommon( const rapidjson::Value &json, const NamedBlocks &blocks,
                                         HlmsDatablock *datablock );

        void loadDatablocks( const rapidjson::Value &json, const NamedBlocks &blocks, Hlms *hlms,
                             const String &filename, const String &resourceGroup,
                             const String &additionalTextureExtension );

    public:
        static void toQuotedStr( FilterOptions value, String &outString );
        static void toQuotedStr( TextureAddressingMode value, String &outString );
        static void toQuotedStr( CompareFunction value, String &outString );
        static void toQuotedStr( CullingMode value, String &outString );
        static void toQuotedStr( PolygonMode value, String &outString );
        static void toQuotedStr( SceneBlendFactor value, String &outString );
        static void toQuotedStr( SceneBlendOperation value, String &outString );
        static void toStr( const ColourValue &value, String &outString );
        static void toStr( const Vector2 &value, String &outString );
        static void toStr( const Vector3 &value, String &outString );
        static void toStr( const Vector4 &value, String &outString );

        String getName( const HlmsMacroblock *macroblock ) const;
        String getName( const HlmsBlendblock *blendblock ) const;
        static String getName( const HlmsSamplerblock *samplerblock );

    protected:
        void saveSamplerblock( const HlmsSamplerblock *samplerblock, String &outString );
        void saveMacroblock( const HlmsMacroblock *macroblock, String &outString );
        void saveBlendblock( const HlmsBlendblock *blendblock, String &outString );
        void saveDatablock( const String &fullName, const HlmsDatablock *datablock, String &outString,
                            const String &additionalTextureExtension );

    public:
        HlmsJson( HlmsManager *hlmsManager, HlmsJsonListener *listener );
        ~HlmsJson();

        /** Loads all Hlms datablocks from a JSON formatted string.
        @remarks
            Will throw ERR_INVALIDPARAMS if JSON is invalid.
        @param filename
            Name of the file. It's only used for providing additional
            info on the log about where it is failing.
        @param resourceGroup
            See filename argument.
        @param jsonString
            Null-terminated C string (UTF8) containing
            valid JSON with the Hlms definitions.
        @param additionalTextureExtension
            Additional string to append to the texture files while loading. e.g.
            if texture to load is "mytex.png" and additionalTextureExtension = ".dds"
            then the actual texture loaded will be "mytex.png.dds"
            Leave it blank if you don't know what to put
        */
        void loadMaterials( const String &filename, const String &resourceGroup,
                            const char *jsonString,
                            const String &additionalTextureExtension );

        /** Saves all the Datablocks defined in the given
            Hlms into a JSON formatted string.
        @param hlms
            Hlms from whose materials to save.
        @param outString [out]
            String with valid JSON output. String is appended to existing contents.
        @param additionalTextureExtension
            Additional string to append to the texture files while saving. e.g.
            if texture name is "mytex.png" and additionalTextureExtension = ".dds"
            then the actual texture saved will be "mytex.png.dds"
            Leave it blank if you don't know what to put
        */
        void saveMaterials( const Hlms *hlms, String &outString,
                            const String &additionalTextureExtension );

        /** Saves a single datablock to a string
        @param datablock
            Material to save.
        @param outString [out]
            String with valid JSON output. String is appended to existing contents.
        @param additionalTextureExtension
            Additional string to append to the texture files while saving. e.g.
            if texture name is "mytex.png" and additionalTextureExtension = ".dds"
            then the actual texture saved will be "mytex.png.dds"
            Leave it blank if you don't know what to put
        */
        void saveMaterial( const HlmsDatablock *datablock, String &outString,
                           const String &additionalTextureExtension );
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

#endif
