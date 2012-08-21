#ifndef __Ogre_Volume_CacheSource_H__
#define __Ogre_Volume_CacheSource_H__

#include "OgreVector3.h"
#include "OgreVector4.h"

#include "OgreVolumeSource.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {
    
    /** A caching Source.
    */
    class _OgreVolumeExport CacheSource : public Source
    {
    protected:
        
        /// Map for the cache
        typedef map<Vector3, Vector4>::type UMapPositionValue;
        mutable UMapPositionValue mCache;

        /// The source to cache.
        const Source *mSrc;
    public:
        
        /** Constructor.
        @param src
            The source to cache.
        */
        CacheSource(const Source *src);
        
        /** Overridden from Source.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const;
        
        /** Overridden from Source.
        */
        virtual Real getValue(const Vector3 &position) const;

    };

}
}

#endif
