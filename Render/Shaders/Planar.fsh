#ifdef GL_ES
precision highp float;
#endif

#include "shader.h"

uniform sampler2D _MainTex;
uniform sampler2D _PlanarTex;
uniform vec4 _LightPosition;

uniform PointLight _PointLights;

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
    
    vec2 ray = normalize(_LightPosition.xy - toZeroOne(screenPosition.xy / screenPosition.w));
    float c0 = MIN_ATTENUATION+2*ATTENUATION_RANGE*clamp(dot(t1.rg, ray), 0.0, 1);
    fragColor = vec4(c0*t0.rgb, t0.a);
}
