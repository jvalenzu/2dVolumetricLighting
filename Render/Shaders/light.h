// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

struct Light
{
    int m_TypePad;
    uint m_Index;
    float m_Range;
    float m_AngleOrLength;
    vec4 m_Color;
    vec4 m_Position;
    vec4 m_Direction;
};

struct DirectionalLight
{
    int m_TypePad;
    int m_IndexPad;
    int m_RangePad;
    int m_Pad;
    vec4 m_Color;
    vec4 m_PositionPad;
    vec4 m_Direction;
};

layout (std140) uniform PointLightData
{
    Light _PointLight[32];
};

layout (std140) uniform CylindricalLightData
{
    Light _CylindricalLight[32];
};

layout (std140) uniform ConicalLightData
{
    Light _ConicalLight[32];
};

uniform uint pointLightMask;
uniform uint cylindricalLightMask;
uniform uint conicalLightMask;
