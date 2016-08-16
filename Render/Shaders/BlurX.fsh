#extension GL_ARB_explicit_attrib_location : enable

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D _MainTex;

in vec2 texCoord;
layout(location=0) out vec4 fragColor;

void main(void)
{
#ifdef BLUR_12
    const float coeffs[12] = float[12]( 0.16818810f, 0.15417242f, 0.11859417f, 0.07623911f,
                                        0.04066086f, 0.01778913f, 0.00627851f, 0.00174403f,
                                        0.00036716f, 0.00005507f, 0.00000525f, 0.00000024f);
    const int numCoeffs = 12;
#else
    const float coeffs[5] = float[5](0.27343750f, 0.21875000f, 0.10937500f, 0.03125000f, 0.00390625f);
    const int numCoeffs = 5;
#endif
    
    vec3 size = vec3(1,0,-1);
    ivec2 is = textureSize(_MainTex, 0);
    vec2 offset = vec2(1.0f, 1.0f) / is;
    
    vec4 t0 = texture(_MainTex, texCoord);
    vec4 c = t0*coeffs[0];
    for (int i=1; i<numCoeffs; ++i)
    {
        c += texture(_MainTex, texCoord+size.xy*offset*i) * coeffs[i];
        c += texture(_MainTex, texCoord+size.zy*offset*i) * coeffs[i];
    }
    
    fragColor.rgb = c.rgb;
    fragColor.a  = 1;
}
