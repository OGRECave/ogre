/** This is a file to try compiling on different C++ standards
    specifically, for constexpr opperation of Ogre::Vector
*/
#include "Ogre.h"

using namespace Ogre;

// all supported ogre versions can initialize vectors constexpr
constexpr Vector2 v2(0,0);
constexpr Vector3 v3(0,0,0);
constexpr Vector4 v4(0,0,0,0);

#if __cpp_constexpr >= 202002L
// given 2020 level support, we get constexpr indexing
constexpr Vector<3, int> int3v(0,0,0);
constexpr int int3vf = int3v[2];
constexpr float v3f = v3[2];

// and thus equality
static_assert(Vector3(0,0,0) == v3);
static_assert(Vector3(1,0,0) != v3);
// and inequality
static_assert(Vector3(0,0,0) < Vector3(1,2,3));
static_assert(Vector3(3,4,5) > Vector3(1,2,3));

#endif
#if __cpp_constexpr >= 202211L
/* constexpr Vector3 a = Vector3(1,1,1); */
/* constexpr Vector3 b = a + Vector(1,2,3);  */
/*  static_assert(b == Vector3(2,3,4));  */
/* constexpr Vector3 b2 = a * Vector(1,2,3); */
/* static_assert(b2 == Vector3(1,2,3)); */
/* constexpr Vector3 b3 = a - Vector(1,2,3); */
/* static_assert(b3 == Vector3(0,-1,-2)); */
/* constexpr Vector3 b4 = Vector(1,2,3) / a; */
/* static_assert(b4 == Vector3(1,2,3)); */

#endif
