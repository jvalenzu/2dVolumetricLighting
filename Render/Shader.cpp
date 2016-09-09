// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Engine/Matrix.h"
#include "Render/Model.h"
#include "Render/Render.h"
#include "Engine/Utils.h"
#include "Render/Material.h"
#include "Render/Asset.h"
#include "Render/Shader.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// internal singleton
struct ShaderManager : SimpleAssetManager<Shader>
{
    int m_Version;
    
    ShaderManager();
    Shader* CreateShader(const char* fname);
    void DestroyShader(Shader* victim);

    virtual void DumpTitle();
    virtual void DumpInternal();
};

ShaderManager::ShaderManager()
{
    float languageVersion;
    sscanf((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "%f", &languageVersion);
    m_Version = (int) (100 * languageVersion);
    
    Printf("OpenGL shader version: %d\n", m_Version);
}


void ShaderManager::DestroyShader(Shader* victim)
{
    // jiv fixme delete shader

    // destroy bookkeeping
    DestroyAsset(victim);
}

Shader* ShaderManager::CreateShader(const char* fname)
{
    const uint32_t crc = Djb(fname);

    Shader temp;
        
    // read vertex shader source
    char vshFilePath[512];
    snprintf(vshFilePath, sizeof vshFilePath, "%s.vsh", fname);
    char* vShaderSource = FileGetAsText(vshFilePath);
    if (vShaderSource == nullptr)
    {
        Printf("Unable to find %s\n", vshFilePath);
        return nullptr;
    }
        
    // read fragment shader source
    char fshFilePath[512];
    snprintf(fshFilePath, sizeof fshFilePath, "%s.fsh", fname);
    char* fShaderSource = FileGetAsText(fshFilePath);
    if (fShaderSource == nullptr)
    {
        Printf("Unable to find %s\n", fshFilePath);
        delete[] vShaderSource;
        return nullptr;
    }
        
    // Create a program object
    temp.m_ProgramName = glCreateProgram();
    
    // bind the attributes
    glBindAttribLocation(temp.m_ProgramName, kVertexAttributePosition, "inPosition");
    glBindAttribLocation(temp.m_ProgramName, kVertexAttributeColor, "inColor");
    glBindAttribLocation(temp.m_ProgramName, kVertexAttributeNormal, "inNormal");
    glBindAttribLocation(temp.m_ProgramName, kVertexAttributeTexCoord, "inTexCoord");
    
    // Get the size of the version preprocessor string info so we know 
    //  how much memory to allocate for our sourceString
    const GLsizei versionStringSize = sizeof("#version 123\n");
    
    // Specify and compile VertexShader
    
    // Prepend our vertex shader source string with the supported GLSL version so
    //  the shader will work on ES, Legacy, and OpenGL 3.2 Core Profile contexts
    char* sourceString = new char[strlen(vShaderSource)+100];
    sprintf(sourceString, "#version %d\n%s", m_Version, vShaderSource);
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar **)&sourceString, nullptr);
    glCompileShader(vertexShader);
    
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar* log = new GLchar[logLength];
            glGetShaderInfoLog(vertexShader, logLength, &logLength, log);
            Printf("Vtx Shader compile log:%s\n", log);
            delete[] log;
        }
            
        FPrintf(stderr, "Failed to compile vtx shader:\n%s\n", sourceString);
            
        delete[] fShaderSource;
        delete[] vShaderSource;
        delete[] sourceString;
            
        return nullptr;
    }
        
    delete [] sourceString;
    sourceString = nullptr;
        
    // Specify and compile Fragment Shader
        
    // Allocate memory for the source string including the version preprocessor information
    sourceString = new char[strlen(fShaderSource)+100];
        
    // Prepend our fragment shader source string with the supported GLSL version so the shader will work on ES, Legacy, and OpenGL 3.2 Core Profile contexts
    sprintf(sourceString, "#version %d\n%s", m_Version, fShaderSource);
        
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const GLchar **)&sourceString, nullptr);
    glCompileShader(fragShader);
    
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        FPrintf(stderr, "Failed to compile %s.fsh\n", fname);
        
        GLint logLength;
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) 
        {
            GLchar* log = new GLchar[logLength];
            glGetShaderInfoLog(fragShader, logLength, &logLength, log);
            Printf("Frag Shader compile log:\n%s\n", log);
            delete[] log;
        }
        
        delete[] fShaderSource;
        delete[] vShaderSource;
        delete[] sourceString;
        
        return nullptr;
    }
    
    delete [] sourceString;
    sourceString = nullptr;
    
    glAttachShader(temp.m_ProgramName, fragShader);
    glAttachShader(temp.m_ProgramName, vertexShader);
    
    GetGLError();
    
    glLinkProgram(temp.m_ProgramName);
    glGetProgramiv(temp.m_ProgramName, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        GLint logLength;
        glGetShaderiv(temp.m_ProgramName, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) 
        {
            GLchar* log = new GLchar[logLength];
            glGetProgramInfoLog(temp.m_ProgramName, logLength, &logLength, log);
            Printf("Frag Shader link log:\n%s\n", log);
            delete[] log;
        }
        
        delete[] fShaderSource;
        delete[] vShaderSource;
        delete[] sourceString;
        
        return nullptr;
    }
    
    glValidateProgram(temp.m_ProgramName);
    
    delete[] vShaderSource;
    delete[] fShaderSource;
    
    GetGLError();
    
    // jiv fixme strdup
    temp.m_DebugName = fname;
    
    Shader* ret = AllocateAsset(crc);
    *ret = temp;
    
    ret->m_PointLightBlockIndex = glGetUniformBlockIndex(ret->m_ProgramName, "PointLightData");
    ret->m_CylindricalLightBlockIndex = glGetUniformBlockIndex(ret->m_ProgramName, "CylindricalLightData");
    ret->m_ConicalLightBlockIndex = glGetUniformBlockIndex(ret->m_ProgramName, "ConicalLightData");
    
    return ret;
}


ShaderManager* g_ShaderManager;

// public API
Shader* ShaderCreate(const char* fname)
{
    const uint32_t crc = Djb(fname);
    const int index = g_ShaderManager->Find(crc);
    Shader* ret;
    
    if (index<0)
        ret = g_ShaderManager->CreateShader(fname);
    else
        ret = &g_ShaderManager->m_Assets[index];
    
    if (ret != nullptr)
    {
        ret->m_Crc = crc;
        ret->m_RefCount++;
    }
    
    return ret;
}

void ShaderDestroy(Shader* victim)
{
    if (victim == nullptr)
        return;
    
    if (--victim->m_RefCount == 0)
        g_ShaderManager->DestroyShader(victim);
}

// internal

Shader* g_SimpleShader;
Shader* g_SimpleTransparentShader;

void ShaderManager::DumpInternal()
{
    for (int i=0; i<m_NumAssets; ++i)
    {
        Shader& shader = m_Assets[i];
        Printf("%2d name %s crc 0x%x refCount %d\n", i, shader.m_DebugName, m_Crc[i], shader.m_RefCount);
    }
}

void ShaderInit()
{
    g_ShaderManager = new ShaderManager();
    
    g_SimpleShader = ShaderCreate("obj/Shader/Simple");
    g_SimpleTransparentShader = ShaderCreate("obj/Shader/SimpleTransparent");
}

void ShaderFini()
{
    ShaderDestroy(g_SimpleShader);
    ShaderDestroy(g_SimpleTransparentShader);
    
    g_SimpleShader = nullptr;
    g_SimpleTransparentShader = nullptr;
    
    g_ShaderManager->Dump();
    g_ShaderManager->Destroy();
    g_ShaderManager = nullptr;
}

void ShaderManager::DumpTitle()
{
    Printf("ShaderManager\n");
}
