#include "Engine/Light.h"

#include <assert.h>
#include <stdio.h>

void DumpLight(const Light& light)
{
    switch (light.m_Type)
    {
        case LightType::kPoint:
        {
            const PointLight& pointLight = (const PointLight&) light;
            puts("PointLight:\n");
            printf("m_Range: %f\n", pointLight.m_Range);
            printf("m_Color: %f %f %f %f\n", pointLight.m_Color.m_X[0], pointLight.m_Color.m_X[1], pointLight.m_Color.m_X[2], pointLight.m_Color.m_X[3]);
            printf("m_Position: %f %f %f\n", pointLight.m_Position.m_X[0], pointLight.m_Position.m_X[1], pointLight.m_Position.m_X[2]);
            
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
        default:
        {
            assert(false);
            break;
        }
    }
}
