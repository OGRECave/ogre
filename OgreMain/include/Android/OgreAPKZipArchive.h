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

#ifndef __APKZipArchive_H__
#define __APKZipArchive_H__

#include <OgreZip.h>
#include <OgreLogManager.h>
#include <android/asset_manager.h>


namespace Ogre{
	class APKZipArchiveFactory : public EmbeddedZipArchiveFactory
	{
	protected:
		AAssetManager* mAssetMgr;
	public:
		APKZipArchiveFactory(AAssetManager* assetMgr) : mAssetMgr(assetMgr) {}
		virtual ~APKZipArchiveFactory() {}

		/// @copydoc FactoryObj::getType
		const String& getType(void) const;

		/// @copydoc FactoryObj::createInstance
		Archive *createInstance( const String& name, bool readOnly )
		{
			String apkName = name;
	        if (apkName.size() > 0 && apkName[0] == '/')
	        	apkName.erase(apkName.begin());

			AAsset* asset = AAssetManager_open(mAssetMgr, apkName.c_str(), AASSET_MODE_BUFFER);
			if(asset)
			{
				EmbeddedZipArchiveFactory::addEmbbeddedFile(apkName, (const Ogre::uint8*)AAsset_getBuffer(asset), AAsset_getLength(asset), 0);
			}

            ZipArchive * resZipArchive = OGRE_NEW ZipArchive(apkName, "APKZip", mPluginIo);
            return resZipArchive;
		}
	};
}

#endif
