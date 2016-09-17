// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

void RenderSetGlobalConstants(RenderContext* renderContext, int* textureSlotItr, int programName);
void RenderSetLightConstants(RenderContext* renderContext, const Shader* shader);

float RenderWorldToScreenDistance(RenderContext* renderContext, float worldDistance);
