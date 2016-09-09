// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <stdint.h>

struct AssetHandle
{
    enum : uint64_t
    {
        kCountShift = 0,
        kCountMask = 0xffff,
        kIndexShift = 16,
        kIndexMask = 0xffff
    };
    uint64_t m_Index;
    
    AssetHandle()
        : m_Index(0xffffffffffffffffULL)
    {
    }

    bool IsValid() const
    {
        return m_Index != 0xffffffffffffffffULL;
    }

    void Invalidate();
};

// AssetManager/AssetHandleTable
//
// This pair of structures is used to allocate simple value types (which may contain embedded pointers to heap memory) s.t.
//   a) allocation/deallocation is simple (increment number into fixed array to allocate, decrement and swap victim with last to deallocate)
//   b) live elements are dense
//
//
//
// The AssetManager returns handles to assets which are resolved on demand.  It uses two indirection tables: forward and reverse (s and t).
// Assets have an asset index which is embedded in handles, and the tables are ordered s.t. m_Assets[s[x]] is the asset with asset index x.
// When allocated, the asset allocated from the end of the liveness block and assigned an asset index x s.t. &asset == &m_Assets[s[x]].
// This asset index x is found from the reverse index table t, each element of which satisfies s[t[x]] == x (t[y] contains the index i in s
// where s[i] == y.
//
// Let n be the number of elements, f be an array of objects type foo, x be the asset index of f, s and t as described above.
//  allocation: d = n. Increment n.  x=t[d], return f[d].
//  deallocation (victim is f[y])
//    o decrement n.
//    o swap f[s[y]] and f[n].
//    o relocate s[y] and n.
//    o swap s[t[s[y]]] and s[t[n]] (aka swap s[y] and s[t[n]] instead).
//    o swap t[y] and t[n]
//
// Clients are required to maintain handles instead of raw pointers.  The tradeoff is suitable for things that require a densely packed array and
// the increased cost of Resolve is appropriate.
struct AssetHandleTable
{
    int m_MaxAssetHandles;
    int* m_Indirect;
    int* m_ReverseIndirect;
    
    void Create(int max);
    void Destroy();
    
    inline int GetForwardIndex(int index)
    {
        return m_Indirect[index];
    }
    
    inline int GetReverseIndex(int index)
    {
        return m_ReverseIndirect[index];
    }
    
    void Relocate(int destIndex, int sourceIndex);
    void CheckInvariant();
    void Dump(int num=-1);
};

template <typename T, typename U>
struct HandleAssetManager
{
    enum
    {
        kMaxAssets = 8192
    };
    T        m_Assets[kMaxAssets];
    int      m_RefCount[kMaxAssets];
    uint32_t m_Crc[kMaxAssets];
    AssetHandleTable m_AssetHandleTable;
    int m_NumAssets;
    
    HandleAssetManager()
    {
        m_NumAssets = 0;
        m_AssetHandleTable.Create(kMaxAssets);
        for (int i=0; i<kMaxAssets; ++i)
        {
            m_Assets[i].Invalidate();
            m_RefCount[i] = 0;
            m_Crc[i] = 0;
        }
    }
    
    void Destroy()
    {
        m_AssetHandleTable.Destroy();
        for (int i=0; i<kMaxAssets; ++i)
            m_Assets[i].Invalidate();
    }
    
    T* GetByHandleIndex(int handleIndex)
    {
        const int index = m_AssetHandleTable.GetForwardIndex(handleIndex);
        if (index >= 0)
            return &m_Assets[index];
        return nullptr;
    }
    
    void IncrementRefCount(int handleIndex)
    {
        const int index = m_AssetHandleTable.GetForwardIndex(handleIndex);
        m_RefCount[index]++;
    }
    
    void DecrementRefCount(int handleIndex)
    {
        const int index = m_AssetHandleTable.GetForwardIndex(handleIndex);
        if (--m_RefCount[index] == 0)
            DestroyAsset(m_Assets[index]);
    }
    
    T* GetBySimpleIndex(int simpleIndex)
    {
        if (simpleIndex >= 0)
            return &m_Assets[simpleIndex];
        return nullptr;
    }
    
    // returns handle
    T*           AllocateAsset(uint32_t crc);
    void         DestroyAsset(T assetHandle);
    int          Find(uint32_t crc); // index on success, -1 on error
    
    void         CheckInvariant();
    void         Dump();
    virtual void DumpInternal() { }
};

// the simple asset manager doesn't maintain a densely packed array
template <typename T>
struct SimpleAssetManager
{
    enum
    {
        kMaxAssets = 8192
    };
    T        m_Assets[kMaxAssets];
    uint32_t m_Crc[kMaxAssets];
    int      m_NumAssets;
    
    SimpleAssetManager()
    {
        m_NumAssets = 0;
        for (int i=0; i<kMaxAssets; ++i)
        {
            m_Assets[i].Invalidate();
            m_Crc[i] = 0;
        }
    }
    
    void Destroy()
    {
        for (int i=0; i<kMaxAssets; ++i)
            m_Assets[i].Invalidate();
    }
    
    T*           AllocateAsset(uint32_t crc);
    void         DestroyAsset(T* victim);
    int          Find(uint32_t crc); // index on success, -1 on error
    
    virtual void CheckInvariant() { }
    void         Dump(int num=-1);
    virtual void DumpTitle() { }
    virtual void DumpInternal(const T* victim) { }
};

void AssetHandleTableTest();
