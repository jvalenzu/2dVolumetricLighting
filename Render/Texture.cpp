#include "Render/Asset.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/Render.h"
#include "Engine/Utils.h"
#include "Render/GL.h"

#include "lodepng.h"

struct TextureManager : SimpleAssetManager<Texture>
{
    Texture* CreateTexture(const char* fname, uint32_t crc);
    Texture* CreateRenderTexture(int width, int height, int depth, Texture::RenderTextureFormat format);
    void DestroyTexture(Texture* victim);
    
    virtual void DumpTitle();
    virtual void DumpInternal(const Texture* t);
};


TextureManager* g_TextureManager;

Texture* TextureRef(Texture* texture)
{
    if (texture)
        texture->m_RefCount++;
    return texture;
}

Texture* TextureManager::CreateRenderTexture(int width, int height, int depth, Texture::RenderTextureFormat format)
{
    GL_ERROR_SCOPE();
    
    Texture* texture = new Texture(width, height);
    texture->m_Flags = Texture::kRenderTexture;
    texture->m_Depth = depth;
    texture->m_TextureId = -1;
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture->m_TextureId);
    glBindTexture(GL_TEXTURE_2D, texture->m_TextureId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    if (format == Texture::RenderTextureFormat::kRgb)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    else if (format == Texture::RenderTextureFormat::kFloat)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &texture->m_FrameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, texture->m_FrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_TextureId, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // jiv fixme: should save/restore
    
    return texture;
}

Texture* TextureManager::CreateTexture(const char* filename, uint32_t crc)
{
    unsigned int width;
    unsigned int height;
    unsigned char* data;
    if (lodepng_decode32_file(&data, &width, &height, filename))
        return nullptr;
    
    Texture texture(width, height);
    texture.m_Flags = Texture::kNone;
    texture.m_FrameBufferId = -1;
    
    texture.m_DebugName = filename;
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture.m_TextureId);
    glBindTexture(GL_TEXTURE_2D, texture.m_TextureId);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    Texture* ret = AllocateAsset(crc);
    *ret = texture;
    return ret;
}

void TextureManager::DestroyTexture(Texture* victim)
{
    glDeleteTextures(1, &victim->m_TextureId);
    victim->m_TextureId = -1;
    
    victim->m_DebugName = nullptr;
    
    glDeleteTextures(1, &victim->m_TextureId);
    victim->m_TextureId = -1;
    
    if (victim->m_Flags == Texture::kRenderTexture)
    {
        glDeleteFramebuffers(1, &victim->m_FrameBufferId);
        victim->m_FrameBufferId = -1;
        delete victim;
        return;
    }
    
    DestroyAsset(victim);
}

void TextureManager::DumpInternal(const Texture* texture)
{
    Printf("name %s refCount %d\n", texture->m_DebugName, texture->m_RefCount);
}

Texture* TextureCreateFromFile(const char* filename)
{
    Texture* ret = nullptr;
    const uint32_t crc = Djb(filename);
    const int index = g_TextureManager->Find(crc);
    if (index<0)
        ret = g_TextureManager->CreateTexture(filename, crc);
    else
        ret = &g_TextureManager->m_Assets[index];
    if (ret != nullptr)
        ret->m_RefCount++;
    return ret;
}

Texture* TextureCreateRenderTexture(int width, int height, int depth, Texture::RenderTextureFormat format)
{
    return g_TextureManager->CreateRenderTexture(width, height, depth, format);
}

void TextureDestroy(Texture* victim)
{
    if (victim == nullptr)
        return;
    
    if (victim->m_Flags == Texture::kRenderTexture)
    {
        glDeleteTextures(1, &victim->m_TextureId);
        glDeleteFramebuffers(1, &victim->m_FrameBufferId);
        delete victim;
        return;
    }
    
#if !defined(WINDOWS)
    if (false)
    {
        Printf("%s\n", victim->m_DebugName);
        puts("\n");
        void* array[8192];
        char** symbols;
        int n = backtrace(array, 8192);
        symbols = backtrace_symbols(array, n);
        for (int i=0; i<n; ++i)
            Printf("%s\n", symbols[i]);
        free(symbols);
    }
#endif
    
    if (--victim->m_RefCount == 0)
        g_TextureManager->DestroyTexture(victim);
}

void TextureSetClearFlags(Texture* texture, Texture::RenderTextureFlags renderTextureFlags, float r, float g, float b, float z)
{
    texture->m_RenderTextureFlags = renderTextureFlags;
    VectorSet(&texture->m_ClearColor, r,g,b);
}

void TextureInit()
{
    g_TextureManager = new TextureManager();
}

void TextureFini()
{
    g_TextureManager->Dump();
    delete g_TextureManager;
    g_TextureManager = nullptr;
}

void TextureManager::DumpTitle()
{
    Printf("TextureManager\n");
}
