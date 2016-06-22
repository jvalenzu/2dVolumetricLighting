#include "Engine/Container/LinkyList.h"
#include "Engine/Scene.h"
#include "Render/Render.h"
#include "Render/Material.h"
#include "Tool/Utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct SortNode
{
    int32_t m_Index;
    uint32_t m_Key;
};

static int s_SortNodeCmp(const void* _ap, const void* _bp)
{
    SortNode* a = (SortNode*)_ap;
    SortNode* b = (SortNode*)_bp;
    uint32_t keyA = a->m_Key;
    uint32_t keyB = b->m_Key;
    
    if (keyA < keyB)
        return -1;
    else if (keyA > keyB)
        return 1;
    
    return 0;
}

static void SceneObjectApplyDelta(SceneObject* scene, const Mat4& delta);
static void LightInitialize(Light* light, const LightOptions& lightOptions)
{
    
}

uint32_t SceneObjectGetSortKey(const SceneObject* sceneObject)
{
    const SimpleModel* simpleModel = sceneObject->m_ModelInstance;
    
    uint32_t materialIdBits = 0;
    uint32_t blendModeBit = 0;
    uint32_t positionBits = 0;
    
    Vec3 pos = Mat4GetTranslation3(simpleModel->m_Po);
    
    if (simpleModel->m_Material->m_BlendMode != Material::BlendMode::kOpaque)
    {
        blendModeBit = 0x80000000;
        positionBits = ((unsigned int) (pos.m_X[2] * 128.0f)) & 0x7fff;
    }
    else
    {
        uint32_t textureId = 0;
        textureId ^= simpleModel->m_Material->m_Texture->m_TextureId;
        textureId = ((textureId>>16) ^ (textureId&0xffff))&0xffff;
        
        materialIdBits = textureId<<15;
    }
    
    uint32_t key = 0;
    key = blendModeBit | positionBits | materialIdBits;
    return key;
}

void SceneCreate(Scene* scene, int maxSceneObjects)
{
    scene->m_NumObjects = 0;
    scene->m_MaxObjects = maxSceneObjects;
    
    scene->m_SceneObjects = new SceneObject*[maxSceneObjects];
    memset(scene->m_SceneGroups, 0, sizeof scene->m_SceneGroups);
    memset(scene->m_SceneGroupAllocated, 0, sizeof scene->m_SceneGroupAllocated);
    
    scene->m_SortArray = malloc(maxSceneObjects*sizeof(SortNode));
}

void SceneDestroy(Scene* scene)
{
    for (int i=0; i<scene->m_NumObjects; ++i)
    {
        SceneObjectDestroy(scene, scene->m_SceneObjects[i]);
        scene->m_SceneObjects[i] = nullptr;
    }
    delete[] scene->m_SceneObjects;
}

void SceneUpdate(Scene* scene)
{
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    
    for (int i=0,n=scene->m_NumObjects; i<n; ++i)
    {
        SceneObject* sceneObject = scene->m_SceneObjects[i];
        if (sceneObject->m_Flags & SceneObject::Flags::kDirty)
        {
            // update children
            if (sceneObject->m_Child)
            {
                Mat4 delta;
                MatrixCalculateDelta(&delta, sceneObject->m_LocalToWorld, sceneObject->m_PrevLocalToWorld);
                
                SceneObjectApplyDelta(sceneObject->m_Child, delta);
            }
            
            // clear flag
            sceneObject->m_Flags &= ~SceneObject::Flags::kDirty;
            
            // update position and orientation
            SimpleModel* modelInstance = sceneObject->m_ModelInstance;
            MatrixCopy(&modelInstance->m_Po, sceneObject->m_LocalToWorld);
            
            // update prev local to world
            MatrixCopy(&sceneObject->m_PrevLocalToWorld, sceneObject->m_LocalToWorld);
        }
        
        sortNodes[i].m_Index = i;
        sortNodes[i].m_Key = SceneObjectGetSortKey(sceneObject);
    }
    
    qsort(sortNodes, scene->m_NumObjects, sizeof(SortNode), s_SortNodeCmp);
}

static void SceneObjectApplyDelta(SceneObject* scene, const Mat4& delta)
{
    SceneObject* itr = scene;
    while (itr)
    {
        Mat4 mat;
        MatrixMultiply(&mat, itr->m_LocalToWorld, delta);
        MatrixCopy(&itr->m_LocalToWorld, mat);
        
        itr->m_Flags |= SceneObject::Flags::kDirty;
        
        itr = itr->m_SiblingNext;
    }
}

static SceneObject* SceneObjectAllocate(Scene* scene, SceneObjectType type)
{
    if (scene->m_NumObjects >= scene->m_MaxObjects)
        return nullptr;
    
    int sceneIndex = scene->m_NumObjects++;
    SceneObject* sceneObject = scene->m_SceneObjects[sceneIndex] = new SceneObject();
    if (sceneObject)
    {
        sceneObject->m_Type = type;
        sceneObject->m_SceneIndex = sceneIndex;
        sceneObject->m_Flags = 0;
        sceneObject->m_DebugName = nullptr;
        sceneObject->m_Next = nullptr;
        sceneObject->m_Prev = nullptr;
        
        sceneObject->m_Parent = nullptr;
        sceneObject->m_Child = nullptr;
        sceneObject->m_SiblingNext = nullptr;
        
        MatrixMakeIdentity(&sceneObject->m_PrevLocalToWorld);
        MatrixMakeIdentity(&sceneObject->m_LocalToWorld);
    }
    
    return sceneObject;
}

SceneObject* SceneCreateSprite(Scene* scene, Material* material, const SpriteOptions& spriteOptions)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kSprite);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = RenderGenerateSprite(spriteOptions, material);
        sceneObject->m_Obb = ToolGenerateObbFromSimpleModel(sceneObject->m_ModelInstance);
    }
    
    return sceneObject;
}

SceneObject* SceneCreateLight(Scene* scene, const LightOptions& lightOptions)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kLight);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = nullptr;
        LightInitialize(&sceneObject->m_Light, lightOptions);
    }
    
    return sceneObject;
}

SceneObject* SceneCreateSpriteFromFile(Scene* scene, const char* fname)
{
    Texture* texture = TextureCreateFromFile(fname);
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    TextureDestroy(texture); // material will own reference
    SpriteOptions spriteOptions;
    return SceneCreateSprite(scene, material, spriteOptions);
}


SceneObject* SceneCreateSpriteFromFile(Scene* scene, const char* fname, const SpriteOptions& spriteOptions)
{
    Texture* texture = TextureCreateFromFile(fname);
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    TextureDestroy(texture); // material will own reference
    return SceneCreateSprite(scene, material, spriteOptions);
}

SceneObject* SceneCreateSpriteFromRenderTexture(Scene* scene, int width, int height)
{
    Texture* texture = TextureCreateRenderTexture(width, height, 0); // jiv fixme
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    SpriteOptions spriteOptions;
    return SceneCreateSprite(scene, material, spriteOptions);
}

SceneObject* SceneCreateCube(Scene* scene, Material* material)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kCube);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = RenderGenerateCube(1.0f);
        sceneObject->m_Obb = ToolGenerateObbFromSimpleModel(sceneObject->m_ModelInstance);
    }
    return sceneObject;
}

void SceneDraw(Scene* scene, RenderContext* renderContext)
{
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    for (int i=0,n=scene->m_NumObjects; i<n; ++i)
    {
        int index = sortNodes[i].m_Index;
        SceneObject* sceneObject = scene->m_SceneObjects[index];
        
        Vec3 pos = Mat4GetTranslation3(sceneObject->m_ModelInstance->m_Po);
        RenderDrawModel(renderContext, sceneObject->m_ModelInstance);
    }
}

void SceneDraw(Scene* scene, RenderContext* renderContext, int groupId)
{
    if (!scene->m_SceneGroupAllocated[groupId])
        return;
    
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    for (int i=0,n=scene->m_NumObjects; i<n; ++i)
    {
        int index = sortNodes[i].m_Index;
        SceneObject* sceneObject = scene->m_SceneObjects[index];
        
        SceneObject* itr = scene->m_SceneGroups[groupId];
        bool found = itr == sceneObject;
        while (!found && itr)
            found = (itr=LinkyListNext(itr)) == sceneObject;
        
        if (!found)
            continue;
        
        Vec3 pos = Mat4GetTranslation3(sceneObject->m_ModelInstance->m_Po);
        RenderDrawModel(renderContext, sceneObject->m_ModelInstance);
    }
}

int SceneGroupCreate(Scene* scene)
{
    for (int i=0; i<Scene::kGroupMax; ++i)
    {
        if (scene->m_SceneGroupAllocated[i] == false)
        {
            scene->m_SceneGroupAllocated[i] = true;
            return i;
        }
    }
    return -1;
}

void SceneGroupDestroy(Scene* scene, int index)
{
    if (scene->m_SceneGroupAllocated[index])
    {
        scene->m_SceneGroupAllocated[index] = false;
        SceneObject* itr = scene->m_SceneGroups[index];
        while (itr)
        {
            SceneObject* next = itr->m_Next;
            
            next->m_Prev = nullptr;
            itr->m_Next = nullptr;
            
            itr = next;
        }
    }
}

void SceneGroupAdd(Scene* scene, int index, SceneObject* sceneObject)
{
    if (scene->m_SceneGroupAllocated[index])
        LinkyListAdd(scene->m_SceneGroups[index], sceneObject);
}

void SceneGroupRemove(Scene* scene, int index, SceneObject* sceneObject)
{
    if (scene->m_SceneGroupAllocated[index])
        LinkyListRemove(scene->m_SceneGroups[index], sceneObject);
}

void SceneGroupAddChild(SceneObject* parent, SceneObject* child)
{
    if (child->m_Parent != nullptr)
        SceneGroupRemoveChild(parent, child);
        
    child->m_Parent = parent;

    if (parent->m_Child == nullptr)
    {
        parent->m_Child = child;
        return;
    }

    SceneObject* itr = parent->m_Child;
    while (itr->m_SiblingNext != nullptr)
        itr = itr->m_SiblingNext;
    itr->m_SiblingNext = child;
}

void SceneGroupRemoveChild(SceneObject* parent, SceneObject* child)
{
    if (!child || !child->m_Parent || child->m_Parent != parent)
        return;
    
    child->m_Parent = nullptr;
    
    if (parent->m_Child == child)
    {
        parent->m_Child = child->m_SiblingNext;
        child->m_SiblingNext = nullptr;
        return;
    }
    
    SceneObject* itr = parent->m_Child;
    while (itr && itr->m_SiblingNext != child)
        itr = itr->m_SiblingNext;
    
    if (itr->m_SiblingNext == child)
    {
        itr->m_SiblingNext = child->m_SiblingNext;
        child->m_SiblingNext = nullptr;
        return;
    }
}

// SceneObjectDestroy
void SceneObjectDestroy(Scene* scene, SceneObject* sceneObject)
{
    if (sceneObject != nullptr)
    {
        if (sceneObject->m_Child)
        {
            SceneGroupRemoveChild(sceneObject, sceneObject->m_Child);
            sceneObject->m_Child = nullptr;
        }
        
        if (sceneObject->m_Parent)
        {
            SceneGroupRemoveChild(sceneObject->m_Parent, sceneObject);
            sceneObject->m_Parent = nullptr;
        }
        
        SimpleModelDestroy(sceneObject->m_ModelInstance);
        sceneObject->m_ModelInstance = nullptr;
        
        for (int i=0; i<Scene::kGroupMax; ++i)
            SceneGroupRemove(scene, i, sceneObject);
        
        scene->m_SceneObjects[sceneObject->m_SceneIndex] = scene->m_SceneObjects[--scene->m_NumObjects];
        scene->m_SceneObjects[sceneObject->m_SceneIndex]->m_SceneIndex = sceneObject->m_SceneIndex;
        scene->m_SceneObjects[scene->m_NumObjects] = nullptr;

#ifndef NDEBUG
        memset(sceneObject, 0xff, sizeof *sceneObject);
#endif
        
        delete sceneObject;
    }
}
