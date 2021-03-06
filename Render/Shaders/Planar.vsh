// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

uniform mat4 view;
uniform mat4 modelView;
uniform mat4 normalModel;
uniform mat4 project;
uniform mat4 orthoProject;

in vec3 inPosition; // position attribute
in vec3 inNormal; // normal attribute
in vec4 inColor; // color attribute
in vec2 inTexCoord; // texcoord attribute

out vec2 texCoord;
out vec4 screenPosition;

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position = project * modelView * vec4(inPosition.xyz, 1.0f);
    
    screenPosition = gl_Position;
    texCoord = inTexCoord;
}
