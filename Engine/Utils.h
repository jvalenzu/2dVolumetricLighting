// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include <math.h>
#include <stdio.h>
#include <stdint.h>

namespace Utils
{
    template <typename T>
    void swap(T& a, T& b)
    {
        T temp = a;
        a = b;
        b = temp;
    }
}


#define OFFSETOF(s,m)        (((size_t)(&((s*)0x10)->m)) - 0x10)
#define BUFFER_OFFSETOF(s,m) ((char *)nullptr + (offsetof(s,m)))
#define MIN(a,b)             ((a)>(b)?(a):(b))
#define ELEMENTSOF(a)        (sizeof(a)/(sizeof(a)[0]))

inline bool FloatEqual(float a, float b, float epsilon)
{
    return fabsf(a-b) < epsilon;
}

char* FileGetAsText(const char* fname);

// http://www.cse.yorku.ca/~oz/hash.html
inline uint32_t Djb(const char* str)
{
    uint32_t hash = 5381;
    int c;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

int Printf(const char* fmt, ...);
int FPrintf(FILE* fh, const char* fmt, ...);
