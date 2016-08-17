// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Engine/Light.h"
#include "Engine/Utils.h"

#include <assert.h>
#define _USE_MATH_DEFINES 1
#include <math.h>
#include <stdio.h>

void DumpLight(const Light& light)
{
    switch (light.m_Type)
    {
        case LightType::kPoint:
        {
            const PointLight& pointLight = (const PointLight&) light;
            puts("PointLight:\n");
            Printf("m_Range: %f\n", pointLight.m_Range);
            Printf("m_Color: %f %f %f %f\n", pointLight.m_Color.m_X[0], pointLight.m_Color.m_X[1], pointLight.m_Color.m_X[2], pointLight.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", pointLight.m_Position.m_X[0], pointLight.m_Position.m_X[1], pointLight.m_Position.m_X[2]);
            
            break;
        }
        case LightType::kConical:
        {
            const ConicalLight& conicalLight = (const ConicalLight&) light;
            puts("CylindricalLight:\n");
            Printf("m_Range: %f\n", conicalLight.m_Range);
            Printf("m_Color: %f %f %f %f\n", conicalLight.m_Color.m_X[0], conicalLight.m_Color.m_X[1], conicalLight.m_Color.m_X[2], conicalLight.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", conicalLight.m_Position.m_X[0], conicalLight.m_Position.m_X[1], conicalLight.m_Position.m_X[2]);
            break;
        }        
        case LightType::kCylindrical:
        {
            const CylindricalLight& cylindricalLight = (const CylindricalLight&) light;
            puts("CylindricalLight:\n");
            Printf("m_Range: %f\n", cylindricalLight.m_Range);
            Printf("m_Color: %f %f %f %f\n", cylindricalLight.m_Color.m_X[0], cylindricalLight.m_Color.m_X[1], cylindricalLight.m_Color.m_X[2], cylindricalLight.m_Color.m_X[3]);
            Printf("m_Position: %f %f %f\n", cylindricalLight.m_Position.m_X[0], cylindricalLight.m_Position.m_X[1], cylindricalLight.m_Position.m_X[2]);
            break;
        }        
    }
}

void LightInitialize(Light* light, const LightOptions& lightOptions)
{
    switch (lightOptions.m_Type)
    {
        case LightType::kPoint:
        {
            PointLight* pointLight = (PointLight*) light;
            pointLight->m_Type = LightType::kPoint;
            pointLight->m_Range = lightOptions.m_Range;
            pointLight->m_Color = lightOptions.m_Color;
            pointLight->m_Position = lightOptions.m_Position;
            break;
        }
        case LightType::kConical:
        {
            ConicalLight* conicalLight = (ConicalLight*) light;
            conicalLight->m_Type = LightType::kConical;
            conicalLight->m_CosAngle = cosf(float(M_PI) * lightOptions.m_Angle / 360.0f);
            conicalLight->m_Range = lightOptions.m_Range;
            conicalLight->m_Color = lightOptions.m_Color;
            conicalLight->m_Position = lightOptions.m_Position;
            conicalLight->m_Direction = lightOptions.m_Direction;
            break;
        }
        case LightType::kCylindrical:
        {
            CylindricalLight* cylindricalLight = (CylindricalLight*) light;
            cylindricalLight->m_Type = LightType::kCylindrical;
            cylindricalLight->m_Range = lightOptions.m_Range;
            cylindricalLight->m_Length = lightOptions.m_Length;
            cylindricalLight->m_Color = lightOptions.m_Color;
            cylindricalLight->m_Position = lightOptions.m_Position;
            cylindricalLight->m_Direction = lightOptions.m_Direction;
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
}
