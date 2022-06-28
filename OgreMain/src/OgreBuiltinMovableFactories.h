// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifndef __BUILTIN_MOVABLE_FACTORIES_H_
#define __BUILTIN_MOVABLE_FACTORIES_H_

#include <OgreMovableObject.h>
#include <OgreRectangle2D.h>

namespace Ogre
{
class Rectangle2DFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params) override;

public:
    static const String FACTORY_TYPE_NAME;
    const String& getType(void) const override { return FACTORY_TYPE_NAME; }
};
} // namespace Ogre

#endif