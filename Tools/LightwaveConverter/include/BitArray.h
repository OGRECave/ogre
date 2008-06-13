#ifndef _BITARRAY_H_
#define _BITARRAY_H_

class BitArray
{
private:
/** The number of bits in this array
	*/
	unsigned long arraysize;
	/** The number of unsigned longs for storing at least arraysize bits
	*/
	unsigned long bitlongs;
	/** The array of unsigned longs containing the bits
	*/
	unsigned long *bits;
public:
/** Constructors.
    */
	BitArray(unsigned long newsize);
	BitArray(const BitArray& b);
	
	BitArray(unsigned long newsize, bool setclear);
	BitArray(unsigned long newsize, unsigned long *newbits);
	
    /** Destructor.
    */
	~BitArray();
	
	BitArray& operator =(const BitArray& b);
	BitArray operator ~(void);
	BitArray& operator ^=(const BitArray& b);
	BitArray& operator &=(const BitArray& b);
	BitArray& operator |=(const BitArray& b);
	BitArray operator ^(const BitArray& b);
	BitArray operator &(const BitArray& b);
	BitArray operator |(const BitArray& b);
	
	/** Test to see if a single bit is set.
	*/
	inline bool bitSet(unsigned long index) const
	{
		return bits[(index>>5)] >> (index & 0x0000001f) & 0x00000001;
	}
	/** Clear all bits in this array.
	*/
	inline void clear(void)
	{
		fillBitArray(0x00000000);
	}
	/** Clear a single bit.
	*/
	inline void clearBit(unsigned long index)
	{
		bits[index >> 5] &= ~(0x00000001 << (index & 0x0000001f));
	}
	/** fill with a 32-bit pattern.
	*/
	inline void fillBitArray(unsigned long pattern)
	{
		for (unsigned long i=0; i < bitlongs; bits[i++]=pattern); 
	}

	/** flip a single bit.
	*/
	inline void flipBit(unsigned long index)
	{
		if (bitSet(index))
			clearBit(index);
		else
			setBit(index);
	};

	/** Returns index of next set bit in array (wraps around)
    */
	inline long getNextSet(unsigned long index)
	{
		unsigned long i;
		for (i=index+1;i<arraysize;i++) if (bitSet(i)) return i;
		for (i=0;i<index-1;i++) if (bitSet(i)) return i;
		return -1;
	}
	
	/** Returns index of previous set bit in array (wraps around)
    */
	inline long getPreviousSet(unsigned long index)
	{
		unsigned long i;
		if (index != 0)
		{
			for (i=index-1;i>0;i--) if (bitSet(i)) return i;
			if (bitSet(0)) return 0;
		}
		for (i=arraysize-1;i>index;i--) if (bitSet(i)) return i;
		return -1;
	}
	
	/** Set all bits in this array.
	*/
	inline void set(void)
	{
		fillBitArray(0xffffffff);
	}
	
	/** Set a single bit.
	*/
	inline void setBit(unsigned long index)
	{
		bits[index >> 5] |= 0x00000001 << (index & 0x0000001f);
	}
	
	/** return the number of bits in this bit array..
	*/
	inline unsigned long size(void)
	{
		return arraysize;
	}
};

#endif // _BITARRAY_H_

