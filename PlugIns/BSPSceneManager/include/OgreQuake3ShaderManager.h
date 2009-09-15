/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __QUAKE3SHADERMANAGER_H__
#define __QUAKE3SHADERMANAGER_H__

#include "OgreBspPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreResourceManager.h"
#include "OgreQuake3Shader.h"
#include "OgreBlendMode.h"

namespace Ogre {


    /** Class for managing Quake3 custom shaders.
        Quake3 uses .shader files to define custom shaders, or Materials in Ogre-speak.
        When a surface texture is mentioned in a level file, it includes no file extension
        meaning that it can either be a standard texture image (+lightmap) if there is only a .jpg or .tga
        file, or it may refer to a custom shader if a shader with that name is included in one of the .shader
        files in the scripts/ folder. Because there are multiple shaders per file you have to parse all the
        .shader files available to know if there is a custom shader available. This class is designed to parse
        all the .shader files available and save their settings for future use. </p>
        I choose not to set up Material instances for shaders found since they may or may not be used by a level,
        so it would be very wasteful to set up Materials since they load texture images for each layer (apart from the
        lightmap). Once the usage of a shader is confirmed, a full Material instance can be set up from it.</p>
        Because this is a subclass of ScriptLoader, any files mentioned will be searched for in any path or
        archive added to the ResourceGroupManager::WORLD_GROUP_NAME group. See ResourceGroupManager for details.
    */
    class Quake3ShaderManager : public ScriptLoader, public Singleton<Quake3ShaderManager>, public ResourceAlloc
    {
    protected:
        void parseNewShaderPass(DataStreamPtr& stream, Quake3Shader* pShader);
        void parseShaderAttrib( const String& line, Quake3Shader* pShader);
        void parseShaderPassAttrib( const String& line, Quake3Shader* pShader, Quake3Shader::Pass* pPass);
        SceneBlendFactor convertBlendFunc( const String& q3func);

		typedef map<String, Quake3Shader*>::type Quake3ShaderMap;
        Quake3ShaderMap mShaderMap;
        StringVector mScriptPatterns;


    public:
        Quake3ShaderManager();
        virtual ~Quake3ShaderManager();

        /** @copydoc ScriptLoader::getScriptPatterns */
        const StringVector& getScriptPatterns(void) const;

        /** @copydoc ScriptLoader::parseScript */
        void parseScript(DataStreamPtr& stream, const String& groupName);

        /** @copydoc ScriptLoader::parseScript */
        Real getLoadingOrder(void) const;

        /** Create implementation. */
        Quake3Shader* create(const String& name);
        /** Clear all the current shaders */
        void clear(void);
        /** Retrieve a Quake3Shader by name */
        Quake3Shader* getByName(const String& name);

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static Quake3ShaderManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static Quake3ShaderManager* getSingletonPtr(void);


    };

}

#endif
