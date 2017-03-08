// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slib/Common/Util.h"
#include "slib/Container/FixedVector.h"
#include "slib/Container/LinkyList.h"
#include "Engine/DebugUi.h"
#include "Engine/Light.h"
#include "Engine/Scene.h"
#include "Engine/Utils.h"
#include "Render/Asset.h"
#include "Render/Render.h"
#include "Render/Model.h"
#include "Render/Material.h"
#include "Tool/Utils.h"
#include "Tool/Test.h"

#define kNumFramesStep 40
#define GAME_NAME "foo"
#define VERSION "1.0"

struct InputValues
{
    bool m_KeyInput;
    int m_Key;
    int m_ScanCode;
    int m_KeyAction;
    int m_KeyMods;
    
    bool m_MouseInput;
    int m_Button;
    int m_MouseAction;
    int m_MouseMods;
    
    float m_CursorPosition[2];
};

struct UserInputState
{
    bool m_Dragging;
    float m_DragStart[2];
    UserInputState()
        : m_Dragging(false)
    {
    }
};

struct CameraControllerState
{
    float m_Pitch;
    float m_Yaw;
    float m_Zoom;
    CameraControllerState()
        : m_Pitch(0.0f)
        , m_Yaw(0.0f)
        , m_Zoom(10.0f)
    {
    }
};

static InputValues s_InputValues;

static void ProcessKeys(void* data, int key, int scanCode, int action, int mods);
static void ProcessMouse(void* data, int button, int action, int mods);
static void ProcessCursorPosition(void* data, double x, double y);

static void MainLoop(RenderContext* renderContext);

void Test()
{
    AssetHandleTableTest();
    
    assert(Mat3Test());
}

static void ApplyUserInput(RenderContext* renderContext, UserInputState* userInputState, CameraControllerState* cameraControllerState, Mat4* cameraLocalToWorld)
{
    if (s_InputValues.m_KeyInput)
    {
        s_InputValues.m_KeyInput = false;
    }
    
    if (s_InputValues.m_MouseInput)
    {
        s_InputValues.m_MouseInput = false;
        
        if (s_InputValues.m_MouseAction == GLFW_PRESS)
        {
            userInputState->m_Dragging = true;
            userInputState->m_DragStart[0] = s_InputValues.m_CursorPosition[0];
            userInputState->m_DragStart[1] = s_InputValues.m_CursorPosition[1];
        }
        else if (s_InputValues.m_MouseAction == GLFW_RELEASE)
        {
            userInputState->m_Dragging = false;
        }
    }
    
    if (userInputState->m_Dragging)
    {
        Vec2 direction = Vec2(s_InputValues.m_CursorPosition[0] - userInputState->m_DragStart[0],
                              s_InputValues.m_CursorPosition[1] - userInputState->m_DragStart[1]);
        direction[0] /= renderContext->m_Width;
        direction[1] /= renderContext->m_Height;
        
        if (s_InputValues.m_MouseMods & GLFW_MOD_CONTROL)
        {
            if (VectorDot(direction, Vec2(0,1)) < -1e-1f)
            {
                cameraControllerState->m_Zoom -= 0.01f;
            }
            else if (VectorDot(direction, Vec2(0,1)) > 1e-1f)
            {
                cameraControllerState->m_Zoom += 0.01f;
            }
            
            cameraControllerState->m_Zoom = Clamp(cameraControllerState->m_Zoom, 1.0f, 10.0f);
        }
        else
        {
            cameraControllerState->m_Yaw   += 0.1f*direction[0] * M_PI * 0.5f;
            cameraControllerState->m_Pitch += 0.1f*direction[1] * M_PI * 0.5f;
        }
        
        Vec3 translation(0,0,cameraControllerState->m_Zoom);
        
        const float kExtent = 0.639065f; // determined experimentally
        cameraControllerState->m_Yaw = Clamp(cameraControllerState->m_Yaw, -kExtent, kExtent);
        cameraControllerState->m_Pitch = Clamp(cameraControllerState->m_Pitch, -kExtent, kExtent);
        
        Mat4 xrot;
        MatrixSetRotAboutAxis(&xrot, Vec3(0,1,0), cameraControllerState->m_Yaw);
        Mat4 yrot;
        MatrixSetRotAboutAxis(&yrot, Vec3(1,0,0), cameraControllerState->m_Pitch);
        
        Mat4 properTranslation;
        MatrixMakeIdentity(&properTranslation);
        properTranslation.SetTranslation(translation);
        
        *cameraLocalToWorld = properTranslation * xrot * yrot;
    }
}

static void ProcessCursor(void* data, double x, double y)
{
    s_InputValues.m_CursorPosition[0] = (float) x;
    s_InputValues.m_CursorPosition[1] = (float) y;
}

static void ProcessKeys(void* data, int key, int scanCode, int action, int mods)
{
    s_InputValues.m_KeyInput = true;
    s_InputValues.m_Key = key;
    s_InputValues.m_ScanCode = scanCode;
    s_InputValues.m_KeyAction = action;
    s_InputValues.m_KeyMods = mods;
}

static void ProcessMouse(void* data, int button, int action, int mods)
{
    s_InputValues.m_MouseInput = true;
    s_InputValues.m_Button = button;
    s_InputValues.m_MouseAction = action;
    s_InputValues.m_MouseMods = mods;
}

int main(int argc, char* argv[])
{
    if (argc > 1 && !strcmp(argv[1], "--test"))
    {
        Test();
        return 0;
    }
    
    int width = -1;
    int height = -1;
    
    RenderContext renderContext;
    RenderInit(&renderContext, width, height);
    
    RenderSetProcessKeysCallback(&renderContext, ProcessKeys);
    RenderSetProcessMouseCallback(&renderContext, ProcessMouse);
    RenderSetProcessCursorCallback(&renderContext, ProcessCursor);
    
    MainLoop(&renderContext);
    
    RenderContextDestroy(&renderContext);
    
    return 0;
}

static void MainLoop(RenderContext* renderContext)
{
    CameraControllerState cameraControllerState;
    
    Scene scene;
    SceneCreate(&scene, 256);
    
    // set the clear color
    RenderSetClearColor(renderContext, Vec3(0.25,0.45,0.45));
    
    // move camera to 40.0f
    renderContext->m_Camera.SetTranslation(0.0f, 0.0f, cameraControllerState.m_Zoom);
    MatrixInvert(&renderContext->m_View, renderContext->m_Camera);
    
    SceneObject* triangleList = SceneCreateEmpty(&scene);
    {
        ModelClass* modelClass = ModelClassCreate(renderContext, "Rooster.bin");
        triangleList->m_ModelInstance = new ModelInstance();
        triangleList->m_LocalToWorld.Scale(Vec3(3.0f, 3.0f, 3.0f));
        MatrixMakeIdentity(&triangleList->m_ModelInstance->m_Po);
        
        triangleList->m_ModelInstance->m_ModelClass = modelClass;
        triangleList->m_Obb = ToolGenerateObbFromModelClass(modelClass);
        triangleList->m_Flags |= SceneObject::Flags::kDirty;
    }
    
    // directional light
    const float kWeak = 0.1875f;
    const float kStrong = 0.2625f;
    SceneObject* dirLights[4];
    dirLights[0] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3(+1, +1, +1), Vec4(  kWeak,   kWeak, kStrong, 1.0f)));
    dirLights[1] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3(-1, -1, -1), Vec4(kStrong,   kWeak,   kWeak, 1.0f)));
    dirLights[2] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3(+1, -1, -1), Vec4(  kWeak, kStrong,   kWeak, 1.0f)));
    dirLights[3] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3(+0, -1, +0), Vec4(  kWeak,   kWeak, kStrong, 1.0f)));
    
    // initialize debug UI
    DebugUi::Init(renderContext, false);
    
    UserInputState userInputState;
    
    bool running = true;
    while (running)
    {
        // start render frame
        RenderFrameInit(renderContext);
        
        // IMGUI
        DebugUi::NewFrame();
        
        // scene update
        SceneUpdate(&scene);
        
        // upload light data
        SceneLightsUpdate(&scene, renderContext);
        
        // draw actual scene
        SceneDraw(&scene, renderContext);
        
        // apply the user input
        ApplyUserInput(renderContext, &userInputState, &cameraControllerState, &renderContext->m_Camera);
        MatrixInvert(&renderContext->m_View, renderContext->m_Camera);
        
        ImGui::Render();
        running = RenderFrameEnd(renderContext);
    }
    
    DebugUi::Shutdown();
    
    SceneDestroy(&scene);
}
