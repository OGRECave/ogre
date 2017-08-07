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

#if !OGRE_NO_JSON
#ifndef _OgreHlmsJsonTerra_H_
#define _OgreHlmsJsonTerra_H_

#include "Terra/Hlms/OgreHlmsTerraPrerequisites.h"
#include "Terra/Hlms/OgreHlmsTerraDatablock.h"
#include "OgreHlmsJson.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class HlmsJsonTerra
    {
        HlmsManager *mHlmsManager;

        static TerraBrdf::TerraBrdf parseBrdf( const char *value );
        static void parseOffset( const rapidjson::Value &jsonArray, Vector4 &offsetScale );
        static void parseScale( const rapidjson::Value &jsonArray, Vector4 &offsetScale );

        static inline Vector3 parseVector3Array( const rapidjson::Value &jsonArray );

        void loadTexture( const rapidjson::Value &json, const char *keyName,
                          TerraTextureTypes textureType, HlmsTerraDatablock *datablock,
                          TerraPackedTexture textures[NUM_TERRA_TEXTURE_TYPES] );

        void loadTexture( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                          TerraTextureTypes textureType, HlmsTerraDatablock *datablock,
                          TerraPackedTexture textures[NUM_TERRA_TEXTURE_TYPES] );

        void saveTexture( const char *blockName,
                          TerraTextureTypes textureType,
                          const HlmsTerraDatablock *datablock, String &outString,
                          bool writeTexture=true );
        void saveTexture( const Vector3 &value, const char *blockName,
                          TerraTextureTypes textureType,
                          const HlmsTerraDatablock *datablock, String &outString,
                          bool writeTexture=true );

        void saveTexture( const Vector3 &value, const char *blockName,
                          TerraTextureTypes textureType,
                          bool writeValue, bool writeTexture,
                          const HlmsTerraDatablock *datablock, String &outString );

    public:
        HlmsJsonTerra( HlmsManager *hlmsManager );

        void loadMaterial( const rapidjson::Value &json, const HlmsJson::NamedBlocks &blocks,
                           HlmsDatablock *datablock );
        void saveMaterial( const HlmsDatablock *datablock, String &outString );

        static void collectSamplerblocks( const HlmsDatablock *datablock,
                                          set<const HlmsSamplerblock*>::type &outSamplerblocks );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif

#endif
