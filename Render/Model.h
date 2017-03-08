// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <stdint.h>

#define kVertexAttributePosition 0
#define kVertexAttributeColor    1
#define kVertexAttributeNormal   2
#define kVertexAttributeTexCoord 3

#define kModelClassBuiltinCube   1

class ModelClassSubset;
class ModelInstance;
class RenderContext;

struct ModelClass
{
    ModelClassSubset* m_Subsets;
    int m_NumSubsets;
    
    int m_RefCount;
    uint32_t m_Crc;

    void Invalidate()
    {
        m_RefCount = 0;
        m_Crc = 0;
        m_NumSubsets = 0;
        m_Subsets = nullptr;
    }
    
    ModelClass() : m_RefCount(0), m_Crc(0)
    {
    }
};


// -------------------------------------------------------------------------------------------------
void      ModelClassInit(RenderContext* renderContext);
void      ModelClassFini();

// -------------------------------------------------------------------------------------------------
ModelClass*        ModelClassCreate(RenderContext* renderContext, const char* path);

// -------------------------------------------------------------------------------------------------
ModelClass*        ModelClassAllocateSpawned(int numSubsets);

// -------------------------------------------------------------------------------------------------
// does not refcount increase
ModelClass*        ModelClassFind(uint32_t crc);

// -------------------------------------------------------------------------------------------------
inline ModelClass* ModelClassRef(ModelClass* modelClass)
{
    if (modelClass)
        modelClass->m_RefCount++;
    return modelClass;
}

// -------------------------------------------------------------------------------------------------
void               ModelClassDestroy(ModelClass* modelClass);

// -------------------------------------------------------------------------------------------------
void               ModelInstanceDestroy(ModelInstance* simpleModel);
