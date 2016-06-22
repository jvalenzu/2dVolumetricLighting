#pragma once

#include <stdint.h>

struct Material;
struct Shader;
struct Texture;
struct RenderContext;

struct PostEffect
{
    Shader* m_Shader;
};

enum PostEffectResolution : uint32_t
{
    kFull,
    kHalf,
    kQuarter
};

PostEffect* PostEffectCreate(Shader* shader, PostEffectResolution resolution=kFull);
void        PostEffectDestroy(PostEffect* victim);
