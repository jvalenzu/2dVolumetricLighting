#pragma once

#include <OpenCL/opencl.h>
#include <OpenGL/gl3.h>
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
    
    cl_context m_Context;
    cl_command_queue m_CommandQueue;
    
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
    GLuint m_CylindricalLightUbo;
    GLuint m_ConicalLightUbo;
    
    Vec4 m_AmbientLightColor;
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
    
    int getNumIndices() const
    {
        return 4*m_NumLines;
    }
    
    void clear()
    {
        m_NumLines = 0;
    }
    
    DebugRenderContext()
    {
        clear();
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
};

void RenderOptionsInit(RenderOptions* renderOptions, int width, int height);

void RenderInit(RenderContext* renderContext, int width, int height);
void RenderInit(RenderContext* renderContext, const RenderOptions& renderOptions);
void RenderContextDestroy(RenderContext* renderContext);

void RenderFrameInit(RenderContext* context);
bool RenderFrameEnd(RenderContext* context);
void RenderSetAmbientLight(RenderContext* context, Vec4 color);
void RenderUseMaterial(RenderContext* context, const Material* material);
void RenderSetMaterialConstants(RenderContext* renderContext, const Material* material);

void RenderSetRenderTarget(RenderContext* renderContexxt, Texture* texture);
void RenderSetReplacementShader(RenderContext* renderContext, Shader* shader);
void RenderClearReplacementShader(RenderContext* renderContext);

typedef void ProcessKeysCallback(void* data, int key, int scanCode, int action, int mods);
void RenderSetProcessKeysCallback(RenderContext* context, ProcessKeysCallback (*processKeysCallback));


void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model);

void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, Texture* texture);
void RenderDrawFullscreen(RenderContext* renderContext, Material* material, Texture* texture);

void RenderAttachPostEffect(RenderContext* renderContext, PostEffect* effect);

Vec3 RenderGetScreenPos(RenderContext* renderContext, Vec3 avatarPosition);

void RenderSetBlendMode(Material::BlendMode blendMode);


SimpleModel* RenderGenerateSphere(float radius);
SimpleModel* RenderGenerateCube(float halfExtent);
SimpleModel* RenderGenerateSprite(const SpriteOptions& spriteOptions, Material* material);

void RenderDumpModel(const SimpleModel* model);
void RenderDumpModelTransformed(const SimpleModel* model, const Mat4& a);
void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model);

void RenderUpdatePointLights(RenderContext* renderContext, const PointLight* pointLights, int numPointLights);
void RenderUpdateConicalLights(RenderContext* renderContext, const ConicalLight* conicalLights, int numConicalLights);
void RenderUpdateCylindricalLights(RenderContext* renderContext, const CylindricalLight* cylindricalLights, int numCylindricalLights);

void SimpleModelSetVertexAttributes(const SimpleModel* simpleModel);

void SimpleModelDestroy(SimpleModel* simpleModel);

const char * GetGLErrorString(GLenum error);

#ifndef NDEBUG
#define GetGLError()                                            \
{                                                               \
    GLenum err = glGetError();                                  \
    int errCount =  0;                                          \
    while (err != GL_NO_ERROR) {                                \
        fprintf(stderr, "[%s:%d] GLError %s set in\n",          \
                __FILE__,                                       \
                __LINE__,                                       \
                GetGLErrorString(err));                         \
        err = glGetError();                                     \
        errCount++;                                             \
    }                                                           \
    if (errCount) { __builtin_trap(); exit(1); }                \
}

class GLErrorScope
{
public:
    GLErrorScope();
    ~GLErrorScope();
};

#define GL_ERROR_SCOPE() GLErrorScope errorScope

#else
#define GetGLError()
#define GL_ERROR_SCOPE()
#endif
