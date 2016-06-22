#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;
in  vec2 texCoord;
out vec4 fragColor;

void main(void)
{
    vec4 c = texture(_MainTex, texCoord);
    fragColor = vec4(c.a, c.a, c.a, 1.0);
}
