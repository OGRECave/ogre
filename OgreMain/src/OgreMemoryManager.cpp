/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#include "OgreStableHeaders.h"
//---- ORIGINAL COPYRIGHT FOLLOWS -------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000, Paul Nettle. All rights reserved.
//
// You are free to use this source code in any commercial or non-commercial product.
//
// mmgr.cpp - Memory manager & tracking software
//
// The most recent version of this software can be found at: ftp://ftp.GraphicsPapers.com/pub/ProgrammingTools/MemoryManagers/
//
// [NOTE: Best when viewed with 8-character tabs]
//
// ---------------------------------------------------------------------------------------------------------------------------------

#include "OgreMemoryManager.h"

//-----------------------------------------------------------------------------
// Allow the use of the real *alloc/free/new/delete functions
#include "OgreNoMemoryMacros.h"
//-----------------------------------------------------------------------------

namespace Ogre
{

    //-----------------------------------------------------------------------------
    MemoryManager& MemoryManager::instance(void)
    {
        // Ensure MemoryManager construct before any memory allocate,
        // and then destruct after all memory deallocated.
        static MemoryManager sMemManager;

        return sMemManager;
    }
    //-----------------------------------------------------------------------------

#if OGRE_DEBUG_MEMORY_MANAGER && OGRE_DEBUG_MODE

#define OGRE_MEMMANAGER_STRESS_TEST 0

#if OGRE_MEMORY_STRESS_TEST

    bool randomWipe           = true;
    bool alwaysValidateAll    = true;
    bool alwaysLogAll         = true;
    bool alwaysWipeAll        = false;
    bool cleanupLogOnFirstRun = true;

    const unsigned int hashBits    = 24;
    const unsigned int paddingSize = 1024; // An extra 8K per allocation!

#else

    bool randomWipe           = false;
    bool alwaysValidateAll    = false;
    bool alwaysLogAll         = false;
    bool alwaysWipeAll        = true;
    bool cleanupLogOnFirstRun = true;

    const unsigned int hashBits    = 24;
    const unsigned int paddingSize = 4;

#endif

    //---------------------------------------------------------------------------------------------
    // We define our own assert, because we don't want to bring up an assertion dialog, since that 
    // allocates RAM. Our new assert simply declares a forced breakpoint.
    //
    // The BEOS assert added by Arvid Norberg <arvid@iname.com>.    
    #ifdef    WIN32
        #ifdef    _DEBUG
            #if defined(_MSC_VER) 
                    #define m_assert(x)  if( (x) == false ) __debugbreak();                
            #elif defined(__GNUC__)
                #define m_assert(x)  if( (x) == false ) __asm ("int $3");  
            #endif
        #else
            #define    m_assert(x)
        #endif
    #elif defined(__BEOS__)
        #ifdef DEBUG
    extern void debugger(const char *message);
            #define m_assert(x) { if( (x) == false ) debugger("mmgr: assert failed") }
        #else
            #define m_assert(x)
        #endif
    #else    // We can use this safely on *NIX, since it doesn't bring up a dialog window.
        #define m_assert(cond) assert(cond)
    #endif
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Here, we turn off our macros because any place in this source file where the word 'new' or 
    // the word 'delete' (etc.) appear will be expanded by the macro. So to avoid problems using 
    // them within this source file, we'll just #undef them.
    #include "OgreNoMemoryMacros.h"
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Get to know these values. They represent the values that will be used to fill unused and 
    // deallocated RAM.    
    unsigned int prefixPattern   = 0xbaadf00d; // Fill pattern for bytes preceeding allocated blocks
    unsigned int postfixPattern  = 0xdeadc0de; // Fill pattern for bytes following allocated blocks
    unsigned int unusedPattern   = 0xfeedface; // Fill pattern for freshly allocated blocks 
    unsigned int releasedPattern = 0xdeadbeef; // Fill pattern for deallocated blocks
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Other locals    
    const unsigned int hashSize = 1 << hashBits;
    const char *allocationTypes[] = 
    {
        "Unknown", 
        "new",     
        "new[]",  
        "malloc",   
        "calloc", 
        "realloc", 
        "delete", 
        "delete[]", 
        "free"
    };
    //---------------------------------------------------------------------------------------------

    sAllocUnit *hashTable[hashSize];
    sAllocUnit *reservoir = NULL;

    unsigned int currentAllocationCount = 0;
    unsigned int breakOnAllocationCount = 0;

    sMStats stats;
        
    const char *sourceFile = "??";
    const char *sourceFunc = "??";
    unsigned int sourceLine = 0;

    sAllocUnit    **reservoirBuffer      = NULL;
    unsigned int    reservoirBufferSize    = 0;

    const char *memoryLogFile     = "OgreMemory.log";
    const char *memoryLeakLogFile = "OgreLeaks.log";

    void doCleanupLogOnFirstRun();

    //---------------------------------------------------------------------------------------------
    // Local functions only
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    /** Logs a piece of information.
    */
    void log( const char *format, ... )
    {
        // The buffer
        char buffer[2048];

        va_list ap;
        va_start( ap, format );
        vsprintf( buffer, format, ap );
        va_end( ap );

        // Cleanup the log?

        if( cleanupLogOnFirstRun )
            doCleanupLogOnFirstRun();

        // Open the log file
        FILE *fp = fopen( memoryLogFile, "ab" );

        // If you hit this assert, then the memory logger is unable to log 
        // information to a file (can't open the file for some reason.) You can 
        // interrogate the variable 'buffer' to see what was supposed to be logged 
        // (but won't be.)
        m_assert(fp);

        if( !fp ) 
            return;

        // Spit out the data to the log
        fprintf( fp, "%s\r\n", buffer );

        fclose( fp );
    }

    //---------------------------------------------------------------------------------------------
    /** Cleans up the log.
    */
    void doCleanupLogOnFirstRun()
    {
        if( cleanupLogOnFirstRun )
        {
            unlink( memoryLogFile );
            cleanupLogOnFirstRun = false;

            // Print a header for the log
            time_t t = time(NULL);
            log("--------------------------------------------------------------------------------");
            log("");
            log("      %s - Memory logging file created on %s", memoryLogFile, asctime(localtime(&t)));
            log("--------------------------------------------------------------------------------");
            log("");
            log("This file contains a log of all memory operations performed during the last run.");
            log("");
            log("Interrogate this file to track errors or to help track down memory-related");
            log("issues. You can do this by tracing the allocations performed by a specific owner");
            log("or by tracking a specific address through a series of allocations and");
            log("reallocations.");
            log("");
            log("There is a lot of useful information here which, when used creatively, can be");
            log("extremely helpful.");
            log("");
            log("Note that the following guides are used throughout this file:");
            log("");
            log("   [!] - Error");
            log("   [+] - Allocation");
            log("   [~] - Reallocation");
            log("   [-] - Deallocation");
            log("   [I] - Generic information");
            log("   [F] - Failure induced for the purpose of stress-testing your application");
            log("   [D] - Information used for debugging this memory manager");
            log("");
            log("...so, to find all errors in the file, search for \"[!]\"");
            log("");
            log("--------------------------------------------------------------------------------");
        }
    }

    //---------------------------------------------------------------------------------------------
    /** This function strips the path from the beginning of the string that 
        contains a path and a file name.
        @returns
            If possible, only the file name. Otherwise, the full string is 
            returned.
    */
    const char *sourceFileStripper( const char *sourceFile )
    {
        const char *ptr = strrchr(sourceFile, '\\');
        if( ptr ) 
            return ptr + 1;
        ptr = strrchr( sourceFile, '/' );
        if( ptr ) 
            return ptr + 1;
        return sourceFile;
    }

    //---------------------------------------------------------------------------------------------
    /** This helper function writes file and line information to a string.
        @note
            This function is not thread-safe.
    */
    const char *ownerString(
        const char *sourceFile, 
        const unsigned int sourceLine, 
        const char *sourceFunc )
    {
        static char str[90];
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined( __MINGW32__ )
		_snprintf( 
#else
        snprintf( 
#endif
			str, 89, "%s(%05d)::%s", 
            sourceFileStripper(sourceFile), 
            sourceLine, 
            sourceFunc);
        return str;
    }

    //---------------------------------------------------------------------------------------------
    /** This helper function transforms an integer into a string with decimal 
        separators.
        @note
            This function is not thread-safe.
    */
    const char *insertCommas( size_t value )
    {
        static char str[30];

        // This pointer is used to add digits moving backwards in the string.
        char *p = &str[28];
        // The current digit
        int c_digit = 1;

        // Set the last character in the string to NULL.
        str[29] = 0;

        // While we've still got some digits in value, add them.
        while( value )
        {
            *p-- = '0' + (char)( value % 10 ); value /= 10;

            // If the digit which was inserted was at the end of a group, add a comma.
            if( !( c_digit % 3 ) )
                *p-- = ',';

            c_digit++;
        }

        // Now return the offset in the static string above.
        return ++p;
    }

    //---------------------------------------------------------------------------------------------
    /** Converts a decimal memory value into human-readable format.
        @note
            This function is not thread-safe.
    */
    const char *memorySizeString( size_t size )
    {       
        static char str[90];

        if( size > 1048576 )
            sprintf( str, "%10s (%7.2fM)", insertCommas( size ), (float) size / 1048576.0f );
        else if( size > 1024 )        
            sprintf( str, "%10s (%7.2fK)", insertCommas( size ), (float) size / 1024.0f );
        else
            sprintf( str, "%10s bytes     ", insertCommas( size ) );
        return str;
    }

    //---------------------------------------------------------------------------------------------
    /** Tries to locate an allocation unit.
    */
    sAllocUnit *findAllocUnit(const void *reportedAddress)
    {
        // Just in case...
        m_assert( reportedAddress != NULL );

        // Use the address to locate the hash index. Note that we shift off the 
        // lower four bits. This is because most allocated addresses will be on 
        // four-, eight- or even sixteen-byte boundaries. If we didn't do this, 
        // the hash index would not have very good coverage.

        size_t hashIndex = ( (size_t)reportedAddress >> 4 ) & ( hashSize - 1 );
        sAllocUnit *ptr = hashTable[ hashIndex ];
        while( ptr )
        {
            if( ptr->reportedAddress == reportedAddress )
                return ptr;
            ptr = ptr->next;
        }

        return NULL;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    inline static size_t calculateActualSize( const size_t reportedSize )
    {
        // We use DWORDS as our padding, and a long is guaranteed to be 4 bytes, 
        // but an int is not (ANSI defines an int as being the standard word size 
        // for a processor; on a 32-bit machine, that's 4 bytes, but on a 64-bit 
        // machine, it's 8 bytes, which means an int can actually be larger than 
        // a long.)

        return reportedSize + paddingSize * sizeof(long) * 2;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    inline static size_t calculateReportedSize( const size_t actualSize )
    {
        // We use DWORDS as our padding, and a long is guaranteed to be 4 bytes, 
        // but an int is not (ANSI defines an int as being the standard word size 
        // for a processor; on a 32-bit machine, that's 4 bytes, but on a 64-bit 
        // machine, it's 8 bytes, which means an int can actually be larger than a 
        // long.)

        return actualSize - paddingSize * sizeof(long) * 2;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    inline void *calculateReportedAddress( const void *actualAddress )
    {
        // We allow this...
        if (!actualAddress)
            return NULL;

        // Just account for the padding
        return (void *)((char *) actualAddress + sizeof(long) * paddingSize );
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void wipeWithPattern(
        sAllocUnit *allocUnit, 
        unsigned long pattern, 
        const size_t originalReportedSize = 0 )
    {
        // For a serious test run, we use wipes of random a random value. However, 
        // if this causes a crash, we don't want it to crash in a different place 
        // each time, so we specifically DO NOT call srand. If, by chance your 
        // program calls srand(), you may wish to disable that when running with a 
        // random wipe test. This will make any crashes more consistent so they
        // can be tracked down easier.
        if( randomWipe )
        {
            pattern = ((rand() & 0xff) << 24) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 8) | (rand() & 0xff);
        }

        // -DOC- We should wipe with 0's if we're not in debug mode, so we can help
        // hide bugs if possible when we release the product. So uncomment the 
        // following line for releases.
        //
        // Note that the "alwaysWipeAll" should be turned on for this to have 
        // effect, otherwise it won't do much good. But we will leave it this way (as 
        // an option) because this does slow things down.

        //    pattern = 0;

        // This part of the operation is optional
        if( alwaysWipeAll && allocUnit->reportedSize > originalReportedSize )
        {
            // Fill the bulk
            long  *lptr = (long *) ((char *)allocUnit->reportedAddress + originalReportedSize);
            size_t length = allocUnit->reportedSize - originalReportedSize;
            size_t i;
            for( i = 0; i < (length >> 2); i++, lptr++ )
            {
                *lptr = pattern;
            }

            // Fill the remainder
            unsigned int shiftCount = 0;
            char *cptr = (char *) lptr;
            for( i = 0; i < ( length & 0x3 ); i++, cptr++, shiftCount += 8 )
            {
                *cptr =  (char)((( pattern & ( 0xff << shiftCount ) ) >> shiftCount) & 0xff);
            }
        }

        // Write in the prefix/postfix bytes
        long        *pre = (long *)allocUnit->actualAddress;
        long        *post = (long *)((char *)allocUnit->actualAddress + allocUnit->actualSize - paddingSize * sizeof(long));
        for (unsigned int i = 0; i < paddingSize; i++, pre++, post++)
        {
            *pre = prefixPattern;
            *post = postfixPattern;
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void dumpAllocations(FILE *fp)
    {
        fprintf(fp, "Alloc.   Addr       Size       Addr       Size                        BreakOn BreakOn              \r\n");
        fprintf(fp, "Number Reported   Reported    Actual     Actual     Unused    Method  Dealloc Realloc Allocated by \r\n");
        fprintf(fp, "------ ---------- ---------- ---------- ---------- ---------- -------- ------- ------- --------------------------------------------------- \r\n");

        for( unsigned int i = 0; i < hashSize; i++ )
        {
            sAllocUnit *ptr = hashTable[i];
            while( ptr )
            {
                fprintf(fp, "%06d 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X %-8s    %c       %c    %s(%d) %s\r\n",
                    ptr->allocationNumber,
                    (size_t) ptr->reportedAddress, ptr->reportedSize,
                    (size_t) ptr->actualAddress, ptr->actualSize,
                    MemoryManager::instance().calcUnused(ptr),
                    allocationTypes[ptr->allocationType],
                    ptr->breakOnDealloc ? 'Y':'N',
                    ptr->breakOnRealloc ? 'Y':'N',
                    ptr->sourceFile, ptr->sourceLine, ptr->sourceFunc
                    /*ownerString(ptr->sourceFile, ptr->sourceLine, ptr->sourceFunc)*/);
                ptr = ptr->next;
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void dumpLeakReport()
    {
        // Open the report file
        FILE *fp = fopen(memoryLeakLogFile, "w+b");

        // If you hit this assert, then the memory report generator is unable to 
        // log information to a file (can't open the file for some reason.)
        m_assert(fp);

        if( !fp )
            return;

        // Any leaks?

        // Header
        char timeString[25];
        memset( timeString, 0, sizeof(timeString) );
        time_t t = time(NULL);
        struct tm *tme = localtime(&t);

        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                          Memory leak report for:  %02d/%02d/%04d %02d:%02d:%02d                                            |\r\n", tme->tm_mon + 1, tme->tm_mday, tme->tm_year + 1900, tme->tm_hour, tme->tm_min, tme->tm_sec);
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "\r\n");
        fprintf(fp, "\r\n");

        if( stats.totalAllocUnitCount )
        {
            fprintf(fp, "%d memory leak%s found:\r\n", 
                stats.totalAllocUnitCount, 
                stats.totalAllocUnitCount == 1 ? "":"s" );
        }
        else
        {
            fprintf(fp, "Congratulations! No memory leaks found!\r\n");

            // We can finally free up our own memory allocations
            if (reservoirBuffer)
            {
                for (unsigned int i = 0; i < reservoirBufferSize; i++)
                {
                    free( reservoirBuffer[i] );
                }
                free( reservoirBuffer );
                reservoirBuffer = NULL;
                reservoirBufferSize = 0;
                reservoir = NULL;
            }
        }
        fprintf(fp, "\r\n");

        if( stats.totalAllocUnitCount )
        {
            dumpAllocations(fp);
        }

        fclose(fp);
    }

    /** When chasing a hard-to-pinpoint bug, use this function to make the memory 
        manager exectue a breakpoint when the passed memory location is deallocated.
    */
    bool& MemoryManager::breakOnDealloc(void *reportedAddress)
    {
    #ifdef _DEBUG
        // Locate the existing allocation unit
        sAllocUnit *au = findAllocUnit( reportedAddress );

        // If you hit this assert, you tried to set a breakpoint on deallocation 
        // for an address that doesn't exist. Interrogate the stack frame or the 
        // variable 'au' to see which allocation this is.
        m_assert(au != NULL);

        return au->breakOnDealloc;
    #else
        static bool b;
        return b;
    #endif
    }

    /** When tracking down a hard-to-pinpoint bug, use this function to make the
        memory manager execute a breakpoint when a specified number of allocations
        ave been made
    */
    void MemoryManager::breakOnAlloc(unsigned int count)
    {
    #ifdef _DEBUG
        breakOnAllocationCount = count;
    #endif
    }

    void MemoryManager::setOwner(const char *file, const unsigned int line, const char *func)
    {
        // You're probably wondering about this...
        //
        // It's important for this memory manager to primarily work with global 
        // new/delete in their original forms (i.e. with no extra parameters.) In 
        // order to do this, we use macros that call this function prior to 
        // operators new & delete. This is fine... usually. Here's what actually 
        // happens when you use this macro to delete an object:
        //
        // setOwner( __FILE__, __LINE__, __FUNCTION__ ) --> object::~object() --> delete
        //
        // Note that the compiler inserts a call to the object's destructor just 
        // prior to calling our overridden operator delete. 
        //
        // But what happens when we delete an object whose destructor deletes 
        // another object, whose desctuctor deletes another object? Here's a 
        // diagram (indentation follows stack depth):
        //
        // setOwner(...) -> ~obj1()            // original call to delete obj1
        //     setOwner(...) -> ~obj2()        // obj1's destructor deletes obj2
        //         setOwner(...) -> ~obj3()    // obj2's destructor deletes obj3
        //             ...                       // obj3's destructor just does some stuff
        //         delete                        // back in obj2's destructor, we call delete
        //     delete                            // back in obj1's destructor, we call delete
        // delete                                // back to our original call, we call delete
        //
        // Because setOwner() just sets up some variables (below) it's important 
        // that each call to setOwner() and successive calls to new/delete 
        // alternate. However, in this case, three calls to setOwner() happen in 
        // succession followed by three calls to delete in succession (with a few 
        // calls to destructors mixed in for fun.) This means that only the final 
        // call to delete (in this chain of events) will have the proper reporting, 
        // and the first two in the chain will not have ANY owner-reporting 
        // information. The deletes will still work fine, we just won't know who 
        // called us.
        //
        // "Then build a stack, my friend!" you might think... but it's a very 
        // common thing that people will be working with third-party libraries 
        // (including MFC under Windows) which is not compiled with this memory 
        // manager's macros. In those cases, setOwner() is never called, and 
        // rightfully should not have the proper trace-back information. So if one 
        // of the destructors in the chain ends up being a call to a delete from 
        // a non-mmgr-compiled library, the stack will get confused.
        //
        // I've been unable to find a solution to this problem, but at least we can 
        // detect it and report the data before we lose it. That's what this is all
        // about. It makes it somewhat confusing to read in the logs, but at least
        // ALL the information is present...
        //
        // There's a caveat here... The compiler is not required to call operator 
        // delete if the value being deleted is NULL. In this case, any call to 
        // delete with a NULL will sill call setOwner(), which will make 
        // setOwner() think that there is a destructor chain becuase we setup the 
        // variables, but nothing gets called to clear them. Because of this we 
        // report a "Possible destructor chain".
        //
        // Thanks to J. Woznack (from Kodiak Interactive Software Studios -- 
        // www.kodiakgames.com) for pointing this out.

        if( sourceLine && alwaysLogAll )
        {
            log( "[I] NOTE! Possible destructor chain: previous owner is %s", 
                ownerString(sourceFile, sourceLine, sourceFunc) );
        }

        // Okay... save this stuff off so we can keep track of the caller
        sourceFile = file;
        sourceLine = line;
        sourceFunc = func;
    }

    void resetGlobals()
    {
        sourceFile = "??";
        sourceLine = 0;
        sourceFunc = "??";
    }

    /** Allocates a portion of memory.
    */
    void* MemoryManager::allocMem(
        const char *sourceFile, 
        const unsigned int sourceLine,  
        const char *sourceFunc, 
        const unsigned int allocationType, 
        const size_t reportedSize, 
        const unsigned processID )
    {
        // If we don't have a process ID yet, get one now
        if( !gProcessID )
            gProcessID = _getProcessID();

        try
        {
            // Increase our allocation count
            currentAllocationCount++;

            // Log the request
            if( alwaysLogAll ) 
                log("[+] %05d %8s of size 0x%08X(%08d) by %s", 
                    currentAllocationCount, 
                    allocationTypes[allocationType], 
                    reportedSize, 
                    reportedSize, 
                    ownerString(sourceFile, sourceLine, sourceFunc) );

            // If you hit this assert, you requested a breakpoint on a specific 
            // allocation count
            m_assert( currentAllocationCount != breakOnAllocationCount );

            // If necessary, grow the reservoir of unused allocation units
            if( !reservoir )
            {
                // Allocate 256 reservoir elements
                reservoir = (sAllocUnit *) malloc( sizeof(sAllocUnit) * 256 );

                // If you hit this assert, then the memory manager failed to 
                // allocate internal memory for tracking the allocations
                m_assert( reservoir != NULL );

                // Danger Will Robinson!
                if( reservoir == NULL ) 
                    throw "Unable to allocate RAM for internal memory tracking data";

                // Build a linked-list of the elements in our reservoir
                memset( reservoir, 0, sizeof(sAllocUnit) * 256 );
                for (unsigned int i = 0; i < 256 - 1; i++)
                {
                    reservoir[i].next = &reservoir[i+1];
                }

                // Add this address to our reservoirBuffer so we can free it later
                sAllocUnit **temp = (sAllocUnit **)realloc( reservoirBuffer, (reservoirBufferSize + 1) * sizeof(sAllocUnit *) );
                m_assert( temp );
                if( temp )
                {
                    reservoirBuffer = temp;
                    reservoirBuffer[reservoirBufferSize++] = reservoir;
                }
            }

            // Logical flow says this should never happen...
            m_assert( reservoir != NULL );

            // Grab a new allocaton unit from the front of the reservoir
            sAllocUnit    *au = reservoir;
            reservoir = au->next;

            // Populate it with some real data
            // HACK the allocator should not need to memset the sAllocUnit
            // memset( au, 0, sizeof(sAllocUnit) );
            au->actualSize = calculateActualSize(reportedSize);
            #ifdef RANDOM_FAILURE
            double    a = rand();
            double    b = RAND_MAX / 100.0 * RANDOM_FAILURE;
            if( a > b )
            {
                au->actualAddress = malloc( au->actualSize );
            }
            else
            {
                log("[F] Random faiure");
                au->actualAddress = NULL;
            }
            #else
            au->actualAddress     = malloc(au->actualSize);
            #endif
            au->reportedSize      = reportedSize;
            au->reportedAddress   = calculateReportedAddress( au->actualAddress );
            au->allocationType    = allocationType;
            au->sourceLine        = sourceLine;
            au->allocationNumber  = currentAllocationCount;
            au->processID         = processID;

            if( sourceFile ) 
                strncpy( au->sourceFile, sourceFileStripper(sourceFile), sizeof(au->sourceFile) - 1 );
            else
                strcpy( au->sourceFile, "??" );

            if( sourceFunc ) 
                strncpy( au->sourceFunc, sourceFunc, sizeof(au->sourceFunc) - 1 );
            else
                strcpy( au->sourceFunc, "??" );

            // We don't want to assert with random failures, because we want the application to deal with them.

            #ifndef RANDOM_FAILURE
            // If you hit this assert, then the requested allocation simply failed 
            // (you're out of memory.) Interrogate the variable 'au' or the stack 
            // frame to see what you were trying to do.
            m_assert( au->actualAddress != NULL );
            #endif

            if( au->actualAddress == NULL )
            {
                throw "Request for allocation failed. Out of memory.";
            }

            // If you hit this assert, then this allocation was made from a source 
            // that isn't setup to use this memory tracking software, use the stack
            // frame to locate the source and include our H file.
            m_assert( allocationType != m_alloc_unknown );

            if( allocationType == m_alloc_unknown )
            {
                log( "[!] Allocation made from outside memory tracker in %s(%d)::%s:", au->sourceFile, au->sourceLine, au->sourceFunc );
                dumpAllocUnit( au, "  " );
            }

            // Insert the new allocation into the hash table
            size_t hashIndex = ((size_t) au->reportedAddress >> 4) & (hashSize - 1);
            if( hashTable[hashIndex]) 
            {
                hashTable[hashIndex]->prev = au;
            }
            au->next = hashTable[hashIndex];
            au->prev = NULL;
            hashTable[hashIndex] = au;

            // Account for the new allocatin unit in our stats
            stats.totalReportedMemory += au->reportedSize;
            stats.totalActualMemory   += au->actualSize;
            stats.totalAllocUnitCount++;

            if( stats.totalReportedMemory > stats.peakReportedMemory )
                stats.peakReportedMemory = stats.totalReportedMemory;
            if( stats.totalActualMemory   > stats.peakActualMemory )
                stats.peakActualMemory   = stats.totalActualMemory;
            if( stats.totalAllocUnitCount > stats.peakAllocUnitCount )
                stats.peakAllocUnitCount = stats.totalAllocUnitCount;

            stats.accumulatedReportedMemory += au->reportedSize;
            stats.accumulatedActualMemory += au->actualSize;
            stats.accumulatedAllocUnitCount++;

            // Prepare the allocation unit for use (wipe it with recognizable garbage)
            wipeWithPattern(au, unusedPattern);

            // calloc() expects the reported memory address range to be filled with 0's
            memset( au->reportedAddress, 0, au->reportedSize );
            
            // Validate every single allocated unit in memory
            if( alwaysValidateAll )
                validateAllAllocs();

            // Log the result
            if( alwaysLogAll )
                log("[+] ---->             addr 0x%08X", (size_t) au->reportedAddress);

            // Resetting the globals insures that if at some later time, somebody 
            // calls our memory manager from an unknown source (i.e. they didn't 
            // include our H file) then we won't think it was the last allocation.
            resetGlobals();
            
            // Return the (reported) address of the new allocation unit
            return au->reportedAddress;
        }
        catch( const char *err )
        {
            // Deal with the errors

            log("[!] %s", err);
            resetGlobals();

            return NULL;
        }
    }

    /** Memory reallocator.
    */
    void * MemoryManager::rllocMem(
        const char *sourceFile, 
        const unsigned int sourceLine, 
        const char *sourceFunc, 
        const unsigned int reallocationType, 
        const size_t reportedSize, 
        void *reportedAddress, 
        const unsigned processID )
    {
        // If we don't have a process ID yet, get one now
        if( !gProcessID )
            gProcessID = _getProcessID();

        try
        {
            // ANSI says: Calling realloc with a NULL should force same operations 
            // as a malloc
            if( !reportedAddress )
            {
                return allocMem(sourceFile, sourceLine, sourceFunc, reallocationType, reportedSize, processID );
            }

            // Increase our allocation count
            currentAllocationCount++;

            // If you hit this assert, you requested a breakpoint on a specific 
            // allocation count
            m_assert( currentAllocationCount != breakOnAllocationCount );

            // Log the request
            if( alwaysLogAll ) 
                log("[~] %05d %8s of size 0x%08X(%08d) by %s", 
                currentAllocationCount, 
                allocationTypes[reallocationType], 
                reportedSize, 
                reportedSize, 
                ownerString(sourceFile, sourceLine, sourceFunc) );

            // Locate the existing allocation unit
            sAllocUnit *au = findAllocUnit( reportedAddress );

            // If you hit this assert, you tried to reallocate RAM that wasn't 
            // allocated by this memory manager.
            m_assert(au != NULL);
            if( au == NULL )
                throw "Request to reallocate RAM that was never allocated";

            // If you hit this assert, then the allocation unit that is about to be
            // reallocated is damaged. But you probably already know that from a 
            // previous assert you should have seen in validateAllocUnit() :)
            m_assert( validateAlloc( au ) );        

            // If you hit this assert, then this reallocation was made from a source 
            // that isn't setup to use this memory tracking software, use the stack
            // frame to locate the source and include our H file.
            m_assert( reallocationType != m_alloc_unknown );
            if( reallocationType == m_alloc_unknown )
            {
                log( "[!] Allocationfrom outside memory tracker in %s(%d)::%s :", sourceFile, sourceLine, sourceFunc );
                dumpAllocUnit( au, "  " );
            }

            // If you hit this assert, you were trying to reallocate RAM that was 
            // not allocated in a way that is compatible with realloc. In other 
            // words, you have a allocation/reallocation mismatch.
            m_assert(
                au->allocationType == m_alloc_malloc ||
                au->allocationType == m_alloc_calloc ||
                au->allocationType == m_alloc_realloc);
            if( reallocationType == m_alloc_unknown )
            {
                log( "[!] Allocation-deallocation mismatch in %s(%d)::%s :", sourceFile, sourceLine, sourceFunc );
                dumpAllocUnit( au, "  " );
            }

            // If you hit this assert, then the "break on realloc" flag for this 
            // allocation unit is set (and will continue to be set until you 
            // specifically shut it off. Interrogate the 'au' variable to determine
            // information about this allocation unit.
            m_assert( au->breakOnRealloc == false );

            // Keep track of the original size
            size_t originalReportedSize = au->reportedSize;

            if (alwaysLogAll) log("[~] ---->             from 0x%08X(%08d)", 
                originalReportedSize, 
                originalReportedSize);

            // Do the reallocation
            void   *oldReportedAddress = reportedAddress;
            size_t newActualSize = calculateActualSize(reportedSize);
            void   *newActualAddress = NULL;

            #ifdef RANDOM_FAILURE

            double    a = rand();
            double    b = RAND_MAX / 100.0 * RANDOM_FAILURE;
            if (a > b)
            {
                newActualAddress = realloc(au->actualAddress, newActualSize);
            }
            else
            {
                log("[F] Random faiure");
            }

            #else

            newActualAddress = realloc(au->actualAddress, newActualSize);

            #endif

            // We don't want to assert with random failures, because we want the 
            // application to deal with them.

            #ifndef RANDOM_FAILURE
            // If you hit this assert, then the requested allocation simply failed 
            // (you're out of memory) Interrogate the variable 'au' to see the 
            // original allocation. You can also query 'newActualSize' to see the 
            // amount of memory trying to be allocated. Finally, you can query 
            // 'reportedSize' to see how much memory was requested by the caller.
            m_assert(newActualAddress);
            #endif

            if (!newActualAddress) 
                throw "Request for reallocation failed. Out of memory.";

            // Remove this allocation from our stats (we'll add the new reallocation again later)
            stats.totalReportedMemory -= au->reportedSize;
            stats.totalActualMemory   -= au->actualSize;

            // Update the allocation with the new information

            au->actualSize        = newActualSize;
            au->actualAddress     = newActualAddress;
            au->reportedSize      = calculateReportedSize(newActualSize);
            au->reportedAddress   = calculateReportedAddress(newActualAddress);
            au->allocationType    = reallocationType;
            au->sourceLine        = sourceLine;
            au->allocationNumber  = currentAllocationCount;
            au->processID         = processID;
            if( sourceFile )
                strncpy(au->sourceFile, sourceFileStripper(sourceFile), sizeof(au->sourceFile) - 1);
            else
                strcpy( au->sourceFile, "??" );
            if( sourceFunc )
                strncpy( au->sourceFunc, sourceFunc, sizeof(au->sourceFunc) - 1 );
            else
                strcpy( au->sourceFunc, "??" );

            // The reallocation may cause the address to change, so we should 
            // relocate our allocation unit within the hash table

            size_t hashIndex = (unsigned int) -1;
            if( oldReportedAddress != au->reportedAddress )
            {
                // Remove this allocation unit from the hash table
                {
                    size_t hashIndex = ((size_t) oldReportedAddress >> 4) & (hashSize - 1);
                    if( hashTable[hashIndex] == au )
                    {
                        hashTable[hashIndex] = hashTable[hashIndex]->next;
                    }
                    else
                    {
                        if (au->prev)
                            au->prev->next = au->next;
                        if (au->next)
                            au->next->prev = au->prev;
                    }
                }

                // Re-insert it back into the hash table
                hashIndex = ((size_t) au->reportedAddress >> 4) & (hashSize - 1);
                if (hashTable[hashIndex]) 
                    hashTable[hashIndex]->prev = au;
                au->next = hashTable[hashIndex];
                au->prev = NULL;
                hashTable[hashIndex] = au;
            }

            // Account for the new allocatin unit in our stats
            stats.totalReportedMemory += au->reportedSize;
            stats.totalActualMemory   += au->actualSize;
            if (stats.totalReportedMemory > stats.peakReportedMemory) 
                stats.peakReportedMemory = stats.totalReportedMemory;
            if (stats.totalActualMemory   > stats.peakActualMemory)   
                stats.peakActualMemory   = stats.totalActualMemory;
            size_t deltaReportedSize = reportedSize - originalReportedSize;
            if( deltaReportedSize > 0 )
            {
                stats.accumulatedReportedMemory += deltaReportedSize;
                stats.accumulatedActualMemory += deltaReportedSize;
            }

            // Prepare the allocation unit for use (wipe it with recognizable 
            // garbage)
            wipeWithPattern( au, unusedPattern, originalReportedSize );

            // If you hit this assert, then something went wrong, because the 
            // allocation unit was properly validated PRIOR to the reallocation. 
            // This should not happen.
            m_assert( validateAlloc(au) );

            // Validate every single allocated unit in memory
            if( alwaysValidateAll ) 
                validateAllAllocs();

            // Log the result
            if (alwaysLogAll) log("[~] ---->             addr 0x%08X", 
                (size_t) au->reportedAddress);

            // Resetting the globals insures that if at some later time, somebody 
            // calls our memory manager from an unknown source (i.e. they didn't 
            // include our H file) then we won't think it was the last allocation.
            resetGlobals();

            // Return the (reported) address of the new allocation unit
            return au->reportedAddress;
        }
        catch(const char *err)
        {
            // Deal with the errors
            log("[!] %s", err);
            resetGlobals();

            return NULL;
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    // Deallocate memory and track it
    // ---------------------------------------------------------------------------------------------------------------------------------

    void MemoryManager::dllocMem(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc, const unsigned int deallocationType, const void *reportedAddress, const unsigned processID )
    {
		// early-out for NULL
		if (!reportedAddress)
			return;

        try
        {
            // Log the request
            if (alwaysLogAll) log("[-] ----- %8s of addr 0x%08X           by %s", 
                allocationTypes[deallocationType], 
                (size_t) reportedAddress, 
                ownerString(sourceFile, sourceLine, sourceFunc) );

            // Go get the allocation unit

            sAllocUnit *au = findAllocUnit( reportedAddress );

            // If you hit this assert, you tried to deallocate RAM that wasn't 
            // allocated by this memory manager.
            m_assert(au != NULL);
            if (au == NULL) 
                throw "Request to deallocate RAM that was never allocated";

            // If you hit this assert, then the allocation unit that is about to be
            // deallocated is damaged. But you probably already know that from a 
            // previous assert you should have seen in validateAllocUnit() :)
            m_assert(validateAlloc(au));

            // If you hit this assert, then this deallocation was made from a 
            // source that isn't setup to use this memory tracking software, use 
            // the stack frame to locate the source and include our H file.
            m_assert(deallocationType != m_alloc_unknown);
            if( deallocationType == m_alloc_unknown )
            {
                log( "[!] Allocation-deallocation mismatch in %s(%d)::%s :", sourceFile, sourceLine, sourceFunc );
                dumpAllocUnit( au, "  " );
            }

            // If you hit this assert, you were trying to deallocate RAM that was 
            // not allocated in a way that is compatible with the deallocation 
            // method requested. In other words, you have a allocation/deallocation 
            // mismatch.
            m_assert(
                (deallocationType == m_alloc_delete       && au->allocationType == m_alloc_new      ) ||
                (deallocationType == m_alloc_delete_array && au->allocationType == m_alloc_new_array) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_malloc   ) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_calloc   ) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_realloc  ) ||
                (deallocationType == m_alloc_unknown                                                ) );
            if( 
                !(
                (deallocationType == m_alloc_delete       && au->allocationType == m_alloc_new      ) ||
                (deallocationType == m_alloc_delete_array && au->allocationType == m_alloc_new_array) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_malloc   ) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_calloc   ) ||
                (deallocationType == m_alloc_free         && au->allocationType == m_alloc_realloc  ) ||
                (deallocationType == m_alloc_unknown                                                ) ) )
            {
                log( "[!] Allocation-deallocation mismatch in %s(%d)::%s :", sourceFile, sourceLine, sourceFunc );
                dumpAllocUnit( au, "  " );
            }

            // If you hit this assert, then this deallocation was made from another
            // process than the one in which the allocation was made.
            // m_assert( au->processID == processID );

            // If you hit this assert, then the "break on dealloc" flag for this 
            // allocation unit is set. Interrogate the 'au'
            // variable to determine information about this allocation unit.
            m_assert(au->breakOnDealloc == false);

            // Wipe the deallocated RAM with a new pattern. This doen't actually do 
            // us much good in debug mode under WIN32, because Microsoft's memory 
            // debugging & tracking utilities will wipe it right after we do. 
            // Oh well.
            wipeWithPattern( au, releasedPattern );

            // Do the deallocation
            free(au->actualAddress);

            // Remove this allocation unit from the hash table
            size_t hashIndex = ((size_t) au->reportedAddress >> 4) & (hashSize - 1);
            if( hashTable[hashIndex] == au )
            {
                hashTable[hashIndex] = au->next;
            }
            else
            {
                if (au->prev)
                    au->prev->next = au->next;
                if (au->next)
                    au->next->prev = au->prev;
            }

            // Remove this allocation from our stats
            stats.totalReportedMemory -= au->reportedSize;
            stats.totalActualMemory   -= au->actualSize;
            stats.totalAllocUnitCount--;

            // Add this allocation unit to the front of our reservoir of unused allocation units
            memset( au, 0, sizeof(sAllocUnit) );
            au->next = reservoir;
            reservoir = au;

            // Resetting the globals insures that if at some later time, somebody 
            // calls our memory manager from an unknown source (i.e. they didn't 
            // include our H file) then we won't think it was the last allocation.
            resetGlobals();

            // Validate every single allocated unit in memory
            if( alwaysValidateAll )
                validateAllAllocs();

            // If we're in the midst of  deinitialization time, track any pending memory leaks
            if( m_bDeinitTime )
                dumpLeakReport();
        }
        catch(const char *err)
        {
            // Deal with errors
            log("[!] %s", err);
            resetGlobals();
        }
    }

    /** By using this function you can be proactive in tracking bugs by being able
        to check wether a memory address has been allocated by the memory manager.
    */
    bool MemoryManager::validateAddr( const void *reportedAddress )
    {
        // Just see if the address exists in our allocation routines
        return findAllocUnit(reportedAddress) != NULL;
    }

    bool MemoryManager::validateAlloc( const sAllocUnit *allocUnit )
    {
        // Make sure the padding is untouched
        long *pre = (long *)allocUnit->actualAddress;
        long *post = (long *)((char *)allocUnit->actualAddress + allocUnit->actualSize - paddingSize * sizeof(long));
        bool errorFlag = false;
        for( unsigned int i = 0; i < paddingSize; i++, pre++, post++ )
        {
            if( *pre != (long)prefixPattern )
            {
                log("[!] A memory allocation unit was corrupt because of an underrun:");
                dumpAllocUnit( allocUnit, "  " );
                errorFlag = true;
            }

            // If you hit this assert, then you should know that this allocation 
            // unit has been damaged. Something (possibly the owner?) has underrun 
            // the allocation unit (modified a few bytes prior to the start). You 
            // can interrogate the variable 'allocUnit' to see statistics and 
            // information about this damaged allocation unit.
            m_assert(*pre == (long) prefixPattern);

            if (*post != (long) postfixPattern)
            {
                log("[!] A memory allocation unit was corrupt because of an overrun:");
                dumpAllocUnit(allocUnit, "  ");
                errorFlag = true;
            }

            // If you hit this assert, then you should know that this allocation 
            // unit has been damaged. Something (possibly the owner?) has overrun 
            // the allocation unit (modified a few bytes after the end). You can 
            // interrogate the variable 'allocUnit' to see statistics and 
            // information about this damaged allocation unit.
            m_assert(*post == (long) postfixPattern);
        }

        // Return the error status (we invert it, because a return of 'false' means error)
        return !errorFlag;
    }

    bool MemoryManager::validateAllAllocs()
    {
        // Just go through each allocation unit in the hash table and count the ones that have errors
        unsigned int errors = 0;
        unsigned int allocCount = 0;

        for( unsigned int i = 0; i < hashSize; i++ )
        {
            sAllocUnit *ptr = hashTable[i];
            while(ptr)
            {
                allocCount++;
                if (!validateAlloc(ptr)) 
                    errors++;
                ptr = ptr->next;
            }
        }

        // Test for hash-table correctness
        if( allocCount != stats.totalAllocUnitCount )
        {
            log("[!] Memory tracking hash table corrupt!");
            errors++;
        }

        // If you hit this assert, then the internal memory (hash table) used by 
        // this memory tracking software is damaged! The best way to track this 
        // down is to use the alwaysLogAll flag in conjunction with STRESS_TEST 
        // macro to narrow in on the offending code. After running the application 
        // with these settings (and hitting this assert again), interrogate the 
        // memory.log file to find the previous successful operation. The 
        // corruption will have occurred between that point and this assertion.
        m_assert( allocCount == stats.totalAllocUnitCount );

        // If you hit this assert, then you've probably already been notified that 
        // there was a problem with a allocation unit in a prior call to 
        // validateAllocUnit(), but this assert is here just to make sure you know 
        // about it. :)
        m_assert( errors == 0 );

        // Log any errors
        if (errors) 
            log("[!] While validating all allocation units, %d allocation unit(s) were found to have problems", 
            errors );

        // Return the error status
        return errors != 0;
    }

    /** Determines how much RAM is unused.
    */
    unsigned int MemoryManager::calcUnused( const sAllocUnit *allocUnit )
    {
        const unsigned long *ptr = (const unsigned long *)allocUnit->reportedAddress;
        unsigned int count = 0;

        for( unsigned int i = 0; i < allocUnit->reportedSize; i += sizeof(long), ptr++ )
        {
            if (*ptr == unusedPattern) count += sizeof(long);
        }

        return count;
    }

    unsigned int MemoryManager::calcAllUnused()
    {
        // Just go through each allocation unit in the hash table and count the 
        // unused RAM
        unsigned int total = 0;
        for( unsigned int i = 0; i < hashSize; i++ )
        {
            sAllocUnit *ptr = hashTable[i];
            while(ptr)
            {
                total += calcUnused(ptr);
                ptr = ptr->next;
            }
        }

        return total;
    }

    void MemoryManager::dumpAllocUnit(const sAllocUnit *allocUnit, const char *prefix)
    {
        log("[I] %sAddress (reported): %010p",       prefix, allocUnit->reportedAddress);
        log("[I] %sAddress (actual)  : %010p",       prefix, allocUnit->actualAddress);
        log("[I] %sSize (reported)   : 0x%08X (%s)", prefix, allocUnit->reportedSize, 
                                                    memorySizeString(allocUnit->reportedSize));
        log("[I] %sSize (actual)     : 0x%08X (%s)", prefix, allocUnit->actualSize, 
                                                    memorySizeString(allocUnit->actualSize));
        log("[I] %sOwner             : %s(%d)::%s",  prefix, allocUnit->sourceFile, allocUnit->sourceLine, allocUnit->sourceFunc);
        log("[I] %sAllocation type   : %s",          prefix, allocationTypes[allocUnit->allocationType]);
        log("[I] %sAllocation number : %d",          prefix, allocUnit->allocationNumber);
    }

    void MemoryManager::dumpMemReport(const char *filename, const bool overwrite)
    {
        // Open the report file
        FILE    *fp = NULL;
        
        if (overwrite)    
            fp = fopen(filename, "w+b");
        else        
            fp = fopen(filename, "ab");

        // If you hit this assert, then the memory report generator is unable to 
        // log information to a file (can't open the file for some reason.)
        m_assert(fp);
        if (!fp) 
            return;

        // Header
        char    timeString[25];
        memset(timeString, 0, sizeof(timeString));
        time_t  t = time(NULL);
        struct  tm *tme = localtime(&t);

        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                             Memory report for: %02d/%02d/%04d %02d:%02d:%02d                                     |\r\n", tme->tm_mon + 1, tme->tm_mday, tme->tm_year + 1900, tme->tm_hour, tme->tm_min, tme->tm_sec);
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "\r\n");
        fprintf(fp, "\r\n");

        // Report summary
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                                           T O T A L S                                                            |\r\n");
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "              Allocation unit count: %10s\r\n", insertCommas(stats.totalAllocUnitCount));
        fprintf(fp, "            Reported to application: %s\r\n", memorySizeString(stats.totalReportedMemory));
        fprintf(fp, "         Actual total memory in use: %s\r\n", memorySizeString(stats.totalActualMemory));
        fprintf(fp, "           Memory tracking overhead: %s\r\n", memorySizeString(stats.totalActualMemory - stats.totalReportedMemory));
        fprintf(fp, "\r\n");

        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                                            P E A K S                                                             |\r\n");
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "              Allocation unit count: %10s\r\n", insertCommas(stats.peakAllocUnitCount));
        fprintf(fp, "            Reported to application: %s\r\n", memorySizeString(stats.peakReportedMemory));
        fprintf(fp, "                             Actual: %s\r\n", memorySizeString(stats.peakActualMemory));
        fprintf(fp, "           Memory tracking overhead: %s\r\n", memorySizeString(stats.peakActualMemory - stats.peakReportedMemory));
        fprintf(fp, "\r\n");

        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                                      A C C U M U L A T E D                                                       |\r\n");
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "              Allocation unit count: %s\r\n", memorySizeString(stats.accumulatedAllocUnitCount));
        fprintf(fp, "            Reported to application: %s\r\n", memorySizeString(stats.accumulatedReportedMemory));
        fprintf(fp, "                             Actual: %s\r\n", memorySizeString(stats.accumulatedActualMemory));
        fprintf(fp, "\r\n");

        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "|                                                           U N U S E D                                                            |\r\n");
        fprintf(fp, " ---------------------------------------------------------------------------------------------------------------------------------- \r\n");
        fprintf(fp, "    Memory allocated but not in use: %s\r\n", memorySizeString(calcAllUnused()));
        fprintf(fp, "\r\n");

        dumpAllocations(fp);

        fclose(fp);
    }

    sMStats    MemoryManager::getMemStats()
    {
        return stats;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    // Global new/new[]
    //
    // These are the standard new/new[] operators. They are merely interface functions that operate like normal new/new[], but use our
    // memory tracking routines.
    // ---------------------------------------------------------------------------------------------------------------------------------
    void *MemoryManager::op_new_sc( size_t reportedSize, unsigned processID )
    {
        // Save these off...
        const    char        *file = sourceFile;
        const    unsigned int line = sourceLine;
        const    char        *func = sourceFunc;

        // ANSI says: allocation requests of 0 bytes will still return a valid value
        if (reportedSize == 0) reportedSize = 1;

        // ANSI says: loop continuously because the error handler could possibly free up some memory
        for(;;)
        {
            // Try the allocation
            void *ptr = allocMem(file, line, func, m_alloc_new, reportedSize, processID );
            if( ptr )
            {
                return ptr;
            }

            // There isn't a way to determine the new handler, except through setting it. So we'll just set it to NULL, then
            // set it back again.
            std::new_handler nh = std::set_new_handler(0);
            std::set_new_handler(nh);

            // If there is an error handler, call it
            if (nh)
            {
                (*nh)();
            }

            // Otherwise, throw the exception
            else
            {
                throw std::bad_alloc();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void *MemoryManager::op_new_vc( size_t reportedSize, unsigned processID )
    {
        // Save these off...
        const    char        *file = sourceFile;
        const    unsigned int line = sourceLine;
        const    char        *func = sourceFunc;

        // The ANSI standard says that allocation requests of 0 bytes will still return a valid value
        if (reportedSize == 0) reportedSize = 1;

        // ANSI says: loop continuously because the error handler could possibly free up some memory
        for(;;)
        {
            // Try the allocation
            void    *ptr = allocMem(file, line, func, m_alloc_new_array, reportedSize, processID );
            if( ptr )
            {
                return ptr;
            }

            // There isn't a way to determine the new handler, except through setting it. So we'll just set it to NULL, then
            // set it back again.
            std::new_handler    nh = std::set_new_handler(0);
            std::set_new_handler(nh);

            // If there is an error handler, call it
            if (nh)
            {
                (*nh)();
            }

            // Otherwise, throw the exception
            else
            {
                throw std::bad_alloc();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    // Other global new/new[]
    //
    // These are the standard new/new[] operators as used by Microsoft's memory tracker. We don't want them interfering with our memory
    // tracking efforts. Like the previous versions, these are merely interface functions that operate like normal new/new[], but use
    // our memory tracking routines.
    // ---------------------------------------------------------------------------------------------------------------------------------
    void *MemoryManager::op_new_sc( size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID )
    {
        // The ANSI standard says that allocation requests of 0 bytes will still 
        // return a valid value
        if( reportedSize == 0 )
            reportedSize = 1;

        // ANSI says: loop continuously because the error handler could possibly 
        // free up some memory
        for(;;)
        {
            // Try the allocation

            void    *ptr = allocMem(sourceFile, sourceLine, "??", m_alloc_new, reportedSize, processID );
            if (ptr)
            {
                return ptr;
            }

            // There isn't a way to determine the new handler, except through setting it. So we'll just set it to NULL, then
            // set it back again.

            std::new_handler    nh = std::set_new_handler(0);
            std::set_new_handler(nh);

            // If there is an error handler, call it

            if (nh)
            {
                (*nh)();
            }

            // Otherwise, throw the exception

            else
            {
                throw std::bad_alloc();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void *MemoryManager::op_new_vc(size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID )
    {
        // ANSI says : allocation requests of 0 bytes will still return a valid 
        // value
        if( reportedSize == 0 )
            reportedSize = 1;

        // ANSI says: loop continuously because the error handler could possibly 
        // free up some memory

        for(;;)
        {
            // Try the allocation
            void *ptr = allocMem(
                sourceFile, 
                sourceLine, 
                "??", 
                m_alloc_new_array, 
                reportedSize, 
                processID );
            if( ptr )
            {
                return ptr;
            }

            // There isn't a way to determine the new handler, except through 
            // setting it. So we'll just set it to NULL, then set it back again.
            std::new_handler nh = std::set_new_handler(0);
            std::set_new_handler(nh);

            // If there is an error handler, call it
            if( nh )
            {
                (*nh)();
            }

            // Otherwise, throw the exception
            else
            {
                throw std::bad_alloc();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    // Global delete/delete[]
    //
    // These are the standard delete/delete[] operators. They are merely interface functions that operate like normal delete/delete[],
    // but use our memory tracking routines.
    // ---------------------------------------------------------------------------------------------------------------------------------
    void MemoryManager::op_del_sc(void *reportedAddress, unsigned processID )
    {
        // ANSI says: delete & delete[] allow NULL pointers (they do nothing)
        if( reportedAddress )
            dllocMem(sourceFile, sourceLine, sourceFunc, m_alloc_delete, reportedAddress, processID );
        else if( alwaysLogAll )
            log("[-] ----- %8s of NULL                      by %s", 
            allocationTypes[m_alloc_delete], 
            ownerString(sourceFile, sourceLine, sourceFunc) );

        // Resetting the globals insures that if at some later time, somebody calls
        // our memory manager from an unknown source (i.e. they didn't include our 
        // H file) then we won't think it was the last allocation.
        resetGlobals();
    }

    // ---------------------------------------------------------------------------------------------------------------------------------
    void MemoryManager::op_del_vc(void *reportedAddress, unsigned processID )
    {
        // ANSI says: delete & delete[] allow NULL pointers (they do nothing)
        if (reportedAddress) 
            dllocMem(
                sourceFile, 
                sourceLine, 
                sourceFunc, 
                m_alloc_delete_array, 
                reportedAddress, 
                processID );
        else if( alwaysLogAll )
            log("[-] ----- %8s of NULL                      by %s", allocationTypes[m_alloc_delete_array], ownerString(sourceFile, sourceLine, sourceFunc));

        // Resetting the globals insures that if at some later time, somebody calls
        // our memory manager from an unknown source (i.e. they didn't include our 
        // H file) then we won't think it was the last allocation.
        resetGlobals();
    }

    MemoryManager::MemoryManager()
        : m_uProcessIDs( 0 ), m_bDeinitTime( false )
    {
        doCleanupLogOnFirstRun();
    }

    MemoryManager::~MemoryManager()
    {
        m_bDeinitTime = true;
        dumpLeakReport();    
    }

    unsigned MemoryManager::_getProcessID()
    {
        return ++m_uProcessIDs;
    }
 
#else

    //-----------------------------------------------------------------------------
    MemoryManager::MemoryManager()
    {    
    }

    //-----------------------------------------------------------------------------
    MemoryManager::~MemoryManager()
    {                
    }

    //-----------------------------------------------------------------------------
    void * MemoryManager::allocMem( const char *szFile, size_t uLine, 
            size_t count ) throw()
    {
        void *ptr = malloc( count );
        return ptr;
    }        

    //-----------------------------------------------------------------------------
    void * MemoryManager::rllocMem( 
        const char *szFile, size_t uLine, void *ptr , size_t count ) throw()
    {
        void *nptr = realloc( ptr, count );
        return nptr;
    }

    //-----------------------------------------------------------------------------
    void * MemoryManager::cllocMem( 
        const char *szFile, size_t uLine, size_t num, size_t size ) throw()
    {
        void *ptr = malloc( num * size );

        if( ptr )
        {
            memset( ptr , 0, num * size );
        }
        return ptr;
    }

    //-----------------------------------------------------------------------------
    void MemoryManager::dllocMem( const char *szFile, size_t uLine, 
            void *ptr) throw()
    {
        free( ptr );
    }

#endif // OGRE_DEBUG_MEMORY_MANAGER

}

