#extension GL_ARB_explicit_attrib_location : enable

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;
uniform vec4 TintColor;

in vec2 texCoord;
in vec3 normal;
in vec4 colorV;
layout(location=0) out vec4 fragColor;

void main (void)
{
    vec4 c = texture(_MainTex, texCoord);
    c.rgb *= TintColor.rgb;
    fragColor = c;
}
