#ifndef __asm_math_H__
#define __asm_math_H__

#include "OgrePrerequisites.h"
#include "OgrePlatformInformation.h"

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC
#  pragma warning (push)
// disable "instruction may be inaccurate on some Pentiums"
#  pragma warning (disable : 4725)
#endif
namespace Ogre
{

/*=============================================================================
 ASM math routines posted by davepermen et al on flipcode forums
=============================================================================*/
const float pi = 4.0f * atan( 1.0f );
const float half_pi = 0.5f * pi;

/*=============================================================================
    NO EXPLICIT RETURN REQUIRED FROM THESE METHODS!! 
=============================================================================*/
#if  OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86
#   pragma warning( push )
#   pragma warning( disable: 4035 ) 
#endif

float asm_arccos( float r ) {
    // return half_pi + arctan( r / -sqr( 1.f - r * r ) );
    
#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    float asm_one = 1.f;
    float asm_half_pi = half_pi;
    __asm {
        fld r // r0 = r
        fld r // r1 = r0, r0 = r
        fmul r // r0 = r0 * r
        fsubr asm_one // r0 = r0 - 1.f
        fsqrt // r0 = sqrtf( r0 )
        fchs // r0 = - r0
        fdiv // r0 = r1 / r0
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
        fadd asm_half_pi // r0 = r0 + pi / 2
    } // returns r0

#else

    return float( acos( r ) );

#endif
}

float asm_arcsin( float r ) {
    // return arctan( r / sqr( 1.f - r * r ) );

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    const float asm_one = 1.f;
    __asm {
        fld r // r0 = r
        fld r // r1 = r0, r0 = r
        fmul r // r0 = r0 * r
        fsubr asm_one // r0 = r0 - 1.f
        fsqrt // r0 = sqrtf( r0 )
        fdiv // r0 = r1 / r0
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
    } // returns r0

#else

    return float( asin( r ) );

#endif

}

float asm_arctan( float r ) {

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    __asm {
        fld r // r0 = r
        fld1 // {{ r0 = atan( r0 )
        fpatan // }}
    } // returns r0

#else

    return float( atan( r ) );

#endif

}

float asm_sin( float r ) {

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    __asm {
        fld r // r0 = r
        fsin // r0 = sinf( r0 )
    } // returns r0

#else

    return sin( r );

#endif

}

float asm_cos( float r ) {

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    __asm {
        fld r // r0 = r
        fcos // r0 = cosf( r0 )
    } // returns r0

#else
    
    return cos( r );

#endif
}

float asm_tan( float r ) {

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    // return sin( r ) / cos( r );
    __asm {
        fld r // r0 = r
        fsin // r0 = sinf( r0 )
        fld r // r1 = r0, r0 = r
        fcos // r0 = cosf( r0 )
        fdiv // r0 = r1 / r0
    } // returns r0

#else
    
    return tan( r );

#endif
}

// returns a for a * a = r
float asm_sqrt( float r )
{
#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    __asm {
        fld r // r0 = r
        fsqrt // r0 = sqrtf( r0 )
    } // returns r0

#else

    return sqrt( r );

#endif
}

// returns 1 / a for a * a = r
// -- Use this for Vector normalisation!!!
float asm_rsq( float r )
{
#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    __asm {
        fld1 // r0 = 1.f
        fld r // r1 = r0, r0 = r
        fsqrt // r0 = sqrtf( r0 )
        fdiv // r0 = r1 / r0
    } // returns r0

#else

    return 1. / sqrt( r );

#endif
}

// returns 1 / a for a * a = r
// Another version
float apx_rsq( float r ) {

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    const float asm_dot5 = 0.5f;
    const float asm_1dot5 = 1.5f;

    __asm {
        fld r // r0 = r
        fmul asm_dot5 // r0 = r0 * .5f
        mov eax, r // eax = r
        shr eax, 0x1 // eax = eax >> 1
        neg eax // eax = -eax
        add eax, 0x5F400000 // eax = eax & MAGICAL NUMBER
        mov r, eax // r = eax
        fmul r // r0 = r0 * r
        fmul r // r0 = r0 * r
        fsubr asm_1dot5 // r0 = 1.5f - r0
        fmul r // r0 = r0 * r
    } // returns r0

#else

    return 1. / sqrt( r );

#endif
}

/* very MS-specific, commented out for now
   Finally the best InvSqrt implementation?
   Use for vector normalisation instead of 1/length() * x,y,z
*/
#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

__declspec(naked) float __fastcall InvSqrt(float fValue)
{
    __asm
    {
        mov        eax, 0be6eb508h
        mov        dword ptr[esp-12],03fc00000h
        sub        eax, dword ptr[esp + 4]
        sub        dword ptr[esp+4], 800000h
        shr        eax, 1
        mov        dword ptr[esp -  8], eax

        fld        dword ptr[esp -  8]
        fmul    st, st
        fld        dword ptr[esp -  8]
        fxch    st(1)
        fmul    dword ptr[esp +  4]
        fld        dword ptr[esp - 12]
        fld        st(0)
        fsub    st,st(2)

        fld        st(1)
        fxch    st(1)
        fmul    st(3),st
        fmul    st(3),st
        fmulp    st(4),st
        fsub    st,st(2)

        fmul    st(2),st
        fmul    st(3),st
        fmulp    st(2),st
        fxch    st(1)
        fsubp    st(1),st

        fmulp    st(1), st
        ret 4
    }
}

#endif

// returns a random number
FORCEINLINE float asm_rand()
{

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86
  #if 0
    #if OGRE_COMP_VER >= 1300

    static unsigned __int64 q = time( NULL );

    _asm {
        movq mm0, q

        // do the magic MMX thing
        pshufw mm1, mm0, 0x1E
        paddd mm0, mm1

        // move to integer memory location and free MMX
        movq q, mm0
        emms
    }

    return float( q );
    #endif
  #else
    // VC6 does not support pshufw
    return float( rand() );
  #endif
#else
    // GCC etc

    return float( rand() );

#endif
}

// returns the maximum random number
FORCEINLINE float asm_rand_max()
{

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86
  #if 0
    #if OGRE_COMP_VER >= 1300

    return (std::numeric_limits< unsigned __int64 >::max)();
    return 9223372036854775807.0f;
    #endif
  #else
    // VC6 does not support unsigned __int64
    return float( RAND_MAX );
  #endif

#else
    // GCC etc
    return float( RAND_MAX );

#endif
}

// returns log2( r ) / log2( e )
float asm_ln( float r ) {    

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86

    const float asm_1_div_log2_e = .693147180559f;
    const float asm_neg1_div_3 = -.33333333333333333333333333333f;
    const float asm_neg2_div_3 = -.66666666666666666666666666667f;
    const float asm_2 = 2.f;

    int log_2 = 0;

    __asm {
        // log_2 = ( ( r >> 0x17 ) & 0xFF ) - 0x80;
        mov eax, r
        sar eax, 0x17
        and eax, 0xFF
        sub eax, 0x80
        mov log_2, eax

        // r = ( r & 0x807fffff ) + 0x3f800000;
        mov ebx, r
        and ebx, 0x807FFFFF
        add ebx, 0x3F800000
        mov r, ebx

        // r = ( asm_neg1_div_3 * r + asm_2 ) * r + asm_neg2_div_3;   // (1)
        fld r
        fmul asm_neg1_div_3
        fadd asm_2
        fmul r
        fadd asm_neg2_div_3
        fild log_2
        fadd
        fmul asm_1_div_log2_e
    }

#else

    return log( r );

#endif
}

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC &&  OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86
#   pragma warning( pop )
#endif
} // namespace

#if  OGRE_COMPILER == OGRE_COMPILER_MSVC
#  pragma warning (pop)
#endif

#endif
