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
#include "OgreOverlayTranslator.h"
#include "OgreFontManager.h"
#include "OgreScriptTranslator.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"

#include <sstream>

namespace Ogre
{
//! [font_translate]
void FontTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr& node)
{
    ObjectAbstractNode* obj = static_cast<ObjectAbstractNode*>(node.get());

    // Must have a name - unless we are in legacy mode. Then the class is the name.
    if (obj->name.empty() && obj->cls == "font")
    {
        compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
                           "font must be given a name");
        return;
    }

    String& name = obj->cls == "font" ? obj->name : obj->cls;

    FontPtr font = FontManager::getSingleton().create(name, compiler->getResourceGroup());
    font->_notifyOrigin(obj->file);

    for (auto& c : obj->children)
    {
        if (c->type == ANT_PROPERTY)
        {
            parseAttribute(compiler, font, static_cast<PropertyAbstractNode*>(c.get()));
        }
    }
}
//! [font_translate]

void FontTranslator::parseAttribute(ScriptCompiler* compiler, FontPtr& pFont,
                                    PropertyAbstractNode* prop)
{
    String& attrib = prop->name;
    String val;

    if (attrib == "glyph")
    {
        std::vector<float> coords;
        if (prop->values.size() != 5 || !getString(prop->values.front(), &val) ||
            !getVector(++prop->values.begin(), prop->values.end(), coords, 4))
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return;
        }

        // Set
        // Support numeric and character glyph specification
        Font::CodePoint cp;
        if (val.size() > 1 && val[0] == 'u')
        {
            // Unicode glyph spec
            String trimmed = val.substr(1);
            cp = StringConverter::parseUnsignedInt(trimmed);
        }
        else
        {
            // Direct character
            cp = val[0];
        }
        pFont->setGlyphInfoFromTexCoords(
            cp, FloatRect(coords[0], coords[1], coords[2], coords[3])); // assume image is square
    }
    else if (attrib == "antialias_colour")
    {
        bool flag;
        if (prop->values.empty() || !getBoolean(prop->values.front(), &flag))
        {
            compiler->addError(ScriptCompiler::CE_STRINGEXPECTED, prop->file, prop->line);
            return;
        }
        pFont->setAntialiasColour(flag);
        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line, attrib);
    }
    else if (attrib == "code_points")
    {
        if (prop->values.empty())
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
            return;
        }

        for (auto& v : prop->values)
        {

            bool succ = getString(v, &val);
            StringVector itemVec = StringUtil::split(val, "-");
            if (succ && itemVec.size() == 2)
            {
                pFont->addCodePointRange(
                    Font::CodePointRange(StringConverter::parseUnsignedInt(itemVec[0]),
                                         StringConverter::parseUnsignedInt(itemVec[1])));
            }
        }
    }
    else if(attrib == "character_spacer")
        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line, attrib);
    else if (prop->values.empty() || !getString(prop->values.front(), &val) ||
             !pFont->setParameter(attrib, val))
    {
        compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
    }
}

void ElementTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr& node)
{
    ObjectAbstractNode* obj = static_cast<ObjectAbstractNode*>(node.get());

    String name;
    // legacy compat
    if ((obj->cls == "template") && !obj->values.empty())
    {
        getString(obj->values.front(), &name);
        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, obj->file, obj->line, "template. use overlay_element");
    }
    else
        name = obj->name;

    String type;
    if(obj->values.empty() || ((obj->cls == "template") && obj->values.size() == 1))
    {
        // legacy naming support
        std::vector<String> params = StringUtil::split(name, "()", 2);

        if (params.size() != 2)
        {
            compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
                               "expecting 'element type(name)' or 'overlay_element name type'");
            return;
        }

        compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, obj->file, obj->line, "'element type(name)'. use 'overlay_element name type'");

        type = params[0];
        name = params[1];
    }
    else
        getString(obj->values.front(), &type);

    String templateName;
    if(!obj->bases.empty())
        templateName = obj->bases.front();

    OverlayElement* newElement =
        OverlayManager::getSingleton().createOverlayElementFromTemplate(templateName, type, name);

    if(obj->parent && obj->parent->context.has_value())
    {
        Overlay** overlay = any_cast<Overlay*>(&obj->parent->context);
        if(overlay)
            if(newElement->isContainer())
                (*overlay)->add2D((OverlayContainer*)newElement);
            else
                compiler->addError(ScriptCompiler::CE_OBJECTALLOCATIONERROR, obj->file, obj->line,
                                   "Top level components must be containers, but '" + type + "' is an element");
        else
            any_cast<OverlayContainer*>(obj->parent->context)->addChild(newElement);
    }

    if(newElement->isContainer())
        obj->context = (OverlayContainer*)newElement;

    String val;
    for (auto& c : obj->children)
    {
        if (c->type == ANT_PROPERTY)
        {
            PropertyAbstractNode* prop = static_cast<PropertyAbstractNode*>(c.get());

            bool succ = true;
            if(prop->values.size() > 1)
            {
                // FIXME: joining string, just so setParameter can split it again..
                StringStream ss;
                for(auto& v : prop->values) {
                    succ = succ && getString(v, &val);
                    ss << val << " ";
                }
                val = ss.str();
            }
            else
            {
                succ = getString(prop->values.front(), &val);
            }

            if(prop->name == "space_width")
                compiler->addError(ScriptCompiler::CE_DEPRECATEDSYMBOL, prop->file, prop->line, prop->name);

            if(!succ || !newElement->setParameter(prop->name, val))
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        }
        else if(c->type == ANT_OBJECT)
            translate(compiler, c); // recurse
    }
}

void OverlayTranslator::translate(ScriptCompiler* compiler, const AbstractNodePtr& node)
{
    ObjectAbstractNode* obj = static_cast<ObjectAbstractNode*>(node.get());

    // Must have a name - unless we are in legacy mode. Then the class is the name.
    if (obj->name.empty() && obj->cls == "overlay")
    {
        compiler->addError(ScriptCompiler::CE_OBJECTNAMEEXPECTED, obj->file, obj->line,
                           "overlay must be given a name");
        return;
    }

    String& name = obj->cls == "overlay" ? obj->name : obj->cls;
    Overlay* overlay = OverlayManager::getSingleton().create(name);
    overlay->_notifyOrigin(obj->file);

    obj->context = overlay;

    for (auto& c : obj->children)
    {
        if (c->type == ANT_PROPERTY)
        {
            PropertyAbstractNode* prop = static_cast<PropertyAbstractNode*>(c.get());

            uint32 zorder;
            if (prop->name != "zorder" || prop->values.empty() || !getUInt(prop->values.front(), &zorder))
            {
                compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, obj->file, obj->line, prop->name);
                continue;
            }
            overlay->setZOrder(Math::uint16Cast(zorder));
        }
        else if(c->type == ANT_OBJECT)
            processNode(compiler, c);
    }
}

OverlayTranslatorManager::OverlayTranslatorManager()
{
//! [font_register]
    ScriptCompilerManager::getSingleton().addTranslatorManager(this);
    ID_FONT = ScriptCompilerManager::getSingleton().registerCustomWordId("font");
//! [font_register]
    ID_OVERLAY_ELEMENT = ScriptCompilerManager::getSingleton().registerCustomWordId("overlay_element");
    ID_OVERLAY = ScriptCompilerManager::getSingleton().registerCustomWordId("overlay");
    ID_CONTAINER = ScriptCompilerManager::getSingleton().registerCustomWordId("container");
    ID_ELEMENT = ScriptCompilerManager::getSingleton().registerCustomWordId("element");
    ID_TEMPLATE = ScriptCompilerManager::getSingleton().registerCustomWordId("template");
}

OverlayTranslatorManager::~OverlayTranslatorManager()
{
    ScriptCompilerManager::getSingleton().removeTranslatorManager(this);
}

//! [font_get_translator]
ScriptTranslator* OverlayTranslatorManager::getTranslator(const AbstractNodePtr& node)
{
    if (node->type != ANT_OBJECT)
        return NULL;

    ObjectAbstractNode* obj = static_cast<ObjectAbstractNode*>(node.get());

    if (obj->id == ID_FONT)
        return &mFontTranslator;
//! [font_get_translator]

    // legacy compatibility: assume this is a font if we are in a .fontdef file
    if (obj->id == 0 && StringUtil::endsWith(node->file, ".fontdef"))
        return &mFontTranslator;

    if (obj->id == ID_CONTAINER || obj->id == ID_TEMPLATE || obj->id == ID_ELEMENT || obj->id == ID_OVERLAY_ELEMENT)
        return &mElementTranslator;

    // legacy compatibility: assume this is an overlay if we are in a .overlay file
    if(obj->id == ID_OVERLAY || (obj->id == 0 && StringUtil::endsWith(node->file, ".overlay")))
        return &mOverlayTranslator;

    return NULL;
}
}
