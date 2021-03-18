#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    //Coordinates in screen space:
    vec3 ndc = gl_Position.xyz / gl_Position.w;
    vec2 viewportCoord = (ndc.xy * 0.5 + 0.5);
    TexCoord = viewportCoord;
}
