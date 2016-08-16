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
        kBlend
    };
    
    enum MaterialPropertyType : uint32_t
    {
        kUnused,
        kFloat,
        kVec4,
        kTexture
    };
    
    struct MaterialProperty
    {
        enum { kNameMax = 31 };
        MaterialPropertyType m_Type;
        char m_Key[kNameMax+1];
        union
        {
            float m_Float;
            Vec4 m_Vector;
            int m_TextureId;
        };
        
        MaterialProperty()
        {
            m_Type = kUnused;
        }
    };

    void ReserveProperties(int numProperties);
    void SetFloat(int index, float value);
    void SetVector(int index, Vec4 value);
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

void      MaterialReserveProperties(Material* material, int numProperties);
void      MaterialSetMaterialPropertyType(Material* material, int index, const char* materialPropertyName, Material::MaterialPropertyType type);
void      MaterialSetMaterialPropertyFloat(Material* material,   int index, float value);
void      MaterialSetMaterialPropertyVector(Material* material,  int index, Vec4 value);
void      MaterialSetMaterialPropertyTexture(Material* material, int index, int textureId);
void      MaterialSetMaterialPropertyTexture(Material* material, int index, Texture* texture);
int       MaterialGetPropertyIndex(Material* material, const char* materialPropertyName); // -1 on error

void      MaterialDestroy(Material* victim);
