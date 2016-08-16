#extension GL_ARB_uniform_buffer_object : enable

#define kPiOverTwo  1.5707963268
#define kPi         3.14159265359
#define kTwoPi      6.28318530718
#define kRootTwo    1.41421356237
#define kInvTwoPi   0.159154943092

vec2 fromZeroOne(vec2 param)
{
    return param*2.0f - 1.0f;
}

vec3 fromZeroOne(vec3 param)
{
    return param*2.0f - 1.0f;
}

vec4 fromZeroOne(vec4 param)
{
    return param*2.0f - 1.0f;
}

vec2 toZeroOne(vec2 param)
{
    return (param+1.0f)*0.5f;
}

vec3 toZeroOne(vec3 param)
{
    return (param+1.0f)*0.5f;
}

vec4 toZeroOne(vec4 param)
{
    return (param+1.0f)*0.5f;
}

vec2 pointOnLineSegment(vec2 p0, vec2 p1, vec2 q0)
{
    vec2 ret;

    vec2 p1p0 = p1 - p0;
    float t0 = dot(q0 + p0, p1p0)/dot(p1p0, p1p0);
    float t1 = min(t0, 1.0f);
    float t2 = max(t1, 0.0f);
    return p0+p1p0*t2;
}

// clip to box
// p0 -> point in range -1,1
// p1 -> point in space -1,1
// n  -> edge normal to test
vec2 clipC(vec2 p0, vec2 p1, vec2 n)
{
    vec2 pe = n;
    vec2 d = p1 - p0;
    float t = dot(n, p0 - pe) / -dot(n, d);
    return d*t+p0;
}

bool onBox(vec2 p)
{
    vec2 ap = abs(p);
    return max(ap.x, ap.y) <= 1;
}

// there has got to be a better way to do line clipping on a unit box
vec2 border(vec2 p0, vec2 p1)
{
    float kSqrt8 = 2.0*1.41421356237;
    
    // convert lsp to ray space
    vec2 lspN = fromZeroOne(p0);
    vec2 uvN = fromZeroOne(p1);
    
    // vector going for lsp to our uv.  Multiply by 2.0*sqrt(2) because the longest vector can be -1,-1 to 1,1 in ray space.
    vec2 ray = normalize(uvN - lspN);
	
    // project our uv outside of our raybox.  This is the position we have to clip to the raybox
    vec2 outsideUv = uvN + ray;
	
    vec2 n0 = vec2( 1.0,  0.0); // right
    vec2 n1 = vec2(-1.0,  0.0); // left
    vec2 n2 = vec2( 0.0, -1.0); // bottom
    vec2 n3 = vec2( 0.0,  1.0); // top
	
    // determine which sides we need to test.  should be no more than two.
    bool validNorm[4];
    validNorm[0] = validNorm[1] = validNorm[2] = validNorm[3] = true;
    
    if (lspN.x < -1)
        validNorm[1] = false; // left
    if (lspN.x > 1)
        validNorm[0] = false; // right
    if (lspN.y < -1)
        validNorm[2] = false; // bottom
    if (lspN.y > 1)
        validNorm[3] = false; // top
	
    if (outsideUv.y > lspN.y)
        validNorm[2] = false; // bottom
    else
        validNorm[3] = false; // top
	
    if (outsideUv.x > lspN.x)
        validNorm[1] = false; // left
    else
        validNorm[0] = false; // right
	
    vec2 clippy0;
    vec2 clippy1;
    vec2 clippy2;
    vec2 clippy3;
	
    if (validNorm[0])
        clippy0 = clipC(outsideUv, lspN, n0);
    if (validNorm[1])
        clippy1 = clipC(outsideUv, lspN, n1);
    if (validNorm[2])
        clippy2 = clipC(outsideUv, lspN, n2);
    if (validNorm[3])
        clippy3 = clipC(outsideUv, lspN, n3);
	
    vec2 projectedRay = vec2(0,0);
	
    if (validNorm[0] && onBox(clippy0))
        projectedRay = clippy0;
    if (validNorm[1] && onBox(clippy1))
        projectedRay = clippy1;
    if (validNorm[2] && onBox(clippy2))
        projectedRay = clippy2;
    if (validNorm[3] && onBox(clippy3))
        projectedRay = clippy3;
	
    return toZeroOne(projectedRay);
}

vec2 clampCircle(vec2 uv)
{
    vec2 origin = uv;
    vec2 absOrigin = abs(origin);
    origin /= max(absOrigin.x, absOrigin.y);
    return toZeroOne(origin);
}

struct PointLight
{
    float m_TypePad;
    float m_Range;
    int m_PadB;
    int m_PadC;
    vec4 m_Color;
    vec4 m_Position;
};

struct DirectionalLight
{
    vec4 m_Direction;
    vec4 m_Color;
};

struct ConicalLight
{
    int m_TypePad;
    float m_Range;
    float m_Angle;
    int m_Pad;
    vec4 m_Color;
    vec4 m_Position;
    vec4 m_Direction;
};

struct CylindricalLight
{
    int m_TypePad;
    float m_Range;
    int m_Pad;
    float m_Length;
    vec4 m_Color;
    vec2 m_Position;
    vec2 m_Position1;
    vec4 m_Direction;
};
