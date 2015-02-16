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

#include "TextureBlitTest.h"

/* GIMP RGB C-Source image dump (image.c) */

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char	 pixel_data[16 * 16 * 3 + 1];
} ColorImage = {
  16, 16, 3,
  "\377\0\0\377\0T\377\0\265\351\0\377\210\0\377'\0\377\0:\377\0\233\377\0\373"
  "\377\0\377\243\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377\\\0\377"
  "\0\0\377\0T\377\0\265\350\0\377\210\0\377(\0\377\0""9\377\0\232\377\0\373"
  "\377\0\377\242\0\377A\37\377\0\177\377\0\340\377\0\377\275\0\377]\0\377\0"
  "\0\377\0T\377\0\265\350\0\377\207\0\377'\0\377\0""9\377\0\232\377\0\373\377"
  "\0\377\243\0\377B\37\377\0\177\377\0\341\377\0\377\275\0\377\\\0\377\0\0"
  "\377\0T\377\0\264\351\0\377\210\0\377'\0\377\0:\377\0\232\377\0\373\377\0"
  "\377\242\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377]\0\377\0\0\377"
  "\0T\377\0\265\351\0\377\210\0\377(\0\377\0""9\377\0\233\377\0\373\377\0\377"
  "\242\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0"
  "T\377\0\264\351\0\377\210\0\377'\0\377\0""9\377\0\232\377\0\373\377\0\377"
  "\243\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377]\0\377\0\0\377\0U"
  "\377\0\265\351\0\377\210\0\377'\0\377\0:\377\0\232\377\0\373\377\0\377\243"
  "\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0U\377"
  "\0\264\351\0\377\210\0\377'\0\377\0""9\377\0\233\377\0\373\377\0\377\242"
  "\0\377B\37\377\0\200\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0T\377"
  "\0\265\350\0\377\210\0\377'\0\377\0""9\377\0\232\377\0\373\377\0\377\242"
  "\0\377B\37\377\0\177\377\0\340\377\0\377\275\0\377]\0\377\0\0\377\0T\377"
  "\0\264\351\0\377\210\0\377'\0\377\0""9\377\0\232\377\0\373\377\0\377\242"
  "\0\377B\37\377\0\200\377\0\341\377\0\377\275\0\377]\0\377\0\0\377\0T\377"
  "\0\265\351\0\377\210\0\377'\0\377\0:\377\0\232\377\0\373\377\0\377\243\0"
  "\377B\37\377\0\177\377\0\341\377\0\377\275\0\377\\\0\377\0\0\377\0T\377\0"
  "\265\350\0\377\210\0\377'\0\377\0""9\377\0\232\377\0\372\377\0\377\243\0"
  "\377B\37\377\0\200\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0T\377\0"
  "\265\350\0\377\207\0\377'\0\377\0""9\377\0\232\377\0\372\377\0\377\243\0"
  "\377B\37\377\0\200\377\0\340\377\0\377\275\0\377]\0\377\0\0\377\0T\377\0"
  "\265\350\0\377\207\0\377'\0\377\0:\377\0\232\377\0\373\377\0\377\243\0\377"
  "B\37\377\0\177\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0U\377\0\264"
  "\350\0\377\210\0\377'\0\377\0:\377\0\232\377\0\372\377\0\377\243\0\377A\37"
  "\377\0\177\377\0\340\377\0\377\275\0\377\\\0\377\0\0\377\0T\377\0\265\351"
  "\0\377\210\0\377'\0\377\0""9\377\0\232\377\0\373\377\0\377\243\0\377B\37"
  "\377\0\200\377\0\340\377\0\377\275\0\377\\\0",
};

/* GIMP RGB C-Source image dump (depth.c) */

static const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char	 pixel_data[16 * 16 * 2 + 1];
} DepthImage = {
  16, 16, 2,
  "\0\0a\10\343\30f)\3479\211R\353b\216k\357{r\224\24\245\226\255\30\276\232"
  "\326\374\346\236\357\0\0b\10\344\30e)\3479jJ\353Z\216s\20|r\214\364\244\226"
  "\265\27\276\232\326\34\347\236\367\0\0b\20\343\30e1\3509jJ\13[\216k\360{"
  "q\214\24\235u\255\30\306z\326\34\347\236\367\0\0a\10\3\31e1\3479\211J\353"
  "Zns\357{r\224\363\234\226\265\27\306\232\326\34\347\236\367\0\0a\10\3\31"
  "e)\3479jJ\14[mk\20|r\224\364\244u\265\30\306\231\316\34\347\236\367\0\0a"
  "\10\343\30e)\3479iJ\353Z\215s\357{q\224\23\235\225\265\30\306\231\326\34"
  "\347\236\367\0\0a\10\343\30e1\3479jR\14[mk\17\204q\224\364\244u\265\30\306"
  "\232\326\33\347\236\367\0\0\201\10\343\30e)\7B\212J\354bms\360{\222\224\24"
  "\235\225\265\30\276\232\316\34\347\236\367\0\0a\10\343\30f)\3479iJ\353Zm"
  "s\357{\222\214\24\235\226\265\27\306\231\326\34\347\236\367\0\0a\10\343\30"
  "e)\347AiR\14[nk\17|q\224\24\245\225\255\30\306\232\326\34\347\236\367\0\0"
  "a\10\343\30e)\7:\212J\353bns\17\204q\224\24\235\225\255\30\306\232\326\34"
  "\347\236\367\0\0a\10\343\40e)\3509\212J\13[\215s\20\204\222\214\24\245\225"
  "\255\30\276\232\326\34\337\236\357\0\0a\10\343\30e)\3509iJ\353b\215s\360"
  "\203\222\214\364\234u\265\27\306\231\326\34\347\236\367\0\0a\10\343\30e1"
  "\7:iR\13cnk\357\203\221\214\23\245v\265\30\306\232\326\374\346\236\367\0"
  "\0a\10\343\30\206)\3479iJ\353Zmk\20|\222\214\24\235\226\255\30\276\232\326"
  "\33\347\236\367\0\0a\10\343\30e1\350AjR\13[\216k\17|q\224\23\235v\265\370"
  "\305\232\326\34\347\236\357",
};


namespace
{
    void ReplaceTexture(Ogre::MaterialPtr mat, const Ogre::String & texture)
    {
        // Now look for the texture
        bool found = false;
        unsigned short numTechniques = mat->getNumTechniques();
        for (unsigned short i = 0; i < numTechniques; ++i)
        {
            Ogre::Technique* tech = mat->getTechnique(i);
            unsigned short numPasses = tech->getNumPasses();
            for (unsigned short j = 0; j < numPasses; ++j)
            {
                Ogre::Pass* pass = tech->getPass(j);
                unsigned short numTUs = pass->getNumTextureUnitStates();
                for (unsigned short k = 0; k < numTUs; ++k)
                {
                    found = true;

                    Ogre::TextureUnitState* tu = pass->getTextureUnitState(k);

                    tu->setTextureName(texture);
                }

                if (found) break;
            }

            if (found) break;
        }

        assert(found);
    }
}


TextureBlitTest::TextureBlitTest()
{
    mInfo["Title"] = "VTests_TextureBlit";
    mInfo["Description"] = "Tests texture blitting.";
    addScreenshotFrame(50);
}
//---------------------------------------------------------------------------

void TextureBlitTest::setupContent()
{
    static const Ogre::String ColorTextureName = "VTests_TextureBlit_color_texture";
    static const Ogre::String DepthTextureName = "VTests_TextureBlit_depth_texture";
    static const int TextureSize = 8;

    mViewport->setBackgroundColour(ColourValue(0.8,0.8,0.8));

    /// TEST COLOUR
    m_colorTexture = Ogre::TextureManager::getSingleton().createManual(
        ColorTextureName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        TextureSize, TextureSize,
        0,
        Ogre::PF_BYTE_RGB,
        Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    Ogre::PixelBox pb(ColorImage.width, ColorImage.height, 1, Ogre::PF_BYTE_RGB, (void *)ColorImage.pixel_data);
    Ogre::HardwarePixelBufferSharedPtr buffer = m_colorTexture->getBuffer();
    buffer->blitFromMemory(pb);

    // Get the material
    Ogre::MaterialPtr matColourPtr = Ogre::MaterialManager::getSingleton().getByName("Examples/OgreDance");

    ReplaceTexture(matColourPtr, ColorTextureName);

    // create a standard plane entity
    Entity* ent = mSceneMgr->createEntity("Plane_color", SceneManager::PT_PLANE);

    // attach it to a node, scale it, and position appropriately
    SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    node->setPosition(-30.0, 0.0, 0.0);
    node->setScale(0.25, 0.25, 0.25);
    node->attachObject(ent);

    ent->setMaterial(matColourPtr);  // give it the material we prepared

    /// TEST DEPTH
    m_depthTexture = Ogre::TextureManager::getSingleton().createManual(
        DepthTextureName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        TextureSize, TextureSize,
        0,
        Ogre::PF_DEPTH,
        Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    Ogre::PixelBox dpb(DepthImage.width, DepthImage.height, 1, Ogre::PF_DEPTH, (void *)DepthImage.pixel_data);
    Ogre::HardwarePixelBufferSharedPtr dbuffer = m_depthTexture->getBuffer();
    dbuffer->blitFromMemory(dpb);

    // Get the material
    Ogre::MaterialPtr matDepthPtr = Ogre::MaterialManager::getSingleton().getByName("Examples/OgreParade");

    ReplaceTexture(matDepthPtr, DepthTextureName);

    // create a standard plane entity
    Entity* depthEnt = mSceneMgr->createEntity("Plane_depth", SceneManager::PT_PLANE);

    // attach it to a node, scale it, and position appropriately
    SceneNode* depthNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    depthNode->setPosition(30.0, 0.0, 0.0);
    depthNode->setScale(0.25, 0.25, 0.25);
    depthNode->attachObject(depthEnt);

    depthEnt->setMaterial(matDepthPtr);  // give it the material we prepared

    mCamera->setPosition(0,0,125);
    mCamera->setDirection(0,0,-1);
}
//-----------------------------------------------------------------------


