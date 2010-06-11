#include "ACPFile.h"

#include <fstream>
#include <locale>
#include <string.h>

#include <OgreStringConverter.h>
#include <OgreLogManager.h>
#include <OgreDataStream.h>
#include <android/log.h>

#define  LOG_TAG    "AndroidArchive"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

const size_t ACPFile::INVALID_INDEX = -1;

ACPFile::ACPFile()
{
	
}

ACPFile::~ACPFile()
{
	for(size_t i = 0; i < mChunks.size(); ++i)
		delete mChunks[i];
}

ACPChunk *ACPFile::createChunk()
{
	ACPChunk *ptr = new ACPChunk;
	mChunks.push_back(ptr);
	return ptr;
}

void ACPFile::removeChunk(size_t i)
{
	mChunks.erase(mChunks.begin() + i);
}

static bool compare_string(const std::string &str1, const std::string &str2)
{
	size_t len = std::max(str1.size(), str2.size());
	for(size_t i = 0; i < len; ++i)
	{
		if(str1[i] != str2[i])
			return false;
	}
	return true;
}

size_t ACPFile::findChunk(const std::string &name) const
{
	for(size_t i = 0; i < mChunks.size(); ++i)
	{
		if(compare_string(name, mChunks[i]->getName()))
			return i;
	}
	return INVALID_INDEX;
}

ACPChunk *ACPFile::getChunk(size_t i) const
{
	ACPChunk *chunk = 0;

	if(i < mChunks.size())
	{
		std::vector<ACPChunk*>::const_iterator iter = mChunks.begin() + i;
		return *iter;
	}

	return chunk;
}

size_t ACPFile::getChunkCount() const
{
	return mChunks.size();
}

void ACPFile::clearChunks()
{
	for(size_t i = 0; i < mChunks.size(); ++i)
		delete mChunks[i];
	mChunks.clear();
}

static size_t read_int(std::fstream &stream)
{
	size_t n = 0;
	stream.read((char*)&n, sizeof(size_t));
	return n;
}

static void write_int(std::fstream &stream, size_t n)
{
	stream.write((char*)&n, sizeof(size_t));
}

static std::string read_string(std::fstream &stream)
{
	size_t n = read_int(stream);

	std::string s;
	char c = 0;
	for(size_t i = 0; i < n; ++i)
	{
		stream.read(&c, 1);
		s += c;
	}
	return s;
}

static void write_string(std::fstream &stream, const std::string &s)
{
	write_int(stream, s.size());
	stream.write((char*)s.c_str(), s.size());
}

static bool read_bool(std::fstream &stream)
{
	bool b = false;
	stream.read((char*)&b, sizeof(bool));
	return b;
}

static void write_bool(std::fstream &stream, bool b)
{
	stream.write((char*)&b, sizeof(bool));
}

void ACPFile::load(const char *path)
{
	clearChunks();
	std::map<std::string,size_t> header = loadHeader(path);

	std::fstream stream(path, std::ios_base::in|std::ios_base::binary);
	if(stream.is_open())
	{
		for(std::map<std::string,size_t>::iterator i = header.begin(); i != header.end(); ++i)
		{
			ACPChunk *chunk = createChunk();

			// Jump to offset
			stream.seekg(i->second, std::ios_base::beg);

			bool compressed = read_bool(stream);
			size_t bufferSize = read_int(stream);
			void *buffer = malloc(bufferSize);
			stream.read((char*)buffer, bufferSize);
			if(compressed)
				chunk->loadCompressed(i->first.c_str(), buffer, bufferSize);
			else
				chunk->load(i->first.c_str(), buffer, bufferSize);
		}
		stream.close();
	}
}

void ACPFile::load(void *buffer, size_t size)
{
	LOGI("Reading header");
	clearChunks();
	std::map<std::string,size_t> header = loadHeader(buffer, size);

	Ogre::DataStreamPtr stream(OGRE_NEW Ogre::MemoryDataStream(buffer, size, false, true));
	for(std::map<std::string,size_t>::iterator i = header.begin(); i != header.end(); ++i)
	{
		LOGI("creating chunk");
		ACPChunk *chunk = createChunk();

		// Jump to offset
		stream->seek(i->second);
		LOGI("Jumped to offset");
		
		if(stream->eof())
			break;

		// Get the compressed state
		bool compressed = false;
		stream->read(&compressed, sizeof(bool));
		if(compressed)
			LOGI("Chunk is compressed");
		else
			LOGI("Chunk is uncompressed");
			
		if(stream->eof())
			break;

		size_t bufferSize = 0;
		stream->read(&bufferSize, sizeof(size_t));
		
		Ogre::String msg = "Buffer size: ";
		msg += Ogre::StringConverter::toString(bufferSize);
		LOGI(msg.c_str());
		
		if(stream->eof())
			break;
			
		void *buffer = malloc(bufferSize);
		stream->read(buffer, bufferSize);

		LOGI("Loading chunk");
		if(compressed)
			chunk->loadCompressed(i->first.c_str(), buffer, bufferSize);
		else
			chunk->load(i->first.c_str(), buffer, bufferSize);
	}
}

void ACPFile::save(const char *path)
{
	std::fstream stream(path, std::ios_base::out|std::ios_base::binary);
	if(stream.is_open())
	{
		// Write out magic chars
		stream.write("ACP", sizeof(char)*3);

		// Calculate the full size of the header
		size_t headerSize = sizeof(char)*3 + sizeof(size_t); // Magic chars + item count
		for(size_t i = 0; i < mChunks.size(); ++i)
		{
			ACPChunk *chunk = mChunks[i];
			// Name size + name chars + offset position
			headerSize = headerSize + sizeof(size_t) + sizeof(char)*chunk->getName().size() + sizeof(size_t);
		}

		// Store offset from data position start
		size_t offset = 0;

		// Create a header table
		write_int(stream, mChunks.size());
		for(size_t i = 0; i < mChunks.size(); ++i)
		{
			ACPChunk *chunk = mChunks[i];

			write_string(stream, chunk->getName());
			write_int(stream, headerSize + offset);

			offset = offset + sizeof(bool) + sizeof(size_t) + chunk->getSize(); // Move forward to next data block
		}

		// Write out data chunks
		for(size_t i = 0; i < mChunks.size(); ++i)
		{
			ACPChunk *chunk = mChunks[i];

			write_bool(stream, chunk->getCompressed());
			write_int(stream, chunk->getSize());
			stream.write((char*)chunk->getBuffer(), chunk->getSize());
		}

		stream.close();
	}
}

std::map<std::string,size_t> ACPFile::loadHeader(const char *path)
{
	std::map<std::string,size_t> header;

	std::fstream stream(path, std::ios_base::in|std::ios_base::binary);
	if(stream.is_open())
	{
		char mc[3];
		stream.read(mc, sizeof(char)*3);

		if(mc[0] == 'A' && mc[1] == 'C' && mc[2] == 'P')
		{
			size_t chunkCount = read_int(stream);
			for(size_t i = 0; i < chunkCount; ++i)
			{
				std::string name = read_string(stream);
				size_t offset = read_int(stream);
				header[name] = offset;
			}
		}
		
		stream.close();
	}

	return header;
}

std::map<std::string,size_t> ACPFile::loadHeader(void *buffer, size_t size)
{
	std::map<std::string,size_t> header;

	Ogre::DataStreamPtr stream(OGRE_NEW Ogre::MemoryDataStream(buffer, size, false, true));
	char mc[3] = "";
	stream->read(mc, sizeof(char)*3);
	if(mc[0] == 'A' && mc[1] == 'C' && mc[2] == 'P')
	{
		LOGI("Magic char check passed");
		
		if(stream->eof())
			return header;
			
		size_t chunkCount = 0;
		stream->read(&chunkCount, sizeof(size_t));
		
		Ogre::String msg = "Got chunk count: ";
		msg += Ogre::StringConverter::toString(chunkCount);
		LOGI(msg.c_str());
		
		char *nameBuf = 0;
		size_t nameBufLen = 0;
		
		for(size_t i = 0; i < chunkCount; ++i)
		{
			if(stream->eof())
				break;
				
			LOGI("Getting name");
			
			size_t nameLen = 0;
			stream->read(&nameLen, sizeof(size_t));
			
			msg = "Got name length: ";
			msg += Ogre::StringConverter::toString(nameLen);
			LOGI(msg.c_str());
			
			if(stream->eof())
				break;
				
			if(nameBufLen < nameLen + 1)
			{
				if(nameBuf)
					free(nameBuf);
				nameBuf = (char*)malloc(nameLen + 1);
				nameBufLen = nameLen + 1;
			}
			stream->read(nameBuf, nameLen);
			nameBuf[nameLen] = 0;
			
			msg = "Got name: ";
			msg += nameBuf;
			LOGI(msg.c_str());

			if(stream->eof())
				break;
				
			size_t offset = 0;
			stream->read(&offset, sizeof(size_t));
			
			msg = "Got offset: ";
			msg += Ogre::StringConverter::toString(offset);
			LOGI(msg.c_str());

			header[nameBuf] = offset;
		}
		
		if(nameBuf)
			free(nameBuf);
	}

	return header;
}

