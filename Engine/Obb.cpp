#include "Engine/Matrix.h"
#include "Engine/Obb.h"
#include "Engine/Utils.h"

void ObbInit(Obb* obb)
{
    MatrixMakeIdentity(&obb->m_Axes);
    
    obb->m_HalfExtents[0] = 0.0f;
    obb->m_HalfExtents[1] = 0.0f;
    obb->m_HalfExtents[2] = 0.0f;
}

void ObbRender(const Obb& obb)
{
}

void ObbDump(const Obb& obb, const char* prefix)
{
    Printf("%s obb\n", prefix);
    MatrixDump(obb.m_Axes, "    localToWorld");
    Printf("extents x: %f y: %f: z: %f\n", obb.m_HalfExtents[0], obb.m_HalfExtents[1], obb.m_HalfExtents[2]);
}
