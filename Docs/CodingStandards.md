# OGRE Coding Standards

<h3>Introduction</h3>

This document describes the coding standards all developers are expected to adhere to when writing code for the OGRE project.

<h3>Top-level organisation issues</h3>

<ol>
    <li>All source files must begin with the standard OGRE copyright statement:    <pre>/*-------------------------------------------------------------------------
This source file is a part of OGRE
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
THE SOFTWARE
-------------------------------------------------------------------------*/</pre>
    </li>
    <li>All publicly visible classes should be declared in their own header file using the .h extension, placed in the `include` folder of the sub-project in question, and named after the class but prefixed with `Ogre` e.g. `OgreMyClass.h`. Only very tightly related classes should be declared in the same header file. </li>
    <li>Implementations should be placed in a source file called the same name as the class but with an extension of .cpp.</li>

    <li>Everything must be declared inside the namespace `Ogre`.</li>
</ol>

<h3>Portablity</h3>

<ol>
    <li>All code must be cross platform, ANSI C++. No dependencies on platform-specific headers and / or types are allowed (the only exception is when dealing with platform-specific features like windowing, which must be implemented for each platform separately).</li>
    <li>If you serialise / deserialise any data, subclass from Serializer and use its methods, it will insulate you from endianness issues. If you need to serialise any types it doesn't already handle, make sure you deal with endianness issues in the same way Serializer does (ie use native endianness by default but detect the inverse on reading and perform flipping if required).</li>
</ol>

<h3>C++ Standards compliance</h3>

<ol>

    <li>Always prefer the STL over custom containers / algorithms.</li>

    <li>Always prefer C++ techniques over C.
        <ul><li>Avoid C-strings (`char*` and functions like sprintf, strcpy, use `Ogre::String`)</li>
            <li>Avoid old I/O routines (fopen et al, use `<iostream>`)</li>

            <li>Use abstract classes or templates not `void*`</li>

            <li>Use overloaded methods not varargs.</li>
        </ul></li>
    <li>The target C++ level C++03. Do not use C++11 features.</li>
    <li>Use the <a href="http://www.boost.org/libs/serialization/doc/pimpl.html">PImpl idiom</a> to reduce dependencies between classes.</li>

    <li>Always use <a href="http://www.cprogramming.com/tutorial/const_correctness.html">const-correctness</a>. Methods taking non-primitive types as parameters should generally take them as const references, methods returning non-primitive types should generally return them as const references. Declare all methods that do not modify internal state `const`. For lazy-update getter methods, declare the internal state which is lazy-updated `mutable`.</li>

    <li>Prefer `protected` over `private` to encourage specialisation where appropriate</li>

    <li>Always declare destructors `virtual` unless the class you are writing should not have any vtable (no other virtual methods).</li>

    <li>Avoid non-const by-ref parameters unless you have no other option. We prefer not to have in/our parameters since they are less intuitive.</li>
</ol>


<h3>Naming conventions &amp; Documentation</h3>

<ol>

    <li>Classes, types and structures must be title case (MyNewClass). </li>

    <li>Methods and local variables must be camel case (myNewMethod). </li>
    <li>Member variables should be prefixed with `m` (mInstanceVar), static member variables should be prefixed `ms` (msStaticMemberVar). Do not use any other prefixing such as Hungarian notation.</li>
    <li>Preprocessor macros must be all upper case and prefixed with OGRE_</li>

    <li>Enums should be named in title case, enum values should be all upper case</li>
    <li>All classes and methods must be fully documented in English using Doxygen-compatible comments. Use the `@param` and `@returns` directives to define inputs and outputs clearly, and `@note` to indicate points of interest.</li>
    <li>Use verbose, descriptive names for classes, methods, variables - everything except trival counters. Code should be self-describing, don't be obtuse.</li>
</ol>


<h3>Memory Management</h3>
<ol>
    <li>Do not derive from `AllocatedObject`</li>
    <li>use `new` / `delete` directly instead of `OGRE_NEW`/ `OGRE_DELETE` (However first consider using a SharedPtr instead)</li>
    <li>When defining STL containers, just use `std::vector<T>` or `std::list<T>` etc, instead of using the memory-manager versions `vector<T>::type` and `list<T>::type` respectively.</li>
</ol>
You will find usages of these throughout the codebase - these come from a time where using nedmalloc was up to 125x faster than using the default Windows XP allocator. However today nedmalloc is unmaintained, Windows XP is history and the approach of using Macros/ Inheritance to make the allocator configurable has proven to be overly complicated.
<h3>Style issues</h3>

<ol>

    <li>Insert a newline before an open brace (contentious I know!)</li>
    <li>Use typedefs to declare template-based types that you use to avoid ugliness e.g. typedef std::list<MyType*> MyTypeList;</li>
    <li>Always insert spaces in between operators and operands (x + y, not x+y)</li>
    <li>Use parenthesis to make the operator precedence unambiguous, even when it is not required ((x * y) + 1, not x * y + 1)</li>
</ol>


<h3>Error handling</h3>

<ol>

    <li>Fatal errors should always be dealt with though exception handling. No return-value error reporting.</li>

    <li>Whenever you make an assumption in your code, verify it with an assert().</li>

</ol>


<h3>Design issues</h3>

<ol>

    <li>Use existing design patterns and identify them by their well known names. A good starting reference is the "Gang of Four" book.</li>
    <li>Use strong encapsulation. Top-level interfaces should hide implementations and not require the user of the library to understand internals. Avoid public attributes except in structs.</li>
    <li>Don't use `friend` if you can avoid it. Where classes need to collaborate on an internal implementation, prefix the methods they use to communicate with `_` (this is our demarcation for "recommended for internal use only"). This can also be used to expose advanced functionality only intended for very skilled users.</li>

</ol>


<h3>Final words</h3>

If in doubt, do as the existing code does!
