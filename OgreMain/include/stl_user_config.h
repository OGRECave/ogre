/*
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

/*
 * Purpose of this file :
 *
 * To hold user-definable portion of STLport settings which may be overridden
 * on per-project basis.
 * Please note that if you use STLport iostreams (compiled library) then you have
 * to use consistent settings when you compile STLport library and your project. 
 * Those settings are defined in _site_config.h and have to be the same for a given
 * STLport installation.
 *
 */


//==========================================================
// User-settable macros that control compilation:
//              Features selection
//==========================================================

/* _STLP_NO_OWN_IOSTREAMS: 
 * __STL_NO_SGI_IOSTREAMS (in older versions)
 *  This is major configuration switch.
 *  Turn it on to disable use of SGI iostreams and use wrappers 
 *  around your compiler's iostreams, like before.
 *  Keep it off if you want to use  SGI iostreams 
 *  (Note that in this case you have to compile library in ../src
 *  and supply resulting library at link time).
 *
 */

// # define   _STLP_NO_OWN_IOSTREAMS	1


/* 
 * This macro only works in non-SGI iostreams mode.
 *
 * Uncomment to suppress using new-style streams even if they are
 * available.
 * Beware - _STLP_USE_OWN_NAMESPACE depends on this macro, too.
 * Do that only if you are absolutely sure backwards-compatible 
 * <iostream.h> is not actually a wrapper with <iostream>
 * Hint : In VC++ 6.x, they are not.
 */

// #define   _STLP_NO_NEW_IOSTREAMS	1

/*
 * Use this switch for embedded systems where no iostreams are available
 * at all. STLport own iostreams will also get disabled automatically then.
 */
// # define _STLP_NO_IOSTREAMS 1

/* 
 * Set _STLP_DEBUG to turn the "Debug Mode" on.
 * That gets you checked iterators/ranges in the manner
 * of "Safe STL". Very useful for debugging. Thread-safe.
 * Please do not forget to link proper STLport library flavor
 * (e.g libstlport_gcc_stldebug.a) when you set this flag in STLport iostreams mode.
 */
#ifdef _DEBUG
	#define _STLP_DEBUG 1
#endif


/* 
 *
 *  _STLP_NO_CUSTOM_IO : define this if you do not instantiate basic_xxx iostream classes  with custom types (which is most likely the case).
 *  Custom means types other than char, wchar and char_traits<>,
 *  like basic_ostream<my_char_type, my_traits<my_char_type> >
 *  When this option is on, most non-inline template functions definitions for iostreams are not seen by the client.
 *  Default is off, just not to break compilation for those who do use those types.
 *  which saves a lot of compile time for most compilers, also object and executable size for some.
 *  That also guarantees that you still use optimized standard i/o when you compile your program without optimization. 
 *  Option does not affect STLport library build; you may use the same binary library with and without this option, 
 *  on per-project basis.
 *
 */
//#define _STLP_NO_CUSTOM_IO


/* 
 * _STLP_NO_RELOPS_NAMESPACE: if defined, don't put the relational
 * operator templates (>, <=. >=, !=) in namespace std::rel_ops, even
 * if the compiler supports namespaces.
 * Note : if the compiler do not support namespaces, those operators are not be provided by default,
 * to simulate hiding them into rel_ops. This was proved to resolve many compiler bugs with ambiguity.
 */

// #define _STLP_NO_RELOPS_NAMESPACE 1


/*
 * If _STLP_USE_OWN_NAMESPACE is in effect, STLport by default will not try
 * to rename std:: for the user
 * to _STL::. If you do want this feature, please define the following switch :
 */
// # define _STLP_REDEFINE_STD 1


/*
 * _STLP_WHOLE_NATIVE_STD : only meaningful in _STLP_USE_OWN_NAMESPACE mode.
 * Normally, STLport only imports necessary components from native std:: namespace -
 * those not yet provided by STLport (<iostream>, <complex>, etc.) 
 * and their dependencies (<string>, <stdexcept>). 
 * You might want everything from std:: being available in std:: namespace when you
 * include corresponding STLport header (like STLport <map> provides std::map as well, etc.),
 * if you are going to use both stlport:: and std:: components in your code.
 * Otherwise this option is not recommended as it increases the size of your object files
 * and slows down compilation.
 */
// # define _STLP_WHOLE_NATIVE_STD


/*
 * Use this option to catch uninitialized members in your classes.
 * When it is set, construct() and destroy() fill the class storage
 * with _STLP_SHRED_BYTE (see below). 
 * Note : _STLP_DEBUG and _STLP_DEBUG_ALLOC don't set this option automatically.
 */

// # define _STLP_DEBUG_UNINITIALIZED 1

/*
 * Uncomment and provide a definition for the byte with which raw memory
 * will be filled if _STLP_DEBUG_ALLOC or _STLP_DEBUG_UNINITIALIZED is defined. 
 * Choose a value which is likely to cause a noticeable problem if dereferenced 
 * or otherwise abused. A good value may already be defined for your platform; see
 * stl_config.h
 */
// #define _STLP_SHRED_BYTE 0xA3

/*
 *  This option is for gcc users only and only affects systems where native linker
 *  does not let gcc to implement automatic instantiation of static template data members/
 *  It is being put in this file as there is no way to check if we are using GNU ld automatically,
 *  so it becomes user's responsibility.
 * 
 */

// #define _STLP_GCC_USES_GNU_LD


//==========================================================
// Compatibility section
//==========================================================

/*
 *  Define this macro to disable anachronistic constructs (like the ones used in HP STL and
 *  not included in final standard, etc. 
 */
// define _STLP_NO_ANACHRONISMS 1

/*
 *  Define this macro to disable STLport extensions (for example, to make sure your code will 
 *  compile with some other implementation )
 */
// define _STLP_NO_EXTENSIONS   1


/* 
 * You should define this macro if compiling with MFC - STLport <stl/_config.h>
 * then include <afx.h> instead of <windows.h> to get synchronisation primitives 
 *
 */

// # define _STLP_USE_MFC 1


// boris : this setting is here as we cannot detect precense of new Platform SDK automatically 
// If you are using new PSDK with VC++ 6.0 or lower, please define this to get correct prototypes for InterlockedXXX functions
//# define _STLP_NEW_PLATFORM_SDK 1

/*
 * Use minimum set of default arguments on template classes that have more
 * than one - for example map<>, set<>.
 * This has effect only if _STLP_LIMITED_DEFAULT_TEMPLATES is on.
 * If _STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS is set, you'll be able to compile
 * set<T> with those compilers, but you'll have to use __set__<T, less<T>>
 *
 * Affects : map<>, multimap<>, set<>, multiset<>, hash_*<>, 
 * queue<>, priority_queue<>, stack<>, istream_iterator<>
 */

// # define _STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS 1

//==========================================================

// Local Variables:
// mode:C++
// End:
