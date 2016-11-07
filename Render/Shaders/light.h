// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

struct Light
{
    int m_TypePad;
    uint m_Index;
    float m_Range;
    float m_Angle;
    vec4 m_Color;
    vec4 m_Position;
    vec4 m_Direction;
    float m_OrthogonalRange;
    int m_PadA;
    int m_PadB;
    int m_PadC;
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

layout (std140) uniform ConicalLightData
{
    Light _ConicalLight[32];
};

layout (std140) uniform CylindricalLightData
{
    Light _CylindricalLight[32];
};

layout (std140) uniform DirectionalLightData
{
    Light _DirectionalLight[32];
};
uniform uint numDirectionalLights;

uniform uint pointLightMask;
uniform uint conicalLightMask;
uniform uint cylindricalLightMask;
