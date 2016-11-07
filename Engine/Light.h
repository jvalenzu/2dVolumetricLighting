// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-
#pragma once

#include "Engine/Matrix.h"
#include <stdint.h>

struct Obb;

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
    uint32_t m_Index;
    
    float m_Range;
    float m_CosAngle;
    Vec4 m_Color;
    Vec4 m_Position;
    Vec4 m_Direction;
    float m_OrthogonalRange;
    int m_Pad[3];
};

struct LightOptions
{
    LightType m_Type;
    Vec4 m_Position;
    Vec4 m_Direction;
    Vec4 m_Color;
    float m_Range;
    float m_OrthogonalRange;
    float m_Angle;
    
    static LightOptions MakeDirectionalLight(Vec4 direction, Vec4 color)
    {
        LightOptions ret;
        ret.m_Type = kDirectional;
        ret.m_Direction = direction;
        ret.m_Position = Vec3(0.0f, 0.0f, 0.0f);
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
    
    static LightOptions MakeCylindricalLight(Vec4 position, Vec4 direction, Vec4 color, float range, float orthogonalRange)
    {
        LightOptions ret;
        ret.m_Type = kCylindrical;
        ret.m_Position = position;
        ret.m_Direction = direction;
        ret.m_Color = color;
        ret.m_Range = range;
        ret.m_OrthogonalRange = orthogonalRange;
        return ret;
    }
};

void DumpLight(const Light& light);
void LightInitialize(Light* light, const LightOptions& lightOptions);
void LightGenerateObb(Obb* dest, const LightOptions& lightOptions);
