// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <math.h>
#include <stdlib.h>

#define kVecEpsilon 1e-3f

struct Vec4;

struct Vec2
{
    float m_X[2];
    
    Vec2()    {}
    
    Vec2(float x, float y)
    {
        m_X[0] = x;
        m_X[1] = y;
    }
    
    inline Vec2& operator*= (float value)
    {
        m_X[0] *= value;
        m_X[1] *= value;
        return *this;
    }
    
    inline Vec2& operator*= (const Vec2& rhs)
    {
        m_X[0] *= rhs.m_X[0];
        m_X[1] *= rhs.m_X[1];
        return *this;
    }
    
    inline Vec2 operator* (float value) const
    {
        Vec2 ret(*this);
        ret *= value;
        return ret;
    }
    
    inline Vec2 operator* (const Vec2& value) const
    {
        Vec2 ret(*this);
        ret[0] *= value[0];
        ret[1] *= value[1];
        return ret;
    }
    
    inline Vec2& operator+= (const Vec2& value)
    {
        m_X[0] += value.m_X[0];
        m_X[1] += value.m_X[1];
        return *this;
    }
    
    inline Vec2& operator-= (const Vec2& value)
    {
        m_X[0] -= value.m_X[0];
        m_X[1] -= value.m_X[1];
        return *this;
    }
    
    inline Vec2 operator- (const Vec2& value) const
    {
        Vec2 ret(*this);
        ret -= value;
        return ret;
    }

    inline float& operator[] (size_t index)
    {
        return asFloat()[index];
    }

    inline float operator[] (size_t index) const
    {
        return asFloat()[index];
    }
    
    inline float LengthSquared() const
    {
        return m_X[0]*m_X[0]+m_X[1]*m_X[1];
    }
    
    inline float Length() const
    {
        return sqrtf(LengthSquared());
    }
    
    inline void Normalize()
    {
        const float lengthSquared = LengthSquared();
        if (lengthSquared > kVecEpsilon)
        {
            float len = sqrtf(lengthSquared);
            m_X[0] /= len;
            m_X[1] /= len;
        }
    }
    
    inline Vec2 Normalized() const
    {
        Vec2 ret(*this);
        ret.Normalize();
        return ret;
    }
    
    inline Vec2 xy() const
    {
        return *this;
    }

    inline Vec2& xy()
    {
        return *(Vec2*)asFloat();
    }
    
    inline float& x()
    {
        return asFloat()[0];
    }
    
    inline float& y()
    {
        return asFloat()[1];
    }
    
    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
};

inline Vec2 operator+ (const Vec2& a, const Vec2& b)
{
    Vec2 ret(a);
    ret += b;
    return ret;
}

struct Vec3
{
    static Vec3 kZero;
    static Vec3 kUp;
    float m_X[3];

    Vec3()    {}
    
    Vec3(float x, float y, float z)
    {
        m_X[0] = x;
        m_X[1] = y;
        m_X[2] = z;
    }
    
    Vec3(const float v[3])
    {
        m_X[0] = v[0];
        m_X[1] = v[1];
        m_X[2] = v[2];
    }

    Vec3(const Vec2& v2, float z)
    {
        m_X[0] = v2.m_X[0];
        m_X[1] = v2.m_X[1];
        m_X[2] = z;
    }
    
    inline Vec3& operator*= (float value)
    {
        m_X[0] *= value;
        m_X[1] *= value;
        m_X[2] *= value;
        return *this;
    }
    
    inline Vec3 operator* (float value) const
    {
        Vec3 ret(*this);
        ret *= value;
        return ret;
    }
    
    inline Vec3& operator+= (const Vec3& value)
    {
        m_X[0] += value.m_X[0];
        m_X[1] += value.m_X[1];
        m_X[2] += value.m_X[2];
        return *this;
    }
    
    inline Vec3& operator-= (const Vec3& value)
    {
        m_X[0] -= value.m_X[0];
        m_X[1] -= value.m_X[1];
        m_X[2] -= value.m_X[2];
        return *this;
    }
    
    inline Vec3 operator- (const Vec3& value) const
    {
        Vec3 ret(*this);
        ret -= value;
        return ret;
    }
    
    inline Vec3 operator- () const
    {
        Vec3 ret(-m_X[0], -m_X[1], -m_X[2]);
        return ret;
    }
    
    inline Vec3& operator/= (float f)
    {
        m_X[0] /= f;
        m_X[1] /= f;
        m_X[2] /= f;
        return *this;
    }

    inline float& operator[](size_t index)
    {
        return m_X[index];
    }

    inline float operator[](size_t index) const
    {
        return m_X[index];
    }
    
    inline void Splat(float value)
    {
        m_X[0] = m_X[1] = m_X[2] = value;
    }
    
    inline float LengthSquared() const
    {
        return m_X[0]*m_X[0]+m_X[1]*m_X[1]+m_X[2]*m_X[2];
    }
    
    inline float Length() const
    {
        return sqrtf(LengthSquared());
    }
    
    inline void Normalize()
    {
        const float lengthSquared = LengthSquared();
        if (lengthSquared > kVecEpsilon)
        {
            float len = sqrtf(lengthSquared);
            m_X[0] /= len;
            m_X[1] /= len;
            m_X[2] /= len;
        }
    }

    inline Vec3 Normalized() const
    {
        Vec3 ret = *this;
        const float lengthSquared = LengthSquared();
        if (lengthSquared > kVecEpsilon)
        {
            float len = 1.0f / sqrtf(lengthSquared);
            ret *= len;
        }
        return ret;
    }
    
    inline Vec2 xy() const
    {
        return Vec2(m_X[0], m_X[1]);
    }
    
    inline Vec3 xy(float z) const
    {
        return Vec3(m_X[0], m_X[1], z);
    }
    
    inline Vec4 xyz1() const;
    inline Vec4 xyz0() const;
    
    inline float x() const
    {
        return m_X[0];
    }
    
    inline float& x()
    {
        return m_X[0];
    }

    inline float y() const
    {
        return m_X[1];
    }
    
    inline float& y()
    {
        return m_X[1];
    }
    
    inline float z() const
    {
        return m_X[2];
    }
    
    inline float& z()
    {
        return m_X[2];
    }

    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
};

inline Vec3 operator+ (const Vec3& a, const Vec3& b)
{
    Vec3 ret(a);
    ret += b;
    return ret;
}

inline Vec3 operator* (float v, const Vec3& a)
{
    Vec3 ret(a);
    ret *= v;
    return ret;
}

inline Vec2 operator* (float v, const Vec2& a)
{
    Vec2 ret(a);
    ret *= v;
    return ret;
}

struct Vec4
{
    static Vec4 kZero;
    
    float m_X[4];
    
    Vec4()
    {
    }
    
    Vec4(float x, float y, float z, float w)
    {
        m_X[0] = x;
        m_X[1] = y;
        m_X[2] = z;
        m_X[3] = w;
    }
    
    Vec4(Vec3 value, float w)
    {
        m_X[0] = value.m_X[0];
        m_X[1] = value.m_X[1];
        m_X[2] = value.m_X[2];
        m_X[3] = w;
    }
    
    Vec4(Vec2 value, float z, float w)
    {
        m_X[0] = value.m_X[0];
        m_X[1] = value.m_X[1];
        m_X[2] = z;
        m_X[3] = w;
    }
    
    Vec4(Vec3 value)
    {
        m_X[0] = value.m_X[0];
        m_X[1] = value.m_X[1];
        m_X[2] = value.m_X[2];
        m_X[3] = 0.0f;
    }
    
    float w() const
    {
        return m_X[3];
    }
    
    inline Vec4& operator/=(float v)
    {
        m_X[0] /= v;
        m_X[1] /= v;
        m_X[2] /= v;
        m_X[3] /= v;
        return *this;
    }
    
    inline Vec4 operator/(float v)
    {
        Vec4 ret(*this);
        ret /= v;
        return ret;
    }
    
    inline Vec4& operator*= (float value)
    {
        m_X[0] *= value;
        m_X[1] *= value;
        m_X[2] *= value;
        m_X[3] *= value;
        return *this;
    }
    
    inline Vec4 operator* (float value) const
    {
        Vec4 ret(*this);
        ret *= value;
        return ret;
    }
    
    inline Vec4 operator* (const Vec4& rhs) const
    {
        Vec4 ret(*this);
        ret[0] *= rhs[0];
        ret[1] *= rhs[1];
        ret[2] *= rhs[2];
        ret[3] *= rhs[3];
        return ret;
    }
    
    inline Vec4& operator+= (const Vec4& value)
    {
        m_X[0] += value.m_X[0];
        m_X[1] += value.m_X[1];
        m_X[2] += value.m_X[2];
        m_X[3] += value.m_X[3];
        return *this;
    }
    
    inline Vec4 operator+ (const Vec4& value)
    {
        Vec4 ret(*this);
        ret += value;
        return ret;
    }
    
    inline Vec4& operator-= (const Vec4& value)
    {
        m_X[0] -= value.m_X[0];
        m_X[1] -= value.m_X[1];
        m_X[2] -= value.m_X[2];
        m_X[3] -= value.m_X[3];
        return *this;
    }
    
    inline Vec4 operator- (const Vec4& value) const
    {
        Vec4 ret(*this);
        ret -= value;
        return ret;
    }

    inline Vec4 operator- () const
    {
        Vec4 ret(*this);
        ret *= -1.0f;
        return ret;
    }
    
    inline float x() const
    {
        return asFloat()[0];
    }

    inline float& x()
    {
        return asFloat()[0];
    }
    
    inline float y() const
    {
        return asFloat()[1];
    }
    
    inline float& y()
    {
        return asFloat()[1];
    }
    
    inline float z() const
    {
        return asFloat()[2];
    }
    
    inline float& z()
    {
        return asFloat()[2];
    }
    
    inline Vec2 xy() const
    {
        return Vec2(m_X[0], m_X[1]);
    }

    inline Vec2& xy()
    {
        return *(Vec2*)(asFloat());
    }

    inline Vec3 xy(float z) const
    {
        return Vec3(m_X[0], m_X[1], z);
    }
    
    inline Vec2 zw() const
    {
        return Vec2(m_X[2], m_X[3]);
    }
    
    inline Vec2& zw()
    {
        return *(Vec2*)(asFloat()+2);
    }
    
    inline Vec3 xyz() const
    {
        return Vec3(m_X[0], m_X[1], m_X[2]);
    }
    
    inline Vec3& xyz()
    {
        return *(Vec3*)(asFloat());
    }
    
    inline Vec4 xyz0() const
    {
        return Vec4(m_X[0], m_X[1], m_X[2], 0.0f);
    }
    
    inline Vec4 xyz1() const
    {
        return Vec4(m_X[0], m_X[1], m_X[2], 1.0f);
    }

    inline float& operator[](size_t index)
    {
        return asFloat()[index];
    }
    
    inline float operator[](size_t index) const
    {
        return asFloat()[index];
    }
    
    inline void xy(const Vec2& xy)
    {
        m_X[0] = xy.m_X[0];
        m_X[1] = xy.m_X[1];
    }
    
    inline void yx(const Vec2& xy)
    {
        m_X[1] = xy.m_X[0];
        m_X[0] = xy.m_X[1];
    }
    
    inline Vec2 yx() const
    {
        return Vec2(m_X[1], m_X[0]);
    }
    
    inline void zw(const Vec2& zw)
    {
        m_X[2] = zw.m_X[0];
        m_X[3] = zw.m_X[1];
    }
    
    inline float LengthSquared() const
    {
        float t = 0.0f;
        t += m_X[0]*m_X[0];
        t += m_X[1]*m_X[1];
        t += m_X[2]*m_X[2];
        t += m_X[3]*m_X[3];
        return t;
    }
    
    inline float Length() const
    {
        return sqrtf(LengthSquared());
    }
    
    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
};


inline Vec4 Vec3::xyz1() const
{
    return Vec4(m_X[0], m_X[1], m_X[2], 1.0f);
}

inline Vec4 Vec3::xyz0() const
{
    return Vec4(m_X[0], m_X[1], m_X[2], 0.0f);
}

struct Mat3
{
    float m_X[3];
    float m_Y[3];
    float m_Z[3];
    
    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
    
    inline Vec3& operator[](size_t index)
    {
        Vec3* ret = (Vec3*) (asFloat() + index*3);
        return *ret;
    }
    
    inline const Vec3& operator[](size_t index) const
    {
        const Vec3* ret = (const Vec3*) (asFloat() + index*3);
        return *ret;
    }
    
    inline Mat3 operator/= (float f)
    {
        const float rf = 1.0f / f;
        for (int i=0; i<3; ++i)
        {
            m_X[i] *= rf;
            m_Y[i] *= rf;
            m_Z[i] *= rf;
        }
        return *this;
    }

    // Transpose in place
    void Transpose();

    // Scale in place
    void Scale(float value);

    // true if the identity matrix, within epsilon tolerance
    bool IsIdentity(float epsilon) const;

    float Determinant() const;

    // given an eigenvalue lambda, calculate eigenvector
    Vec3 CalculateEigenvector(float lambda) const;
};

struct Mat4
{
    float m_X[4];
    float m_Y[4];
    float m_Z[4];
    float m_W[4];

    inline void Set(float a, float b, float c, float d,
                    float e, float f, float g, float h,
                    float i, float j, float k, float l,
                    float m, float n, float o, float p)
    {
        m_X[0] = a; m_X[1] = b; m_X[2] = c; m_X[3] = d;
        m_Y[0] = e; m_Y[1] = f; m_Y[2] = g; m_Y[3] = h;
        m_Z[0] = i; m_Z[1] = j; m_Z[2] = k; m_Z[3] = l;
        m_W[0] = m; m_W[1] = n; m_W[2] = o; m_W[3] = p;
    }
    
    inline void SetRot(const Mat3& mat33)
    {
        for (int i=0; i<3; ++i)
        {
            m_X[i] = mat33.m_X[i];
            m_Y[i] = mat33.m_Y[i];
            m_Z[i] = mat33.m_Z[i];
        }
    }
    
    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
    
    inline Vec3 GetTranslation() const
    {
        return Vec3(m_W[0], m_W[1], m_W[2]);
    }
    
    inline Vec3 GetUp() const
    {
        return Vec3(m_Y[0], m_Y[1], m_Y[2]);
    }
    
    inline Vec3 GetRight() const
    {
        return -Vec3(m_X[0], m_X[1], m_X[2]);
    }
    
    inline Vec3 GetForward() const
    {
        return Vec3(m_Z[0], m_Z[1], m_Z[2]);
    }
    
    inline void AddTranslation(Vec3 pos)
    {
        m_W[0] += pos.m_X[0];
        m_W[1] += pos.m_X[1];
        m_W[2] += pos.m_X[2];
    }
    
    inline void AddTranslation(float x, float y, float z)
    {
        m_W[0] += x;
        m_W[1] += y;
        m_W[2] += z;
    }
    
    inline void NegateTranslation()
    {
        m_W[0] *= -1.0f;
        m_W[1] *= -1.0f;
        m_W[2] *= -1.0f;
        m_W[3] = 1.0f;
    }
    
    inline void SetTranslation(const Vec3& pos)
    {
        m_W[0] = pos.m_X[0];
        m_W[1] = pos.m_X[1];
        m_W[2] = pos.m_X[2];
        m_W[3] = 1.0f;
    }
    
    inline void SetTranslation(const Vec4& pos)
    {
        m_W[0] = pos.m_X[0];
        m_W[1] = pos.m_X[1];
        m_W[2] = pos.m_X[2];
        m_W[3] = pos.m_X[3];
    }
    
    inline void SetTranslation(float x, float y, float z)
    {
        m_W[0] = x;
        m_W[1] = y;
        m_W[2] = z;
        m_W[3] = 1.0f;
    }
    
    inline void SetUp(const Vec3& pos)
    {
        m_Y[0] = pos.m_X[0];
        m_Y[1] = pos.m_X[1];
        m_Y[2] = pos.m_X[2];
        m_Y[3] = 0.0f;
    }
    
    inline void SetForward(const Vec3& pos)
    {
        m_Z[0] = pos.m_X[0];
        m_Z[1] = pos.m_X[1];
        m_Z[2] = pos.m_X[2];
        m_Z[3] = 0.0f;
    }
    
    inline void SetRight(const Vec3& pos)
    {
        m_X[0] = pos.m_X[0];
        m_X[1] = pos.m_X[1];
        m_X[2] = pos.m_X[2];
        m_X[3] = 0.0f;
    }
    
    inline Mat4 operator/= (float f)
    {
        const float rf = 1.0f / f;
        for (int i=0; i<4; ++i)
        {
            m_X[i] *= rf;
            m_Y[i] *= rf;
            m_Z[i] *= rf;
            m_W[i] *= rf;
        }
        return *this;
    }

    // Transpose in place
    void Transpose();
    
    // Scale in place
    void Scale(float value);
    void Scale(const Vec3& value);

    void OrthoNormalize();
};

// splat value in dest
void VectorSplat(Vec3* dest, float value);

// component max/min
Vec3 VectorMax(const Vec3& a, const Vec3& b);
Vec3 VectorMin(const Vec3& a, const Vec3& b);
Vec4 VectorMax(const Vec4& a, const Vec4& b);
Vec4 VectorMin(const Vec4& a, const Vec4& b);

// project a on the plane defined by normal
Vec3 PlaneProj(const Vec3& a, const Vec3& normal);

// set dest explicitly
void VectorSet(Vec3* dest, float x, float y, float z);

// return distance squared
float DistanceSquared(const Vec4& a, const Vec4& b);
float DistanceSquared(const Vec3& a, const Vec3& b);

// simple distance
inline float Distance(const Vec2& a, const Vec2& b)
{
    return (a-b).Length();
}

// return the 2d vector orthogonal to a
Vec2 Orthogonal(const Vec2& a);

// returns inner product
float VectorDot(const Vec4& a, const Vec4& b);
float VectorDot(const Vec3& a, const Vec3& b);
float VectorDot(const Vec2& a, const Vec2& b);

// cross project
Vec3 VectorCross(const Vec3& a, const Vec3& b);

// returns length squared
float VectorLengthSquared(const Vec3& a);

// copy vector
void VectorCopy(Vec3* a, const Vec3& b);

// vector dump
void VectorDump(const Vec2& a, const char* prefix);
void VectorDump(const Vec3* a, const char* prefix);
void VectorDump(const Vec3& a, const char* prefix);
void VectorDump(const Vec4* a, const char* prefix);
void VectorDump(const Vec4& a, const char* prefix);

// matrix dump
void MatrixDump(const Mat4& m, const char* prefix);
void MatrixDump(const Mat3& m, const char* prefix);

// return a vector with -1,0,1 in the component if the corresponding component in temp is negative,0, positive
Vec3 VectorSign(const Vec3& temp);

// number of negative components of argument
int VectorNegativeCount(const Vec3& a);

int VectorEqual(const Vec3& a, const Vec3& b);
int VectorApproxEqual(const Vec3& a, const Vec3& b, float eps=1e-3f);

void MatrixMakeIdentity(Mat3* dest);
void MatrixMakeIdentity(Mat4* dest);

void MatrixMakeRandy(Mat3* dest, float mid, float extent);
void MatrixMakeRandy(Mat4* dest, float mid, float extent);

void MatrixMakeZero(Mat3* dest);
void MatrixMakeZero(Mat4* dest);

void MatrixMakeDiagonal(Mat3* dest, float diags[3]);

void MatrixMultiply(Mat4* dest, const Mat4& a, const Mat4& b);
void MatrixMultiply(Mat3* dest, const Mat3& a, const Mat3& b);

// v is a row vector.
void MatrixMultiplyVec(Vec4* dest, const Vec4& v, const Mat4& a);

// v is a column vector.
void MatrixMultiplyVec(Vec4* dest, const Mat4& a, const Vec4& v);
void MatrixMultiplyVec(Vec3* dest, const Mat3& a, const Vec3& v);

inline Mat4 operator* (const Mat4& lhs, const Mat4& rhs)
{
    Mat4 temp;
    MatrixMultiply(&temp, lhs, rhs);
    return temp;
}
inline Mat3 operator* (const Mat3& lhs, const Mat3& rhs)
{
    Mat3 temp;
    MatrixMultiply(&temp, lhs, rhs);
    return temp;
}

inline Vec4 operator* (float f, const Vec4& rhs)
{
    Vec4 temp = rhs;
    temp *= f;
    return temp;
}

inline Vec4 operator* (const Vec4& lhs, const Mat4& rhs)
{
    Vec4 temp;
    MatrixMultiplyVec(&temp, lhs, rhs);
    return temp;
}

float MatrixDotTransposed(const Mat4& a, int col, const float* v);
float MatrixDotTransposed(const Mat3& a, int col, const float* v);

void MatrixSetRotAboutAxis(Mat4* dest, const float uvw[4], float theta);
void MatrixSetRotAboutAxis(Mat4* dest, const Vec3& axis, float theta);

// transpose matrix
void MatrixTranspose(Mat4* dest, const Mat4& a);

// general invert
void MatrixInvert(Mat4* dest, const Mat4& a);
void MatrixInvert(Mat3* dest, const Mat3& a);

// bool MatrixIsIdent(const Mat3& a, float epsilon);

// convert Mat4 to float array (or float array transposed)
void MatrixToGl(float temp[16], const Mat4& a);
void MatrixTransposeToGl(float temp[16], const Mat4& a);

void MatrixCalculateDelta(Mat4* dest, const Mat4& current, const Mat4& prev);

// FLOAT
bool FloatApproxEqual(float a, float b, float eps=1e-3f);

// misc
inline Vec3 ToZeroOne(const Vec3& a)
{
    Vec3 ret;
    ret.m_X[0] = (ret.m_X[0]+1.0f)*0.5f;
    ret.m_X[1] = (ret.m_X[1]+1.0f)*0.5f;
    ret.m_X[2] = (ret.m_X[2]+1.0f)*0.5f;
    return ret;
}

inline Vec2 FromZeroOne(const Vec2& a)
{
    Vec2 ret;
    ret.m_X[0] = (a.m_X[0]*2.0f)-1.0f;
    ret.m_X[1] = (a.m_X[1]*2.0f)-1.0f;
    return ret;
}

inline Vec3 FromZeroOne(const Vec3& a)
{
    Vec3 ret;
    ret.m_X[0] = (a.m_X[0]*2.0f)-1.0f;
    ret.m_X[1] = (a.m_X[1]*2.0f)-1.0f;
    ret.m_X[2] = (a.m_X[2]*2.0f)-1.0f;
    return ret;
}

inline Vec4 FromZeroOne(const Vec4& a)
{
    Vec4 ret;
    ret.m_X[0] = (a.m_X[0]*2.0f)-1.0f;
    ret.m_X[1] = (a.m_X[1]*2.0f)-1.0f;
    ret.m_X[2] = (a.m_X[2]*2.0f)-1.0f;
    ret.m_X[3] = (a.m_X[3]*2.0f)-1.0f;
    return ret;
}

inline Vec4 ToZeroOne(const Vec4& a)
{
    Vec4 ret;
    ret.m_X[0] = (a.m_X[0]+1.0f)*0.5f;
    ret.m_X[1] = (a.m_X[1]+1.0f)*0.5f;
    ret.m_X[2] = (a.m_X[2]+1.0f)*0.5f;
    ret.m_X[3] = (a.m_X[3]+1.0f)*0.5f;
    return ret;
}

inline bool operator!=(const Vec4& a, const Vec4& b)
{
    bool ret = true;
    ret = ret && FloatApproxEqual(a.m_X[0], b.m_X[0], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[1], b.m_X[1], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[2], b.m_X[2], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[3], b.m_X[3], 1e-3f);
    return ret;
}

inline bool operator!=(const Vec3& a, const Vec3& b)
{
    bool ret = true;
    ret = ret && FloatApproxEqual(a.m_X[0], b.m_X[0], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[1], b.m_X[1], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[2], b.m_X[2], 1e-3f);
    return ret;
}

inline bool operator!=(const Vec2& a, const Vec2& b)
{
    bool ret = true;
    ret = ret && FloatApproxEqual(a.m_X[0], b.m_X[0], 1e-3f);
    ret = ret && FloatApproxEqual(a.m_X[1], b.m_X[1], 1e-3f);
    return ret;
}

// mat3 diagonalize
void Mat3Diagonalize(Mat3* s, float lambda3[3], Mat3* sInv, const Mat3& a);

void Mat3InvertIterate(Vec3* dest, const Mat3& a, float lambda);

void Mat3Solve(Vec3* dest, const Mat3& a, const Vec3& b);

// diagonal can be null
void Mat3DecomposeLdu(Mat3* l, Mat3* d, Mat3* u, int p[3], const Mat3& a);

inline float Distance(float a, float b)
{
    return fabsf(a - b);
}

bool Mat3Test();
