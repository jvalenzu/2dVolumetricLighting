// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include <stdint.h>
#include "Engine/Matrix.h"

struct Obb
{
    Mat3 m_Axes;
    float m_HalfExtents[3];
    uint32_t m_Pad;
};

void ObbInit(Obb* obb);
void ObbRender(const Obb& obb);
void ObbDump(const Obb& obb, const char* prefix="");


