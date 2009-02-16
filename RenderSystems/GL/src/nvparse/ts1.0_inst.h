#ifndef _INST_H
#define _INST_H

#define TSP_MAX_ARGS 7
#define TSP_NUM_TEXTURE_UNITS 4
#include <OgreGLPrerequisites.h>
#ifdef _WIN32
# define BYTE_ORDER !BIG_ENDIAN
#endif
#include <stdlib.h>  

typedef union _InstructionEnum {
  struct {
#if BYTE_ORDER != BIG_ENDIAN
    unsigned int instruction   :10; // instruction id
    unsigned int stage         : 4; // stage number
    unsigned int dependent     : 1; // dependent operation
    unsigned int noOutput      : 1; // no RGBA output
#else
    unsigned int noOutput      : 1;
    unsigned int dependent     : 1;
    unsigned int stage         : 4;
    unsigned int instruction   :10;
#endif
  } bits;
  unsigned int word;
} InstructionEnum;

// WARNING:  Don't monkey with the above structure or this macro
// unless you're absolutely sure of what you're doing!
// This constant allocation makes validation *much* cleaner.
#define TSP_SET_INSTRUCTION_ENUM(inst, st, dep, noout) \
	 ((noout << 15) | (dep << 14) | (st << 10) | inst)

#define TSP_NOP					TSP_SET_INSTRUCTION_ENUM(0, 0, 0, 1)
#define TSP_TEXTURE_1D			TSP_SET_INSTRUCTION_ENUM(1, 0, 0, 0)
#define TSP_TEXTURE_2D			TSP_SET_INSTRUCTION_ENUM(2, 0, 0, 0)
#define TSP_TEXTURE_RECTANGLE	TSP_SET_INSTRUCTION_ENUM(3, 0, 0, 0)
#define TSP_TEXTURE_3D			TSP_SET_INSTRUCTION_ENUM(4, 0, 0, 0)
#define TSP_TEXTURE_CUBE_MAP	TSP_SET_INSTRUCTION_ENUM(5, 0, 0, 0)
#define TSP_CULL_FRAGMENT		TSP_SET_INSTRUCTION_ENUM(6, 0, 0, 1)
#define TSP_PASS_THROUGH		TSP_SET_INSTRUCTION_ENUM(7, 0, 0, 0)
#define TSP_DEPENDENT_AR		TSP_SET_INSTRUCTION_ENUM(8, 0, 1, 0)
#define TSP_DEPENDENT_GB		TSP_SET_INSTRUCTION_ENUM(9, 0, 1, 0)
#define TSP_OFFSET_2D			TSP_SET_INSTRUCTION_ENUM(10, 0, 1, 0)
#define TSP_OFFSET_2D_SCALE		TSP_SET_INSTRUCTION_ENUM(11, 0, 1, 0)
#define TSP_OFFSET_RECTANGLE			TSP_SET_INSTRUCTION_ENUM(12, 0, 1, 0)
#define TSP_OFFSET_RECTANGLE_SCALE		TSP_SET_INSTRUCTION_ENUM(13, 0, 1, 0)

#define TSP_DOT_PRODUCT_2D_1_OF_2				TSP_SET_INSTRUCTION_ENUM(14, 0, 1, 1)
#define TSP_DOT_PRODUCT_2D_2_OF_2				TSP_SET_INSTRUCTION_ENUM(14, 1, 1, 0)

#define TSP_DOT_PRODUCT_RECTANGLE_1_OF_2				TSP_SET_INSTRUCTION_ENUM(15, 0, 1, 1)
#define TSP_DOT_PRODUCT_RECTANGLE_2_OF_2				TSP_SET_INSTRUCTION_ENUM(15, 1, 1, 0)

#define TSP_DOT_PRODUCT_DEPTH_REPLACE_1_OF_2	TSP_SET_INSTRUCTION_ENUM(16, 0, 1, 1)
#define TSP_DOT_PRODUCT_DEPTH_REPLACE_2_OF_2	TSP_SET_INSTRUCTION_ENUM(16, 1, 1, 0)

#define TSP_DOT_PRODUCT_3D_1_OF_3		TSP_SET_INSTRUCTION_ENUM(17, 0, 1, 1)
#define TSP_DOT_PRODUCT_3D_2_OF_3		TSP_SET_INSTRUCTION_ENUM(17, 1, 1, 1)
#define TSP_DOT_PRODUCT_3D_3_OF_3		TSP_SET_INSTRUCTION_ENUM(17, 2, 1, 0)

#define TSP_DOT_PRODUCT_CUBE_MAP_1_OF_3	TSP_SET_INSTRUCTION_ENUM(18, 0, 1, 1)
#define TSP_DOT_PRODUCT_CUBE_MAP_2_OF_3	TSP_SET_INSTRUCTION_ENUM(18, 1, 1, 1)
#define TSP_DOT_PRODUCT_CUBE_MAP_3_OF_3	TSP_SET_INSTRUCTION_ENUM(18, 2, 1, 0)

#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3	TSP_SET_INSTRUCTION_ENUM(19, 0, 1, 1)
#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3	TSP_SET_INSTRUCTION_ENUM(19, 1, 1, 1)
#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3	TSP_SET_INSTRUCTION_ENUM(19, 2, 1, 0)

#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3	TSP_SET_INSTRUCTION_ENUM(20, 0, 1, 1)
#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3	TSP_SET_INSTRUCTION_ENUM(20, 1, 1, 1)
#define TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3	TSP_SET_INSTRUCTION_ENUM(20, 2, 1, 0)

#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3	TSP_SET_INSTRUCTION_ENUM(21, 0, 1, 1)
#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3	TSP_SET_INSTRUCTION_ENUM(21, 1, 1, 0)
#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3	TSP_SET_INSTRUCTION_ENUM(21, 2, 1, 0)

#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3		TSP_SET_INSTRUCTION_ENUM(22, 0, 1, 1)
#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3		TSP_SET_INSTRUCTION_ENUM(22, 1, 1, 0)
#define TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3		TSP_SET_INSTRUCTION_ENUM(22, 2, 1, 0)

typedef struct _MappedVariable {
	float var;
	int expand;
} MappedVariable, *MappedVariablePtr;

typedef class Inst {
public:
	Inst(int inst, float arg0 = 0., float arg1 = 0., float arg2 = 0., float arg3 = 0., float arg4 = 0., float arg5 = 0., float arg6 = 0.);
	Inst(int inst, MappedVariablePtr arg0, float arg1 = 0., float arg2 = 0., float arg3 = 0., float arg4 = 0., float arg5 = 0., float arg6 = 0.);
	void Invoke();
	InstructionEnum opcode;
	float args[TSP_MAX_ARGS];
private:
	int expand;
} *InstPtr;

#ifdef TEST_BIT_FIELDS

#include <stdio.h>

class InstructionEnumTest {
  public:
    InstructionEnumTest()
    {
      InstructionEnum inst;
      bool error = false;

      if (sizeof(inst.bits) != sizeof(inst.word))
        error = true;

      inst.word = 0; inst.bits.instruction = 0x3FF;
      if (TSP_SET_INSTRUCTION_ENUM(0x3FF, 0, 0, 0) != inst.word)
        error = true;

      inst.word = 0; inst.bits.stage = 0x0F;
      if (TSP_SET_INSTRUCTION_ENUM(0, 0x0F, 0, 0) != inst.word) 
        error = true;

      inst.word = 0; inst.bits.dependent = true;
      if (TSP_SET_INSTRUCTION_ENUM(0, 0, 1, 0) != inst.word) 
        error = true;

      inst.word = 0; inst.bits.noOutput = true;
      if (TSP_SET_INSTRUCTION_ENUM(0, 0, 0, 1) != inst.word) 
        error = true;

      if (error) {
	fprintf(stderr, "ERROR: Bit Fields were not compiled correctly in " __FILE__ "!\n");
	exit(1);
      }
    }
};

static InstructionEnumTest instructionEnumTest;

#endif /* TEST_BIT_FIELDS */

#endif
