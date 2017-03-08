// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Render/GL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "Engine/Matrix.h"
#include "Render/Model.h"
#include "Render/Render.h"
#include "Engine/Utils.h"
#include "Render/Material.h"

// -------------------------------------------------------------------------------------------------
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

// -------------------------------------------------------------------------------------------------
// MaterialCreate
//
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

// -------------------------------------------------------------------------------------------------
// MaterialDestroy
//
void MaterialDestroy(Material* victim)
{
    if (victim == nullptr)
        return;
    
    ShaderDestroy(victim->m_Shader);
    TextureDestroy(victim->m_Texture);
    
    victim->m_Shader = nullptr;
    victim->m_Texture = nullptr;
    
    if (victim->m_MaterialPropertyBlock)
    {
        delete [] victim->m_MaterialPropertyBlock;
        victim->m_MaterialPropertyBlock = nullptr;
    }
    
    delete victim;
}

// -------------------------------------------------------------------------------------------------
void RenderSetMaterialProperty(int* textureSlots, GLuint programName, const Material::MaterialProperty* materialProperty)
{
    GLint slot = glGetUniformLocation(programName, materialProperty->m_Key);
    if (slot < 0)
        return;
    
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
        case Material::kUInt:
        {
            glUniform1ui(slot, materialProperty->m_Int);
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
            glActiveTexture(GL_TEXTURE0 + *textureSlots);
            glBindTexture(GL_TEXTURE_2D, materialProperty->m_TextureId);
            glProgramUniform1i(programName, slot, (*textureSlots)++);
            break;
        }
        case Material::kMat4:
        {
            glUniformMatrix4fv(slot, 1, GL_FALSE, materialProperty->m_Matrix.asFloat());
            break;
        }
    }
}

int Material::GetPropertyIndex(const char* materialPropertyName)
{
    for (int i=0; i<m_NumMaterialProperties; ++i)
    {
        if (!strcmp(m_MaterialPropertyBlock[i].m_Key, materialPropertyName))
            return i;
    }
    
    return -1;
}

void Material::ReserveProperties(int numProperties)
{
    m_MaterialPropertyBlock = new Material::MaterialProperty[numProperties];
    m_NumMaterialProperties = numProperties;
    for (int i=0; i<numProperties; ++i)
        m_MaterialPropertyBlock[i].m_Type = Material::MaterialPropertyType::kUnused;
}

void Material::SetFloat(int index, float value)
{
    assert(index < m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kFloat);
    materialProperty->m_Float = value;
}

void Material::SetInt(int index, int value)
{
    assert(index < m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kUInt);
    materialProperty->m_Int = value;
}

void Material::SetVector(int index, Vec4 value)
{
    assert(index < m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kVec4);
    materialProperty->m_Vector = value;
}

void Material::SetMatrix(int index, const Mat4& value)
{
    assert(index < m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kMat4);
    materialProperty->m_Matrix = value;
}

void Material::SetTexture(int index, int textureId)
{
    assert(index < m_NumMaterialProperties);
    Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
    assert(materialProperty->m_Type == Material::MaterialPropertyType::kTexture);
    materialProperty->m_TextureId = textureId;
}

void Material::SetTexture(int index, Texture* texture)
{
    int textureId = 0;
    if (texture != nullptr)
        textureId = texture->m_TextureId;
    SetTexture(index, textureId);
}

int Material::SetPropertyType(const char* materialPropertyName, Material::MaterialPropertyType type)
{
    int index = -1;
    for (int i=0; index==-1 && i<m_NumMaterialProperties; ++i)
    {
        if (m_MaterialPropertyBlock[i].m_Type == Material::MaterialPropertyType::kUnused)
            index = i;
    }
    
    if (index>=0)
    {
        Material::MaterialProperty* materialProperty = &m_MaterialPropertyBlock[index];
        materialProperty->m_Type = type;
        
        strncpy(materialProperty->m_Key, materialPropertyName, sizeof materialProperty->m_Key-1);
        materialProperty->m_Key[sizeof materialProperty->m_Key-1] = '\0';
    }
    
    return index;
}
