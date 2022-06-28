#include "OgreStableHeaders.h"

#include <android/asset_manager.h>

namespace Ogre{
namespace {
    class APKFileSystemArchive : public Archive
    {
    private:
        AAssetManager* mAssetMgr;
        String mPathPreFix;

    public:
        APKFileSystemArchive(const String& name, const String& archType, AAssetManager* assetMgr);
        ~APKFileSystemArchive();

        /// @copydoc Archive::isCaseSensitive
        bool isCaseSensitive(void) const;

        /// @copydoc Archive::load
        void load();
        /// @copydoc Archive::unload
        void unload();

        /// @copydoc Archive::open
        DataStreamPtr open(const String& filename, bool readOnly = true) const;

        /// @copydoc Archive::create
        DataStreamPtr create(const String& filename);

        /// @copydoc Archive::remove
        void remove(const String& filename);

        /// @copydoc Archive::list
        StringVectorPtr list(bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::listFileInfo
        FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::find
        StringVectorPtr find(const String& pattern, bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::findFileInfo
        FileInfoListPtr findFileInfo(const String& pattern, bool recursive = true, bool dirs = false) const;

        /// @copydoc Archive::exists
        bool exists(const String& filename) const;

        /// @copydoc Archive::getModifiedTime
        time_t getModifiedTime(const String& filename) const;
    };

	std::map<String, std::vector< String > > mFiles;

	bool IsFolderParsed( const String& Folder ) {
		bool parsed = false;
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( Folder );
		if(iter != mFiles.end()) parsed = true;
		return parsed;
	}

	void ParseFolder( AAssetManager* AssetMgr, const String& Folder ) {
		std::vector<String> mFilenames;
		AAssetDir* dir = AAssetManager_openDir(AssetMgr, Folder.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL) {
			mFilenames.push_back( String( fileName ) );
		}
		mFiles.insert( std::make_pair( Folder, mFilenames ) );
	}
}
	APKFileSystemArchive::APKFileSystemArchive(const String& name, const String& archType, AAssetManager* assetMgr)
		:Archive(name, archType), mAssetMgr(assetMgr)
	{
		if (mName.size() > 0 && mName[0] == '/')
			mName.erase(mName.begin());

		mPathPreFix = mName;
		if (mPathPreFix.size() > 0)
			mPathPreFix += "/";
			
		if(!IsFolderParsed( mName )) {
			ParseFolder( mAssetMgr, mName );
		}			
	}

	APKFileSystemArchive::~APKFileSystemArchive()
	{
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( mName );
		if (iter != mFiles.end()) {
			iter->second.clear();
			mFiles.erase( iter );
		}
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
	    MemoryDataStreamPtr stream;
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_BUFFER);
		if(asset)
		{
			off_t length = AAsset_getLength(asset);
            stream = std::make_shared<MemoryDataStream>(filename, length, true, true);
			memcpy(stream->getPtr(), AAsset_getBuffer(asset), length);
			AAsset_close(asset);
		}
		return stream;
	}

	DataStreamPtr APKFileSystemArchive::create(const Ogre::String &filename)
	{
		return DataStreamPtr();
	}

	void APKFileSystemArchive::remove(const String &filename)
	{

	}

	StringVectorPtr APKFileSystemArchive::list(bool recursive, bool dirs) const
	{
		StringVectorPtr files(new StringVector);
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( mName );
		std::vector< String > fileList = iter->second;
		for( size_t i = 0; i < fileList.size(); i++ )
		{
			files->push_back(fileList[i]);
		}
		return files;
	}

	FileInfoListPtr APKFileSystemArchive::listFileInfo(bool recursive, bool dirs) const
	{
		FileInfoListPtr files(new FileInfoList);
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( mName );
		std::vector< String > fileList = iter->second;
		for( size_t i = 0; i < fileList.size(); i++ )
		{
			AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + fileList[i]).c_str(), AASSET_MODE_UNKNOWN);
			if(asset)
			{
				FileInfo info;
				info.archive = this;
				info.filename = fileList[i];
				info.path = mName;
				info.basename = fileList[i];
				info.compressedSize = AAsset_getLength(asset);
				info.uncompressedSize = info.compressedSize;
				files->push_back(info);
				AAsset_close(asset);
			}
		}
		return files;
	}

	StringVectorPtr APKFileSystemArchive::find(const String& pattern, bool recursive, bool dirs) const
	{
		StringVectorPtr files(new StringVector);
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( mName );
		std::vector< String > fileList = iter->second;
		for( size_t i = 0; i < fileList.size(); i++ ) 
		{
			if(StringUtil::match(fileList[i], pattern))
				files->push_back(fileList[i]);
		}
		return files;
	}

	FileInfoListPtr APKFileSystemArchive::findFileInfo(const String& pattern, bool recursive, bool dirs) const
	{
		FileInfoListPtr files(new FileInfoList);
		std::map<String, std::vector< String > >::iterator iter = mFiles.find( mName );
		std::vector< String > fileList = iter->second;
		for( size_t i = 0; i < fileList.size(); i++ ) 
		{
			if(StringUtil::match(fileList[i], pattern)) 
			{
				AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + fileList[i]).c_str(), AASSET_MODE_UNKNOWN);
				if(asset) {
					FileInfo info;
					info.archive = this;
					info.filename = fileList[i];
					info.path = mName;
					info.basename = fileList[i];
					info.compressedSize = AAsset_getLength(asset);
					info.uncompressedSize = info.compressedSize;
					files->push_back(info);
					AAsset_close(asset);
				}
			}
		}
		return files;
	}

	bool APKFileSystemArchive::exists(const String& filename) const
	{
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_UNKNOWN);
		if(asset)
		{
			AAsset_close(asset);
			return true;
		}
		return false;
	}

	time_t APKFileSystemArchive::getModifiedTime(const Ogre::String &filename) const
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////

	const String &APKFileSystemArchiveFactory::getType() const
	{
		static String type = "APKFileSystem";
		return type;
	}

    Archive *APKFileSystemArchiveFactory::createInstance( const String& name, bool readOnly )
    {
        return OGRE_NEW APKFileSystemArchive(name, getType(), mAssetMgr);
    }
}
