// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_EXT_gpu_shader4 : enable

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
uniform vec4 _AmbientLight;

in vec2 texCoord;
in vec4 screenPosition;
out vec4 fragColor;

void main (void)
{
    vec4 t0 = texture(_MainTex, texCoord);
    vec4 t1 = texture(_PlanarTex, texCoord);
    t1 = fromZeroOne(t1);
    
    vec2 fragmentPos = screenPosition.xy / screenPosition.w;
    vec4 color = vec4(_AmbientLight.rgb, 1);
    
    uint mask = texture(_LightPrepassTex, toZeroOne(fragmentPos.xy)).x;
    
    for (int i=0; i<numPointLights; ++i)
    {
        PointLight pointLight = _PointLight[i];
        uint bit = (1U<<pointLight.m_Index);
        if ((bit & mask) == bit)
        {
            vec2 ray = normalize(pointLight.m_Position.xy - fragmentPos);
            float c0 = clamp(dot(t1.rg, ray), 0.0f, 1.0f);
            
            // screenspace distance based attenuation squared
            float d0 = distance(pointLight.m_Position.xy, fragmentPos.xy);
            float d1 = 1.0f - clamp(d0*pointLight.m_Range, 0.0f, 1.0f);
            float d2 = d1*d1;
            
            color.rgb += d2*c0*pointLight.m_Color.rgb;
        }
    }
    
    for (int i=0; i<numConicalLights; ++i)
    {
        ConicalLight conicalLight = _ConicalLight[i];
        uint bit = (1U<<conicalLight.m_Index);
        if ((bit & mask) == bit)
        {
            vec2 ray = normalize(conicalLight.m_Position.xy - fragmentPos);
            float c0 = clamp(dot(t1.rg, ray), 0.0, 1);
            
            // screenspace distance based attenuation squared
            float d0 = distance(conicalLight.m_Position.xy, fragmentPos.xy);
            float d1 = 1.0f - clamp(d0*conicalLight.m_Range, 0.0f, 1.0f);
            
            // angle attenuation.  This is a sharp falloff, but typically it would also attenuation based on distance from the center ray.
            float theta = dot(ray, -conicalLight.m_Direction.xy);
            if (theta > conicalLight.m_Angle)
                color.rgb += d1*c0*conicalLight.m_Color.rgb;
        }
    }
    
    for (int i=0; i<numCylindricalLights; ++i)
    {
        CylindricalLight cylindricalLight = _CylindricalLight[i];
        uint bit = (1U<<cylindricalLight.m_Index);
        if ((bit & mask) == bit)
        {
            // screenspace distance based attenuation squared
            float temp0 = pointOnLineSegmentT(cylindricalLight.m_Position.xy, cylindricalLight.m_Position1.xy, fragmentPos.xy);
            vec2 p0 = pointOnLineSegment(temp0, cylindricalLight.m_Position.xy, cylindricalLight.m_Position1.xy);
            
            vec2 ray = normalize(p0 - fragmentPos);
            float c0 = clamp(dot(t1.rg, ray), 0.0, 1);
            
            float d0 = distance(p0, fragmentPos.xy);
            float d1 = clamp((d0*d0) * cylindricalLight.m_Range, 0.0f, 1.0f);
            
            color.rgb += (1-d1)*c0*cylindricalLight.m_Color.rgb;
        }
    }
    
    fragColor = vec4(color.rgb*t0.rgb, t0.a);
}
