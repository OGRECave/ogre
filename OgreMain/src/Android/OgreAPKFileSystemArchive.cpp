#include "Android/OgreAPKFileSystemArchive.h"

#include <OgreStringConverter.h>
#include <OgreLogManager.h>

namespace Ogre{

	APKFileSystemArchive::APKFileSystemArchive(const String& name, const String& archType, AAssetManager* assetMgr)
		:Archive(name, archType), mAssetMgr(assetMgr)
	{
        if (mName.size() > 0 && mName[0] == '/')
        	mName.erase(mName.begin());

        mPathPreFix = mName;
        if (mPathPreFix.size() > 0)
        	mPathPreFix += "/";
	}

	APKFileSystemArchive::~APKFileSystemArchive()
	{
		unload();
	}

	bool APKFileSystemArchive::isCaseSensitive() const
	{
		return true;
	}

	void APKFileSystemArchive::load()
	{

	}

	void APKFileSystemArchive::unload()
	{

	}

	DataStreamPtr APKFileSystemArchive::open(const Ogre::String &filename, bool readOnly) const
	{
		DataStreamPtr stream;
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_BUFFER);
		if(asset)
		{
			off_t length = AAsset_getLength(asset);
			void* membuf = OGRE_MALLOC(length, Ogre::MEMCATEGORY_GENERAL);
			memcpy(membuf, AAsset_getBuffer(asset), length);
			AAsset_close(asset);

			stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(membuf, length, true, true));
		}
		return stream;
	}

	DataStreamPtr APKFileSystemArchive::create(const Ogre::String &filename) const
	{
		return DataStreamPtr();
	}

	void APKFileSystemArchive::remove(const String &filename) const
	{

	}

	StringVectorPtr APKFileSystemArchive::list(bool recursive, bool dirs)
	{
		StringVectorPtr files(new StringVector);

		AAssetDir* dir = AAssetManager_openDir(mAssetMgr, mName.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL)
		{
			files->push_back(fileName);
		}
		AAssetDir_close(dir);

		return files;
	}

	FileInfoListPtr APKFileSystemArchive::listFileInfo(bool recursive, bool dirs)
	{
		FileInfoListPtr files(new FileInfoList);

		AAssetDir* dir = AAssetManager_openDir(mAssetMgr, mName.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL)
		{
			AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + String(fileName)).c_str(), AASSET_MODE_UNKNOWN);
			if(asset)
			{
				FileInfo info;
				info.archive = this;
				info.filename = fileName;
				info.path = mName;
				info.basename = fileName;
				info.compressedSize = AAsset_getLength(asset);
				info.uncompressedSize = info.compressedSize;
				files->push_back(info);

				AAsset_close(asset);
			}

		}
		AAssetDir_close(dir);

		return files;
	}

	StringVectorPtr APKFileSystemArchive::find(const String& pattern, bool recursive, bool dirs)
	{
		StringVectorPtr files(new StringVector);

		AAssetDir* dir = AAssetManager_openDir(mAssetMgr, mName.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL)
		{
			if(StringUtil::match(fileName, pattern))
					files->push_back(fileName);
		}
		AAssetDir_close(dir);

		return files;
	}

	FileInfoListPtr APKFileSystemArchive::findFileInfo(const String& pattern, bool recursive, bool dirs) const
	{
		FileInfoListPtr files(new FileInfoList);

		AAssetDir* dir = AAssetManager_openDir(mAssetMgr, mName.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL)
		{
			if(StringUtil::match(fileName, pattern))
			{
				AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + String(fileName)).c_str(), AASSET_MODE_UNKNOWN);
				if(asset)
				{
					FileInfo info;
					info.archive = this;
					info.filename = fileName;
					info.path = mName;
					info.basename = fileName;
					info.compressedSize = AAsset_getLength(asset);
					info.uncompressedSize = info.compressedSize;
					files->push_back(info);

					AAsset_close(asset);
				}
			}
		}
		AAssetDir_close(dir);

		return files;
	}

	bool APKFileSystemArchive::exists(const String& filename)
	{
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_UNKNOWN);
		if(asset)
		{
			AAsset_close(asset);
			return true;
		}
		return false;
	}

	time_t APKFileSystemArchive::getModifiedTime(const Ogre::String &filename)
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////

	const String &APKFileSystemArchiveFactory::getType() const
	{
		static String type = "APKFileSystem";
		return type;
	}

}
