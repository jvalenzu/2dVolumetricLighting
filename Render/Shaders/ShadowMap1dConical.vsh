#ifdef GL_ES
precision highp float;
#endif

uniform mat4 view;
uniform mat4 modelView;
uniform mat4 normalModel;
uniform mat4 project;

in vec3 inPosition; // position attribute
in vec2 inTexCoord; // texcoord attribute

out vec2 texCoord;
out vec4 screenPosition;

void main(void)
{
    // Transform vertex by modelview projection matrix
    gl_Position = project * modelView * vec4(inPosition.xyz, 1.0);
    screenPosition = gl_Position;
    texCoord = inTexCoord;
}
