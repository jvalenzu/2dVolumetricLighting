// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable

#ifdef GL_ES
precision highp float;
#endif

#include "shader.h"
#include "light.h"

uniform sampler2D _MainTex;

// material properties
uniform vec4 _MaterialDiffuse;
uniform vec4 _MaterialAmbient;
uniform vec4 _MaterialSpecular;
uniform float _MaterialSpecularPower;
uniform vec4 _MaterialTransparency;

in vec4 colorV;
in vec4 normalV;
in vec4 cameraDirectionV;
layout(location=0) out vec4 fragColor;

void main (void)
{
    vec4 color = vec4(0,0,0,0);
    vec3 specular = vec3(0,0,0);

    vec4 v = normalize(cameraDirectionV);
    
    for (int i=0; i<int(numDirectionalLights); ++i)
    {
        Light directionalLight = _DirectionalLight[i];
        float c0 = clamp(dot(normalV.xyz, -directionalLight.m_Direction.xyz), 0.0f, 1.0f);
        color.rgb += c0*directionalLight.m_Color.rgb;
        
        // specular
        float ndotl = dot(normalV.xyz, -directionalLight.m_Direction.xyz);
        float intensity = clamp(ndotl, 0.0f, 1.0f);
        vec3 h = normalize(v.xyz - directionalLight.m_Direction.xyz);
        float ndoth = dot(normalV.xyz, h);
        intensity = pow(clamp(ndoth, 0.0f, 1.0f), _MaterialSpecularPower);
        
        specular += intensity * _MaterialSpecular.rgb;
    }
    
    fragColor.rgb = _MaterialDiffuse.rgb * color.rgb + _MaterialSpecular.rgb * specular;
    fragColor.a = 1.0f;
}
