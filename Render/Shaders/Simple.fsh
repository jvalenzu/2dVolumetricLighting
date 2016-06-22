#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;

in vec2 texCoord;
in vec3 normal;
in vec4 colorV;
layout(location=0) out vec4 fragColor;

void main (void)
{
    vec4 c = texture(_MainTex, texCoord);
    fragColor.rgb = c.rgb;
    fragColor.a  = 1;
}
