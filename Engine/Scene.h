#pragma once

#include <stdint.h>

#include "Engine/Matrix.h"
#include "Engine/Obb.h"

struct SimpleModel;
struct RenderContext;
struct Material;
struct SpriteOptions;
struct LightOptions;

enum SceneObjectType : uint32_t
{
    kCube,
    kSprite,
    kLight
};

enum LightType : uint32_t
{
    kDirectional,
    kPoint,
    kConical,
    kCylindrical
};

struct Light
{
    LightType m_Type;
    Vec4 m_Position;
    Vec4 m_Direction;
    Vec4 m_Color;
};

struct SceneObject
{
    enum Flags : uint32_t
    {
        kDirty = 1
    };
    Mat4 m_PrevLocalToWorld;
    Mat4 m_LocalToWorld;
    SimpleModel* m_ModelInstance;
    Obb m_Obb;
    Light m_Light;
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
};

void         SceneCreate(Scene* scene, int maxSceneObjects);
void         SceneDestroy(Scene* scene);
void         SceneUpdate(Scene* scene);
void         SceneDraw(Scene* scene, RenderContext* renderContext);
void         SceneDraw(Scene* scene, RenderContext* renderContext, int groupId);

int          SceneGroupCreate(Scene* scene);
void         SceneGroupDestroy(Scene* scene, int index);
void         SceneGroupAdd(Scene* scene, int index, SceneObject* sceneObject);
void         SceneGroupRemove(Scene* scene, SceneObject* sceneObject);

void         SceneGroupAddChild(SceneObject* parent, SceneObject* child);
void         SceneGroupRemoveChild(SceneObject* parent, SceneObject* child);

SceneObject* SceneCreateLight(Scene* scene, const LightOptions& lightOptions);
SceneObject* SceneCreateSprite(Scene* scene, Material* material, const SpriteOptions& spriteOptions);
SceneObject* SceneCreateSpriteFromFile(Scene* scene, const char* fname);
SceneObject* SceneCreateSpriteFromFile(Scene* scene, const char* fname, const SpriteOptions& spriteOptions);
SceneObject* SceneCreateSpriteFromRenderTexture(Scene* scene, int width, int height);
SceneObject* SceneCreateSpriteFromMaterial(Scene* scene, Material* material);
SceneObject* SceneCreateCube(Scene* scene, Material* material);

void         SceneObjectDestroy(Scene* scene, SceneObject* sceneObject);
