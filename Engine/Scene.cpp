// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

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

// 1d texture dimension
#define kShadowMapSize 1024

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

uint32_t SceneObjectGetSortKey(const SceneObject* sceneObject)
{
    if (sceneObject->m_Type == SceneObjectType::kLight)
        return 0;
    
    const SimpleModel* simpleModel = sceneObject->m_ModelInstance;
    
    uint32_t materialIdBits = 0;
    uint32_t blendModeBit = 0;
    uint32_t positionBits = 0;
    
    Vec3 pos = Mat4GetTranslation(simpleModel->m_Po);
    
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
    // reset light count
    scene->m_NumPointLights = 0;
    scene->m_NumConicalLights = 0;
    scene->m_NumCylindricalLights = 0;
    
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    
    for (int i=0,n=scene->m_NumObjects; i<n; ++i)
    {
        SceneObject* sceneObject = scene->m_SceneObjects[i];
        if (sceneObject->m_Flags & SceneObject::Flags::kDirty)
        {
            // update children
            if (sceneObject->m_Child && sceneObject->m_Flags&SceneObject::Flags::kUpdatedOnce)
            {
                Mat4 delta;
                MatrixCalculateDelta(&delta, sceneObject->m_LocalToWorld, sceneObject->m_PrevLocalToWorld);
                
                SceneObjectApplyDelta(sceneObject->m_Child, delta);
            }
            
            // clear flag
            sceneObject->m_Flags &= ~SceneObject::Flags::kDirty;
            
            // update position and orientation
            if (sceneObject->m_ModelInstance)
                MatrixCopy(&sceneObject->m_ModelInstance->m_Po, sceneObject->m_LocalToWorld);
            
            // update prev local to world
            MatrixCopy(&sceneObject->m_PrevLocalToWorld, sceneObject->m_LocalToWorld);
        }
        
        sceneObject->m_Flags |= SceneObject::Flags::kUpdatedOnce;
        
        sortNodes[i].m_Index = i;
        sortNodes[i].m_Key = SceneObjectGetSortKey(sceneObject);
        
        if (sceneObject->m_Type == SceneObjectType::kLight)
        {
            Light* light = (Light*) &sceneObject->m_LightData[0];
            if (light->m_Type == LightType::kPoint)
            {
                if (scene->m_NumPointLights == Light::kMaxLights)
                    continue;
                
                PointLight* source = (PointLight*) light;
                PointLight* dest = &scene->m_PointLights[scene->m_NumPointLights++];
                *dest = *source;
                
                // transform position
                dest->m_Position = Mat4GetTranslation(sceneObject->m_LocalToWorld);
            }
            
            if (light->m_Type == LightType::kConical)
            {
                if (scene->m_NumConicalLights == Light::kMaxLights)
                    continue;
                
                ConicalLight* source = (ConicalLight*) light;
                ConicalLight* dest = &scene->m_ConicalLights[scene->m_NumConicalLights++];
                *dest = *source;
                
                // transform position
                dest->m_Position = Mat4GetTranslation(sceneObject->m_LocalToWorld);
                
                // jiv fixme: it's in world space, shouldn't be
                dest->m_Direction = Mat4GetRight(sceneObject->m_LocalToWorld);
            }
            
            if (light->m_Type == LightType::kCylindrical)
            {
                if (scene->m_NumCylindricalLights == Light::kMaxLights)
                    continue;
                
                CylindricalLight* source = (CylindricalLight*) light;
                CylindricalLight* dest = &scene->m_CylindricalLights[scene->m_NumCylindricalLights++];
                *dest = *source;
                
                // transform position
                dest->m_Position = Mat4GetTranslation(sceneObject->m_LocalToWorld);
                
                // jiv fixme: it's in world space, shouldn't be
                // dest->m_Direction = Mat4GetRight(sceneObject->m_LocalToWorld);
            }            
        }
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
        
        sceneObject->m_Shadow1dMap = nullptr;
        
        MatrixMakeIdentity(&sceneObject->m_PrevLocalToWorld);
        MatrixMakeIdentity(&sceneObject->m_LocalToWorld);
    }
    
    return sceneObject;
}

SceneObject* SceneCreateSprite(Scene* scene, RenderContext* renderContext, Material* material, const SpriteOptions& spriteOptions)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kSprite);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = RenderGenerateSprite(renderContext, spriteOptions, material);
        sceneObject->m_Obb = ToolGenerateObbFromSimpleModel(sceneObject->m_ModelInstance);
        
        sceneObject->m_LocalToWorld.m_X[0] *= spriteOptions.m_Scale.m_X[0];
        sceneObject->m_LocalToWorld.m_Y[1] *= spriteOptions.m_Scale.m_X[1];
    }
    
    return sceneObject;
}

SceneObject* SceneCreateLight(Scene* scene, const LightOptions& lightOptions)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kLight);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = nullptr;
        LightInitialize((Light*)&sceneObject->m_LightData[0], lightOptions);
        
        Mat4ApplyTranslation(&sceneObject->m_LocalToWorld, lightOptions.m_Position.xyz());
        
        // jiv fixme: make some lights not drive shadows
        sceneObject->m_Shadow1dMap = TextureCreateRenderTexture(kShadowMapSize, 1, 0, Texture::RenderTextureFormat::kFloat);
        
        LightGenerateObb(&sceneObject->m_Obb, lightOptions);
    }
    
    return sceneObject;
}

SceneObject* SceneCreateSpriteFromFile(Scene* scene, RenderContext* renderContext, const char* fname)
{
    Texture* texture = TextureCreateFromFile(fname);
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    material->ReserveProperties(1);
    material->SetPropertyType("TintColor", Material::MaterialPropertyType::kVec4);
    
    TextureDestroy(texture); // material will own reference
    SpriteOptions spriteOptions;
    return SceneCreateSprite(scene, renderContext, material, spriteOptions);
}


SceneObject* SceneCreateSpriteFromFile(Scene* scene, RenderContext* renderContext, const char* fname, const SpriteOptions& spriteOptions)
{
    Texture* texture = TextureCreateFromFile(fname);
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    material->ReserveProperties(1);
    material->SetPropertyType("TintColor", Material::MaterialPropertyType::kVec4);
    
    TextureDestroy(texture); // material will own reference
    SceneObject* ret = SceneCreateSprite(scene, renderContext, material, spriteOptions);
    return ret;
}

SceneObject* SceneCreateSpriteFromRenderTexture(Scene* scene, RenderContext* renderContext, int width, int height)
{
    Texture* texture = TextureCreateRenderTexture(width, height, 0); // jiv fixme
    Material* material = MaterialCreate(g_SimpleTransparentShader, texture);
    SpriteOptions spriteOptions;
    return SceneCreateSprite(scene, renderContext, material, spriteOptions);
}

SceneObject* SceneCreateCube(Scene* scene, RenderContext* renderContext, Material* material)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kCube);
    if (sceneObject)
    {
        sceneObject->m_ModelInstance = RenderGenerateCube(renderContext, 1.0f);
        sceneObject->m_Obb = ToolGenerateObbFromSimpleModel(sceneObject->m_ModelInstance);
    }
    return sceneObject;
}

void SceneLightsUpdate(Scene* scene, RenderContext* renderContext)
{
    // update point lights
    RenderUpdatePointLights(renderContext, scene->m_PointLights, scene->m_NumPointLights);
    RenderUpdateConicalLights(renderContext, scene->m_ConicalLights, scene->m_NumConicalLights);
    RenderUpdateCylindricalLights(renderContext, scene->m_CylindricalLights, scene->m_NumCylindricalLights);
}

void SceneDraw(Scene* scene, RenderContext* renderContext)
{
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    for (int i=0,n=scene->m_NumObjects; i<n; ++i)
    {
        int index = sortNodes[i].m_Index;
        SceneObject* sceneObject = scene->m_SceneObjects[index];
        if (sceneObject->m_ModelInstance == nullptr)
            continue;
        
        Vec3 pos = Mat4GetTranslation(sceneObject->m_ModelInstance->m_Po);
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
        
        Vec3 pos = Mat4GetTranslation(sceneObject->m_ModelInstance->m_Po);
        RenderDrawModel(renderContext, sceneObject->m_ModelInstance);
    }
}

// 
//  ____                                       
// /\  _`\
// \ \ \L\_\   _ __    ___    __  __   _____
//  \ \ \L_L  /\`'__\ / __`\ /\ \/\ \ /\ '__`\
//   \ \ \/, \\ \ \/ /\ \L\ \\ \ \_\ \\ \ \L\ \
//    \ \____/ \ \_\ \ \____/ \ \____/ \ \ ,__/
//     \/___/   \/_/  \/___/   \/___/   \ \ \/ 
//                                       \ \_\
//                                        \/_/
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

void SceneObjectAssert(SceneObject* sceneObject)
{
    if (sceneObject->m_Child != nullptr)
        assert(sceneObject->m_Child->m_Parent == sceneObject);
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
        // remove children.  Don't use SceneGroupRemoveChild because we're going to have to iterate anyway.
        {
            SceneObject* itr = sceneObject->m_Child;
            while (itr)
            {
                SceneObject* prev = itr;
                
                itr->m_Parent = sceneObject->m_Parent;
                itr = itr->m_SiblingNext;
                
                // if parent is null, we're not really parented anymore, so remove sibling links
                if (sceneObject->m_Parent == nullptr)
                    prev->m_SiblingNext = nullptr;
            }
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
        
        TextureDestroy(sceneObject->m_Shadow1dMap);
        sceneObject->m_Shadow1dMap = nullptr;
        
#ifndef NDEBUG
        memset(sceneObject, 0xff, sizeof *sceneObject);
#endif
        
        delete sceneObject;
    }
}

// SceneCreateEmpty
SceneObject* SceneCreateEmpty(Scene* scene)
{
    SceneObject* sceneObject = SceneObjectAllocate(scene, SceneObjectType::kEmpty);
    return sceneObject;
}

// SceneGetSceneObjectsByType
int SceneGetSceneObjectsByType(SceneObject** dest, int size, Scene* scene, SceneObjectType type)
{
    SceneObject** first = &dest[0];
    SceneObject** write = &dest[0];
    SceneObject** last  = &dest[size];
    
    for (int i=0,n=scene->m_NumObjects; write<last && i<n; ++i)
    {
        SceneObject* sceneObject = scene->m_SceneObjects[i];
        if (sceneObject->m_Type == type)
            *write++ = sceneObject;
    }
    
    return (int) (write - first);
}

// SceneDrawObb
void SceneDrawObb(Scene* scene, RenderContext* renderContext, const SceneObject* sceneObject)
{
    Mat4 localToWorld;
    MatrixMakeZero(&localToWorld);
    
    localToWorld.SetRot(sceneObject->m_Obb.m_Axes);
    localToWorld.SetTranslation(sceneObject->m_LocalToWorld.GetTranslation());
    
    const float aspectRatio = (float) renderContext->m_Width/renderContext->m_Height;
    
    if (sceneObject->m_Type == SceneObjectType::kLight)
    {
        const Light* light = (Light*) &sceneObject->m_LightData[0];
        switch (light->m_Type)
        {
            default:
            {
                break;
            }
            case LightType::kPoint:
            {
                const PointLight* pointLight = (PointLight*) light;
                const float range = pointLight->m_Range;
                MatrixScaleInsitu(&localToWorld, Vec3(range*aspectRatio, range, range));
                break;
            }
            case LightType::kConical:
            {
                const ConicalLight* conicalLight = (ConicalLight*) light;
                const float range = conicalLight->m_Range;
                MatrixScaleInsitu(&localToWorld, Vec3(range*aspectRatio, range, range));
                break;
            }
        }
    }
    
    RenderDrawModel(renderContext, renderContext->m_CubeModel, localToWorld);
}
