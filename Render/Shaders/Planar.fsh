#ifdef GL_ES
precision highp float;
#endif

#include "shader.h"

uniform sampler2D _MainTex;
uniform sampler2D _PlanarTex;

layout (std140) uniform LightData
{
    PointLight _PointLight[32];
};

uniform mat4 project;
uniform mat4 modelView;
uniform vec4 _AmbientLight;

in vec2 texCoord;
in vec4 screenPosition;
out vec4 fragColor;

#define MIN_ATTENUATION 0.25f
#define ATTENUATION_RANGE (1.0f - MIN_ATTENUATION)

void main (void)
{
    vec4 t0 = texture(_MainTex, texCoord);
    vec4 t1 = texture(_PlanarTex, texCoord);
    t1 = fromZeroOne(t1);
    
    vec2 fragmentPos = screenPosition.xy / screenPosition.w;
    
    vec4 color = vec4(_AmbientLight.rgb,1);
    for (int i=0; i<32; ++i)
    {
        PointLight pointLight = _PointLight[i];
        vec2 ray = normalize(pointLight.m_Position.xy - fragmentPos);
        float c0 = clamp(dot(t1.rg, ray), 0.0, 1);
        
        // screenspace distance based attenuation squared
        float d0 = distance(pointLight.m_Position.xy, fragmentPos.xy);
        float d1 = clamp((d0*d0) / pointLight.m_Range, 0.0f, 1.0f);
        
        color.rgb += (1-d1)*c0*pointLight.m_Color.rgb;
    }
    
    fragColor = vec4(color.rgb*t0.rgb, t0.a);
}
