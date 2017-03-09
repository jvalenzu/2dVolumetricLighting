// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

struct RenderContext;
struct ModelClass;
struct ModelClassSubset;

void RenderSetGlobalConstants(RenderContext* renderContext, int* textureSlotItr, int programName);
void RenderSetLightConstants(RenderContext* renderContext, const Shader* shader);

float RenderWorldToScreenDistance(RenderContext* renderContext, float worldDistance);

ModelClass* RenderGenerateCubeModelClass(RenderContext* renderContext);

void ModelClassSubsetCalcBSphere(ModelClassSubset* inplace);

void RenderGenerateCubeModelClass(RenderContext* renderContext, ModelClass* dest);
void RenderGenerateSpriteModelClass(RenderContext* renderContext, ModelClass* modelClass);
