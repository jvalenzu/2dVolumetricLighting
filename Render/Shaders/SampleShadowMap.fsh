// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;
uniform vec4      _LightPosition;
uniform vec4      _LightColor;
in      vec2      texCoord;
out     vec4      fragColor;

#include "shader.h"

#define kShadowBlendFactor 0.5f

void main(void)
{
    // intersect the extruded ray from lsp (_LightPosition.xy) to texCoord to unit box
    vec2 ray = _LightPosition.xy - texCoord;
    vec2 projectedUv = border(_LightPosition.xy, texCoord);
    vec2 projectedRay = fromZeroOne(projectedUv);
    float theta = (atan(projectedRay.y, projectedRay.x) + kPi) * kInvTwoPi;
    vec4 d = texture(_MainTex, vec2(theta, 0));
    float lr = length(ray);
    if (d.r <= lr)
        fragColor = vec4(_LightColor.rgb,kShadowBlendFactor);
    else
        fragColor = vec4(0,0,0,0.0f);
}
