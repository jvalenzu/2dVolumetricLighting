#pragma once

#include <new>

template <typename T, int maxSize>
struct FixedVector
{
    enum
    {
        kMaxSize = maxSize
    };
    char m_Raw[maxSize * sizeof(T)];
    int m_Index = 0;
    
    FixedVector()
    {
    }
    
    T* Data()
    {
        return (T*)m_Raw;
    }
    
    const T* Data() const
    {
        return (const T*)m_Raw;
    }
    
    void SetCount(int count)
    {
        m_Index = count;
    }
    
    int Count() const
    {
        return m_Index;
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
    
    T operator[](size_t index) const
    {
        const T* array = Data();
        return array[index];
    }
    
    T operator[](size_t index)
    {
        T* array = Data();
        return array[index];
    }
};
