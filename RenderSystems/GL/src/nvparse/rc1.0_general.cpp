#include "rc1.0_general.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"
#include <stdio.h>

void GeneralCombinersStruct::Validate(int numConsts, ConstColorStruct *pcc)
{
    GLint maxGCs;
    glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV, &maxGCs);
    if (num > maxGCs) {
        char buffer[256];
        sprintf(buffer, "%d general combiners specified, only %d supported", num, (int)maxGCs);
        errors.set(buffer);
        num = maxGCs;
    }

    if (0 == num) {
        // Setup a "fake" general combiner 0
        general[0].ZeroOut();
        num = 1;
    }

    localConsts = 0;
    int i;
    for (i = 0; i < num; i++)
        localConsts += general[i].numConsts;

    if (localConsts > 0)
    {
        if (NULL == glCombinerStageParameterfvNV)
            errors.set("local constant(s) specified, but not supported -- ignored");
        else
            for (i = 0; i < num; i++)
                general[i].SetUnusedLocalConsts(numConsts, pcc);
    }

    for (i = 0; i < num; i++)
        general[i].Validate(i);


    for (; i < maxGCs; i++)
        general[i].ZeroOut();
}

void GeneralCombinersStruct::Invoke()
{
    glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, num);
    int i;
    for (i = 0; i < num; i++)
        general[i].Invoke(i);
    
    if (NULL != glCombinerStageParameterfvNV) {
        if (localConsts > 0)
            glEnable(GL_PER_STAGE_CONSTANTS_NV);
        else
            glDisable(GL_PER_STAGE_CONSTANTS_NV);
    }
        
}

void GeneralCombinerStruct::ZeroOut()
{
        numPortions = 2;
        numConsts = 0;

        portion[0].ZeroOut();
        portion[0].designator = RCP_RGB;
        portion[1].ZeroOut();
        portion[1].designator = RCP_ALPHA;
}


void GeneralCombinerStruct::SetUnusedLocalConsts(int numGlobalConsts, ConstColorStruct *globalCCs)
{
        int i;
    for (i = 0; i < numGlobalConsts; i++) {
        bool constUsed = false;
        int j;
        for (j = 0; j < numConsts; j++)
            constUsed |= (cc[j].reg.bits.name == globalCCs[i].reg.bits.name);
        if (!constUsed)
            cc[numConsts++] = globalCCs[i];
    }
}


void GeneralCombinerStruct::Validate(int stage)
{
    if (2 == numConsts &&
        cc[0].reg.bits.name == cc[1].reg.bits.name)
        errors.set("local constant set twice");

    switch (numPortions)
    {
    case 0:
        portion[0].designator = RCP_RGB;
        // Fallthru
    case 1:
        portion[1].designator = ((RCP_RGB == portion[0].designator) ? RCP_ALPHA : RCP_RGB);
        // Fallthru
    case 2:
        if (portion[0].designator == portion[1].designator)
            errors.set("portion declared twice");
        break;
    }
    int i;
    for (i = 0; i < numPortions; i++)
        portion[i].Validate(stage);

    for (; i < 2; i++)
        portion[i].ZeroOut();

}

void GeneralCombinerStruct::Invoke(int stage)
{
    int i;

    if (NULL != glCombinerStageParameterfvNV)
        for (i = 0; i < numConsts; i++)
            glCombinerStageParameterfvNV(GL_COMBINER0_NV + stage, cc[i].reg.bits.name, &(cc[i].v[0]));

    for (i = 0; i < 2; i++)
        portion[i].Invoke(stage);
}

void GeneralPortionStruct::Validate(int stage)
{
    gf.Validate(stage, designator);
}

void GeneralPortionStruct::Invoke(int stage)
{
    gf.Invoke(stage, designator, bs);
}

void GeneralPortionStruct::ZeroOut()
{
    gf.ZeroOut();
    bs.word = RCP_SCALE_BY_ONE;
}

void GeneralFunctionStruct::ZeroOut()
{
    // Create mapped registers for zero and discard
    MappedRegisterStruct unsignedZero;
    RegisterEnum zero;
    zero.word = RCP_ZERO;
    unsignedZero.Init(zero);

    MappedRegisterStruct unsignedDiscard;
    RegisterEnum discard;
    discard.word = RCP_DISCARD;
    unsignedDiscard.Init(discard);

    numOps = 3;

    op[0].op = RCP_MUL;
    op[0].reg[0] = unsignedDiscard;
    op[0].reg[1] = unsignedZero;
    op[0].reg[2] = unsignedZero;

    op[1].op = RCP_MUL;
    op[1].reg[0] = unsignedDiscard;
    op[1].reg[1] = unsignedZero;
    op[1].reg[2] = unsignedZero;

    op[2].op = RCP_SUM;
    op[2].reg[0] = unsignedDiscard;

}

void GeneralFunctionStruct::Validate(int stage, int portion)
{
        int i;
    for (i = 0; i < numOps; i++)
        op[i].Validate(stage, portion);
    // Check if multiple ops are writing to same register (and it's not DISCARD)
    if (numOps > 1 &&
        op[0].reg[0].reg.bits.name == op[1].reg[0].reg.bits.name &&
        GL_DISCARD_NV != op[0].reg[0].reg.bits.name)
        errors.set("writing to same register twice");
    if (numOps > 2 &&
        (op[0].reg[0].reg.bits.name == op[2].reg[0].reg.bits.name ||
         op[1].reg[0].reg.bits.name == op[2].reg[0].reg.bits.name) &&
        GL_DISCARD_NV != op[2].reg[0].reg.bits.name)
        errors.set("writing to same register twice");

    // Set unused outputs to discard, unused inputs to zero/unsigned_identity
    if (numOps < 2) {
        // Set C input to zero
        op[1].reg[1].reg.bits.name = GL_ZERO;
        op[1].reg[1].map = GL_UNSIGNED_IDENTITY_NV;
        op[1].reg[1].reg.bits.channel = portion;

        // Set D input to zero
        op[1].reg[2].reg.bits.name = GL_ZERO;
        op[1].reg[2].map = GL_UNSIGNED_IDENTITY_NV;
        op[1].reg[2].reg.bits.channel = portion;

        // Discard CD output
        op[1].op = false;
        op[1].reg[0].reg.bits.name = GL_DISCARD_NV;
    }

    if (numOps < 3) {
        // Discard muxSum output
        op[2].reg[0].reg.bits.name = GL_DISCARD_NV;
        op[2].op = RCP_SUM;
    }
}


void GeneralFunctionStruct::Invoke(int stage, int portion, BiasScaleEnum bs)
{
    GLenum portionEnum = (RCP_RGB == portion) ? GL_RGB : GL_ALPHA;

    glCombinerInputNV(GL_COMBINER0_NV + stage,
        portionEnum,
        GL_VARIABLE_A_NV,
        op[0].reg[1].reg.bits.name,
        op[0].reg[1].map,
        MAP_CHANNEL(op[0].reg[1].reg.bits.channel));

    glCombinerInputNV(GL_COMBINER0_NV + stage,
        portionEnum,
        GL_VARIABLE_B_NV,
        op[0].reg[2].reg.bits.name,
        op[0].reg[2].map,
        MAP_CHANNEL(op[0].reg[2].reg.bits.channel));

    glCombinerInputNV(GL_COMBINER0_NV + stage,
        portionEnum,
        GL_VARIABLE_C_NV,
        op[1].reg[1].reg.bits.name,
        op[1].reg[1].map,
        MAP_CHANNEL(op[1].reg[1].reg.bits.channel));

    glCombinerInputNV(GL_COMBINER0_NV + stage,
        portionEnum,
        GL_VARIABLE_D_NV,
        op[1].reg[2].reg.bits.name,
        op[1].reg[2].map,
        MAP_CHANNEL(op[1].reg[2].reg.bits.channel));

    glCombinerOutputNV(GL_COMBINER0_NV + stage,
        portionEnum,
        op[0].reg[0].reg.bits.name,
        op[1].reg[0].reg.bits.name,
        op[2].reg[0].reg.bits.name,
        bs.bits.scale,
        bs.bits.bias,
        op[0].op,
        op[1].op,
        (op[2].op == RCP_MUX) ? true : false);
}

// This helper function assigns a channel to an undesignated input register
static void ConvertRegister(RegisterEnum& reg, int portion)
{
    if (RCP_NONE == reg.bits.channel) {
        reg.bits.channel = portion;
        if (GL_FOG == reg.bits.name && RCP_ALPHA == portion)
            // Special case where fog alpha is final only, but RGB is not
            reg.bits.finalOnly = true;
    }
}


void OpStruct::Validate(int stage, int portion)
{
    int args = 1;

    if (RCP_DOT == op || RCP_MUL == op)
        args = 3;
    else
        args = 1;

    if (reg[0].reg.bits.readOnly)
        errors.set("writing to a read-only register");

    if (RCP_ALPHA == portion &&
        RCP_DOT == op)
        errors.set("dot used in alpha portion");
    int i;
    for (i = 0; i < args; i++) {
        ConvertRegister(reg[i].reg, portion);
        if (reg[i].reg.bits.finalOnly)
            errors.set("final register used in general combiner");
        if (RCP_RGB == portion &&
            RCP_BLUE == reg[i].reg.bits.channel)
            errors.set("blue register used in rgb portion");
        if (RCP_ALPHA == portion &&
            RCP_RGB == reg[i].reg.bits.channel)
            errors.set("rgb register used in alpha portion");
        if (i > 0 &&
            GL_DISCARD_NV == reg[i].reg.bits.name)
            errors.set("reading from discard");
    }
}
