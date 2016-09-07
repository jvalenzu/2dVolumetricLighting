// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "Render/GL.h"
#include "Render/Texture.h"
#include "Engine/Matrix.h"
#include "Render/Shader.h"

struct RenderContext;

struct Material
{
    enum BlendMode : uint32_t
    {
        kOpaque,
        kCutout,
        kBlend,
        kOr
    };
    
    enum MaterialPropertyType : uint32_t
    {
        kUnused,
        kFloat,
        kUInt,
        kVec4,
        kTexture,
        kMat4,
    };
    
    struct MaterialProperty
    {
        enum { kNameMax = 31 };
        MaterialPropertyType m_Type;
        char m_Key[kNameMax+1];
        union
        {
            float m_Float;
            int m_Int;
            Vec4 m_Vector;
            int m_TextureId;
            Mat4 m_Matrix;
        };
        
        MaterialProperty()
        {
            m_Type = kUnused;
        }
    };
    
    void ReserveProperties(int numProperties);
    int SetPropertyType(const char* materialPropertyName, Material::MaterialPropertyType type);
    int GetPropertyIndex(const char* materialPropertyName);
    void SetFloat(int index, float value);
    void SetInt(int index, int value);
    void SetVector(int index, Vec4 value);
    void SetMatrix(int index, const Mat4& value);
    void SetTexture(int index, int textureId);
    void SetTexture(int index, Texture* texture);
    
    MaterialProperty* m_MaterialPropertyBlock;
    int m_NumMaterialProperties;
    BlendMode m_BlendMode;
    Shader* m_Shader;
    Texture* m_Texture;
};

// create/destroy material
Material* MaterialCreate(Shader* shader, Texture* texture);
Material* MaterialRef(Material* material);
void      MaterialDestroy(Material* victim);


