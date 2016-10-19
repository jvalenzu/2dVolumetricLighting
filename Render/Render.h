// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "slib/Container/FixedVector.h"
#include "Render/GL.h"
#include "Engine/Matrix.h"
#include "Render/Material.h"
#include "Engine/Light.h"

#include <assert.h>

struct SimpleModel;
struct Material;
struct GLFWwindow;
struct PostEffect;
struct Texture;
struct Shader;

struct RenderContext
{
    const Material* m_CachedMaterial;
    Mat4 m_Projection;
    Mat4 m_Camera;
    Mat4 m_View;
    int m_Width;
    int m_Height;
    GLFWwindow* m_Window;
    float m_LastTime;
    int m_FrameCount;
    int m_FrameRollover;
    int m_LightIndex;
    
#if USE_CL
    cl_context m_Context;
    cl_command_queue m_CommandQueue;
#endif
    
    GLuint m_QuadVertexArrayId;
    GLuint m_QuadVertexBufferId;
    
    GLuint m_FrameBufferIds[2];
    GLuint m_FrameBufferColorIds[2];
    
    PostEffect** m_PostEffects;
    int m_NumPostEffects;
    int m_MaxNumPostEffects;
    
    Shader* m_ReplacementShader;
    
    Vec3 m_ClearColor;
    
    GLuint m_PointLightUbo;
    GLuint m_ConicalLightUbo;
    GLuint m_CylindricalLightUbo;
    GLuint m_DirectionalLightUbo;
    
    uint32_t m_PointLightMask;
    uint32_t m_CylindricalLightMask;
    uint32_t m_ConicalLightMask;
    uint32_t m_NumDirectionalLights;
    
    Texture* m_WhiteTexture;
    
    SimpleModel* m_CubeModel;
    
    FixedVector<Material::MaterialProperty, 32> m_MaterialProperties;
};

struct RenderOptions
{
    enum CameraType
    {
        kPerspective,
        kOrthographic
    };
    
    CameraType m_CameraType;
    int m_Width;
    int m_Height;
};

struct DebugRenderContext
{
    enum
    {
        kMaxLines = 1024
    };
    
    struct Line
    {
        float m_StartX, m_StartY;
        float m_EndX, m_EndY;
    };
    Line m_Lines;
    int m_NumLines;

    GLuint m_VertexBufferName;
    GLuint m_IndexBufferName;
    
    int GetNumIndices() const
    {
        return 4*m_NumLines;
    }
    
    void Clear()
    {
        m_NumLines = 0;
    }
    
    DebugRenderContext()
    {
        Clear();
    }
};

struct Material;

struct SimpleVertex
{
    GLfloat m_Position[3];
    GLfloat m_Normal[3];
    GLfloat m_Uv[2];
    uint32_t m_Color;
};

struct SimpleModel
{
    Mat4 m_Po;
    SimpleVertex* m_Vertices;
    int32_t m_NumVertices;
    
    unsigned short* m_Indices;
    int m_NumIndices;
    
    Material* m_Material;
    
    GLuint m_VaoName;
    GLuint m_VertexBufferName;
    GLuint m_IndexBufferName;
};

struct SpriteOptions
{
    const float kDefaultPixelsPerUnit = 10.0f;
    
    float m_PixelsPerUnit;
    Vec2 m_Pivot;
    Vec4 m_TintColor;
    Vec2 m_Scale;
    
    SpriteOptions()
        : m_PixelsPerUnit(kDefaultPixelsPerUnit)
        , m_Pivot(0.5f, 0.5f)
        , m_TintColor(1.0f, 1.0f, 1.0f, 1.0f)
        , m_Scale(1.0f, 1.0f)
    {
    }
    
    void ResetPivot()
    {
        m_Pivot = Vec2(0.5f, 0.5f);
    }
};

void RenderOptionsInit(RenderOptions* renderOptions, int width, int height);

void RenderInit(RenderContext* renderContext, int width, int height);
void RenderInit(RenderContext* renderContext, const RenderOptions& renderOptions);
void RenderContextDestroy(RenderContext* renderContext);

void RenderFrameInit(RenderContext* context);
bool RenderFrameEnd(RenderContext* context);
void RenderUseMaterial(RenderContext* context, int* textureSlotItr, const Material* material);
void RenderSetMaterialConstants(RenderContext* renderContext, int* textureSlotItr, const Material* material);

void RenderSetRenderTarget(RenderContext* renderContexxt, Texture* texture);
void RenderSetReplacementShader(RenderContext* renderContext, Shader* shader);
void RenderClearReplacementShader(RenderContext* renderContext);

typedef void ProcessKeysCallback(void* data, int key, int scanCode, int action, int mods);
void RenderSetProcessKeysCallback(RenderContext* context, ProcessKeysCallback (*processKeysCallback));

void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model);
void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model, const Mat4& localToWorld);

void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, Texture* texture);
void RenderDrawFullscreen(RenderContext* renderContext, Material* material, Texture* texture);

void RenderAttachPostEffect(RenderContext* renderContext, PostEffect* effect);

Vec3 RenderGetScreenPos(RenderContext* renderContext, Vec3 avatarPosition);

void RenderSetBlendMode(Material::BlendMode blendMode);

SimpleModel* RenderGenerateCube(RenderContext* renderContext, float halfExtent);
SimpleModel* RenderGenerateSprite(RenderContext* renderContext, const SpriteOptions& spriteOptions, Material* material);

void RenderDumpModel(const SimpleModel* model);
void RenderDumpModelTransformed(const SimpleModel* model, const Mat4& a);
void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model);

void RenderUpdatePointLights(RenderContext* renderContext, const Light* pointLights, int numPointLights);
void RenderUpdateConicalLights(RenderContext* renderContext, const Light* conicalLights, int numConicalLights);
void RenderUpdateCylindricalLights(RenderContext* renderContext, const Light* cylindricalLights, int numCylindricalLights);
void RenderUpdateDirectionalLights(RenderContext* renderContext, const Light* directionalLights, int numDirectionalLights);

// global properties
int RenderAddGlobalProperty(RenderContext* renderContext, const char* materialPropertyName, Material::MaterialPropertyType type);
void RenderGlobalSetFloat(RenderContext* renderContext, int index, float value);
void RenderGlobalSetInt(RenderContext* renderContext, int index, int value);
void RenderGlobalSetVector(RenderContext* renderContext, int index, Vec4 value);
void RenderGlobalSetMatrix(RenderContext* renderContext, int index, const Mat4& value);
void RenderGlobalSetTexture(RenderContext* renderContext, int index, int textureId);
void RenderGlobalSetTexture(RenderContext* renderContext, int index, Texture* texture);

void SimpleModelSetVertexAttributes(const SimpleModel* simpleModel);

void SimpleModelDestroy(SimpleModel* simpleModel);

const char * GetGLErrorString(GLenum error);

#ifndef NDEBUG

// jiv fixme copy pasta
#if defined(WINDOWS)
#include <stdlib.h>

#define _GetGLError(f,l)                                        \
{                                                               \
    GLenum err = glGetError();                                  \
    int errCount =  0;                                          \
    while (err != GL_NO_ERROR) {                                \
        FPrintf(stderr, "[%s:%d] GLError %s\n",                 \
                (f),(l),                                        \
                GetGLErrorString(err));                         \
        err = glGetError();                                     \
        errCount++;                                             \
    }                                                           \
    if (errCount) { DebugBreak(); exit(1); }    \
}

#else

#define _GetGLError(f,l)                                        \
{                                                               \
    GLenum err = glGetError();                                  \
    int errCount =  0;                                          \
    while (err != GL_NO_ERROR) {                                \
        FPrintf(stderr, "[%s:%d] GLError %s\n",                 \
                (f),(l),                                        \
                GetGLErrorString(err));                         \
        err = glGetError();                                     \
        errCount++;                                             \
    }                                                           \
    if (errCount) { __builtin_trap(); exit(1); }                \
}
#endif

#define GetGLError() _GetGLError(__FILE__, __LINE__)

class GLErrorScope
{
    const char* m_File;
    int m_Line;
    
public:
    GLErrorScope(const char* file, int line);
    ~GLErrorScope();
};

#define GL_ERROR_SCOPE() GLErrorScope errorScope(__FILE__, __LINE__)

#else
#define GetGLError()
#define GL_ERROR_SCOPE()
#endif
