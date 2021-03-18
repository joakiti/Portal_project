#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform vec3 color;
uniform sampler2D texture;

void main() {
    FragColor = texture(texture, TexCoord);
    //This doesn't work
    //FragColor = texture(texture, gl_FragCoord.xy/gl_FragCoord.z);
    //FragColor = vec4(TexCoord, 0, 0);
}
