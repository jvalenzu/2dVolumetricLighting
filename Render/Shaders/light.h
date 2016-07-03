
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
