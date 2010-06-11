#include "AndroidArchive.h"

#include <OgreStringConverter.h>
#include <OgreLogManager.h>

#include "acpwrapper.h"

#include <android/log.h>

#define  LOG_TAG    "AndroidArchive"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

namespace Ogre{

	AndroidArchive::AndroidArchive(const String& name, const String& archType, JNIEnv *env)
		:Archive(name, archType), mBuffer(0), mEnv(env), mBufferSize(0)
	{
	}

	AndroidArchive::~AndroidArchive()
	{
		unload();
	}

	bool AndroidArchive::isCaseSensitive() const
	{
		return false;
	}

	void AndroidArchive::load()
	{
		if(acp_has_file(mEnv, mName.c_str()) != 0)
		{
			LogManager::getSingleton().logMessage(mName + " package found");
			
			int size = 0;
			if(acp_get_file(mEnv, mName.c_str(), &mBuffer, &size) == 0)
			{
				LogManager::getSingleton().logMessage(mName + " package loaded");
				mBufferSize = size;
				
				mFile.load(mBuffer, mBufferSize);
			}
		}
	}

	void AndroidArchive::unload()
	{
		if(mBuffer)
		{
			OGRE_FREE(mBuffer, MEMCATEGORY_GENERAL);
			mBuffer = 0;
		}
	}

	DataStreamPtr AndroidArchive::open(const Ogre::String &filename, bool readOnly) const
	{
		DataStreamPtr stream;
		
		size_t i = mFile.findChunk(filename);
		if(i != ACPFile::INVALID_INDEX)
		{
			ACPChunk *chunk = mFile.getChunk(i);
			if(chunk->getCompressed())
				chunk->uncompress();
			
			stream = DataStreamPtr(new MemoryDataStream(chunk->getBuffer(), chunk->getSize(), false, readOnly));
		}
		
		return stream;
	}

	DataStreamPtr AndroidArchive::create(const Ogre::String &filename) const
	{
		return DataStreamPtr();
	}

	void AndroidArchive::remove(const String &filename) const
	{

	}

	StringVectorPtr AndroidArchive::list(bool recursive, bool dirs)
	{
		StringVectorPtr files(new StringVector);

		for(size_t i = 0; i < mFile.getChunkCount(); ++i)
			files->push_back(mFile.getChunk(i)->getName());

		return files;
	}

	FileInfoListPtr AndroidArchive::listFileInfo(bool recursive, bool dirs)
	{
		FileInfoListPtr files(new FileInfoList);

		for(size_t i = 0; i < mFile.getChunkCount(); ++i)
		{
			ACPChunk *chunk = mFile.getChunk(i);
			
			FileInfo info;
			info.archive = this;
			info.filename = chunk->getName();
			info.path = chunk->getName();
			info.basename = chunk->getName();
			info.compressedSize = chunk->getSize();
			info.uncompressedSize = chunk->getFullSize();
			files->push_back(info);
		}

		return files;
	}

	StringVectorPtr AndroidArchive::find(const String& pattern, bool recursive, bool dirs)
	{
		StringVectorPtr files(new StringVector);

		for(size_t i = 0; i < mFile.getChunkCount(); ++i)
		{
			ACPChunk *chunk = mFile.getChunk(i);
			
			if(StringUtil::match(chunk->getName(), pattern))
				files->push_back(chunk->getName());
		}

		return files;
	}

	FileInfoListPtr AndroidArchive::findFileInfo(const String& pattern, bool recursive, bool dirs)
	{
		FileInfoListPtr files(new FileInfoList);

		
		for(size_t i = 0; i < mFile.getChunkCount(); ++i)
		{
			ACPChunk *chunk = mFile.getChunk(i);
			if(StringUtil::match(chunk->getName(), pattern))
			{				
				FileInfo info;
				info.archive = this;
				info.basename = info.filename = info.path = chunk->getName();
				info.compressedSize = chunk->getSize();
				info.uncompressedSize = chunk->getFullSize();
				files->push_back(info);
			}
		}

		return files;
	}

	bool AndroidArchive::exists(const String& filename)
	{
		for(size_t i = 0; i < mFile.getChunkCount(); ++i)
		{
			ACPChunk *chunk = mFile.getChunk(i);
			if(chunk->getName() == filename)
				return true;
		}
		return false;
	}

	time_t AndroidArchive::getModifiedTime(const Ogre::String &filename)
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////

	const String &AndroidArchiveFactory::getType() const
	{
		static String type = "Android";
		return type;
	}

}