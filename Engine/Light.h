#pragma once

#include "Engine/Scene.h"

struct LightOptions
{
    LightType m_Type;
    Vec4 m_Position;
    Vec4 m_Direction;
    Vec4 m_Color;
    float m_Range;
    float m_Angle;

    LightOptions MakeDirectionalLight(Vec4 direction, Vec4 color, float range)
    {
        LightOptions ret;
        ret.m_Type = kDirectional;
        ret.m_Direction = direction;
        ret.m_Color = color;
        return ret;
    }
    
    LightOptions MakePointLight(Vec4 position, Vec4 color, float range)
    {
        LightOptions ret;
        ret.m_Type = kPoint;
        ret.m_Position = position;
        ret.m_Color = color;
        ret.m_Range = range;
        return ret;
    }
    
    LightOptions MakeConicalLight(Vec4 position, Vec4 direction, Vec4 color, float angle, float range)
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
    
    LightOptions MakeCylindricalLight(Vec4 position, Vec4 direction, Vec4 color, float range)
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
