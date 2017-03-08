// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable

#include "shader.h"

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;

in vec4 colorV;
layout(location=0) out vec4 fragColor;

void main (void)
{
    fragColor.rgb = colorV.rgb;
    fragColor.a  = 1;
}
