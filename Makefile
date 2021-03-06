# make -k -C /Users/jvalenzu/Source/2dVolumetricLighting -f /Users/jvalenzu/Source/2dVolumetricLighting/Makefile

include Build/rules.mk

CPPFLAGS += -Wunused-variable -Werror
CPPFLAGS += -DGLFW_INCLUDE_GLCOREARB=1

INCLUDES += -IExternal
INCLUDES += -IExternal/include
INCLUDES += -I/usr/local/include

LDFLAGS += -L/usr/local/lib 
LIBRARIES += -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework OpenCL

CLC = /System/Library/Frameworks/OpenCL.framework/Libraries/openclc

# C++ source code to object files
SRCS += Engine/DebugUI.cpp
SRCS += Engine/Light.cpp
SRCS += Engine/Matrix.cpp
SRCS += Engine/Obb.cpp
SRCS += Engine/Scene.cpp
SRCS += Engine/Utils.cpp
SRCS += Render/Material.cpp
SRCS += Render/Render.cpp
SRCS += Render/Texture.cpp
SRCS += Render/PostEffect.cpp
SRCS += Render/Shader.cpp
SRCS += Render/Asset.cpp
SRCS += Render/Model.cpp
SRCS += Tool/Utils.cpp
SRCS += Tool/Test.cpp
SRCS += External/src/imgui/imgui.cpp
SRCS += External/src/imgui/imgui_draw.cpp
SRCS += External/src/lodepng/lodepng.c
OBJS := $(foreach src,$(SRCS),$(call outName,$(src)))

MAIN_OBJ_2D += $(call outName,Main.cpp)
MAIN_OBJ_TS += $(call outName,TriangleSort.cpp)

# Shader code preprocessing
SHADER_SRCS += Render/Shaders/SampleShadowMap.fsh
SHADER_SRCS += Render/Shaders/SampleShadowMap.vsh
SHADER_SRCS += Render/Shaders/ShadowMap1dPoint.fsh
SHADER_SRCS += Render/Shaders/ShadowMap1dPoint.vsh
SHADER_SRCS += Render/Shaders/ShadowMap1dConical.fsh
SHADER_SRCS += Render/Shaders/ShadowMap1dConical.vsh
SHADER_SRCS += Render/Shaders/Simple.fsh
SHADER_SRCS += Render/Shaders/Simple.vsh
SHADER_SRCS += Render/Shaders/SimpleColor.fsh
SHADER_SRCS += Render/Shaders/SimpleColor.vsh
SHADER_SRCS += Render/Shaders/SimpleTransparent.fsh
SHADER_SRCS += Render/Shaders/SimpleTransparent.vsh
SHADER_SRCS += Render/Shaders/blurX.fsh
SHADER_SRCS += Render/Shaders/blurX.vsh
SHADER_SRCS += Render/Shaders/blurY.fsh
SHADER_SRCS += Render/Shaders/blurY.vsh
SHADER_SRCS += Render/Shaders/shadowCasters.fsh
SHADER_SRCS += Render/Shaders/shadowCasters.vsh
SHADER_SRCS += Render/Shaders/Planar.fsh
SHADER_SRCS += Render/Shaders/Planar.vsh
SHADER_SRCS += Render/Shaders/LightPrepass.fsh
SHADER_SRCS += Render/Shaders/LightPrepass.vsh
SHADER_SRCS += Render/Shaders/DebugLightPrepassSample.fsh
SHADER_SRCS += Render/Shaders/DebugLightPrepassSample.vsh
SHADER_SRCS += Render/Shaders/LitColor.fsh
SHADER_SRCS += Render/Shaders/LitColor.vsh
SHADER_SRCS += Render/Shaders/LitWaveFront2.fsh
SHADER_SRCS += Render/Shaders/LitWaveFront2.vsh
SHADER_TRANSFORMED := $(foreach src,$(SHADER_SRCS),$(call shaderOutName,$(src)))

all: 2dVolumetricLighting TriangleSort

2dVolumetricLighting : $(OBJS) $(MAIN_OBJ_2D) $(SHADER_TRANSFORMED) Makefile
	$(CXX) -o $@ $(OBJS) $(MAIN_OBJ_2D) -I$(CURDIR) $(LDFLAGS) $(LIBRARIES)

TriangleSort : $(OBJS) $(MAIN_OBJ_TS) $(SHADER_TRANSFORMED) Makefile
	$(CXX) -o $@ $(OBJS) $(MAIN_OBJ_TS) -I$(CURDIR) $(LDFLAGS) $(LIBRARIES)

shaders : $(SHADER_TRANSFORMED)

$(foreach src,$(SRCS),$(eval $(call srcToObj,$(src))))
$(foreach src,$(SHADER_SRCS),$(eval $(call shaderSrcToObj,$(src))))

$(eval $(call srcToObj,Main.cpp))
$(eval $(call srcToObj,TriangleSort.cpp))

CLEAN += 2dVolumetricLighting
CLEAN += TriangleSort

include Build/rules2.mk
