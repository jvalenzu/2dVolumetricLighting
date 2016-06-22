#include "Engine/Matrix.h"
#include "Engine/Obb.h"

void ObbInit(Obb* obb, float xExtent, float yExtent, float zExtent)
{
    MatrixMakeIdentity(&obb->m_LocalToWorld);
    
    obb->m_HalfExtents[0] = xExtent;
    obb->m_HalfExtents[1] = yExtent;
    obb->m_HalfExtents[2] = zExtent;
}

void ObbRender(const Obb& obb)
{
}
