#include "BitArray.h"

BitArray::BitArray(unsigned long newsize)
{
	arraysize = newsize;
	bitlongs = ((arraysize - 1) >> 5) + 1;
	bits = new unsigned long[bitlongs];
}

BitArray::BitArray(const BitArray& b)
{
	arraysize = b.arraysize;
	bitlongs = b.bitlongs;
	bits = new unsigned long[bitlongs];
	for (unsigned long i = 0; i < bitlongs; i++)
		bits[i] = b.bits[i];
}

BitArray::BitArray(unsigned long newsize, bool bitvalues)
{
	arraysize = newsize;
	bitlongs = ((arraysize - 1) >> 5) + 1;
	bits = new unsigned long[bitlongs];
    if (bitvalues) set(); else clear();
}

BitArray::BitArray(unsigned long newsize, unsigned long *newbits)
{
	arraysize = newsize;
	bitlongs = ((arraysize - 1) >> 5) + 1;
	bits = new unsigned long[bitlongs];
	for (unsigned long i = 0; i < bitlongs; i++)
		bits[i] = newbits[i];
}

BitArray::~BitArray()
{
	delete []bits;
}

BitArray& BitArray::operator =(const BitArray& b)
{
	bool equalsize = arraysize == b.arraysize;
	
	arraysize = b.arraysize;
	bitlongs = b.bitlongs;
	
	if (!equalsize) {
		delete []bits;
		bits = new unsigned long[bitlongs];
	}
	for (unsigned long i = 0; i < bitlongs; i++)
		bits[i] = b.bits[i];
	
	return (*this);
}

BitArray BitArray::operator ~(void)
{
	BitArray result(arraysize);
	
	for (unsigned long i = 0; i < bitlongs; i++)
		result.bits[i] = ~bits[i];
	
	return (result);
}

BitArray& BitArray::operator ^=(const BitArray& b)
{
	for (unsigned long i = 0; i < ((bitlongs < b.bitlongs) ? bitlongs : b.bitlongs); i++)
		bits[i] ^= b.bits[i];
	return (*this);
}

BitArray& BitArray::operator &=(const BitArray& b)
{
	for (unsigned long i = 0; i < ((bitlongs < b.bitlongs) ? bitlongs : b.bitlongs); i++)
		bits[i] &= b.bits[i];
	return (*this);
}

BitArray& BitArray::operator |=(const BitArray& b)
{
	for (unsigned long i = 0; i < (bitlongs < b.bitlongs ? bitlongs : b.bitlongs); i++)
		bits[i] |= b.bits[i];
	return (*this);
}

BitArray BitArray::operator ^(const BitArray& b)
{
	BitArray result((arraysize < b.arraysize) ? arraysize : b.arraysize);
	
	for (unsigned long i = 0; i < result.bitlongs; i++)
		result.bits[i] = bits[i] ^ b.bits[i];
	return (result);
}

BitArray BitArray::operator &(const BitArray& b)
{
	BitArray result((arraysize < b.arraysize) ? arraysize : b.arraysize);
	
	for (unsigned long i = 0; i < result.bitlongs; i++)
		result.bits[i] = bits[i] & b.bits[i];
	return (result);
}

BitArray BitArray::operator |(const BitArray& b)
{
	BitArray result((arraysize < b.arraysize) ? arraysize : b.arraysize);
	
	for (unsigned long i = 0; i < result.bitlongs; i++)
		result.bits[i] = bits[i] | b.bits[i];
	return (result);
}
