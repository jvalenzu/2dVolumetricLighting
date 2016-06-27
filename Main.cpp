#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    RenderContext renderContext;
    RenderInit(&renderContext, 1024, 768);
    
    RenderSetProcessKeysCallback(&renderContext, s_ProcessKeys);
    
    MainLoop(&renderContext);
    
    RenderContextDestroy(&renderContext);
    
    glfwTerminate();
    
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
    s_Target = Mat4GetTranslation3(s_SceneObject->m_LocalToWorld);
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
    MaterialSetMaterialPropertyType(treeAppleMaterial, 1, "_LightPosition", Material::MaterialPropertyType::kVec4);
    MaterialSetMaterialPropertyTexture(treeAppleMaterial, 0, treeAppleNormal);
    MaterialSetMaterialPropertyVector(treeAppleMaterial, 1, Vec4(1,1,1,1));

    SceneObject* sceneObjects[4] = { 0 };
    for (int i=0; i<4; ++i)
    {
        SceneObject* sprite1 = sceneObjects[i] = SceneCreateSprite(&scene, MaterialRef(treeAppleMaterial), SpriteOptions::SpriteOptions());
        Mat4ApplyTranslation(&sprite1->m_LocalToWorld, xes[i], yes[i], -1);
        sprite1->m_Flags |= SceneObject::Flags::kDirty;
        
        char name[10];
        sprintf(name, "enemy%d\n", i);
        sprite1->m_DebugName = strdup(name); // jiv fixme
        
        SceneGroupAdd(&scene, shadowCasterGroupId, sprite1);
    }
    
    // create and attach a light to our mover
    spriteOptions.m_Pivot = Vec2(0.5f, 0.0f);
    spriteOptions.m_TintColor = Vec4(1.0f, 0.25f, 0.25f, 1.0f);
    
    SceneObject* lightSprite0 = SceneCreateSpriteFromFile(&scene, "Beam.png", spriteOptions);
    Mat4ApplyTranslation(&lightSprite0->m_LocalToWorld, 10, 0, -1);
    lightSprite0->m_Flags |= SceneObject::Flags::kDirty;
    SceneGroupAddChild(s_SceneObject, lightSprite0);
    
    spriteOptions.m_TintColor = Vec4(0.25f, 1.0f, 0.25f, 1.0f);
    SceneObject* lightSprite1 = SceneCreateSpriteFromFile(&scene, "Beam.png", spriteOptions);
    Mat4ApplyTranslation(&lightSprite1->m_LocalToWorld, -10, 0, -1);
    lightSprite1->m_Flags |= SceneObject::Flags::kDirty;
    SceneGroupAddChild(s_SceneObject, lightSprite1);
    
    SceneObject* light0 = SceneCreateLight(&scene, LightOptions::MakePointLight(Vec3( 10,0,-1), Vec4(1,0.25,0.25,1), 2.0f));
    SceneGroupAddChild(lightSprite0, light0);
    
    SceneObject* light1 = SceneCreateLight(&scene, LightOptions::MakePointLight(Vec3(-10,0,-1), Vec4(0.25,1,0.25,1), 2.0f));
    SceneGroupAddChild(lightSprite1, light1);
    
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
    Shader* shadowMap1dShader = ShaderCreate("obj/Shader/ShadowMap1d");
    Material* shadow1dMaterial = MaterialCreate(shadowMap1dShader, shadowCasterRenderTarget);
    MaterialReserveProperties(shadow1dMaterial, 1);
    MaterialSetMaterialPropertyType(shadow1dMaterial, 0, "_LightPosition", Material::MaterialPropertyType::kVec4);
    MaterialSetMaterialPropertyVector(shadow1dMaterial, 0, Vec4(0.25f, 0.25f, 0.0f, 0.0f));
    
    shadow1dMaterial->m_BlendMode = Material::BlendMode::kOpaque;
    Texture* shadow1dMap = TextureCreateRenderTexture(1024, 1, 0, Texture::RenderTextureFormat::kFloat);
    
    Shader* sampleShadowMapShader = ShaderCreate("obj/Shader/SampleShadowMap");
    Material* shadowMapSampleMaterial = MaterialCreate(sampleShadowMapShader, nullptr);
    MaterialReserveProperties(shadowMapSampleMaterial, 1);
    MaterialSetMaterialPropertyType(shadowMapSampleMaterial, 0, "_LightPosition", Material::MaterialPropertyType::kVec4);
    MaterialSetMaterialPropertyVector(shadowMapSampleMaterial, 0, Vec4(0.25f, 0.25f, 0.0f, 0.0f));
    shadowMapSampleMaterial->m_BlendMode = Material::BlendMode::kBlend;

    bool running = true;
    while (running)
    {
        RenderFrameInit(renderContext);
        
        SceneUpdate(&scene);
        
        // calculate the sceen position of our light source
        Vec3 screenPos = RenderGetScreenPos(renderContext, Mat4GetTranslation3(s_SceneObject->m_LocalToWorld));
        MaterialSetMaterialPropertyVector(shadow1dMaterial, 0, Vec4(screenPos, 0.0f));
        MaterialSetMaterialPropertyVector(shadowMapSampleMaterial, 0, Vec4(screenPos, 0.0f));
        
        for (int i=0; i<ELEMENTSOF(sceneObjects); ++i)
            MaterialSetMaterialPropertyVector(sceneObjects[i]->m_ModelInstance->m_Material, 1, Vec4(screenPos, 0.0f));
        
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
        
        // generate 1d shadow map
        RenderSetRenderTarget(renderContext, shadow1dMap);
        RenderDrawFullscreen(renderContext, shadow1dMaterial, shadowCasterRenderTarget);
        RenderSetRenderTarget(renderContext, nullptr);
        
        // render shadow map
        RenderDrawFullscreen(renderContext, shadowMapSampleMaterial, shadow1dMap);
        
        if (true)
        {
            // blur
            RenderSetRenderTarget(renderContext, renderTextureTemp[0]);
            RenderDrawFullscreen(renderContext, shaderBlurX, nullptr);
            RenderSetRenderTarget(renderContext, renderTextureTemp[1]);
            RenderDrawFullscreen(renderContext, shaderBlurY, renderTextureTemp[0]);
            
            for (int i=1; i<8; ++i)
            {
                const int prevTextureIndex = (i-1)%2;
                const int nextTextureIndex = (i+1)%2;
                const int textureIndex = i%2;
                
                RenderSetRenderTarget(renderContext, renderTextureTemp[textureIndex]);
                RenderDrawFullscreen(renderContext, shaderBlurX, renderTextureTemp[prevTextureIndex]);
                
                if (i==7)
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
            Vec3 pos = Mat4GetTranslation3(s_SceneObject->m_LocalToWorld);
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
    for (int i=0; i<4; ++i)
        SceneObjectDestroy(&scene, sceneObjects[i]);
    
    // destroy blur textures and shaders
    for (int i=0; i<2; ++i)
        TextureDestroy(renderTextureTemp[i]);
    
    ShaderDestroy(shadowCasterShader);
    ShaderDestroy(sampleShadowMapShader);
    ShaderDestroy(shaderBlurX);
    ShaderDestroy(shaderBlurY);
    ShaderDestroy(shadowMap1dShader);
    
    // destroy materials
    MaterialDestroy(shadow1dMaterial);
    MaterialDestroy(shadowMapSampleMaterial);
    
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
        float step = 3.0f;
        
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
        
        // printf("mask  %f/%f/%f\n",  mask.m_X[0],  mask.m_X[1],  mask.m_X[2]);
        // printf("maskL %f/%f/%f\n", maskL.m_X[0], maskL.m_X[1], maskL.m_X[2]);
        // printf("strength %i\n", s_Strength);
        
        // Mat4 rot;
        // float uvw[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
        // MatrixSetRotAboutAxis(&rot, uvw, 0.0872664625995f);
        // 
        // Mat4 t1;
        // MatrixMultiply(&t1, rot, s_SceneObject->m_LocalToWorld);
        // MatrixCopy(&s_SceneObject->m_LocalToWorld, t1);
        
        s_Target = Mat4GetTranslation3(s_SceneObject->m_LocalToWorld);
        s_Target += temp;
        
        s_SceneObject->m_Flags |= SceneObject::Flags::kDirty;
    }
}
