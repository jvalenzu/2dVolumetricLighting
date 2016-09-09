// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader5 : enable

#include "shader.h"

#ifdef GL_ES
precision highp float;
#endif

uniform usampler2D _MainTex;
in vec2 texCoord;

layout(location=0) out vec4 fragColor;

void main (void)
{
    uint c = texture(_MainTex, texCoord).x;
    float grad = bitCount(c)*0.25f;
    fragColor.rgb = vec3(grad,grad,grad);
    fragColor.a = 1;
}
