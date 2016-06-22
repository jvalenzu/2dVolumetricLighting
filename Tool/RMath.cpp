// todo

#include "Tool/RMath.h"
#include "Engine/Matrix.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <new>

#define kSmallEpsilon 1e-6
#define kVerySmallEpsilon 1e-9

static int numUniqueDouble(double* controlPoints, int dim);

static inline double minf(double a, double b)
{
    return a<b ? a : b;
}

// doubleDist
static inline double doubleDist(double a, double b)
{
    if (a<b)
        return b-a;
    return a-b;
}

// Lerp
double Lerp(double a, double b, double t)
{
    return a + (b-a) * t;
}

// PolySize
size_t PolySize(int order)
{
    return sizeof(int32_t) + sizeof(double)*order;
}

// PolySize
size_t PolySize(const Poly* a)
{
    return PolySize(a->m_Order);
}

// PolyHeapAlloc
Poly* PolyHeapAlloc(int order)
{
    char* data = (char*) malloc(PolySize(order));
    return PolyAlloc(data, order);
}

// PolyAlloc
Poly* PolyAlloc(void* data, int order)
{
    Poly* ret = (Poly*) data;
    memset(ret, 0, sizeof *ret);
    ret->m_Order = order;
    return ret;
}

// PolyCopyAlloc
static Poly* PolyCopyAlloc(void* data, const Poly* a)
{
    Poly* ret = PolyAlloc(data, a->m_Order);
    PolyCopy(ret, a);
    return ret;
}

// PolyDerivative
void PolyDerivative(Poly* dest, const Poly* a)
{
    const size_t order = a->m_Order;
    int i=1;
    for (i=1; i<order; ++i)
        dest->m_X[i-1] = a->m_X[i]*i;
    while (i<=dest->m_Order)
        dest->m_X[i++-1] = 0.0;
}

// PolyDivide
int PolyDivide(Poly* dest, const Poly* poly, const Poly* divider)
{
    if (divider->m_Order > poly->m_Order)
        return 0;
    if (divider->m_Order != 2)
        return 0; // jiv fixme
    
    double* r = (double*) alloca(10*sizeof(double)); // jiv fixme
    double* r2 = (double*) alloca(10*sizeof(double)); // jiv fixme
    double* rBase = r;
    
    Poly* q = PolyCopyAlloc(alloca(PolySize(poly->m_Order)), poly);
    
    int iteration = 0;
    
    // iterate coefficients
    while (q->m_Order)
    {
        // our current power
        const double q0 = q->m_X[q->m_Order-1];
        
        // remove current power
        q->m_Order--;
        
        // push multiplier for this coefficient
        const double p0 = divider->m_X[divider->m_Order-1];
        *r++ = q0 / p0;
        
        const double p1 = divider->m_X[divider->m_Order-2];
        const double pp1 = (q0 / p0) * p1;
        
        if (q->m_Order == 1)
        {
            const int numR = r - rBase;
            for (int j=0; j<numR; ++j)
                dest->m_X[j] = rBase[numR-j-1];
            dest->m_Order = numR;
            
            return 1;
        }
        
        q->m_X[q->m_Order-1] -= pp1;
    }
    
    return 0;
}

// PolyDump
void PolyDump(const char* label, const Poly* a)
{
    const size_t order = a->m_Order;
    printf("[%s(%ld) ", label, order);
    int numWritten = 0;
    for (int i=0; i<order; ++i)
    {
        const int power = order-i-1;
        const double coeff = a->m_X[order-i-1];
        if (coeff == 0.0)
            continue;
        const char* op = (numWritten==0) ? "" : "+";
        if (coeff < 0.0)
            op = "-";
        const char* opSpace = (numWritten==0) ? "" : " "; // drop space between operator if initial coefficient
        const double aCoeff = fabs(coeff);
        
        const char* end = numWritten==order-1 ? "" : " ";
        
        char vBuf[32] = { 0 };
        if (power)
            snprintf(vBuf, sizeof vBuf, "x^%d", power);
        
        if (doubleDist(coeff, 0.0) < kSmallEpsilon)
            continue;
        
        numWritten++;
        
        if (coeff == ceilf(coeff))
            printf("%s%s%.0lf%s%s", op, opSpace, aCoeff, vBuf, end);
        else
            printf("%s%s%lf%s%s", op, opSpace, aCoeff, vBuf, end);
    }
    printf("]\n");
}

// jiv fixme: horner's method
double PolyEvaluate(const Poly* a, double x)
{
    double temp = 1.0;
    double ret = 0.0;
    const size_t order = a->m_Order;
    for (int i=0; i<order; ++i)
    {
        ret += temp * a->m_X[i];
        temp *= x;
    }
    return ret;
}

// PolyPlot
void PolyPlot(int iteration, const Poly* poly, double (*f)(double x), const double* extremaAbscissa, int dim0, const double* roots, int dim1, double a, double b)
{
    char fLabel[32];
    snprintf(fLabel, sizeof fLabel, "foo%d.dat", iteration);
    
    FILE* fh = fopen(fLabel, "wt");
    if (fh)
    {
        const int numSteps = 1000;
        const double step = (b - a) / double(numSteps);
        double x = a;
        for (int i=0; i<numSteps; ++i)
        {
            const double v0 = PolyEvaluate(poly, x);
            const double v1 = f(x);
            fprintf(fh, "%lf\t%lf\t%lf\t%lf\n", x, v0, v1, v0-v1);
            x += step;
        }
        
        fclose(fh);
    }
    
    char nLabel[32];
    snprintf(nLabel, sizeof nLabel, "extrema%d.dat", iteration);
    fh = fopen(nLabel, "wt");
    if (fh)
    {
        for (int i=0; i<dim0; ++i)
        {
            const double x = extremaAbscissa[i];
            const double v = PolyErrorEvaluate(poly, f, x);
            fprintf(fh, "%lf\t%lf\n", x, v);
        }
        fclose(fh);
    }
    
    char rLabel[32];
    snprintf(rLabel, sizeof rLabel, "roots%d.dat", iteration);
    fh = fopen(rLabel, "wt");
    if (fh)
    {
        for (int i=0; i<dim1; ++i)
        {
            const double x = roots[i];
            const double v = PolyErrorEvaluate(poly, f, x);
            fprintf(fh, "%lf\t%lf\n", x, v);
        }
        fclose(fh);
    }
}

static int numUniqueDouble(double* controlPoints, int dim)
{
    if (dim == 1)
        return 1;
    
    double* temp = (double*) alloca(dim*sizeof(double));
    memcpy(temp, controlPoints, dim*sizeof(double));
    
    for (int i=0; i<dim; ++i)
    {
        for (int j=i+1; j<dim; ++j)
        {
            if (temp[i] > temp[j])
            {
                const double t = temp[i];
                temp[i] = temp[j];
                temp[j] = t;
            }
        }
    }
    
    int unique = 1;
    for (int i=1; i<dim; ++i)
    {
        if (temp[i] != temp[i-1])
            unique++;
    }
    return unique;
}

static inline double fabsDist(double a_, double b_)
{
    const double a = fabs(a_);
    const double b = fabs(b_);
    if (a<b)
        return b-a;
    return a-b;
}

// getSingleRoot
//
// Uses root estimate of min(n*abs(a0/a1), nthroot (abs(a0/an))) which will be a root of
// this polynomial.
static double getSingleRoot(const Poly* a)
{
    const size_t order = a->m_Order;
    if (order == 0)
        return a->m_X[0];
    if (order == 1)
        return -a->m_X[0]/a->m_X[1]; // solve explicitly
    if (order == 3)
        return (-a->m_X[1] + sqrtf(a->m_X[1]*a->m_X[1] - 4 * a->m_X[2] * a->m_X[0])) / (2 * a->m_X[2]);
    
    // >= order 2
    const double n = (double) order;
    const double a0 = a->m_X[order];
    const double a1 = a->m_X[order-1];
    const double an = a->m_X[0];
    
    double rootEstimate = n * fabs(a0/a1);
    if (an)
        rootEstimate = minf(rootEstimate, powf(fabs(a0 / an), 1.0/n));
    
    // calculate derivative
    char* buf = (char*) alloca(PolySize(order-1));
    Poly* dPoly = PolyAlloc(buf, order-1);
    PolyDerivative(dPoly, a);
    
    double eRoot = 1000;
    
    // NR iterate until we're within
    int numIterations = 0;
    while (fabs(eRoot)>1e-6 && numIterations++<10000)
    {
        // evaluated value at this root
        eRoot = PolyEvaluate(a, rootEstimate);
        
        const double d = PolyEvaluate(dPoly, rootEstimate);
        double newRootEstimate = rootEstimate;
        if (d)
            newRootEstimate -= eRoot / d;
        rootEstimate = newRootEstimate;
    }
    
    double rootEstimateCeil = ceilf(rootEstimate);
    double rootEstimateFloor = floorf(rootEstimate);
    if (doubleDist(rootEstimate, rootEstimateCeil) < 1e-4)
        return rootEstimateCeil;
    if (doubleDist(rootEstimate, rootEstimateFloor) < 1e-4)
        return rootEstimateFloor;
    
    return rootEstimate;
}

void PolyCopy(Poly* dest, const Poly* a)
{
    dest->m_Order = a->m_Order;
    memcpy(dest->m_X, a->m_X, sizeof (double) * a->m_Order);
}

// PolyRoots
int PolyRoots(double* realRoots, const Poly* a)
{
    double* saveRealRoots = realRoots;
    
    if (a->m_X[0] == 0.0)
    {
        *realRoots = 0.0;
        return 1;
    }
    
    int numRoots = a->m_Order;
    
    // find smallest root
    char qBuf[64];
    Poly* q = PolyAlloc(qBuf, 2);
    q->m_X[1] = 1.0;
    
    char pBuf[128];
    Poly* poly = PolyAlloc(pBuf, a->m_Order);
    PolyCopy(poly, a);
    while (poly->m_Order >= 2)
    {
        const double rootEstimate = getSingleRoot(poly);
        q->m_X[0] = -rootEstimate;
        
        // push root estimate
        *realRoots++ = rootEstimate;
        
        Poly *divRes = PolyAlloc(alloca(PolySize(q->m_Order)), q->m_Order);
        if (PolyDivide(divRes, poly, q))
        {
            PolyCopy(poly, divRes);
        }
    }
    
    int retVal = realRoots - saveRealRoots;
    realRoots = saveRealRoots;
    
    // exclude non-real roots.
    // - jiv fixme: handle complex roots
    int i=0;
    while (i<retVal)
    {
        double e = PolyEvaluate(a, realRoots[i]);
        double diff = doubleDist(e, 0.0);
        if (diff > kSmallEpsilon)
            realRoots[i] = realRoots[--retVal];
        else
            i++;
    }
    
    return retVal;
}

void PolyMultiply(Poly* dest, const Poly* a, const Poly* b)
{
}

//  ____        ___             ____
// /\  _`\     /\_ \           /\  _`\
// \ \ \L\ \___\//\ \    __  __\ \ \L\_\  _ __   _ __   ___   _ __
//  \ \ ,__/ __`\\ \ \  /\ \/\ \\ \  _\L /\`'__\/\`'__\/ __`\/\`'__\
//   \ \ \/\ \L\ \\_\ \_\ \ \_\ \\ \ \L\ \ \ \/ \ \ \//\ \L\ \ \ \/
//    \ \_\ \____//\____\\/`____ \\ \____/\ \_\  \ \_\\ \____/\ \_\
//     \/_/\/___/ \/____/ `/___/> \\/___/  \/_/   \/_/ \/___/  \/_/
//                           /\___/
//                           \/__/

// PolyErrorEvaluate
//
// Evaluate error function P(x) - f(x) at x0
double PolyErrorEvaluate(const Poly* poly, double (*f)(double x), double x0)
{
    const double v0 = PolyEvaluate(poly, x0);
    const double v1 = f(x0);
    return v0 - v1;
}

// PolyErrorDerivativeAt
float PolyErrorDerivativeAt(const Poly* poly, double (*f)(double x), double x)
{
    const float h = 0.01;
    
    const float abscissa0 = x-2*h;
    const float abscissa1 = x-h;
    const float abscissa2 = x+h;
    const float abscissa3 = x+2*h;
    
    const float x0 = +1*PolyErrorEvaluate(poly, f, abscissa0);
    const float x1 = -8*PolyErrorEvaluate(poly, f, abscissa1);
    const float x2 = +8*PolyErrorEvaluate(poly, f, abscissa2);
    const float x3 = -1*PolyErrorEvaluate(poly, f, abscissa3);
    
    return (x0 + x1 + x2 + x3) / (12.0*h);
}

// PolyErrorPrimeDerivativeAt
float PolyErrorPrimeDerivativeAt(const Poly* poly, double (*f)(double x), double x)
{
    const float h = 0.01;
    
    const float abscissa0 = x-2*h;
    const float abscissa1 = x-h;
    const float abscissa2 = x+h;
    const float abscissa3 = x+2*h;
    
    const float x0 = +1*PolyErrorDerivativeAt(poly, f, abscissa0);
    const float x1 = -8*PolyErrorDerivativeAt(poly, f, abscissa1);
    const float x2 = +8*PolyErrorDerivativeAt(poly, f, abscissa2);
    const float x3 = -1*PolyErrorDerivativeAt(poly, f, abscissa3);
    
    return (x0 + x1 + x2 + x3) / (12.0*h);
}

// PolyErrorGetRoot
double PolyErrorGetRoot(const Poly* poly, double (*f)(double x), double a, double b, float x0)
{
    double x = x0;
    
    double oldValue = nan(nullptr);
    double currentValue = nan(nullptr);
    
    while (doubleDist(oldValue, currentValue) > kSmallEpsilon)
    {
        const double fVal = PolyErrorEvaluate(poly, f, x);
        const double fdVal = PolyErrorDerivativeAt(poly, f, x);
        
        x = x - fVal / fdVal;
        
        // clamp to inteval
        x = (x > a) ? x : a;
        x = (x < b) ? x : b;
        
        oldValue = currentValue;
        currentValue = f(x);
    }
    
    return x;
}

// PolyErrorGetExtremaAbscissa
double PolyErrorGetExtremaAbscissa(const Poly* poly, double (*f)(double x), double a, double b, double x0)
{
    double x = x0;
    
    double oldValue = nan(nullptr);
    double currentValue = nan(nullptr);
    
    while (doubleDist(oldValue, currentValue) > kSmallEpsilon)
    {
        const double fd  = PolyErrorDerivativeAt(poly, f, x);
        const double fpd = PolyErrorPrimeDerivativeAt(poly, f, x);
        
        x = x - fd/fpd;
        
        // clamp to inteval
        x = (x<a) ? a : x;
        x = (x>b) ? b : x;
        
        oldValue = currentValue;
        currentValue = fd;
    }
    
    return x;
}

//                       __
//                      /\ \__
//  __  __     __    ___\ \ ,_\   ___   _ __
// /\ \/\ \  /'__`\ /'___\ \ \/  / __`\/\`'__\
// \ \ \_/ |/\  __//\ \__/\ \ \_/\ \L\ \ \ \/
//  \ \___/ \ \____\ \____\\ \__\ \____/\ \_\
//   \/__/   \/____/\/____/ \/__/\/___/  \/_/
//
size_t RVectorSize(int dimension)
{
    return sizeof(int32_t) + dimension * sizeof(double);
}

size_t RVectorSize(const RVector* a)
{
    return RVectorSize(a->m_Dimension);
}

void RVectorPermute(RVector* dest, int* p)
{
    const int rowSize = sizeof(double)*dest->m_Dimension;
    for (int i=0; i<dest->m_Dimension; ++i)
    {
        const double temp = dest->m_X[i];
        dest->m_X[i] = dest->m_X[p[i]];
        dest->m_X[p[i]] = temp;
        
        int pIndex = -1;
        for (int c=0; pIndex<0 && c<dest->m_Dimension; ++c)
        {
            if (p[c] == i)
                pIndex = c;
        }
        
        const int tempI = p[i];
        p[i] = p[pIndex];
        p[pIndex] = tempI;
    }
}

void RVectorPermute(RVector* dest, const RVector* b, int* p, size_t elementsP)
{
    assert (elementsP == b->m_Dimension);
    assert (dest->m_Dimension == b->m_Dimension);
    
    for (int i=0; i<b->m_Dimension; ++i)
        dest->m_X[i] = b->m_X[p[i]];
}

// RVectorAlloc
RVector* RVectorAlloc(void* data, int dimensions)
{
    RVector* ret = (RVector*) data;
    ret->m_Dimension = dimensions;
    return ret;
}

// RVectorDot
double RVectorDot(const RVector* a, const RVector* b)
{
    assert(a->m_Dimension == b->m_Dimension);
    double sum = 0.0;
    for (int i=0,count=a->m_Dimension; i<count; ++i)
        sum += a->m_X[i] * b->m_X[i];
    return sum;
}

// RVectorDump
void RVectorDump(const char* label, const RVector* a)
{
    const size_t order = a->m_Dimension;
    printf("%s(%ld): [ ", label, order);
    for (int i=0; i<order; ++i)
    {
        const double coeff = a->m_X[i];
        const double aCoeff = fabs(coeff);
        
        const char* end = i==order-1 ? "" : " ";
        printf("%.2f%s", coeff, end);
    }
    printf(" ]\n");
}

void RVectorScale(RVector* dest, const RVector* a, double scale)
{
    for (int i=0; i<a->m_Dimension; ++i)
        dest->m_X[i] = a->m_X[i] * scale;
}

void RVectorNormalize(RVector* dest, const RVector* a)
{
    float lengthSqr = RVectorDot(a, a);
    if (fabsf(lengthSqr)>1e-3f)
        RVectorScale(dest, a, 1/sqrt(lengthSqr));
}

// RMatPermute
void RMatPermute(RMat* dest, const RMat* a, int* p)
{
    for (int i=0; i<a->m_Rows; ++i)
    {
        double* dDouble = RMatGetRow(dest, i);
        const double* source = RMatGetRow(a, p[i]);
        memcpy(dDouble, source, sizeof(double)*a->m_Columns);
    }
}

// RMatPermute
void RMatPermute(RMat* dest, int* p)
{
    const int rowSize = sizeof(double)*dest->m_Columns;
    double* temp = (double*) alloca(dest->m_Columns*sizeof(double));
    for (int i=0; i<dest->m_Rows; ++i)
    {
        double* sourceDouble = RMatGetRow(dest, i);
        double* mappedDouble = RMatGetRow(dest, p[i]);
        
        memcpy(temp, sourceDouble, rowSize);
        memcpy(sourceDouble, mappedDouble, rowSize);
        memcpy(mappedDouble, temp, rowSize);
        
        int pIndex = -1;
        for (int c=0; pIndex<0 && c<dest->m_Rows; ++c)
        {
            if (p[c] == i)
                pIndex = c;
        }
        
        const int tempI = p[i];
        p[i] = p[pIndex];
        p[pIndex] = tempI;
    }
}

// RMatPermuteIdentity
void RMatPermuteIdentity(RMat* dest, int* p)
{
    for (int i=0; i<dest->m_Rows; ++i)
    {
        double* dDouble = RMatGetRow(dest, i);
        memset(dDouble, 0.0, sizeof(double)*dest->m_Columns);
        dDouble[p[i]] = 1.0;
    }
}

// RMatCopy
void RMatCopy(RMat* dest, const RMat* source)
{
    *dest = *source;
    memcpy(dest->m_Data, source->m_Data, source->m_Rows*source->m_Columns*sizeof(double));
}

void RMatCopy(RMat* dest, const Mat4* mat4)
{
    dest->m_Rows = 4;
    dest->m_Columns = 4;
    memcpy(dest->m_Data, mat4, 64);
}

void RMatCopy(RMat* dest, const Mat3* mat3)
{
    dest->m_Rows = 3;
    dest->m_Columns = 3;
    memcpy(dest->m_Data, mat3, 36);
}

RMat* RMatCopy(void* data, const RMat* a)
{
    RMat* dest = (RMat*) data;
    RMatCopy(dest, a);
    return dest;
}

size_t RMatSize(int rows, int columns)
{
    return 2*sizeof(int32_t) + rows * columns * sizeof(double);
}

size_t RMatSize(const RMat* rmat)
{
    return RMatSize(rmat->m_Rows, rmat->m_Columns);
}

RMat* RMatAlloc(void* data, int rows, int columns)
{
    RMat* ret = (RMat*) data;
    ret->m_Rows = rows;
    ret->m_Columns = columns;
    return ret;
}

// RMatMakeIdentity
void RMatMakeIdentity(RMat* dest)
{
    assert(dest->m_Rows == dest->m_Columns);
    memset(dest->m_Data, 0, dest->m_Rows * dest->m_Columns * sizeof(double));
    for (int r=0; r<dest->m_Rows; ++r)
        dest->m_Data[r+r*dest->m_Columns] = 1.0;
}

// RMatMakeZero
void RMatMakeZero(RMat* dest)
{
    memset(dest->m_Data, 0, dest->m_Rows*dest->m_Columns*sizeof(double));
}

// RMatTranspose
void RMatTranspose(RMat* dest, const RMat* a)
{
    assert(dest->m_Rows == a->m_Columns);
    assert(dest->m_Columns == a->m_Rows);
    
    for (int r=0; r<a->m_Rows; ++r)
    {
        for (int c=0; c<a->m_Columns; ++c)
        {
            const double value = RMatGetData(a, r, c);
            RMatSetData(dest, c, r, value);
        }
    }
}

// RMatMulMat
// dest = a * b
void RMatMulMat(RMat* dest, const RMat* a, const RMat* b)
{
    for (int c=0; c<b->m_Columns; ++c)
    {
        const int prevRows = a->m_Rows;
        const int prevCols = a->m_Columns;
        
        for (int r=0; r<a->m_Rows; ++r)
        {
            const double* aRow = RMatGetRow(a, r);
            
            double sum = 0.0;
            for (int i=0; i<a->m_Columns; ++i)
                sum += aRow[i] * RMatGetData(b, i, c);
            
            RMatSetData(dest, r, c, sum);
        }
    }
}

// RMatMulVec
// dest = a*b
void RMatMulVec(RVector* dest, const RMat* a, const RVector* b)
{
    assert(a->m_Columns == b->m_Dimension);
    for (int r=0; r<a->m_Rows; ++r)
    {
        double sum = 0.0;
        for (int k=0; k<b->m_Dimension; ++k)
            sum += a->m_Data[k+r*a->m_Columns] * b->m_X[k];
        dest->m_X[r] = sum;
    }
}

// RMatDump
void RMatDump(const char* label, const RMat* a)
{
    printf("%s(%d,%d):\n[\n", label, a->m_Rows, a->m_Columns);
    for (int r=0; r<a->m_Rows; ++r)
    {
        const bool lastRow = r == a->m_Rows - 1;
        printf("    [ ");
        for (int c=0; c<a->m_Columns; ++c)
        {
            const bool lastColumn = c == a->m_Columns - 1;
            
            const double val = RMatGetData(a, r, c);
            const double aVal = fabs(val);
            const char* signSpacer = lastColumn ? " " : ", ";
            printf("%+lf%s", val, signSpacer);
        }
        printf("]%s", lastRow ? "\n" : ",\n");
    }
    printf("]\n");
}

// RMatDump
void RMatDump(const char* label, const RMat* a, const RMat* b)
{
    printf("%s(%d,%d):\n[\n", label, a->m_Rows, a->m_Columns);
    for (int r=0; r<a->m_Rows; ++r)
    {
        const bool lastRow = r == a->m_Rows - 1;
        printf("    [ ");
        for (int c=0; c<a->m_Columns; ++c)
        {
            const bool lastColumn = c == a->m_Columns - 1;
            
            const char* end = lastColumn ? "" : "";
            const double val = RMatGetData(a, r, c);
            const double aVal = fabs(val);
            const char* signSpacer = RMatGetData(a, r, c+1)>=0 ? (lastColumn ? "" : ", ") : " "; // jiv fixme
            printf("%+.10lf%s%s", val, signSpacer, end);
        }
        printf(" | ");
        for (int c=0; c<a->m_Columns; ++c)
        {
            const bool lastColumn = c == a->m_Columns - 1;
            
            const char* end = lastColumn ? "" : "";
            const double val = RMatGetData(b, r, c);
            const double aVal = fabs(val);
            const char* signSpacer = RMatGetData(b, r, c+1)>=0 ? (lastColumn ? "" : ", ") : " "; // jiv fixme
            printf("%+.10lf%s%s", val, signSpacer, end);
        }
        printf(" ]%s", lastRow ? "\n" : ",\n");
    }
    printf("]\n");
}

// RMatDecomposeLdu
void RMatDecomposeLdu(RMat* lower, RMat* diagonal, RMat* upper, int* p, const RMat* a)
{
    int* indices = p;
    
    // which indices have been set yet
    uint8_t* indicesSet = (uint8_t*)alloca(sizeof(uint8_t)*a->m_Columns);
    memset(indicesSet, 0, sizeof(uint8_t)*a->m_Columns);
    
    for (int col=0; col<a->m_Columns; ++col)
    {
        int ndx = -1;
        double value = -1.0;
        for (int row=0; row<a->m_Rows; ++row)
        {
            if (indicesSet[row])
                continue;
            
            const double scalar = fabs(RMatGetData(a, row, col));
            if (scalar > value)
            {
                value = scalar;
                ndx = row;
            }
        }
        
        indicesSet[ndx] = 1;
        *p++ = ndx;
    }
    
    // initialize upper
    RMatPermute(upper, a, indices);
    RMatMakeIdentity(lower);
    
    // iterate over each pivot column
    for (int ri=0; ri<a->m_Rows; ++ri)
    {
        const double* pivotSourceRow = RMatGetRow(upper, ri);
        const double pivotDiv = RMatGetData(upper, ri, ri);
        
        // iterate over the other rows, adding -(row_value / pivot_value)
        for (int v=ri+1; v<a->m_Rows; ++v)
        {
            const double* leftSourceRow = RMatGetRow(upper, v);
            double* leftDestRow = RMatGetRow(upper, v);
            double* rightDestRow = RMatGetRow(lower, v);
            
            const double rowValAtPivotColumn = RMatGetData(upper, v, ri);
            if (fabs(pivotDiv) < kSmallEpsilon || fabs(rowValAtPivotColumn) < kSmallEpsilon)
                continue;
            
            const double scale = 1/pivotDiv * rowValAtPivotColumn;
            
            // update U with pivot
            for (int i=0; i<a->m_Columns; ++i)
                leftDestRow[i] = leftSourceRow[i] - scale * pivotSourceRow[i];
            
            // save inverse to L
            RMatSetData(lower, v, ri, scale);
        }
    }
    
    // divide out diagonal elements
    if (diagonal)
    {
        // initialize diagonal matrix
        RMatMakeZero(diagonal);
        
        for (int ri=0; ri<upper->m_Rows; ++ri)
        {
            const double dValue = RMatGetData(upper, ri, ri);
            const double invDValue = 1.0 / dValue;
            
            RMatSetData(diagonal, ri, ri, dValue);
            double* dest = RMatGetRow(upper, ri);
            for (int i=0; i<upper->m_Columns; ++i)
            {
                dest[i] *= invDValue;
                if (dest[i] == -0.0)
                    dest[i] = 0.0;
            }
        }
    }
}

// DoubleDump
void DoubleDump(const char* label, const double* x, int rows)
{
    printf("%s[ ", label);
    for (int i=0; i<rows; ++i)
        printf("%.3f ", x[i]);
    printf("]\n");
}

// IntDump
void IntDump(const char* label, const int* x, int rows)
{
    printf("%s[ ", label);
    for (int i=0; i<rows; ++i)
        printf("%d ", x[i]);
    printf("]\n");
}

// lower columns = upper rows
// lower rows = a rows
// upper columns = a columns
void RMatSolve(RVector* dest, const RMat* a, const RVector* b)
{
    RMat* upper = RMatAllocMatchSize(alloca(RMatSize(a)), a);
    RMat* lower = RMatAlloc(alloca(RMatSize(a->m_Rows, a->m_Rows)), a->m_Rows, a->m_Rows);
    int* p = (int*)alloca(a->m_Rows * sizeof(int));
    
    // decompose
    RMatDecomposeLdu(lower, nullptr, upper, p, a);
    
    // initialize destination array
    memset(dest->m_X, 0, dest->m_Dimension*sizeof(double));
    
    // solve lower
    for (int i=0; i<lower->m_Rows; ++i)
    {
        double bi = b->m_X[p[i]];
        for (int j=0; j<i; ++j)
            bi -= dest->m_X[j] * RMatGetData(lower, i, j);
        dest->m_X[i] = bi;
    }
    
    // solve upper
    for (int i=upper->m_Rows-1; i>=0; --i)
    {
        const double coeff = RMatGetData(upper, i, i);
        double bi = dest->m_X[i];
        
        for (int j=upper->m_Columns-1; j>i; --j)
        {
            const double rUpper = RMatGetData(upper, i, j);
            bi -= dest->m_X[j] * rUpper;
        }
        dest->m_X[i] = bi / RMatGetData(upper, i, i);
    }
}

// RMatLeastSquare
void RMatLeastSquare(RVector* dest, const RMat* a, const RVector* b)
{
    RMat* aT = RMatAlloc(alloca(RMatSize(a->m_Columns, a->m_Rows)), a->m_Columns, a->m_Rows);
    RMat* aTa = RMatAlloc(alloca(RMatSize(a->m_Columns, a->m_Columns)), a->m_Columns, a->m_Columns);
    RVector* aTb = RVectorAlloc(alloca(RVectorSize(b)), b->m_Dimension);
    
    // get aTa
    RMatTranspose(aT, a);
    RMatMulMat(aTa, aT, a);
    
    // get aTb
    RMatMulVec(aTb, aT, b);
    
    // solve for aTa = aTb
    RMatSolve(dest, aTa, aTb);
}

// ChebyshevRoots
void ChebyshevRoots(double* dest, int dim, double a, double b)
{
    const double bias  = 0.5 * (a + b);
    const double scale = 0.5 * (b - a);
    
    const double twoN = M_PI / (2.0f * dim);
    
    for (int k=0; k<dim; ++k)
    {
        const double cf = cos((2*(k+1)-1) * twoN);
        dest[k] = bias + scale * cf;
    }
}

// ChebyshevExtrema
void ChebyshevExtrema(double* dest, int dim, double a, double b)
{
    const double bias  = 0.5 * (a + b);
    const double scale = 0.5 * (b - a);
    
    const double twoN = M_PI / (2.0f * dim);
    
    for (int k=0; k<dim; ++k)
    {
        const double cf = -cos((k*M_PI)/(dim-1));
        dest[k] = bias + scale * cf;
    }
}

// RMatLeastSquarePoly
void RMatLeastSquarePoly(Poly* poly, const double* abscissae, const RVector* y)
{
    const int dim = poly->m_Order;
    
    // use least squares to give us P, our interpolating polynomial
    RMat* leastSquares = RMatAlloc(alloca(RMatSize(dim, dim)), dim, dim);
    for (int r=0; r<dim; ++r)
    {
        double t = 1.0;
        for (int c=0; c<dim; ++c)
        {
            RMatSetData(leastSquares, r, c, t);
            t *= abscissae[r];
        }
    }
    
    RMatLeastSquare((RVector*)poly, leastSquares, y);
}

// Remez
void Remez(RVector* dest, double (*f)(double x), double a, double b)
{
    // dim   = n+1
    // dim+1 = n+2
    const int dim = dest->m_Dimension;
    
    // come up with a least squares polynomial approximation
    Poly* poly = PolyAlloc(alloca(dim), dim);
    
    double* controlPoints = (double*) alloca(sizeof(double)*(dim+1));
    ChebyshevExtrema(controlPoints, dim+1, a, b);
    
    Poly* p = PolyAlloc(alloca(RVectorSize(dim)), dim);
    RVector* y = RVectorAlloc(alloca(RVectorSize(dim+1)), dim+1);
    
    int iteration = 0;
    do
    {
        // step 0: check to see if we're done.
        bool found = true;
        
        for (int i=0; found && i<dim; ++i)
        {
            const double cp0 = PolyErrorEvaluate(p, f, controlPoints[i]);
            const double cp1 = PolyErrorEvaluate(p, f, controlPoints[i+1]);
            
            // 0a:
            // errors alternate in sign
            if (fabs(cp0)>kVerySmallEpsilon && signbit(cp0) == signbit(cp1))
                found = false;
            
            // 0b:
            // errors are equal in magnitude
            if (fabsDist(cp0, cp1) > kSmallEpsilon)
                found = false;
        }
        
        if (found)
            break;
        
        // 0a: errors alternate in sign
        
        // step 1: solve for P(x) + -1^i*E = f(x), for x_0...x_n-1 at extrema
        //     In linear form [p_0 + -1^i*E] [pPrime_0] = [yPrime_0]
        //
        // y -> [ f(x_0), ..., f(x_n-1) ]
        // p -> [ p_0, ..., p_n-1 ]^T
        
        {
            RVector* temp = RVectorAlloc(alloca(RVectorSize(dim+1)), dim+1);
            RMat* leastSquares = RMatAlloc(alloca(RMatSize(dim+1, dim+1)), dim+1, dim+1);
            for (int r=0; r<dim+1; ++r)
            {
                double t = 1.0;
                for (int c=0; c<dim; ++c)
                {
                    RMatSetData(leastSquares, r, c, t);
                    t *= controlPoints[r];
                }
                
                // error
                double sign = (r&1) ? -1.0 : 1.0;
                RMatSetData(leastSquares, r, dim, sign);
                
                y->m_X[r] = f(controlPoints[r]);
            }
            
            RMatSolve(temp, leastSquares, y);
            memcpy(p->m_X, temp->m_X, p->m_Order*sizeof(double));
        }
        
        // step 2:
        // locate extrema of error function
        assert(numUniqueDouble(controlPoints, dim+1) == dim+1);
        
        // 2a:
        // find roots of error function.  They are between our control points
        double* roots = (double*) alloca(sizeof(double)*dim+2);
        roots[0] = a;
        roots[dim+1] = b;
        for (int i=0; i<dim; ++i)
        {
            const double intervalBegin = controlPoints[i];
            const double intervalEnd = controlPoints[i+1];
            const double x0 = Lerp(intervalBegin, intervalEnd, (i+1)/double(dim+1));
            roots[i+1] = PolyErrorGetRoot(p, f, intervalBegin, intervalEnd, x0);
        }
        
        assert(numUniqueDouble(roots, dim+2) == dim+2);
        assert(numUniqueDouble(controlPoints, dim+1) == dim+1);
        
        // 2b:
        // find extrema of error function.  They are between our roots, plus a and b.
        for (int i=0; i<dim+1; ++i)
        {
            const double intervalBegin = roots[i];
            const double intervalEnd = roots[i+1];
            const double x0 = Lerp(intervalBegin, intervalEnd, i/double(dim));
            const double x = PolyErrorGetExtremaAbscissa(p, f, intervalBegin, intervalEnd, x0);
            controlPoints[i] = x;
        }
        
        assert(numUniqueDouble(controlPoints, dim+1) == dim+1);
    }
    while (true);
    
    memcpy(dest->m_X, p->m_X, p->m_Order*sizeof(double));
}

size_t RMatSize(const Mat4* mat4)
{
    return RMatSize(4, 4);
}

size_t RMatSize(const Mat3* mat4)
{
    return RMatSize(3, 3);
}

void RMatInvertIterate(RVector* dest, const RMat* a, float lambda)
{
    RMat* u = RMatCopy(alloca(RMatSize(a)), a);
    
    //  0  1  2
    //  3  4  5
    //  6  7  8
    u->m_Data[ 0] -= lambda;
    u->m_Data[ 4] -= lambda;
    u->m_Data[ 8] -= lambda;

    dest->m_X[0] = rand()/(float)RAND_MAX * 2.0f * lambda - lambda;
    dest->m_X[1] = rand()/(float)RAND_MAX * 2.0f * lambda - lambda;
    dest->m_X[2] = rand()/(float)RAND_MAX * 2.0f * lambda - lambda;
    
    RVector* x = RVectorAlloc(alloca(RVectorSize(a->m_Rows)), a->m_Rows);
    for (int i=0; i<10; ++i)
    {
        RMatSolve(x, u, dest);
        RVectorNormalize(dest, x);
    }
}
