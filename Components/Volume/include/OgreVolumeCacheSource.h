#ifndef __Ogre_Volume_CacheSource_H__
#define __Ogre_Volume_CacheSource_H__

#include "OgreVector3.h"
#include "OgreVector4.h"

#include "OgreVolumeSource.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {
    
    /** == operator for two vectors.
    @param a
        The first vector to test.
    @param b
        The second vector to test.
    */
    bool _OgreVolumeExport operator==(Vector3 const& a, Vector3 const& b);
    
    /** A less operator. 
    @note
        This operator is needed so that Vertex can serve as the key in a map structrue 
    @param a
        The first vector to test.
    @param b
        The second vector to test.
    */
    bool _OgreVolumeExport operator<(const Vector3& a, const Vector3& b);

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
        
        /** Gets a density value and gradient from the cache.
        @param position
            The position of the density value and gradient.
        @return
            The density value (w-component) and the gradient (x, y and z component).
        */
        inline Vector4 getFromCache(const Vector3 &position) const
        {
            Vector4 result;
            map<Vector3, Vector4>::iterator it = mCache.find(position);
            if (it == mCache.end())
            {
                result = mSrc->getValueAndGradient(position);
                mCache[position] = result;
            }
            else
            {
                result = it->second;
            }
            return result;
        }

    public:
        
        /** Constructor.
        @param src
            The source to cache.
        */
        CacheSource(const Source *src);
        
        /** Overridden from Source.
        */
        virtual Vector4 getValueAndGradient(const Vector3 &position) const;
        
        /** Overridden from Source.
        */
        virtual Real getValue(const Vector3 &position) const;

    };

}
}

#endif
