// -*- mode: glsl; tab-width: 4; c-basic-offset: 4; -*-

#ifdef GL_ES
precision highp float;
#endif

in vec3 inPosition; // position attribute
in vec2 inTexCoord; // color attribute

out vec4 colorV; // output color
out vec2 texCoord;

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position = vec4(inPosition.xyz, 1.0);
    texCoord = inTexCoord;
}
