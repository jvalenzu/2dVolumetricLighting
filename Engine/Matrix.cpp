#include <string.h>
#include <stdio.h>
#include <math.h>

#include "Engine/Matrix.h"
#include "Engine/Utils.h"
#include "Tool/RMath.h"

// MatrixIsIdent
//
// Returns whether a is the identity matrix, within tolerance epsilon
bool MatrixIsIdent(const Mat3& a, float epsilon)
{
    bool isIdent = true;
    
    const float* fData = a.asFloat();
    for (int i=0; isIdent && i<3; ++i)
    {
        for (int j=0; isIdent && j<3; ++j)
        {
            const float source = fData[i*3+j];
            
            if (i==j)
                isIdent = FloatEqual(source, 1.0f, epsilon);
            else
                isIdent = FloatEqual(source, 0.0f, epsilon);
        }
    }
    
    return isIdent;
}

// MatrixMakeIdentity
//
// Make a 4x4 identity matrix
void MatrixMakeIdentity(Mat4* dest)
{
    memset(dest, 0, sizeof *dest);
    dest->m_X[0] = dest->m_Y[1] = dest->m_Z[2] = dest->m_W[3] = 1.0;
}

// MatrixMakeRandy
//
// Make a 4x4 matrix of random entries centered about mid
void MatrixMakeRandy(Mat4* dest, float mid, float extent)
{
    float* f = dest->asFloat();
    for (int i=0; i<16; ++i)
        f[i] = rand()/(float)RAND_MAX * extent - mid;
}

void Mat4ApplyTranslation(Mat4* dest, float x, float y, float z)
{
    dest->m_W[0] = x;
    dest->m_W[1] = y;
    dest->m_W[2] = z;
}

void Mat4ApplyTranslation(Mat4* dest, const Vec3& a)
{
    dest->m_W[0] = a.m_X[0];
    dest->m_W[1] = a.m_X[1];
    dest->m_W[2] = a.m_X[2];
}

void Mat4GetTranslation(const Mat4& dest, float* x, float* y, float* z)
{
    *x = dest.m_W[0];
    *y = dest.m_W[1];
    *z = dest.m_W[2];
}

Vec3 Mat4GetTranslation3(const Mat4& dest)
{
    Vec3 ret;
    ret.m_X[0] = dest.m_W[0];
    ret.m_X[1] = dest.m_W[1];
    ret.m_X[2] = dest.m_W[2];
    return ret;
}

// [ 1 0 0 0 ]   [ 1 0 0 0 ]   [ lhs.x . column 0 rhs | lhs.x . column 1 rhs | lhs.x . column 2 rhs | lhs.x . column 3 rhs ]
// [ 0 1 0 0 ] x [ 0 1 0 0 ] = [ lhs.y . column 0 rhs | lhs.y . column 1 rhs | lhs.y . column 2 rhs | lhs.y . column 3 rhs ]
// [ 0 0 1 0 ]   [ 0 0 1 0 ]   [ lhs.z . column 0 rhs | lhs.z . column 1 rhs | lhs.z . column 2 rhs | lhs.z . column 3 rhs ]
// [ x y z 1 ]   [ 0 0 0 1 ]   [ lhs.w . column 0 rhs | lhs.w . column 1 rhs | lhs.w . column 2 rhs | lhs.w . column 3 rhs ]
void MatrixMultiply(Mat4* dest, const Mat4& lhs, const Mat4& rhs)
{
    for (int i=0; i<4; ++i)
    {
        dest->m_X[i] = MatrixDotTransposed(rhs, i, lhs.m_X);
        dest->m_Y[i] = MatrixDotTransposed(rhs, i, lhs.m_Y);
        dest->m_Z[i] = MatrixDotTransposed(rhs, i, lhs.m_Z);
        dest->m_W[i] = MatrixDotTransposed(rhs, i, lhs.m_W);
    }
}

// [ 1 0 0 ]   [ 1 0 0 ]   [ lhs.x . column 0 rhs | lhs.x . column 1 rhs | lhs.x . column 2 rhs | lhs.x ]
// [ 0 1 0 ] x [ 0 1 0 ] = [ lhs.y . column 0 rhs | lhs.y . column 1 rhs | lhs.y . column 2 rhs | lhs.y ]
// [ 0 0 1 ]   [ 0 0 1 ]   [ lhs.z . column 0 rhs | lhs.z . column 1 rhs | lhs.z . column 2 rhs | lhs.z ]
void MatrixMultiply(Mat3* dest, const Mat3& lhs, const Mat3& rhs)
{
    for (int i=0; i<3; ++i)
    {
        dest->m_X[i] = MatrixDotTransposed(rhs, i, lhs.m_X);
        dest->m_Y[i] = MatrixDotTransposed(rhs, i, lhs.m_Y);
        dest->m_Z[i] = MatrixDotTransposed(rhs, i, lhs.m_Z);
    }
}

void MatrixMultiplyTransposed(Mat4* dest, const Mat4& lhs, const Mat4& rhs)
{
    for (int i=0; i<4; ++i)
    {
        dest->m_X[i] = MatrixDotTransposed(rhs, i, lhs.m_X);
        dest->m_Y[i] = MatrixDotTransposed(rhs, i, lhs.m_Y);
        dest->m_Z[i] = MatrixDotTransposed(rhs, i, lhs.m_Z);
        dest->m_W[i] = MatrixDotTransposed(rhs, i, lhs.m_W);
    }
    
    MatrixTranspose(dest, *dest);
}

void MatrixSetRotAboutAxis(Mat4* dest, const float uvw[4], float theta)
{
    float u = uvw[0];
    float v = uvw[1];
    float w = uvw[2];
    
    const float c = cosf(theta);
    const float s = sinf(theta);
    
    dest->m_X[0] = u*u+(1-u*u)*c;
    dest->m_Y[0] = u*v*(1-c)+w*s;
    dest->m_Z[0] = u*w*(1-c)-v*s;
    dest->m_W[0] = 0;
    
    dest->m_X[1] = u*v*(1-c)-w*s;
    dest->m_Y[1] = v*v+(1-v*v)*c;
    dest->m_Z[1] = v*w*(1-c)+u*s;
    dest->m_W[1] = 0;
    
    dest->m_X[2] = u*w*(1-c)+v*s;
    dest->m_Y[2] = v*w*(1-c)-u*s;
    dest->m_Z[2] = w*w+(1-w*w)*c;
    dest->m_W[2] = 0;
    
    dest->m_X[3] = 0;
    dest->m_Y[3] = 0;
    dest->m_Z[3] = 0;
    dest->m_W[3] = 1;
}

float MatrixDotTransposed(const Mat4& a, int col, const float* v)
{
    float sum = 0.0f;
    
    sum += a.m_X[col] * v[0];
    sum += a.m_Y[col] * v[1];
    sum += a.m_Z[col] * v[2];
    sum += a.m_W[col] * v[3];
    
    return sum;
}

float MatrixDotTransposed(const Mat3& a, int col, const float* v)
{
    float sum = 0.0f;
    
    sum += a.m_X[col] * v[0];
    sum += a.m_Y[col] * v[1];
    sum += a.m_Z[col] * v[2];
    
    return sum;
}

void MatrixTransposeToGl(float temp[16], const Mat4& a)
{
    temp[0] = a.m_X[0];
    temp[1] = a.m_Y[0];
    temp[2] = a.m_Z[0];
    temp[3] = a.m_W[0];
    
    temp[4] = a.m_X[1];
    temp[5] = a.m_Y[1];
    temp[6] = a.m_Z[1];
    temp[7] = a.m_W[1];
    
    temp[8]  = a.m_X[2];
    temp[9]  = a.m_Y[2];
    temp[10] = a.m_Z[2];
    temp[11] = a.m_W[2];
    
    temp[12] = a.m_X[3];
    temp[13] = a.m_Y[3];
    temp[14] = a.m_Z[3];
    temp[15] = a.m_W[3];
}


void MatrixToGl(float temp[16], const Mat4& a)
{
    temp[0] = a.m_X[0];
    temp[1] = a.m_X[1];
    temp[2] = a.m_X[2];
    temp[3] = a.m_X[3];
    
    temp[4] = a.m_Y[0];
    temp[5] = a.m_Y[1];
    temp[6] = a.m_Y[2];
    temp[7] = a.m_Y[3];
    
    temp[8]  = a.m_Z[0];
    temp[9]  = a.m_Z[1];
    temp[10] = a.m_Z[2];
    temp[11] = a.m_Z[3];
    
    temp[12] = a.m_W[0];
    temp[13] = a.m_W[1];
    temp[14] = a.m_W[2];
    temp[15] = a.m_W[3];
}

void MatrixDump(const Mat4& m, const char* prefix)
{
    printf("%s  \n", prefix);
    printf("[ [ %f %f %f %f ],\n", m.m_X[0], m.m_X[1], m.m_X[2], m.m_X[3]);
    printf("  [ %f %f %f %f ],\n", m.m_Y[0], m.m_Y[1], m.m_Y[2], m.m_Y[3]);
    printf("  [ %f %f %f %f ],\n", m.m_Z[0], m.m_Z[1], m.m_Z[2], m.m_Z[3]);
    printf("  [ %f %f %f %f ] ]\n", m.m_W[0], m.m_W[1], m.m_W[2], m.m_W[3]);
}

void MatrixDump(const Mat3& m, const char* prefix)
{
    printf("%s  \n", prefix);
    printf("[ [ %f, %f, %f ],\n", m.m_X[0], m.m_X[1], m.m_X[2]);
    printf("  [ %f, %f, %f ],\n", m.m_Y[0], m.m_Y[1], m.m_Y[2]);
    printf("  [ %f, %f, %f ] ]\n", m.m_Z[0], m.m_Z[1], m.m_Z[2]);
}

void MatrixTransposeInsitu(Mat4* dest)
{
    Utils::swap(dest->m_X[1], dest->m_Y[0]);
    Utils::swap(dest->m_X[2], dest->m_Z[0]);
    Utils::swap(dest->m_X[3], dest->m_W[0]);
    
    Utils::swap(dest->m_Y[2], dest->m_Z[1]);
    Utils::swap(dest->m_Y[3], dest->m_W[1]);
    
    Utils::swap(dest->m_Z[3], dest->m_W[2]);
}


// | x0 x1 x2 |
// | y0 y1 y2 |
// | z0 z1 z2 |
//
void MatrixTransposeInsitu(Mat3* dest)
{
    Utils::swap(dest->m_X[1], dest->m_Y[0]);
    Utils::swap(dest->m_X[2], dest->m_Z[0]);
    Utils::swap(dest->m_Y[2], dest->m_Z[1]);
}

void MatrixTranspose(Mat4* dest, const Mat4& a)
{
    if (dest == &a)
    {
        MatrixTransposeInsitu(dest);
        return;
    }
    
    dest->m_X[0] = a.m_X[0];
    dest->m_X[1] = a.m_Y[0];
    dest->m_X[2] = a.m_Z[0];
    dest->m_X[3] = a.m_W[0];
    
    dest->m_Y[0] = a.m_X[1];
    dest->m_Y[1] = a.m_Y[1];
    dest->m_Y[2] = a.m_Z[1];
    dest->m_Y[3] = a.m_W[1];
    
    dest->m_Z[0] = a.m_X[2];
    dest->m_Z[1] = a.m_Y[2];
    dest->m_Z[2] = a.m_Z[2];
    dest->m_Z[3] = a.m_W[2];
    
    dest->m_W[0] = a.m_X[3];
    dest->m_W[1] = a.m_Y[3];
    dest->m_W[2] = a.m_Z[3];
    dest->m_W[3] = a.m_W[3];
}

void MatrixMultiplyVec(Vec4* dest, Vec4 v, const Mat4& a)
{
    dest->m_X[0] = v.m_X[0]*a.m_X[0] + v.m_X[1]*a.m_X[1] + v.m_X[2]*a.m_X[2] + v.m_X[3]*a.m_X[3];
    dest->m_X[1] = v.m_X[0]*a.m_Y[0] + v.m_X[1]*a.m_Y[1] + v.m_X[2]*a.m_Y[2] + v.m_X[3]*a.m_Y[3];
    dest->m_X[2] = v.m_X[0]*a.m_Z[0] + v.m_X[1]*a.m_Z[1] + v.m_X[2]*a.m_Z[2] + v.m_X[3]*a.m_Z[3];
    dest->m_X[3] = v.m_X[0]*a.m_W[0] + v.m_X[1]*a.m_W[1] + v.m_X[2]*a.m_W[2] + v.m_X[3]*a.m_W[3];
}

// right multiply v by a, storing in dest
void MatrixMultiplyVec(float dest[4], const float v[4], const Mat4& a)
{
    dest[0] = v[0]*a.m_X[0] + v[1]*a.m_X[1] + v[2]*a.m_X[2] + v[3]*a.m_X[3];
    dest[1] = v[0]*a.m_Y[0] + v[1]*a.m_Y[1] + v[2]*a.m_Y[2] + v[3]*a.m_Y[3];
    dest[2] = v[0]*a.m_Z[0] + v[1]*a.m_Z[1] + v[2]*a.m_Z[2] + v[3]*a.m_Z[3];
    dest[3] = v[0]*a.m_W[0] + v[1]*a.m_W[1] + v[2]*a.m_W[2] + v[3]*a.m_W[3];
}

// right multiply v by a, storing in dest
void MatrixMultiplyVecW0(float dest[3], const float v[3], const Mat4& a)
{
    dest[0] = v[0]*a.m_X[0] + v[1]*a.m_X[1] + v[2]*a.m_X[2];
    dest[1] = v[0]*a.m_Y[0] + v[1]*a.m_Y[1] + v[2]*a.m_Y[2];
    dest[2] = v[0]*a.m_Z[0] + v[1]*a.m_Z[1] + v[2]*a.m_Z[2];
}

void MatrixMultiplyVecW1(Vec3* dest, Vec3 v, const Mat4& a)
{
    dest->m_X[0] = v.m_X[0]*a.m_X[0] + v.m_X[1]*a.m_X[1] + v.m_X[2]*a.m_X[2] + a.m_X[3];
    dest->m_X[1] = v.m_X[0]*a.m_Y[0] + v.m_X[1]*a.m_Y[1] + v.m_X[2]*a.m_Y[2] + a.m_Y[3];
    dest->m_X[2] = v.m_X[0]*a.m_Z[0] + v.m_X[1]*a.m_Z[1] + v.m_X[2]*a.m_Z[2] + a.m_Z[3];
}

void MatrixMultiplyVecW1(Vec4* dest, Vec3 v, const Mat4& a)
{
    dest->m_X[0] = a.m_X[0]*v.m_X[0] + a.m_X[1]*v.m_X[1] + a.m_X[2]*v.m_X[2] + a.m_X[3];
    dest->m_X[1] = a.m_Y[0]*v.m_X[0] + a.m_Y[1]*v.m_X[1] + a.m_Y[2]*v.m_X[2] + a.m_Y[3];
    dest->m_X[2] = a.m_Z[0]*v.m_X[0] + a.m_Z[1]*v.m_X[1] + a.m_Z[2]*v.m_X[2] + a.m_Z[3];
    dest->m_X[3] = a.m_W[0]*v.m_X[0] + a.m_W[1]*v.m_X[1] + a.m_W[2]*v.m_X[2] + a.m_W[3];
}

// right multiply v by a, storing in dest
void MatrixMultiplyVec(float dest[3], const float v[3], const Mat3& a)
{
    dest[0] = v[0]*a.m_X[0] + v[1]*a.m_X[1] + v[2]*a.m_X[2];
    dest[1] = v[0]*a.m_Y[0] + v[1]*a.m_Y[1] + v[2]*a.m_Y[2];
    dest[2] = v[0]*a.m_Z[0] + v[1]*a.m_Z[1] + v[2]*a.m_Z[2];
}


// MatrixCopy
void MatrixCopy(Mat4* dest, const Mat4& a)
{
    memcpy(dest, a.asFloat(), 64);
}

// vector3 dump
void Vector3Dump(float v[3], const char* prefix)
{
    printf("%s[%f %f %f]\n", prefix, v[0], v[1], v[2]);
}

void VectorDump(const Vec3* a, const char* prefix)
{
    printf("%s[%f %f %f]\n", prefix, a->m_X[0], a->m_X[1], a->m_X[2]);
}

void VectorDump(const Vec3& a, const char* prefix)
{
    printf("%s[%f %f %f]\n", prefix, a.m_X[0], a.m_X[1], a.m_X[2]);
}

void VectorDump(const Vec4* a, const char* prefix)
{
    printf("%s[%f %f %f %f]\n", prefix, a->m_X[0], a->m_X[1], a->m_X[2], a->m_X[3]);
}

void VectorDump(const Vec4& a, const char* prefix)
{
    printf("%s[%f %f %f %f]\n", prefix, a.m_X[0], a.m_X[1], a.m_X[2], a.m_X[3]);
}


// MatrixDet2x2
//
// Returns determinant of 2x2 matrix.
float MatrixDet2x2(float a, float b, float c, float d)
{
    return a*d-b*c;
}

// MatrixDet3x3
//
// Returns determinant of 3x3 matrix.
float MatrixDet3x3(float a, float b, float c,
                   float s, float t, float u,
                   float x, float y, float z)
{
    const float am = a * MatrixDet2x2(t, u, y, z); // skip 0
    const float bm = b * MatrixDet2x2(s, u, x, z); // skip 1
    const float cm = c * MatrixDet2x2(s, t, x, y); // skip 2

    return am-bm+cm;
}

// MatrixDet4x4
//
// Returns determinant of 4x4 matrix.
float MatrixDet4x4(float a, float b, float c, float d,
                   float i, float j, float k, float l,
                   float s, float t, float u, float v,
                   float x, float y, float z, float w)
{
    const float am = a * MatrixDet3x3(j,k,l, t,u,v, y,z,w);
    const float bm = b * MatrixDet3x3(i,k,l, s,u,v, x,z,w);
    
    const float cm = c * MatrixDet3x3(i,j,l, s,t,v, x,y,w);
    const float dm = d * MatrixDet3x3(i,j,k, s,t,u, x,y,z);
    return am - bm + cm - dm;
}

// MatrixCofactor
//
// Returns cofactor of a 4x4 matrix made from slicing off row/col
float MatrixCofactor(const Mat4& a, int row, int col)
{
    float rowVals[3][3];
    const float* aFloat = a.asFloat();
    
    int rItr = 0;
    for (int i=0; i<4; ++i)
    {
        if (i == row)
            continue;
        
        const int row = rItr++;
        
        int cItr = 0;
        for (int k=0; k<4; ++k)
        {
            if (k == col)
                continue;
            
            rowVals[row][cItr++] = aFloat[i * 4+k];
        }
    }
    
    const float val = MatrixDet3x3(rowVals[0][0], rowVals[0][1], rowVals[0][2],
                                   rowVals[1][0], rowVals[1][1], rowVals[1][2],
                                   rowVals[2][0], rowVals[2][1], rowVals[2][2]);
    const float ret = val * ((row+col)&1 ? -1.0f : 1.0f);
    return ret;
}

// MatrixCofactor
//
// Returns cofactor of a 3x3 matrix made from slicing off row/col
float MatrixCofactor(const Mat3& a, int row, int col)
{
    float rowVals[2][2];
    const float* aFloat = a.asFloat();
    
    int rItr = 0;
    for (int i=0; i<3; ++i)
    {
        if (i == row)
            continue;
        
        const int row = rItr++;
        
        int cItr = 0;
        for (int k=0; k<3; ++k)
        {
            if (k == col)
                continue;
            
            rowVals[row][cItr++] = aFloat[i * 3+k];
        }
    }
    
    const float val = MatrixDet2x2(rowVals[0][0], rowVals[0][1],
                                   rowVals[1][0], rowVals[1][1]);
    const float ret = val * ((row+col)&1 ? -1.0f : 1.0f);
    return ret;
}

// MatrixAdjoint4x4
//
// Return adjoint of 4x4 matrix
void MatrixAdjoint4x4(Mat4* dest, const Mat4& a)
{
    for (int i=0; i<4; ++i)
    {
        dest->m_X[i] = MatrixCofactor(a, 0, i);
        dest->m_Y[i] = MatrixCofactor(a, 1, i);
        dest->m_Z[i] = MatrixCofactor(a, 2, i);
        dest->m_W[i] = MatrixCofactor(a, 3, i);
    }
    
    MatrixTransposeInsitu(dest);
}

// MatrixAdjoint3x3
//
// Return adjoint of 3x3 matrix
void MatrixAdjoint3x3(Mat3* dest, const Mat3& a)
{
    for (int i=0; i<3; ++i)
    {
        dest->m_X[i] = MatrixCofactor(a, 0, i);
        dest->m_Y[i] = MatrixCofactor(a, 1, i);
        dest->m_Z[i] = MatrixCofactor(a, 2, i);
    }
    
    MatrixTransposeInsitu(dest);
}

void MatrixScaleInsitu(Mat4* dest, float value)
{
    for (int i=0; i<4; ++i)
    {
        dest->m_X[i] = value * dest->m_X[i];
        dest->m_Y[i] = value * dest->m_Y[i];
        dest->m_Z[i] = value * dest->m_Z[i];
        dest->m_W[i] = value * dest->m_W[i];
    }
}


void MatrixScaleInsitu(Mat3* dest, float value)
{
    for (int i=0; i<3; ++i)
    {
        dest->m_X[i] = value * dest->m_X[i];
        dest->m_Y[i] = value * dest->m_Y[i];
        dest->m_Z[i] = value * dest->m_Z[i];
    }
}

void MatrixInvert(Mat4* dest, const Mat4& a)
{
    const float* vals = a.asFloat();
    
    const float det = MatrixDet4x4(vals[0], vals[1], vals[2], vals[3],
                                   vals[4], vals[5], vals[6], vals[7],
                                   vals[8], vals[9], vals[10], vals[11],
                                   vals[12], vals[13], vals[14], vals[15]);
    if (det == 0.0f)
        return;
    
    MatrixAdjoint4x4(dest, a);
    MatrixScaleInsitu(dest, 1.0f / det);
}


// Mat3
void MatrixMakeIdentity(Mat3* dest)
{
    memset(dest, 0, sizeof *dest);
    dest->m_X[0] = dest->m_Y[1] = dest->m_Z[2] = 1.0;
}

void MatrixMakeRandy(Mat3* dest, float mid, float extent)
{
    float* f = dest->asFloat();
    for (int i=0; i<9; ++i)
        f[i] = rand()/(float)RAND_MAX * extent - mid;
}

void MatrixMakeZero(Mat3* dest)
{
    memset(dest, 0, sizeof *dest);
}

void MatrixMakeZero(Mat4* dest)
{
    memset(dest, 0, sizeof *dest);
}

// Given a real symmetric 3x3 matrix A, compute the eigenvalues
// 
// = A(1,2)^2 + A(1,3)^2 + A(2,3)^2
// (p1 == 0) 
// % A is diagonal.
// eig1 = A(1,1)
// eig2 = A(2,2)
// eig3 = A(3,3)
// e
// q = trace(A)/3
// p2 = (A(1,1) - q)^2 + (A(2,2) - q)^2 + (A(3,3) - q)^2 + 2 * p1
// p = sqrt(p2 / 6)
// B = (1 / p) * (A - q * I)       % I is the identity matrix
// r = det(B) / 2
// 
// % In exact arithmetic for a symmetric matrix  -1 <= r <= 1
// % but computation error can leave it slightly outside this range.
// if (r <= -1) 
//    phi = pi / 3
// elseif (r >= 1)
//    phi = 0
// else
//    phi = acos(r) / 3
// end
// 
// % the eigenvalues satisfy eig3 <= eig2 <= eig1
// eig1 = q + 2 * p * cos(phi)
// eig3 = q + 2 * p * cos(phi + (2*pi/3))
// eig2 = 3 * q - eig1 - eig3     % since trace(A) = eig1 + eig2 + eig3
//

float Mat3Trace(const Mat3& a)
{
    return a.m_X[0] + a.m_Y[1] + a.m_Z[2];
}

float Mat3Trace(const Mat3* a)
{
    return a->m_X[0] + a->m_Y[1] + a->m_Z[2];
}

void Mat3MakeScale(Mat3* dest, float s)
{
    MatrixMakeZero(dest);
    
    dest->m_X[0] = s;
    dest->m_Y[1] = s;
    dest->m_Z[2] = s;
}

void Mat3MulScalar(Mat3* dest, const Mat3& a, float s)
{
    for (int i=0; i<3; ++i)
    {
        dest->m_X[i] = a.m_X[i] * s;
        dest->m_Y[i] = a.m_Y[i] * s;
        dest->m_Z[i] = a.m_Z[i] * s;
    }
}

void Mat3Sub(Mat3* dest, const Mat3&a, const Mat3& b)
{
    for (int i=0; i<3; ++i)
    {
        dest->m_X[i] = a.m_X[i] - b.m_X[i];
        dest->m_Y[i] = a.m_Y[i] - b.m_Y[i];
        dest->m_Z[i] = a.m_Z[i] - b.m_Z[i];
    }
}

float MatrixDeterminant(const Mat3& m)
{
    return MatrixDet3x3(m.m_X[0], m.m_X[1], m.m_X[2],
                        m.m_Y[0], m.m_Y[1], m.m_Y[2],
                        m.m_Z[0], m.m_Z[1], m.m_Z[2]);
}

void MatrixInvert(Mat3* dest, const Mat3& a)
{
    const float* vals = a.asFloat();
    
    const float det = MatrixDet3x3(vals[0], vals[1], vals[2],
                                   vals[3], vals[4], vals[5],
                                   vals[6], vals[7], vals[8]);
    if (det == 0.0f)
        return;
    
    MatrixAdjoint3x3(dest, a);
    MatrixScaleInsitu(dest, 1.0f / det);
}

void Mat3Diagonalize(Mat3* s, float lambda3[3], Mat3* sInv, const Mat3& a)
{
    // determine eigenvalues
    const float t0 = Mat3Trace(a);
    const float q = t0 / 3.0f;
    const float p1 = (a.m_X[1]*a.m_X[1]) + (a.m_X[2]*a.m_X[2]) + (a.m_Y[2]*a.m_Y[2]);
    const float p2 = powf(a.m_X[0] - q, 2.0f) + powf(a.m_Y[1] - q, 2.0f) + powf(a.m_Z[2] - q, 2.0f) + 2.0f*p1;
    const float p = sqrtf(p2 / 6.0f);
    
    const float t1 = MatrixDeterminant(a);
    
    Mat3 qi;
    Mat3MakeScale(&qi, q);
    
    Mat3 b0;
    Mat3Sub(&b0, a, qi);
    
    Mat3 b;
    Mat3MulScalar(&b, b0, 1.0f/p);
    
    const float r = MatrixDeterminant(b) * 0.5f;
    
    float phi;
    if (r <= -1.0f)
        phi = M_PI / 3.0f;
    else if (r >= 1.0f)
        phi = 0.0f;
    else
        phi = acosf(r) / 3.0f;
    
    lambda3[0] = q + 2.0f*p*cosf(phi);
    lambda3[2] = q + 2.0f*p*cosf(phi + (2.0f*M_PI/3.0f));
    lambda3[1] = 3.0f * q - lambda3[0] - lambda3[2];
    
    // generate eigenvalues from lambda
    RMat* rma = (RMat*) RMAT_A(3, 3);
    for (int i=0; i<9; ++i)
        rma->m_Data[i] = a.asFloat()[i];
    RVector* dest = (RVector*) alloca(RVectorSize(3));
    
    RMatInvertIterate(dest, rma, lambda3[0]);
    s->m_X[0] = dest->m_X[0];
    s->m_Y[0] = dest->m_X[1];
    s->m_Z[0] = dest->m_X[2];
    
    RMatInvertIterate(dest, rma, lambda3[1]);
    s->m_X[1] = dest->m_X[0];
    s->m_Y[1] = dest->m_X[1];
    s->m_Z[1] = dest->m_X[2];
    
    RMatInvertIterate(dest, rma, lambda3[2]);
    s->m_X[2] = dest->m_X[0];
    s->m_Y[2] = dest->m_X[1];
    s->m_Z[2] = dest->m_X[2];
    
    MatrixInvert(sInv, *s);
}

void MatrixMakeDiagonal(Mat3* dest, float diags[3])
{
    MatrixMakeZero(dest);
    
    dest->m_X[0] = diags[0];
    dest->m_Y[1] = diags[1];
    dest->m_Z[2] = diags[2];
}

void MatrixCalculateDelta(Mat4* dest, const Mat4& current, const Mat4& prev)
{
    Mat4 t0;
    MatrixInvert(&t0, prev);
    
    MatrixMultiply(dest, t0, current);
}

float VectorDistanceSquared(const Vec4& a, const Vec4& b)
{
    Vec4 v = a - b;
    return VectorDot(v, v);
}

float VectorDistanceSquared(const Vec3& a, const Vec3& b)
{
    Vec3 v = a - b;
    return VectorDot(v, v);
}

Vec4 VectorSub(const Vec4& a, const Vec4& b)
{
    Vec4 ret;
    ret.m_X[0] = a.m_X[0]-b.m_X[0];
    ret.m_X[1] = a.m_X[1]-b.m_X[1];
    ret.m_X[2] = a.m_X[2]-b.m_X[2];
    ret.m_X[3] = a.m_X[3]-b.m_X[3];
    return ret;
}

Vec3 VectorSub(const Vec3& a, const Vec3& b)
{
    Vec3 ret;
    ret.m_X[0] = a.m_X[0]-b.m_X[0];
    ret.m_X[1] = a.m_X[1]-b.m_X[1];
    ret.m_X[2] = a.m_X[2]-b.m_X[2];
    return ret;
}


float VectorDot(const Vec4& a, const Vec4& b)
{
    return a.m_X[0]*b.m_X[0]+a.m_X[1]*b.m_X[1]+a.m_X[2]*b.m_X[2]+a.m_X[3]*b.m_X[3];
}

float VectorDot(const Vec3& a, const Vec3& b)
{
    float ret = 0.0f;
    ret += a.m_X[0]*b.m_X[0];
    ret += a.m_X[1]*b.m_X[1];
    ret += a.m_X[2]*b.m_X[2];
    return ret;
}

void VectorSplat(Vec3* dest, float value)
{
    dest->m_X[0] = dest->m_X[1] = dest->m_X[2] = value;
}

void VectorSet(Vec3* dest, float x, float y, float z)
{
    dest->m_X[0] = x;
    dest->m_X[1] = y;
    dest->m_X[2] = z;
}

void VectorNormalize(Vec3* a)
{
    a->Normalize();
}

void VectorNormalize(Vec3* dest, const Vec3& a)
{
    const float lengthSquared = a.LengthSquared();
    if (lengthSquared > 1e-3f)
    {
        const float length = sqrtf(lengthSquared);
        dest->m_X[0] = a.m_X[0] / length;
        dest->m_X[1] = a.m_X[1] / length;
        dest->m_X[2] = a.m_X[2] / length;
    }
}

float VectorLengthSquared(const Vec3& a)
{
    float val = 0.0f;
    val += a.m_X[0]*a.m_X[0];
    val += a.m_X[1]*a.m_X[1];
    val += a.m_X[2]*a.m_X[2];
    return val;
}

void VectorCopy(Vec3* a, const Vec3& b)
{
    a->m_X[0] = b.m_X[0];
    a->m_X[1] = b.m_X[1];
    a->m_X[2] = b.m_X[2];
}

Vec3 VectorSign(const Vec3& temp)
{
    Vec3 mask;
    mask.m_X[0] = temp.m_X[0] < 0 ? -1.0f : (temp.m_X[0] == 0.0f ? 0.0f : 1.0f);
    mask.m_X[1] = temp.m_X[1] < 0 ? -1.0f : (temp.m_X[1] == 0.0f ? 0.0f : 1.0f);
    mask.m_X[2] = temp.m_X[2] < 0 ? -1.0f : (temp.m_X[2] == 0.0f ? 0.0f : 1.0f);
    return mask;
}

Vec3 VectorMul(const Vec3& a, const Vec3& b)
{
    Vec3 ret;
    ret.m_X[0] = a.m_X[0] * b.m_X[0];
    ret.m_X[1] = a.m_X[1] * b.m_X[1];
    ret.m_X[2] = a.m_X[2] * b.m_X[2];
    return ret;
}

int VectorNegativeCount(const Vec3& a)
{
    int ret = 0;
    ret += a.m_X[0] < 0 ? 1 : 0;
    ret += a.m_X[1] < 0 ? 1 : 0;
    ret += a.m_X[2] < 0 ? 1 : 0;
    return ret;
}

int VectorEqual(const Vec3& a, const Vec3& b)
{
    int ret = 0;
    ret += a.m_X[0] == b.m_X[0];
    ret += a.m_X[1] == b.m_X[1];
    ret += a.m_X[2] == b.m_X[2];
    return ret;
}

bool FloatApproxEqual(float a, float b, float eps)
{
    float v = fabsf(a-b);
    return v<eps;
}

int VectorApproxEqual(const Vec3& a, const Vec3& b, float eps)
{
    int ret = 0;
    if (FloatApproxEqual(a.m_X[0], b.m_X[0], eps))
        ret++;
    if (FloatApproxEqual(a.m_X[1], b.m_X[1], eps))
        ret++;
    if (FloatApproxEqual(a.m_X[2], b.m_X[2], eps))
        ret++;
    return ret;
}
