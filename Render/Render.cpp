// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Render/GL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "slib/Common/Util.h"
#include "Render/Private/Material.h"
#include "Render/Private/Render.h"
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
#define kDirectionalLightBinding 5

// -------------------------------------------------------------------------------------------------
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
GLErrorScope::GLErrorScope(const char* file, int line)
    : m_File(file), m_Line(line)
{
    _GetGLError(file, line);
}

GLErrorScope::~GLErrorScope()
{
    _GetGLError(m_File, m_Line);
}
#endif

// -------------------------------------------------------------------------------------------------
// s_CgInit
//
// Initialize GL
static bool s_CgInit(RenderContext* renderContext)
{
#if USE_CL
    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);
    // cl_context_properties properties[] =
    // {
    //     CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
    //     (cl_context_properties)shareGroup,
    //     0
    // };
    
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

// -------------------------------------------------------------------------------------------------
// ResetFrameBufferTextureBuffers
//
static void ResetFrameBufferTextureBuffers(RenderContext* renderContext)
{
    GL_ERROR_SCOPE();
    
    for (int i=0; i<2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, renderContext->m_FrameBufferColorIds[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderContext->m_Width, renderContext->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        renderContext->m_FrameBufferDepthIds[i] = -1;
        
        glGenTextures(1, &renderContext->m_FrameBufferDepthIds[i]);
        glBindTexture(GL_TEXTURE_2D, renderContext->m_FrameBufferDepthIds[i]);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderContext->m_Width, renderContext->m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        
        glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderContext->m_FrameBufferColorIds[i], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderContext->m_FrameBufferDepthIds[i], 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// -------------------------------------------------------------------------------------------------
// s_WindowSizeCallback
//
// Called when framebuffer resizes
static void s_WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    RenderContext* renderContext = (RenderContext*) glfwGetWindowUserPointer(window);
    renderContext->m_Width = width;
    renderContext->m_Height = height;
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ResetFrameBufferTextureBuffers(renderContext);
    
    const float aspectRatio = (float) renderContext->m_Width/renderContext->m_Height;
    
    // initialize perspective matrix
    ToolLoadPerspective(&renderContext->m_Projection, 45.0f, aspectRatio, 1.0f, 16777216.0f);
}

// -------------------------------------------------------------------------------------------------
void RenderInit(RenderContext* renderContext, int width, int height)
{
    RenderOptions renderOptions;
    RenderOptionsInit(&renderOptions, width, height);
    RenderInit(renderContext, renderOptions);
}

// -------------------------------------------------------------------------------------------------
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
    
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    
    if (width == -1 || height == -1)
    {
        if (monitors && count > 0)
        {
            const GLFWvidmode* glfwVidMode = glfwGetVideoMode(monitors[0]);
            if (glfwVidMode)
            {
                if (width == -1)
                    width = glfwVidMode->width;
                if (height == -1)
                    height = glfwVidMode->height;
            }
        }
    }
    
    // make a fullscreen option jiv fixme
    monitors = nullptr;
    
    renderContext->m_Window = glfwCreateWindow(width, height, "asdf", monitors ? monitors[0] : nullptr, nullptr);
    if (!renderContext->m_Window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwMakeContextCurrent(renderContext->m_Window);
    
#if defined(WINDOWS)
    WindowsGLInit();
#endif
    
    // view setup
    glfwGetFramebufferSize(renderContext->m_Window, &width, &height);
    renderContext->m_Width = width;
    renderContext->m_Height = height;
    glViewport(0, 0, renderContext->m_Width, renderContext->m_Height);
    
    // set clear color
    VectorSet(&renderContext->m_ClearColor, 0.5f, 0.5f, 0.5f);
    glClearColor(renderContext->m_ClearColor.m_X[0], renderContext->m_ClearColor.m_X[1], renderContext->m_ClearColor.m_X[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // set initial window position
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
        ToolLoadPerspective(&renderContext->m_Projection, 45.0f, aspectRatio, 1.0f, 16777216.0f); // jiv fixme
    
    // initialize shader
    ShaderInit();
    TextureInit();
    
    // white texture, mostly for debugging.  ModelClassInit needs this.
    renderContext->m_WhiteTexture = TextureCreateFromFile("white.png");
    
    ModelClassInit(renderContext); // must happen after shader, texture init
    
    // enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // cull back faces
    glEnable(GL_CULL_FACE);
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
        /* position */ -1.0f, -1.0f, 0.0f, /* texcoord */ 0.0, 0.0,
        /* position */ +1.0f, -1.0f, 0.0f, /* texcoord */ 1.0, 0.0,
        /* position */ -1.0f, +1.0f, 0.0f, /* texcoord */ 0.0, 1.0,
        /* position */ -1.0f, +1.0f, 0.0f, /* texcoord */ 0.0, 1.0,
        /* position */ +1.0f, -1.0f, 0.0f, /* texcoord */ 1.0, 0.0,
        /* position */ +1.0f, +1.0f, 0.0f, /* texcoord */ 1.0, 1.0,
    };
    
    GLuint quadVertexBuffer;
    glGenBuffers(1, &quadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof quadVertexBufferData, quadVertexBufferData, GL_STATIC_DRAW);
    
    renderContext->m_QuadVertexArrayId = quadVertexArrayId;
    renderContext->m_QuadVertexBufferId = quadVertexBuffer;
    
    // generate frame buffer
    glGenFramebuffers(2, renderContext->m_FrameBufferIds);
    glGenTextures(2, renderContext->m_FrameBufferColorIds);
    
    // generate the relevant color data for frame buffers
    ResetFrameBufferTextureBuffers(renderContext);
    
    // posteffects
    renderContext->m_NumPostEffects = 0;
    renderContext->m_MaxNumPostEffects = 64;
    renderContext->m_PostEffects = (PostEffect**) malloc(sizeof(PostEffect*)*renderContext->m_MaxNumPostEffects);
    
    // replacement shader
    renderContext->m_ReplacementShader = nullptr;
    
    // light ubo
    glGenBuffers(1, &renderContext->m_PointLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_PointLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kPointLightBinding, renderContext->m_PointLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glGenBuffers(1, &renderContext->m_ConicalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_ConicalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kConicalLightBinding, renderContext->m_ConicalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glGenBuffers(1, &renderContext->m_CylindricalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_CylindricalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kCylindricalLightBinding, renderContext->m_CylindricalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glGenBuffers(1, &renderContext->m_DirectionalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_DirectionalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kDirectionalLightBinding, renderContext->m_DirectionalLightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glfwSetFramebufferSizeCallback(renderContext->m_Window, s_WindowSizeCallback);
    glfwSetWindowUserPointer(renderContext->m_Window, renderContext);
    
    // expose some stuff to shaders
    renderContext->m_ShaderTimeIndex = RenderAddGlobalProperty(renderContext, "_Time", Material::MaterialPropertyType::kVec4);
    renderContext->m_AspectRatioIndex = RenderAddGlobalProperty(renderContext, "_AspectRatio", Material::MaterialPropertyType::kFloat);
    
    // set aspect ratio
    RenderGlobalSetFloat(renderContext, renderContext->m_AspectRatioIndex, aspectRatio);
    
#if 0
    GLint n=0; 
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (int i=0; i<n; i++)
    {
        const char* extension = (const char*) glGetStringi(GL_EXTENSIONS, i);
        Printf("Ext %d: %s\n", i, extension); 
    }
#endif

    renderContext->m_CurrentFrameBufferIndex = 0;
}

// -------------------------------------------------------------------------------------------------
// RenderContextDestroy
void RenderContextDestroy(RenderContext* renderContext)
{
    // destroy "singleton" graphics assets
    TextureDestroy(renderContext->m_WhiteTexture);
    renderContext->m_WhiteTexture = nullptr;
    
    // teardown shader/texture singleton data structures
    ModelClassFini();
    ShaderFini();
    TextureFini();
    
    free(renderContext->m_PostEffects);
    renderContext->m_PostEffects = nullptr;
}

// -------------------------------------------------------------------------------------------------
void RenderUpdatePointLights(RenderContext* renderContext, const Light* pointLights, int numPointLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_PointLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    
    memset(p, 0, Light::kMaxLights*sizeof(Light));
    
    uint32_t pointLightMask = 0;
    
    Light* base = (Light*)p;
    for (int i=0; i<numPointLights; ++i)
    {
        const Light* source = &pointLights[i];
        Light* dest = &base[source->m_Index];
        
        pointLightMask |= 1ULL<<source->m_Index;
        
        *dest = *source;
        
        const float range = pointLights[i].m_Range;
        const Vec3 adjustedLightPosition = Vec3(pointLights[i].m_Position.xy(), kLightZ); // camera has fixed orientation, making this easier
        const Vec3 adjustedLightPosition2 = adjustedLightPosition + Vec3(range, range, 0.0f);
        
        const Vec3 screenPosition = RenderGetScreenPos(renderContext, adjustedLightPosition).xyz();
        const Vec3 screenPosition2 = RenderGetScreenPos(renderContext, adjustedLightPosition2).xyz();
        const float screenRange = (screenPosition - screenPosition2).Length();
        dest[i].m_Position = Vec4(FromZeroOne(screenPosition), 1.0f);
        dest[i].m_Range = 1.0f / screenRange;
        dest[i].m_Index = pointLights[i].m_Index;
    }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    renderContext->m_PointLightMask = pointLightMask;
}

// -------------------------------------------------------------------------------------------------
// void RenderUpdateConicalLights(RenderContext* renderContext, const Light* conicalLights, int numConicalLights)
//
// 
void RenderUpdateConicalLights(RenderContext* renderContext, const Light* conicalLights, int numConicalLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_ConicalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    
    uint32_t conicalLightMask = 0;
    
    Light* base = (Light*)p;
    for (int i=0; i<numConicalLights; ++i)
    {
        const Light* source = &conicalLights[i];
        Light* dest = &base[source->m_Index];
        
        *dest = *source;
        
        conicalLightMask |= 1ULL<<source->m_Index;
        
        const float range = source->m_Range;
        
        const Vec3 adjustedLightPosition = Vec3(source->m_Position.xy(), kLightZ); // camera has fixed orientation, making this easier
        const Vec3 adjustedLightPosition2 = adjustedLightPosition + Vec3(range*source->m_Direction.xy(), 0.0f);
        const Vec3 screenPosition = RenderGetScreenPos(renderContext, adjustedLightPosition).xyz();
        const Vec3 screenPosition2 = RenderGetScreenPos(renderContext, adjustedLightPosition2).xyz();
        
        const float screenRange = (screenPosition - screenPosition2).Length();
        dest[i].m_Position = Vec4(FromZeroOne(screenPosition), 1.0f);
        dest[i].m_Range = 1.0f/screenRange;
        dest[i].m_Index = source->m_Index;
    }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    renderContext->m_ConicalLightMask = conicalLightMask;
}

// -------------------------------------------------------------------------------------------------
void RenderUpdateCylindricalLights(RenderContext* renderContext, const Light* cylindricalLights, int numCylindricalLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_CylindricalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    
    uint32_t cylindricalLightMask = 0;
    
    Light* base = (Light*)p;
    for (int i=0; i<numCylindricalLights; ++i)
    {
        const Light* source = &cylindricalLights[i];
        Light* dest = &base[source->m_Index];
        
        *dest = *source;
        
        cylindricalLightMask |= 1ULL<<source->m_Index;
        
        const Vec3 adjustedLightPosition1 = Vec3(source->m_Position.xy(), kLightZ);
        dest->m_Position = FromZeroOne(RenderGetScreenPos(renderContext, adjustedLightPosition1)).xyz1();
        
        const Vec3 adjustedLightPosition2 = Vec3(source->m_Position.xy() + dest->m_Direction.xy()*dest->m_Range, kLightZ);
        dest->m_Direction = FromZeroOne(RenderGetScreenPos(renderContext, adjustedLightPosition2)).xyz1();
        dest->m_Range = 1.0f / (dest->m_Position.xy() - dest->m_Direction.xy()).Length();
        
        float cameraDist = (adjustedLightPosition1.xyz1() * renderContext->m_View).z();
        dest->m_OrthogonalRange = (-cameraDist / dest->m_OrthogonalRange);
        
        dest->m_Index = cylindricalLights[i].m_Index;
    }
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    renderContext->m_CylindricalLightMask = cylindricalLightMask;
}

// -------------------------------------------------------------------------------------------------
// RenderUpdateDirectionalLights
void RenderUpdateDirectionalLights(RenderContext* renderContext, const Light* directionalLights, int numDirectionalLights)
{
    glBindBuffer(GL_UNIFORM_BUFFER, renderContext->m_DirectionalLightUbo);
    glBufferData(GL_UNIFORM_BUFFER, Light::kMaxLights*sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    
    GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY|GL_MAP_UNSYNCHRONIZED_BIT);
    memcpy(p, directionalLights, numDirectionalLights*sizeof(Light));
    
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    renderContext->m_NumDirectionalLights = numDirectionalLights;
}

// -------------------------------------------------------------------------------------------------
void RenderOptionsInit(RenderOptions* renderOptions, int width, int height)
{
    renderOptions->m_CameraType = RenderOptions::kPerspective;
    renderOptions->m_Width = width;
    renderOptions->m_Height = height;
}

// -------------------------------------------------------------------------------------------------
void RenderDumpModel(const ModelInstance* model)
{
    Printf("model:\n");
    MatrixDump(model->m_Po, "    ");

    for (int j=0; j<model->m_ModelClass->m_NumSubsets; ++j)
    {
        Printf("    subset: %d\n", j);

        const ModelClassSubset& modelClassSubset = model->m_ModelClass->m_Subsets[j];
        Printf("    numVertices: %d\n", modelClassSubset.m_NumVertices);
        Printf("    numIndices: %d\n", modelClassSubset.m_NumIndices);
        
        for (int i=0; i<modelClassSubset.m_NumIndices; ++i)
        {
            const SimpleVertex* vertex = &modelClassSubset.m_Vertices[modelClassSubset.m_Indices[i]];
            Vec3 in{vertex->m_Position[0], vertex->m_Position[1], vertex->m_Position[2]};
            Vec4 out;
            MatrixMultiplyVec(&out, model->m_Po, in.xyz1());
            Printf("    point[%d] = { %.2f, %.2f, %.2f }\n", i, out[0], out[1], out[2]);
        }
    }
}

// -------------------------------------------------------------------------------------------------
void RenderDumpModelTransformed(const ModelInstance* model, const Mat4& a)
{
    Printf("model:\n");
    MatrixDump(a, "    ");

    for (int j=0; j<model->m_ModelClass->m_NumSubsets; ++j)
    {
        const ModelClassSubset* modelClassSubset = &model->m_ModelClass->m_Subsets[j];
        
        Printf("    subset: %d\n", j);
        
        Printf("    numVertices: %d\n", modelClassSubset->m_NumVertices);
        Printf("    numIndices: %d\n", modelClassSubset->m_NumIndices);
        
        for (int i=0; i<modelClassSubset->m_NumIndices; ++i)
        {
            const SimpleVertex* vertex = modelClassSubset->m_Vertices + modelClassSubset->m_Indices[i];
            Vec4 out;
            MatrixMultiplyVec(&out, a, Vec3(vertex->m_Position).xyz1());
            Printf("    point[%d] = [ %.2f, %.2f, %.2f, 1 ] -> [ %.2f, %.2f, %.2f, 1 ]\n",
                   i, vertex->m_Position[0], vertex->m_Position[1], vertex->m_Position[2], out[0], out[1], out[2]);
        }
    }
}

// -------------------------------------------------------------------------------------------------
// RenderSetBlendMode
//
// Set the blend mode
void RenderSetBlendMode(Material::BlendMode blendMode)
{
    GL_ERROR_SCOPE();
    
    switch (blendMode)
    {
        case Material::BlendMode::kOpaque:
        {
            glDisable(GL_COLOR_LOGIC_OP);
            glDepthMask(GL_TRUE);
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            
            glDisable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ZERO);
            
            break;
        }
        case Material::BlendMode::kCutout:
        case Material::BlendMode::kBlend:
        {
            glDisable(GL_COLOR_LOGIC_OP);
            glDepthMask(GL_FALSE);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            
            break;
        }
        case Material::BlendMode::kOr:
        {
            glDepthMask(GL_FALSE);
            glDisable(GL_BLEND);
            glEnable(GL_COLOR_LOGIC_OP);
            glLogicOp(GL_OR);
            
            break;
        }
    }
}

// -------------------------------------------------------------------------------------------------
// RenderResetLightIndices
//
// Each frame, we generate light indices for all our lights.  It requires we reset
// them before we do our frame-ly iteration
void RenderResetLightIndices(RenderContext* renderContext)
{
    renderContext->m_LightIndex = 0;
}

// -------------------------------------------------------------------------------------------------
static inline void RenderUseProgram(const Shader* shader)
{
    // Printf("Using %s\n", shader->m_DebugName);
    if (shader)
        glUseProgram(shader->m_ProgramName);
}

// -------------------------------------------------------------------------------------------------
// RenderUseMaterial
//
void RenderUseMaterial(RenderContext* renderContext, int* textureSlotItr, const Material* material)
{
    GL_ERROR_SCOPE();
    
    const Shader* replacementShader = renderContext->m_ReplacementShader;
    if (replacementShader == nullptr)
    {
        RenderUseProgram(material->m_Shader);
        
        if (true || renderContext->m_CachedMaterial != material)
        {
            renderContext->m_CachedMaterial = material;
            RenderSetMaterialConstants(renderContext, textureSlotItr, material);
            RenderSetBlendMode(material->m_BlendMode);
        }
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, material->m_Texture->m_TextureId);
    }
}

// -------------------------------------------------------------------------------------------------
// RenderSetGlobalConstants
//
void RenderSetGlobalConstants(RenderContext* renderContext, int* textureSlotItr, int programName)
{    
    for (int i=0; i<renderContext->m_MaterialProperties.kMaxSize; ++i)
    {
        const Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[i];
        RenderSetMaterialProperty(textureSlotItr, programName, materialProperty);
    }
}

// -------------------------------------------------------------------------------------------------
// RenderSetMaterialConstants
//
void RenderSetMaterialConstants(RenderContext* renderContext, int* textureSlotItr, const Material* material)
{
    GL_ERROR_SCOPE();
    
    const Shader* shader = material->m_Shader;
    
    glActiveTexture(GL_TEXTURE0);
    
    if (material->m_Texture)
        glBindTexture(GL_TEXTURE_2D, material->m_Texture->m_TextureId);
    else
        glBindTexture(GL_TEXTURE_2D, 0);
    
    GLint mainTextureSlot = glGetUniformLocation(shader->m_ProgramName, "_MainTex");
    glProgramUniform1i(shader->m_ProgramName, mainTextureSlot, 0);
    
    GLint cameraPosSlot = glGetUniformLocation(shader->m_ProgramName, "_CameraPos");
    if (cameraPosSlot >= 0)
    {
        const Vec4& cameraPos = renderContext->m_Camera.GetTranslation();
        glUniform4f(cameraPosSlot, cameraPos.m_X[0], cameraPos.m_X[1], cameraPos.m_X[2], cameraPos.m_X[2]);
    }
    
    for (int i=0; i<material->m_NumMaterialProperties; ++i)
    {
        const Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[i];
        RenderSetMaterialProperty(textureSlotItr, shader->m_ProgramName, materialProperty);
    }
}

// -------------------------------------------------------------------------------------------------
// RenderFrameInit
//
void RenderFrameInit(RenderContext* renderContext)
{
    GL_ERROR_SCOPE();

    glBindFramebuffer(GL_FRAMEBUFFER, renderContext->m_FrameBufferIds[0]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glfwMakeContextCurrent(renderContext->m_Window);
    
    float currentTime = (float) glfwGetTime()*0.125f;
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
    
    // update shader time
    RenderGlobalSetVector(renderContext, renderContext->m_ShaderTimeIndex, Vec4(currentTime, currentTime, currentTime, currentTime));
}

// -------------------------------------------------------------------------------------------------
// RenderAttachPostEffect
//
void RenderAttachPostEffect(RenderContext* renderContext, PostEffect* effect)
{
    if (renderContext->m_NumPostEffects != renderContext->m_MaxNumPostEffects)
        renderContext->m_PostEffects[renderContext->m_NumPostEffects++] = effect;
}

// -------------------------------------------------------------------------------------------------
// RenderFullscreenSetVertexAttributes
//
void RenderFullscreenSetVertexAttributes(RenderContext* renderContext)
{
    glBindBuffer(GL_ARRAY_BUFFER, renderContext->m_QuadVertexBufferId);
    
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

// -------------------------------------------------------------------------------------------------
void RenderSetProcessKeysCallback(RenderContext* context, ProcessKeysCallback (*processKeysCallback))
{
    // auto wrapper = [] (GLFWwindow* window, int key, int scanCode, int action, int mods)
    // {
    //     processKeysCallback(key, scanCode, action, mode);
    // };
    
    glfwSetKeyCallback(context->m_Window, (GLFWkeyfun) processKeysCallback);
}

// -------------------------------------------------------------------------------------------------
void RenderSetProcessMouseCallback(RenderContext* context, ProcessMouseCallback (*processMouseCallback))
{
    glfwSetMouseButtonCallback(context->m_Window, (GLFWmousebuttonfun) processMouseCallback);
}

// -------------------------------------------------------------------------------------------------
void RenderSetProcessCursorCallback(RenderContext* context, ProcessCursorCallback (*processCursorCallback))
{
    glfwSetCursorPosCallback(context->m_Window, (GLFWcursorposfun) processCursorCallback);
}

// -------------------------------------------------------------------------------------------------
void RenderDumpUniforms(RenderContext* renderContext, const Shader* shader)
{
#if 0
    const int program_id = shader->m_ProgramName;
    
    int max= 0;
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &max);
    Printf("JIV MAX %d\n", max);
    
    // http://stackoverflow.com/questions/4783912/how-can-i-find-a-list-of-all-the-uniforms-in-opengl-es-2-0-vertex-shader-pro
    int total = -1;
    glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &total);
    for (int i=0; i<total; ++i)
    {
        int name_len=-1, num=-1;
        GLenum type = GL_ZERO;
        char name[128];
        glGetActiveUniform(program_id, GLuint(i), sizeof(name)-1, &name_len, &num, &type, name);
        name[name_len] = 0;
        GLuint location = glGetUniformLocation(program_id, name);
        Printf("JIV %s %d\n", name, location);
    }
#endif
}

// -------------------------------------------------------------------------------------------------
void RenderSetLightConstants(RenderContext* renderContext, const Shader* shader)
{
    GL_ERROR_SCOPE();
    
    GLuint pointLightsBlockIndex = shader->m_PointLightBlockIndex;
    if (pointLightsBlockIndex != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->m_ProgramName, pointLightsBlockIndex, kPointLightBinding);
        
        GLint pointLightMaskIndex = glGetUniformLocation(shader->m_ProgramName, "pointLightMask");
        if (pointLightMaskIndex != GL_INVALID_INDEX)
            glUniform1ui(pointLightMaskIndex, renderContext->m_PointLightMask);
    }
    
    GLuint conicalLightsBlockIndex = shader->m_ConicalLightBlockIndex;
    if (conicalLightsBlockIndex != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->m_ProgramName, conicalLightsBlockIndex, kConicalLightBinding);
        
        GLint conicalLightMaskIndex = glGetUniformLocation(shader->m_ProgramName, "conicalLightMask");
        if (conicalLightMaskIndex != GL_INVALID_INDEX)
            glUniform1ui(conicalLightMaskIndex, renderContext->m_ConicalLightMask);
    }
    
    GLuint cylindricalLightsBlockIndex = shader->m_CylindricalLightBlockIndex;
    if (cylindricalLightsBlockIndex != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->m_ProgramName, cylindricalLightsBlockIndex, kCylindricalLightBinding);
        
        GLint cylindricalLightMaskIndex = glGetUniformLocation(shader->m_ProgramName, "cylindricalLightMask");
        if (cylindricalLightMaskIndex != GL_INVALID_INDEX)
            glUniform1ui(cylindricalLightMaskIndex, renderContext->m_CylindricalLightMask);
    }
    
    GLuint directionalLightsBlockIndex = shader->m_DirectionalLightBlockIndex;
    if (directionalLightsBlockIndex != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->m_ProgramName, directionalLightsBlockIndex, kDirectionalLightBinding);
        
        GLint direcitonalLightNumIndex = glGetUniformLocation(shader->m_ProgramName, "numDirectionalLights");
        if (direcitonalLightNumIndex != GL_INVALID_INDEX)
            glUniform1ui(direcitonalLightNumIndex, renderContext->m_NumDirectionalLights);
    }
}

// -------------------------------------------------------------------------------------------------
void RenderDrawFullscreen(RenderContext* renderContext, Material* material, int textureId)
{
    GL_ERROR_SCOPE();
    
    const Shader* shader = material->m_Shader;
    
    if (material == nullptr || shader == nullptr)
        return;
    
    RenderFullscreenSetVertexAttributes(renderContext);
    
    RenderUseProgram(shader);
    
    int textureSlotItr = 1;
    RenderSetMaterialConstants(renderContext, &textureSlotItr, material);
    RenderSetBlendMode(material->m_BlendMode);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // global constants
    RenderSetGlobalConstants(renderContext, &textureSlotItr, shader->m_ProgramName);
    
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
    
    RenderSetLightConstants(renderContext, shader);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// -------------------------------------------------------------------------------------------------
// RenderDrawFullscreen
//
void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, int textureId)
{
    GL_ERROR_SCOPE();
    
    RenderUseProgram(shader);
    
    RenderFullscreenSetVertexAttributes(renderContext);

    // overwrite existing contents
    RenderSetBlendMode(Material::BlendMode::kOpaque);
    glDisable(GL_DEPTH_TEST);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // global constants
    int textureSlotItr=1;
    RenderSetGlobalConstants(renderContext, &textureSlotItr, shader->m_ProgramName);
    
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
    
    RenderSetLightConstants(renderContext, shader);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// -------------------------------------------------------------------------------------------------
void RenderDrawFullscreen(RenderContext* renderContext, Shader* shader, Texture* texture)
{
    GL_ERROR_SCOPE();
    
    int textureId = renderContext->m_FrameBufferColorIds[0];
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    
    RenderDrawFullscreen(renderContext, shader, textureId);
}

// -------------------------------------------------------------------------------------------------
// RenderDrawFullscreen
void RenderDrawFullscreen(RenderContext* renderContext, Material* material, Texture* texture)
{
    GL_ERROR_SCOPE();
    
    int textureId = renderContext->m_FrameBufferColorIds[0];
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    
    RenderDrawFullscreen(renderContext, material, textureId);
}

// -------------------------------------------------------------------------------------------------
void RenderSetRenderTarget(RenderContext* renderContext, Texture* texture)
{
    if (texture && texture->m_FrameBufferId>0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, texture->m_FrameBufferId);
        glViewport(0, 0, texture->m_Width, texture->m_Height);

        switch (texture->m_RenderTextureFlags & (Texture::RenderTextureFlags::kClearColor|Texture::RenderTextureFlags::kClearDepth))
        {
            case Texture::RenderTextureFlags::kClearColor|Texture::RenderTextureFlags::kClearDepth:
            {
                glClearColor(texture->m_ClearColor.m_X[0], texture->m_ClearColor.m_X[1], texture->m_ClearColor.m_X[2], 0);
                glClear(GL_COLOR_BUFFER_BIT);
                break;
            }
            case Texture::RenderTextureFlags::kClearColor:
            {
                glClearColor(texture->m_ClearColor.m_X[0], texture->m_ClearColor.m_X[1], texture->m_ClearColor.m_X[2], 0);
                glClear(GL_COLOR_BUFFER_BIT);
                break;
            }
            case Texture::RenderTextureFlags::kClearDepth:
            {
                glClearColor(texture->m_ClearColor.m_X[0], texture->m_ClearColor.m_X[1], texture->m_ClearColor.m_X[2], 0);
                glClear(GL_COLOR_BUFFER_BIT);
                break;
            }
        }
        
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
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

// -------------------------------------------------------------------------------------------------
void RenderClearReplacementShader(RenderContext* renderContext)
{
    renderContext->m_ReplacementShader = nullptr;
}

// -------------------------------------------------------------------------------------------------
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
        
        // jiv fixme: set blend mode externally
        RenderSetBlendMode(Material::BlendMode::kBlend);
    }
}

// -------------------------------------------------------------------------------------------------
// RenderFrameEnd
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
        glDisable(GL_DEPTH_TEST);
        RenderDrawFullscreen(renderContext, g_SimpleShader, renderContext->m_FrameBufferColorIds[0]);
    }
    
    if (glfwWindowShouldClose(renderContext->m_Window))
        ret = false;
    
    // poll events
    glfwSwapBuffers(renderContext->m_Window);
    glfwPollEvents();
    
    return ret;
}

bool g_Trace = false;

// -------------------------------------------------------------------------------------------------
// RenderGetScreenPos
Vec4 RenderGetScreenPos(const Mat4& view, const Mat4& proj, float width, float height, const Vec3& worldPos)
{
    Vec4 temp = worldPos.xyz1() * view;
    
    // Vec4 ret = proj * temp;
    Vec4 ret = temp * proj;
    
    const float two_w = 2.0f * ret.w();

    if (g_Trace)
        printf("ret: (%f %f)\n", ret[0], ret[1]);
    
    ret.m_X[0] = (ret.m_X[0] * width)  / (two_w) + width*0.5f;
    ret.m_X[1] = (ret.m_X[1] * height) / (two_w) + height*0.5f;
    ret.m_X[0] /= width;
    ret.m_X[1] /= height;

    return ret;
}

// -------------------------------------------------------------------------------------------------
// RenderGetScreenPos
Vec4 RenderGetScreenPos(const RenderContext* renderContext, const Vec3& worldPos)
{
    return RenderGetScreenPos(renderContext->m_View, renderContext->m_Projection, renderContext->m_Width, renderContext->m_Height, worldPos);
}

// -------------------------------------------------------------------------------------------------
// RenderGetWorldPos
Vec3 RenderGetWorldPos(const RenderContext* renderContext, const Vec2& screenPos, float z)
{
    //Mat4 temp = (renderContext->m_View * renderContext->m_Projection);
    Mat4 temp = (renderContext->m_Projection * renderContext->m_View);
    Mat4 viewToProj;
    MatrixInvert(&viewToProj, temp);
    
    Vec2 temp0 = screenPos * 2.0f - Vec2(1.0f, 1.0f);
    temp0 *= Vec2(renderContext->m_Width, renderContext->m_Height);
    Vec4 ret = Vec4(temp0, 0.0f, 0.1f) * viewToProj;
    return ret.xyz();
}

// -------------------------------------------------------------------------------------------------
// RenderGenerateSprite
// 
ModelInstance* RenderGenerateSprite(RenderContext* renderContext, const SpriteOptions& spriteOptions, Material* material)
{
    GL_ERROR_SCOPE();
    
    ModelClass* modelClass = ModelClassAllocateSpawned(1);
    modelClass->m_RefCount = 1;
    
    const float halfWidth = material->m_Texture->m_Width / (2.0f * spriteOptions.m_PixelsPerUnit);
    const float halfHeight = material->m_Texture->m_Height / (2.0f * spriteOptions.m_PixelsPerUnit);
    
    ModelClassSubset* modelClassSubset = &modelClass->m_Subsets[0];
    modelClassSubset->m_Material = material;
    
    int tintColorIndex = material->GetPropertyIndex("TintColor");
    if (tintColorIndex >= 0)
        material->SetVector(tintColorIndex, spriteOptions.m_TintColor);
    
    modelClassSubset->m_NumVertices = 4;
    modelClassSubset->m_Vertices = new SimpleVertex[modelClassSubset->m_NumVertices];
    modelClassSubset->m_Indices = new unsigned short[6];
    
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
    SimpleVertex* simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffff00ff;
    simpleVertex->m_Position[0] = leftOffset; simpleVertex->m_Position[1] = topOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;     simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  0.0f;      simpleVertex->m_Uv[1]       =  0.0f;
    
    // 1
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = rightOffset; simpleVertex->m_Position[1] = topOffset;  simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;       simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  1.0f;      simpleVertex->m_Uv[1]       =  0.0f;
    
    // 2
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = rightOffset; simpleVertex->m_Position[1] = bottomOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;         simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  1.0f;      simpleVertex->m_Uv[1]       =  1.0f;
    
    // 3
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffffffff;
    simpleVertex->m_Position[0] = leftOffset; simpleVertex->m_Position[1] = bottomOffset; simpleVertex->m_Position[2] = 0.0f;
    simpleVertex->m_Normal[0]   =  0.0f;      simpleVertex->m_Normal[1]   =  0.0f;        simpleVertex->m_Normal[2]   = +1.0f;
    simpleVertex->m_Uv[0]       =  0.0f;      simpleVertex->m_Uv[1]       =  1.0f;
    
    // front
    int numIndices = 0;
    
    modelClassSubset->m_Indices[numIndices++] = 0;
    modelClassSubset->m_Indices[numIndices++] = 3;
    modelClassSubset->m_Indices[numIndices++] = 2;
    
    modelClassSubset->m_Indices[numIndices++] = 2;
    modelClassSubset->m_Indices[numIndices++] = 1;
    modelClassSubset->m_Indices[numIndices++] = 0;
    
    // save number of elements
    modelClassSubset->m_NumIndices = numIndices;
    
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
                          2,                                          // x/y/z
                          GL_FLOAT,                                   // type
                          GL_FALSE,                                   // normalize?
                          sizeof(SimpleVertex),                       // stride
                          BUFFER_OFFSETOF(SimpleVertex, m_Uv));       // offset in buffer data
    
    glGenBuffers(1, &modelClassSubset->m_IndexBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_IndexBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_NumIndices*sizeof(unsigned short), modelClassSubset->m_Indices, GL_STATIC_DRAW);

    // calculate bsphere
    ModelClassSubsetCalcBSphere(modelClassSubset);
    
    ModelInstance* modelInstance = new ModelInstance();
    MatrixMakeIdentity(&modelInstance->m_Po);
    modelInstance->m_ModelClass = modelClass;
    
    return modelInstance;
}

// -------------------------------------------------------------------------------------------------
// RenderGenerateCube
void RenderGenerateCubeModelClass(RenderContext* renderContext, ModelClass* modelClass)
{
    ModelClassSubset* modelClassSubset = &modelClass->m_Subsets[0];
    float halfWidth = 0.5f;
    
    modelClassSubset->m_Material = MaterialCreate(g_SimpleShader, renderContext->m_WhiteTexture);
    
    modelClassSubset->m_NumVertices = 8;
    modelClassSubset->m_Vertices = new SimpleVertex[modelClassSubset->m_NumVertices];
    modelClassSubset->m_Indices = new unsigned short[36];
    
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
    SimpleVertex* simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0x00ffff00;
    simpleVertex->m_Position[0] = -halfWidth; // 0
    simpleVertex->m_Position[1] = +halfWidth; // 0
    simpleVertex->m_Position[2] = +halfWidth; // 0
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0x0000ffff;
    simpleVertex->m_Position[0] = +halfWidth; // 1
    simpleVertex->m_Position[1] = +halfWidth; // 1
    simpleVertex->m_Position[2] = +halfWidth; // 1
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0x00ff00ff;
    simpleVertex->m_Position[0] = +halfWidth; // 2
    simpleVertex->m_Position[1] = +halfWidth; // 2
    simpleVertex->m_Position[2] = -halfWidth; // 2
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xff00ffff;
    simpleVertex->m_Position[0] = -halfWidth; // 3
    simpleVertex->m_Position[1] = +halfWidth; // 3
    simpleVertex->m_Position[2] = -halfWidth; // 3
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = +1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xffff00ff;
    simpleVertex->m_Position[0] = -halfWidth; // 4
    simpleVertex->m_Position[1] = -halfWidth; // 4
    simpleVertex->m_Position[2] = -halfWidth; // 4
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xf0fff0ff;
    simpleVertex->m_Position[0] = +halfWidth; // 5
    simpleVertex->m_Position[1] = -halfWidth; // 5
    simpleVertex->m_Position[2] = -halfWidth; // 5
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = -1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0x0ffff0ff;
    simpleVertex->m_Position[0] = +halfWidth; // 6
    simpleVertex->m_Position[1] = -halfWidth; // 6
    simpleVertex->m_Position[2] = +halfWidth; // 6
    simpleVertex->m_Normal[0] = +1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    simpleVertex = &modelClassSubset->m_Vertices[numPositions++];
    simpleVertex->m_Color = 0xf0fff0ff;
    simpleVertex->m_Position[0] = -halfWidth; // 7
    simpleVertex->m_Position[1] = -halfWidth; // 7
    simpleVertex->m_Position[2] = +halfWidth; // 7
    simpleVertex->m_Normal[0] = -1.0f;
    simpleVertex->m_Normal[1] = -1.0f;
    simpleVertex->m_Normal[2] = +1.0f;
    
    // front
    int numIndices = 0;
    
    // top
    modelClassSubset->m_Indices[numIndices++] = 0;
    modelClassSubset->m_Indices[numIndices++] = 1;
    modelClassSubset->m_Indices[numIndices++] = 2;
    
    modelClassSubset->m_Indices[numIndices++] = 2;
    modelClassSubset->m_Indices[numIndices++] = 3;
    modelClassSubset->m_Indices[numIndices++] = 0;
    
    // left side
    modelClassSubset->m_Indices[numIndices++] = 0;
    modelClassSubset->m_Indices[numIndices++] = 3;
    modelClassSubset->m_Indices[numIndices++] = 4;
    
    modelClassSubset->m_Indices[numIndices++] = 4;
    modelClassSubset->m_Indices[numIndices++] = 7;
    modelClassSubset->m_Indices[numIndices++] = 0;
    
    // bottom
    modelClassSubset->m_Indices[numIndices++] = 5;
    modelClassSubset->m_Indices[numIndices++] = 6;
    modelClassSubset->m_Indices[numIndices++] = 7;
    
    modelClassSubset->m_Indices[numIndices++] = 7;
    modelClassSubset->m_Indices[numIndices++] = 4;
    modelClassSubset->m_Indices[numIndices++] = 5;
    
    // right side
    modelClassSubset->m_Indices[numIndices++] = 2;
    modelClassSubset->m_Indices[numIndices++] = 1;
    modelClassSubset->m_Indices[numIndices++] = 6;
    
    modelClassSubset->m_Indices[numIndices++] = 6;
    modelClassSubset->m_Indices[numIndices++] = 5;
    modelClassSubset->m_Indices[numIndices++] = 2;
    
    // 
    // 
    // back
    modelClassSubset->m_Indices[numIndices++] = 0;
    modelClassSubset->m_Indices[numIndices++] = 7;
    modelClassSubset->m_Indices[numIndices++] = 6;
    
    modelClassSubset->m_Indices[numIndices++] = 6;
    modelClassSubset->m_Indices[numIndices++] = 1;
    modelClassSubset->m_Indices[numIndices++] = 0;
    
    // front
    modelClassSubset->m_Indices[numIndices++] = 3;
    modelClassSubset->m_Indices[numIndices++] = 2;
    modelClassSubset->m_Indices[numIndices++] = 5;
    
    modelClassSubset->m_Indices[numIndices++] = 5;
    modelClassSubset->m_Indices[numIndices++] = 4;
    modelClassSubset->m_Indices[numIndices++] = 3;
    
    assert(numIndices <= 36);
    
    // save number of elements
    modelClassSubset->m_NumIndices = numIndices;
    
    // generate vertex name
    glGenVertexArrays(1, &modelClassSubset->m_VaoName);
    glBindVertexArray(modelClassSubset->m_VaoName);
    
    // generate vertex buffer name
    glGenBuffers(1, &modelClassSubset->m_VertexBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, modelClassSubset->m_VertexBufferName);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex)*modelClassSubset->m_NumVertices, modelClassSubset->m_Vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &modelClassSubset->m_IndexBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_IndexBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_NumIndices*sizeof(unsigned short), modelClassSubset->m_Indices, GL_STATIC_DRAW);

    // update bsphere
    ModelClassSubsetCalcBSphere(modelClassSubset);
}

// -------------------------------------------------------------------------------------------------
ModelInstance* RenderGenerateCube(RenderContext* renderContext, float halfWidth)
{
    return nullptr;
    
    ModelInstance* modelInstance = new ModelInstance();
    MatrixMakeIdentity(&modelInstance->m_Po);
    modelInstance->m_Po.Scale(Vec3(2.0f*halfWidth, 2.0f*halfWidth, 2.0f*halfWidth));
    modelInstance->m_ModelClass = ModelClassFind(kModelClassBuiltinCube);
    return modelInstance;
}

// -------------------------------------------------------------------------------------------------
// ModelInstanceDestroy
//
void ModelInstanceDestroy(ModelInstance* simpleModel)
{
    if (simpleModel == nullptr)
        return;
    
    ModelClassDestroy(simpleModel->m_ModelClass);
    delete simpleModel;
}

// -------------------------------------------------------------------------------------------------
// ModelInstanceSetVertexAttributes
//
//
void ModelInstanceSetVertexAttributes(const ModelClassSubset* modelClassSubset)
{
    GL_ERROR_SCOPE();
    
    glBindBuffer(GL_ARRAY_BUFFER, modelClassSubset->m_VertexBufferName);
    
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
                          GL_TRUE,                                    // normalize?
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
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelClassSubset->m_IndexBufferName);
}

// -------------------------------------------------------------------------------------------------
// RenderAddGlobalProperty
int RenderAddGlobalProperty(RenderContext* renderContext, const char* materialPropertyName, Material::MaterialPropertyType type)
{
    int index = -1;
    
    Material::MaterialProperty* materialProperty = renderContext->m_MaterialProperties.Alloc();
    if (materialProperty)
    {
        materialProperty->m_Type = type;
        strncpy(materialProperty->m_Key, materialPropertyName, sizeof materialProperty->m_Key-1);
        materialProperty->m_Key[sizeof materialProperty->m_Key-1] = '\0';
        
        index = renderContext->m_MaterialProperties.IndexOf(materialProperty);
    }
    
    return index;
}

// -------------------------------------------------------------------------------------------------
void RenderGlobalSetFloat(RenderContext* renderContext, int index, float value)
{
    assert(index < renderContext->m_MaterialProperties.kMaxSize);
    Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kFloat);
    materialProperty->m_Float = value;
}

// -------------------------------------------------------------------------------------------------
void RenderGlobalSetInt(RenderContext* renderContext, int index, int value)
{
    assert(index < renderContext->m_MaterialProperties.kMaxSize);
    Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kUInt);
    materialProperty->m_Int = value;
}

// -------------------------------------------------------------------------------------------------
void RenderGlobalSetVector(RenderContext* renderContext, int index, Vec4 value)
{
    assert(index < renderContext->m_MaterialProperties.kMaxSize);
    Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kVec4);
    materialProperty->m_Vector = value;
}

// -------------------------------------------------------------------------------------------------
void RenderGlobalSetMatrix(RenderContext* renderContext, int index, const Mat4& value)
{
    assert(index < renderContext->m_MaterialProperties.kMaxSize);
    Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kMat4);
    materialProperty->m_Matrix = value;
}

// -------------------------------------------------------------------------------------------------
void RenderGlobalSetTexture(RenderContext* renderContext, int index, int textureId)
{
    assert(index < renderContext->m_MaterialProperties.kMaxSize);
    Material::MaterialProperty* materialProperty = &renderContext->m_MaterialProperties[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kTexture);
    materialProperty->m_TextureId = textureId;
}

// -------------------------------------------------------------------------------------------------
// RenderGlobalSetTexture
void RenderGlobalSetTexture(RenderContext* renderContext, int index, Texture* texture)
{
    int textureId = 0;
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    RenderGlobalSetTexture(renderContext, index, textureId);
}

// -------------------------------------------------------------------------------------------------
uint32_t RenderModelSubsetGetSortKey(const ModelClassSubset* modelClassSubset)
{
    uint32_t materialIdBits = 0;
    uint32_t blendModeBit = 0;
    uint32_t positionBits = 0;
    
    const BSphere& bsphere = modelClassSubset->m_BSphere;
    const float minz = bsphere.z() - bsphere.radius();
    
    if (modelClassSubset->m_Material->m_BlendMode != Material::BlendMode::kOpaque)
    {
        blendModeBit = 0x80000000;
        positionBits = ((unsigned int) (minz * 128.0f)) & 0x7fff;
    }
    else
    {
        uint32_t textureId = 0;
        textureId ^= modelClassSubset->m_Material->m_Texture->m_TextureId;
        textureId = ((textureId>>16) ^ (textureId&0xffff))&0xffff;
        
        materialIdBits = textureId<<15;
    }
    
    uint32_t key = 0;
    key = blendModeBit | positionBits | materialIdBits;
    return key;
}

// -------------------------------------------------------------------------------------------------
// RenderDrawModelSubset
//
void RenderDrawModelSubset(RenderContext* renderContext, const Mat4& localToWorld, const ModelClassSubset* modelClassSubset)
{
    GL_ERROR_SCOPE();
    
    const Material* material = modelClassSubset->m_Material;
    const Shader* shader = material->m_Shader;
    
    if (renderContext->m_ReplacementShader)
        shader = renderContext->m_ReplacementShader;
    
    Mat4 normalModel;
    MatrixInvert(&normalModel, localToWorld);
    normalModel.Transpose();
    
    // use material
    int textureSlotItr = 1;
    RenderUseMaterial(renderContext, &textureSlotItr, material);
    
    // global constants
    RenderSetGlobalConstants(renderContext, &textureSlotItr, shader->m_ProgramName);
    
    GLint pi = glGetUniformLocation(shader->m_ProgramName, "project");
    glUniformMatrix4fv(pi, 1, GL_FALSE, renderContext->m_Projection.asFloat());
    
    GLint nmi = glGetUniformLocation(shader->m_ProgramName, "normalModel");
    glUniformMatrix4fv(nmi, 1, GL_FALSE, normalModel.asFloat());
    
    GLint l2wi = glGetUniformLocation(shader->m_ProgramName, "localToWorld");
    glUniformMatrix4fv(l2wi, 1, GL_FALSE, localToWorld.asFloat());
    
    Mat4 modelView;
    MatrixMultiply(&modelView, localToWorld, renderContext->m_View);
    
    GLint mvi = glGetUniformLocation(shader->m_ProgramName, "modelView");
    glUniformMatrix4fv(mvi, 1, GL_FALSE, modelView.asFloat());
    
    GLint viewIndex = glGetUniformLocation(shader->m_ProgramName, "view");
    glUniformMatrix4fv(viewIndex, 1, GL_FALSE, renderContext->m_View.asFloat());
    
    RenderSetLightConstants(renderContext, shader);
    
    ModelInstanceSetVertexAttributes(modelClassSubset);
    
    glDrawElements(GL_TRIANGLES, modelClassSubset->m_NumIndices, GL_UNSIGNED_SHORT, (void*) 0);
    glDisable(GL_BLEND);
}

// -------------------------------------------------------------------------------------------------
void ModelClassSubsetCalcBSphere(ModelClassSubset* inplace)
{
    if (inplace->m_NumIndices)
    {
        Vec3 centroid = Vec3(inplace->m_Vertices[inplace->m_Indices[0]].m_Position);
        float recip = 1.0f / inplace->m_NumIndices;
        for (int i=1; i<inplace->m_NumIndices; ++i)
        {
            const SimpleVertex& simpleVertex = inplace->m_Vertices[inplace->m_Indices[i]];
            centroid += Vec3(simpleVertex.m_Position) * recip;
        }
        
        float max_radius_squared = 0.0f;
        for (int i=0; i<inplace->m_NumIndices; ++i)
        {
            const SimpleVertex& simpleVertex = inplace->m_Vertices[inplace->m_Indices[i]];
            float radius_squared = DistanceSquared(centroid, Vec3(simpleVertex.m_Position));
            max_radius_squared = Max(radius_squared, max_radius_squared);
        }
        
        inplace->m_BSphere = BSphere(centroid.asFloat(), sqrtf(max_radius_squared));
    }
}

// -------------------------------------------------------------------------------------------------
void RenderSetClearColor(RenderContext* renderContext, Vec3 color)
{
    renderContext->m_ClearColor = color;
    glClearColor(color[0], color[1], color[2], 0.0f);
}
