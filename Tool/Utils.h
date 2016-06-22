#pragma once

#define ELEMENTS_OF(x) ((sizeof(x)/sizeof(*(x))))

#include "Engine/Matrix.h"
#include "Engine/Obb.h"

struct SimpleModel;

void ToolLoadPerspective(Mat4* mtx, float fov, float aspect, float nearZ, float farZ);
void ToolLoadOrthographic(Mat4* dest, float left, float right, float bottom, float top, float nearZ, float farZ);

Obb ToolGenerateObbFromSimpleModel(const SimpleModel* model);

