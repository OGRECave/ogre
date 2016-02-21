#ifndef _RC10_COMBINERS_H
#define _RC10_COMBINERS_H

#include "rc1.0_general.h"
#include "rc1.0_final.h"

class CombinersStruct {
public:
    void Init(GeneralCombinersStruct _gcs, FinalCombinerStruct _fc, ConstColorStruct _cc0, ConstColorStruct _cc1)
    { generals = _gcs; final = _fc; cc[0] = _cc0; cc[1] = _cc1; numConsts = 2;}
    void Init(GeneralCombinersStruct _gcs, FinalCombinerStruct _fc, ConstColorStruct _cc0)
    { generals = _gcs; final = _fc; cc[0] = _cc0; numConsts = 1;}
    void Init(GeneralCombinersStruct _gcs, FinalCombinerStruct _fc)
    { generals = _gcs; final = _fc; numConsts = 0;}
    void Validate();
    void Invoke();
private:
    GeneralCombinersStruct generals;
    FinalCombinerStruct final;
    ConstColorStruct cc[2];
    int numConsts;
};

#endif
