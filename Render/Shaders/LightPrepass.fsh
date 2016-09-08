// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#extension GL_ARB_explicit_attrib_location : enable

#include "shader.h"

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;
uniform uint      _LightBitmask;

layout(location=0) out uvec4 fragColor;

void main (void)
{
    fragColor.r = _LightBitmask;
}
