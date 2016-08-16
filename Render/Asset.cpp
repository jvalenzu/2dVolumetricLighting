#include "Render/Asset.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include "Engine/Utils.h"

#include <assert.h>
#include <stdio.h>

void AssetHandle::Invalidate()
{
    m_Index = 0xffffffffffffffffULL;
}

void AssetHandleTable::Create(int max)
{
    m_MaxAssetHandles = max;
    m_Indirect = new int[max];
    m_ReverseIndirect = new int[max];
    
    for (int i=0; i<max; ++i)
        m_Indirect[i] = m_ReverseIndirect[i] = i;
}

void AssetHandleTable::Destroy()
{
    delete[] m_Indirect;
    delete[] m_ReverseIndirect;
}

void AssetHandleTable::Relocate(int destIndex, int sourceIndex)
{
    int origDestIndex = m_ReverseIndirect[destIndex];
    int origSourceIndex = m_ReverseIndirect[sourceIndex];
    
    int temp = destIndex; // aka m_Indirect[origDestIndex];
    m_Indirect[origDestIndex] = m_Indirect[origSourceIndex];
    m_Indirect[origSourceIndex] = temp;
    
    int temp0 = m_ReverseIndirect[destIndex];
    m_ReverseIndirect[destIndex] = m_ReverseIndirect[sourceIndex];
    m_ReverseIndirect[sourceIndex] = temp0;
    
    CheckInvariant();
}

void AssetHandleTable::CheckInvariant()
{
    for (int i=0; i<m_MaxAssetHandles; ++i)
    {
        assert(m_Indirect[m_ReverseIndirect[i]] == i);
    }
}

void AssetHandleTable::Dump(int num)
{
    if (num == -1)
        num = m_MaxAssetHandles;
    
    Printf("num: %d\n", num);
    Printf("  # |   s   |      t\n");
    Printf("---------------------\n");
    for (int i=0; i<num; ++i)
        Printf("%3d | %3d   |    %3d\n", i, m_Indirect[i], m_ReverseIndirect[i]);
    Printf("\n");
}

void AssetHandleTableTest()
{
    AssetHandleTable assetHandleTable;
    assetHandleTable.Create(4);
    assetHandleTable.Relocate(1, 3);
    assetHandleTable.Relocate(0, 3);
    assetHandleTable.Relocate(1, 3);
    assetHandleTable.Relocate(2, 3);
    
    assetHandleTable.Dump();
}

template <typename T, typename U>
int HandleAssetManager<T,U>::Find(uint32_t needle)
{
    for (int i=0; i<m_NumAssets; ++i)
        if (m_Crc[i] == needle)
            return i;
    return -1;
}

template <typename T, typename U>
void HandleAssetManager<T,U>::Dump()
{
    int max = 0;
    for (int i=0; i<m_NumAssets; ++i)
    {
        if (m_Assets[i].m_AssetIndex+1 > max)
            max = m_Assets[i].m_AssetIndex+1;
    }
    
    if (max)
        m_AssetHandleTable.Dump(max);
    
    DumpInternal();
}

template <typename T, typename U>
void HandleAssetManager<T,U>::CheckInvariant()
{
    for (int i=0; i<m_NumAssets; ++i)
    {
        T* shader = &m_Assets[i];
        U shaderHandle = shader->GetHandle();
        if (shaderHandle.Resolve() != shader)
        {
            Printf("shader mismatch %d assetIndex %d name %s vs assetIndex %d name %s\n",
                   i,
                   shaderHandle.Resolve()->m_AssetIndex,
                   shaderHandle.Resolve()->m_DebugName,
                   shader->m_AssetIndex,
                   shader->m_DebugName);
        }
        assert(shaderHandle.Resolve() == shader);
    }
}

template<typename T, typename U>
T* HandleAssetManager<T,U>::AllocateAsset(uint32_t crc)
{
    if (m_NumAssets < kMaxAssets)
    {
        int actualIndex = m_NumAssets++;
        T* ret = &m_Assets[actualIndex];
        ret->Invalidate();
        ret->m_AssetIndex = m_AssetHandleTable.GetReverseIndex(actualIndex);
        m_RefCount[actualIndex] = 1;
        m_Crc[actualIndex] = crc;
        return ret;
    }
    
    return nullptr;
}

template<typename T, typename U>
void HandleAssetManager<T,U>::DestroyAsset(T victim)
{
    CheckInvariant();
    
    // stomp victim with mover
    T mover = m_Assets[--m_NumAssets];
    
    const int destIndex = m_AssetHandleTable.GetForwardIndex(victim.m_AssetIndex);
    
    m_Assets[destIndex] = mover;
    if (m_NumAssets != destIndex)
    {
        m_Assets[m_NumAssets].Invalidate();
        m_RefCount[destIndex] = m_RefCount[m_NumAssets];
        m_Crc[destIndex] = m_Crc[m_NumAssets];
        m_RefCount[m_NumAssets] = 0;
        m_Crc[m_NumAssets] = 0;
    }
    
    // fixup indices
    m_AssetHandleTable.Relocate(destIndex, m_NumAssets);
    
    CheckInvariant();
}

template <typename T>
void SimpleAssetManager<T>::DestroyAsset(T* victim)
{
    size_t index = victim - m_Assets;
    m_Crc[index] = 0;
    m_NumAssets--;
    assert(m_NumAssets >= 0);
}

template <typename T>
T* SimpleAssetManager<T>::AllocateAsset(uint32_t crc)
{
    for (int i=0; i<kMaxAssets; ++i)
    {
        if (m_Crc[i] == 0)
        {
            m_Crc[i] = crc;
            m_NumAssets++;
            return &m_Assets[i];
        }
    }
    
    return nullptr;
}

template <typename T>
int SimpleAssetManager<T>::Find(uint32_t crc)
{
    for (int i=0; i<kMaxAssets; ++i)
    {
        if (m_Crc[i] == crc)
            return i;
    }

    return -1;
}

template <typename T>
void SimpleAssetManager<T>::Dump(int num)
{
    if (num == -1)
        num = m_NumAssets;
    
    if (num>0)
    {
        DumpTitle();
        
        Printf("num: %d\n", num);
        Printf("  # |   crc\n");
        Printf("---------------------\n");
        for (int i=0; num&&i<kMaxAssets; ++i)
        {
            if (m_Crc[i]==0)
                continue;
            num--;
            Printf("%3d | 0x%x ", i, m_Crc[i]);
            DumpInternal(&m_Assets[i]);
            
        }
        
        Printf("\n");
    }
}

template struct SimpleAssetManager<Shader>;
template struct SimpleAssetManager<Texture>;
