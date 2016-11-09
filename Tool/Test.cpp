// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-
#include "Engine/Matrix.h"
#include "Engine/Utils.h"
#include <stdio.h>
#include <stdlib.h>

#define _TEST(func, funcString) if (!func()) { Printf(funcString " failed!\n"); exit(1); }
#define TEST(func) _TEST(func, #func)

static bool Mat3MakeZero();
static bool Mat3DiagonalizeTest();
static bool MatrixMultiplyVecTest();

void MatrixTest()
{
    TEST(Mat3MakeZero);
    TEST(MatrixMultiplyVecTest);
    TEST(Mat3DiagonalizeTest);
}

static bool MatrixMultiplyVecTest()
{
  Vec3 v;
  v[0] = rand()/(float)RAND_MAX * 1024 - 512.0f;
  v[1] = rand()/(float)RAND_MAX * 1024 - 512.0f;
  v[2] = rand()/(float)RAND_MAX * 1024 - 512.0f;
  
  Mat3 r;
  MatrixMakeRandy(&r, 512.0f, 1024.0f);
  
  Vec3 dest;
  MatrixMultiplyVec(&dest, r, v);
  
  bool ret = true;
  
  const float* f = r.asFloat();
  if (dest[0] != f[0]*v[0] + f[1]*v[1] + f[2]*v[2])
    ret = false;
  if (dest[1] != f[3]*v[0] + f[4]*v[1] + f[5]*v[2])
    ret = false;
  if (dest[2] != f[6]*v[0] + f[7]*v[1] + f[8]*v[2])
    ret = false;
  
  if (!ret)
  {
    VectorDump(v, "v: ");
    MatrixDump(r, "r");
    VectorDump(dest, "dest: ");
  }
    
  return true;
}

static bool Mat3MakeZero()
{
    Mat3 mat3;
    MatrixMakeZero(&mat3);
    
    const float* f = mat3.asFloat();
    for (int i=0; i<9; ++i)
    {
        if (f[i] != 0.0f)
            return false;
    }
    return true;
}

static bool Mat3DiagonalizeTest()
{
    Mat3 s;
    Mat3 sInv;
    Mat3 a;
    
    MatrixMakeZero(&s);
    MatrixMakeZero(&sInv);
    MatrixMakeZero(&a);
    
    a.m_X[0] = 3;
    a.m_X[1] = 1;
    a.m_X[2] = -2;
    
    a.m_Y[0] = 1;
    a.m_Y[1] = -1;
    a.m_Y[2] = 2;
    
    a.m_Z[0] = -2;
    a.m_Z[1] = 2;
    a.m_Z[2] = -3;
    
    float lambdas[3];
    Mat3Diagonalize(&s, lambdas, &sInv, a);

    Mat3 d;
    MatrixMakeDiagonal(&d, lambdas);

    Mat3 sd;
    MatrixMultiply(&sd, s, d);

    Mat3 sdsInv;
    MatrixMultiply(&sdsInv, sd, sInv);

    Mat3 sdsInvInv;
    MatrixInvert(&sdsInvInv, sdsInv);

    Mat3 ident;    
    MatrixMultiply(&ident, sdsInvInv, a);
    
    if (MatrixIsIdent(ident, 1e-3f))
        return true;
    
    MatrixDump(a, "a");
    MatrixDump(s, "s");
    MatrixDump(d, "d");
    MatrixDump(sInv, "sInv");
    
    return false;
}
