#ifndef _RC10_GENERAL_H
#define _RC10_GENERAL_H

#include "rc1.0_register.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

enum {
	RCP_MUL = 0,
	RCP_DOT,
	RCP_MUX,
	RCP_SUM
};

class ConstColorStruct {
public:
	void Init(RegisterEnum _reg, float _v0, float _v1, float _v2, float _v3)
	{ reg = _reg; v[0] = _v0; v[1] = _v1; v[2] = _v2; v[3] = _v3; }
	RegisterEnum reg;
	float v[4];
};

class OpStruct {
public:
	void Init(int _op, RegisterEnum _reg0, MappedRegisterStruct _reg1, MappedRegisterStruct _reg2)
	{ op = _op; reg[0].reg = _reg0; reg[1] = _reg1; reg[2] = _reg2; }
	void Init(int _op, RegisterEnum _reg0)
	{ op = _op; reg[0].reg = _reg0; }
	int op;
	MappedRegisterStruct reg[3];
	void Validate(int stage, int portion);
};

class GeneralFunctionStruct {
public:
	void Init(OpStruct _op0, OpStruct _op1, OpStruct _op2) { op[0] = _op0; op[1] = _op1; op[2] = _op2; numOps = 3; }
	void Init(OpStruct _op0, OpStruct _op1) { op[0] = _op0; op[1] = _op1; numOps = 2; }
	void Init(OpStruct _op0) { op[0] = _op0; numOps = 1; }
	void Validate(int stage, int portion);
	void Invoke(int stage, int portion, BiasScaleEnum bs);
	void ZeroOut();
	int numOps;
	OpStruct op[3];
};


class GeneralPortionStruct {
public:
	void Init(int _designator, GeneralFunctionStruct _gf, BiasScaleEnum _bs)
	{ designator = _designator; gf = _gf; bs = _bs; }

	void Validate(int stage);
	void Invoke(int stage);
	void ZeroOut();
	int designator;
	GeneralFunctionStruct gf;
	BiasScaleEnum bs;
};

class GeneralCombinerStruct {
public:
	void Init(GeneralPortionStruct _portion0, GeneralPortionStruct _portion1, ConstColorStruct _cc0, ConstColorStruct _cc1)
	{ portion[0] = _portion0; portion[1] = _portion1; numPortions = 2; cc[0] = _cc0; cc[1] = _cc1; numConsts = 2; }
	void Init(GeneralPortionStruct _portion0, GeneralPortionStruct _portion1, ConstColorStruct _cc0)
	{ portion[0] = _portion0; portion[1] = _portion1; numPortions = 2; cc[0] = _cc0; numConsts = 1; }
	void Init(GeneralPortionStruct _portion0, GeneralPortionStruct _portion1)
	{ portion[0] = _portion0; portion[1] = _portion1; numPortions = 2; numConsts = 0; }

	void Init(GeneralPortionStruct _portion0, ConstColorStruct _cc0, ConstColorStruct _cc1)
	{ portion[0] = _portion0; numPortions = 1; cc[0] = _cc0; cc[1] = _cc1; numConsts = 2; }
	void Init(GeneralPortionStruct _portion0, ConstColorStruct _cc0)
	{ portion[0] = _portion0; numPortions = 1; cc[0] = _cc0; numConsts = 1; }
	void Init(GeneralPortionStruct _portion0)
	{ portion[0] = _portion0; numPortions = 1; numConsts = 0; }

	void Validate(int stage);
	void SetUnusedLocalConsts(int numGlobalConsts, ConstColorStruct *globalCCs);
	void Invoke(int stage);
	void ZeroOut();
	GeneralPortionStruct portion[2];
	int numPortions;
	ConstColorStruct cc[2];
	int numConsts;
};

class GeneralCombinersStruct {
public:
	void Init() {num = 0;}
	void Init(GeneralCombinerStruct _gc) { num = 1; general[0] = _gc; }
	GeneralCombinersStruct& operator+=(GeneralCombinerStruct& _gc)
	{
		if (num < RCP_NUM_GENERAL_COMBINERS)
			general[num++] = _gc;
		else
			errors.set("Too many general combiners.");
		return *this;
	}
	void Validate(int numConsts, ConstColorStruct *cc);
	void Invoke();
	GeneralCombinerStruct general[RCP_NUM_GENERAL_COMBINERS];
	int num;
private:
	int localConsts;
};


#endif
