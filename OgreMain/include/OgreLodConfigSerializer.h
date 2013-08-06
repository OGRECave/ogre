#include <OgreSerializer.h>
#include <OgreLodConfig.h>
#include <fstream>
#include <OgreDataStream.h>
#include <OgreLogManager.h>
#include <OgreMeshManager.h>
#include <OgreLodStrategyManager.h>
namespace Ogre
{

class LodConfigSerializer :
	public Ogre::Serializer
{

public:

	LodConfigSerializer();

	void exportLodConfig(Ogre::LodConfig& config, const Ogre::String& filename,
		Endian endianMode = ENDIAN_NATIVE);
	void exportLodConfig(Ogre::LodConfig& config, Ogre::DataStreamPtr stream,
		Endian endianMode = ENDIAN_NATIVE);

	void importLodConfig(Ogre::LodConfig* config, const Ogre::String& filename);
	void importLodConfig(Ogre::LodConfig* config, DataStreamPtr& stream);

protected:

	enum LodConfigChunkID {
		LCCID_FILE_HEADER = 0x300,
		LCCID_LOD_CONFIG = 0x400,
		LCCID_BASIC_INFO = 0x500,
		LCCID_LOD_LEVELS = 0x600,
		LCCID_ADVANCED_INFO = 0x700,
		LCCID_PROFILE = 0x800,
	};

	void cleanup();

	void readLodConfig();
	void readLodBasicInfo();
	void readLodLevels();
	void readLodAdvancedInfo();
	void readLodProfile();

	void writeLodConfig();
	size_t calcLodConfigSize();
	void writeLodBasicInfo();
	size_t calcLodBasicInfoSize();
	void writeLodLevels();
	size_t calcLodLevelsSize();
	void writeLodAdvancedInfo();
	size_t calcLodAdvancedInfoSize();
	void writeLodProfile();
	size_t calcLodProfileSize();
	
	LodConfig* mLodConfig;
};
}
