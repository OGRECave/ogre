#ifndef _VS10INST_H
#define _VS10INST_H

#define VS10_ADD        1
#define VS10_DP3        2
#define VS10_DP4        3
#define VS10_DST        4
#define VS10_EXP        5
#define VS10_EXPP       6
#define VS10_FRC        7
#define VS10_LIT        8
#define VS10_LOG        9
#define VS10_LOGP       10
#define VS10_M3X2       11
#define VS10_M3X3       12
#define VS10_M3X4       13
#define VS10_M4X3       14
#define VS10_M4X4       15
#define VS10_MAD        16
#define VS10_MAX        17
#define VS10_MIN        18
#define VS10_MOV        19
#define VS10_MUL        20
#define VS10_NOP        21
#define VS10_RCP        22
#define VS10_RSQ        23
#define VS10_SGE        24
#define VS10_SLT        25
#define VS10_SUB        26
#define VS10_COMMENT    27
#define VS10_HEADER     28

#define TYPE_TEMPORARY_REG          1
#define TYPE_VERTEX_ATTRIB_REG      2
#define TYPE_ADDRESS_REG            3
#define TYPE_CONSTANT_MEM_REG       4
#define TYPE_CONSTANT_A0_REG        5
#define TYPE_CONSTANT_A0_OFFSET_REG 6
#define TYPE_POSITION_RESULT_REG    7
#define TYPE_COLOR_RESULT_REG       8
#define TYPE_TEXTURE_RESULT_REG     9
#define TYPE_FOG_RESULT_REG         10
#define TYPE_POINTS_RESULT_REG      11

class VS10Reg {
public:
//    VS10Reg();
//    VS10Reg(const VS10Reg &r);
//	VS10Reg& operator=(const VS10Reg &r);
	void Init();
    void Translate();
    int type;
    int index;
	int sign;
    char mask[4];

    int ValidateIndex();
};

typedef class VS10Inst {
public:
	~VS10Inst();
	VS10Inst();
	VS10Inst(int currline);
	VS10Inst(const VS10Inst &inst);
    VS10Inst& operator=(const VS10Inst &inst);
	VS10Inst(int currline, int inst);
	VS10Inst(int currline, int inst, char *cmt);
	VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0);
	VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0, VS10Reg src1);
	VS10Inst(int currline, int inst, VS10Reg dreg, VS10Reg src0, VS10Reg src1, VS10Reg src2);
    void Validate( int &vsflag );
	int Translate();
	VS10Reg dst;
    VS10Reg src[3];
private:
    int line;
    int instid;
    char *comment;
    void ValidateRegIndices();
    void ValidateDestMask();
    void ValidateSrcMasks();
    void ValidateDestWritable();
    void ValidateSrcReadable();
    void ValidateReadPorts();

} *VS10InstPtr;

#endif
