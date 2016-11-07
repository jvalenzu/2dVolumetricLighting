// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "Render/GL.h"
#include <stdint.h>
#include <stdio.h>

struct Shader
{
    const char* m_DebugName;
    GLuint m_ProgramName;
    int m_RefCount;
    uint32_t m_Crc;

    GLuint m_PointLightBlockIndex;
    GLuint m_ConicalLightBlockIndex;
    GLuint m_CylindricalLightBlockIndex;
    GLuint m_DirectionalLightBlockIndex;
    
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
extern Shader* g_SimpleColorShader;
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


