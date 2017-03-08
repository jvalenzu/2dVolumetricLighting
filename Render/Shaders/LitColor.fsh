// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable

#ifdef GL_ES
precision highp float;
#endif

#include "shader.h"
#include "light.h"

uniform sampler2D _MainTex;

in vec4 colorV;
in vec4 normalV;
layout(location=0) out vec4 fragColor;

void main (void)
{
    vec4 color = vec4(0,0,0,0);
    
    for (int i=0; i<numDirectionalLights; ++i)
    {
        Light directionalLight = _DirectionalLight[i];
        float c0 = clamp(dot(normalV.xyz, -directionalLight.m_Direction.xyz), 0.0f, 1.0f);
        color.rgb += c0*directionalLight.m_Color.rgb;
    }
    
    fragColor.rgb = colorV.rgb * color.rgb;
    fragColor.a  = 1;
}
