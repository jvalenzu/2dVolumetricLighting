// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#pragma warning(push)
#pragma warning (disable:4200)


struct RVector;

//                ___
//               /\_ \
//  _____     ___\//\ \    __  __
// /\ '__`\  / __`\\ \ \  /\ \/\ \
// \ \ \L\ \/\ \L\ \\_\ \_\ \ \_\ \
//  \ \ ,__/\ \____//\____\\/`____ \
//   \ \ \/  \/___/ \/____/ `/___/> \
//    \ \_\                    /\___/
//     \/_/                    \/__/

struct Poly
{
    int32_t m_Order;
    float m_X[]; // in reverse order.  m_X[0] = x^0
};

size_t  PolySize(int order);
size_t  PolySize(const Poly* a);

Poly*   PolyAlloc(void* data, int order);
Poly*   PolyHeapAlloc(int order);
int     PolyRoots(float* realRoots, const Poly* poly);
void    PolyMultiply(Poly* dest, const Poly* a, const Poly* b);

void    PolyDerivative(Poly* dest, const Poly* a);
float  PolyEvaluate(const Poly* a, float x);

void    PolyCopy(Poly* des, const Poly* a);

void    PolyDump(const char* label, const Poly* a);

void    PolyPlot(const Poly* poly, float (*f)(float x), const float* coeffs, int dim, float a, float b);

// find the abscissa of the extrema of an error function (P(x) - f(x)) nbetween a and b
float  PolyErrorGetExtremaAbscissa(const Poly* poly, float (*f)(float x), float a, float b, float x0);

float  PolyErrorGetRoot(const Poly* poly, float (*f)(float x), float a, float b, float x0);
float  PolyErrorEvaluate(const Poly* poly, float (*f)(float x), float x);
void    PolyErrorDerivativeAt(Poly* dest, const Poly* a, float x);
void    PolyErrorPrimeDerivativeAt(Poly* dest, const Poly* a, float x);



//                       __
//                      /\ \__
//  __  __     __    ___\ \ ,_\   ___   _ __
// /\ \/\ \  /'__`\ /'___\ \ \/  / __`\/\`'__\
// \ \ \_/ |/\  __//\ \__/\ \ \_/\ \L\ \ \ \/
//  \ \___/ \ \____\ \____\\ \__\ \____/\ \_\
//   \/__/   \/____/\/____/ \/__/\/___/  \/_/
//

struct RVector
{
    int32_t m_Dimension;
    float m_X[];
};

size_t RVectorSize(int dimension);
size_t RVectorSize(const RVector* a);
RVector* RVectorAlloc(void* data, int dimensions);
void RVectorDump(const char* label, const RVector* a);
void RVectorScale(RVector* dest, float scale);
void RVectorScale(RVector* dest, const RVector* a, float scale);
void RVectorPermute(RVector* dest, const RVector* b, int* p, size_t elementsP);

float RVectorDot(const RVector* a, const RVector* b);
void RVectorNormalize(RVector* dest, const RVector* a);

#define RVEC_A(a) RVectorAlloc(alloca(RVectorSize(a)), (a)->m_Dimension)

//                     __
//                    /\ \__         __
//   ___ ___      __  \ \ ,_\  _ __ /\_\   __  _
// /' __` __`\  /'__`\ \ \ \/ /\`'__\/\ \ /\ \/'\
// /\ \/\ \/\ \/\ \L\.\_\ \ \_\ \ \/ \ \ \\/>  </
// \ \_\ \_\ \_\ \__/.\_\\ \__\\ \_\  \ \_\/\_/\_\
//  \/_/\/_/\/_/\/__/\/_/ \/__/ \/_/   \/_/\//\/_/
//

struct RMat
{
    int32_t m_Rows;
    int32_t m_Columns;
    float m_Data[];
};

size_t RMatSize(int rows, int columns);
size_t RMatSize(const RMat* rmat);

RMat* RMatCopy(void* data, const RMat* a);

RMat* RMatAlloc(void* data, int rows, int columns);

#define RMAT_A(r, c) ((RMat*) RMatAlloc(alloca(RMatSize((r), (c))), (r), (c)))

inline RMat* RMatAllocMatchSize(void* data, const RMat* a)
{
    return RMatAlloc(data, a->m_Rows, a->m_Columns);
}

void RMatMakeIdentity(RMat* dest);

inline void RMatSetData(RMat* dest, int row, int col, float value)
{
    const int prevRows = dest->m_Rows;
    const int prevCols = dest->m_Columns;
    
    dest->m_Data[col+row*dest->m_Columns] = value;
    
    assert(dest->m_Rows == prevRows);
    assert(dest->m_Columns == prevCols);
}

inline float RMatGetData(const RMat* dest, int row, int col)
{
    return dest->m_Data[col+row*dest->m_Columns];
}

inline const float* RMatGetRow(const RMat* dest, int row)
{
    return &dest->m_Data[row*dest->m_Columns];
}

inline float* RMatGetRow(RMat* dest, int row)
{
    return &dest->m_Data[row*dest->m_Columns];
}

inline size_t RMatRowSize(const RMat* a)
{
    return a->m_Columns * sizeof(float);
}

void RMatTranspose(RMat* dest, const RMat* a);
void RMatMulMat(RMat* dest, const RMat* a, const RMat* b);
void RMatMulVec(RVector* dest, const RMat* a, const RVector* b);
void RMatDump(const char* label, const RMat* a);

void RMatDecomposeLdu(RMat* l, RMat* d, RMat* u, int* p, const RMat* a);
void RMatSolve(RVector* dest, const RMat* a, const RVector* b);
void RMatLeastSquare(RVector* dest, const RMat* a, const RVector* b);
void RMatInvertIterate(RVector* dest, const RMat* a, float lambda);

// implement Remez algorithm
void Remez(RVector* dest, float (*f)(float x), float a, float b);

// Chebyshev nodes
void ChebyshevNodes(RVector* dest, float a, float b);

// Lerp
float Lerp(float a, float b, float t);

//
struct Mat4;
struct Mat3;
size_t RMatSize(const Mat4* mat4);
size_t RMatSize(const Mat3* mat3);

void RMatCopy(RMat* dest, const Mat4* mat4);
void RMatCopy(RMat* dest, const Mat3* mat3);

#pragma warning(pop)
