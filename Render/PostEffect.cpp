// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Render/PostEffect.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/Model.h"
#include "Render/Render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PostEffect* PostEffectCreate(Shader* shader, PostEffectResolution resolution)
{
    PostEffect* ret = new PostEffect();
    ret->m_Shader = ShaderRef(shader);
    return ret;
}

void PostEffectDestroy(PostEffect* victim)
{
    ShaderDestroy(victim->m_Shader);
    delete victim;
}
