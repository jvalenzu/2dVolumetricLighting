#pragma once

#include "Engine/Matrix.h"
#include <stdint.h>

enum LightType : uint32_t
{
    kDirectional,
    kPoint,
    kConical,
    kCylindrical
};

struct Light
{
    enum { kMaxLights = 32 };
    
    LightType m_Type;
};

struct PointLight : Light
{
    float m_Range;
    int m_Pad[2];
    Vec4 m_Color;
    Vec4 m_Position;
};

struct DirectonalLight : Light
{
    int m_Pad[3];
    Vec4 m_Color;
};

struct ConicalLight : Light
{
    float m_Range;
    float m_Angle;
    int m_Pad[1];
    Vec4 m_Color;
    Vec4 m_Position;
    Vec4 m_Direction;
};

struct CylindricalLight : Light
{
    float m_Range;
    float m_Angle;
    float m_Length;
    Vec4 m_Color;
    Vec4 m_Position;
    Vec4 m_Direction;
};

union LightUnion
{
    PointLight m_PointLight;
    DirectonalLight m_DirectonalLight;
    ConicalLight m_ConicalLight;
    CylindricalLight m_CylindricalLight;
};

struct LightOptions
{
    LightType m_Type;
    Vec4 m_Position;
    Vec4 m_Direction;
    Vec4 m_Color;
    float m_Range;
    float m_Angle;

    static LightOptions MakeDirectionalLight(Vec4 direction, Vec4 color, float range)
    {
        LightOptions ret;
        ret.m_Type = kDirectional;
        ret.m_Direction = direction;
        ret.m_Color = color;
        return ret;
    }
    
    static LightOptions MakePointLight(Vec4 position, Vec4 color, float range)
    {
        LightOptions ret;
        ret.m_Type = kPoint;
        ret.m_Position = position;
        ret.m_Color = color;
        ret.m_Range = range;
        return ret;
    }

    static LightOptions MakePointLight(Vec3 position, Vec4 color, float range)
    {
        return MakePointLight(Vec4(position, 1.0f), color, range);
    }
    
    static LightOptions MakeConicalLight(Vec4 position, Vec4 direction, Vec4 color, float angle, float range)
    {
        LightOptions ret;
        ret.m_Type = kConical;
        ret.m_Position = position;
        ret.m_Direction = direction;
        ret.m_Color = color;
        ret.m_Range = range;
        ret.m_Angle = angle;
        return ret;
    }
    
    static LightOptions MakeCylindricalLight(Vec4 position, Vec4 direction, Vec4 color, float range)
    {
        LightOptions ret;
        ret.m_Type = kCylindrical;
        ret.m_Position = position;
        ret.m_Direction = direction;
        ret.m_Color = color;
        ret.m_Range = range;
        return ret;
    }
};

void DumpLight(const Light& light);
void LightInitialize(Light* light, const LightOptions& lightOptions);
