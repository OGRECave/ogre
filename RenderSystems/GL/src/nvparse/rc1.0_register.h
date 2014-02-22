#ifndef _RC10_REGISTER_H
#define _RC10_REGISTER_H

#include <stdlib.h>
#include "OgreGLPrerequisites.h"
#include <GL/glew.h>

#define RCP_NUM_GENERAL_COMBINERS 8

#define RCP_RGB     0
#define RCP_ALPHA   1
#define RCP_BLUE    2
#define RCP_NONE    3

typedef union _RegisterEnum {
  struct {
#if BYTE_ORDER != BIG_ENDIAN
    unsigned int name          :16; // OpenGL enum for register
    unsigned int channel       : 2; // RCP_RGB, RCP_ALPHA, etc
    unsigned int readOnly      : 1; // true or false
    unsigned int finalOnly     : 1; // true or false
    unsigned int unused        :12;
#else
    unsigned int unused        :12;
    unsigned int finalOnly     : 1; // true or false
    unsigned int readOnly      : 1; // true or false
    unsigned int channel       : 2; // RCP_RGB, RCP_ALPHA, RCP_BLUE, RCP_NONE
    unsigned int name          :16; // OpenGL enum for register
#endif
  } bits;
  unsigned int word;
} RegisterEnum;
// No need for writeOnly flag, since DISCARD is the only register in that category

// WARNING:  Don't monkey with the above structure or this macro
// unless you're absolutely sure of what you're doing!
// This constant allocation makes validation *much* cleaner.
#define RCP_SET_REGISTER_ENUM(name, channel, readonly, finalonly) \
     ((finalonly << 19) | (readonly << 18) | (channel << 16) | name)

#define RCP_FOG_RGB                 RCP_SET_REGISTER_ENUM(GL_FOG, RCP_RGB,   1, 0)
#define RCP_FOG_ALPHA               RCP_SET_REGISTER_ENUM(GL_FOG, RCP_ALPHA, 1, 1)
#define RCP_FOG_BLUE                RCP_SET_REGISTER_ENUM(GL_FOG, RCP_BLUE,  1, 0)
#define RCP_FOG                     RCP_SET_REGISTER_ENUM(GL_FOG, RCP_NONE,  1, 0)
#define RCP_PRIMARY_COLOR_RGB       RCP_SET_REGISTER_ENUM(GL_PRIMARY_COLOR_NV, RCP_RGB,   0, 0)
#define RCP_PRIMARY_COLOR_ALPHA     RCP_SET_REGISTER_ENUM(GL_PRIMARY_COLOR_NV, RCP_ALPHA, 0, 0)
#define RCP_PRIMARY_COLOR_BLUE      RCP_SET_REGISTER_ENUM(GL_PRIMARY_COLOR_NV, RCP_BLUE,  0, 0)
#define RCP_PRIMARY_COLOR           RCP_SET_REGISTER_ENUM(GL_PRIMARY_COLOR_NV, RCP_NONE,  0, 0)
#define RCP_SECONDARY_COLOR_RGB     RCP_SET_REGISTER_ENUM(GL_SECONDARY_COLOR_NV, RCP_RGB,   0, 0)
#define RCP_SECONDARY_COLOR_ALPHA   RCP_SET_REGISTER_ENUM(GL_SECONDARY_COLOR_NV, RCP_ALPHA, 0, 0)
#define RCP_SECONDARY_COLOR_BLUE    RCP_SET_REGISTER_ENUM(GL_SECONDARY_COLOR_NV, RCP_BLUE,  0, 0)
#define RCP_SECONDARY_COLOR         RCP_SET_REGISTER_ENUM(GL_SECONDARY_COLOR_NV, RCP_NONE,  0, 0)
#define RCP_SPARE0_RGB              RCP_SET_REGISTER_ENUM(GL_SPARE0_NV, RCP_RGB,   0, 0)
#define RCP_SPARE0_ALPHA            RCP_SET_REGISTER_ENUM(GL_SPARE0_NV, RCP_ALPHA, 0, 0)
#define RCP_SPARE0_BLUE             RCP_SET_REGISTER_ENUM(GL_SPARE0_NV, RCP_BLUE,  0, 0)
#define RCP_SPARE0                  RCP_SET_REGISTER_ENUM(GL_SPARE0_NV, RCP_NONE,  0, 0)
#define RCP_SPARE1_RGB              RCP_SET_REGISTER_ENUM(GL_SPARE1_NV, RCP_RGB,   0, 0)
#define RCP_SPARE1_ALPHA            RCP_SET_REGISTER_ENUM(GL_SPARE1_NV, RCP_ALPHA, 0, 0)
#define RCP_SPARE1_BLUE             RCP_SET_REGISTER_ENUM(GL_SPARE1_NV, RCP_BLUE,  0, 0)
#define RCP_SPARE1                  RCP_SET_REGISTER_ENUM(GL_SPARE1_NV, RCP_NONE,  0, 0)
#define RCP_TEXTURE0_RGB            RCP_SET_REGISTER_ENUM(GL_TEXTURE0_ARB, RCP_RGB,   0, 0)
#define RCP_TEXTURE0_ALPHA          RCP_SET_REGISTER_ENUM(GL_TEXTURE0_ARB, RCP_ALPHA, 0, 0)
#define RCP_TEXTURE0_BLUE           RCP_SET_REGISTER_ENUM(GL_TEXTURE0_ARB, RCP_BLUE,  0, 0)
#define RCP_TEXTURE0                RCP_SET_REGISTER_ENUM(GL_TEXTURE0_ARB, RCP_NONE,  0, 0)
#define RCP_TEXTURE1_RGB            RCP_SET_REGISTER_ENUM(GL_TEXTURE1_ARB, RCP_RGB,   0, 0)
#define RCP_TEXTURE1_ALPHA          RCP_SET_REGISTER_ENUM(GL_TEXTURE1_ARB, RCP_ALPHA, 0, 0)
#define RCP_TEXTURE1_BLUE           RCP_SET_REGISTER_ENUM(GL_TEXTURE1_ARB, RCP_BLUE,  0, 0)
#define RCP_TEXTURE1                RCP_SET_REGISTER_ENUM(GL_TEXTURE1_ARB, RCP_NONE,  0, 0)
#define RCP_TEXTURE2_RGB            RCP_SET_REGISTER_ENUM(GL_TEXTURE2_ARB, RCP_RGB,   0, 0)
#define RCP_TEXTURE2_ALPHA          RCP_SET_REGISTER_ENUM(GL_TEXTURE2_ARB, RCP_ALPHA, 0, 0)
#define RCP_TEXTURE2_BLUE           RCP_SET_REGISTER_ENUM(GL_TEXTURE2_ARB, RCP_BLUE,  0, 0)
#define RCP_TEXTURE2                RCP_SET_REGISTER_ENUM(GL_TEXTURE2_ARB, RCP_NONE,  0, 0)
#define RCP_TEXTURE3_RGB            RCP_SET_REGISTER_ENUM(GL_TEXTURE3_ARB, RCP_RGB,   0, 0)
#define RCP_TEXTURE3_ALPHA          RCP_SET_REGISTER_ENUM(GL_TEXTURE3_ARB, RCP_ALPHA, 0, 0)
#define RCP_TEXTURE3_BLUE           RCP_SET_REGISTER_ENUM(GL_TEXTURE3_ARB, RCP_BLUE,  0, 0)
#define RCP_TEXTURE3                RCP_SET_REGISTER_ENUM(GL_TEXTURE3_ARB, RCP_NONE,  0, 0)
#define RCP_CONST_COLOR0_RGB        RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR0_NV, RCP_RGB,   1, 0)
#define RCP_CONST_COLOR0_ALPHA      RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR0_NV, RCP_ALPHA, 1, 0)
#define RCP_CONST_COLOR0_BLUE       RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR0_NV, RCP_BLUE,  1, 0)
#define RCP_CONST_COLOR0            RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR0_NV, RCP_NONE,  1, 0)
#define RCP_CONST_COLOR1_RGB        RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR1_NV, RCP_RGB,   1, 0)
#define RCP_CONST_COLOR1_ALPHA      RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR1_NV, RCP_ALPHA, 1, 0)
#define RCP_CONST_COLOR1_BLUE       RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR1_NV, RCP_BLUE,  1, 0)
#define RCP_CONST_COLOR1            RCP_SET_REGISTER_ENUM(GL_CONSTANT_COLOR1_NV, RCP_NONE,  1, 0)
#define RCP_ZERO_RGB                RCP_SET_REGISTER_ENUM(GL_ZERO, RCP_RGB,   1, 0)
#define RCP_ZERO_ALPHA              RCP_SET_REGISTER_ENUM(GL_ZERO, RCP_ALPHA, 1, 0)
#define RCP_ZERO_BLUE               RCP_SET_REGISTER_ENUM(GL_ZERO, RCP_BLUE,  1, 0)
#define RCP_ZERO                    RCP_SET_REGISTER_ENUM(GL_ZERO, RCP_NONE,  1, 0)
#define RCP_ONE_RGB                 RCP_SET_REGISTER_ENUM(GL_ONE, RCP_RGB,   1, 0)
#define RCP_ONE_ALPHA               RCP_SET_REGISTER_ENUM(GL_ONE, RCP_ALPHA, 1, 0)
#define RCP_ONE_BLUE                RCP_SET_REGISTER_ENUM(GL_ONE, RCP_BLUE,  1, 0)
#define RCP_ONE                     RCP_SET_REGISTER_ENUM(GL_ONE, RCP_NONE,  1, 0)
#define RCP_DISCARD                 RCP_SET_REGISTER_ENUM(GL_DISCARD_NV, RCP_NONE, 0, 0)
#define RCP_FINAL_PRODUCT           RCP_SET_REGISTER_ENUM(GL_E_TIMES_F_NV, RCP_NONE, 1, 1)
#define RCP_COLOR_SUM               RCP_SET_REGISTER_ENUM(GL_SPARE0_PLUS_SECONDARY_COLOR_NV, RCP_NONE, 1, 1)

#define MAP_CHANNEL(channel) ((RCP_RGB == (channel)) ? GL_RGB : (RCP_ALPHA == (channel) ? GL_ALPHA : GL_BLUE))

typedef union _BiasScaleEnum {
  struct {
#if BYTE_ORDER != BIG_ENDIAN
    unsigned int bias          :16; // OpenGL enum for bias
    unsigned int scale         :16; // OpenGL enum for scale
#else
    unsigned int scale         :16; // OpenGL enum for scale
    unsigned int bias          :16; // OpenGL enum for bias
#endif
  } bits;
  unsigned int word;
} BiasScaleEnum;

// WARNING:  Don't monkey with the above structure or this macro
// unless you're absolutely sure of what you're doing!
// This constant allocation makes validation *much* cleaner.
#define RCP_SET_BIAS_SCALE_ENUM(bias, scale) ((scale << 16) | bias)

#define RCP_BIAS_BY_NEGATIVE_ONE_HALF_SCALE_BY_TWO  RCP_SET_BIAS_SCALE_ENUM(GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_SCALE_BY_TWO_NV)
#define RCP_BIAS_BY_NEGATIVE_ONE_HALF               RCP_SET_BIAS_SCALE_ENUM(GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_NONE)
#define RCP_SCALE_BY_ONE_HALF                       RCP_SET_BIAS_SCALE_ENUM(GL_NONE, GL_SCALE_BY_ONE_HALF_NV)
#define RCP_SCALE_BY_ONE                            RCP_SET_BIAS_SCALE_ENUM(GL_NONE, GL_NONE)
#define RCP_SCALE_BY_TWO                            RCP_SET_BIAS_SCALE_ENUM(GL_NONE, GL_SCALE_BY_TWO_NV)
#define RCP_SCALE_BY_FOUR                           RCP_SET_BIAS_SCALE_ENUM(GL_NONE, GL_SCALE_BY_FOUR_NV)

class MappedRegisterStruct {
public:
    void Init(RegisterEnum _reg, int _map = GL_UNSIGNED_IDENTITY_NV)
    {
        if (RCP_ONE == _reg.word) {
            _reg.word = RCP_ZERO;
            switch (_map) {
            case GL_UNSIGNED_IDENTITY_NV:
                _map = GL_UNSIGNED_INVERT_NV;
                break;
            case GL_UNSIGNED_INVERT_NV:
                _map = GL_UNSIGNED_IDENTITY_NV;
                break;
            case GL_EXPAND_NORMAL_NV:
                _map = GL_UNSIGNED_INVERT_NV;
                break;
            case GL_EXPAND_NEGATE_NV:
                _map = GL_EXPAND_NORMAL_NV;
                break;
            case GL_HALF_BIAS_NORMAL_NV:
                _map = GL_HALF_BIAS_NEGATE_NV;
                break;
            case GL_HALF_BIAS_NEGATE_NV:
                _map = GL_HALF_BIAS_NORMAL_NV;
                break;
            case GL_SIGNED_IDENTITY_NV:
                _map = GL_UNSIGNED_INVERT_NV;
                break;
            case GL_SIGNED_NEGATE_NV:
                _map = GL_EXPAND_NORMAL_NV;
                break;
            }
        }
        map = _map;
        reg = _reg;
    }
    int map;
    RegisterEnum reg;
};

#ifdef TEST_BIT_FIELDS

class RegisterEnumTest {
  public:
    RegisterEnumTest()
    {
      RegisterEnum reg;
      bool error = false;

      if (sizeof(reg.bits) != sizeof(reg.word))
        error = true;

      reg.word = 0; reg.bits.name = 0xFFFF;
      if (RCP_SET_REGISTER_ENUM(0xFFFF, 0, 0, 0) != reg.word)
        error = true;

      reg.word = 0; reg.bits.channel = 3;
      if (RCP_SET_REGISTER_ENUM(0, 3, 0, 0) != reg.word) 
        error = true;

      reg.word = 0; reg.bits.readOnly = true;
      if (RCP_SET_REGISTER_ENUM(0, 0, 1, 0) != reg.word) 
        error = true;

      reg.word = 0; reg.bits.finalOnly = true;
      if (RCP_SET_REGISTER_ENUM(0, 0, 0, 1) != reg.word) 
        error = true;

      if (error) {
    fprintf(stderr, "ERROR: Bit Fields were not compiled correctly in " __FILE__ "!\n");
    exit(1);
      }
    }
};

static RegisterEnumTest registerEnumTest;
#endif

#endif
