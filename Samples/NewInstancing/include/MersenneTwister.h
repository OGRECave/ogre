#ifndef __MERSENNETWISTER_H__
#define __MERSENNETWISTER_H__

//Random number generator. Written by FrozenKnight
//Taken from http://www.cpplc.net/forum/index.php?topic=2862.0

#define MTSZ 624
#define MTSZSP 397

class MersenneTwister {
private:
	int seed[MTSZ];
	int index;
 
	void generate()
	{
		for (int i = 0; i < MTSZ; i++) 
		{
			int y = (((seed[i] << 31) & 0x80000000) + (seed[(i+1) % MTSZ] & 0x7FFFFFFF)) >> 1;
			seed[i] = seed[(i + MTSZSP) % MTSZ] ^ y;
			seed[i] ^= (y&1)? 0x9908b0df: 0;
		}
	} 
public:
	MersenneTwister()
	{
		index = 0;
		randomize();
	}

	void randomize(int seedVal = 0x12345678)
	{
		seed[0] = seedVal;
		for (int i = 1; i < MTSZ; ++i)
			seed[i] = (int)(0x6c078965*((this->seed[i-1] >> 30)+i));
	}

	unsigned int nextUInt()
	{
		unsigned int ret;
		if (index == 0)
			generate();
		ret = seed[index];

		ret ^= ret >> 11;
		ret ^= (ret << 7) & 0x9d2c5680;
		ret ^= (ret << 15) & 0xefc60000;
		ret ^= ret >> 18;

		index = (++index < MTSZ)? index: 0;

		return ret;
	}

	float nextFloat()
	{
		return (nextUInt() / (float)0xFFFFFFFF);
	}
};
  #endif
