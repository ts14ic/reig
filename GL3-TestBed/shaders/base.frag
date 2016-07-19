#version 330 core

in vec4 fragColor;
in vec2 fragTexCoord;

uniform sampler2D fragTexture;

out vec4 color;

void main() {
    color = texture(fragTexture, fragTexCoord) * fragColor;
}
