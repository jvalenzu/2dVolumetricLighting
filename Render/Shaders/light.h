// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-


struct PointLight
{
    int m_TypePad;
    uint m_Index;
    float m_Range;
    int m_PadC;
    vec4 m_Color;
    vec4 m_Position;
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

struct ConicalLight
{
    int m_TypePad;
    uint m_Index;
    float m_Range;
    float m_Angle;
    vec4 m_Color;
    vec4 m_Position;
    vec4 m_Direction;
};

struct CylindricalLight
{
    int m_TypePad;
    uint m_Index;
    float m_Range;
    float m_Length;
    vec4 m_Color;
    vec2 m_Position;
    vec2 m_Position1;
    vec4 m_Direction;
};

layout (std140) uniform PointLightData
{
    PointLight _PointLight[32];
};

layout (std140) uniform CylindricalLightData
{
    CylindricalLight _CylindricalLight[32];
};

layout (std140) uniform ConicalLightData
{
    ConicalLight _ConicalLight[32];
};
