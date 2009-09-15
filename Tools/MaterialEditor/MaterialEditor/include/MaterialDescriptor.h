/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef _MATERIALDESCRIPTOR_H_
#define _MATERIALDESCRIPTOR_H_

#include "OgreString.h"

class EventArgs;
class MaterialController;
class MaterialEventArgs;
class PassEventArgs;
class TechniqueEventArgs;

using Ogre::MaterialPtr;
using Ogre::String;

class MaterialDescriptor
{
public:
	MaterialDescriptor();
	MaterialDescriptor(const String& name);
	virtual ~MaterialDescriptor();
	
	const String& getName() const;
	const String& getScript() const;
	MaterialController* getMaterialController();
	MaterialPtr& getMaterial();
	void setMaterial(MaterialPtr& mp);
		
	void OnRootInitialized(EventArgs& args);
	void OnRootShutdown(EventArgs& args);
	
protected:
	String mName;
	String mScript;
	MaterialController* mMaterialController;
};

#endif _MATERIALDESCRIPTOR_H_