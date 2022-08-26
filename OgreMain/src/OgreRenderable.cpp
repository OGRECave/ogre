// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#include "OgreStableHeaders.h"

namespace Ogre
{
void Renderable::setCustomParameter(size_t index, const Vector4f& value) { mCustomParameters[index] = value; }

void Renderable::removeCustomParameter(size_t index) { mCustomParameters.erase(index); }

bool Renderable::hasCustomParameter(size_t index) const
{
    return mCustomParameters.find(index) != mCustomParameters.end();
}

const Vector4f& Renderable::getCustomParameter(size_t index) const
{
    CustomParameterMap::const_iterator i = mCustomParameters.find(index);
    if (i != mCustomParameters.end())
    {
        return i->second;
    }
    else
    {
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Parameter at the given index was not found.");
    }
}

void Renderable::_updateCustomGpuParameter(const GpuProgramParameters::AutoConstantEntry& constantEntry,
                                           GpuProgramParameters* params) const
{
    CustomParameterMap::const_iterator i = mCustomParameters.find(constantEntry.data);
    if (i != mCustomParameters.end())
    {
        params->_writeRawConstant(constantEntry.physicalIndex, i->second, constantEntry.elementCount);
    }
}
} // namespace Ogre
