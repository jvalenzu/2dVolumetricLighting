#pragma once

#include <new>

template <typename T, int maxSize>
struct FixedVector
{
    char m_Raw[maxSize * sizeof(T)];
    int m_Index = 0;
    
    FixedVector()
    {
    }
    
    T* alloc()
    {
        if (m_Index == maxSize)
            return nullptr;
        T* ret = m_Raw[m_Index++ * sizeof(T)];
        new (ret) T();
        return ret;
    }
    
    void erase(T* victim)
    {
        int index = victim - (T*)m_Raw[0];
        if (index >= maxSize)
            return;
        // victim->~T();
        *victim = *(T*)m_Raw[--m_Index*sizeof(T)];
    }
};
