#include <math.h>
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
