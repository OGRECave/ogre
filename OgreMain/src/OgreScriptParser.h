/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef __SCRIPTPARSER_H_
#define __SCRIPTPARSER_H_

#include "OgreScriptLexer.h"
#include "OgreScriptCompiler.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /** These enums hold the types of the concrete parsed nodes */
    enum ConcreteNodeType
    {
        CNT_VARIABLE,
        CNT_VARIABLE_ASSIGN,
        CNT_WORD,
        CNT_IMPORT,
        CNT_QUOTE,
        CNT_LBRACE,
        CNT_RBRACE,
        CNT_COLON
    };

    /** The ConcreteNode is the struct that holds an un-conditioned sub-tree of parsed input */
    struct ConcreteNode : public ScriptCompilerAlloc
    {
        String token, file;
        unsigned int line;
        ConcreteNodeType type;
        ConcreteNodeList children;
        ConcreteNode *parent;
    };

    class _OgrePrivate ScriptParser : public ScriptCompilerAlloc
    {
    public:
        static ConcreteNodeListPtr parse(const ScriptTokenList &tokens, const String& file);
        static ConcreteNodeListPtr parseChunk(const ScriptTokenList &tokens, const String& file);
    private:
        static const ScriptToken *getToken(ScriptTokenList::const_iterator i, ScriptTokenList::const_iterator end, int offset);
        static ScriptTokenList::const_iterator skipNewlines(ScriptTokenList::const_iterator i, ScriptTokenList::const_iterator end);
    };
    
    /** @} */
    /** @} */
}

#endif
