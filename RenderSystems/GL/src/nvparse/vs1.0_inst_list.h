#ifndef _VS10_H
#define _VS10_H

#define WRITEMASK_X		0x01
#define WRITEMASK_Y		0x02
#define WRITEMASK_Z		0x04
#define WRITEMASK_W		0x08

#include "vs1.0_inst.h"

typedef class VS10InstList {
public:
	VS10InstList();
	~VS10InstList();
	int Size();
	VS10InstList& operator+=(VS10InstPtr t);
	void Validate();
	void Translate();
private:
    VS10InstPtr list;
    int size;
    int max;
} *VS10InstListPtr;

#endif
