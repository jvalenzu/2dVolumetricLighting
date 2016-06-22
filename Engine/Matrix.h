#pragma once

#include <math.h>

#define kVecEpsilon 1e-3f

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
    
    inline Vec2 operator* (float value) const
    {
        Vec2 ret(*this);
        ret *= value;
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
};

inline Vec2 operator+ (const Vec2& a, const Vec2& b)
{
    Vec2 ret(a);
    ret += b;
    return ret;
}

struct Vec3
{
    float m_X[3];

    Vec3()    {}
    
    Vec3(float x, float y, float z)
    {
        m_X[0] = x;
        m_X[1] = y;
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
};

inline Vec3 operator+ (const Vec3& a, const Vec3& b)
{
    Vec3 ret(a);
    ret += b;
    return ret;
}

struct Vec4
{
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
    
    Vec3 XYZ() const
    {
        return Vec3(m_X[0], m_X[1], m_X[2]);
    }
    
    float W() const
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
};

struct Mat4
{
    float m_X[4];
    float m_Y[4];
    float m_Z[4];
    float m_W[4];

    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
};

struct Mat3
{
    float m_X[3];
    float m_Y[3];
    float m_Z[3];
    
    float* asFloat() { return &m_X[0]; }
    const float* asFloat() const { return &m_X[0]; }
};

// splat value in dest
void VectorSplat(Vec3* dest, float value);

// component multiplication
Vec3 VectorMul(const Vec3& a, const Vec3& b);

// set dest explicitly
void VectorSet(Vec3* dest, float x, float y, float z);

// return distance squared
float VectorDistanceSquared(const Vec4& a, const Vec4& b);
float VectorDistanceSquared(const Vec3& a, const Vec3& b);

// returns inner product
float VectorDot(const Vec4& a, const Vec4& b);
float VectorDot(const Vec3& a, const Vec3& b);


// returns length squared
float VectorLengthSquared(const Vec3& a);

// normalize, one (destructive) and two argument mode
void VectorNormalize(Vec3* a);
void VectorNormalize(Vec3* dest, const Vec3& a);

// copy vector
void VectorCopy(Vec3* a, const Vec3& b);

void VectorDump(const Vec3* a, const char* prefix);
void VectorDump(const Vec3& a, const char* prefix);
void VectorDump(const Vec4* a, const char* prefix);
void VectorDump(const Vec4& a, const char* prefix);

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

void Mat4GetTranslation(const Mat4& dest, float* x, float* y, float* z);
Vec3 Mat4GetTranslation3(const Mat4& dest);

// set translation
void Mat4ApplyTranslation(Mat4* dest, float x, float y, float z);

// set translation
void Mat4ApplyTranslation(Mat4* dest, const Vec3& a);

void MatrixMultiply(Mat4* dest, const Mat4& a, const Mat4& b);
void MatrixMultiply(Mat3* dest, const Mat3& a, const Mat3& b);
void MatrixMultiplyVec(float dest[3], const float v[3], const Mat3& a);
void MatrixMultiplyVec(Vec4* dest, Vec4 v, const Mat4& a);
void MatrixMultiplyVecW0(float dest[3], const float v[3], const Mat4& a); // implied v.w=0
void MatrixMultiplyVecW1(Vec3* dest, Vec3 v, const Mat4& a); // implied v.w=0
void MatrixMultiplyVecW1(Vec4* dest, Vec3 v, const Mat4& a); // implied v.w=0
void MatrixMultiplyVec(float dest[4], const float v[4], const Mat4& a);
float MatrixDotTransposed(const Mat4& a, int col, const float* v);
float MatrixDotTransposed(const Mat3& a, int col, const float* v);
void MatrixCopy(Mat4* dest, const Mat4& a);

void MatrixSetRotAboutAxis(Mat4* dest, const float uvw[4], float theta);

// transpose matrix
void MatrixTranspose(Mat4* dest, const Mat4& a);
void MatrixTransposeInsitu(Mat4* dest);
void MatrixTransposeInsitu(Mat3* dest);

// general invert
void MatrixInvert(Mat4* dest, const Mat4& a);
void MatrixInvert(Mat3* dest, const Mat3& a);

// true if a is the identity matrix, within epsilon tolerance
bool MatrixIsIdent(const Mat3& a, float epsilon);

// convert Mat4 to float array (or float array transposed)
void MatrixToGl(float temp[16], const Mat4& a);
void MatrixTransposeToGl(float temp[16], const Mat4& a);

void MatrixCalculateDelta(Mat4* dest, const Mat4& current, const Mat4& prev);

void MatrixScaleInsitu(Mat4* dest, float value);
void MatrixScaleInsitu(Mat3* dest, float value);

void Vector3Dump(float v[3], const char* prefix);

void MatrixDump(const Mat4& m, const char* prefix);
void MatrixDump(const Mat3& m, const char* prefix);

void Mat3Diagonalize(Mat3* s, float lambda3[3], Mat3* sInv, const Mat3& a);

float MatrixDeterminant(const Mat3& m);

// FLOAT
bool FloatApproxEqual(float a, float b, float eps);
