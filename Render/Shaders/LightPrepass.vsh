// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-


#ifdef GL_ES
precision highp float;
#endif

uniform mat4 modelView;
uniform mat4 project;

in vec3 inPosition; // position attribute

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position = project * modelView * vec4(inPosition.xyz, 1.0);
}
