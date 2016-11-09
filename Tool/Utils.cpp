// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include "Engine/Matrix.h"
#include "Engine/Utils.h"
#include "Render/Render.h"
#include "Tool/Utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void ToolLoadPerspective(Mat4* mtx, float fov, float aspect, float nearZ, float farZ)
{
    const float f = 1.0f / tanf(0.5f * fov);
    
    // identity
    MatrixMakeIdentity(mtx);
    
    //  0   1   2   3
    //  4   5   6   7
    //  8   9  10  11
    // 12  13  14  15
    
    // mtx
    mtx->m_X[0] = f / aspect;
    mtx->m_Y[1] = f;
    mtx->m_Z[2] = (farZ + nearZ) / (nearZ - farZ);
    mtx->m_Z[3] = -1.0f;
    mtx->m_W[2] = 2.0f * (farZ * nearZ) / (nearZ - farZ);
}

void ToolLoadOrthographic(Mat4* dest, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float* mtx = dest->asFloat();
    
    mtx[ 0] = 2.0f / (right - left);
    mtx[ 1] = 0.0f;
    mtx[ 2] = 0.0f;
    mtx[ 3] = 0.0f;
    
    mtx[ 4] = 0.0f;
    mtx[ 5] = 2.0f / (top - bottom);
    mtx[ 6] = 0.0f;
    mtx[ 7] = 0.0f;
    
    mtx[ 8] = 0.0f;
    mtx[ 9] = 0.0f;
    mtx[10] = -2.0f / (farZ - nearZ);
    mtx[11] = 0.0f;
    
    mtx[12] = -(right + left) / (right - left);
    mtx[13] = -(top + bottom) / (top - bottom);
    mtx[14] = -(farZ + nearZ) / (farZ - nearZ);
    mtx[15] = 1.0f;
}

Obb ToolGenerateObbFromSimpleModel(const SimpleModel* model)
{
    Obb ret;
    ObbInit(&ret);
    
    if (model->m_NumVertices)
    {
        float m[3] = { 0.0f, 0.0f, 0.0f };
        
        for (int i=0,n=model->m_NumVertices; i<n; ++i)
        {
            const SimpleVertex& vertex = model->m_Vertices[i];
            m[0] += vertex.m_Position[0];
            m[1] += vertex.m_Position[1];
            m[2] += vertex.m_Position[2];
        }
        
        const float recipN = 1.0f / model->m_NumVertices;
        m[0] *= recipN;
        m[1] *= recipN;
        m[2] *= recipN;
        
        Mat3 covar;
        MatrixMakeZero(&covar);
        
        float* covarf = covar.asFloat();
        
        // covariance (xi, xj) = (E[(xi - mi)(xj - mj)])
        for (int c=0,n=model->m_NumVertices; c<n; ++c)
        {
            const float* f = model->m_Vertices[c].m_Position;
            
            for (int i=0; i<3; ++i)
            {
                for (int j=0; j<3; ++j)
                {
                    for (int k=0; k<3; ++k)
                    {
                        const float x_variance = f[i] - m[i];
                        const float y_variance = f[j] - m[j];
                        const float z_variance = f[k] - m[k];
                        
                        const float x_variance_2 = x_variance*x_variance;
                        const float y_variance_2 = y_variance*y_variance;
                        const float z_variance_2 = z_variance*z_variance;
                        
                        covarf[i*3+j] += x_variance_2*x_variance_2 + y_variance_2*y_variance_2 + z_variance_2*z_variance_2;
                    }
                }
            }
        }
        
        // eigendecompose
        Mat3 s;
        Mat3 sInv;
        
        float lambdas[3];
        Mat3Diagonalize(&s, lambdas, &sInv, covar);
        
        ret.m_Axes = s;
        
        for (int i=0; i<3; ++i)
            ret.m_HalfExtents[i] = lambdas[i] * 0.5f;
    }
    
    return ret;
}

// jiv fixme duplication
Obb ToolGenerateObbFromVec3(const Vec3* vertices, size_t n)
{
    Obb ret;
    ObbInit(&ret);
    
    float m[3] = { 0.0f, 0.0f, 0.0f };
    
    for (int i=0; i<n; ++i)
    {
        m[0] += vertices[i].m_X[0];
        m[1] += vertices[i].m_X[1];
        m[2] += vertices[i].m_X[2];
    }
    
    const float recipN = 1.0f / n;
    m[0] *= recipN;
    m[1] *= recipN;
    m[2] *= recipN;
    
    Mat3 covar;
    float* covarf = covar.asFloat();
    
#if 0
    Printf("X=[");
    for (int c=0; c<n; ++c)
    {
        const float* f = vertices[c].asFloat();
        Printf("%f%s", f[0], (c==n-1)?"":",");
    }
    Printf("]\n");
    Printf("Y=[");
    for (int c=0; c<n; ++c)
    {
        const float* f = vertices[c].asFloat();
        Printf("%f%s", f[1], (c==n-1)?"":",");
    }
    Printf("]\n");
#endif
    
    // covariance (xi, xj) = Σ[(xi - mi)(xj - mj)]
    for (int i=0; i<3; ++i)
    {
        for (int j=0; j<3; ++j)
        {
            covarf[i*3+j] = 0.0f;
            
            for (int c=0; c<n; ++c)
            {
                const float* f = vertices[c].asFloat();
                
                const float x_variance = f[i] - m[i];
                const float y_variance = f[j] - m[j];
                
                covarf[i*3+j] += x_variance*y_variance;
            }
            
            covarf[i*3+j] /= (n-1);
        }
    }
    
    // eigendecompose
    Mat3 sInv;
    
    float lambdas[3];
    Mat3Diagonalize(&ret.m_Axes, lambdas, &sInv, covar);
    
    for (int i=0; i<3; ++i)
        ret.m_HalfExtents[i] = lambdas[i] * 0.5f;
    
    return ret;
}
