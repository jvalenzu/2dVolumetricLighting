// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_gpu_shader5 : enable

#include "shader.h"
#include "light.h"

#ifdef GL_ES
precision highp float;
#endif

uniform mat4 project;
uniform mat4 view;

uniform usampler2D _MainTex;
in vec2 texCoord;
in vec4 screenPosition;

layout(location=0) out vec4 fragColor;

void main (void)
{
    uint c = texture(_MainTex, texCoord).x;
    float grad = bitCount(c)*0.25f;
    fragColor.rgb = vec3(grad, grad, grad);
    fragColor.a = 1;
    
    vec2 fragmentPos = screenPosition.xy / screenPosition.w;
    
    vec2 temp1 = fragmentPos;
    
    float d = 0;
    
    uint cylindricalMask = c & cylindricalLightMask;
    while (cylindricalMask != 0U)
    {
        int index = findLSB(cylindricalMask);
        uint bit = 1U<<index;
        
        cylindricalMask &= ~bit;
        
        Light cylindricalLight = _CylindricalLight[index];
        
        float t = pointOnLineSegmentT(cylindricalLight.m_Position.xy, cylindricalLight.m_Direction.xy, fragmentPos.xy);
        float t0 = clamp(t, 0.0f, 1.0f);
        if (t >= 0.0f && t <= 1.0f)
        {
            vec2 q0 = pointOnLineSegment(cylindricalLight.m_Position.xy, cylindricalLight.m_Direction.xy, fragmentPos.xy);
            vec2 orthogonalProjection = fragmentPos.xy - q0;
            float orthogonalAttenuation = 1.0f-clamp(length(orthogonalProjection)*cylindricalLight.m_OrthogonalRange, 0.0f, 1.0f);
            
            fragColor.rgb = vec3(orthogonalAttenuation, orthogonalAttenuation, orthogonalAttenuation);
        }
        
        // distinguish port/starboard
        if (false)
        {
            vec2 nHat = normalize(cylindricalLight.m_Direction.xy - cylindricalLight.m_Position.xy);
            vec2 qHat = normalize(cylindricalLight.m_Direction.xy - fragmentPos.xy);
            
            vec3 zAxis = vec3(0,0,1);
            vec3 nOrtho = cross(vec3(nHat,0.0f), zAxis);
            vec3 qOrtho = cross(vec3(qHat,0.0f), zAxis);
        
            if (dot(nOrtho.xy, qHat) > 0.0f)
                fragColor.rgb = vec3(0.25,0.25,0.25);
            else
                fragColor.rgb = vec3(0.5,0.5,0.5);
        }
        
        if (aspectCorrectedDistance(fragmentPos.xy, cylindricalLight.m_Position.xy) < 0.025)
            fragColor.rgb = vec3(1,0,0);
        if (aspectCorrectedDistance(fragmentPos.xy, cylindricalLight.m_Direction.xy) < 0.025)
            fragColor.rgb = vec3(0,1,0);
        
        vec2 p0 = pointOnLineSegment(cylindricalLight.m_Position.xy, cylindricalLight.m_Direction.xy, fragmentPos.xy);
        if (distance(fragmentPos.xy, p0) < 0.01)
            fragColor.rgb = vec3(0,0,1);
    }
}
