#ifndef _LWENVELOPE_H_
#define _LWENVELOPE_H_

#include "lwo.h"

#include <iostream>

class lwKey
{
public:
	lwKey(float ntime, float nvalue) : time(ntime), value(nvalue) {}
	float          time;
	float          value;
	unsigned int   shape;               /* ID_TCB, ID_BEZ2, etc. */
	float          tension;
	float          continuity;
	float          bias;
	float          param[ 4 ];
};

typedef vector<lwKey*> vkeys;

inline bool operator < (const lwKey &k1, const lwKey &k2)
{
	return k1.time < k2.time;
}

#define BEH_RESET      0
#define BEH_CONSTANT   1
#define BEH_REPEAT     2
#define BEH_OSCILLATE  3
#define BEH_OFFSET     4
#define BEH_LINEAR     5

class lwEnvelope
{
	float range( float v, float lo, float hi, int *i );
	void hermite( float t, float *h1, float *h2, float *h3, float *h4 );
	float bezier( float x0, float x1, float x2, float x3, float t );
	float bez2_time( float x0, float x1, float x2, float x3, float time, float *t0, float *t1 );
	float bez2( lwKey *key0, lwKey *key1, float time );
	float outgoing( unsigned int key0, unsigned int key1 );
	float incoming( unsigned int key0, unsigned int key1 );
	
public:
	lwEnvelope()
	{
		name = 0;
	}

	~lwEnvelope()
	{
		if (name) free(name);
		unsigned int i;
		for (i=0; i<keys.size(); delete keys[i++]);
		for (i=0; i<cfilters.size(); delete cfilters[i++]);
	}
	
	float evaluate( float time );
	lwKey *addKey( float time, float value );

	int       index;
	int       type;
	char     *name;
	vkeys     keys;                /* linked list of keys */
	int       behavior[ 2 ];       /* pre and post (extrapolation) */
	vplugins  cfilters;            /* linked list of channel filters */
	int       ncfilters;
};

typedef vector<lwEnvelope*> venvelopes;

#endif // _LWENVELOPE_H_

