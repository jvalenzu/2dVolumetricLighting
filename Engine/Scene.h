// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <stdint.h>

#include "slib/Container/FixedVector.h"
#include "Engine/Matrix.h"
#include "Engine/Obb.h"
#include "Engine/Light.h"

struct SimpleModel;
struct RenderContext;
struct Material;
struct SpriteOptions;
struct LightOptions;
struct Texture;

enum SceneObjectType : uint32_t
{
    kCube,
    kSprite,
    kLight,
    kEmpty
};

struct SceneObject
{
    enum Flags : uint32_t
    {
        kDirty = 1,
        kUpdatedOnce = 2,
        kEnabled = 4
    };
    Mat4 m_PrevLocalToWorld;
    Mat4 m_LocalToWorld;
    SimpleModel* m_ModelInstance;
    Obb m_Obb;
    Light m_Light;
    Texture* m_Shadow1dMap;
    uint32_t m_Flags;
    const char* m_DebugName;
    int m_SceneIndex;
    SceneObjectType m_Type;
    
    SceneObject* m_Parent;
    SceneObject* m_Child;
    SceneObject* m_SiblingNext;
    
    // group
    SceneObject* m_Next;
    SceneObject* m_Prev;
};

struct Scene
{
    int m_NumObjects;
    int m_MaxObjects;
    SceneObject** m_SceneObjects;
    void* m_SortArray;
    
    enum { kGroupMax = 16 };
    SceneObject* m_SceneGroups[kGroupMax];
    bool m_SceneGroupAllocated[kGroupMax];
    
    Light m_PointLights[Light::kMaxLights];
    int m_NumPointLights;
    
    Light m_ConicalLights[Light::kMaxLights];
    int m_NumConicalLights;
    
    Light m_CylindricalLights[Light::kMaxLights];
    int m_NumCylindricalLights;
    
    Light m_DirectionalLights[Light::kMaxLights];
    int m_NumDirectionalLights;
};

void         SceneCreate(Scene* scene, int maxSceneObjects);
void         SceneDestroy(Scene* scene);
void         SceneUpdate(Scene* scene);
void         SceneDraw(Scene* scene, RenderContext* renderContext);
void         SceneDraw(Scene* scene, RenderContext* renderContext, int groupId);
void         SceneDrawObb(Scene* scene, RenderContext* renderContext, const SceneObject* sceneObject);

void         SceneLightsUpdate(Scene* scene, RenderContext* renderContext);

int          SceneGroupCreate(Scene* scene);
void         SceneGroupDestroy(Scene* scene, int index);
void         SceneGroupAdd(Scene* scene, int index, SceneObject* sceneObject);
void         SceneGroupRemove(Scene* scene, SceneObject* sceneObject);

void         SceneGroupAddChild(SceneObject* parent, SceneObject* child);
void         SceneGroupRemoveChild(SceneObject* parent, SceneObject* child);

// SceneObject creation helper functions.  Things that require models/GL allocations also require a RenderContext, although most don't use them.
SceneObject* SceneCreateLight(Scene* scene, const LightOptions& lightOptions);
SceneObject* SceneCreateSprite(Scene* scene, RenderContext* renderContext, Material* material, const SpriteOptions& spriteOptions);
SceneObject* SceneCreateSpriteFromFile(Scene* scene, RenderContext* renderContext, const char* fname);
SceneObject* SceneCreateSpriteFromFile(Scene* scene, RenderContext* renderContext, const char* fname, const SpriteOptions& spriteOptions);
SceneObject* SceneCreateSpriteFromRenderTexture(Scene* scene, int width, int height);
SceneObject* SceneCreateSpriteFromMaterial(Scene* scene, RenderContext* renderContext, Material* material);
SceneObject* SceneCreateCube(Scene* scene, RenderContext* renderContext, Material* material);
SceneObject* SceneCreateEmpty(Scene* scene);

// SceneObjectGetLight
inline Light* SceneObjectGetLight(SceneObject* sceneObject)
{
    if (sceneObject->m_Type == SceneObjectType::kLight)
        return &sceneObject->m_Light;
    return nullptr;
}


// SceneObjectDestroy
void         SceneObjectDestroy(Scene* scene, SceneObject* sceneObject);

// SceneGetSceneObjectsByType
int          SceneGetSceneObjectsByType(SceneObject** dest, int size, Scene* scene, SceneObjectType type);

template <typename N>
inline void SceneGetSceneObjectsByType(N* dest, Scene* scene, SceneObjectType type)
{
    int ret = SceneGetSceneObjectsByType(dest->Data(), N::kMaxSize, scene, type);
    dest->SetCount(ret);
}

inline void SceneSetEnabled(SceneObject* sceneObject, bool value)
{
    if (sceneObject != nullptr)
    {
        sceneObject->m_Flags &= ~SceneObject::kEnabled;
        if (value)
            sceneObject->m_Flags |= SceneObject::kEnabled;
    }
}

inline void SceneSetEnabledRecursive(SceneObject* sceneObject, bool value)
{
    SceneSetEnabled(sceneObject, value);
    if (sceneObject)
        SceneSetEnabledRecursive(sceneObject->m_Child, value);
}

inline bool SceneGetEnabled(const SceneObject* sceneObject)
{
    if (sceneObject && sceneObject->m_Flags & SceneObject::kEnabled)
        return true;
    return false;
}
