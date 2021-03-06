// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "slib/Container/LinkyList.h"
#include "Engine/Scene.h"
#include "Render/Render.h"
#include "Render/Material.h"
#include "Render/Model.h"
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
    SceneObject* m_SceneObject;
    ModelClassSubset* m_ModelClassSubset;
    uint32_t m_Key;
};

static void SceneObjectApplyDelta(SceneObject* scene, const Mat4& delta);

void SceneCreate(Scene* scene, int maxSceneObjects)
{
    scene->m_NumObjects = 0;
    scene->m_MaxObjects = maxSceneObjects;
    
    scene->m_SceneObjects = new SceneObject*[maxSceneObjects];
    memset(scene->m_SceneGroups, 0, sizeof scene->m_SceneGroups);
    memset(scene->m_SceneGroupAllocated, 0, sizeof scene->m_SceneGroupAllocated);
    scene->m_SortArray = malloc(Scene::kMaxSubsets*sizeof(SortNode));
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

// -------------------------------------------------------------------------------------------------
void SceneUpdate(Scene* scene)
{
    // reset light count
    scene->m_NumPointLights = 0;
    scene->m_NumConicalLights = 0;
    scene->m_NumCylindricalLights = 0;
    scene->m_NumDirectionalLights = 0;
    
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    scene->m_SortIndex = 0;
    
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
                sceneObject->m_ModelInstance->m_Po = sceneObject->m_LocalToWorld;
            
            // update prev local to world
            sceneObject->m_PrevLocalToWorld = sceneObject->m_LocalToWorld;
        }
        
        sceneObject->m_Flags |= SceneObject::Flags::kUpdatedOnce;
        
        if (sceneObject->m_Type != SceneObjectType::kLight && sceneObject->m_ModelInstance)
        {
            for (int j=0,m=sceneObject->m_ModelInstance->m_ModelClass->m_NumSubsets; j<m; ++j)
            {
                const int write_index = scene->m_SortIndex++;
                ModelClassSubset* modelClassSubset = &sceneObject->m_ModelInstance->m_ModelClass->m_Subsets[j];
                
                sortNodes[write_index].m_SceneObject = sceneObject;
                sortNodes[write_index].m_ModelClassSubset = modelClassSubset;
                sortNodes[write_index].m_Key = RenderModelSubsetGetSortKey(modelClassSubset);
            }
        }
        
        if (sceneObject->m_Type == SceneObjectType::kLight)
        {
            const Light* light = &sceneObject->m_Light;
            if ((sceneObject->m_Flags & SceneObject::kEnabled) == 0)
                continue;
            
            if (light->m_Type == LightType::kPoint)
            {
                if (scene->m_NumPointLights == Light::kMaxLights)
                    continue;
                
                Light* dest = &scene->m_PointLights[scene->m_NumPointLights++];
                *dest = *light;
                
                // transform position
                dest->m_Position = sceneObject->m_LocalToWorld.GetTranslation();
            }
            
            if (light->m_Type == LightType::kConical)
            {
                if (scene->m_NumConicalLights == Light::kMaxLights)
                    continue;
                
                Light* dest = &scene->m_ConicalLights[scene->m_NumConicalLights++];
                *dest = *light;
                
                // transform position
                dest->m_Position = sceneObject->m_LocalToWorld.GetTranslation();
                
                // jiv fixme: it's in world space, shouldn't be
                dest->m_Direction = sceneObject->m_LocalToWorld.GetUp();
            }
            
            if (light->m_Type == LightType::kCylindrical)
            {
                if (scene->m_NumCylindricalLights == Light::kMaxLights)
                    continue;
                
                Light* dest = &scene->m_CylindricalLights[scene->m_NumCylindricalLights++];
                *dest = *light;
                
                // transform position and direction
                dest->m_Position = sceneObject->m_LocalToWorld.GetTranslation();
                dest->m_Direction = light->m_Direction.xyz0() * sceneObject->m_LocalToWorld;
            }
            
            if (light->m_Type == LightType::kDirectional)
            {
                if (scene->m_NumDirectionalLights == Light::kMaxLights)
                    continue;
                
                Light* dest = &scene->m_DirectionalLights[scene->m_NumDirectionalLights++];
                *dest = *light;
            }
        }
    }
    
    auto cmp = [](const void* _ap, const void* _bp)
    {
        const SortNode* a = (SortNode*)_ap;
        const SortNode* b = (SortNode*)_bp;
        const uint32_t keyA = a->m_Key;
        const uint32_t keyB = b->m_Key;
        
        if (keyA < keyB)
            return -1;
        else if (keyA > keyB)
            return 1;
        
        return 0;
    };
    
    qsort(sortNodes, scene->m_SortIndex, sizeof(SortNode), cmp);
}

static void SceneObjectApplyDelta(SceneObject* itr, const Mat4& delta)
{
    while (itr)
    {
        Mat4 mat;
        MatrixMultiply(&mat, itr->m_LocalToWorld, delta);
        itr->m_LocalToWorld = mat;
        
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
        sceneObject->m_Flags = SceneObject::kEnabled;
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
        sceneObject->m_Obb = ToolGenerateObbFromModelClass(sceneObject->m_ModelInstance->m_ModelClass);
        
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
        LightInitialize(&sceneObject->m_Light, lightOptions);
        
        assert(lightOptions.m_Position.m_X[1] < 10000.0f);
        
        sceneObject->m_LocalToWorld.SetTranslation(lightOptions.m_Position.xyz());
        
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
        sceneObject->m_Obb = ToolGenerateObbFromModelClass(sceneObject->m_ModelInstance->m_ModelClass);
    }
    return sceneObject;
}

void SceneLightsUpdate(Scene* scene, RenderContext* renderContext)
{
    // update point lights
    RenderUpdatePointLights(renderContext, scene->m_PointLights, scene->m_NumPointLights);
    RenderUpdateConicalLights(renderContext, scene->m_ConicalLights, scene->m_NumConicalLights);
    RenderUpdateCylindricalLights(renderContext, scene->m_CylindricalLights, scene->m_NumCylindricalLights);
    RenderUpdateDirectionalLights(renderContext, scene->m_DirectionalLights, scene->m_NumDirectionalLights);
}

// -------------------------------------------------------------------------------------------------
void SceneDraw(Scene* scene, RenderContext* renderContext)
{
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    for (int i=0,n=scene->m_SortIndex; i<n; ++i)
    {
        const SceneObject* sceneObject = sortNodes[i].m_SceneObject;
        if ((sceneObject->m_Flags & SceneObject::kEnabled) == 0)
            continue;
        
        const ModelClassSubset* modelClassSubset = sortNodes[i].m_ModelClassSubset;
        RenderDrawModelSubset(renderContext, sceneObject->m_ModelInstance->m_Po, modelClassSubset);
    }
}

// -------------------------------------------------------------------------------------------------
void SceneDraw(Scene* scene, RenderContext* renderContext, int groupId)
{
    if (!scene->m_SceneGroupAllocated[groupId])
        return;
    
    SortNode* sortNodes = (SortNode*) scene->m_SortArray;
    for (int i=0,n=scene->m_SortIndex; i<n; ++i)
    {
        const SceneObject* sceneObject = sortNodes[i].m_SceneObject;
        SceneObject* itr = scene->m_SceneGroups[groupId];
        
        bool found = itr == sceneObject;
        while (!found && itr)
            found = (itr=LinkyListNext(itr)) == sceneObject;
        
        if (!found)
            continue;
        
        if ((itr->m_Flags & SceneObject::kEnabled) == 0)
            continue;
        
        const ModelClassSubset* modelClassSubset = sortNodes[i].m_ModelClassSubset;
        RenderDrawModelSubset(renderContext, sceneObject->m_ModelInstance->m_Po, modelClassSubset);
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
        
        ModelInstanceDestroy(sceneObject->m_ModelInstance);
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
    if (sceneObject)
    {
        Mat4 axes;
        MatrixMakeIdentity(&axes);
        axes.SetRot(sceneObject->m_Obb.m_Axes);
        
        SceneDrawObb(scene, renderContext, sceneObject, sceneObject->m_LocalToWorld * axes);
    }
}

// SceneDrawObb
void SceneDrawObb(Scene* scene, RenderContext* renderContext, const SceneObject* sceneObject, const Mat4& _localToWorld)
{
    if ((sceneObject->m_Flags & SceneObject::kEnabled) == 0)
        return;
    
    Mat4 localToWorld = _localToWorld;
    
    if (sceneObject->m_Type == SceneObjectType::kLight)
    {
        const Light& light = sceneObject->m_Light;
        switch (light.m_Type)
        {
            default:
            {
                break;
            }
            case LightType::kPoint:
            {
                const float range = light.m_Range;
                localToWorld.Scale(Vec3(range, range, range));
                break;
            }
            case LightType::kConical:
            {
                const float range = light.m_Range;
                const float half_range = light.m_Range * 0.5f;
                Vec3 offset = half_range * localToWorld.GetUp();
                localToWorld.AddTranslation(offset);
                
                localToWorld.Scale(Vec3(half_range, range, half_range));
                break;
            }
            case LightType::kCylindrical:
            {
                localToWorld.Scale(Vec3(2.0f*light.m_OrthogonalRange, 2.0f*light.m_Range, light.m_Range));
                break;
            }
        }
    }

    const ModelClass* cubeModelClass = ModelClassFind(kModelClassBuiltinCube);
    RenderDrawModelSubset(renderContext, localToWorld, &cubeModelClass->m_Subsets[0]);
}
