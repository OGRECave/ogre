// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.

#ifndef OGREMAIN_INCLUDE_OGREDEPRECATED_H_
#define OGREMAIN_INCLUDE_OGREDEPRECATED_H_

#include "OgreStdHeaders.h"
#include "OgreMemoryAllocatorConfig.h"

#include <deque>

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \defgroup deprecated deprecated
    *   deprecated std:: wrappers for backwards-compatibility only
    *  @{
    */
    template<class T> using AtomicScalar = std::atomic<T>;

    struct SPFMNone {
        void operator()(void*) {}
    };
    const SPFMNone SPFM_NONE;

    typedef std::map<int, MaterialPtr> QuadMaterialMap;

#define OGRE_HashMap ::std::unordered_map
#define OGRE_HashMultiMap ::std::unordered_multimap
#define OGRE_HashSet ::std::unordered_set
#define OGRE_HashMultiSet ::std::unordered_multiset

    template <typename T>
    struct deque
    {
        typedef typename std::deque<T> type;
        typedef typename std::deque<T>::iterator iterator;
        typedef typename std::deque<T>::const_iterator const_iterator;
    };

    template <typename T>
    struct vector
    {
        typedef typename std::vector<T> type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
    };

    template <typename T>
    struct list
    {
        typedef typename std::list<T> type;
        typedef typename std::list<T>::iterator iterator;
        typedef typename std::list<T>::const_iterator const_iterator;
    };

    template <typename T, typename P = std::less<T> >
    struct set
    {
        typedef typename std::set<T, P> type;
        typedef typename std::set<T, P>::iterator iterator;
        typedef typename std::set<T, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct map
    {
        typedef typename std::map<K, V, P> type;
        typedef typename std::map<K, V, P>::iterator iterator;
        typedef typename std::map<K, V, P>::const_iterator const_iterator;
    };

    template <typename K, typename V, typename P = std::less<K> >
    struct multimap
    {
        typedef typename std::multimap<K, V, P> type;
        typedef typename std::multimap<K, V, P>::iterator iterator;
        typedef typename std::multimap<K, V, P>::const_iterator const_iterator;
    };

    /// Bitmask containing scene types
    typedef uint16 SceneTypeMask;
    enum SceneType
    {
        ST_GENERIC = 1,
        ST_EXTERIOR_CLOSE = 2,
        ST_EXTERIOR_FAR = 4,
        ST_EXTERIOR_REAL_FAR = 8,
        ST_INTERIOR = 16
    };
    /** @} */
    /** @} */
} // Ogre

#endif /* OGREMAIN_INCLUDE_OGREDEPRECATED_H_ */
