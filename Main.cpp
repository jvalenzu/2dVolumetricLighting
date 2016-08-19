// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-
// baseline 11.76ms


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Engine/Container/FixedVector.h"
#include "Engine/Container/LinkyList.h"
#include "Engine/Light.h"
#include "Engine/Scene.h"
#include "Engine/Utils.h"
#include "Render/Asset.h"
#include "Render/Render.h"
#include "Render/PostEffect.h"
#include "Render/Material.h"
#include "Tool/Test.h"

#define kNumFramesStep 40
#define GAME_NAME "foo"
#define VERSION "1.0"

int nbFrames;
double lastTime;

Vec3 s_Target;
Vec3 s_LastDir;
Vec3 s_Dir;
int s_Strength = 0;

SceneObject* s_SceneObject;
static void s_ProcessKeys(void* data, int key, int scanCode, int action, int mods);
static void MainLoop(RenderContext* renderContext);

static float s_X = 0.0f;
static float s_Y = 0.0f;
static float s_Z = 0.0f;

void Test()
{
    AssetHandleTableTest();
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
    Mat4ApplyTranslation(&renderContext->m_Camera, 0.0f, 0.0f, 40.0f);
    MatrixInvert(&renderContext->m_View, renderContext->m_Camera);
    
    // LSP
    SpriteOptions spriteOptions;
    // spriteOptions.m_Pivot = Vec2(0.5f, 0.0f);
    SceneObject* sprite0 = s_SceneObject = SceneCreateSpriteFromFile(&scene, "Avatar.png", spriteOptions);
    Mat4ApplyTranslation(&sprite0->m_LocalToWorld, 0, 0, -1);
    sprite0->m_Flags |= SceneObject::Flags::kDirty;
    sprite0->m_DebugName = "lsp";
    
    RenderSetAmbientLight(renderContext, Vec4(0.5f, 0.5f, 0.5f, 0.5f));
    
    // initialize target
    s_Target = Mat4GetTranslation(s_SceneObject->m_LocalToWorld);
    VectorSplat(&s_Dir, 0.0f);
    VectorSplat(&s_LastDir, 0.0f);
    
    // create a group for the shadow casting things
    int shadowCasterGroupId = SceneGroupCreate(&scene);
    
    float xes[] =
    {
        +0.0f,
        +20.0f,
        +0.0f,
        -20.0f
    };
    float yes[] =
    {
        -20.0f,
        +15.0f,
        +20.0f,
        +15.0f
    };
    
    Texture* treeAppleTexture = TextureCreateFromFile("TreeApple.png");
    Texture* treeAppleNormal = TextureCreateFromFile("TreeApple_OUTPUT.png");
    Shader* outlineLightShader = ShaderCreate("obj/Shader/Planar");
    Material* treeAppleMaterial = MaterialCreate(outlineLightShader, treeAppleTexture);
    MaterialReserveProperties(treeAppleMaterial, 2);
    MaterialSetMaterialPropertyType(treeAppleMaterial, 0, "_PlanarTex", Material::MaterialPropertyType::kTexture);
    MaterialSetMaterialPropertyTexture(treeAppleMaterial, 0, treeAppleNormal);
    
    SceneObject* sceneObjects[4] = { 0 };
    for (int i=0; i<4; ++i)
    {
        SpriteOptions spriteOptionsTree;
        // spriteOptionsTree.m_Scale = Vec2(5.0f, 5.0f);
        
        SceneObject* sprite1 = sceneObjects[i] = SceneCreateSprite(&scene, MaterialRef(treeAppleMaterial), spriteOptionsTree);
        Mat4ApplyTranslation(&sprite1->m_LocalToWorld, xes[i], yes[i], -1);
        sprite1->m_Flags |= SceneObject::Flags::kDirty;
        
        SceneGroupAdd(&scene, shadowCasterGroupId, sprite1);
    }
    
    // create and attach a light to our mover
    spriteOptions.m_Pivot = Vec2(0.5f, 0.0f);
    spriteOptions.m_Scale = Vec2(0.5f, 0.5f);
    
    // conical light
    spriteOptions.m_TintColor = Vec4(1.0f, 0.25f, 0.25f, 1.0f);
    SceneObject* lightSprite0 = SceneCreateSpriteFromFile(&scene, "Beam.png", spriteOptions);
    Mat4ApplyTranslation(&lightSprite0->m_LocalToWorld, 10, 0, -1);
    lightSprite0->m_Flags |= SceneObject::Flags::kDirty;
    SceneGroupAddChild(s_SceneObject, lightSprite0);
    
    // rotate lightSprite0 so it's more obvious what directory it's casting.
    {
        Mat4 rot;
        float uvw[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
        MatrixSetRotAboutAxis(&rot, uvw, 1.5707963268f);
    
        Mat4 t1;
        MatrixMultiply(&t1, rot, lightSprite0->m_LocalToWorld);
        MatrixCopy(&lightSprite0->m_LocalToWorld, t1);
    }
    
    SceneObject* light0 = SceneCreateLight(&scene, LightOptions::MakeConicalLight(Vec3(10.0f, 0.0f, -1.0f),
                                                                                  Vec3( 0.0f, 1.0f,  0.0f),
                                                                                  spriteOptions.m_TintColor,
                                                                                  45.0f,
                                                                                  20.0f));
    SceneGroupAddChild(lightSprite0, light0);
    
    // point light
    spriteOptions.m_TintColor = Vec4(1.0f, 1.0f, 0.25f, 1.0f);
    SceneObject* lightSprite1 = SceneCreateSpriteFromFile(&scene, "Beam.png", spriteOptions);
    Mat4ApplyTranslation(&lightSprite1->m_LocalToWorld, -10, 0, -1);
    lightSprite1->m_Flags |= SceneObject::Flags::kDirty;
    SceneGroupAddChild(s_SceneObject, lightSprite1);
    
    SceneObject* light1 = SceneCreateLight(&scene, LightOptions::MakePointLight(Vec3(-10.0f, 0.0f, -1.0f), spriteOptions.m_TintColor, 2.0f));
    SceneGroupAddChild(lightSprite1, light1);
    
    // cylindrical light
    spriteOptions.m_TintColor = Vec4(0.25f, 0.25f, 1.0f, 1.0f);
    
    SceneObject* lightSprite2 = SceneCreateSpriteFromFile(&scene, "Beam.png", spriteOptions);
    Mat4ApplyTranslation(&lightSprite2->m_LocalToWorld, 0.0f, 5.0f, -1.0f);
    lightSprite2->m_Flags |= SceneObject::Flags::kDirty;
    SceneGroupAddChild(s_SceneObject, lightSprite2);
    
    SceneObject* light2 = SceneCreateLight(&scene, LightOptions::MakeCylindricalLight(Vec3(0.0f, 5.0f, -1.0f),
                                                                                      Vec3(0.0f, 1.0f,  0.0f),
                                                                                      spriteOptions.m_TintColor,
                                                                                      0.5f,
                                                                                      10.0f));
    SceneGroupAddChild(lightSprite2, light2);
    
    // blur temp textures
    Texture* renderTextureTemp[2];
    for (int i=0; i<2; ++i)
        renderTextureTemp[i] = TextureCreateRenderTexture(512, 512, 0);
    
    Shader* shaderBlurX = ShaderCreate("obj/Shader/BlurX");
    Shader* shaderBlurY = ShaderCreate("obj/Shader/BlurY");
    
    // blur post effects
    // PostEffect* postEffect0 = PostEffectInit(ShaderCreate("Render/Shaders/BlurX"));
    // PostEffect* postEffect1 = PostEffectInit(ShaderCreate("Render/Shaders/BlurY"));
    
    // RenderAttachPostEffect(renderContext, postEffect0);
    // RenderAttachPostEffect(renderContext, postEffect1);
    
    float dir[3] = { 1, 0, 0 };
    
    Texture* shadowCasterRenderTarget = TextureCreateRenderTexture(512, 512, 0);
    TextureSetClearFlags(shadowCasterRenderTarget, Texture::RenderTextureFlags::kClearColor, 0,1,0,1);
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
        shadow1dMaterial->ReserveProperties(2);
        MaterialSetMaterialPropertyType(shadow1dMaterial, 0, "_LightPosition", Material::MaterialPropertyType::kVec4);
        MaterialSetMaterialPropertyType(shadow1dMaterial, 1, "_LightFacingAngle", Material::MaterialPropertyType::kVec4);
        shadow1dMaterial->m_BlendMode = Material::BlendMode::kOpaque;
    }
    
    Shader* sampleShadowMapShader = ShaderCreate("obj/Shader/SampleShadowMap");
    
    Material* shadowMapSampleSimpleMaterial = MaterialCreate(sampleShadowMapShader, nullptr);
    shadowMapSampleSimpleMaterial->ReserveProperties(2);
    MaterialSetMaterialPropertyType(shadowMapSampleSimpleMaterial, 0, "_LightPosition", Material::MaterialPropertyType::kVec4);
    MaterialSetMaterialPropertyType(shadowMapSampleSimpleMaterial, 1, "_LightColor", Material::MaterialPropertyType::kVec4);
    shadowMapSampleSimpleMaterial->m_BlendMode = Material::BlendMode::kBlend;
    
    Material* shadowMapSampleMaterials[] =
    {
        shadowMapSampleSimpleMaterial,
        shadowMapSampleSimpleMaterial,
        shadowMapSampleSimpleMaterial,
        shadowMapSampleSimpleMaterial,
    };
    
    bool running = true;
    while (running)
    {
        RenderFrameInit(renderContext);
        
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
        
        // draw shadow casters
        RenderSetRenderTarget(renderContext, shadowCasterRenderTarget);
        RenderSetReplacementShader(renderContext, shadowCasterShader);
        
        SceneDraw(&scene, renderContext, shadowCasterGroupId);
        
        RenderSetRenderTarget(renderContext, nullptr);
        RenderClearReplacementShader(renderContext);
        
        if (true)
        {
            // 4ms
            FixedVector<SceneObject*,32> lights;
            SceneGetSceneObjectsByType(&lights, &scene, SceneObjectType::kLight);
            for (int i=0,n=lights.Count(); i<n; ++i)
            {
                SceneObject* lightObject = lights[i];
                Light* light = SceneObjectGetLight(lightObject);
                if (light == nullptr)
                    continue;
                
                // calculate the sceen position of our light source
                // jiv fixme: we already calculate this and cache it via SceneDraw
                Vec4 screenPos = RenderGetScreenPos(renderContext, Mat4GetTranslation(lightObject->m_LocalToWorld));
                
                // 1d mapping material
                Material* shadow1dMaterial = shadow1dMaterials[light->m_Type];
                
                // sample the 1d raycast texture.  Point/Spotlight sample based on light position to fragment, cylinder lights need to
                // raycast to the nearest intersection point
                Material* shadowMapSampleMaterial = shadowMapSampleMaterials[light->m_Type];
                shadowMapSampleMaterial->SetVector(0, screenPos);
                shadowMapSampleMaterial->SetVector(1, ((PointLight*)light)->m_Color);
                
                // set light position
                shadow1dMaterial->SetVector(0, screenPos);
                
                if (light->m_Type == LightType::kConical)
                {
                    Vec4 direction = Mat4GetRight(lightObject->m_LocalToWorld);
                    direction.m_X[3] = ((ConicalLight*)light)->m_CosAngle;
                    shadow1dMaterial->SetVector(1, direction);
                }
                
                // generate 1d shadow map
                RenderSetRenderTarget(renderContext, lightObject->m_Shadow1dMap);
                RenderDrawFullscreen(renderContext, shadow1dMaterial, shadowCasterRenderTarget);
                RenderSetRenderTarget(renderContext, nullptr);
                
                RenderDrawFullscreen(renderContext, shadowMapSampleMaterial, lightObject->m_Shadow1dMap);
            }
        }
        
        // 3ms
        if (true)
        {
            // blur
            RenderSetRenderTarget(renderContext, renderTextureTemp[0]);
            RenderDrawFullscreen(renderContext, shaderBlurX, nullptr);
            RenderSetRenderTarget(renderContext, renderTextureTemp[1]);
            RenderDrawFullscreen(renderContext, shaderBlurY, renderTextureTemp[0]);
            
            const int limit=8;
            for (int i=1; i<limit; ++i)
            {
                const int prevTextureIndex = (i-1)%2;
                const int nextTextureIndex = (i+1)%2;
                const int textureIndex = i%2;
                
                RenderSetRenderTarget(renderContext, renderTextureTemp[textureIndex]);
                RenderDrawFullscreen(renderContext, shaderBlurX, renderTextureTemp[prevTextureIndex]);
                
                if (i==(limit-1))
                    RenderSetRenderTarget(renderContext, nullptr);
                else
                    RenderSetRenderTarget(renderContext, renderTextureTemp[nextTextureIndex]);
                
                RenderDrawFullscreen(renderContext, shaderBlurY, renderTextureTemp[textureIndex]);
            }
        }
        
        // draw actual scene
        SceneDraw(&scene, renderContext);
        
        if (true)
        {
            // handle user input
            float err = 0.0f;
            Vec3 pos = Mat4GetTranslation(s_SceneObject->m_LocalToWorld);
            Vec3 dir = s_Target - pos;
            if ((err = VectorLengthSquared(dir)) > 1e-3f)
            {
                dir.Normalize();
                dir *= fminf(err, 0.2f);
                pos += dir;
                
                Mat4ApplyTranslation(&s_SceneObject->m_LocalToWorld, pos);
                s_SceneObject->m_Flags |= SceneObject::Flags::kDirty;
            }
        }
        
        running = RenderFrameEnd(renderContext);
    }
    
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
    
    MaterialDestroy(shadowMapSampleSimpleMaterial);
    
    TextureDestroy(treeAppleTexture);
    TextureDestroy(treeAppleNormal);
    ShaderDestroy(outlineLightShader);
    MaterialDestroy(treeAppleMaterial);
    
    // PostEffectDestroy(postEffect0);
    // PostEffectDestroy(postEffect1);

    SceneDestroy(&scene);
}

// s_ProcessKeys
//
// Callback what updates our key state
static void s_ProcessKeys(void* data, int key, int scanCode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        float step = 2.0f;
        
        float x=0.0f, y=0.0f, z=0.0f;
        
        if (key == GLFW_KEY_A)
            x -= step;
        if (key == GLFW_KEY_D)
            x += step;
        if (key == GLFW_KEY_W)
            y += step;
        if (key == GLFW_KEY_S)
            y -= step;
        
        if (key == GLFW_KEY_Z)
            z -= step;
        if (key == GLFW_KEY_X)
            z += step;
        
        Vec3 temp;
        VectorSet(&temp, x, y, z);
        
        Vec3 mask  = VectorSign(temp);
        Vec3 maskL = VectorSign(s_LastDir);
        int popCount = VectorEqual(mask, maskL);
        if (popCount < 3)
            s_Strength = 0;
        else
            s_Strength++;
        
        s_LastDir = s_Dir;
        s_Dir.Normalize();
        
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
        {
            const float sign = key == GLFW_KEY_LEFT ? 1.0f : -1.0f;
            
            Mat4 rot;
            float uvw[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
            MatrixSetRotAboutAxis(&rot, uvw, sign*0.0872664625995f);
            
            Mat4 t1;
            MatrixMultiply(&t1, rot, s_SceneObject->m_LocalToWorld);
            MatrixCopy(&s_SceneObject->m_LocalToWorld, t1);
        }
        else
        {
            s_Target = Mat4GetTranslation(s_SceneObject->m_LocalToWorld);
            
            if (key == GLFW_KEY_W || key == GLFW_KEY_S)
                s_Target += Mat4GetUp(s_SceneObject->m_LocalToWorld) * (signbit(y)?-1.0f:1.0f) * step;
            else
                s_Target += Mat4GetRight(s_SceneObject->m_LocalToWorld) * (signbit(x)?-1.0f:1.0f) * step;
        }
        
        s_SceneObject->m_Flags |= SceneObject::Flags::kDirty;
    }
}
