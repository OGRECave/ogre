#ifndef __ACP_FILE_H__
#define __ACP_FILE_H__

#include "ACPChunk.h"
#include <vector>
#include <map>

class ACPFile
{
private:
	std::vector<ACPChunk*> mChunks;
public:
	ACPFile();
	~ACPFile();

	ACPChunk *createChunk();
	void removeChunk(size_t i);
	size_t findChunk(const std::string &name) const;
	ACPChunk *getChunk(size_t i) const;
	size_t getChunkCount() const;
	void clearChunks();
	
	void load(const char *path);
	void load(void *buffer, size_t size);
	void save(const char *path);

	std::map<std::string,size_t> loadHeader(const char *path);
	std::map<std::string,size_t> loadHeader(void *buffer, size_t size);

	static const size_t INVALID_INDEX;
};

#endif
