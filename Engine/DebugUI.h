// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "imgui/imgui.h"

struct GLFWwindow;
struct RenderContext;

namespace DebugUi
{
  void RenderDrawLists(ImDrawData* draw_data);
  void MouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/);
  void ScrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset);
  void KeyCallback(GLFWwindow*, int key, int, int action, int mods);
  void CharCallback(GLFWwindow*, unsigned int c);
  bool CreateFontsTexture();
  bool CreateDeviceObjects();
  void InvalidateDeviceObjects();
  
  bool Init(RenderContext* context, bool install_callbacks);
  void Shutdown();
  void NewFrame();
}
