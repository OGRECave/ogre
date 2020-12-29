# OGRE Coding Standards

This document describes the coding standards all developers are expected to adhere to when writing code for the OGRE project.

## Top-level organisation issues

<ol>
    <li>All source files must begin with the standard OGRE copyright statement:    <pre>// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT</pre>
    </li>
    <li>All publicly visible classes should be declared in their own header file using the .h extension, placed in the `include` folder of the sub-project in question, and named after the class but prefixed with `Ogre` e.g. `OgreMyClass.h`. Only very tightly related classes should be declared in the same header file. </li>
    <li>Headers of private classes must be placed in the `src` folder of the sub-project in question</li>
    <li>Implementations should be placed in a source file called the same name as the class but with an extension of .cpp.</li>
    <li>Everything must be declared inside the namespace `Ogre`.</li>
</ol>

## Portablity

<ol>
    <li>All code must be cross platform, ANSI C++. No dependencies on platform-specific headers and / or types are allowed (the only exception is when dealing with platform-specific features like windowing, which must be implemented for each platform separately).</li>
    <li>If you serialise / deserialise any data, subclass from Serializer and use its methods, it will insulate you from endianness issues. If you need to serialise any types it doesn't already handle, make sure you deal with endianness issues in the same way Serializer does (ie use native endianness by default but detect the inverse on reading and perform flipping if required).</li>
</ol>

## C++ Standards compliance

<ol>
    <li>Always prefer the STL over custom containers / algorithms.</li>
    <li>Always prefer C++ techniques over C.
        <ul><li>Avoid C-strings (`char*` and functions like sprintf, strcpy, use `Ogre::String`)</li>
            <li>Avoid old I/O routines (fopen et al, use `<iostream>`)</li>
            <li>Use abstract classes or templates not `void*`</li>
            <li>Use overloaded methods not varargs.</li>
        </ul></li>
    <li>Minimum C++ compiler level is MSVC 12 (VS2013) or gcc 4.8. Compilers which do not support C++11 properly are not supported.</li>
    <li>Use the <a href="https://en.cppreference.com/w/cpp/language/pimpl">PImpl idiom</a> to reduce dependencies between classes.</li>
    <li>Always use <a href="https://isocpp.org/wiki/faq/const-correctness">const-correctness</a>. Methods taking non-primitive types as parameters should generally take them as const references, methods returning non-primitive types should generally return them as const references. Declare all methods that do not modify internal state `const`. For lazy-update getter methods, declare the internal state which is lazy-updated `mutable`.</li>
    <li>Prefer `private` over `protected` to encourage encapsulation. Use public interfaces internally too.</li>
    <li>Always declare destructors `virtual` unless the class you are writing should not have any vtable (no other virtual methods).</li>
    <li>Avoid non-const by-ref parameters unless you have no other option. We prefer not to have inout parameters since they are less intuitive.</li>
</ol>


## Naming conventions &amp; Documentation

<ol>
    <li>Classes, types and structures must be title case (MyNewClass). </li>
    <li>Methods and local variables must be camel case (myNewMethod). </li>
    <li>Member variables should be prefixed with `m` (mInstanceVar), static member variables should be prefixed `ms` (msStaticMemberVar). Do not use any other prefixing such as Hungarian notation.</li>
    <li>Preprocessor macros must be all upper case and prefixed with OGRE_</li>
    <li>Enums should be named in title case, enum values should be all upper case</li>
    <li>All classes and methods must be fully documented in English using Doxygen-compatible comments. Use the `@param` and `@return` directives to define inputs and outputs clearly, and `@note` to indicate points of interest.</li>
    <li>Use verbose, descriptive names for classes, methods, variables - everything except trival counters. Code should be self-describing, don't be obtuse.</li>
</ol>

## Style issues

<ol>
    <li>Insert a newline before an open brace (contentious I know!)</li>
    <li>Use typedefs to declare template-based types that you use to avoid ugliness e.g. typedef std::list<MyType*> MyTypeList;</li>
    <li>Always insert spaces in between operators and operands (x + y, not x+y)</li>
    <li>Use parenthesis to make the operator precedence unambiguous, even when it is not required ((x * y) + 1, not x * y + 1)</li>
</ol>


## Error handling

<ol>
    <li>Fatal errors should always be dealt with though exception handling. No return-value error reporting.</li>
    <li>Whenever you make an assumption in your code, verify it with an assert.

* By default use `OgreAssert`. This will generate an exception - even in release builds. 
* For performance critical code resort to `OgreAssertDbg` that only fires in debug builds.
* In public header files use the C `assert` macro to follow user-defined compilation flags.
* Prefer `OGRE_EXCEPT` for runtime-conditions outside of developer control e.g. minimal required OpenGL Version. This allows building Ogre with no assertion for release.
    </li>
</ol>


## Design issues

<ol>
    <li>Use existing design patterns and identify them by their well known names. A good starting reference is the "Gang of Four" book.</li>
    <li>Use strong encapsulation. Top-level interfaces should hide implementations and not require the user of the library to understand internals. Avoid public attributes except in structs.</li>
    <li>Don't use `friend` if you can avoid it. Where classes need to collaborate on an internal implementation, prefix the methods they use to communicate with `_` (this is our demarcation for "recommended for internal use only"). This can also be used to expose advanced functionality only intended for very skilled users.</li>

</ol>


## Final words

If in doubt, do as the existing code does!
