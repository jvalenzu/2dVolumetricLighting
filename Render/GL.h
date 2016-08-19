#pragma once

#if defined(WINDOWS)

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// #define GL_GLEXT_PROTOTYPES 1

#include <gl/gl.h>
#include "GL/glext.h"
#include "GL/wglext.h"

#include "Render/WindowsGL.h"

#define USE_CL 0

#else

#include <OpenCL/opencl.h>
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>

#define USE_CL 1
#endif

#include <GLFW/glfw3.h>
