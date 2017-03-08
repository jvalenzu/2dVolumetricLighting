// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

uniform mat4 view;
uniform mat4 modelView;
uniform mat4 project;
uniform mat4 localToWorld;
uniform vec4 _CameraPos;

in vec3 inPosition; // position attribute
in vec3 inNormal;   // normal attribute
in vec4 inColor;    // color attribute

// outputs
out vec4 colorV;
out vec4 normalV;
out vec4 cameraDirectionV;

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position        = project * modelView * vec4(inPosition.xyz, 1.0);
    normalV            = localToWorld * vec4(inNormal.xyz, 1.0);
    colorV             = inColor;
    
    vec4 worldPosition = localToWorld * vec4(inPosition.xyz, 1.0);
    cameraDirectionV = _CameraPos - worldPosition;
    
    colorV.a = 1;
    colorV.r = inPosition.x > 0 ? 1 : 0;
    colorV.g = inPosition.y > 0 ? 1 : 0;
    colorV.b = inPosition.z > 0 ? 1 : 0;
}
