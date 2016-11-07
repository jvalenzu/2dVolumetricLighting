// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "slib/Common/Util.h"
#include "Engine/Light.h"
#include "Engine/Utils.h"
#include "Tool/Utils.h"

#include <assert.h>
#define _USE_MATH_DEFINES 1
#include <math.h>
#include <stdio.h>

void DumpLight(const Light& light)
{
    switch (light.m_Type)
    {
        case LightType::kDirectional:
        {
            puts("DirectionalLight:\n");
            Printf("m_Color: %f %f %f %f\n", light.m_Color.m_X[0],    light.m_Color.m_X[1],    light.m_Color.m_X[2], light.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", light.m_Position.m_X[0], light.m_Position.m_X[1], light.m_Position.m_X[2]);
            break;
        }
        case LightType::kPoint:
        {
            puts("PointLight:\n");
            Printf("m_Range: %f\n", light.m_Range);
            Printf("m_Color: %f %f %f %f\n", light.m_Color.m_X[0],    light.m_Color.m_X[1],    light.m_Color.m_X[2], light.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", light.m_Position.m_X[0], light.m_Position.m_X[1], light.m_Position.m_X[2]);
            
            break;
        }
        case LightType::kConical:
        {
            puts("CylindricalLight:\n");
            Printf("m_Range: %f\n", light.m_Range);
            Printf("m_Color: %f %f %f %f\n", light.m_Color.m_X[0], light.m_Color.m_X[1], light.m_Color.m_X[2], light.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", light.m_Position.m_X[0], light.m_Position.m_X[1], light.m_Position.m_X[2]);
            break;
        }
        case LightType::kCylindrical:
        {
            puts("CylindricalLight:\n");
            Printf("m_Range: %f\n", light.m_Range);
            Printf("m_Color: %f %f %f %f\n", light.m_Color.m_X[0], light.m_Color.m_X[1], light.m_Color.m_X[2], light.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", light.m_Position.m_X[0], light.m_Position.m_X[1], light.m_Position.m_X[2]);
            break;
        }
    }
}

// LightInitialize
void LightInitialize(Light* light, const LightOptions& lightOptions)
{
    switch (lightOptions.m_Type)
    {
        case LightType::kDirectional:
        {
            light->m_Type = LightType::kDirectional;
            light->m_Color = lightOptions.m_Color;
            light->m_Direction = lightOptions.m_Direction;
            light->m_Position = Vec3(0.0f, 0.0f, 0.0f);
            break;
        }
        case LightType::kPoint:
        {
            light->m_Type = LightType::kPoint;
            light->m_Range = lightOptions.m_Range;
            light->m_Color = lightOptions.m_Color;
            light->m_Position = lightOptions.m_Position;
            break;
        }
        case LightType::kConical:
        {
            light->m_Type = LightType::kConical;
            light->m_CosAngle = cosf(float(M_PI) * lightOptions.m_Angle / 360.0f);
            assert(light->m_CosAngle>=0.0f);
            light->m_Range = lightOptions.m_Range;
            light->m_Color = lightOptions.m_Color;
            light->m_Position = lightOptions.m_Position;
            light->m_Direction = lightOptions.m_Direction;
            break;
        }
        case LightType::kCylindrical:
        {
            light->m_Type = LightType::kCylindrical;
            light->m_Range = lightOptions.m_Range;
            light->m_OrthogonalRange = lightOptions.m_OrthogonalRange;
            light->m_Color = lightOptions.m_Color;
            light->m_Position = lightOptions.m_Position;
            light->m_Direction = lightOptions.m_Direction;
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
}

void LightGenerateObb(Obb* dest, const LightOptions& lightOptions)
{
    const float q2 = 1.41421356237f;
    Vec3 verticesPoint[] =
    {
        { -q2, -q2,  q2 },
        {  q2, -q2,  q2 },
        {  q2,  q2,  q2 },
        { -q2,  q2,  q2 },
        { -q2,  q2, -q2 },
        { -q2, -q2, -q2 },
        {  q2, -q2, -q2 },
        {  q2,  q2, -q2 }
    };
    Vec3 verticesCone[] =
    {
        { -q2, -q2, q2 },
        {  q2, -q2, q2 },
        {  q2,  q2, q2 },
        { -q2,  q2, q2 },
        { -q2,  q2, -q2 },
        { -q2, -q2, -q2 },
        {  q2, -q2, -q2 },
        {  q2,  q2, -q2 }
    };
    Vec3* vertices;
    int n;
    
    if (lightOptions.m_Type == LightType::kPoint)
    {
        n = ELEMENTSOF(verticesPoint);
        vertices = verticesPoint;
    }
    else
    {
        n = ELEMENTSOF(verticesCone);
        vertices = verticesCone;
    }
    
    for (int i=0; i<n; ++i)
        vertices[i] *= lightOptions.m_Range;
    
    *dest = ToolGenerateObbFromVec3(vertices, n);
}
