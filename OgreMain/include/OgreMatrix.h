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
#ifndef __MatrixT__
#define __MatrixT__

// Precompiler options
#include "OgreMath.h"
#include "OgrePrerequisites.h"

namespace Ogre
{
/** \addtogroup Core
*  @{
*/
/** \addtogroup Math
*  @{
*/

/// helper class to allow legacy x, y, z access
template <int dims> struct VectorData
{
    VectorData() {}
    Real data[dims];
    Real* ptr() { return data; }
    const Real* ptr() const { return data; }
};

template <> struct VectorData<2>
{
    VectorData() {}
    VectorData(Real _x, Real _y) : x(_x), y(_y) {}
    Real x, y;
    Real* ptr() { return &x; }
    const Real* ptr() const { return &x; }
};

template <> struct VectorData<3>
{
    VectorData() {}
    VectorData(Real _x, Real _y, Real _z) : x(_x), y(_y), z(_z) {}
    Real x, y, z;
    Real* ptr() { return &x; }
    const Real* ptr() const { return &x; }
};

template <> struct VectorData<4>
{
    VectorData() {}
    VectorData(Real _x, Real _y, Real _z, Real _w) : x(_x), y(_y), z(_z), w(_w) {}
    Real x, y, z, w;
    Real* ptr() { return &x; }
    const Real* ptr() const { return &x; }
};

/// base template class for vectors
template<int dims>
class _OgreExport Vector : public VectorData<dims>
{
public:
    Vector() {}
    Vector(Real _x, Real _y) : VectorData<dims>(_x, _y) {}
    Vector(Real _x, Real _y, Real _z) : VectorData<dims>(_x, _y, _z) {}
    Vector(Real _x, Real _y, Real _z, Real _w) : VectorData<dims>(_x, _y, _z, _w) {}

    template<typename T>
    explicit Vector(const T* ptr) {
        for (int i = 0; i < dims; i++)
            this->ptr()[i] = ptr[i];
    }

    explicit Vector(Real s)
    {
        for (int i = 0; i < dims; i++)
            this->ptr()[i] = s;
    }

    /** Swizzle-like narrowing operations
    */
    Vector<3> xyz() const
    {
        static_assert(dims > 3, "just use assign");
        return Vector<3>(this->ptr());
    }
    Vector<2> xy() const
    {
        static_assert(dims > 2, "just use assign");
        return Vector<2>(this->ptr());
    }

    /** Exchange the contents of this vector with another.
    */
    void swap(Vector& other)
    {
        for (int i = 0; i < dims; i++)
            std::swap((*this)[i], other[i]);
    }

    Real operator[](size_t i) const
    {
        assert(i < dims);
        return *(this->ptr() + i);
    }

    Real& operator[](size_t i)
    {
        assert(i < dims);
        return *(this->ptr() + i);
    }

    bool operator==(const Vector& v) const
    {
        for (int i = 0; i < dims; i++)
            if ((*this)[i] != v[i])
                return false;
        return true;
    }

    /** Returns whether this vector is within a positional tolerance
        of another vector.
    @param rhs The vector to compare with
    @param tolerance The amount that each element of the vector may vary by
        and still be considered equal
    */
    bool positionEquals(const Vector& rhs, Real tolerance = 1e-03) const
    {
        for (int i = 0; i < dims; i++)
            if (!Math::RealEqual((*this)[i], rhs[i], tolerance))
                return false;
        return true;
    }

    bool operator!=(const Vector& v) const { return !(*this == v); }

    /** Returns true if the vector's scalar components are all greater
        that the ones of the vector it is compared against.
    */
    bool operator<(const Vector& rhs) const
    {
        for (int i = 0; i < dims; i++)
            if (!((*this)[i] < rhs[i]))
                return false;
        return true;
    }

    /** Returns true if the vector's scalar components are all smaller
        that the ones of the vector it is compared against.
    */
    bool operator>(const Vector& rhs) const
    {
        for (int i = 0; i < dims; i++)
            if (!((*this)[i] > rhs[i]))
                return false;
        return true;
    }

    /** Sets this vector's components to the minimum of its own and the
        ones of the passed in vector.
        @remarks
            'Minimum' in this case means the combination of the lowest
            value of x, y and z from both vectors. Lowest is taken just
            numerically, not magnitude, so -1 < 0.
    */
    void makeFloor(const Vector& cmp)
    {
        for (int i = 0; i < dims; i++)
            if (cmp[i] < (*this)[i])
                (*this)[i] = cmp[i];
    }

    /** Sets this vector's components to the maximum of its own and the
        ones of the passed in vector.
        @remarks
            'Maximum' in this case means the combination of the highest
            value of x, y and z from both vectors. Highest is taken just
            numerically, not magnitude, so 1 > -3.
    */
    void makeCeil(const Vector& cmp)
    {
        for (int i = 0; i < dims; i++)
            if (cmp[i] > (*this)[i])
                (*this)[i] = cmp[i];
    }

    /** Calculates the dot (scalar) product of this vector with another.
        @remarks
            The dot product can be used to calculate the angle between 2
            vectors. If both are unit vectors, the dot product is the
            cosine of the angle; otherwise the dot product must be
            divided by the product of the lengths of both vectors to get
            the cosine of the angle. This result can further be used to
            calculate the distance of a point from a plane.
        @param
            vec Vector with which to calculate the dot product (together
            with this one).
        @return
            A float representing the dot product value.
    */
    Real dotProduct(const Vector& vec) const
    {
        Real ret = 0;
        for (int i = 0; i < dims; i++)
            ret += (*this)[i] * vec[i];
        return ret;
    }

    /** Calculates the cross-product of 2 vectors, i.e. the vector that
        lies perpendicular to them both.
        @remarks
            The cross-product is normally used to calculate the normal
            vector of a plane, by calculating the cross-product of 2
            non-equivalent vectors which lie on the plane (e.g. 2 edges
            of a triangle).
        @param rkVector
            Vector which, together with this one, will be used to
            calculate the cross-product.
        @return
            A vector which is the result of the cross-product. This
            vector will <b>NOT</b> be normalised, to maximise efficiency
            - call Vector3::normalise on the result if you wish this to
            be done. As for which side the resultant vector will be on, the
            returned vector will be on the side from which the arc from 'this'
            to rkVector is anticlockwise, e.g. UNIT_Y.crossProduct(UNIT_Z)
            = UNIT_X, whilst UNIT_Z.crossProduct(UNIT_Y) = -UNIT_X.
            This is because OGRE uses a right-handed coordinate system.
        @par
            For a clearer explanation, look a the left and the bottom edges
            of your monitor's screen. Assume that the first vector is the
            left edge and the second vector is the bottom edge, both of
            them starting from the lower-left corner of the screen. The
            resulting vector is going to be perpendicular to both of them
            and will go <i>inside</i> the screen, towards the cathode tube
            (assuming you're using a CRT monitor, of course).
    */
    Vector crossProduct( const Vector& rkVector ) const;

    /** Generates a vector perpendicular to this vector (eg an 'up' vector).
        @remarks
            This method will return a vector which is perpendicular to this
            vector. There are an infinite number of possibilities but this
            method will guarantee to generate one of them. If you need more
            control you should use the Quaternion class.
    */
    Vector perpendicular(void) const;

    /** Returns the square of the length(magnitude) of the vector.
        @remarks
            This  method is for efficiency - calculating the actual
            length of a vector requires a square root, which is expensive
            in terms of the operations required. This method returns the
            square of the length of the vector, i.e. the same as the
            length but before the square root is taken. Use this if you
            want to find the longest / shortest vector without incurring
            the square root.
    */
    Real squaredLength() const { return dotProduct(*this); }

    /** Returns true if this vector is zero length. */
    bool isZeroLength() const
    {
        return squaredLength() < 1e-06 * 1e-06;
    }

    /** Returns the length (magnitude) of the vector.
        @warning
            This operation requires a square root and is expensive in
            terms of CPU operations. If you don't need to know the exact
            length (e.g. for just comparing lengths) use squaredLength()
            instead.
    */
    Real length() const { return Math::Sqrt(squaredLength()); }

    /** Returns the distance to another vector.
        @warning
            This operation requires a square root and is expensive in
            terms of CPU operations. If you don't need to know the exact
            distance (e.g. for just comparing distances) use squaredDistance()
            instead.
    */
    Real distance(const Vector& rhs) const
    {
        return (*this - rhs).length();
    }

    /** Returns the square of the distance to another vector.
        @remarks
            This method is for efficiency - calculating the actual
            distance to another vector requires a square root, which is
            expensive in terms of the operations required. This method
            returns the square of the distance to another vector, i.e.
            the same as the distance but before the square root is taken.
            Use this if you want to find the longest / shortest distance
            without incurring the square root.
    */
    Real squaredDistance(const Vector& rhs) const
    {
        return (*this - rhs).squaredLength();
    }

    /** Normalises the vector.
        @remarks
            This method normalises the vector such that it's
            length / magnitude is 1. The result is called a unit vector.
        @note
            This function will not crash for zero-sized vectors, but there
            will be no changes made to their components.
        @return The previous length of the vector.
    */
    Real normalise()
    {
        Real fLength = length();

        // Will also work for zero-sized vectors, but will change nothing
        // We're not using epsilons because we don't need to.
        // Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
        if (fLength > Real(0.0f))
        {
            Real fInvLength = 1.0f / fLength;
            for (int i = 0; i < dims; i++)
                (*this)[i] *= fInvLength;
        }

        return fLength;
    }

    /** As normalise, except that this vector is unaffected and the
        normalised vector is returned as a copy. */
    Vector normalisedCopy() const
    {
        Vector ret = *this;
        ret.normalise();
        return ret;
    }

    /// Check whether this vector contains valid values
    bool isNaN() const
    {
        for (int i = 0; i < dims; i++)
            if (Math::isNaN(this->ptr()[i]))
                return true;
        return false;
    }

    /** Gets the angle between 2 vectors.
    @remarks
        Vectors do not have to be unit-length but must represent directions.
    */
    Radian angleBetween(const Vector& dest) const
    {
        Real lenProduct = length() * dest.length();

        // Divide by zero check
        if(lenProduct < 1e-6f)
            lenProduct = 1e-6f;

        Real f = dotProduct(dest) / lenProduct;

        f = Math::Clamp(f, (Real)-1.0, (Real)1.0);
        return Math::ACos(f);

    }

    /** Calculates a reflection vector to the plane with the given normal .
    @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
    */
    Vector reflect(const Vector& normal) const { return *this - (2 * dotProduct(normal) * normal); }

    // transparent conversion to legacy impl
    template <int N = dims, typename = typename std::enable_if<N == 3>::type>
    operator const Vector3&()
    {
        return static_cast<Vector3&>(*this);
    }

    template <int N = dims, typename = typename std::enable_if<N == 2>::type>
    operator const Vector2&()
    {
        return static_cast<Vector2&>(*this);
    }
};

/// base template class for matrices
template <int rows, int cols>
class _OgreExport Matrix
{
protected:
    /// The matrix entries, indexed by [row][col].
    Real m[rows][cols];

public:
    /** Exchange the contents of this matrix with another.
    */
    void swap(Matrix& other)
    {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                std::swap(m[i][j], other.m[i][j]);
    }

    Real* operator[](size_t iRow)
    {
        assert(iRow < rows);
        return m[iRow];
    }

    const Real* operator[](size_t iRow) const
    {
        assert(iRow < rows);
        return m[iRow];
    }

    /** Tests 2 matrices for equality.
    */
    bool operator==(const Matrix& m2) const
    {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (m[i][j] != m2.m[i][j])
                    return false;
        return true;
    }

    /** Tests 2 matrices for inequality.
    */
    bool operator!=(const Matrix& m2) const { return !(*this == m2); }

    Matrix transpose(void) const
    {
        Matrix r;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                r.m[i][j] = m[j][i];
        return r;
    }

    // only implemented in some specializations
    Matrix inverse() const;
    Real determinant() const;

    /** Determines if this matrix involves a scaling. */
    bool hasScale() const
    {
        // check magnitude of column vectors (==local axes)
        Real t = m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0];
        if (!Math::RealEqual(t, 1.0, (Real)1e-04))
            return true;
        t = m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1];
        if (!Math::RealEqual(t, 1.0, (Real)1e-04))
            return true;
        t = m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2];
        if (!Math::RealEqual(t, 1.0, (Real)1e-04))
            return true;

        return false;
    }
};

// actually available specializations
template<> Matrix<3, 3> Matrix<3, 3>::inverse() const;
template<> Matrix<4, 4> Matrix<4, 4>::inverse() const;
template<> Real Matrix<3, 3>::determinant() const;
template<> Real Matrix<4, 4>::determinant() const;

template<>
inline Vector<3> Vector<3>::crossProduct( const Vector<3>& rkVector ) const
{
    return Vector<3>(
        this->ptr()[1] * rkVector.z - this->ptr()[2] * rkVector.y,
        this->ptr()[2] * rkVector.x - this->ptr()[0] * rkVector.z,
        this->ptr()[0] * rkVector.y - this->ptr()[1] * rkVector.x);
}

template<>
inline Vector<2> Vector<2>::perpendicular(void) const
{
    return Vector<2>(-y, x);
}

template<>
inline Vector<3> Vector<3>::perpendicular() const
{
    // From Sam Hocevar's article "On picking an orthogonal
    // vector (and combing coconuts)"
    Vector<3> perp = Math::Abs(x) > Math::Abs(z)
                 ? Vector<3>(-y, x, 0.0) : Vector<3>(0.0, -z, y);
    return perp.normalisedCopy();
}

// Scalar * Vector
template <int N>
static inline Vector<N> operator-(const Vector<N>& v)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = -v[i];
    return ret;
}

template <int N>
static inline Vector<N> operator*(const Real s, const Vector<N>& v)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = s * v[i];
    return ret;
}

template <int N>
static inline Vector<N> operator*(const Vector<N>& v, const Real s)
{
    return s * v;
}

template <int N>
static inline Vector<N> operator/(const Real s, const Vector<N>& v)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = s / v[i];
    return ret;
}

template <int N>
static inline Vector<N> operator/(const Vector<N>& v, const Real s)
{
    assert( s != 0.0 ); // legacy assert
    Real fInv = 1.0f / s;
    return fInv * v;
}

template <int N>
static inline Vector<N> operator+(const Real s, const Vector<N>& v)
{
    return v + s;
}

template <int N>
static inline Vector<N> operator-(const Vector<N>& v, const Real s)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = v[i] - s;
    return ret;
}

template <int N>
static inline Vector<N> operator-(const Real s, const Vector<N>& v)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = s - v[i];
    return ret;
}

template <int N>
static inline Vector<N> operator+(const Vector<N>& v, const Real s)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = s + v[i];
    return ret;
}

// Vector * Vector
template <int N>
static inline Vector<N> operator+(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = a[i] + b[i];
    return ret;
}

template <int N>
static inline Vector<N> operator-(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = a[i] - b[i];
    return ret;
}

template <int N>
static inline Vector<N> operator*(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = a[i] * b[i];
    return ret;
}

template <int N>
static inline Vector<N> operator/(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> ret;
    for (int i = 0; i < N; i++)
        ret[i] = a[i] / b[i];
    return ret;
}

// Vector: arithmetic updates
template <int N>
static inline Vector<N>& operator*=(Vector<N>& v, const Real s)
{
    for (int i = 0; i < N; i++)
        v[i] *= s;
    return v;
}

template <int N>
static inline Vector<N>& operator/=(Vector<N>& v, const Real s)
{
    assert( s != 0.0 ); // legacy assert
    Real fInv = 1.0f/s;
    for (int i = 0; i < N; i++)
        v[i] *= fInv;
    return v;
}


template <int N>
static inline Vector<N>& operator+=(Vector<N>& v, const Real s)
{
    for (int i = 0; i < N; i++)
        v[i] += s;
    return v;
}

template <int N>
static inline Vector<N>& operator-=(Vector<N>& v, const Real s)
{
    for (int i = 0; i < N; i++)
        v[i] -= s;
    return v;
}

template <int N>
static inline Vector<N> operator+=(Vector<N>& a, const Vector<N>& b)
{
    for (int i = 0; i < N; i++)
        a[i] += b[i];
    return a;
}

template <int N>
static inline Vector<N> operator-=(Vector<N>& a, const Vector<N>& b)
{
    for (int i = 0; i < N; i++)
        a[i] -= b[i];
    return a;
}

template <int N>
static inline Vector<N> operator*=(Vector<N>& a, const Vector<N>& b)
{
    for (int i = 0; i < N; i++)
        a[i] *= b[i];
    return a;
}

template <int N>
static inline Vector<N>& operator/=(Vector<N>& a, const Vector<N>& b)
{
    for (int i = 0; i < N; i++)
        a[i] /= b[i];
    return a;
}

// Vector * matrix
template <int N, int M>
static inline Vector<M> operator*(const Vector<N>& v, const Matrix<N, M>& m)
{
    Vector<M> ret(0);
    for (int j = 0; j < M; j++)
        for (int i = 0; i < N; i++)
            ret[i] += m[i][j] * v[j];
    return ret;
}

template <int N, int M>
static inline Vector<N> operator*(const Matrix<N, M>& m, const Vector<M>& v)
{
    Vector<N> ret(0);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            ret[i] += m[i][j] * v[j];
    return ret;
}

// Scalar * matrix
template <int N, int M>
static inline Matrix<N, M> operator*(const Matrix<N, M>& m, Real scalar)
{
    Matrix<N, M> r;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            r[i][j] = scalar * m[i][j];
    return r;
}
template <int N, int M>
static inline Matrix<N, M> operator*(Real scalar, const Matrix<N, M>& matrix)
{
    return matrix * scalar;
}

// Matrix * Matrix
template <int N, int M>
static inline Matrix<N, M> operator*(const Matrix<M, N>& m, const Matrix<M, N>& m2)
{
    Matrix<N, M> r;

    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
        {
            Real s = 0;
            for (int k = 0; k < M; k += 4)
                s += m[i][k] * m2[k][j] + m[i][k + 1] * m2[k + 1][j] + m[i][k + 2] * m2[k + 2][j] + m[i][k + 3] * m2[k + 3][j];
            r[i][j] = s;
        }

    return r;
}

template <int N, int M>
static inline Matrix<N, M> operator+(const Matrix<M, N>& m, const Matrix<N, M>& m2)
{
    Matrix<N, M> r;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            r[i][j] = m[i][j] + m2[i][j];
    return r;
}

template <int N, int M>
static inline Matrix<N, M> operator-(const Matrix<M, N>& m, const Matrix<N, M>& m2)
{
    Matrix<N, M> r;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            r[i][j] = m[i][j] - m2[i][j];
    return r;
}

// Functions for writing to a stream.
template <int N, int M>
static inline std::ostream& operator<<(std::ostream& o, const Matrix<N, M>& mat)
{
    o << "Matrix" << N << "(";

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            o << mat[i][j];
            if (j != M - 1)
                o << ", ";
        }
        if (i != N - 1)
            o << "; ";
    }
    o << ")";
    return o;
}

template <int dims>
static inline std::ostream& operator<<(std::ostream& o, const Vector<dims>& v)
{
    o << "Vector" << dims << "(";
    for (int i = 0; i < dims; i++) {
        o << v[i];
        if(i != dims - 1) o << ", ";
    }
    o <<  ")";
    return o;
}

/** @} */
/** @} */
}
#endif
