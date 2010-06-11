#ifndef __ACP_CHUNK_H__
#define __ACP_CHUNK_H__

#include <string>

class ACPChunk
{
private:
	void *mBuffer; // The data stored
	bool mCompressed;
	size_t mSize, mFullSize;
	std::string mName;
public:
	ACPChunk();
	~ACPChunk();

	void load(const char *name, const char *path);
	void load(const char *name, void *buffer, size_t size);
	void loadCompressed(const char *name, void *buffer, size_t size);

	void compress();
	void uncompress();

	bool getCompressed() const;
	size_t getSize() const;
	size_t getFullSize() const;
	const std::string &getName() const;
	void *getBuffer() const;
};

#endif
