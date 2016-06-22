#pragma once

#include <stdint.h>
#include <stdio.h>

struct Shader
{
    const char* m_DebugName;
    GLuint m_ProgramName;
    int m_RefCount;
    uint32_t m_Crc;
    
    Shader() : m_RefCount(0), m_Crc(0)
    {
    }
    
    inline void Invalidate()
    {
        // jiv fixme: delete?
        m_DebugName = nullptr;
        m_ProgramName = -1;
    }
};

// default simple shader
extern Shader* g_SimpleShader;
extern Shader* g_SimpleTransparentShader;

// create shader system
void      ShaderInit();
void      ShaderFini();

// create and destroy shader instance
Shader* ShaderCreate(const char* fname);
inline Shader* ShaderRef(Shader* shader)
{
    if (shader)
        shader->m_RefCount++;
    return shader;
}

void    ShaderDestroy(Shader* victim);


