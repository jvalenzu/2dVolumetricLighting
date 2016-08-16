#include "Render/GL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Render/Render.h"
#include "Engine/Matrix.h"
#include "Render/Material.h"
#include "Render/Model.h"
#include "Render/PostEffect.h"
#include "Render/Texture.h"
#include "Engine/Utils.h"
#include "Tool/Utils.h"

#define kPointLightBinding 2
#define kCylindricalLightBinding 3
#define kConicalLightBinding 4

const char* GetGLErrorString(GLenum error)
{
    switch (error)
    {
        case GL_NO_ERROR:
        {
            return "GL_NO_ERROR";
            break;
        }
        case GL_INVALID_ENUM:
        {
            return "GL_INVALID_ENUM";
            break;
        }
        case GL_INVALID_VALUE:
        {
            return "GL_INVALID_VALUE";
            break;
        }
        case GL_INVALID_OPERATION:
        {
            return "GL_INVALID_OPERATION";
            break;
        }
#if defined __gl_h_ || defined __gl3_h_
        case GL_OUT_OF_MEMORY:
        {
            return "GL_OUT_OF_MEMORY";
            break;
        }
        case GL_INVALID_FRAMEBUFFER_OPERATION:
        {
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
#endif
#if defined __gl_h_
        case GL_STACK_OVERFLOW:
        {
            return "GL_STACK_OVERFLOW";
            break;
        }
        case GL_STACK_UNDERFLOW:
        {
            return "GL_STACK_UNDERFLOW";
            break;
        }
        case GL_TABLE_TOO_LARGE:
        {
            return "GL_TABLE_TOO_LARGE";
            break;
        }
#endif
    }
    return "(ERROR: Unknown Error Enum)";
}

#ifndef NDEBUG
GLErrorScope::GLErrorScope()
{
    GetGLError();
}

GLErrorScope::~GLErrorScope()
{
    GetGLError();
}
#endif

// s_CgInit
//
// Initialize GL
static bool s_CgInit(RenderContext* renderContext)
{
#if USE_CL
    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
    cl_context_properties properties[] =
    {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)shareGroup,
        0
    };

    gcl_gl_set_sharegroup(shareGroup);
    
    cl_device_id device_ids[2];
    cl_uint num_devices;
    int err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 2, device_ids, &num_devices);
    if (err != CL_SUCCESS)
    {
        Printf("Error: Failed to create a device group!\n");
        return false;
    }
    
    // cl_device_id device_id = device_ids[0];
    cl_device_id device_id = device_ids[1];
    
    // create a compute context
    cl_context context = renderContext->m_Context = clCreateContext(0, 1, &device_id, nullptr, nullptr, &err);
    if (!context)
    {
        Printf("Error: Failed to create a compute context!\n");
        return false;
    }
    
    // create a command queue
    cl_command_queue commands = renderContext->m_CommandQueue = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        Printf("Error: Failed to create a command commands!\n");
        return false;
    }
    
    // size_t extensionSize;
    // clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, nullptr, &extensionSize);
    // 
    // char* extensions = new char[extensionSize];
    // clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, extensionSize, extensions, nullptr);
    // FPrintf(stderr, "\nDevice Extensions:\n");
    // for (int i=0; i<(int)strlen(extensions); ++i)
    // {
    //     if (extensions[i] == ' ')
    //         extensions[i] = '\n';
    // }
    // FPrintf(stderr, "%s\n", extensions);
    // delete[] extensions;
#endif
    
    return true;
}

void RenderInit(RenderContext* renderContext, int width, int height)
{
    RenderOptions renderOptions;
    RenderOptionsInit(&renderOptions, width, height);
    RenderInit(renderContext, renderOptions);
}

void RenderInit(RenderContext* renderContext, const RenderOptions& renderOptions)
{
    int width = renderOptions.m_Width;
    int height = renderOptions.m_Height;

    auto errorCallback = [] (int error, const char* description)
    {
        FPrintf(stderr, "Error[%d]: %s\n", error, description);
    };
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    renderContext->m_Window = glfwCreateWindow(width, height, "asdf", NULL, NULL);
    if (!renderContext->m_Window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    
    glfwMakeContextCurrent(renderContext->m_Window);

    #if defined(WINDOWS)
    WindowsGLInit();
#endif
    


    glfwGetFramebufferSize(renderContext->m_Window, &width, &height);
    renderContext->m_Width = width;
    renderContext->m_Height = height;
    
    glViewport(0, 0, renderContext->m_Width, renderContext->m_Height);
    
    VectorSet(&renderContext->m_ClearColor, 0.5f, 0.5f, 0.5f);
    glClearColor(renderContext->m_ClearColor.m_X[0], renderContext->m_ClearColor.m_X[1], renderContext->m_ClearColor.m_X[2], 0);
    glfwSetWindowPos(renderContext->m_Window, 10, 10);
    glfwShowWindow(renderContext->m_Window);
    
    glfwSwapInterval(0);
    
    // identity
    MatrixMakeIdentity(&renderContext->m_Camera);
    MatrixMakeIdentity(&renderContext->m_View);
    
    const float aspectRatio = (float) renderContext->m_Width/renderContext->m_Height;
    
    // initialize perspective matrix
    if (renderOptions.m_CameraType == RenderOptions::kPerspective)
        ToolLoadPerspective(&renderContext->m_Projection, 45.0f, aspectRatio, 1.0f, 16777216.0f);
    else
        ToolLoadPerspective(&renderContext->m_Projection, 45.0f, aspectRatio, 1.0f, 16777216.0f);
    
    // initialize shader
    ShaderInit();
    TextureInit();
    
    // view setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, renderContext->m_Width, renderContext->m_Height);
    
    // enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // cull back faces
    glCullFace(GL_BACK);
    
    // initialize GL
    if (!s_CgInit(renderContext))
        exit(1);
    
    renderContext->m_LastTime = 0.0f;
    renderContext->m_FrameCount = 0;
    renderContext->m_FrameRollover = 0;
    
    GLuint quadVertexArrayId;
    glGenVertexArrays(1, &quadVertexArrayId);
    glBindVertexArray(quadVertexArrayId);
    
    const GLfloat quadVertexBufferData[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0, 0.0,
        +1.0f, -1.0f, 0.0f, 1.0, 0.0,
        -1.0f, +1.0f, 0.0f, 0.0, 1.0,
        -1.0f, +1.0f, 0.0f, 0.0, 1.0,
        +1.0f, -1.0f, 0.0f, 1.0, 0.0,
        +1.0f, +1.0f, 0.0f, 1.0, 1.0
    };
    
    GLuint quadVertexBuffer;
    glGenBuffers(1, &quadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof quadVertexBufferData, quadVertexBufferData, GL_STATIC_DRAW);
    
    renderContext->m_QuadVertexArrayId = quadVertexArrayId;
    renderContext->m_QuadVertexBufferId = quadVertexBuffer;
    
    glEnableVertexAttribArray(kVertexAttributePosition);
    glVertexAttribPointer(kVertexAttributePosition,
                          3,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          20,                                         // stride
                          0);                                         // offset in buffer data
    
    glEnableVertexAttribArray(kVertexAttributeTexCoord);
    glVertexAttribPointer(kVertexAttributeTexCoord,
                          2,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          20,                                         // stride
                          (void*) 12);                                // offset in buffer data
    
    // generate frame buffer
    glGenFramebuffers(2, renderContext->m_FrameBufferIds);
    
    glGenTextures(2, renderContext->m_FrameBufferColorIds);
    
    for (int i=0; i<2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, renderContext->m_FrameBufferColorIds[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderContext->m_FrameBufferColorIds[i], 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // posteffects
    renderContext->m_NumPostEffects = 0;
    renderContext->m_MaxNumPostEffects = 64;
    renderContext->m_PostEffects = (PostEffect**) malloc(sizeof(PostEffect*)*renderContext->m_MaxNumPostEffects);
    
    // replacement shader
    renderContext->m_ReplacementShader = nullptr;
    
    // light ubo
    glGenBuffers(1, &renderContext->m_PointLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_PointLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(PointLight), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kPointLightBinding, renderContext->m_PointLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glGenBuffers(1, &renderContext->m_CylindricalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_CylindricalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(CylindricalLight), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kCylindricalLightBinding, renderContext->m_CylindricalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glGenBuffers(1, &renderContext->m_ConicalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_ConicalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(ConicalLight), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kConicalLightBinding, renderContext->m_ConicalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    renderContext->m_AmbientLightColor = Vec4(0,0,0,0);
}

// RenderContextDestroy
void RenderContextDestroy(RenderContext* renderContext)
{
    ShaderFini();
    TextureFini();
    
    free(renderContext->m_PostEffects);
    renderContext->m_PostEffects = nullptr;
}

// RenderDrawModel
//
// Render model.
void RenderDrawModel(RenderContext* renderContext, const SimpleModel* model)
{
    GL_ERROR_SCOPE();
    
    const Material* material = model->m_Material;
    const Shader* shader = material->m_Shader;
    
    if (renderContext->m_ReplacementShader)
        shader = renderContext->m_ReplacementShader;
    
    Mat4 normalModel;
    MatrixInvert(&normalModel, model->m_Po);
    MatrixTransposeInsitu(&normalModel);
    
    RenderUseMaterial(renderContext, material);
    
    GetGLError();
    GLint pi = glGetUniformLocation(shader->m_ProgramName, "project");
    glUniformMatrix4fv(pi, 1, GL_FALSE, renderContext->m_Projection.asFloat());
    GetGLError();
    
    GLint nmi = glGetUniformLocation(shader->m_ProgramName, "normalModel");
    glUniformMatrix4fv(nmi, 1, GL_FALSE, normalModel.asFloat());
    
    Mat4 modelView;
    MatrixMultiply(&modelView, model->m_Po, renderContext->m_View);
    
    GLint mvi = glGetUniformLocation(shader->m_ProgramName, "modelView");
    glUniformMatrix4fv(mvi, 1, GL_FALSE, modelView.asFloat());
    
    GLint viewIndex = glGetUniformLocation(shader->m_ProgramName, "view");
    glUniformMatrix4fv(viewIndex, 1, GL_FALSE, renderContext->m_View.asFloat());
    
    GLuint pointLightsBlockIndex = shader->m_PointLightBlockIndex;
    if (pointLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, pointLightsBlockIndex, kPointLightBinding);
    
    GLuint cylindricalLightsBlockIndex = shader->m_CylindricalLightBlockIndex;
    if (cylindricalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, cylindricalLightsBlockIndex, kCylindricalLightBinding);
    
    GLuint conicalLightsBlockIndex = shader->m_ConicalLightBlockIndex;
    if (conicalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, conicalLightsBlockIndex, kConicalLightBinding);
    
    SimpleModelSetVertexAttributes(model);
    
    glDrawElements(GL_TRIANGLES, model->m_NumIndices, GL_UNSIGNED_SHORT, (void*) 0);
}

void RenderUpdatePointLights(RenderContext* renderContext, const PointLight* pointLights, int numPointLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_PointLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(PointLight), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    memcpy(p, pointLights, numPointLights * sizeof(PointLight));
    
    PointLight* dest = (PointLight*)p;
    for (int i=0; i<numPointLights; ++i)
    {
        Vec3 screenPosition = RenderGetScreenPos(renderContext, pointLights[i].m_Position.xyz());
        dest[i].m_Position = Vec4(FromZeroOne(screenPosition), 1.0f);
        dest[i].m_Range = 1.0f / dest[i].m_Range;
    }
    
    // Printf("------------------\n");
    // Printf("numPointLights: %d\n", numPointLights);
    // for (int i=0; i<numPointLights; ++i)
    // {
    //     Vec3 screenPos = RenderGetScreenPos(renderContext, pointLights[i].m_Position.xyz());
    //     Printf("[%d] screenPosition: %f %f %f -> %f %f\n",
    //            i,
    //            pointLights[i].m_Position.m_X[0], pointLights[i].m_Position.m_X[1], pointLights[i].m_Position.m_X[2],
    //            screenPos.m_X[0], screenPos.m_X[1]);
    //     // DumpLight(pointLights[i]);
    // }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RenderUpdateCylindricalLights(RenderContext* renderContext, const CylindricalLight* cylindricalLights, int numCylindricalLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_CylindricalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(PointLight), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    memcpy(p, cylindricalLights, numCylindricalLights * sizeof(CylindricalLight));
    
    CylindricalLight* dest = (CylindricalLight*)p;
    for (int i=0; i<numCylindricalLights; ++i)
    {
        Vec3 screenPosition = RenderGetScreenPos(renderContext, cylindricalLights[i].m_Position.xyz());
        dest[i].m_Position.SetXY(Vec4(FromZeroOne(screenPosition), 1.0f).xy());
        dest[i].m_Position.SetZW(dest[i].m_Position.xy() + dest[i].m_Direction.xy()*dest[i].m_Length);
        dest[i].m_Range = 1.0f / dest[i].m_Range;
    }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RenderUpdateConicalLights(RenderContext* renderContext, const ConicalLight* conicalLights, int numConicalLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_ConicalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(PointLight), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    memcpy(p, conicalLights, numConicalLights * sizeof(ConicalLight));
    
    ConicalLight* dest = (ConicalLight*)p;
    for (int i=0; i<numConicalLights; ++i)
    {
        Vec3 screenPosition = RenderGetScreenPos(renderContext, conicalLights[i].m_Position.xyz());
        dest[i].m_Position = Vec4(FromZeroOne(screenPosition), 1.0f);
        dest[i].m_Range = 1.0f / dest[i].m_Range;
    }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void RenderOptionsInit(RenderOptions* renderOptions, int width, int height)
{
    renderOptions->m_CameraType = RenderOptions::kPerspective;
    renderOptions->m_Width = width;
    renderOptions->m_Height = height;
}

void RenderDumpModel(const SimpleModel* model)
{
    Printf("model:\n");
    MatrixDump(model->m_Po, "    ");
    Printf("    numVertices: %d\n", model->m_NumVertices);
    Printf("    numIndices: %d\n", model->m_NumIndices);
    
    for (int i=0; i<model->m_NumIndices; ++i)
    {
        const SimpleVertex* vertex = &model->m_Vertices[model->m_Indices[i]];
        float in[3] = { vertex->m_Position[0], vertex->m_Position[1], vertex->m_Position[2] };
        float out[3];
        MatrixMultiplyVec(out, in, model->m_Po);
        Printf("    point[%d] = { %.2f, %.2f, %.2f }\n", i, out[0], out[1], out[2]);
    }
}

void RenderDumpModelTransformed(const SimpleModel* model, const Mat4& a)
{
    Printf("model:\n");
    MatrixDump(a, "    ");
    Printf("    numVertices: %d\n", model->m_NumVertices);
    Printf("    numIndices: %d\n", model->m_NumIndices);
    
    for (int i=0; i<model->m_NumIndices; ++i)
    {
        const SimpleVertex* vertex = model->m_Vertices + model->m_Indices[i];
        float out[3];
        MatrixMultiplyVec(out, vertex->m_Position, a);
        Printf("    point[%d] = [ %.2f, %.2f, %.2f, 1 ] -> [ %.2f, %.2f, %.2f, 1 ]\n",
               i, vertex->m_Position[0], vertex->m_Position[1], vertex->m_Position[2], out[0], out[1], out[2]);
    }
}

void RenderSetBlendMode(Material::BlendMode blendMode)
{
    switch (blendMode)
    {
        case Material::BlendMode::kOpaque:
        {
            glDepthMask(GL_TRUE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ZERO);
            glBlendEquation(GL_FUNC_ADD);
            
            break;
        }
        case Material::BlendMode::kCutout:
        case Material::BlendMode::kBlend:
        {
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            
            break;
        }
    }
}

static inline void RenderUseProgram(const Shader* shader)
{
    // Printf("Using %s\n", shader->m_DebugName);
    glUseProgram(shader->m_ProgramName);
}

// RenderUseMaterial
//
void RenderUseMaterial(RenderContext* renderContext, const Material* material)
{
    GL_ERROR_SCOPE();
    
    const Shader* replacementShader = renderContext->m_ReplacementShader;
    if (replacementShader == nullptr)
    {
        RenderUseProgram(material->m_Shader);
        
        if (renderContext->m_CachedMaterial != material)
        {
            renderContext->m_CachedMaterial = material;
            RenderSetMaterialConstants(renderContext, material);
            RenderSetBlendMode(material->m_BlendMode);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, material->m_Texture->m_TextureId);
    }
}

// RenderFrameInit
//
void RenderFrameInit(RenderContext* renderContext)
{
    GL_ERROR_SCOPE();
    
    glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[0]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwMakeContextCurrent(renderContext->m_Window);
    
    float currentTime = (float) glfwGetTime();
    renderContext->m_FrameCount++;
    renderContext->m_FrameRollover++;
    
    if (currentTime - renderContext->m_LastTime >= 1.0f)
    {
        char title[256] = { 0 };
        snprintf(title, sizeof title-1, "%s %s - [%3.2f ms]", "asdf", "version", 1000.0f / (float)renderContext->m_FrameRollover);
        
        glfwSetWindowTitle(renderContext->m_Window, title);
        
        renderContext->m_FrameRollover = 0;
        renderContext->m_LastTime += 1.0f;
    };
}

// RenderAttachPostEffect
//
void RenderAttachPostEffect(RenderContext* renderContext, PostEffect* effect)
{
    if (renderContext->m_NumPostEffects != renderContext->m_MaxNumPostEffects)
        renderContext->m_PostEffects[renderContext->m_NumPostEffects++] = effect;
}

// RenderFullscreenSetVertexAttributes
//
void RenderFullscreenSetVertexAttributes(RenderContext* renderContext)
{
    glBindBuffer(GL_ARRAY_BUFFER, renderContext->m_QuadVertexBufferId);
    
    glEnableVertexAttribArray(kVertexAttributePosition);
    glEnableVertexAttribArray(kVertexAttributeTexCoord);
    glDisableVertexAttribArray(kVertexAttributeColor);
    glDisableVertexAttribArray(kVertexAttributeNormal);
    
    glEnableVertexAttribArray(kVertexAttributePosition);
    glVertexAttribPointer(kVertexAttributePosition,
                          3,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          20,                                         // stride
                          0);                                         // offset in buffer data
    
    glEnableVertexAttribArray(kVertexAttributeTexCoord);
    glVertexAttribPointer(kVertexAttributeTexCoord,
                          2,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          20,                                         // stride
                          (void*) 12);                                // offset in buffer data
}

void RenderSetProcessKeysCallback(RenderContext* context, ProcessKeysCallback (*processKeysCallback))
{
    // auto wrapper = [] (GLFWwindow* window, int key, int scanCode, int action, int mods)
    // {
    //     processKeysCallback(key, scanCode, action, mode);
    // };
    
    glfwSetKeyCallback(context->m_Window, (GLFWkeyfun) processKeysCallback);
}

void RenderDrawFullscreen(RenderContext* renderContext, Material* material, int textureId)
{
    GL_ERROR_SCOPE();
    
    const Shader* shader = material->m_Shader;
    
    if (material == nullptr || shader == nullptr)
        return;
    
    RenderFullscreenSetVertexAttributes(renderContext);
    
    RenderUseProgram(shader);
    
    RenderSetMaterialConstants(renderContext, material);
    RenderSetBlendMode(material->m_BlendMode);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    Mat4 identity;
    MatrixMakeIdentity(&identity);
    
    GLint pi = glGetUniformLocation(shader->m_ProgramName, "project");
    glUniformMatrix4fv(pi, 1, GL_FALSE, identity.asFloat());
    
    GLint nmi = glGetUniformLocation(shader->m_ProgramName, "normalModel");
    glUniformMatrix4fv(nmi, 1, GL_FALSE, identity.asFloat());
    
    GLint mvi = glGetUniformLocation(shader->m_ProgramName, "modelView");
    glUniformMatrix4fv(mvi, 1, GL_FALSE, identity.asFloat());
    
    GLint viewIndex = glGetUniformLocation(shader->m_ProgramName, "view");
    glUniformMatrix4fv(viewIndex, 1, GL_FALSE, identity.asFloat());
    
    GLint mainTextureSlot = glGetUniformLocation(shader->m_ProgramName, "_MainTex");
    glProgramUniform1i(shader->m_ProgramName, mainTextureSlot, 0);
    
    GLuint pointLightsBlockIndex = shader->m_PointLightBlockIndex;
    if (pointLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, pointLightsBlockIndex, kPointLightBinding);
    
    GLuint cylindricalLightsBlockIndex = shader->m_CylindricalLightBlockIndex;
    if (cylindricalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, cylindricalLightsBlockIndex, kCylindricalLightBinding);
    
    GLuint conicalLightsBlockIndex = shader->m_ConicalLightBlockIndex;
    if (conicalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, conicalLightsBlockIndex, kConicalLightBinding);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// RenderDrawFullscreen
//
void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, int textureId)
{
    GL_ERROR_SCOPE();
    
    RenderFullscreenSetVertexAttributes(renderContext);
    
    RenderUseProgram(shader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    Mat4 identity;
    MatrixMakeIdentity(&identity);
    
    GLint pi = glGetUniformLocation(shader->m_ProgramName, "project");
    glUniformMatrix4fv(pi, 1, GL_FALSE, identity.asFloat());
    
    GLint nmi = glGetUniformLocation(shader->m_ProgramName, "normalModel");
    glUniformMatrix4fv(nmi, 1, GL_FALSE, identity.asFloat());
    
    GLint mvi = glGetUniformLocation(shader->m_ProgramName, "modelView");
    glUniformMatrix4fv(mvi, 1, GL_FALSE, identity.asFloat());
    
    GLint viewIndex = glGetUniformLocation(shader->m_ProgramName, "view");
    glUniformMatrix4fv(viewIndex, 1, GL_FALSE, identity.asFloat());
    
    GLint mainTextureSlot = glGetUniformLocation(shader->m_ProgramName, "_MainTex");
    glProgramUniform1i(shader->m_ProgramName, mainTextureSlot, 0);
    
    GLuint pointLightsBlockIndex = shader->m_PointLightBlockIndex;
    if (pointLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, pointLightsBlockIndex, kPointLightBinding);
    
    GLuint cylindricalLightsBlockIndex = shader->m_CylindricalLightBlockIndex;
    if (cylindricalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, cylindricalLightsBlockIndex, kCylindricalLightBinding);
    
    GLuint conicalLightsBlockIndex = shader->m_ConicalLightBlockIndex;
    if (conicalLightsBlockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->m_ProgramName, conicalLightsBlockIndex, kConicalLightBinding);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, Texture* texture)
{
    GL_ERROR_SCOPE();
    
    int textureId = renderContext->m_FrameBufferIds[0];
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    
    RenderDrawFullscreen(renderContext, shader, textureId);
}

void RenderDrawFullscreen(RenderContext* renderContext, Material* material, Texture* texture)
{
    GL_ERROR_SCOPE();
    
    int textureId = renderContext->m_FrameBufferColorIds[0];
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    
    RenderDrawFullscreen(renderContext, material, textureId);
}

void RenderSetRenderTarget(RenderContext* renderContext, Texture* texture)
{
    if (texture && texture->m_FrameBufferId>0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, texture->m_FrameBufferId);
        glViewport(0, 0, texture->m_Width, texture->m_Height);
        if (texture->m_RenderTextureFlags & Texture::RenderTextureFlags::kClearColor)
        {
            glClearColor(texture->m_ClearColor.m_X[0], texture->m_ClearColor.m_X[1], texture->m_ClearColor.m_X[2], 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.5f, 0.5f, 0.5f, 0);
        }
    }
    else
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            Printf("Error - framebuffer is not ready\n");
        
        glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[0]);
        glViewport(0, 0, renderContext->m_Width, renderContext->m_Height);
        glClearColor(renderContext->m_ClearColor.m_X[0], renderContext->m_ClearColor.m_X[1], renderContext->m_ClearColor.m_X[2], 0);
    }
}

void RenderClearReplacementShader(RenderContext* renderContext)
{
    renderContext->m_ReplacementShader = nullptr;
}

void RenderSetReplacementShader(RenderContext* renderContext, Shader* shader)
{
    renderContext->m_ReplacementShader = shader;
    if (shader)
    {
        RenderUseProgram(shader);
        
        GLint pi = glGetUniformLocation(shader->m_ProgramName, "project");
        glUniformMatrix4fv(pi, 1, GL_FALSE, renderContext->m_Projection.asFloat());
        
        glActiveTexture(GL_TEXTURE0);
        GLint mainTextureSlot = glGetUniformLocation(shader->m_ProgramName, "_MainTex");
        glProgramUniform1i(shader->m_ProgramName, mainTextureSlot, 0);
        
        // jiv fixme
        RenderSetBlendMode(Material::BlendMode::kBlend);
    }
}

bool RenderFrameEnd(RenderContext* renderContext)
{
    GL_ERROR_SCOPE();
    
    bool ret = true;
    
    if (renderContext->m_NumPostEffects > 0)
    {
        for (int i=0; i<renderContext->m_NumPostEffects; ++i)
        {
            // current color buffer
            int frameBufferColorId = renderContext->m_FrameBufferColorIds[i&1];
            
            if (i != renderContext->m_NumPostEffects-1)
                glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[~i&1]); // target next post effect
            else
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // target final destination
            
            RenderDrawFullscreen(renderContext, renderContext->m_PostEffects[i]->m_Shader, frameBufferColorId);
        }
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // target final destination
        RenderDrawFullscreen(renderContext, g_SimpleShader, renderContext->m_FrameBufferColorIds[0]);
    }
    
    if (glfwWindowShouldClose(renderContext->m_Window))
        ret = false;
    
    // poll events
    glfwSwapBuffers(renderContext->m_Window);
    glfwPollEvents();
    
    return ret;
}

Vec3 RenderGetScreenPos(RenderContext* renderContext, Vec3 worldPos)
{
    Vec4 temp;
    MatrixMultiplyVecW1(&temp, worldPos, renderContext->m_View);
    
    Vec4 ret;
    MatrixMultiplyVec(&ret, temp, renderContext->m_Projection);
    
    float w = ret.W();
    // ret /= w;
    
#if 1
    ret.m_X[0] = (ret.m_X[0] * renderContext->m_Width)  / (2.0f*w) + renderContext->m_Width*0.5f;
    ret.m_X[1] = (ret.m_X[1] * renderContext->m_Height) / (2.0f*w) + renderContext->m_Height*0.5f;
    
    ret.m_X[0] /= renderContext->m_Width;
    ret.m_X[1] /= renderContext->m_Height;
#else
    ret.m_X[0] = (ret.m_X[0] * renderContext->m_Width)  / (2.0f*w) + renderContext->m_Width*0.5f;
    ret.m_X[1] = (ret.m_X[1] * renderContext->m_Height) / (2.0f*w) + renderContext->m_Height*0.5f;
    
    ret.m_X[0] /= renderContext->m_Width;
    ret.m_X[1] /= renderContext->m_Height;
#endif
    
    return ret.XYZ();
}


void SimpleModelDestroy(SimpleModel* simpleModel)
{
    if (simpleModel == nullptr)
        return;
    
    MaterialDestroy(simpleModel->m_Material);
    
    glDeleteBuffers(1, &simpleModel->m_VertexBufferName);
    glDeleteBuffers(1, &simpleModel->m_IndexBufferName);
    glDeleteVertexArrays(1, &simpleModel->m_VaoName);
    
    delete [] simpleModel->m_Vertices;
    delete [] simpleModel->m_Indices;
    delete simpleModel;
}

SimpleModel* RenderGenerateSprite(const SpriteOptions& spriteOptions, Material* material)
{
    GL_ERROR_SCOPE();
    
    const float halfWidth = material->m_Texture->m_Width / (2.0f * spriteOptions.m_PixelsPerUnit);
    const float halfHeight = material->m_Texture->m_Height / (2.0f * spriteOptions.m_PixelsPerUnit);
    
    SimpleModel* simpleModel = new SimpleModel();
    simpleModel->m_Material = material;
    
    int index = MaterialGetPropertyIndex(material, "TintColor");
    if (index >= 0)
        MaterialSetMaterialPropertyVector(material, index, spriteOptions.m_TintColor);
    
    // initialize the matrix position/orientation
    MatrixMakeIdentity(&simpleModel->m_Po);
    
    simpleModel->m_NumVertices = 4;
    simpleModel->m_Vertices = new SimpleVertex[simpleModel->m_NumVertices];
    simpleModel->m_Indices = new unsigned short[6];
    
    //  |   0------1
    //  |   |      |
    //  |   |      |
    //  |   |      |
    //  |   |      |
    //  |   3------2
    
    int numPositions = 0;
    
    const float leftOffset = spriteOptions.m_Pivot.m_X[0] * halfWidth * -1.0f;
    const float rightOffset = (1.0f-spriteOptions.m_Pivot.m_X[0]) * halfWidth;
    
    const float topOffset = (1.0f-spriteOptions.m_Pivot.m_X[1]) * halfHeight;
    const float bottomOffset = spriteOptions.m_Pivot.m_X[1] * halfHeight * -1.0f;
    
    // 0
    SimpleVertex* simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffff00ff;
    simpleVertex->m_Position[0] = leftOffset; simpleVertex->m_Position[1] = topOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;     simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  0.0f;      simpleVertex->m_Uv[1]       =  0.0f;
    
    // 1
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = rightOffset; simpleVertex->m_Position[1] = topOffset;  simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;       simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  1.0f;      simpleVertex->m_Uv[1]       =  0.0f;
    
    // 2
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = rightOffset; simpleVertex->m_Position[1] = bottomOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;         simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  1.0f;      simpleVertex->m_Uv[1]       =  1.0f;
    
    // 3
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = leftOffset; simpleVertex->m_Position[1] = bottomOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;        simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  0.0f;      simpleVertex->m_Uv[1]       =  1.0f;
    
    // front
    int numIndices = 0;
    
    simpleModel->m_Indices[numIndices++] = 0;
    simpleModel->m_Indices[numIndices++] = 1;
    simpleModel->m_Indices[numIndices++] = 2;
    
    simpleModel->m_Indices[numIndices++] = 2;
    simpleModel->m_Indices[numIndices++] = 3;
    simpleModel->m_Indices[numIndices++] = 0;
    
    // save number of elements
    simpleModel->m_NumIndices = numIndices;
    
    // generate vertex name
    glGenVertexArrays(1, &simpleModel->m_VaoName);
    glBindVertexArray(simpleModel->m_VaoName);
    
    // generate vertex buffer name
    glGenBuffers(1, &simpleModel->m_VertexBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, simpleModel->m_VertexBufferName);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex)*simpleModel->m_NumVertices, simpleModel->m_Vertices, GL_STATIC_DRAW);
    
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
                          2,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          sizeof(SimpleVertex),                       // stride
                          BUFFER_OFFSETOF(SimpleVertex, m_Uv));       // offset in buffer data
    
    glGenBuffers(1, &simpleModel->m_IndexBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simpleModel->m_IndexBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, simpleModel->m_NumIndices*sizeof(unsigned short), simpleModel->m_Indices, GL_STATIC_DRAW);
    
    return simpleModel;
}

// renderGenerateCube
//
SimpleModel* RenderGenerateCube(float halfWidth)
{
    SimpleModel* simpleModel = new SimpleModel;
    
    // initialize the matrix position/orientation
    MatrixMakeIdentity(&simpleModel->m_Po);
    
    simpleModel->m_NumVertices = 8;
    simpleModel->m_Vertices = new SimpleVertex[simpleModel->m_NumVertices];
    simpleModel->m_Indices = new unsigned short[36];
    
    //  | +z
    //  |
    //  |   0------1..    
    //  |   |    7 | ....6
    //  |   |      |     |
    //  |   |      |     |
    //  |   |      |     |
    //  |   3------2.....|
    //  |        4-------5
    //  |
    
    int numPositions = 0;
    SimpleVertex* simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = -halfWidth; // 0
    simpleVertex->m_Position[1] = +halfWidth; // 0
    simpleVertex->m_Position[2] = +halfWidth; // 0
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = +halfWidth; // 1
    simpleVertex->m_Position[1] = +halfWidth; // 1
    simpleVertex->m_Position[2] = +halfWidth; // 1
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = +halfWidth; // 2
    simpleVertex->m_Position[1] = +halfWidth; // 2
    simpleVertex->m_Position[2] = -halfWidth; // 2
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = -halfWidth; // 3
    simpleVertex->m_Position[1] = +halfWidth; // 3
    simpleVertex->m_Position[2] = -halfWidth; // 3
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = -halfWidth; // 4
    simpleVertex->m_Position[1] = -halfWidth; // 4
    simpleVertex->m_Position[2] = -halfWidth; // 4
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = +halfWidth; // 5
    simpleVertex->m_Position[1] = -halfWidth; // 5
    simpleVertex->m_Position[2] = -halfWidth; // 5
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = +halfWidth; // 6
    simpleVertex->m_Position[1] = -halfWidth; // 6
    simpleVertex->m_Position[2] = +halfWidth; // 6
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &simpleModel->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = -halfWidth; // 7
    simpleVertex->m_Position[1] = -halfWidth; // 7
    simpleVertex->m_Position[2] = +halfWidth; // 7
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    // front
    int numIndices = 0;
    
    // top
    simpleModel->m_Indices[numIndices++] = 0;
    simpleModel->m_Indices[numIndices++] = 1;
    simpleModel->m_Indices[numIndices++] = 2;
    
    simpleModel->m_Indices[numIndices++] = 2;
    simpleModel->m_Indices[numIndices++] = 3;
    simpleModel->m_Indices[numIndices++] = 0;
    
    // left side
    simpleModel->m_Indices[numIndices++] = 0;
    simpleModel->m_Indices[numIndices++] = 3;
    simpleModel->m_Indices[numIndices++] = 4;
    
    simpleModel->m_Indices[numIndices++] = 4;
    simpleModel->m_Indices[numIndices++] = 7;
    simpleModel->m_Indices[numIndices++] = 0;
    
    // bottom
    simpleModel->m_Indices[numIndices++] = 5;
    simpleModel->m_Indices[numIndices++] = 6;
    simpleModel->m_Indices[numIndices++] = 7;
    
    simpleModel->m_Indices[numIndices++] = 7;
    simpleModel->m_Indices[numIndices++] = 4;
    simpleModel->m_Indices[numIndices++] = 5;
    
    // right side
    simpleModel->m_Indices[numIndices++] = 2;
    simpleModel->m_Indices[numIndices++] = 1;
    simpleModel->m_Indices[numIndices++] = 6;
    
    simpleModel->m_Indices[numIndices++] = 6;
    simpleModel->m_Indices[numIndices++] = 5;
    simpleModel->m_Indices[numIndices++] = 2;
    
    // 
    // 
    // back
    simpleModel->m_Indices[numIndices++] = 0;
    simpleModel->m_Indices[numIndices++] = 7;
    simpleModel->m_Indices[numIndices++] = 6;
    
    simpleModel->m_Indices[numIndices++] = 6;
    simpleModel->m_Indices[numIndices++] = 1;
    simpleModel->m_Indices[numIndices++] = 0;
    
    // front
    simpleModel->m_Indices[numIndices++] = 3;
    simpleModel->m_Indices[numIndices++] = 2;
    simpleModel->m_Indices[numIndices++] = 5;
    
    simpleModel->m_Indices[numIndices++] = 5;
    simpleModel->m_Indices[numIndices++] = 4;
    simpleModel->m_Indices[numIndices++] = 3;
    
    assert(numIndices <= 36);
    
    // save number of elements
    simpleModel->m_NumIndices = numIndices;
    
    // generate vertex name
    glGenVertexArrays(1, &simpleModel->m_VaoName);
    glBindVertexArray(simpleModel->m_VaoName);
    
    // generate vertex buffer name
    glGenBuffers(1, &simpleModel->m_VertexBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, simpleModel->m_VertexBufferName);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex)*simpleModel->m_NumVertices, simpleModel->m_Vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &simpleModel->m_IndexBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simpleModel->m_IndexBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, simpleModel->m_NumIndices*sizeof(unsigned short), simpleModel->m_Indices, GL_STATIC_DRAW);
    
    return simpleModel;
}

void SimpleModelSetVertexAttributes(const SimpleModel* simpleModel)
{
    glBindBuffer(GL_ARRAY_BUFFER, simpleModel->m_VertexBufferName);
    
    // setup vertex attributes.  shaders bind on kVertexAttributePosition etc in ShaderCreate
    
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
                          2,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          sizeof(SimpleVertex),                       // stride
                          BUFFER_OFFSETOF(SimpleVertex, m_Uv));       // offset in buffer data
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simpleModel->m_IndexBufferName);
}

void RenderSetAmbientLight(RenderContext* renderContext, Vec4 color)
{
    renderContext->m_AmbientLightColor = color;
}
