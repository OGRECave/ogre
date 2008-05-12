#ifndef _InstList_h
#define _InstList_h

#include "ts1.0_inst.h"

typedef class InstList {
public:
	InstList();
	~InstList();
	int Size();
	InstList& operator+=(InstPtr t);
	void Validate();
	void Invoke();
private:
    InstPtr list;
    int size;
    int max;
} *InstListPtr;

#endif
