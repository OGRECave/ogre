/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _OverlayScriptTranslator_
#define _OverlayScriptTranslator_

#include "OgreOverlay.h"
#include "OgreScriptTranslator.h"
#include "OgreFont.h"

namespace Ogre
{
//! [font_translator]
struct FontTranslator : public ScriptTranslator
{
    void translate(ScriptCompiler* compiler, const AbstractNodePtr& node);
    void parseAttribute(ScriptCompiler* compiler, FontPtr& pFont, PropertyAbstractNode* prop);
};
//! [font_translator]

struct ElementTranslator : public ScriptTranslator
{
    void translate(ScriptCompiler* compiler, const AbstractNodePtr& node);
};

struct OverlayTranslator : public ScriptTranslator
{
    void translate(ScriptCompiler* compiler, const AbstractNodePtr& node);
};

class OverlayTranslatorManager : public ScriptTranslatorManager
{
    FontTranslator mFontTranslator;
    ElementTranslator mElementTranslator;
    OverlayTranslator mOverlayTranslator;
    uint32 ID_FONT;
    uint32 ID_OVERLAY_ELEMENT;
    uint32 ID_OVERLAY;
    uint32 ID_CONTAINER;
    uint32 ID_ELEMENT;
    uint32 ID_TEMPLATE;
public:
    OverlayTranslatorManager();
    ~OverlayTranslatorManager();
    ScriptTranslator* getTranslator(const AbstractNodePtr& node);
};
}

#endif
