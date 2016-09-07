// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "Render/GL.h"

#include <stdint.h>
#include "Engine/Matrix.h"

struct Texture
{
    enum Flags : uint32_t
    {
        kNone,
        kRenderTexture
    };
    
    enum RenderTextureFormat : uint32_t
    {
        kRgb,
        kRgba,
        kFloat,
        kUInt
    };
    
    enum RenderTextureFlags : uint32_t
    {
        kClearNone,
        kClearColor,
        kClearDepth
    };
    
    Flags m_Flags;
    RenderTextureFlags m_RenderTextureFlags;
    int32_t m_Depth;
    GLuint m_TextureId;
    GLuint m_FrameBufferId;
    int32_t m_Width;
    int32_t m_Height;
    Vec3 m_ClearColor;
    int m_RefCount;
    const char* m_DebugName;
    uint32_t m_Format;
    
    inline void Invalidate()
    {
        m_TextureId = -1;
        m_FrameBufferId = -1;
        m_DebugName = nullptr;
    }
    
    Texture() : m_RefCount(0), m_DebugName(nullptr)
    {
    }
    
    Texture(int width, int height) : m_Width(width), m_Height(height), m_RefCount(0), m_DebugName(nullptr)
    {
    }
    
    Texture(const Texture& rhs) :
        m_Flags(rhs.m_Flags),
        m_RenderTextureFlags(rhs.m_RenderTextureFlags),
        m_Depth(rhs.m_Depth),
        m_TextureId(rhs.m_TextureId),
        m_FrameBufferId(rhs.m_FrameBufferId),
        m_Width(rhs.m_Width),
        m_Height(rhs.m_Height),
        m_ClearColor(rhs.m_ClearColor), 
        m_RefCount(rhs.m_RefCount),
        m_DebugName(nullptr)
    {
    }

    void SetClearFlags(Texture::RenderTextureFlags renderTextureFlags, float r=0, float g=0, float b=0, float z=1.0f);
};

void      TextureInit();
void      TextureFini();

// create/destroy material
Texture*  TextureCreateFromFile(const char* path);
Texture*  TextureCreateRenderTexture(int width, int height, int depth, Texture::RenderTextureFormat format = Texture::RenderTextureFormat::kRgb);
void      TextureDestroy(Texture* victim);

Texture*  TextureRef(Texture* texture);
