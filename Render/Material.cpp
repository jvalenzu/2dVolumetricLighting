#include <opengl/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Engine/Matrix.h"
#include "Render/Model.h"
#include "Render/Render.h"
#include "Engine/Utils.h"
#include "Render/Material.h"

// create/destroy material
Material* MaterialRef(Material* material)
{
    Material* ret = new Material();
    ret->m_BlendMode = material->m_BlendMode;
    ret->m_Shader = ShaderRef(material->m_Shader);
    ret->m_Texture = TextureRef(material->m_Texture);
    
    ret->m_NumMaterialProperties = material->m_NumMaterialProperties;
    ret->m_MaterialPropertyBlock = new Material::MaterialProperty[material->m_NumMaterialProperties];
    memcpy(ret->m_MaterialPropertyBlock, material->m_MaterialPropertyBlock, material->m_NumMaterialProperties*sizeof(Material::MaterialProperty));
    
    return ret;
}

// create/destroy material
Material* MaterialCreate(Shader* shader, Texture* texture)
{
    Material* material = new Material();
    material->m_BlendMode = Material::BlendMode::kCutout;
    material->m_Shader = ShaderRef(shader);
    material->m_Texture = TextureRef(texture);
    material->m_NumMaterialProperties = 0;
    material->m_MaterialPropertyBlock = nullptr;
    
    return material;
}

// MaterialDestroy
void MaterialDestroy(Material* victim)
{
    if (victim == nullptr)
        return;
    
    ShaderDestroy(victim->m_Shader);
    TextureDestroy(victim->m_Texture);
    
    delete [] victim->m_MaterialPropertyBlock;
    victim->m_MaterialPropertyBlock = nullptr;
    
    delete victim;
}

void RenderSetMaterialConstants(RenderContext* renderContext, const Material* material)
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
    
    GLint ambientLightColorSlot = glGetUniformLocation(shader->m_ProgramName, "_AmbientLight");
    glProgramUniform4f(shader->m_ProgramName, ambientLightColorSlot, renderContext->m_AmbientLightColor.m_X[0], renderContext->m_AmbientLightColor.m_X[1], renderContext->m_AmbientLightColor.m_X[2], renderContext->m_AmbientLightColor.m_X[3]);
    
    int textureSlotItr = 1;
    for (int i=0; i<material->m_NumMaterialProperties; ++i)
    {
        const Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[i];
        GLint slot = glGetUniformLocation(shader->m_ProgramName, materialProperty->m_Key);
        switch (materialProperty->m_Type)
        {
            case Material::kUnused:
            {
                break;
            }
            case Material::kFloat:
            {
                glUniform1f(slot, materialProperty->m_Float);
                break;
            }
            case Material::kVec4:
            {
                glUniform4f(slot, materialProperty->m_Vector.m_X[0],
                                  materialProperty->m_Vector.m_X[1],
                                  materialProperty->m_Vector.m_X[2],
                                  materialProperty->m_Vector.m_X[3]);
                break;
            }
            case Material::kTexture:
            {
                glActiveTexture(GL_TEXTURE0 + textureSlotItr);
                glBindTexture(GL_TEXTURE_2D, materialProperty->m_TextureId);
                glProgramUniform1i(shader->m_ProgramName, slot, textureSlotItr++);
                break;
            }
        }
    }
}

void MaterialReserveProperties(Material* material, int maxProperties)
{
    material->m_MaterialPropertyBlock = new Material::MaterialProperty[maxProperties];
    material->m_NumMaterialProperties = maxProperties;
    for (int i=0; i<maxProperties; ++i)
        material->m_MaterialPropertyBlock[i].m_Type = Material::MaterialPropertyType::kUnused;
}

void MaterialSetMaterialPropertyType(Material* material, int index, const char* materialPropertyName, Material::MaterialPropertyType type)
{
    assert(index < material->m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[index];
    materialProperty->m_Type = type;
    
    strncpy(materialProperty->m_Key, materialPropertyName, sizeof materialProperty->m_Key-1);
    materialProperty->m_Key[sizeof materialProperty->m_Key-1] = '\0';
}

int MaterialGetPropertyIndex(Material* material, const char* materialPropertyName)
{
    int index = -1;
    for (int i=0; index==-1&&i<material->m_NumMaterialProperties; ++i)
    {
        if (strcmp(material->m_MaterialPropertyBlock[i].m_Key, materialPropertyName))
            continue;
        index = i;
    }
    return index;
}

void MaterialSetMaterialPropertyFloat(Material* material, int index, float value)
{
    assert(index < material->m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kFloat);
    materialProperty->m_Float = value;
}

void MaterialSetMaterialPropertyVector(Material* material, int index, Vec4 value)
{
    assert(index < material->m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kVec4);
    materialProperty->m_Vector = value;
    
    // printf("setting %s to (%f %f %f %f)\n", materialProperty->m_Key, materialProperty->m_Vector.m_X[0], materialProperty->m_Vector.m_X[1], materialProperty->m_Vector.m_X[2], materialProperty->m_Vector.m_X[3]);
}

void MaterialSetMaterialPropertyTexture(Material* material, int index, int textureId)
{
    assert(index < material->m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &material->m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kTexture);
    materialProperty->m_TextureId = textureId;
}

void MaterialSetMaterialPropertyTexture(Material* material, int index, Texture* texture)
{
    int textureId = 0;
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    MaterialSetMaterialPropertyTexture(material, index, textureId);
}

void Material::ReserveProperties(int numProperties)
{
    MaterialReserveProperties(this, numProperties);
}

void Material::SetFloat(int index, float value)
{
    MaterialSetMaterialPropertyFloat(this, index, value);
}

void Material::SetVector(int index, Vec4 value)
{
    MaterialSetMaterialPropertyVector(this, index, value);
}

void Material::SetTexture(int index, int textureId)
{
    MaterialSetMaterialPropertyTexture(this, index, textureId);
}

void Material::SetTexture(int index, Texture* texture)
{
    if (texture)
        MaterialSetMaterialPropertyTexture(this, index, texture->m_TextureId);
}

