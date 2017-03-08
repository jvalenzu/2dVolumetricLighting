// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Render/GL.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "slib/Common/Util.h"
#include "Render/Private/Material.h"
#include "Render/Private/Render.h"
#include "Render/Asset.h"
#include "Render/Render.h"
#include "Engine/Matrix.h"
#include "Render/Material.h"
#include "Render/Model.h"
#include "Render/PostEffect.h"
#include "Render/Texture.h"
#include "Engine/Utils.h"
#include "Tool/Utils.h"

// -------------------------------------------------------------------------------------------------
// internal singleton
struct ModelClassManager : SimpleAssetManager<ModelClass>
{
    ModelClass* CreateModelClass(RenderContext* renderContext, const char* fname);
    void        DestroyModelClass(ModelClass* victim);
    ModelClass* AllocateModelClass(uint32_t crc, int numSubsets);
    
    virtual void DumpTitle();
    virtual void DumpInternal();
};

void ModelClassManager::DestroyModelClass(ModelClass* victim)
{
    DestroyAsset(victim);
    
    for (int i=0; i<victim->m_NumSubsets; ++i)
    {
        ModelClassSubset* modelClassSubset = &victim->m_Subsets[i];
        
        MaterialDestroy(modelClassSubset->m_Material);
        
        glDeleteBuffers(1, &modelClassSubset->m_VertexBufferName);
        glDeleteBuffers(1, &modelClassSubset->m_IndexBufferName);
        glDeleteVertexArrays(1, &modelClassSubset->m_VaoName);
        
        delete [] modelClassSubset->m_Vertices;
        delete [] modelClassSubset->m_Indices;
    }
    
    delete[] victim->m_Subsets;
}

ModelClass* ModelClassManager::CreateModelClass(RenderContext* renderContext, const char* fname)
{
    size_t fname_length = strlen(fname);
    if (fname_length == 0)
        return nullptr;
    
    if (fname_length >= 4 && !strcmp(&fname[fname_length-4], ".bin"))
    {
        ModelClass* ret = nullptr;
        struct stat buf;
        if (stat(fname, &buf) >= 0)
        {
            FILE* fh = fopen(fname, "rb");
            char* data = (char*) malloc(buf.st_size);
            char* base = data;
            fread(data, 1, buf.st_size, fh);
            fclose(fh);
            
            const int numSurfaces = *(short*)data;
            data += 2;
            
            // pad
            data += 2;
            
            const uint32_t crc = Djb(fname);
            ret = AllocateModelClass(crc, numSurfaces);
            ret->m_NumSubsets = numSurfaces;
            
            for (int i=0; i<numSurfaces; ++i)
            {
                ModelClassSubset* modelClassSubset = &ret->m_Subsets[i];
                
                const int numVertices = *(short*)data;
                data += 2;
                const int numIndices = *(short*)data;
                data += 2;
                
                float ns;
                memcpy(&ns, data, 4);
                data += 4;
                
                float ka[3];
                memcpy(ka, data, 12);
                data += 12;
                
                float kd[3];
                memcpy(kd, data, 12);
                data += 12;
                
                float ks[3];
                memcpy(ks, data, 12);
                data += 12;
                
                float ke[3];
                memcpy(ke, data, 12);
                data += 12;
                
                float ni;
                memcpy(&ni, data, 4);
                data += 4;
                
                float d;
                memcpy(&d, data, 4);
                data += 4;

                {
                    Shader* shader = ShaderCreate("obj/Shader/LitWaveFront2");
                    modelClassSubset->m_Material = MaterialCreate(shader, renderContext->m_WhiteTexture);
                    ShaderDestroy(shader);
                }
                
                modelClassSubset->m_Material->ReserveProperties(5);
                int aslot = modelClassSubset->m_Material->SetPropertyType("_MaterialAmbient", Material::MaterialPropertyType::kVec4);
                modelClassSubset->m_Material->SetVector(aslot, ka);
                int dslot = modelClassSubset->m_Material->SetPropertyType("_MaterialDiffuse", Material::MaterialPropertyType::kVec4);
                modelClassSubset->m_Material->SetVector(dslot, kd);
                int sslot = modelClassSubset->m_Material->SetPropertyType("_MaterialSpecular", Material::MaterialPropertyType::kVec4);
                modelClassSubset->m_Material->SetVector(sslot, ks);
                int trslot = modelClassSubset->m_Material->SetPropertyType("_MaterialTransparency", Material::MaterialPropertyType::kFloat);
                modelClassSubset->m_Material->SetFloat(trslot, 1.0f-d);
                int nsslot = modelClassSubset->m_Material->SetPropertyType("_MaterialSpecularPower", Material::MaterialPropertyType::kFloat);
                modelClassSubset->m_Material->SetFloat(nsslot, ns);
                if (d < 1.0f)
                    modelClassSubset->m_Material->m_BlendMode = Material::BlendMode::kBlend;
                else
                    modelClassSubset->m_Material->m_BlendMode = Material::BlendMode::kOpaque;
                
                modelClassSubset->m_Vertices = new SimpleVertex[modelClassSubset->m_NumVertices = numVertices];
                modelClassSubset->m_Indices = new unsigned short[modelClassSubset->m_NumIndices = numIndices];
                
                for (int j=0; j<numVertices; ++j)
                {
                    memcpy(&modelClassSubset->m_Vertices[j].m_Position, data, 12);
                    data += 12;
                    memcpy(&modelClassSubset->m_Vertices[j].m_Normal, data, 12);
                    data += 12;
                    memcpy(&modelClassSubset->m_Vertices[j].m_Uv, data, 8);
                    data += 8;
                    memcpy(&modelClassSubset->m_Vertices[j].m_Color, data, 4);
                    data += 4;
                }
                
                memcpy(modelClassSubset->m_Indices, data, 2*numIndices);
                data += 2*numIndices;
                
                // generate vertex name
                glGenVertexArrays(1, &modelClassSubset->m_VaoName);
                glBindVertexArray(modelClassSubset->m_VaoName);
                
                // generate vertex buffer name
                glGenBuffers(1, &modelClassSubset->m_VertexBufferName);
                glBindBuffer(GL_ARRAY_BUFFER, modelClassSubset->m_VertexBufferName);
                glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex)*modelClassSubset->m_NumVertices, modelClassSubset->m_Vertices, GL_STATIC_DRAW);
                
                glEnableVertexAttribArray(kVertexAttributePosition);
                glVertexAttribPointer(kVertexAttributePosition,
                                      3,                                          // x/y/z
                                      GL_FLOAT,                                   // type
                                      GL_FALSE,                                   // normalize?
                                      sizeof(SimpleVertex),                       // stride
                                      BUFFER_OFFSETOF(SimpleVertex, m_Position)); // offset in buffer data
                
                glEnableVertexAttribArray(kVertexAttributeColor);
                glVertexAttribPointer(kVertexAttributeColor,
                                      4,                                          // rgba
                                      GL_UNSIGNED_BYTE,                           // type
                                      GL_FALSE,                                   // normalize?
                                      sizeof(SimpleVertex),                       // stride
                                      BUFFER_OFFSETOF(SimpleVertex, m_Color));    // offset in buffer data
                
                glEnableVertexAttribArray(kVertexAttributeNormal);
                glVertexAttribPointer(kVertexAttributeNormal,
                                      3,                                          // x/y/z
                                      GL_FLOAT,                                   // type
                                      GL_TRUE,                                    // normalize?
                                      sizeof(SimpleVertex),                       // stride
                                      BUFFER_OFFSETOF(SimpleVertex, m_Normal));   // offset in buffer data
                
                glEnableVertexAttribArray(kVertexAttributeTexCoord);
                glVertexAttribPointer(kVertexAttributeTexCoord,
                                      2,                                          // x/y
                                      GL_FLOAT,                                   // type
                                      GL_FALSE,                                   // normalize?
                                      sizeof(SimpleVertex),                       // stride
                                      BUFFER_OFFSETOF(SimpleVertex, m_Uv));       // offset in buffer data
                
                glGenBuffers(1, &modelClassSubset->m_IndexBufferName);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_IndexBufferName);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_NumIndices*sizeof(unsigned short), modelClassSubset->m_Indices, GL_STATIC_DRAW);
                
                ModelClassSubsetCalcBSphere(modelClassSubset);
            }
            
            free(base);
        }
        
        return ret;
    }
    
    return nullptr;
}

ModelClass* ModelClassManager::AllocateModelClass(uint32_t crc, int numSubsets)
{
    ModelClass* modelClass = AllocateAsset(crc);
    modelClass->m_NumSubsets = numSubsets;
    modelClass->m_Subsets = new ModelClassSubset[numSubsets];
    modelClass->m_RefCount = 0;
    return modelClass;
}

void ModelClassManager::DumpTitle()
{
    Printf("ModelClassManager\n");
}

void ModelClassManager::DumpInternal()
{
    for (int i=0; i<m_NumAssets; ++i)
    {
        ModelClass& modelClass = m_Assets[i];
        Printf("%2d crc 0x%x refCount %d\n", i, m_Crc[i], modelClass.m_RefCount);
    }
}

ModelClassManager* g_ModelClassManager;
static ModelClass* g_CubeModelClass;

// -------------------------------------------------------------------------------------------------
ModelClass* ModelClassAllocateSpawned(int numSubsets)
{
    uint32_t crc = (uint32_t) (UINT_MAX * (rand()/(float)RAND_MAX));
    return g_ModelClassManager->AllocateModelClass(crc, numSubsets);
}

// -------------------------------------------------------------------------------------------------
void ModelClassInit(RenderContext* renderContext)
{
    g_ModelClassManager = new ModelClassManager();
    
    g_CubeModelClass = g_ModelClassManager->AllocateModelClass(kModelClassBuiltinCube, 1);
    g_CubeModelClass->m_RefCount = 1;
    RenderGenerateCubeModelClass(renderContext, g_CubeModelClass);
}

// -------------------------------------------------------------------------------------------------
void ModelClassFini()
{
    ModelClassDestroy(g_CubeModelClass);
    g_CubeModelClass = nullptr;
    
    g_ModelClassManager->Dump();
    g_ModelClassManager->Destroy();
    g_ModelClassManager = nullptr;
}

// -------------------------------------------------------------------------------------------------
ModelClass* ModelClassCreate(RenderContext* renderContext, const char* fname)
{
    const uint32_t crc = Djb(fname);
    const int index = g_ModelClassManager->Find(crc);
    ModelClass* ret;
    
    if (index<0)
        ret = g_ModelClassManager->CreateModelClass(renderContext, fname);
    else
        ret = &g_ModelClassManager->m_Assets[index];
    
    if (ret != nullptr)
    {
        ret->m_Crc = crc;
        ret->m_RefCount++;
    }
    
    if (ret == nullptr)
        return g_CubeModelClass; // what's the worst that could happen?
    
    return ret;
}

// -------------------------------------------------------------------------------------------------
ModelClass* ModelClassFind(uint32_t crc)
{
    const int index = g_ModelClassManager->Find(crc);
    if (index>=0)
        return &g_ModelClassManager->m_Assets[index];
    return nullptr;
}

// -------------------------------------------------------------------------------------------------
void ModelClassDestroy(ModelClass* victim)
{
    if (victim == nullptr)
        return;
    
    if (--victim->m_RefCount == 0)
        g_ModelClassManager->DestroyModelClass(victim);
}
