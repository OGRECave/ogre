/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgrePlatformInformation.h"
#include "OgreLog.h"
#include "OgreStringConverter.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#include <excpt.h>      // For SEH values
	#if _MSC_VER >= 1400
		#include <intrin.h>
	#endif
#elif (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
#include <signal.h>
#include <setjmp.h>

    #if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        #include <cpu-features.h>
    #elif OGRE_CPU == OGRE_CPU_ARM 
        #include <sys/sysctl.h>
        #if __MACH__
            #include <mach/machine.h>
            #ifndef CPU_SUBTYPE_ARM64_V8
                #define CPU_SUBTYPE_ARM64_V8 ((cpu_subtype_t) 1)
            #endif
            #ifndef CPU_SUBTYPE_ARM_V8
                #define CPU_SUBTYPE_ARM_V8 ((cpu_subtype_t) 13)
            #endif
        #endif
    #endif
#endif

// Yes, I know, this file looks very ugly, but there aren't other ways to do it better.

namespace Ogre {

#if OGRE_CPU == OGRE_CPU_X86

    //---------------------------------------------------------------------
    // Struct for store CPUID instruction result, compiler-independent
    //---------------------------------------------------------------------
    struct CpuidResult
    {
        // Note: DO NOT CHANGE THE ORDER, some code based on that.
        uint _eax;
        uint _ebx;
        uint _edx;
        uint _ecx;
    };

    //---------------------------------------------------------------------
    // Compiler-dependent routines
    //---------------------------------------------------------------------

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4035)  // no return value
#endif

    //---------------------------------------------------------------------
    // Detect whether CPU supports CPUID instruction, returns non-zero if supported.
    static int _isSupportCpuid(void)
    {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
		// Visual Studio 2005 & 64-bit compilers always supports __cpuid intrinsic
		// note that even though this is a build rather than runtime setting, all
		// 64-bit CPUs support this so since binary is 64-bit only we're ok
	#if _MSC_VER >= 1400 && defined(_M_X64)
		return true;
	#else
		// If we can modify flag register bit 21, the cpu is supports CPUID instruction
        __asm
        {
            // Read EFLAG
            pushfd
            pop     eax
            mov     ecx, eax

            // Modify bit 21
            xor     eax, 0x200000
            push    eax
            popfd

            // Read back EFLAG
            pushfd
            pop     eax

            // Restore EFLAG
            push    ecx
            popfd

            // Check bit 21 modifiable
            xor     eax, ecx
            neg     eax
            sbb     eax, eax

            // Return values in eax, no return statement requirement here for VC.
        }
	#endif
#elif (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
        #if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
           return true;
       #else
        unsigned oldFlags, newFlags;
        __asm__
        (
            "pushfl         \n\t"
            "pop    %0      \n\t"
            "mov    %0, %1  \n\t"
            "xor    %2, %0  \n\t"
            "push   %0      \n\t"
            "popfl          \n\t"
            "pushfl         \n\t"
            "pop    %0      \n\t"
            "push   %1      \n\t"
            "popfl          \n\t"
            : "=r" (oldFlags), "=r" (newFlags)
            : "n" (0x200000)
        );
        return oldFlags != newFlags;
       #endif // 64
#else
        // TODO: Supports other compiler
        return false;
#endif
    }

    //---------------------------------------------------------------------
    // Performs CPUID instruction with 'query', fill the results, and return value of eax.
    static uint _performCpuid(int query, CpuidResult& result)
    {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
	#if _MSC_VER >= 1400 
		int CPUInfo[4];
		__cpuid(CPUInfo, query);
		result._eax = CPUInfo[0];
		result._ebx = CPUInfo[1];
		result._ecx = CPUInfo[2];
		result._edx = CPUInfo[3];
		return result._eax;
	#else
        __asm
        {
            mov     edi, result
            mov     eax, query
            cpuid
            mov     [edi]._eax, eax
            mov     [edi]._ebx, ebx
            mov     [edi]._edx, edx
            mov     [edi]._ecx, ecx
            // Return values in eax, no return statement requirement here for VC.
        }
	#endif
#elif (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
        #if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
        __asm__
        (
            "cpuid": "=a" (result._eax), "=b" (result._ebx), "=c" (result._ecx), "=d" (result._edx) : "a" (query)
        );
        #else
        __asm__
        (
            "pushl  %%ebx           \n\t"
            "cpuid                  \n\t"
            "movl   %%ebx, %%edi    \n\t"
            "popl   %%ebx           \n\t"
            : "=a" (result._eax), "=D" (result._ebx), "=c" (result._ecx), "=d" (result._edx)
            : "a" (query)
        );
       #endif // OGRE_ARCHITECTURE_64
        return result._eax;

#else
        // TODO: Supports other compiler
        return 0;
#endif
    }

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma warning(pop)
#endif

    //---------------------------------------------------------------------
    // Detect whether or not os support Streaming SIMD Extension.
#if (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
    #if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_32 && OGRE_CPU == OGRE_CPU_X86
    static jmp_buf sIllegalJmpBuf;
    static void _illegalHandler(int x)
    {
        (void)(x); // Unused
        longjmp(sIllegalJmpBuf, 1);
    }
    #endif
#endif
    static bool _checkOperatingSystemSupportSSE(void)
    {
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
        /*
            The FP part of SSE introduces a new architectural state and therefore
            requires support from the operating system. So even if CPUID indicates
            support for SSE FP, the application might not be able to use it. If
            CPUID indicates support for SSE FP, check here whether it is also
            supported by the OS, and turn off the SSE FP feature bit if there
            is no OS support for SSE FP.

            Operating systems that do not support SSE FP return an illegal
            instruction exception if execution of an SSE FP instruction is performed.
            Here, a sample SSE FP instruction is executed, and is checked for an
            exception using the (non-standard) __try/__except mechanism
            of Microsoft Visual C/C++.
        */
		// Visual Studio 2005, Both AMD and Intel x64 support SSE
		// note that even though this is a build rather than runtime setting, all
		// 64-bit CPUs support this so since binary is 64-bit only we're ok
	#if _MSC_VER >= 1400 && defined(_M_X64)
			return true;
	#else
        __try
        {
            __asm orps  xmm0, xmm0
            return true;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
	#endif
#elif (OGRE_COMPILER == OGRE_COMPILER_GNUC || OGRE_COMPILER == OGRE_COMPILER_CLANG) && OGRE_PLATFORM != OGRE_PLATFORM_NACL
        #if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64 
            return true;
        #else
        // Does gcc have __try/__except similar mechanism?
        // Use signal, setjmp/longjmp instead.
        void (*oldHandler)(int);
        oldHandler = signal(SIGILL, _illegalHandler);

        if (setjmp(sIllegalJmpBuf))
        {
            signal(SIGILL, oldHandler);
            return false;
        }
        else
        {
            __asm__ __volatile__ ("orps %xmm0, %xmm0");
            signal(SIGILL, oldHandler);
            return true;
        }
       #endif
#else
        // TODO: Supports other compiler, assumed is supported by default
        return true;
#endif
    }

    //---------------------------------------------------------------------
    // Compiler-independent routines
    //---------------------------------------------------------------------

    static uint queryCpuFeatures(void)
    {
#define CPUID_STD_FPU               (1<<0)
#define CPUID_STD_TSC               (1<<4)
#define CPUID_STD_CMOV              (1<<15)
#define CPUID_STD_MMX               (1<<23)
#define CPUID_STD_SSE               (1<<25)
#define CPUID_STD_SSE2              (1<<26)
#define CPUID_STD_HTT               (1<<28)     // EDX[28] - Bit 28 set indicates  Hyper-Threading Technology is supported in hardware.

#define CPUID_STD_SSE3              (1<<0)      // ECX[0] - Bit 0 of standard function 1 indicate SSE3 supported

#define CPUID_FAMILY_ID_MASK        0x0F00      // EAX[11:8] - Bit 11 thru 8 contains family  processor id
#define CPUID_EXT_FAMILY_ID_MASK    0x0F00000   // EAX[23:20] - Bit 23 thru 20 contains extended family processor id
#define CPUID_PENTIUM4_ID           0x0F00      // Pentium 4 family processor id

#define CPUID_EXT_3DNOW             (1<<31)
#define CPUID_EXT_AMD_3DNOWEXT      (1<<30)
#define CPUID_EXT_AMD_MMXEXT        (1<<22)

        uint features = 0;

        // Supports CPUID instruction ?
        if (_isSupportCpuid())
        {
            CpuidResult result;

            // Has standard feature ?
            if (_performCpuid(0, result))
            {
                // Check vendor strings
                if (memcmp(&result._ebx, "GenuineIntel", 12) == 0)
                {
                    if (result._eax > 2)
                        features |= PlatformInformation::CPU_FEATURE_PRO;

                    // Check standard feature
                    _performCpuid(1, result);

                    if (result._edx & CPUID_STD_FPU)
                        features |= PlatformInformation::CPU_FEATURE_FPU;
                    if (result._edx & CPUID_STD_TSC)
                        features |= PlatformInformation::CPU_FEATURE_TSC;
                    if (result._edx & CPUID_STD_CMOV)
                        features |= PlatformInformation::CPU_FEATURE_CMOV;
                    if (result._edx & CPUID_STD_MMX)
                        features |= PlatformInformation::CPU_FEATURE_MMX;
                    if (result._edx & CPUID_STD_SSE)
                        features |= PlatformInformation::CPU_FEATURE_MMXEXT | PlatformInformation::CPU_FEATURE_SSE;
                    if (result._edx & CPUID_STD_SSE2)
                        features |= PlatformInformation::CPU_FEATURE_SSE2;

                    if (result._ecx & CPUID_STD_SSE3)
                        features |= PlatformInformation::CPU_FEATURE_SSE3;

                    // Check to see if this is a Pentium 4 or later processor
                    if ((result._eax & CPUID_EXT_FAMILY_ID_MASK) ||
                        (result._eax & CPUID_FAMILY_ID_MASK) == CPUID_PENTIUM4_ID)
                    {
                        // Check hyper-threading technology
                        if (result._edx & CPUID_STD_HTT)
                            features |= PlatformInformation::CPU_FEATURE_HTT;
                    }
                }
                else if (memcmp(&result._ebx, "AuthenticAMD", 12) == 0)
                {
                    features |= PlatformInformation::CPU_FEATURE_PRO;

                    // Check standard feature
                    _performCpuid(1, result);

                    if (result._edx & CPUID_STD_FPU)
                        features |= PlatformInformation::CPU_FEATURE_FPU;
                    if (result._edx & CPUID_STD_TSC)
                        features |= PlatformInformation::CPU_FEATURE_TSC;
                    if (result._edx & CPUID_STD_CMOV)
                        features |= PlatformInformation::CPU_FEATURE_CMOV;
                    if (result._edx & CPUID_STD_MMX)
                        features |= PlatformInformation::CPU_FEATURE_MMX;
                    if (result._edx & CPUID_STD_SSE)
                        features |= PlatformInformation::CPU_FEATURE_SSE;
                    if (result._edx & CPUID_STD_SSE2)
                        features |= PlatformInformation::CPU_FEATURE_SSE2;

                    if (result._ecx & CPUID_STD_SSE3)
                        features |= PlatformInformation::CPU_FEATURE_SSE3;

                    // Has extended feature ?
                    if (_performCpuid(0x80000000, result) > 0x80000000)
                    {
                        // Check extended feature
                        _performCpuid(0x80000001, result);

                        if (result._edx & CPUID_EXT_3DNOW)
                            features |= PlatformInformation::CPU_FEATURE_3DNOW;
                        if (result._edx & CPUID_EXT_AMD_3DNOWEXT)
                            features |= PlatformInformation::CPU_FEATURE_3DNOWEXT;
                        if (result._edx & CPUID_EXT_AMD_MMXEXT)
                            features |= PlatformInformation::CPU_FEATURE_MMXEXT;
                    }
                }
            }
        }

        return features;
    }
    //---------------------------------------------------------------------
    static uint _detectCpuFeatures(void)
    {
        uint features = queryCpuFeatures();

        const uint sse_features = PlatformInformation::CPU_FEATURE_SSE |
            PlatformInformation::CPU_FEATURE_SSE2 | PlatformInformation::CPU_FEATURE_SSE3;
        if ((features & sse_features) && !_checkOperatingSystemSupportSSE())
        {
            features &= ~sse_features;
        }

        return features;
    }
    //---------------------------------------------------------------------
    static String _detectCpuIdentifier(void)
    {
		// Supports CPUID instruction ?
		if (_isSupportCpuid())
		{
			CpuidResult result;
			uint nExIds;
			char CPUString[0x20];
			char CPUBrandString[0x40];

			StringUtil::StrStreamType detailedIdentStr;


			// Has standard feature ?
			if (_performCpuid(0, result))
			{
				memset(CPUString, 0, sizeof(CPUString));
				memset(CPUBrandString, 0, sizeof(CPUBrandString));

				//*((int*)CPUString) = result._ebx;
                                memcpy(CPUString, &result._ebx, sizeof(int));
				//*((int*)(CPUString+4)) = result._edx;
                                //*((int*)(CPUString+8)) = result._ecx;
                                memcpy(CPUString+8, &result._ecx, sizeof(int));

				detailedIdentStr << CPUString;

				// Calling _performCpuid with 0x80000000 as the query argument
				// gets the number of valid extended IDs.
				nExIds = _performCpuid(0x80000000, result);

				for (uint i=0x80000000; i<=nExIds; ++i)
				{
					_performCpuid(i, result);

					// Interpret CPU brand string and cache information.
					if  (i == 0x80000002)
                    {
						memcpy(CPUBrandString + 0, &result._eax, sizeof(result._eax));
						memcpy(CPUBrandString + 4, &result._ebx, sizeof(result._ebx));
						memcpy(CPUBrandString + 8, &result._ecx, sizeof(result._ecx));
						memcpy(CPUBrandString + 12, &result._edx, sizeof(result._edx));
                    }
					else if  (i == 0x80000003)
                    {
						memcpy(CPUBrandString + 16 + 0, &result._eax, sizeof(result._eax));
						memcpy(CPUBrandString + 16 + 4, &result._ebx, sizeof(result._ebx));
						memcpy(CPUBrandString + 16 + 8, &result._ecx, sizeof(result._ecx));
						memcpy(CPUBrandString + 16 + 12, &result._edx, sizeof(result._edx));
                    }
					else if  (i == 0x80000004)
                    {
						memcpy(CPUBrandString + 32 + 0, &result._eax, sizeof(result._eax));
						memcpy(CPUBrandString + 32 + 4, &result._ebx, sizeof(result._ebx));
						memcpy(CPUBrandString + 32 + 8, &result._ecx, sizeof(result._ecx));
						memcpy(CPUBrandString + 32 + 12, &result._edx, sizeof(result._edx));
                    }
				}

				String brand(CPUBrandString);
				StringUtil::trim(brand);
				if (!brand.empty())
					detailedIdentStr << ": " << brand;

				return detailedIdentStr.str();
			}
		}

		return "X86";
    }

#elif OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
    static uint _detectCpuFeatures(void)
    {
        uint features = 0;
        uint64_t cpufeatures = android_getCpuFeatures();
        
        if (cpufeatures & ANDROID_CPU_ARM_FEATURE_NEON) 
        {
            features |= PlatformInformation::CPU_FEATURE_NEON;
        }
        
        if (cpufeatures & ANDROID_CPU_ARM_FEATURE_VFPv3) 
        {
            features |= PlatformInformation::CPU_FEATURE_VFP;
        }
        return features;
    }
    //---------------------------------------------------------------------
    static String _detectCpuIdentifier(void)
    {
        String cpuID;
        AndroidCpuFamily cpuInfo = android_getCpuFamily();
        
        switch (cpuInfo) {
            case ANDROID_CPU_FAMILY_ARM:
            {
                if (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_ARMv7) 
                {
                    cpuID = "ARMv7";
                }
                else
                {
                    cpuID = "Unknown ARM";
                }
            }
            break;
            case ANDROID_CPU_FAMILY_X86:
                cpuID = "Unknown X86";
                break;   
            default:
                cpuID = "Unknown";
                break;
        }
        return cpuID;
    }
    
#elif OGRE_CPU == OGRE_CPU_ARM  // OGRE_CPU == OGRE_CPU_ARM

    //---------------------------------------------------------------------
    static uint _detectCpuFeatures(void)
    {
        // Use preprocessor definitions to determine architecture and CPU features
        uint features = 0;
#if defined(__ARM_NEON__)
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        int hasNEON;
        size_t len = sizeof(size_t);
        sysctlbyname("hw.optional.neon", &hasNEON, &len, NULL, 0);

        if(hasNEON)
#endif
            features |= PlatformInformation::CPU_FEATURE_NEON;
#elif defined(__VFP_FP__)
            features |= PlatformInformation::CPU_FEATURE_VFP;
#endif
        return features;
    }
    //---------------------------------------------------------------------
    static String _detectCpuIdentifier(void)
    {
        String cpuID;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        // Get the size of the CPU subtype struct
        size_t size;
        sysctlbyname("hw.cpusubtype", NULL, &size, NULL, 0);

        // Get the ARM CPU subtype
        cpu_subtype_t cpusubtype = 0;
        sysctlbyname("hw.cpusubtype", &cpusubtype, &size, NULL, 0);

        switch(cpusubtype)
        {
            case CPU_SUBTYPE_ARM_V6:
                cpuID = "ARMv6";
                break;
            case CPU_SUBTYPE_ARM_V7:
                cpuID = "ARMv7";
                break;
            case CPU_SUBTYPE_ARM_V7F:
                cpuID = "ARM Cortex-A9";
                break;
            case CPU_SUBTYPE_ARM_V7S:
                cpuID = "ARM Swift";
                break;
            case CPU_SUBTYPE_ARM_V8:
                cpuID = "ARMv8";
                break;
            case CPU_SUBTYPE_ARM64_V8:
                cpuID = "ARM64v8";
                break;
            default:
                cpuID = "Unknown ARM";
                break;
        }
#endif
        return cpuID;
    }
    
#elif OGRE_CPU == OGRE_CPU_MIPS  // OGRE_CPU == OGRE_CPU_ARM

    //---------------------------------------------------------------------
    static uint _detectCpuFeatures(void)
    {
        // Use preprocessor definitions to determine architecture and CPU features
        uint features = 0;
#if defined(__mips_msa)
        features |= PlatformInformation::CPU_FEATURE_MSA;
#endif
        return features;
    }
    //---------------------------------------------------------------------
    static String _detectCpuIdentifier(void)
    {
        String cpuID = "MIPS";

        return cpuID;
    }

#else   // OGRE_CPU == OGRE_CPU_MIPS

    //---------------------------------------------------------------------
    static uint _detectCpuFeatures(void)
    {
        return 0;
    }
    //---------------------------------------------------------------------
    static String _detectCpuIdentifier(void)
    {
        return "Unknown";
    }

#endif  // OGRE_CPU

    //---------------------------------------------------------------------
    // Platform-independent routines, but the returns value are platform-dependent
    //---------------------------------------------------------------------

    const String& PlatformInformation::getCpuIdentifier(void)
    {
        static const String sIdentifier = _detectCpuIdentifier();
        return sIdentifier;
    }
    //---------------------------------------------------------------------
    uint PlatformInformation::getCpuFeatures(void)
    {
        static const uint sFeatures = _detectCpuFeatures();
        return sFeatures;
    }
	//---------------------------------------------------------------------
	bool PlatformInformation::hasCpuFeature(CpuFeatures feature)
	{
		return (getCpuFeatures() & feature) != 0;
	}
	//---------------------------------------------------------------------
	void PlatformInformation::log(Log* pLog)
	{
		pLog->logMessage("CPU Identifier & Features");
		pLog->logMessage("-------------------------");
		pLog->logMessage(
			" *   CPU ID: " + getCpuIdentifier());
#if OGRE_CPU == OGRE_CPU_X86
		if(_isSupportCpuid())
		{
			pLog->logMessage(
				" *      SSE: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_SSE), true));
			pLog->logMessage(
				" *     SSE2: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_SSE2), true));
			pLog->logMessage(
				" *     SSE3: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_SSE3), true));
			pLog->logMessage(
				" *      MMX: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_MMX), true));
			pLog->logMessage(
				" *   MMXEXT: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_MMXEXT), true));
			pLog->logMessage(
				" *    3DNOW: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_3DNOW), true));
			pLog->logMessage(
				" * 3DNOWEXT: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_3DNOWEXT), true));
			pLog->logMessage(
				" *     CMOV: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_CMOV), true));
			pLog->logMessage(
				" *      TSC: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_TSC), true));
			pLog->logMessage(
				" *      FPU: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_FPU), true));
			pLog->logMessage(
				" *      PRO: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_PRO), true));
			pLog->logMessage(
				" *       HT: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_HTT), true));
		}
#elif OGRE_CPU == OGRE_CPU_ARM || OGRE_PLATFORM == OGRE_PLATFORM_ANDROID
        pLog->logMessage(
				" *      VFP: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_VFP), true));
        pLog->logMessage(
				" *     NEON: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_NEON), true));
#elif OGRE_CPU == OGRE_CPU_MIPS
        pLog->logMessage(
                " *      MSA: " + StringConverter::toString(hasCpuFeature(CPU_FEATURE_MSA), true));
#endif
		pLog->logMessage("-------------------------");

	}


}
