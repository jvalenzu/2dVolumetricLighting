// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-
// baseline 11.76ms

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
#include "Render/Material.h"
#include "Tool/Utils.h"
#include "Tool/Test.h"

#define kNumFramesStep 40
#define GAME_NAME "foo"
#define VERSION "1.0"

Vec3 s_Target;
Vec3 s_LastDir;
Vec3 s_Dir;
float s_Angle;
SceneObject* s_SceneObject;

static void s_ProcessKeys(void* data, int key, int scanCode, int action, int mods);
static void MainLoop(RenderContext* renderContext);

void Test()
{
    AssetHandleTableTest();
    
    assert(Mat3Test());
}

static void ApplyUserInput(RenderContext* renderContext, SceneObject* sceneObject, const Vec3& targetPos)
{
    // handle user input
    float err = 0.0f;
    Vec3 pos = sceneObject->m_LocalToWorld.GetTranslation();
    Vec3 dir = targetPos - pos;
    if ((err = VectorLengthSquared(dir)) > 1e-3f)
    {
        dir.Normalize();
        dir *= fminf(err, 0.075f);
        pos += dir;
        
        sceneObject->m_LocalToWorld.SetTranslation(pos);
        sceneObject->m_Flags |= SceneObject::Flags::kDirty;
    }
    
    if (fabsf(s_Angle) > 1e-3f)
    {
        const float step_size = 0.1f;
        float tick = s_Angle*step_size;
        
        Mat4 rot;
        float uvw[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
        MatrixSetRotAboutAxis(&rot, uvw, tick*0.7853981634f);
        
        Mat4 t1;
        MatrixMultiply(&t1, rot, s_SceneObject->m_LocalToWorld);
        s_SceneObject->m_LocalToWorld = t1;
        sceneObject->m_Flags |= SceneObject::Flags::kDirty;
        
        s_Angle *= 1.0f - step_size;
    }
}

// s_ProcessKeys
//
// Callback what updates our key state
static void s_ProcessKeys(void* data, int key, int scanCode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        bool translated = false;
        float step = 1.0f;
        float x=0.0f, y=0.0f, z=0.0f;
        
        switch (key)
        {
            case GLFW_KEY_A:
            {
                translated = true;
                x += step;
                break;
            }
            case GLFW_KEY_D:
            {
                translated = true;
                x -= step;
                break;
            }
            case GLFW_KEY_W:
            {
                translated = true;
                y += step;
                break;
            }
            case GLFW_KEY_S:
            {
                translated = true;
                y -= step;
                break;
            }
            case GLFW_KEY_Z:
            {
                translated = true;
                z -= step;
                break;
            }
            case GLFW_KEY_X:
            {
                translated = true;
                z += step;
                break;
            }
        }
        
        Vec3 temp;
        VectorSet(&temp, x, y, z);
        
        s_LastDir = s_Dir;
        s_Dir.Normalize();
        
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
        {
            const float sign = key == GLFW_KEY_LEFT ? 1.0f : -1.0f;
            s_Angle = sign*0.7853981634f;
        }
        
        if (translated)
        {
            s_Target = s_SceneObject->m_LocalToWorld.GetTranslation();
            
            if (key == GLFW_KEY_W || key == GLFW_KEY_S)
                s_Target += s_SceneObject->m_LocalToWorld.GetUp() * (signbit(y)?-1.0f:1.0f) * step;
            else
                s_Target += s_SceneObject->m_LocalToWorld.GetRight() * (signbit(x)?-1.0f:1.0f) * step;
        }
        
        s_SceneObject->m_Flags |= SceneObject::Flags::kDirty;
    }
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
    
    RenderSetProcessKeysCallback(&renderContext, s_ProcessKeys);
    
    MainLoop(&renderContext);
    
    RenderContextDestroy(&renderContext);
    
    return 0;
}

static void MainLoop(RenderContext* renderContext)
{
    Scene scene;
    SceneCreate(&scene, 256);
    
    // move camera to 40.0f
    renderContext->m_Camera.SetTranslation(0.0f, 0.0f, 40.0f);
    MatrixInvert(&renderContext->m_View, renderContext->m_Camera);
    
    // LSP
    SpriteOptions spriteOptions;
    // spriteOptions.m_Pivot = Vec2(0.5f, 0.0f);
    SceneObject* sprite0 = s_SceneObject = SceneCreateSpriteFromFile(&scene, renderContext, "Avatar.png", spriteOptions);
    sprite0->m_LocalToWorld.SetTranslation(0, 0, -1);
    sprite0->m_Flags |= SceneObject::Flags::kDirty;
    sprite0->m_DebugName = "lsp";
    
    // initialize target
    s_Target = s_SceneObject->m_LocalToWorld.GetTranslation();
    VectorSplat(&s_Dir, 0.0f);
    VectorSplat(&s_LastDir, 0.0f);
    
    // create a group for the shadow casting things
    int shadowCasterGroupId = SceneGroupCreate(&scene);
    
    const float xes[] =
    {
        +0.0f,
        +20.0f,
        +0.0f,
        -20.0f
    };
    const float yes[] =
    {
        -20.0f,
        +15.0f,
        +20.0f,
        +15.0f
    };
    
    Texture* treeAppleTexture = TextureCreateFromFile("TreeApple.png");
    Texture* treeAppleNormal = TextureCreateFromFile("TreeApple_OUTPUT.png");
    Shader* planarShader = ShaderCreate("obj/Shader/Planar");
    Material* treeAppleMaterial = MaterialCreate(planarShader, treeAppleTexture);
    treeAppleMaterial->ReserveProperties(2);
    int planarTex = treeAppleMaterial->SetPropertyType("_PlanarTex", Material::MaterialPropertyType::kTexture);
    treeAppleMaterial->SetTexture(planarTex, treeAppleNormal);
    
    // debug material/shader
    Texture* whiteTexture = TextureCreateFromFile("white.png");
    Material* debugMaterial = MaterialCreate(planarShader, whiteTexture);
    debugMaterial->ReserveProperties(1);
    int debugPlanarTex = debugMaterial->SetPropertyType("_PlanarTex", Material::MaterialPropertyType::kTexture);
    debugMaterial->SetTexture(debugPlanarTex, whiteTexture);
    
    SceneObject* sceneObjects[4] = { 0 };
    for (int i=0; i<4; ++i)
    {
        SpriteOptions spriteOptionsTree;
        // spriteOptionsTree.m_Scale = Vec2(5.0f, 5.0f);
        
        SceneObject* sprite1 = sceneObjects[i] = SceneCreateSprite(&scene, renderContext, MaterialRef(treeAppleMaterial), spriteOptionsTree);
        sprite1->m_LocalToWorld.SetTranslation(xes[i], yes[i], -1);
        sprite1->m_Flags |= SceneObject::Flags::kDirty;
        
        SceneGroupAdd(&scene, shadowCasterGroupId, sprite1);
    }
    
    // create and attach a light to our mover
    spriteOptions.m_Pivot = Vec2(0.5f, 0.0f);
    spriteOptions.m_Scale = Vec2(0.5f, 0.5f);
    
    // point light
    SceneObject* lightSprite0;
    SceneObject* light0;
    {
        SpriteOptions pointLightSpriteOptions = spriteOptions;
        pointLightSpriteOptions.m_TintColor = Vec4(1.0f, 1.0f, 0.25f, 1.0f);
        pointLightSpriteOptions.ResetPivot();
        lightSprite0 = SceneCreateSpriteFromFile(&scene, renderContext, "Bulb.png", pointLightSpriteOptions);
        
        lightSprite0->m_LocalToWorld.SetTranslation(0.0f, 5.0f, -1);
        lightSprite0->m_Flags |= SceneObject::Flags::kDirty;
        
        light0 = SceneCreateLight(&scene, LightOptions::MakePointLight(Vec3(0.0f, 5.0f, -1.0f), // position
                                                                       pointLightSpriteOptions.m_TintColor, // color
                                                                       8.0f)); // range
        light0->m_DebugName = "PointLight";
        SceneGroupAddChild(s_SceneObject, lightSprite0);
        SceneGroupAddChild(lightSprite0, light0);
    }
    
    // conical light
    SceneObject* lightSprite1 = nullptr;
    SceneObject* light1 = nullptr;
    {
        SpriteOptions localSpriteOptions = spriteOptions;
        
        localSpriteOptions.m_Scale = Vec2(0.25f, 0.25f);
        localSpriteOptions.m_TintColor = Vec4(1.0f, 0.25f, 0.25f, 1.0f);
        lightSprite1 = SceneCreateSpriteFromFile(&scene, renderContext, "Beam.png", localSpriteOptions);
        lightSprite1->m_LocalToWorld.SetTranslation(0.0f, 5.0f, -1);
        lightSprite1->m_Flags |= SceneObject::Flags::kDirty;
        SceneGroupAddChild(s_SceneObject, lightSprite1);
        
        light1 = SceneCreateLight(&scene, LightOptions::MakeConicalLight(Vec3(0.0f, 5.0f, -1.0f),
                                                                         Vec3(0.0f, 1.0f, 0.0f),
                                                                         localSpriteOptions.m_TintColor, 30.0f, 15.0f));
        
        light1->m_DebugName = "ConicalLight";
        SceneGroupAddChild(lightSprite1, light1);
    }
    
    // cylindrical light
    SceneObject* lightSprite2 = nullptr;
    SceneObject* light2;
    {
        spriteOptions.m_TintColor = Vec4(0.25f, 0.25f, 1.0f, 1.0f);
        
        lightSprite2 = SceneCreateSpriteFromFile(&scene, renderContext, "Beam.png", spriteOptions);
        lightSprite2->m_LocalToWorld.SetTranslation(0.0f, 5.0f, -1.0f);
        lightSprite2->m_Flags |= SceneObject::Flags::kDirty;
        SceneGroupAddChild(s_SceneObject, lightSprite2);
        
        light2 = SceneCreateLight(&scene, LightOptions::MakeCylindricalLight(Vec3(0.0f, 5.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f),
                                                                             spriteOptions.m_TintColor,
                                                                             10.0f, // range
                                                                             5.0f)); // orthogonal range
        light2->m_DebugName = "Cylindrical Light";
        SceneGroupAddChild(lightSprite2, light2);
    }
    
    // directional light
    SceneObject* dirLights[4];
    {
        dirLights[0] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3( 1.0f, 0.0f, 0.0f), Vec4(0.50f, 0.50f, 0.50f, 0.0f)));
        dirLights[1] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3(-1.0f, 0.0f, 0.0f), Vec4(0.25f, 0.25f, 0.25f, 0.0f)));
        dirLights[2] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3( 0.0f, 1.0f, 0.0f), Vec4(0.25f, 0.25f, 0.25f, 0.0f)));
        dirLights[3] = SceneCreateLight(&scene, LightOptions::MakeDirectionalLight(Vec3( 0.0f,-1.0f, 0.0f), Vec4(0.50f, 0.50f, 0.50f, 0.0f)));
    }
    
    // blur temp textures
    Texture* renderTextureTemp[2];
    for (int i=0; i<2; ++i)
        renderTextureTemp[i] = TextureCreateRenderTexture(512, 512, 0);
    
    Shader* shaderBlurX = ShaderCreate("obj/Shader/BlurX");
    Shader* shaderBlurY = ShaderCreate("obj/Shader/BlurY");
    
    Texture* shadowCasterRenderTarget = TextureCreateRenderTexture(512, 512, 0);
    shadowCasterRenderTarget->SetClearFlags(Texture::RenderTextureFlags::kClearColor, 0,1,0,1);
    
    Shader* shadowCasterShader = ShaderCreate("obj/Shader/ShadowCasters");
    
    // 1d shadow map material and texture
    Shader* shadowMap1dShaders[4];
    shadowMap1dShaders[0] = nullptr;
    shadowMap1dShaders[1] = ShaderCreate("obj/Shader/ShadowMap1dPoint");
    shadowMap1dShaders[2] = ShaderCreate("obj/Shader/ShadowMap1dConical");
    shadowMap1dShaders[3] = ShaderCreate("obj/Shader/ShadowMap1dPoint");
    
    Material* shadow1dMaterials[4] = { nullptr };
    for (int i=1; i<4; ++i)
    {
        Material* shadow1dMaterial = shadow1dMaterials[i] = MaterialCreate(shadowMap1dShaders[i], shadowCasterRenderTarget);
        shadow1dMaterial->m_BlendMode = Material::BlendMode::kOpaque;
        shadow1dMaterial->ReserveProperties(2);
        shadow1dMaterial->SetPropertyType("_LightPosition", Material::MaterialPropertyType::kVec4);
        shadow1dMaterial->SetPropertyType("_LightFacingAngle", Material::MaterialPropertyType::kVec4);
    }
    
    Shader* sampleShadowMapShader = ShaderCreate("obj/Shader/SampleShadowMap");
    
    // show map stuff
    Material* shadowMapSampleMaterial = MaterialCreate(sampleShadowMapShader, nullptr);
    shadowMapSampleMaterial->m_BlendMode = Material::BlendMode::kBlend;
    shadowMapSampleMaterial->ReserveProperties(2);
    int shadowMapLightPosition = shadowMapSampleMaterial->SetPropertyType("_LightPosition", Material::MaterialPropertyType::kVec4);
    int shadowMapLightColor = shadowMapSampleMaterial->SetPropertyType("_LightColor", Material::MaterialPropertyType::kVec4);
    
    // light prepass stuff
    Shader* lightPrepassShader = ShaderCreate("obj/Shader/LightPrepass");
    int lightBitmaskIndex = RenderAddGlobalProperty(renderContext, "_LightBitmask", Material::MaterialPropertyType::kUInt);
    int lightPrepassTextureIndex = RenderAddGlobalProperty(renderContext, "_LightPrepassTex", Material::MaterialPropertyType::kTexture);
    Texture* renderTextureInt = TextureCreateRenderTexture(512, 512, 0, Texture::RenderTextureFormat::kUInt);
    RenderGlobalSetTexture(renderContext, lightPrepassTextureIndex, renderTextureInt);
    
    Shader* debugLightPrepassSampleShader = ShaderCreate("obj/Shader/DebugLightPrepassSample");
    
    // which light are we rendering?
    int light_state = 0;
    
    // for switching between normal/debug render modes
    int render_mode = 0;
    
    // initialize debug UI
    DebugUi::Init(renderContext, false);
    
    // point light enabled by default
    SceneSetEnabledRecursive(lightSprite0, true);
    SceneSetEnabledRecursive(lightSprite1, false);
    SceneSetEnabledRecursive(lightSprite2, false);

    // do we enable directional mode or not?
    int directional_mode = 0;
    
    // blur enabled or not?
    int blur_mode = 0;
    
    bool running = true;
    while (running)
    {
        // start render frame
        RenderFrameInit(renderContext);
        
        // IMGUI
        DebugUi::NewFrame();

        ImGui::Text("translate: a/s/d/f");
        ImGui::Text("rotate: <-/->");
        
        // DEBUG: 
        {
            constexpr const char* directional[] =
            {
                "direction on",
                "direction off"
            };
            if (ImGui::Button(directional[directional_mode]))
            {
                directional_mode = (directional_mode+1) & 1;
                for (int i=0; i<ELEMENTSOF(dirLights); ++i)
                    SceneSetEnabled(dirLights[i], directional_mode==0);
            }
        }
        
        // DEBUG: toggle blur
        {
            constexpr const char* blur_labels[] =
            {
                "blur on",
                "blur off"
            };
            if (ImGui::Button(blur_labels[blur_mode]))
                blur_mode = (blur_mode+1) & 1;
        }
        
        // DEBUG: switch which light we're using
        {
            constexpr const char* light_state_labels[] =
            {
                "point",
                "conical",
                "cylindrical"
            };
            
            enum LightState
            {
                kPoint,
                kConical,
                kCylindrical,
                kCount
            };
            
            if (ImGui::Button(light_state_labels[light_state]))
            {
                if (++light_state == LightState::kCount)
                    light_state = LightState::kPoint;
                
                switch (light_state)
                {
                    case LightState::kPoint:
                    {
                        SceneSetEnabledRecursive(lightSprite0, true);
                        SceneSetEnabledRecursive(lightSprite1, false);
                        SceneSetEnabledRecursive(lightSprite2, false);
                        break;
                    }
                    case LightState::kConical:
                    {
                        SceneSetEnabledRecursive(lightSprite0, false);
                        SceneSetEnabledRecursive(lightSprite1, true);
                        SceneSetEnabledRecursive(lightSprite2, false);
                        break;
                    }
                    case LightState::kCylindrical:
                    {
                        SceneSetEnabledRecursive(lightSprite0, false);
                        SceneSetEnabledRecursive(lightSprite1, false);
                        SceneSetEnabledRecursive(lightSprite2, true);
                        break;
                    }
                };
            }
            
            switch (light_state)
            {
                case LightState::kPoint:
                {
                    Light* light = SceneObjectGetLight(light0);
                    ImGui::DragFloat("range", &light->m_Range, 0.1f, 0.0f, 80.0f);
                    ImGui::ColorEdit3("color", light->m_Color.asFloat());
                    break;
                }
                case LightState::kConical:
                {
                    Light* light = SceneObjectGetLight(light1);
                    
                    float angle = light->m_CosAngle * 180.0f / float(M_PI);
                    ImGui::DragFloat("range", &light->m_Range, 0.1f, 0.0f, 80.0f);
                    ImGui::ColorEdit3("color", light->m_Color.asFloat());
                    if (ImGui::DragFloat("angle", &angle, 0.1f, 0.0f, 90.0f))
                        light->m_CosAngle = angle * float(M_PI)/180.0f;
                    break;
                }
                case LightState::kCylindrical:
                {
                    Light* light = SceneObjectGetLight(light2);
                    ImGui::DragFloat("range", &light->m_Range, 0.1f, 0.0f, 80.0f);
                    ImGui::ColorEdit3("color", light->m_Color.asFloat());
                    ImGui::DragFloat("range", &light->m_Range, 0.1f, 0.0f, 80.0f);
                    ImGui::DragFloat("orthogonal range", &light->m_OrthogonalRange, 0.1f, 0.0f, 80.0f);
                    
                    break;
                }
            }
            
            // render mode debug
            constexpr const char* render_mode_debug_labels[]
            {
                "normal",
                "coverage mask",
                "fullframe"
            };
            
            if (ImGui::Button(render_mode_debug_labels[render_mode]))
            {
                render_mode++;
                render_mode %= 3;
            }
            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        
        SceneUpdate(&scene);

        // 
        //        __                   __ 
        //       /\ \                 /\ \
        //   ____\ \ \___      __     \_\ \    ___   __  __  __
        //  /',__\\ \  _ `\  /'__`\   /'_` \  / __`\/\ \/\ \/\ \
        // /\__, `\\ \ \ \ \/\ \L\.\_/\ \L\ \/\ \L\ \ \ \_/ \_/ \
        // \/\____/ \ \_\ \_\ \__/.\_\ \___,_\ \____/\ \___x___/'
        //  \/___/   \/_/\/_/\/__/\/_/\/__,_ /\/___/  \/__//__/  
        //                                                       
        //
        
        // setup shadow caster render target
        RenderSetRenderTarget(renderContext, shadowCasterRenderTarget);
        RenderSetReplacementShader(renderContext, shadowCasterShader);
        
        // draw shadow casters
        SceneDraw(&scene, renderContext, shadowCasterGroupId);
        
        // tear down shadow caster render target
        RenderSetRenderTarget(renderContext, nullptr);
        RenderClearReplacementShader(renderContext);

        // for each light
        // - raymarch shadow casters into 1d polar coordinate render texture
        // - generate 2d fullscreen map from 1d render texture
        if (true)
        {
            // 4ms
            FixedVector<SceneObject*,32> lights;
            SceneGetSceneObjectsByType(&lights, &scene, SceneObjectType::kLight);
            for (int i=0,n=lights.Count(); i<n; ++i)
            {
                SceneObject* lightObject = lights[i];
                if (!SceneGetEnabled(lightObject))
                    continue;
                
                Light* light = SceneObjectGetLight(lightObject);
                if (light == nullptr)
                    continue;
                
                if (light->m_Type == LightType::kDirectional)
                    continue;
                
                // calculate the sceen position of our light source
                // jiv fixme: we already calculate this and cache it via SceneDraw
                Vec4 screenPos = RenderGetScreenPos(renderContext, lightObject->m_LocalToWorld.GetTranslation());
                
                // 1d mapping material
                Material* shadow1dMaterial = shadow1dMaterials[light->m_Type];
                
                // sample the 1d raycast texture.  Point/Spotlight sample based on light position to fragment, cylinder lights need to
                // raycast to the nearest intersection point
                shadowMapSampleMaterial->SetVector(shadowMapLightPosition, screenPos);
                shadowMapSampleMaterial->SetVector(shadowMapLightColor, light->m_Color);
                
                // set light position in screen space.  Relying on initialization order instead of explicit index
                shadow1dMaterial->SetVector(0, screenPos);
                
                if (light->m_Type == LightType::kConical)
                {
                    Vec4 direction = lightObject->m_LocalToWorld.GetUp();
                    direction.m_X[3] = light->m_CosAngle;
                    shadow1dMaterial->SetVector(1, direction);
                }
                
                // raymarch 1d polar coordinate map
                RenderSetRenderTarget(renderContext, lightObject->m_Shadow1dMap);
                RenderDrawFullscreen(renderContext, shadow1dMaterial, shadowCasterRenderTarget);
                RenderSetRenderTarget(renderContext, nullptr);
                
                // fullscreen 1d->2d pass
                RenderDrawFullscreen(renderContext, shadowMapSampleMaterial, lightObject->m_Shadow1dMap);
            }
        }

        // Run multiple blur passes on the current framebuffer, which just now consists only of the shadowed portions.
        // 3ms
        if (blur_mode == 0)
        {
            // ping pong blur buffers.  jesus this is a lot of passes
            const int limit=8;
            for (int i=0; i<limit; ++i)
            {
                const int current_render_target       = i&1;
                const int prev_and_next_render_target = current_render_target^1;
                
                Texture* source = renderTextureTemp[prev_and_next_render_target];
                if (i==0)
                    source = nullptr; // first read comes from frame buffer
                
                RenderSetRenderTarget(renderContext, renderTextureTemp[current_render_target]);
                RenderDrawFullscreen(renderContext, shaderBlurX, source);
                
                if (i==limit-1)
                    RenderSetRenderTarget(renderContext, nullptr);
                else
                    RenderSetRenderTarget(renderContext, renderTextureTemp[prev_and_next_render_target]);
                
                RenderDrawFullscreen(renderContext, shaderBlurY, renderTextureTemp[current_render_target]);
            }
        }
        
        if (true)
        {
            // light prepass
            
            // setup one of the temporary render texture targets to receive the light pass.  We'll render
            // out each light as an opaque OBB which approximates (conservatively) their area of influence
            renderTextureInt->SetClearFlags(Texture::RenderTextureFlags::kClearColor, 0,0,0,1);
            
#if 1
            RenderSetRenderTarget(renderContext, renderTextureInt);
            RenderSetReplacementShader(renderContext, lightPrepassShader);
#else
            // see the obb and faces in perspective projection
            RenderSetReplacementShader(renderContext, g_SimpleColorShader);
#endif
            
            // RenderSetReplacementShader resets this for now
            RenderSetBlendMode(Material::BlendMode::kOr);
            
            // save projection matrix
            FixedVector<SceneObject*,32> lights;
            SceneGetSceneObjectsByType(&lights, &scene, SceneObjectType::kLight);
            for (int i=0,n=lights.Count(); i<n; ++i)
            {
                SceneObject* lightObject = lights[i];
                if (!SceneGetEnabled(lightObject))
                    continue;
                
                Light* light = SceneObjectGetLight(lightObject);
                if (light == nullptr)
                    continue;
                
                if (light->m_Type == LightType::kDirectional)
                    continue;
                
                light->m_Index = i;
                RenderGlobalSetInt(renderContext, lightBitmaskIndex, 1U<<light->m_Index);
                
                const Vec3 cpMinusF = (renderContext->m_Camera.GetTranslation() - lightObject->m_LocalToWorld.GetTranslation()).Normalized();
                
                // rotate toward camera
                Mat4 transform;
                transform.SetRight(-PlaneProj(lightObject->m_LocalToWorld.GetRight(), cpMinusF));
                transform.SetUp(PlaneProj(lightObject->m_LocalToWorld.GetUp(), cpMinusF));
                transform.SetForward(cpMinusF);
                transform.SetTranslation(lightObject->m_LocalToWorld.GetTranslation());
                
                Mat4 axes;
                MatrixMakeIdentity(&axes);
                axes.SetRot(lightObject->m_Obb.m_Axes);
                
                Mat4 localToWorld = transform * axes;
                SceneDrawObb(&scene, renderContext, lightObject, localToWorld);
            }
            
            RenderSetReplacementShader(renderContext, nullptr);
            RenderSetRenderTarget(renderContext, nullptr);
            renderTextureInt->SetClearFlags(Texture::RenderTextureFlags::kClearNone);
        }
        
        // upload light data
        SceneLightsUpdate(&scene, renderContext);
        
        // draw actual scene
        SceneDraw(&scene, renderContext);
        
        // debug: draw fullscreen
        switch (render_mode)
        {
            case 1:
            {
                RenderDrawFullscreen(renderContext, debugLightPrepassSampleShader, renderTextureInt);
                break;
            }
            case 2:
            {
                RenderDrawFullscreen(renderContext, debugMaterial, whiteTexture);
                break;
            }
        }
        
        // apply the user input
        ApplyUserInput(renderContext, s_SceneObject, s_Target);

        ImGui::Render();
        
        running = RenderFrameEnd(renderContext);
    }
    
    DebugUi::Shutdown();
    
    // scene destroy
    SceneObjectDestroy(&scene, sprite0);
    SceneObjectDestroy(&scene, lightSprite0);
    SceneObjectDestroy(&scene, lightSprite1);
    SceneObjectDestroy(&scene, lightSprite2);
    for (int i=0; i<4; ++i)
        SceneObjectDestroy(&scene, sceneObjects[i]);
    
    // destroy blur textures and shaders
    for (int i=0; i<2; ++i)
        TextureDestroy(renderTextureTemp[i]);
    
    ShaderDestroy(shadowCasterShader);
    ShaderDestroy(sampleShadowMapShader);
    
    ShaderDestroy(shaderBlurX);
    ShaderDestroy(shaderBlurY);
    
    for (int i=0; i<4; ++i)
        ShaderDestroy(shadowMap1dShaders[i]);
    
    // destroy materials
    for (int i=0; i<4; ++i)
        MaterialDestroy(shadow1dMaterials[i]);
    
    MaterialDestroy(shadowMapSampleMaterial);
    
    TextureDestroy(treeAppleTexture);
    TextureDestroy(treeAppleNormal);
    TextureDestroy(whiteTexture);
    ShaderDestroy(planarShader);
    MaterialDestroy(treeAppleMaterial);
    
    ShaderDestroy(lightPrepassShader);
    ShaderDestroy(debugLightPrepassSampleShader);
    
    MaterialDestroy(debugMaterial);
    
    SceneDestroy(&scene);
}
