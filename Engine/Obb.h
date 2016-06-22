#pragma once

#include <stdint.h>
#include "Engine/Matrix.h"

struct Obb
{
    Mat4 m_LocalToWorld;
    float m_HalfExtents[3];
    uint32_t m_Pad;
};

void ObbInit(Obb* obb);
void ObbRender(const Obb& obb);


