#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D wood;
uniform sampler2D smiley;

void main()
{
   FragColor = mix(texture(wood, TexCoord), texture(smiley, TexCoord), 0.2);
}