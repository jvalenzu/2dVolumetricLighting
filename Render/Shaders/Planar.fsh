// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_gpu_shader5 : enable

#ifdef GL_ES
precision highp float;
#endif

#include "shader.h"
#include "light.h"

uniform sampler2D _MainTex;
uniform sampler2D _PlanarTex;
uniform usampler2D _LightPrepassTex;

uniform mat4 project;
uniform mat4 modelView;

in vec2 texCoord;
in vec4 screenPosition;
out vec4 fragColor;

void main (void)
{
    vec4 t0 = texture(_MainTex, texCoord);
    vec4 t1 = texture(_PlanarTex, texCoord);
    t1 = fromZeroOne(t1);
    
    vec2 fragmentPos = screenPosition.xy / screenPosition.w;
    vec4 color = vec4(0,0,0,0);
    
    uint mask = texture(_LightPrepassTex, toZeroOne(fragmentPos.xy)).x;
    
    uint pointMask = mask & pointLightMask;
    uint conicalMask = mask & conicalLightMask;
    uint cylindricalMask = mask & cylindricalLightMask;
    
    for (int i=0; i<numDirectionalLights; ++i)
    {
        Light directionalLight = _DirectionalLight[i];
        float c0 = clamp(dot(t1.rg, directionalLight.m_Direction.xy), 0.0f, 1.0f);
        color.rgb += c0*directionalLight.m_Color.rgb;
    }
    
    while (pointMask != 0U)
    {
        int index = findLSB(pointMask);
        uint bit = 1U<<index;
        
        pointMask &= ~bit;
        
        Light pointLight = _PointLight[index];
        vec2 ray = normalize(pointLight.m_Position.xy - fragmentPos);
        float c0 = clamp(dot(t1.rg, ray), 0.0f, 1.0f);
        
        // screenspace distance based attenuation squared
        float d0 = distance(pointLight.m_Position.xy, fragmentPos.xy);
        float d1 = 1.0f - clamp(d0*pointLight.m_Range, 0.0f, 1.0f);
        float d2 = d1*d1;
        
        color.rgb += d2*c0*pointLight.m_Color.rgb;
    }
    
    while (conicalMask != 0U)
    {
        int index = findLSB(conicalMask);
        uint bit = 1U<<index;
        
        conicalMask &= ~bit;
        
        Light conicalLight = _ConicalLight[index];
        vec2 ray = normalize(conicalLight.m_Position.xy - fragmentPos);
        float c0 = clamp(dot(t1.rg, ray), 0.0f, 1.0f);
        
        // screenspace distance based attenuation squared
        float d0 = distance(conicalLight.m_Position.xy, fragmentPos.xy);
        float d1 = 1.0f - clamp(d0*conicalLight.m_Range, 0.0f, 1.0f);
        
        // angle attenuation.  This is a sharp falloff, but typically it would also attenuation based on distance from the center ray.
        float theta = dot(ray, -normalize(conicalLight.m_Direction.xy));
        if (theta > conicalLight.m_Angle)
        {
            float phi = (theta - conicalLight.m_Angle) / (1-conicalLight.m_Angle);
            color.rgb += c0*d1*phi*conicalLight.m_Color.rgb;
        }
    }
    
    while (cylindricalMask != 0U)
    {
        int index = findLSB(cylindricalMask);
        uint bit = 1U<<index;
        
        cylindricalMask &= ~bit;
        
        Light cylindricalLight = _CylindricalLight[index];
        
        vec2 lightAxis = cylindricalLight.m_Direction.xy - cylindricalLight.m_Position.xy;
        float t = pointOnLineSegmentT(cylindricalLight.m_Position.xy, cylindricalLight.m_Direction.xy, fragmentPos.xy);
        vec2 q0 = cylindricalLight.m_Position.xy+lightAxis*clamp(t, 0.0f, 1.0f);
        
        float orthogonalAttenuation = 0.0f;
        if (t >= 0.0f && t <= 1.0f)
        {
            vec2 orthogonalProjection = fragmentPos.xy - q0;
            orthogonalAttenuation = 1.0f-clamp(length(orthogonalProjection)*cylindricalLight.m_OrthogonalRange, 0.0f, 1.0f);
        }
        
        vec2 ray = normalize(fragmentPos.xy - q0.xy);
        float c0 = clamp(dot(t1.rg, ray), 0.0f, 1.0f);
        
        color.rgb += orthogonalAttenuation*cylindricalLight.m_Color.rgb;
    }
    
    fragColor = vec4(color.rgb*t0.rgb, t0.a);
    // fragColor = vec4(color.rgb, t0.a);
}
