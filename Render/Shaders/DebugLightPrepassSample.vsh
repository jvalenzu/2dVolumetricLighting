// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

uniform mat4 modelView;
uniform mat4 project;
uniform float _AspectRatio;

in vec3 inPosition;
in vec2 inTexCoord;
in vec4 inColor;

out vec2 texCoord;
out vec4 screenPosition;

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position = project * modelView * vec4(inPosition.xyz, 1.0);
    screenPosition = gl_Position;
    texCoord = inTexCoord;
}
