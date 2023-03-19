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
class BillboardChainFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class BillboardSetFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class EntityFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class LightFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class ManualObjectFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class ParticleSystemFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class Rectangle2DFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl(const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override { return MOT_RECTANGLE2D; }
};

class RibbonTrailFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;

public:
    const String& getType(void) const override;
};

class StaticGeometryFactory : public MovableObjectFactory
{
    MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override { return NULL; }

public:
    const String& getType(void) const override { return MOT_STATIC_GEOMETRY; }
};
} // namespace Ogre

#endif