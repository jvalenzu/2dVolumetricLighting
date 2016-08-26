// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;
uniform vec4 _LightPosition;
uniform vec4 _LightFacingAngle;
in vec2 texCoord;
in vec4 screenPosition;
out vec4 fragColor;

#include "shader.h"

#define kAlphaThreshold 0.9

void main(void)
{
    float theta = kPi + texCoord.x * kTwoPi;
    float s = sin(theta);
    float c = cos(theta);
    
    vec2 borderPoint = clampCircle(kRootTwo*vec2(c, s));
    
    vec2 ray = (_LightPosition.xy - borderPoint)/1024.0f;
    vec2 itr = borderPoint;
    
    int count = 0;
    float d = kRootTwo;
    bool found = false;
    
    while (count < 1024)
    {
        if (itr.x>0 && itr.y>0 && itr.x<1 && itr.y<1)
        {
            vec4 r = texture(_MainTex, itr);
            if (r.r>kAlphaThreshold)
                d = distance(_LightPosition.xy, itr);
        }
        
        itr += ray;
        count++;
    }
    
    // conical attenuation
    vec2 fragmentPos = borderPoint;
    vec2 facingRay = normalize(_LightPosition.xy - fragmentPos);
    if (dot(facingRay, -_LightFacingAngle.xy) < _LightFacingAngle.w)
        d = kRootTwo;
    
    fragColor = vec4(d,d,d,1.0);
}
